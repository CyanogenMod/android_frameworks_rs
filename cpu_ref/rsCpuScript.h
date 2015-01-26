/*
 * Copyright (C) 2011-2012 The Android Open Source Project
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

#ifndef RSD_BCC_H
#define RSD_BCC_H

#include <rs_hal.h>
#include <rsRuntime.h>

#ifndef RS_COMPATIBILITY_LIB
#include <utility>
#endif

#include "rsCpuCore.h"

#include <vector>

namespace bcc {
    class BCCContext;
    class RSCompilerDriver;
}

namespace bcinfo {
    class MetadataExtractor;
}

namespace android {
namespace renderscript {

class SharedLibraryUtils {
 public:
#ifndef RS_COMPATIBILITY_LIB
  static bool createSharedLibrary(const char* cacheDir, const char* resName);
#endif

  // Load the shared library referred to by cacheDir and resName. If we have
  // already loaded this library, we instead create a new copy (in the
  // cache dir) and then load that. We then immediately destroy the copy.
  // This is required behavior to implement script instancing for the support
  // library, since shared objects are loaded and de-duped by name only.
  static void* loadSharedLibrary(const char* cacheDir, const char* resName);

 private:
  // Attempt to load the shared library from origName, but then fall back to
  // creating a copy of the shared library if necessary (to ensure instancing).
  // This function returns the dlopen()-ed handle if successful.
  static void *loadSOHelper(const char *origName, const char *cacheDir,
                            const char *resName);

  static const char* LD_EXE_PATH;
  static const char* RS_CACHE_DIR;
};

class ScriptExecutable {
 public:
  ScriptExecutable(Context* RSContext,
                   std::vector<void*>& fieldAddress,
                   std::vector<bool>& fieldIsObject,
                   std::vector<InvokeFunc_t>& invokeFunctions,
                   std::vector<ForEachFunc_t>& forEachFunctions,
                   std::vector<uint32_t>& forEachSignatures,
                   std::vector<const char *> &pragmaKeys,
                   std::vector<const char *> &pragmaValues) : mRS(RSContext) {
      mFieldAddress.swap(fieldAddress);
      mFieldIsObject.swap(fieldIsObject);
      mInvokeFunctions.swap(invokeFunctions);
      mForEachFunctions.swap(forEachFunctions);
      mForEachSignatures.swap(forEachSignatures);
      mPragmaKeys.swap(pragmaKeys);
      mPragmaValues.swap(pragmaValues);
  }

  ~ScriptExecutable() {
      for (size_t i = 0; i < mFieldAddress.size(); ++i) {
          if (mFieldIsObject[i]) {
              if (mFieldAddress[i] != nullptr) {
                  rs_object_base *obj_addr =
                      reinterpret_cast<rs_object_base *>(mFieldAddress[i]);
                  rsrClearObject(mRS, obj_addr);
              }
          }
      }

      for (size_t i = 0; i < mPragmaKeys.size(); ++i) {
          delete [] mPragmaKeys[i];
          delete [] mPragmaValues[i];
      }
  }

  static ScriptExecutable*
  createFromSharedObject(Context* RSContext, void* sharedObj);

  size_t getExportedVariableCount() const { return mFieldAddress.size(); }
  size_t getExportedFunctionCount() const { return mInvokeFunctions.size(); }
  size_t getExportedForEachCount() const { return mForEachFunctions.size(); }
  size_t getPragmaCount() const { return mPragmaKeys.size(); }

  void* getFieldAddress(int slot) const { return mFieldAddress[slot]; }
  bool getFieldIsObject(int slot) const { return mFieldIsObject[slot]; }
  InvokeFunc_t getInvokeFunction(int slot) const { return mInvokeFunctions[slot]; }
  ForEachFunc_t getForEachFunction(int slot) const { return mForEachFunctions[slot]; }
  uint32_t getForEachSignature(int slot) const { return mForEachSignatures[slot]; }

  std::vector<const char *> & getPragmaKeys() { return mPragmaKeys; }
  std::vector<const char *> & getPragmaValues() { return mPragmaValues; }

 private:
  std::vector<void*> mFieldAddress;
  std::vector<bool> mFieldIsObject;
  std::vector<InvokeFunc_t> mInvokeFunctions;
  std::vector<ForEachFunc_t> mForEachFunctions;
  std::vector<uint32_t> mForEachSignatures;
  std::vector<const char *> mPragmaKeys;
  std::vector<const char *> mPragmaValues;

  Context* mRS;
};

class RsdCpuScriptImpl : public RsdCpuReferenceImpl::CpuScript {
public:
    typedef void (*outer_foreach_t)(
        const RsExpandKernelParams *,
        uint32_t x1, uint32_t x2,
        uint32_t outstep);

    typedef void (* InvokeFunc_t)(void);
    typedef void (* ForEachFunc_t)(void);
    typedef int (* RootFunc_t)(void);
#ifdef RS_COMPATIBILITY_LIB
    typedef void (*WorkerCallback_t)(void *usr, uint32_t idx);
#endif

    bool init(char const *resName, char const *cacheDir,
              uint8_t const *bitcode, size_t bitcodeSize, uint32_t flags,
              char const *bccPluginName = nullptr);
    virtual void populateScript(Script *);

    virtual void invokeFunction(uint32_t slot, const void *params, size_t paramLength);
    virtual int invokeRoot();
    virtual void preLaunch(uint32_t slot, const Allocation ** ains,
                           uint32_t inLen, Allocation * aout, const void * usr,
                           uint32_t usrLen, const RsScriptCall *sc);
    virtual void postLaunch(uint32_t slot, const Allocation ** ains,
                            uint32_t inLen, Allocation * aout,
                            const void * usr, uint32_t usrLen,
                            const RsScriptCall *sc);

    virtual void invokeForEach(uint32_t slot,
                               const Allocation ** ains,
                               uint32_t inLen,
                               Allocation* aout,
                               const void* usr,
                               uint32_t usrLen,
                               const RsScriptCall* sc);

    virtual void invokeInit();
    virtual void invokeFreeChildren();

    virtual void setGlobalVar(uint32_t slot, const void *data, size_t dataLength);
    virtual void getGlobalVar(uint32_t slot, void *data, size_t dataLength);
    virtual void setGlobalVarWithElemDims(uint32_t slot, const void *data, size_t dataLength,
                                  const Element *e, const uint32_t *dims, size_t dimLength);
    virtual void setGlobalBind(uint32_t slot, Allocation *data);
    virtual void setGlobalObj(uint32_t slot, ObjectBase *data);


    virtual ~RsdCpuScriptImpl();
    RsdCpuScriptImpl(RsdCpuReferenceImpl *ctx, const Script *s);

    const Script * getScript() {return mScript;}

    void forEachMtlsSetup(const Allocation ** ains, uint32_t inLen,
                          Allocation * aout, const void * usr, uint32_t usrLen,
                          const RsScriptCall *sc, MTLaunchStruct *mtls);

    virtual void forEachKernelSetup(uint32_t slot, MTLaunchStruct *mtls);


    const RsdCpuReference::CpuSymbol * lookupSymbolMath(const char *sym);
    static void * lookupRuntimeStub(void* pContext, char const* name);

    virtual Allocation * getAllocationForPointer(const void *ptr) const;
    bool storeRSInfoFromSO();

protected:
    RsdCpuReferenceImpl *mCtx;
    const Script *mScript;
    void *mScriptSO;

#ifndef RS_COMPATIBILITY_LIB
    // Returns the path to the core library we'll use.
    const char* findCoreLib(const bcinfo::MetadataExtractor& bitCodeMetaData, const char* bitcode,
                            size_t bitcodeSize);

    bcc::RSCompilerDriver *mCompilerDriver;
#endif

    RootFunc_t mRoot;
    RootFunc_t mRootExpand;
    InvokeFunc_t mInit;
    InvokeFunc_t mFreeChildren;
    ScriptExecutable* mScriptExec;

    Allocation **mBoundAllocs;
    void * mIntrinsicData;
    bool mIsThreadable;
};

Allocation * rsdScriptGetAllocationForPointer(
                        const Context *dc,
                        const Script *script,
                        const void *);



}

}

#endif
