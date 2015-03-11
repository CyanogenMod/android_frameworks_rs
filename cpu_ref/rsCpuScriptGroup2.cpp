#include "rsCpuScriptGroup2.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <set>
#include <sstream>
#include <string>
#include <vector>

#ifndef RS_COMPATIBILITY_LIB
#include "bcc/Config/Config.h"
#include <sys/wait.h>
#endif

#include "cpu_ref/rsCpuCore.h"
#include "rsClosure.h"
#include "rsContext.h"
#include "rsCpuCore.h"
#include "rsCpuExecutable.h"
#include "rsCpuScript.h"
#include "rsScript.h"
#include "rsScriptGroup2.h"
#include "rsScriptIntrinsic.h"

using std::string;
using std::vector;

namespace android {
namespace renderscript {

namespace {

const size_t DefaultKernelArgCount = 2;

void groupRoot(const RsExpandKernelParams *kparams, uint32_t xstart,
               uint32_t xend, uint32_t outstep) {
    const List<CPUClosure*>& closures = *(List<CPUClosure*>*)kparams->usr;
    RsExpandKernelParams *mutable_kparams = (RsExpandKernelParams *)kparams;
    const void **oldIns  = kparams->ins;
    uint32_t *oldStrides = kparams->inEStrides;

    std::vector<const void*> ins(DefaultKernelArgCount);
    std::vector<uint32_t> strides(DefaultKernelArgCount);

    for (CPUClosure* cpuClosure : closures) {
        const Closure* closure = cpuClosure->mClosure;

        auto in_iter = ins.begin();
        auto stride_iter = strides.begin();

        for (size_t i = 0; i < closure->mNumArg; i++) {
            const void* arg = closure->mArgs[i];
            const Allocation* a = (const Allocation*)arg;
            const uint32_t eStride = a->mHal.state.elementSizeBytes;
            const uint8_t* ptr = (uint8_t*)(a->mHal.drvState.lod[0].mallocPtr) +
                    eStride * xstart;
            if (kparams->dimY > 1) {
                ptr += a->mHal.drvState.lod[0].stride * kparams->y;
            }
            *in_iter++ = ptr;
            *stride_iter++ = eStride;
        }

        mutable_kparams->ins = &ins[0];
        mutable_kparams->inEStrides = &strides[0];

        const Allocation* out = closure->mReturnValue;
        const uint32_t ostep = out->mHal.state.elementSizeBytes;
        const uint8_t* ptr = (uint8_t *)(out->mHal.drvState.lod[0].mallocPtr) +
                ostep * xstart;
        if (kparams->dimY > 1) {
            ptr += out->mHal.drvState.lod[0].stride * kparams->y;
        }

        mutable_kparams->out = (void*)ptr;

        cpuClosure->mFunc(kparams, xstart, xend, ostep);
    }

    mutable_kparams->ins        = oldIns;
    mutable_kparams->inEStrides = oldStrides;
}

}  // namespace

Batch::Batch(CpuScriptGroup2Impl* group, const char* name) :
    mGroup(group), mFunc(nullptr) {
    mName = strndup(name, strlen(name));
}

Batch::~Batch() {
    for (CPUClosure* c : mClosures) {
        delete c;
    }
    free(mName);
}

bool Batch::conflict(CPUClosure* cpuClosure) const {
    if (mClosures.empty()) {
        return false;
    }

    const Closure* closure = cpuClosure->mClosure;

    if (!closure->mIsKernel || !mClosures.front()->mClosure->mIsKernel) {
        // An invoke should be in a batch by itself, so it conflicts with any other
        // closure.
        return true;
    }

    const auto& globalDeps = closure->mGlobalDeps;
    const auto& argDeps = closure->mArgDeps;

    for (CPUClosure* c : mClosures) {
        const Closure* batched = c->mClosure;
        if (globalDeps.find(batched) != globalDeps.end()) {
            return true;
        }
        const auto& it = argDeps.find(batched);
        if (it != argDeps.end()) {
            const auto& args = (*it).second;
            for (const auto &p1 : *args) {
                if (p1.second->get() != nullptr) {
                    return true;
                }
            }
        }
    }

    return false;
}

CpuScriptGroup2Impl::CpuScriptGroup2Impl(RsdCpuReferenceImpl *cpuRefImpl,
                                         const ScriptGroupBase *sg) :
    mCpuRefImpl(cpuRefImpl), mGroup((const ScriptGroup2*)(sg)),
    mExecutable(nullptr), mScriptObj(nullptr) {
    rsAssert(!mGroup->mClosures.empty());

    Batch* batch = new Batch(this, "Batch0");
    int i = 0;
    for (Closure* closure: mGroup->mClosures) {
        CPUClosure* cc;
        const IDBase* funcID = closure->mFunctionID.get();
        RsdCpuScriptImpl* si =
                (RsdCpuScriptImpl *)mCpuRefImpl->lookupScript(funcID->mScript);
        if (closure->mIsKernel) {
            MTLaunchStruct mtls;
            si->forEachKernelSetup(funcID->mSlot, &mtls);
            cc = new CPUClosure(closure, si, (ExpandFuncTy)mtls.kernel);
        } else {
            cc = new CPUClosure(closure, si);
        }

        if (batch->conflict(cc)) {
            mBatches.push_back(batch);
            std::stringstream ss;
            ss << "Batch" << ++i;
            batch = new Batch(this, ss.str().c_str());
        }

        batch->mClosures.push_back(cc);
    }

    rsAssert(!batch->mClosures.empty());
    mBatches.push_back(batch);

#ifndef RS_COMPATIBILITY_LIB
    compile(mGroup->mCacheDir);
    if (mScriptObj != nullptr && mExecutable != nullptr) {
        for (Batch* batch : mBatches) {
            batch->resolveFuncPtr(mScriptObj);
        }
    }
#endif  // RS_COMPATIBILITY_LIB
}

void Batch::resolveFuncPtr(void* sharedObj) {
    std::string funcName(mName);
    if (mClosures.front()->mClosure->mIsKernel) {
        funcName.append(".expand");
    }
    mFunc = dlsym(sharedObj, funcName.c_str());
    rsAssert (mFunc != nullptr);
}

CpuScriptGroup2Impl::~CpuScriptGroup2Impl() {
    for (Batch* batch : mBatches) {
        delete batch;
    }
    // TODO: move this dlclose into ~ScriptExecutable().
    if (mScriptObj != nullptr) {
        dlclose(mScriptObj);
    }
    delete mExecutable;
}

namespace {

#ifndef RS_COMPATIBILITY_LIB

string getCoreLibPath(Context* context, string* coreLibRelaxedPath) {
    *coreLibRelaxedPath = "";

    // If we're debugging, use the debug library.
    if (context->getContextType() == RS_CONTEXT_TYPE_DEBUG) {
        return SYSLIBPATH"/libclcore_debug.bc";
    }

    // Check for a platform specific library

#if defined(ARCH_ARM_HAVE_NEON) && !defined(DISABLE_CLCORE_NEON)
    // NEON-capable ARMv7a devices can use an accelerated math library
    // for all reduced precision scripts.
    // ARMv8 does not use NEON, as ASIMD can be used with all precision
    // levels.
    *coreLibRelaxedPath = SYSLIBPATH"/libclcore_neon.bc";
#endif

#if defined(__i386__) || defined(__x86_64__)
    // x86 devices will use an optimized library.
    return SYSLIBPATH"/libclcore_x86.bc";
#else
    return SYSLIBPATH"/libclcore.bc";
#endif
}

string getFileName(string path) {
    unsigned found = path.find_last_of("/\\");
    return path.substr(found + 1);
}

void setupCompileArguments(
        const vector<string>& inputs, const vector<string>& kernelBatches,
        const vector<string>& invokeBatches,
        const string& output_dir, const string& output_filename,
        const string& coreLibPath, const string& coreLibRelaxedPath,
        vector<const char*>* args) {
    args->push_back(RsdCpuScriptImpl::BCC_EXE_PATH);
    args->push_back("-fPIC");
    args->push_back("-embedRSInfo");
    args->push_back("-mtriple");
    args->push_back(DEFAULT_TARGET_TRIPLE_STRING);
    args->push_back("-bclib");
    args->push_back(coreLibPath.c_str());
    args->push_back("-bclib_relaxed");
    args->push_back(coreLibRelaxedPath.c_str());
    for (const string& input : inputs) {
        args->push_back(input.c_str());
    }
    for (const string& batch : kernelBatches) {
        args->push_back("-merge");
        args->push_back(batch.c_str());
    }
    for (const string& batch : invokeBatches) {
        args->push_back("-invoke");
        args->push_back(batch.c_str());
    }
    args->push_back("-output_path");
    args->push_back(output_dir.c_str());
    args->push_back("-o");
    args->push_back(output_filename.c_str());
    args->push_back(nullptr);
}

bool fuseAndCompile(const char** arguments,
                    const string& commandLine) {
    const pid_t pid = fork();

    if (pid == -1) {
        ALOGE("Couldn't fork for bcc execution");
        return false;
    }

    if (pid == 0) {
        // Child process
        ALOGV("Invoking BCC with: %s", commandLine.c_str());
        execv(RsdCpuScriptImpl::BCC_EXE_PATH, (char* const*)arguments);

        ALOGE("execv() failed: %s", strerror(errno));
        abort();
        return false;
    }

    // Parent process
    int status = 0;
    const pid_t w = waitpid(pid, &status, 0);
    if (w == -1) {
        return false;
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0 ) {
        ALOGE("bcc terminated unexpectedly");
        return false;
    }

    return true;
}

void generateSourceSlot(const Closure& closure,
                        const std::vector<std::string>& inputs,
                        std::stringstream& ss) {
    const IDBase* funcID = (const IDBase*)closure.mFunctionID.get();
    const Script* script = funcID->mScript;

    rsAssert (!script->isIntrinsic());

    const RsdCpuScriptImpl *cpuScript =
            (const RsdCpuScriptImpl*)script->mHal.drv;
    const string& bitcodeFilename = cpuScript->getBitcodeFilePath();

    const int index = find(inputs.begin(), inputs.end(), bitcodeFilename) -
            inputs.begin();

    ss << index << "," << funcID->mSlot << ".";
}

#endif  // RS_COMPATIBILTY_LIB

}  // anonymous namespace

void CpuScriptGroup2Impl::compile(const char* cacheDir) {
#ifndef RS_COMPATIBILITY_LIB
    if (mGroup->mClosures.size() < 2) {
        return;
    }

    //===--------------------------------------------------------------------===//
    // Fuse the input kernels and generate native code in an object file
    //===--------------------------------------------------------------------===//

    std::set<string> inputSet;
    for (Closure* closure : mGroup->mClosures) {
        const Script* script = closure->mFunctionID.get()->mScript;

        // If any script is an intrinsic, give up trying fusing the kernels.
        if (script->isIntrinsic()) {
            return;
        }

        const RsdCpuScriptImpl *cpuScript =
                (const RsdCpuScriptImpl*)script->mHal.drv;
        const string& bitcodeFilename = cpuScript->getBitcodeFilePath();
        inputSet.insert(bitcodeFilename);
    }

    std::vector<string> inputs(inputSet.begin(), inputSet.end());

    std::vector<string> kernelBatches;
    std::vector<string> invokeBatches;

    int i = 0;
    for (const auto& batch : mBatches) {
        rsAssert(batch->size() > 0);

        std::stringstream ss;
        ss << batch->mName << ":";

        if (!batch->mClosures.front()->mClosure->mIsKernel) {
            rsAssert(batch->size() == 1);
            generateSourceSlot(*batch->mClosures.front()->mClosure, inputs, ss);
            invokeBatches.push_back(ss.str());
        } else {
            for (const auto& cpuClosure : batch->mClosures) {
                generateSourceSlot(*cpuClosure->mClosure, inputs, ss);
            }
            kernelBatches.push_back(ss.str());
        }
    }

    rsAssert(cacheDir != nullptr);
    string objFilePath(cacheDir);
    objFilePath.append("/fusedXXXXXX.o");
    // Find unique object file name, to make following file names unique.
    int tempfd = mkstemps(&objFilePath[0], 2);
    if (tempfd == -1) {
      return;
    }
    TEMP_FAILURE_RETRY(close(tempfd));

    string outputFileName = getFileName(objFilePath.substr(0, objFilePath.size() - 2));
    string coreLibRelaxedPath;
    const string& coreLibPath = getCoreLibPath(getCpuRefImpl()->getContext(),
                                               &coreLibRelaxedPath);
    vector<const char*> arguments;
    setupCompileArguments(inputs, kernelBatches, invokeBatches, cacheDir,
                          outputFileName, coreLibPath, coreLibRelaxedPath, &arguments);
    std::unique_ptr<const char> joined(
        rsuJoinStrings(arguments.size() - 1, arguments.data()));
    string commandLine (joined.get());

    if (!fuseAndCompile(arguments.data(), commandLine)) {
        unlink(objFilePath.c_str());
        return;
    }

    //===--------------------------------------------------------------------===//
    // Create and load the shared lib
    //===--------------------------------------------------------------------===//

    const char* resName = outputFileName.c_str();

    if (!SharedLibraryUtils::createSharedLibrary(cacheDir, resName)) {
        ALOGE("Failed to link object file '%s'", resName);
        return;
    }

    mScriptObj = SharedLibraryUtils::loadSharedLibrary(cacheDir, resName);
    if (mScriptObj == nullptr) {
        ALOGE("Unable to load '%s'", resName);
        return;
    }

    mExecutable = ScriptExecutable::createFromSharedObject(
        nullptr,  // RS context. Unused.
        mScriptObj);

#endif  // RS_COMPATIBILITY_LIB
}

void CpuScriptGroup2Impl::execute() {
    for (auto batch : mBatches) {
        batch->setGlobalsForBatch();
        batch->run();
    }
}

void Batch::setGlobalsForBatch() {
    for (CPUClosure* cpuClosure : mClosures) {
        const Closure* closure = cpuClosure->mClosure;
        const IDBase* funcID = closure->mFunctionID.get();
        Script* s = funcID->mScript;;
        for (const auto& p : closure->mGlobals) {
            const void* value = p.second.first;
            int size = p.second.second;
            if (value == nullptr && size == 0) {
                // This indicates the current closure depends on another closure for a
                // global in their shared module (script). In this case we don't need to
                // copy the value. For example, an invoke intializes a global variable
                // which a kernel later reads.
                continue;
            }
            rsAssert(p.first != nullptr);
            ALOGV("Evaluating closure %p, setting field %p (Script %p, slot: %d)",
                  closure, p.first, p.first->mScript, p.first->mSlot);
            Script* script = p.first->mScript;
            const RsdCpuScriptImpl *cpuScript =
                    (const RsdCpuScriptImpl*)script->mHal.drv;
            int slot = p.first->mSlot;
            ScriptExecutable* exec = mGroup->getExecutable();
            if (exec != nullptr) {
                const char* varName = cpuScript->getFieldName(slot);
                void* addr = exec->getFieldAddress(varName);
                if (size < 0) {
                    rsrSetObject(mGroup->getCpuRefImpl()->getContext(),
                                 (rs_object_base*)addr, (ObjectBase*)value);
                } else {
                    memcpy(addr, (const void*)&value, size);
                }
            } else {
                // We use -1 size to indicate an ObjectBase rather than a primitive type
                if (size < 0) {
                    s->setVarObj(slot, (ObjectBase*)value);
                } else {
                    s->setVar(slot, (const void*)&value, size);
                }
            }
        }
    }
}

void Batch::run() {
    if (!mClosures.front()->mClosure->mIsKernel) {
        rsAssert(mClosures.size() == 1);

        // This batch contains a single closure for an invoke function
        CPUClosure* cc = mClosures.front();
        const Closure* c = cc->mClosure;

        if (mFunc != nullptr) {
            // TODO: Need align pointers for x86_64.
            // See RsdCpuScriptImpl::invokeFunction in rsCpuScript.cpp
            ((InvokeFuncTy)mFunc)(c->mParams, c->mParamLength);
        } else {
            const ScriptInvokeID* invokeID = (const ScriptInvokeID*)c->mFunctionID.get();
            rsAssert(invokeID != nullptr);
            cc->mSi->invokeFunction(invokeID->mSlot, c->mParams, c->mParamLength);
        }

        return;
    }

    if (mFunc != nullptr) {
        MTLaunchStruct mtls;
        const CPUClosure* firstCpuClosure = mClosures.front();
        const CPUClosure* lastCpuClosure = mClosures.back();

        firstCpuClosure->mSi->forEachMtlsSetup(
                (const Allocation**)firstCpuClosure->mClosure->mArgs,
                firstCpuClosure->mClosure->mNumArg,
                lastCpuClosure->mClosure->mReturnValue,
                nullptr, 0, nullptr, &mtls);

        mtls.script = nullptr;
        mtls.fep.usr = nullptr;
        mtls.kernel = (ForEachFunc_t)mFunc;

        mGroup->getCpuRefImpl()->launchThreads(
                (const Allocation**)firstCpuClosure->mClosure->mArgs,
                firstCpuClosure->mClosure->mNumArg,
                lastCpuClosure->mClosure->mReturnValue,
                nullptr, &mtls);

        return;
    }

    for (CPUClosure* cpuClosure : mClosures) {
        const Closure* closure = cpuClosure->mClosure;
        const ScriptKernelID* kernelID =
                (const ScriptKernelID*)closure->mFunctionID.get();
        cpuClosure->mSi->preLaunch(kernelID->mSlot,
                                   (const Allocation**)closure->mArgs,
                                   closure->mNumArg, closure->mReturnValue,
                                   nullptr, 0, nullptr);
    }

    const CPUClosure* cpuClosure = mClosures.front();
    const Closure* closure = cpuClosure->mClosure;
    MTLaunchStruct mtls;

    if (cpuClosure->mSi->forEachMtlsSetup((const Allocation**)closure->mArgs,
                                          closure->mNumArg,
                                          closure->mReturnValue,
                                          nullptr, 0, nullptr, &mtls)) {

        mtls.script = nullptr;
        mtls.kernel = (void (*)())&groupRoot;
        mtls.fep.usr = &mClosures;

        mGroup->getCpuRefImpl()->launchThreads(nullptr, 0, nullptr, nullptr, &mtls);
    }

    for (CPUClosure* cpuClosure : mClosures) {
        const Closure* closure = cpuClosure->mClosure;
        const ScriptKernelID* kernelID =
                (const ScriptKernelID*)closure->mFunctionID.get();
        cpuClosure->mSi->postLaunch(kernelID->mSlot,
                                    (const Allocation**)closure->mArgs,
                                    closure->mNumArg, closure->mReturnValue,
                                    nullptr, 0, nullptr);
    }
}

}  // namespace renderscript
}  // namespace android
