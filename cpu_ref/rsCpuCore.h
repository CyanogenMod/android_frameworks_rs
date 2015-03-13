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
#include "rsCppUtils.h"
#include "rsElement.h"
#include "rsScriptC.h"
#include "rsCpuCoreRuntime.h"


#define RS_KERNEL_INPUT_LIMIT 8

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

struct RsLaunchDimensions {
    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint32_t lod;
    uint32_t face;
    uint32_t array[4 /*make a define*/];
};

struct RsExpandKernelDriverInfo {
    // Warning: This structure is shared with the compiler
    // Any change to the fields here requires a matching compiler change

    const uint8_t *inPtr[RS_KERNEL_INPUT_LIMIT];
    uint32_t inStride[RS_KERNEL_INPUT_LIMIT];
    uint32_t inLen;

    uint8_t *outPtr[RS_KERNEL_INPUT_LIMIT];
    uint32_t outStride[RS_KERNEL_INPUT_LIMIT];
    uint32_t outLen;

    // Dimension of the launch
    RsLaunchDimensions dim;

    // The walking itterator of the launch
    RsLaunchDimensions current;

    const void *usr;
    uint32_t usrLen;



    // Items below this line are not used by the compiler and can be change in the driver
    uint32_t lid;
    uint32_t slot;

};

typedef ::RsExpandKernelParams RsExpandKernelParams;

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
    const Allocation * ains[RS_KERNEL_INPUT_LIMIT];
    Allocation * aout[RS_KERNEL_INPUT_LIMIT];

    uint32_t mSliceSize;
    volatile int mSliceNum;
    bool isThreadable;

    RsLaunchDimensions start;
    RsLaunchDimensions end;
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
    virtual void* createScriptGroup(const ScriptGroupBase *sg);

    const RsdCpuReference::CpuSymbol *symLookup(const char *);

    RsdCpuReference::CpuScript * lookupScript(const Script *s) {
        return mScriptLookupFn(mRSC, s);
    }

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
        mBccPluginName.setTo(name);
    }
    virtual const char *getBccPluginName() const {
        return mBccPluginName.string();
    }
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

    bcc::RSLinkRuntimeCallback mLinkRuntimeCallback;
    RSSelectRTCallback mSelectRTCallback;
    RSSetupCompilerCallback mSetupCompilerCallback;
    String8 mBccPluginName;
};


}
}

#endif
