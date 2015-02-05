#include "rsCpuScriptGroup2.h"

#include <dlfcn.h>

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
    const list<CPUClosure*>& closures = *(list<CPUClosure*>*)kparams->usr;
    RsExpandKernelParams *mutable_kparams = (RsExpandKernelParams *)kparams;
    const void **oldIns  = kparams->ins;
    uint32_t *oldStrides = kparams->inEStrides;

    std::vector<const void*> ins(DefaultKernelArgCount);
    std::vector<uint32_t> strides(DefaultKernelArgCount);

    for (CPUClosure* cpuClosure : closures) {
        const Closure* closure = cpuClosure->mClosure;

        auto in_iter = ins.begin();
        auto stride_iter = strides.begin();

        for (const auto& arg : closure->mArgs) {
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

        mutable_kparams->usr = cpuClosure->mUsrPtr;

        cpuClosure->mFunc(kparams, xstart, xend, ostep);
    }

    mutable_kparams->ins        = oldIns;
    mutable_kparams->inEStrides = oldStrides;
    mutable_kparams->usr        = &closures;
}

}  // namespace

Batch::~Batch() {
    for (CPUClosure* c : mClosures) {
        delete c;
    }
    if (mScriptObj) {
        dlclose(mScriptObj);
    }
}

bool Batch::conflict(CPUClosure* closure) const {
    if (mClosures.empty()) {
        return false;
    }

    if (closure->mClosure->mKernelID.get() == nullptr ||
        mClosures.front()->mClosure->mKernelID.get() == nullptr) {
        // An invoke should be in a batch by itself, so it conflicts with any other
        // closure.
        return true;
    }

    for (const auto &p : closure->mClosure->mGlobalDeps) {
        const Closure* dep = p.first;
        for (CPUClosure* c : mClosures) {
            if (c->mClosure == dep) {
                ALOGV("ScriptGroup2: closure %p conflicting with closure %p via its global",
                      closure, dep);
                return true;
            }
        }
    }
    for (const auto &p : closure->mClosure->mArgDeps) {
        const Closure* dep = p.first;
        for (CPUClosure* c : mClosures) {
            if (c->mClosure == dep) {
                for (const auto &p1 : *p.second) {
                    if (p1.second->get() != nullptr) {
                        ALOGV("ScriptGroup2: closure %p conflicting with closure %p via its arg",
                              closure, dep);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

CpuScriptGroup2Impl::CpuScriptGroup2Impl(RsdCpuReferenceImpl *cpuRefImpl,
                                         const ScriptGroupBase *sg) :
    mCpuRefImpl(cpuRefImpl), mGroup((const ScriptGroup2*)(sg)) {
    rsAssert(!mGroup->mClosures.empty());

    Batch* batch = new Batch(this);
    for (Closure* closure: mGroup->mClosures) {
        const ScriptKernelID* kernelID = closure->mKernelID.get();
        RsdCpuScriptImpl* si;
        CPUClosure* cc;
        if (kernelID != nullptr) {
            si = (RsdCpuScriptImpl *)mCpuRefImpl->lookupScript(kernelID->mScript);
            MTLaunchStruct mtls;
            si->forEachKernelSetup(kernelID->mSlot, &mtls);
            // TODO: Is mtls.fep.usrLen ever used?
            cc = new CPUClosure(closure, si, (ExpandFuncTy)mtls.kernel,
                                mtls.fep.usr, mtls.fep.usrLen);
        } else {
            si = (RsdCpuScriptImpl *)mCpuRefImpl->lookupScript(
                    closure->mInvokeID->mScript);
            cc = new CPUClosure(closure, si);
        }

        if (batch->conflict(cc)) {
            mBatches.push_back(batch);
            batch = new Batch(this);
        }

        batch->mClosures.push_back(cc);
    }

    rsAssert(!batch->mClosures.empty());
    mBatches.push_back(batch);

#ifndef RS_COMPATIBILITY_LIB
    for (Batch* batch : mBatches) {
        batch->tryToCreateFusedKernel(mGroup->mCacheDir.c_str());
    }
#endif
}

CpuScriptGroup2Impl::~CpuScriptGroup2Impl() {
    for (Batch* batch : mBatches) {
        delete batch;
    }
}

namespace {

#ifndef RS_COMPATIBILITY_LIB

string getFileName(string path) {
    unsigned found = path.find_last_of("/\\");
    return path.substr(found + 1);
}

void setupCompileArguments(
        const vector<string>& inputs, const vector<int>& kernels,
        const string& output_dir, const string& output_filename,
        const string& rsLib, vector<const char*>* args) {
    args->push_back(RsdCpuScriptImpl::BCC_EXE_PATH);
    args->push_back("-fPIC");
    args->push_back("-embedRSInfo");
    args->push_back("-mtriple");
    args->push_back(DEFAULT_TARGET_TRIPLE_STRING);
    args->push_back("-bclib");
    args->push_back(rsLib.c_str());
    for (const string& input : inputs) {
        args->push_back(input.c_str());
    }
    for (int kernel : kernels) {
        args->push_back("-k");
        string strKernel = std::to_string(kernel);
        args->push_back(strKernel.c_str());
    }
    args->push_back("-output_path");
    args->push_back(output_dir.c_str());
    args->push_back("-o");
    args->push_back(output_filename.c_str());
    args->push_back(nullptr);
}

string convertListToString(int n, const char* const* strs) {
    string ret;
    ret.append(strs[0]);
    for (int i = 1; i < n; i++) {
        ret.append(" ");
        ret.append(strs[i]);
    }
    return ret;
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
#endif

}  // anonymous namespace

void Batch::tryToCreateFusedKernel(const char *cacheDir) {
#ifndef RS_COMPATIBILITY_LIB
    if (mClosures.size() < 2) {
        return;
    }

    //===--------------------------------------------------------------------===//
    // Fuse the input kernels and generate native code in an object file
    //===--------------------------------------------------------------------===//

    std::vector<string> inputFiles;
    std::vector<int> slots;

    for (CPUClosure* cpuClosure : mClosures) {
        const Closure* closure = cpuClosure->mClosure;
        const ScriptKernelID* kernelID = closure->mKernelID.get();
        const Script* script = kernelID->mScript;

        if (script->isIntrinsic()) {
            return;
        }

        const RsdCpuScriptImpl *cpuScript =
                (const RsdCpuScriptImpl*)script->mHal.drv;

        const string& bitcodeFilename = cpuScript->getBitcodeFilePath();

        inputFiles.push_back(bitcodeFilename);
        slots.push_back(kernelID->mSlot);
    }

    string outputPath(tempnam(cacheDir, "fused"));
    string outputFileName = getFileName(outputPath);
    string objFilePath(outputPath);
    objFilePath.append(".o");
    string rsLibPath(SYSLIBPATH"/libclcore.bc");
    vector<const char*> arguments;
    setupCompileArguments(inputFiles, slots, cacheDir, outputFileName, rsLibPath,
                          &arguments);
    string commandLine =
            convertListToString(arguments.size() - 1, arguments.data());

    if (!fuseAndCompile(arguments.data(), commandLine)) {
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

    void* mSharedObj = SharedLibraryUtils::loadSharedLibrary(cacheDir, resName);
    if (mSharedObj == nullptr) {
        ALOGE("Unable to load '%s'", resName);
        return;
    }

    mExecutable = ScriptExecutable::createFromSharedObject(
                                                           nullptr,  // RS context. Unused.
                                                           mSharedObj);

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
        const ScriptKernelID* kernelID = closure->mKernelID.get();
        Script* s;
        if (kernelID != nullptr) {
            s = kernelID->mScript;
        } else {
            s = cpuClosure->mClosure->mInvokeID->mScript;
        }
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
            // We use -1 size to indicate an ObjectBase rather than a primitive type
            if (size < 0) {
                s->setVarObj(p.first->mSlot, (ObjectBase*)value);
            } else {
                s->setVar(p.first->mSlot, (const void*)&value, size);
            }
        }
    }
}

void Batch::run() {
    if (mExecutable != nullptr) {
        MTLaunchStruct mtls;
        const CPUClosure* firstCpuClosure = mClosures.front();
        const CPUClosure* lastCpuClosure = mClosures.back();

        firstCpuClosure->mSi->forEachMtlsSetup(
                (const Allocation**)&firstCpuClosure->mClosure->mArgs[0],
                firstCpuClosure->mClosure->mArgs.size(),
                lastCpuClosure->mClosure->mReturnValue,
                nullptr, 0, nullptr, &mtls);

        mtls.script = nullptr;
        mtls.fep.usr = nullptr;
        mtls.kernel = mExecutable->getForEachFunction(0);

        mGroup->getCpuRefImpl()->launchThreads(
                (const Allocation**)&firstCpuClosure->mClosure->mArgs[0],
                firstCpuClosure->mClosure->mArgs.size(),
                lastCpuClosure->mClosure->mReturnValue,
                nullptr, &mtls);

        return;
    }

    if (mClosures.size() == 1 &&
        mClosures.front()->mClosure->mKernelID.get() == nullptr) {
        // This closure is for an invoke function
        CPUClosure* cc = mClosures.front();
        const Closure* c = cc->mClosure;
        const ScriptInvokeID* invokeID = c->mInvokeID;
        rsAssert(invokeID != nullptr);
        cc->mSi->invokeFunction(invokeID->mSlot, c->mParams, c->mParamLength);
        return;
    }

    for (CPUClosure* cpuClosure : mClosures) {
        const Closure* closure = cpuClosure->mClosure;
        const ScriptKernelID* kernelID = closure->mKernelID.get();
        cpuClosure->mSi->preLaunch(kernelID->mSlot,
                                   (const Allocation**)&closure->mArgs[0],
                                   closure->mArgs.size(), closure->mReturnValue,
                                   cpuClosure->mUsrPtr, cpuClosure->mUsrSize,
                                   nullptr);
    }

    const CPUClosure* cpuClosure = mClosures.front();
    const Closure* closure = cpuClosure->mClosure;
    MTLaunchStruct mtls;

    if (cpuClosure->mSi->forEachMtlsSetup((const Allocation**)&closure->mArgs[0],
                                          closure->mArgs.size(),
                                          closure->mReturnValue,
                                          nullptr, 0, nullptr, &mtls)) {

        mtls.script = nullptr;
        mtls.kernel = (void (*)())&groupRoot;
        mtls.fep.usr = &mClosures;

        mGroup->getCpuRefImpl()->launchThreads(nullptr, 0, nullptr, nullptr, &mtls);
    }

    for (CPUClosure* cpuClosure : mClosures) {
        const Closure* closure = cpuClosure->mClosure;
        const ScriptKernelID* kernelID = closure->mKernelID.get();
        cpuClosure->mSi->postLaunch(kernelID->mSlot,
                                    (const Allocation**)&closure->mArgs[0],
                                    closure->mArgs.size(), closure->mReturnValue,
                                    nullptr, 0, nullptr);
    }
}

}  // namespace renderscript
}  // namespace android
