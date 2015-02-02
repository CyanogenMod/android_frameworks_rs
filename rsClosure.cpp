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

#if 0
void rsi_ClosureEval(Context* rsc, RsClosure closure) {
    ((Closure*)closure)->eval();
}
#endif

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

    mNumArg = i;
    mArgs = new const void*[mNumArg];
    memcpy(mArgs, values, sizeof(const void*) * mNumArg);

    for (; i < (size_t)numValues; i++) {
        rsAssert(fieldIDs[i] != nullptr);
        mGlobals[fieldIDs[i]] = make_pair(values[i], sizes[i]);
        ALOGV("Creating closure %p, binding field %p (Script %p, slot: %d)",
              this, fieldIDs[i], fieldIDs[i]->mScript, fieldIDs[i]->mSlot);
    }

    size_t j = mNumArg;
    for (const auto& p : mGlobals) {
        rsAssert(p.first == fieldIDs[j]);
        rsAssert(p.second.first == values[j]);
        rsAssert(p.second.second == sizes[j]);
        j++;
    }

    // mDependences.insert(depClosures, depClosures + numValues);

    for (i = 0; i < mNumArg; i++) {
        const Closure* dep = depClosures[i];
        if (dep != nullptr) {
            auto mapping = mArgDeps[dep];
            if (mapping == nullptr) {
                mapping = new Map<int, const ObjectBaseRef<ScriptFieldID>*>();
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
                mapping = new Map<const ObjectBaseRef<ScriptFieldID>*,
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
        mGlobals[fieldIDs[i]] = make_pair(values[i], sizes[i]);
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

    delete[] mArgs;
}

void Closure::setArg(const uint32_t index, const void* value, const size_t size) {
    mArgs[index] = value;
}

void Closure::setGlobal(const ScriptFieldID* fieldID, const void* value,
                        const size_t size) {
    mGlobals[fieldID] = make_pair(value, size);
}

}  // namespace renderscript
}  // namespace android
