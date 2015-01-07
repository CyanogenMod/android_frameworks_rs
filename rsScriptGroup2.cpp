#include "rsScriptGroup2.h"

#include "rsContext.h"

namespace android {
namespace renderscript {

void ScriptGroup2::execute(Context* rsc) {
  if (rsc->mHal.funcs.scriptgroup.execute) {
    rsc->mHal.funcs.scriptgroup.execute(rsc, this);
  }
}

RsScriptGroup2 rsi_ScriptGroup2Create(Context* rsc, RsClosure* closures,
                                      size_t numClosures) {
  ScriptGroup2* group = new ScriptGroup2(rsc, (Closure**)closures, numClosures);

  // Create a device-specific implementation by calling the device driver
  if (rsc->mHal.funcs.scriptgroup.init) {
    rsc->mHal.funcs.scriptgroup.init(rsc, group);
  }

  return group;
}

}  // namespace renderscript
}  // namespace android
