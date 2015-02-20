/*
 * Copyright (C) 2011-2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "rsCpuCore.h"
#include "rsCpuScript.h"
#include "rsCpuExecutable.h"

#ifdef RS_COMPATIBILITY_LIB
    #include <stdio.h>
    #include <sys/stat.h>
    #include <unistd.h>
#else
    #include <bcc/BCCContext.h>
    #include <bcc/Config/Config.h>
    #include <bcc/Renderscript/RSCompilerDriver.h>
    #include <bcinfo/MetadataExtractor.h>
    #include <cutils/properties.h>

    #include <sys/types.h>
    #include <sys/wait.h>
    #include <unistd.h>

    #include <string>
    #include <vector>
#endif

#include <set>
#include <string>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#ifdef __LP64__
#define SYSLIBPATH "/system/lib64"
#else
#define SYSLIBPATH "/system/lib"
#endif

namespace {
#ifndef RS_COMPATIBILITY_LIB

static bool is_force_recompile() {
#ifdef RS_SERVER
  return false;
#else
  char buf[PROPERTY_VALUE_MAX];

  // Re-compile if floating point precision has been overridden.
  property_get("debug.rs.precision", buf, "");
  if (buf[0] != '\0') {
    return true;
  }

  // Re-compile if debug.rs.forcerecompile is set.
  property_get("debug.rs.forcerecompile", buf, "0");
  if ((::strcmp(buf, "1") == 0) || (::strcmp(buf, "true") == 0)) {
    return true;
  } else {
    return false;
  }
#endif  // RS_SERVER
}

static void setCompileArguments(std::vector<const char*>* args,
                                const std::string& bcFileName,
                                const char* cacheDir, const char* resName,
                                const char* core_lib, bool useRSDebugContext,
                                const char* bccPluginName) {
    rsAssert(cacheDir && resName && core_lib);
    args->push_back(android::renderscript::RsdCpuScriptImpl::BCC_EXE_PATH);
    args->push_back("-unroll-runtime");
    args->push_back("-scalarize-load-store");
    args->push_back("-o");
    args->push_back(resName);
    args->push_back("-output_path");
    args->push_back(cacheDir);
    args->push_back("-bclib");
    args->push_back(core_lib);
    args->push_back("-mtriple");
    args->push_back(DEFAULT_TARGET_TRIPLE_STRING);

    // Enable workaround for A53 codegen by default.
#if defined(__aarch64__) && !defined(DISABLE_A53_WORKAROUND)
    args->push_back("-aarch64-fix-cortex-a53-835769");
#endif

    // Execute the bcc compiler.
    if (useRSDebugContext) {
        args->push_back("-rs-debug-ctx");
    } else {
        // Only load additional libraries for compiles that don't use
        // the debug context.
        if (bccPluginName && strlen(bccPluginName) > 0) {
            args->push_back("-load");
            args->push_back(bccPluginName);
        }
    }

    args->push_back("-fPIC");
    args->push_back("-embedRSInfo");

    args->push_back(bcFileName.c_str());
    args->push_back(nullptr);
}

static bool compileBitcode(const std::string &bcFileName,
                           const char *bitcode,
                           size_t bitcodeSize,
                           const char **compileArguments,
                           const std::string &compileCommandLine) {
    rsAssert(bitcode && bitcodeSize);

    FILE *bcfile = fopen(bcFileName.c_str(), "w");
    if (!bcfile) {
        ALOGE("Could not write to %s", bcFileName.c_str());
        return false;
    }
    size_t nwritten = fwrite(bitcode, 1, bitcodeSize, bcfile);
    fclose(bcfile);
    if (nwritten != bitcodeSize) {
        ALOGE("Could not write %zu bytes to %s", bitcodeSize,
              bcFileName.c_str());
        return false;
    }

    pid_t pid = fork();

    switch (pid) {
    case -1: {  // Error occurred (we attempt no recovery)
        ALOGE("Couldn't fork for bcc compiler execution");
        return false;
    }
    case 0: {  // Child process
        ALOGV("Invoking BCC with: %s", compileCommandLine.c_str());
        execv(android::renderscript::RsdCpuScriptImpl::BCC_EXE_PATH,
              (char* const*)compileArguments);

        ALOGE("execv() failed: %s", strerror(errno));
        abort();
        return false;
    }
    default: {  // Parent process (actual driver)
        // Wait on child process to finish compiling the source.
        int status = 0;
        pid_t w = waitpid(pid, &status, 0);
        if (w == -1) {
            ALOGE("Could not wait for bcc compiler");
            return false;
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return true;
        }

        ALOGE("bcc compiler terminated unexpectedly");
        return false;
    }
    }
}

#endif  // !defined(RS_COMPATIBILITY_LIB)
}  // namespace

namespace android {
namespace renderscript {

RsdCpuScriptImpl::RsdCpuScriptImpl(RsdCpuReferenceImpl *ctx, const Script *s) {
    mCtx = ctx;
    mScript = s;

    mScriptSO = nullptr;

#ifndef RS_COMPATIBILITY_LIB
    mCompilerDriver = nullptr;
#endif


    mRoot = nullptr;
    mRootExpand = nullptr;
    mInit = nullptr;
    mFreeChildren = nullptr;
    mScriptExec = nullptr;

    mBoundAllocs = nullptr;
    mIntrinsicData = nullptr;
    mIsThreadable = true;
}

bool RsdCpuScriptImpl::storeRSInfoFromSO() {
    mRoot = (RootFunc_t) dlsym(mScriptSO, "root");
    if (mRoot) {
        //ALOGE("Found root(): %p", mRoot);
    }
    mRootExpand = (RootFunc_t) dlsym(mScriptSO, "root.expand");
    if (mRootExpand) {
        //ALOGE("Found root.expand(): %p", mRootExpand);
    }
    mInit = (InvokeFunc_t) dlsym(mScriptSO, "init");
    if (mInit) {
        //ALOGE("Found init(): %p", mInit);
    }
    mFreeChildren = (InvokeFunc_t) dlsym(mScriptSO, ".rs.dtor");
    if (mFreeChildren) {
        //ALOGE("Found .rs.dtor(): %p", mFreeChildren);
    }

    mScriptExec = ScriptExecutable::createFromSharedObject(
            mCtx->getContext(), mScriptSO);

    if (mScriptExec == nullptr) {
        return false;
    }

    size_t varCount = mScriptExec->getExportedVariableCount();
    if (varCount > 0) {
        mBoundAllocs = new Allocation *[varCount];
        memset(mBoundAllocs, 0, varCount * sizeof(*mBoundAllocs));
    }

    mIsThreadable = mScriptExec->getThreadable();
    //ALOGE("Script isThreadable? %d", mIsThreadable);

    return true;
}

bool RsdCpuScriptImpl::init(char const *resName, char const *cacheDir,
                            uint8_t const *bitcode, size_t bitcodeSize,
                            uint32_t flags, char const *bccPluginName) {
    //ALOGE("rsdScriptCreate %p %p %p %p %i %i %p", rsc, resName, cacheDir,
    // bitcode, bitcodeSize, flags, lookupFunc);
    //ALOGE("rsdScriptInit %p %p", rsc, script);

    mCtx->lockMutex();
#ifndef RS_COMPATIBILITY_LIB
    bool useRSDebugContext = false;

    mCompilerDriver = nullptr;

    mCompilerDriver = new bcc::RSCompilerDriver();
    if (mCompilerDriver == nullptr) {
        ALOGE("bcc: FAILS to create compiler driver (out of memory)");
        mCtx->unlockMutex();
        return false;
    }

    // Run any compiler setup functions we have been provided with.
    RSSetupCompilerCallback setupCompilerCallback =
            mCtx->getSetupCompilerCallback();
    if (setupCompilerCallback != nullptr) {
        setupCompilerCallback(mCompilerDriver);
    }

    bcinfo::MetadataExtractor bitcodeMetadata((const char *) bitcode, bitcodeSize);
    if (!bitcodeMetadata.extract()) {
        ALOGE("Could not extract metadata from bitcode");
        mCtx->unlockMutex();
        return false;
    }

    const char* core_lib = findCoreLib(bitcodeMetadata, (const char*)bitcode, bitcodeSize);

    if (mCtx->getContext()->getContextType() == RS_CONTEXT_TYPE_DEBUG) {
        mCompilerDriver->setDebugContext(true);
        useRSDebugContext = true;
    }

    std::string bcFileName(cacheDir);
    bcFileName.append("/");
    bcFileName.append(resName);
    bcFileName.append(".bc");

    std::vector<const char*> compileArguments;
    setCompileArguments(&compileArguments, bcFileName, cacheDir, resName, core_lib,
                        useRSDebugContext, bccPluginName);
    // The last argument of compileArguments ia a nullptr, so remove 1 from the size.
    std::unique_ptr<const char> joined(
        rsuJoinStrings(compileArguments.size() - 1, compileArguments.data()));
    std::string compileCommandLine (joined.get());

    if (!is_force_recompile() && !useRSDebugContext) {
        mScriptSO = SharedLibraryUtils::loadSharedLibrary(cacheDir, resName);
    }

    // If we can't, it's either not there or out of date.  We compile the bit code and try loading
    // again.
    if (mScriptSO == nullptr) {
        if (!compileBitcode(bcFileName, (const char*)bitcode, bitcodeSize,
                            compileArguments.data(), compileCommandLine))
        {
            ALOGE("bcc: FAILS to compile '%s'", resName);
            mCtx->unlockMutex();
            return false;
        }

        if (!SharedLibraryUtils::createSharedLibrary(cacheDir, resName)) {
            ALOGE("Linker: Failed to link object file '%s'", resName);
            mCtx->unlockMutex();
            return false;
        }

        mScriptSO = SharedLibraryUtils::loadSharedLibrary(cacheDir, resName);
        if (mScriptSO == nullptr) {
            ALOGE("Unable to load '%s'", resName);
            mCtx->unlockMutex();
            return false;
        }
    }

    mBitcodeFilePath.setTo(bcFileName.c_str());

    // Read RS symbol information from the .so.
    if ( !mScriptSO) {
        goto error;
    }

    if ( !storeRSInfoFromSO()) {
      goto error;
    }
#else  // RS_COMPATIBILITY_LIB is defined
    const char *nativeLibDir = mCtx->getContext()->getNativeLibDir();
    mScriptSO = SharedLibraryUtils::loadSharedLibrary(cacheDir, resName, nativeLibDir);

    if (!mScriptSO) {
        goto error;
    }

    if (!storeRSInfoFromSO()) {
        goto error;
    }
#endif
    mCtx->unlockMutex();
    return true;

error:

    mCtx->unlockMutex();
    if (mScriptSO) {
        dlclose(mScriptSO);
        mScriptSO = nullptr;
    }
    return false;
}

#ifndef RS_COMPATIBILITY_LIB

const char* RsdCpuScriptImpl::findCoreLib(const bcinfo::MetadataExtractor& ME, const char* bitcode,
                                          size_t bitcodeSize) {
    const char* defaultLib = SYSLIBPATH"/libclcore.bc";

    // If we're debugging, use the debug library.
    if (mCtx->getContext()->getContextType() == RS_CONTEXT_TYPE_DEBUG) {
        return SYSLIBPATH"/libclcore_debug.bc";
    }

    // If a callback has been registered to specify a library, use that.
    RSSelectRTCallback selectRTCallback = mCtx->getSelectRTCallback();
    if (selectRTCallback != nullptr) {
        return selectRTCallback((const char*)bitcode, bitcodeSize);
    }

    // Check for a platform specific library
#if defined(ARCH_ARM_HAVE_NEON) && !defined(DISABLE_CLCORE_NEON)
    enum bcinfo::RSFloatPrecision prec = ME.getRSFloatPrecision();
    if (prec == bcinfo::RS_FP_Relaxed) {
        // NEON-capable ARMv7a devices can use an accelerated math library
        // for all reduced precision scripts.
        // ARMv8 does not use NEON, as ASIMD can be used with all precision
        // levels.
        return SYSLIBPATH"/libclcore_neon.bc";
    } else {
        return defaultLib;
    }
#elif defined(__i386__) || defined(__x86_64__)
    // x86 devices will use an optimized library.
    return SYSLIBPATH"/libclcore_x86.bc";
#else
    return defaultLib;
#endif
}

#endif

void RsdCpuScriptImpl::populateScript(Script *script) {
    // Copy info over to runtime
    script->mHal.info.exportedFunctionCount = mScriptExec->getExportedFunctionCount();
    script->mHal.info.exportedVariableCount = mScriptExec->getExportedVariableCount();
    script->mHal.info.exportedPragmaCount = mScriptExec->getPragmaCount();;
    script->mHal.info.exportedPragmaKeyList = mScriptExec->getPragmaKeys();
    script->mHal.info.exportedPragmaValueList = mScriptExec->getPragmaValues();

    // Bug, need to stash in metadata
    if (mRootExpand) {
        script->mHal.info.root = mRootExpand;
    } else {
        script->mHal.info.root = mRoot;
    }
}


typedef void (*rs_t)(const void *, void *, const void *, uint32_t, uint32_t, uint32_t, uint32_t);

bool RsdCpuScriptImpl::forEachMtlsSetup(const Allocation ** ains,
                                        uint32_t inLen,
                                        Allocation * aout,
                                        const void * usr, uint32_t usrLen,
                                        const RsScriptCall *sc,
                                        MTLaunchStruct *mtls) {

    memset(mtls, 0, sizeof(MTLaunchStruct));

    for (int index = inLen; --index >= 0;) {
        const Allocation* ain = ains[index];

        // possible for this to occur if IO_OUTPUT/IO_INPUT with no bound surface
        if (ain != nullptr &&
            (const uint8_t *)ain->mHal.drvState.lod[0].mallocPtr == nullptr) {

            mCtx->getContext()->setError(RS_ERROR_BAD_SCRIPT,
                                         "rsForEach called with null in allocations");
            return false;
        }
    }

    if (aout &&
        (const uint8_t *)aout->mHal.drvState.lod[0].mallocPtr == nullptr) {

        mCtx->getContext()->setError(RS_ERROR_BAD_SCRIPT,
                                     "rsForEach called with null out allocations");
        return false;
    }

    if (inLen > 0) {
        const Allocation *ain0   = ains[0];
        const Type       *inType = ain0->getType();

        mtls->fep.dim.x = inType->getDimX();
        mtls->fep.dim.y = inType->getDimY();
        mtls->fep.dim.z = inType->getDimZ();

        for (int Index = inLen; --Index >= 1;) {
            if (!ain0->hasSameDims(ains[Index])) {
                mCtx->getContext()->setError(RS_ERROR_BAD_SCRIPT,
                  "Failed to launch kernel; dimensions of input and output"
                  "allocations do not match.");

                return false;
            }
        }

    } else if (aout != nullptr) {
        const Type *outType = aout->getType();

        mtls->fep.dim.x = outType->getDimX();
        mtls->fep.dim.y = outType->getDimY();
        mtls->fep.dim.z = outType->getDimZ();

    } else {
        mCtx->getContext()->setError(RS_ERROR_BAD_SCRIPT,
                                     "rsForEach called with null allocations");
        return false;
    }

    if (inLen > 0 && aout != nullptr) {
        if (!ains[0]->hasSameDims(aout)) {
            mCtx->getContext()->setError(RS_ERROR_BAD_SCRIPT,
              "Failed to launch kernel; dimensions of input and output allocations do not match.");

            return false;
        }
    }

    if (!sc || (sc->xEnd == 0)) {
        mtls->end.x = mtls->fep.dim.x;
    } else {
        mtls->start.x = rsMin(mtls->fep.dim.x, sc->xStart);
        mtls->end.x = rsMin(mtls->fep.dim.x, sc->xEnd);
        if (mtls->start.x >= mtls->end.x) return false;
    }

    if (!sc || (sc->yEnd == 0)) {
        mtls->end.y = mtls->fep.dim.y;
    } else {
        mtls->start.y = rsMin(mtls->fep.dim.y, sc->yStart);
        mtls->end.y = rsMin(mtls->fep.dim.y, sc->yEnd);
        if (mtls->start.y >= mtls->end.y) return false;
    }

    if (!sc || (sc->zEnd == 0)) {
        mtls->end.z = mtls->fep.dim.z;
    } else {
        mtls->start.z = rsMin(mtls->fep.dim.z, sc->zStart);
        mtls->end.z = rsMin(mtls->fep.dim.z, sc->zEnd);
        if (mtls->start.z >= mtls->end.z) return false;
    }

    if (!sc || (sc->arrayEnd == 0)) {
        mtls->end.array[0] = mtls->fep.dim.array[0];
    } else {
        mtls->start.array[0] = rsMin(mtls->fep.dim.array[0], sc->arrayStart);
        mtls->end.array[0] = rsMin(mtls->fep.dim.array[0], sc->arrayEnd);
        if (mtls->start.array[0] >= mtls->end.array[0]) return false;
    }

    if (!sc || (sc->array2End == 0)) {
        mtls->end.array[1] = mtls->fep.dim.array[1];
    } else {
        mtls->start.array[1] = rsMin(mtls->fep.dim.array[1], sc->array2Start);
        mtls->end.array[1] = rsMin(mtls->fep.dim.array[1], sc->array2End);
        if (mtls->start.array[1] >= mtls->end.array[1]) return false;
    }

    if (!sc || (sc->array3End == 0)) {
        mtls->end.array[2] = mtls->fep.dim.array[2];
    } else {
        mtls->start.array[2] = rsMin(mtls->fep.dim.array[2], sc->array3Start);
        mtls->end.array[2] = rsMin(mtls->fep.dim.array[2], sc->array3End);
        if (mtls->start.array[2] >= mtls->end.array[2]) return false;
    }

    if (!sc || (sc->array4End == 0)) {
        mtls->end.array[3] = mtls->fep.dim.array[3];
    } else {
        mtls->start.array[3] = rsMin(mtls->fep.dim.array[3], sc->array4Start);
        mtls->end.array[3] = rsMin(mtls->fep.dim.array[3], sc->array4End);
        if (mtls->start.array[3] >= mtls->end.array[3]) return false;
    }


    // The X & Y walkers always want 0-1 min even if dim is not present
    mtls->end.x    = rsMax((uint32_t)1, mtls->end.x);
    mtls->end.y    = rsMax((uint32_t)1, mtls->end.y);

    mtls->rsc        = mCtx;
    if (ains) {
        memcpy(mtls->ains, ains, inLen * sizeof(ains[0]));
    }
    mtls->aout[0]    = aout;
    mtls->fep.usr    = usr;
    mtls->fep.usrLen = usrLen;
    mtls->mSliceSize = 1;
    mtls->mSliceNum  = 0;

    mtls->isThreadable  = mIsThreadable;

    if (inLen > 0) {
        mtls->fep.inLen = inLen;
        for (int index = inLen; --index >= 0;) {
            mtls->fep.inPtr[index] = (const uint8_t*)ains[index]->mHal.drvState.lod[0].mallocPtr;
            mtls->fep.inStride[index] = ains[index]->getType()->getElementSizeBytes();
        }
    }

    if (aout != nullptr) {
        mtls->fep.outPtr[0] = (uint8_t *)aout->mHal.drvState.lod[0].mallocPtr;
        mtls->fep.outStride[0] = aout->getType()->getElementSizeBytes();
    }

    // All validation passed, ok to launch threads
    return true;
}


void RsdCpuScriptImpl::invokeForEach(uint32_t slot,
                                     const Allocation ** ains,
                                     uint32_t inLen,
                                     Allocation * aout,
                                     const void * usr,
                                     uint32_t usrLen,
                                     const RsScriptCall *sc) {

    MTLaunchStruct mtls;

    if (forEachMtlsSetup(ains, inLen, aout, usr, usrLen, sc, &mtls)) {
        forEachKernelSetup(slot, &mtls);

        RsdCpuScriptImpl * oldTLS = mCtx->setTLS(this);
        mCtx->launchThreads(ains, inLen, aout, sc, &mtls);
        mCtx->setTLS(oldTLS);
    }
}

void RsdCpuScriptImpl::forEachKernelSetup(uint32_t slot, MTLaunchStruct *mtls) {
    mtls->script = this;
    mtls->fep.slot = slot;
    mtls->kernel = mScriptExec->getForEachFunction(slot);
    rsAssert(mtls->kernel != nullptr);
    mtls->sig = mScriptExec->getForEachSignature(slot);
}

int RsdCpuScriptImpl::invokeRoot() {
    RsdCpuScriptImpl * oldTLS = mCtx->setTLS(this);
    int ret = mRoot();
    mCtx->setTLS(oldTLS);
    return ret;
}

void RsdCpuScriptImpl::invokeInit() {
    if (mInit) {
        mInit();
    }
}

void RsdCpuScriptImpl::invokeFreeChildren() {
    if (mFreeChildren) {
        mFreeChildren();
    }
}

void RsdCpuScriptImpl::invokeFunction(uint32_t slot, const void *params,
                                      size_t paramLength) {
    //ALOGE("invoke %i %p %zu", slot, params, paramLength);
    void * ap = nullptr;

#if defined(__x86_64__)
    // The invoked function could have input parameter of vector type for example float4 which
    // requires void* params to be 16 bytes aligned when using SSE instructions for x86_64 platform.
    // So try to align void* params before passing them into RS exported function.

    if ((uint8_t)(uint64_t)params & 0x0F) {
        if ((ap = (void*)memalign(16, paramLength)) != nullptr) {
            memcpy(ap, params, paramLength);
        } else {
            ALOGE("x86_64: invokeFunction memalign error, still use params which"
                  " is not 16 bytes aligned.");
        }
    }
#endif

    RsdCpuScriptImpl * oldTLS = mCtx->setTLS(this);
    reinterpret_cast<void (*)(const void *, uint32_t)>(
        mScriptExec->getInvokeFunction(slot))(ap? (const void *) ap: params, paramLength);

    mCtx->setTLS(oldTLS);
}

void RsdCpuScriptImpl::setGlobalVar(uint32_t slot, const void *data, size_t dataLength) {
    //rsAssert(!script->mFieldIsObject[slot]);
    //ALOGE("setGlobalVar %i %p %zu", slot, data, dataLength);

    //if (mIntrinsicID) {
        //mIntrinsicFuncs.setVar(dc, script, drv->mIntrinsicData, slot, data, dataLength);
        //return;
    //}

    int32_t *destPtr = reinterpret_cast<int32_t *>(mScriptExec->getFieldAddress(slot));
    if (!destPtr) {
        //ALOGV("Calling setVar on slot = %i which is null", slot);
        return;
    }

    memcpy(destPtr, data, dataLength);
}

void RsdCpuScriptImpl::getGlobalVar(uint32_t slot, void *data, size_t dataLength) {
    //rsAssert(!script->mFieldIsObject[slot]);
    //ALOGE("getGlobalVar %i %p %zu", slot, data, dataLength);

    int32_t *srcPtr = reinterpret_cast<int32_t *>(mScriptExec->getFieldAddress(slot));
    if (!srcPtr) {
        //ALOGV("Calling setVar on slot = %i which is null", slot);
        return;
    }
    memcpy(data, srcPtr, dataLength);
}


void RsdCpuScriptImpl::setGlobalVarWithElemDims(uint32_t slot, const void *data, size_t dataLength,
                                                const Element *elem,
                                                const uint32_t *dims, size_t dimLength) {
    int32_t *destPtr = reinterpret_cast<int32_t *>(mScriptExec->getFieldAddress(slot));
    if (!destPtr) {
        //ALOGV("Calling setVar on slot = %i which is null", slot);
        return;
    }

    // We want to look at dimension in terms of integer components,
    // but dimLength is given in terms of bytes.
    dimLength /= sizeof(int);

    // Only a single dimension is currently supported.
    rsAssert(dimLength == 1);
    if (dimLength == 1) {
        // First do the increment loop.
        size_t stride = elem->getSizeBytes();
        const char *cVal = reinterpret_cast<const char *>(data);
        for (uint32_t i = 0; i < dims[0]; i++) {
            elem->incRefs(cVal);
            cVal += stride;
        }

        // Decrement loop comes after (to prevent race conditions).
        char *oldVal = reinterpret_cast<char *>(destPtr);
        for (uint32_t i = 0; i < dims[0]; i++) {
            elem->decRefs(oldVal);
            oldVal += stride;
        }
    }

    memcpy(destPtr, data, dataLength);
}

void RsdCpuScriptImpl::setGlobalBind(uint32_t slot, Allocation *data) {

    //rsAssert(!script->mFieldIsObject[slot]);
    //ALOGE("setGlobalBind %i %p", slot, data);

    int32_t *destPtr = reinterpret_cast<int32_t *>(mScriptExec->getFieldAddress(slot));
    if (!destPtr) {
        //ALOGV("Calling setVar on slot = %i which is null", slot);
        return;
    }

    void *ptr = nullptr;
    mBoundAllocs[slot] = data;
    if (data) {
        ptr = data->mHal.drvState.lod[0].mallocPtr;
    }
    memcpy(destPtr, &ptr, sizeof(void *));
}

void RsdCpuScriptImpl::setGlobalObj(uint32_t slot, ObjectBase *data) {

    //rsAssert(script->mFieldIsObject[slot]);
    //ALOGE("setGlobalObj %i %p", slot, data);

    int32_t *destPtr = reinterpret_cast<int32_t *>(mScriptExec->getFieldAddress(slot));
    if (!destPtr) {
        //ALOGV("Calling setVar on slot = %i which is null", slot);
        return;
    }

    rsrSetObject(mCtx->getContext(), (rs_object_base *)destPtr, data);
}

RsdCpuScriptImpl::~RsdCpuScriptImpl() {
#ifndef RS_COMPATIBILITY_LIB
    if (mCompilerDriver) {
        delete mCompilerDriver;
    }
#endif

    if (mScriptExec != nullptr) {
        delete mScriptExec;
    }
    if (mBoundAllocs) delete[] mBoundAllocs;
    if (mScriptSO) {
        dlclose(mScriptSO);
    }
}

Allocation * RsdCpuScriptImpl::getAllocationForPointer(const void *ptr) const {
    if (!ptr) {
        return nullptr;
    }

    for (uint32_t ct=0; ct < mScript->mHal.info.exportedVariableCount; ct++) {
        Allocation *a = mBoundAllocs[ct];
        if (!a) continue;
        if (a->mHal.drvState.lod[0].mallocPtr == ptr) {
            return a;
        }
    }
    ALOGE("rsGetAllocation, failed to find %p", ptr);
    return nullptr;
}

void RsdCpuScriptImpl::preLaunch(uint32_t slot, const Allocation ** ains,
                                 uint32_t inLen, Allocation * aout,
                                 const void * usr, uint32_t usrLen,
                                 const RsScriptCall *sc) {}

void RsdCpuScriptImpl::postLaunch(uint32_t slot, const Allocation ** ains,
                                  uint32_t inLen, Allocation * aout,
                                  const void * usr, uint32_t usrLen,
                                  const RsScriptCall *sc) {}


}
}
