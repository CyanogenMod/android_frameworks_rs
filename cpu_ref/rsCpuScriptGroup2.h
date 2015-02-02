#ifndef CPU_REF_CPUSCRIPTGROUP2IMPL_H_
#define CPU_REF_CPUSCRIPTGROUP2IMPL_H_

#include "rsd_cpu.h"
#include "rsList.h"

namespace android {
namespace renderscript {

class Closure;
class RsdCpuScriptImpl;
class RsdCpuReferenceImpl;
class ScriptExecutable;
class ScriptGroup2;

struct RsExpandKernelParams;

typedef void (*ExpandFuncTy)(const RsExpandKernelParams*, uint32_t, uint32_t,
                             uint32_t);
typedef void (*InvokeFuncTy)(const void*, uint32_t);

class CPUClosure {
public:
    CPUClosure(const Closure* closure, RsdCpuScriptImpl* si, ExpandFuncTy func,
               const void* usrPtr, const size_t usrSize) :
        mClosure(closure), mSi(si), mFunc(func),
        mUsrPtr(usrPtr), mUsrSize(usrSize) {}

    CPUClosure(const Closure* closure, RsdCpuScriptImpl* si) :
        mClosure(closure), mSi(si), mFunc(nullptr),
        mUsrPtr(nullptr), mUsrSize(0) {}

    // It's important to do forwarding here than inheritance for unbound value
    // binding to work.
    const Closure* mClosure;
    RsdCpuScriptImpl* mSi;
    const ExpandFuncTy mFunc;
    const void* mUsrPtr;
    const size_t mUsrSize;
};

class CpuScriptGroup2Impl;

class Batch {
public:
    Batch(CpuScriptGroup2Impl* group) : mGroup(group), mExecutable(nullptr) {}

    ~Batch();

    // Returns true if closure depends on any closure in this batch for a global
    // variable
    bool conflict(CPUClosure* closure) const;

    void tryToCreateFusedKernel(const char* cacheDir);
    void setGlobalsForBatch();
    void run();

    CpuScriptGroup2Impl* mGroup;
    ScriptExecutable* mExecutable;
    void* mScriptObj;
    List<CPUClosure*> mClosures;
};

class CpuScriptGroup2Impl : public RsdCpuReference::CpuScriptGroup2 {
public:
    CpuScriptGroup2Impl(RsdCpuReferenceImpl *cpuRefImpl, const ScriptGroupBase* group);
    virtual ~CpuScriptGroup2Impl();

    bool init();
    virtual void execute();

    RsdCpuReferenceImpl* getCpuRefImpl() const { return mCpuRefImpl; }

private:
    RsdCpuReferenceImpl* mCpuRefImpl;
    const ScriptGroup2* mGroup;
    List<Batch*> mBatches;
};

}  // namespace renderscript
}  // namespace android

#endif  // CPU_REF_CPUSCRIPTGROUP2IMPL_H_
