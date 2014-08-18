/*
 * Copyright (C) 2012 The Android Open Source Project
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

#ifndef RSD_CPU_CORE_H
#define RSD_CPU_CORE_H

#include "rsd_cpu.h"
#include "rsSignal.h"
#include "rsContext.h"
#include "rsElement.h"
#include "rsScriptC.h"

#include <string>

#define RS_KERNEL_INPUT_THRESHOLD 32

namespace bcc {
    class BCCContext;
    class RSCompilerDriver;
    class RSExecutable;
}

namespace android {
namespace renderscript {

struct StridePair {
  uint32_t eStride;
  uint32_t yStride;
};

struct RsExpandKernelDriverInfo {
    const uint8_t **inPtrs;
    uint32_t inLen;

    uint8_t *outPtr;

    StridePair *inStrides;
    StridePair  outStride;

    uint32_t dimX;
    uint32_t dimY;
    uint32_t dimZ;

    uint32_t slot;

    const void *usr;
    uint32_t usrLen;

    bool heapAllocatedArrays;

    RsExpandKernelDriverInfo() : heapAllocatedArrays(false) {}

    ~RsExpandKernelDriverInfo() {
        if (heapAllocatedArrays) {
            if (inPtrs != NULL) {
                delete[] inPtrs;
            }

            if (inStrides != NULL) {
                delete[] inStrides;
            }
        }
    }
};

struct RsExpandKernelParams {

    // Used by kernels
    const void **ins;
    uint32_t *inEStrides;
    void *out;
    uint32_t y;
    uint32_t z;
    uint32_t lid;

    // Used by ScriptGroup and user kernels.
    const void *usr;

    // Used by intrinsics
    uint32_t dimX;
    uint32_t dimY;
    uint32_t dimZ;

    /*
     * FIXME: This is only used by the blend intrinsic.  If possible, we should
     *        modify blur to not need it.
     */
    uint32_t slot;

    /// Copy fields needed by a kernel from a driver struct.
    void takeFields(const RsExpandKernelDriverInfo &dstruct) {
        this->usr  = dstruct.usr;
        this->slot = dstruct.slot;

        this->dimX = dstruct.dimX;
        this->dimY = dstruct.dimY;
        this->dimZ = dstruct.dimZ;
    }
};

extern bool gArchUseSIMD;

typedef void (* InvokeFunc_t)(void);
typedef void (* ForEachFunc_t)(void);
typedef void (*WorkerCallback_t)(void *usr, uint32_t idx);

class RsdCpuScriptImpl;
class RsdCpuReferenceImpl;

struct ScriptTLSStruct {
    android::renderscript::Context * mContext;
    const android::renderscript::Script * mScript;
    RsdCpuScriptImpl *mImpl;
};

struct MTLaunchStruct {
    RsExpandKernelDriverInfo fep;

    RsdCpuReferenceImpl *rsc;
    RsdCpuScriptImpl *script;

    ForEachFunc_t kernel;
    uint32_t sig;
    const Allocation ** ains;
    Allocation * aout;

    uint32_t mSliceSize;
    volatile int mSliceNum;
    bool isThreadable;

    uint32_t xStart;
    uint32_t xEnd;
    uint32_t yStart;
    uint32_t yEnd;
    uint32_t zStart;
    uint32_t zEnd;
    uint32_t arrayStart;
    uint32_t arrayEnd;

    const uint8_t *inPtrsBuff[RS_KERNEL_INPUT_THRESHOLD];
    StridePair     inStridesBuff[RS_KERNEL_INPUT_THRESHOLD];
};

class RsdCpuReferenceImpl : public RsdCpuReference {
public:
    virtual ~RsdCpuReferenceImpl();
    RsdCpuReferenceImpl(Context *);

    void lockMutex();
    void unlockMutex();

    bool init(uint32_t version_major, uint32_t version_minor, sym_lookup_t, script_lookup_t);
    virtual void setPriority(int32_t priority);
    virtual void launchThreads(WorkerCallback_t cbk, void *data);
    static void * helperThreadProc(void *vrsc);
    RsdCpuScriptImpl * setTLS(RsdCpuScriptImpl *sc);

    Context * getContext() {return mRSC;}
    uint32_t getThreadCount() const {
        return mWorkers.mCount + 1;
    }

    void launchThreads(const Allocation** ains, uint32_t inLen, Allocation* aout,
                       const RsScriptCall* sc, MTLaunchStruct* mtls);

    virtual CpuScript * createScript(const ScriptC *s,
                                     char const *resName, char const *cacheDir,
                                     uint8_t const *bitcode, size_t bitcodeSize,
                                     uint32_t flags);
    virtual CpuScript * createIntrinsic(const Script *s,
                                        RsScriptIntrinsicID iid, Element *e);
    virtual CpuScriptGroup * createScriptGroup(const ScriptGroup *sg);

    const RsdCpuReference::CpuSymbol *symLookup(const char *);

    RsdCpuReference::CpuScript * lookupScript(const Script *s) {
        return mScriptLookupFn(mRSC, s);
    }

#ifndef RS_COMPATIBILITY_LIB
    void setLinkRuntimeCallback(
            bcc::RSLinkRuntimeCallback pLinkRuntimeCallback) {
        mLinkRuntimeCallback = pLinkRuntimeCallback;
    }
    bcc::RSLinkRuntimeCallback getLinkRuntimeCallback() {
        return mLinkRuntimeCallback;
    }

    void setSelectRTCallback(RSSelectRTCallback pSelectRTCallback) {
        mSelectRTCallback = pSelectRTCallback;
    }
    RSSelectRTCallback getSelectRTCallback() {
        return mSelectRTCallback;
    }

    virtual void setSetupCompilerCallback(
            RSSetupCompilerCallback pSetupCompilerCallback) {
        mSetupCompilerCallback = pSetupCompilerCallback;
    }
    virtual RSSetupCompilerCallback getSetupCompilerCallback() const {
        return mSetupCompilerCallback;
    }

    virtual void setBccPluginName(const char *name) {
        mBccPluginName.assign(name);
    }
    virtual const char *getBccPluginName() const {
        return mBccPluginName.c_str();
    }
#endif
    virtual bool getInForEach() { return mInForEach; }

protected:
    Context *mRSC;
    uint32_t version_major;
    uint32_t version_minor;
    //bool mHasGraphics;
    bool mInForEach;

    struct Workers {
        volatile int mRunningCount;
        volatile int mLaunchCount;
        uint32_t mCount;
        pthread_t *mThreadId;
        pid_t *mNativeThreadId;
        Signal mCompleteSignal;
        Signal *mLaunchSignals;
        WorkerCallback_t mLaunchCallback;
        void *mLaunchData;
    };
    Workers mWorkers;
    bool mExit;
    sym_lookup_t mSymLookupFn;
    script_lookup_t mScriptLookupFn;

    ScriptTLSStruct mTlsStruct;

#ifndef RS_COMPATIBILITY_LIB
    bcc::RSLinkRuntimeCallback mLinkRuntimeCallback;
    RSSelectRTCallback mSelectRTCallback;
    RSSetupCompilerCallback mSetupCompilerCallback;
    std::string mBccPluginName;
#endif
};


}
}

#endif
