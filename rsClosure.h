#ifndef ANDROID_RENDERSCRIPT_CLOSURE_H_
#define ANDROID_RENDERSCRIPT_CLOSURE_H_

#include <map>
#include <set>
#include <vector>

#include "rsDefines.h"
#include "rsObjectBase.h"

namespace android {
namespace renderscript {

using std::map;
using std::pair;
using std::set;
using std::vector;

class Allocation;
class Context;
class ScriptFieldID;
class ScriptKernelID;
class Type;

class Closure : public ObjectBase {
 public:
  Closure(Context* context,
          const ScriptKernelID* kernelID,
          Allocation* returnValue,
          const int numValues,
          const ScriptFieldID** fieldIDs,
          const void** values,  // Allocations or primitive (numeric) types
          const size_t* sizes,   // size for data type. -1 indicates an allocation.
          const Closure** depClosures,
          const ScriptFieldID** depFieldIDs);

  virtual ~Closure();

  virtual void serialize(Context *rsc, OStream *stream) const {}

  virtual RsA3DClassID getClassId() const { return RS_A3D_CLASS_ID_CLOSURE; }

  void eval();

  void setArg(const uint32_t index, const void* value, const size_t size);
  void setGlobal(const ScriptFieldID* fieldID, const void* value,
                 const size_t size);

  Context* mContext;
  const ObjectBaseRef<ScriptKernelID> mKernelID;

  // Values referrenced in arguments and globals cannot be futures. They must be
  // either a known value or unbound value.
  // For now, all arguments should be Allocations.
  vector<const void*> mArgs;

  // A global could be allocation or any primitive data type.
  map<const ScriptFieldID*, pair<const void*, int>> mGlobals;

  Allocation* mReturnValue;

  // All the other closures that this closure depends on
  set<const Closure*> mDependences;

  // All the other closures which this closure depends on for one of its
  // arguments, and the fields which it depends on.
  map<const Closure*, map<int, const ObjectBaseRef<ScriptFieldID>*>*> mArgDeps;

  // All the other closures that this closure depends on for one of its fields,
  // and the fields that it depends on.
  map<const Closure*, map<const ObjectBaseRef<ScriptFieldID>*,
                          const ObjectBaseRef<ScriptFieldID>*>*> mGlobalDeps;
};

}  // namespace renderscript
}  // namespace android

#endif  // ANDROID_RENDERSCRIPT_CLOSURE_H_
