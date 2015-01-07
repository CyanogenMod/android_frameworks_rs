#include "rsCpuScriptGroup2.h"

#include "cpu_ref/rsCpuCore.h"
#include "rsClosure.h"
#include "rsContext.h"
#include "rsCpuCore.h"
#include "rsCpuScript.h"
#include "rsScript.h"
#include "rsScriptGroup2.h"

namespace android {
namespace renderscript {

namespace {

static const size_t DefaultKernelArgCount = 2;

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

/*
  Returns true if closure depends on any closure in batch via a glboal variable
  TODO: this probably should go into class Closure.
 */
bool conflict(const list<CPUClosure*> &batch, CPUClosure* closure) {
  for (const auto &p : closure->mClosure->mGlobalDeps) {
    const Closure* dep = p.first;
    for (CPUClosure* c : batch) {
      if (c->mClosure == dep) {
        return true;
      }
    }
  }
  for (const auto &p : closure->mClosure->mArgDeps) {
    const Closure* dep = p.first;
    for (CPUClosure* c : batch) {
      if (c->mClosure == dep) {
        for (const auto &p1 : *p.second) {
          if (p1.second != nullptr) {
            return true;
          }
        }
      }
    }
  }
  return false;
}

}  // namespace

CpuScriptGroup2Impl::CpuScriptGroup2Impl(RsdCpuReferenceImpl *cpuRefImpl,
                                         const ScriptGroupBase *sg) :
    mCpuRefImpl(cpuRefImpl), mGroup((const ScriptGroup2*)(sg)) {
  list<CPUClosure*>* batch = new list<CPUClosure*>();
  for (Closure* closure: mGroup->mClosures) {
    const ScriptKernelID* kernelID = closure->mKernelID.get();
    RsdCpuScriptImpl* si =
        (RsdCpuScriptImpl *)mCpuRefImpl->lookupScript(kernelID->mScript);

    MTLaunchStruct mtls;
    si->forEachKernelSetup(kernelID->mSlot, &mtls);
    // TODO: Is mtls.fep.usrLen ever used?
    CPUClosure* cc = new CPUClosure(closure, si, (ExpandFuncTy)mtls.kernel,
                                    mtls.fep.usr, mtls.fep.usrLen);
    if (conflict(*batch, cc)) {
      mBatches.push_back(batch);
      batch = new list<CPUClosure*>();
    }
    batch->push_back(cc);
  }
  mBatches.push_back(batch);
}

CpuScriptGroup2Impl::~CpuScriptGroup2Impl() {
  for (list<CPUClosure*>* batch : mBatches) {
    for (CPUClosure* c : *batch) {
      delete c;
    }
  }
}

void CpuScriptGroup2Impl::execute() {
  for (list<CPUClosure*>* batch : mBatches) {
    setGlobalsForBatch(*batch);
    runBatch(*batch);
  }
}

void CpuScriptGroup2Impl::setGlobalsForBatch(const list<CPUClosure*>& batch) {
  for (CPUClosure* cpuClosure : batch) {
    const Closure* closure = cpuClosure->mClosure;
    const ScriptKernelID* kernelID = closure->mKernelID.get();
    Script* s = kernelID->mScript;
    for (const auto& p : closure->mGlobals) {
      const void* value = p.second.first;
      int size = p.second.second;
      // We use -1 size to indicate an ObjectBase rather than a primitive type
      if (size < 0) {
        s->setVarObj(p.first->mSlot, (ObjectBase*)value);
      } else {
        s->setVar(p.first->mSlot, (const void*)&value, size);
      }
    }
  }
}

void CpuScriptGroup2Impl::runBatch(const list<CPUClosure*>& batch) {
  for (CPUClosure* cpuClosure : batch) {
    const Closure* closure = cpuClosure->mClosure;
    const ScriptKernelID* kernelID = closure->mKernelID.get();
    cpuClosure->mSi->preLaunch(kernelID->mSlot,
                               (const Allocation**)&closure->mArgs[0],
                               closure->mArgs.size(), closure->mReturnValue,
                               cpuClosure->mUsrPtr, cpuClosure->mUsrSize,
                               nullptr);
  }

  const CPUClosure* cpuClosure = batch.front();
  const Closure* closure = cpuClosure->mClosure;
  MTLaunchStruct mtls;

  cpuClosure->mSi->forEachMtlsSetup((const Allocation**)&closure->mArgs[0],
                                    closure->mArgs.size(),
                                    closure->mReturnValue,
                                    nullptr, 0, nullptr, &mtls);

  mtls.script = nullptr;
  mtls.kernel = (void (*)())&groupRoot;
  mtls.fep.usr = &batch;

  mCpuRefImpl->launchThreads(nullptr, 0, nullptr, nullptr, &mtls);

  for (CPUClosure* cpuClosure : batch) {
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
