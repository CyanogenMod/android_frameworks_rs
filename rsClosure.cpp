#include "rsClosure.h"

#include "cpu_ref/rsCpuCore.h"
#include "rsContext.h" // XXX: necessary to avoid compiler error on rsScript.h below
#include "rsScript.h"
#include "rsType.h"

namespace android {
namespace renderscript {

RsClosure rsi_ClosureCreate(Context* context, RsScriptKernelID kernelID,
                            RsAllocation returnValue,
                            RsScriptFieldID* fieldIDs, size_t fieldIDs_length,
                            uintptr_t* values, size_t values_length,
                            size_t* sizes, size_t sizes_length,
                            RsClosure* depClosures, size_t depClosures_length,
                            RsScriptFieldID* depFieldIDs,
                            size_t depFieldIDs_length) {
  rsAssert(fieldIDs_length == values_length && values_length == sizes_length &&
           sizes_length == depClosures_length &&
           depClosures_length == depFieldIDs_length);

  return (RsClosure)(new Closure(
      context, (const ScriptKernelID*)kernelID, (Allocation*)returnValue,
      fieldIDs_length, (const ScriptFieldID**)fieldIDs, (const void**)values,
      sizes, (const Closure**)depClosures,
      (const ScriptFieldID**)depFieldIDs));
}

RsClosure rsi_InvokeClosureCreate(Context* context, RsScriptInvokeID invokeID,
                                  const void* params, const size_t paramLength,
                                  const RsScriptFieldID* fieldIDs, const size_t fieldIDs_length,
                                  const uintptr_t* values, const size_t values_length,
                                  const size_t* sizes, const size_t sizes_length) {
    rsAssert(fieldIDs_length == values_length && values_length == sizes_length);
    return (RsClosure)(new Closure(
        context, (const ScriptInvokeID*)invokeID, params, paramLength,
        fieldIDs_length, (const ScriptFieldID**)fieldIDs, (const void**)values,
        sizes));
}

void rsi_ClosureEval(Context* rsc, RsClosure closure) {
    ((Closure*)closure)->eval();
}

void rsi_ClosureSetArg(Context* rsc, RsClosure closure, uint32_t index,
                       uintptr_t value, size_t size) {
    ((Closure*)closure)->setArg(index, (const void*)value, size);
}

void rsi_ClosureSetGlobal(Context* rsc, RsClosure closure,
                          RsScriptFieldID fieldID, uintptr_t value,
                          size_t size) {
    ((Closure*)closure)->setGlobal((const ScriptFieldID*)fieldID,
                                   (const void*)value, size);
}

Closure::Closure(Context* context,
                 const ScriptKernelID* kernelID,
                 Allocation* returnValue,
                 const int numValues,
                 const ScriptFieldID** fieldIDs,
                 const void** values,
                 const size_t* sizes,
                 const Closure** depClosures,
                 const ScriptFieldID** depFieldIDs) :
    ObjectBase(context), mContext(context), mKernelID((ScriptKernelID*)kernelID),
    mInvokeID(nullptr), mReturnValue(returnValue), mParams(nullptr),
    mParamLength(0) {
  size_t i;

  for (i = 0; i < (size_t)numValues && fieldIDs[i] == nullptr; i++);

  vector<const void*> args(values, values + i);
  mArgs.swap(args);

  for (; i < (size_t)numValues; i++) {
    mGlobals[fieldIDs[i]] = std::make_pair(values[i], sizes[i]);
  }

  mDependences.insert(depClosures, depClosures + numValues);

  for (i = 0; i < mArgs.size(); i++) {
    const Closure* dep = depClosures[i];
    if (dep != nullptr) {
      auto mapping = mArgDeps[dep];
      if (mapping == nullptr) {
        mapping = new map<int, const ObjectBaseRef<ScriptFieldID>*>();
        mArgDeps[dep] = mapping;
      }
      (*mapping)[i] = new ObjectBaseRef<ScriptFieldID>(
          const_cast<ScriptFieldID*>(depFieldIDs[i]));
    }
  }

  for (; i < (size_t)numValues; i++) {
    const Closure* dep = depClosures[i];
    if (dep != nullptr) {
      auto mapping = mGlobalDeps[dep];
      if (mapping == nullptr) {
        mapping = new map<const ObjectBaseRef<ScriptFieldID>*,
            const ObjectBaseRef<ScriptFieldID>*>();
        mGlobalDeps[dep] = mapping;
      }
      (*mapping)[new ObjectBaseRef<ScriptFieldID>(
          const_cast<ScriptFieldID*>(fieldIDs[i]))] =
          new ObjectBaseRef<ScriptFieldID>(
              const_cast<ScriptFieldID*>(depFieldIDs[i]));
    }
  }
}

Closure::Closure(Context* context, const ScriptInvokeID* invokeID,
                 const void* params, const size_t paramLength,
                 const size_t numValues, const ScriptFieldID** fieldIDs,
                 const void** values, const size_t* sizes) :
    ObjectBase(context), mContext(context), mKernelID(nullptr), mInvokeID(invokeID),
    mReturnValue(nullptr), mParams(params), mParamLength(paramLength) {
    for (size_t i = 0; i < numValues; i++) {
        mGlobals[fieldIDs[i]] = std::make_pair(values[i], sizes[i]);
    }
}

Closure::~Closure() {
    for (const auto& p : mArgDeps) {
        auto map = p.second;
        for (const auto& p1 : *map) {
            delete p1.second;
        }
        delete p.second;
    }

    for (const auto& p : mGlobalDeps) {
        auto map = p.second;
        for (const auto& p1 : *map) {
            delete p1.first;
            delete p1.second;
        }
        delete p.second;
    }
}

void Closure::eval() {
    Script *s = mKernelID->mScript;

    for (const auto& p : mGlobals) {
        const void* value = p.second.first;
        int size = p.second.second;
        // We use -1 size to indicate an ObjectBase rather than a primitive type
        if (size < 0) {
            s->setVarObj(p.first->mSlot, (ObjectBase*)value);
        } else {
            s->setVar(p.first->mSlot, (const void*)&value, size);
        }
    }

    s->runForEach(mContext, mKernelID->mSlot, (const Allocation **)(&mArgs[0]),
                  mArgs.size(), mReturnValue, nullptr, 0, nullptr);
}

void Closure::setArg(const uint32_t index, const void* value, const size_t size) {
    mArgs[index] = value;
}

void Closure::setGlobal(const ScriptFieldID* fieldID, const void* value,
                        const size_t size) {
    mGlobals[fieldID] = std::make_pair(value, size);
}

}  // namespace renderscript
}  // namespace android
