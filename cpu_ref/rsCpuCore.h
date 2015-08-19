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

namespace android {
namespace renderscript {

// Whether the CPU we're running on supports SIMD instructions
extern bool gArchUseSIMD;

// Function types found in RenderScript code
typedef void (*ReduceFunc_t)(const uint8_t *inBuf, uint8_t *outBuf, uint32_t len);
typedef void (*ForEachFunc_t)(const RsExpandKernelDriverInfo *info, uint32_t x1, uint32_t x2, uint32_t outStride);
typedef void (*InvokeFunc_t)(void *params);
typedef void (*InitOrDtorFunc_t)(void);
typedef int  (*RootFunc_t)(void);

// Internal driver callback used to execute a kernel
typedef void (*WorkerCallback_t)(void *usr, uint32_t idx);

class RsdCpuScriptImpl;
class RsdCpuReferenceImpl;

struct ScriptTLSStruct {
    android::renderscript::Context * mContext;
    const android::renderscript::Script * mScript;
    RsdCpuScriptImpl *mImpl;
};

// MTLaunchStruct passes information about a multithreaded kernel launch.
struct MTLaunchStructCommon {
    RsdCpuReferenceImpl *rs;
    RsdCpuScriptImpl *script;

    uint32_t mSliceSize;
    volatile int mSliceNum;
    bool isThreadable;

    // Boundary information about the launch
    RsLaunchDimensions start;
    RsLaunchDimensions end;
    // Points to MTLaunchStructForEach::fep::dim or
    // MTLaunchStructReduce::inputDim.
    RsLaunchDimensions *dimPtr;
};

struct MTLaunchStructForEach : public MTLaunchStructCommon {
    // Driver info structure
    RsExpandKernelDriverInfo fep;

    ForEachFunc_t kernel;
    uint32_t sig;
    const Allocation *ains[RS_KERNEL_INPUT_LIMIT];
    Allocation *aout[RS_KERNEL_INPUT_LIMIT];
};

struct MTLaunchStructReduce : public MTLaunchStructCommon {
    ReduceFunc_t kernel;
    const uint8_t *inBuf;
    uint8_t *outBuf;
    RsLaunchDimensions inputDim;
};

class RsdCpuReferenceImpl : public RsdCpuReference {
public:
    ~RsdCpuReferenceImpl() override;
    RsdCpuReferenceImpl(Context *);

    void lockMutex();
    void unlockMutex();

    bool init(uint32_t version_major, uint32_t version_minor, sym_lookup_t, script_lookup_t);
    void setPriority(int32_t priority) override;
    virtual void launchThreads(WorkerCallback_t cbk, void *data);
    static void * helperThreadProc(void *vrsc);
    RsdCpuScriptImpl * setTLS(RsdCpuScriptImpl *sc);

    Context * getContext() {return mRSC;}
    uint32_t getThreadCount() const {
        return mWorkers.mCount + 1;
    }

    // Launch foreach kernel
    void launchForEach(const Allocation **ains, uint32_t inLen, Allocation *aout,
                       const RsScriptCall *sc, MTLaunchStructForEach *mtls);

    // Launch a reduce kernel
    void launchReduce(const Allocation *ain, Allocation *aout,
                      MTLaunchStructReduce *mtls);

    CpuScript * createScript(const ScriptC *s, char const *resName, char const *cacheDir,
                             uint8_t const *bitcode, size_t bitcodeSize, uint32_t flags) override;
    CpuScript * createIntrinsic(const Script *s, RsScriptIntrinsicID iid, Element *e) override;
    void* createScriptGroup(const ScriptGroupBase *sg) override;

    const RsdCpuReference::CpuSymbol *symLookup(const char *);

    RsdCpuReference::CpuScript *lookupScript(const Script *s) {
        return mScriptLookupFn(mRSC, s);
    }

    void setSelectRTCallback(RSSelectRTCallback pSelectRTCallback) {
        mSelectRTCallback = pSelectRTCallback;
    }
    RSSelectRTCallback getSelectRTCallback() {
        return mSelectRTCallback;
    }

    virtual void setBccPluginName(const char *name) {
        mBccPluginName.setTo(name);
    }
    virtual const char *getBccPluginName() const {
        return mBccPluginName.string();
    }
    bool getInForEach() override { return mInForEach; }

    // Set to true if we should embed global variable information in the code.
    void setEmbedGlobalInfo(bool v) override {
        mEmbedGlobalInfo = v;
    }

    // Returns true if we should embed global variable information in the code.
    bool getEmbedGlobalInfo() const override {
        return mEmbedGlobalInfo;
    }

    // Set to true if we should skip constant (immutable) global variables when
    // potentially embedding information about globals.
    void setEmbedGlobalInfoSkipConstant(bool v) override {
        mEmbedGlobalInfoSkipConstant = v;
    }

    // Returns true if we should skip constant (immutable) global variables when
    // potentially embedding information about globals.
    bool getEmbedGlobalInfoSkipConstant() const override {
        return mEmbedGlobalInfoSkipConstant;
    }

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

    RSSelectRTCallback mSelectRTCallback;
    String8 mBccPluginName;

    // Specifies whether we should embed global variable information in the
    // code via special RS variables that can be examined later by the driver.
    // Defaults to true.
    bool mEmbedGlobalInfo;

    // Specifies whether we should skip constant (immutable) global variables
    // when potentially embedding information about globals.
    // Defaults to true.
    bool mEmbedGlobalInfoSkipConstant;
};


}
}

#endif
