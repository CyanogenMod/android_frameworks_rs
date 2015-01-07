#ifndef ANDROID_RENDERSCRIPT_SCRIPTGROUP2_H_
#define ANDROID_RENDERSCRIPT_SCRIPTGROUP2_H_

#include "rsScriptGroupBase.h"

#include <list>

namespace android {
namespace renderscript {

class Closure;
class Context;

class ScriptGroup2 : public ScriptGroupBase {
 public:
  /*
    TODO:
    Inputs and outputs are set and retrieved in Java runtime.
    They are opaque in the C++ runtime.
    For better compiler optimizations (of a script group), we need to include
    input and output information in the C++ runtime.
   */
  ScriptGroup2(Context* rsc, Closure** closures, size_t numClosures) :
      ScriptGroupBase(rsc), mClosures(closures, closures + numClosures) {}
  virtual ~ScriptGroup2() {}

  virtual SG_API_Version getApiVersion() const { return SG_V2; }
  virtual void execute(Context* rsc);

  std::list<Closure*> mClosures;
};

}  // namespace renderscript
}  // namespace android

#endif  // ANDROID_RENDERSCRIPT_SCRIPTGROUP2_H_
