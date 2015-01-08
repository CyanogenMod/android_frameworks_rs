#ifndef CPU_REF_CPUSCRIPTGROUP2IMPL_H_
#define CPU_REF_CPUSCRIPTGROUP2IMPL_H_

#include <list>

#include "rsd_cpu.h"

using std::list;

namespace android {
namespace renderscript {

class Closure;
class RsdCpuScriptImpl;
class RsdCpuReferenceImpl;
class ScriptGroup2;

struct RsExpandKernelParams;

typedef void (*ExpandFuncTy)(const RsExpandKernelParams*, uint32_t, uint32_t,
                             uint32_t);

class CPUClosure {
 public:
  CPUClosure(const Closure* closure, RsdCpuScriptImpl* si, ExpandFuncTy func,
             const void* usrPtr, const size_t usrSize) :
      mClosure(closure), mSi(si), mFunc(func), mUsrPtr(usrPtr),
      mUsrSize(usrSize) {}

  // It's important to do forwarding here than inheritance for unbound value
  // binding to work.
  const Closure* mClosure;
  RsdCpuScriptImpl* mSi;
  const ExpandFuncTy mFunc;
  const void* mUsrPtr;
  const size_t mUsrSize;
};

class CpuScriptGroup2Impl : public RsdCpuReference::CpuScriptGroup2 {
 public:
  CpuScriptGroup2Impl(RsdCpuReferenceImpl *cpuRefImpl, const ScriptGroupBase* group);
  virtual ~CpuScriptGroup2Impl();

  bool init();
  virtual void execute();

 private:
  void setGlobalsForBatch(const list<CPUClosure*>& batch);
  void runBatch(const list<CPUClosure*>& batch);

  RsdCpuReferenceImpl* mCpuRefImpl;
  const ScriptGroup2* mGroup;

  list<list<CPUClosure*>*> mBatches;
};

}  // namespace renderscript
}  // namespace android

#endif  // CPU_REF_CPUSCRIPTGROUP2IMPL_H_
