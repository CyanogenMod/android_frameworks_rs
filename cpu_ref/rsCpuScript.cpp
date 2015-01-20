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

#include "rsCpuCore.h"
#include "rsCpuScript.h"

#ifdef RS_COMPATIBILITY_LIB
    #include <stdio.h>
    #include <sys/stat.h>
    #include <unistd.h>
#else
    #include <bcc/BCCContext.h>
    #include <bcc/Config/Config.h>
    #include <bcc/Renderscript/RSCompilerDriver.h>
    #include <bcc/Renderscript/RSInfo.h>
    #include <bcinfo/MetadataExtractor.h>
    #include <cutils/properties.h>

    #include <sys/types.h>
    #include <sys/wait.h>
    #include <unistd.h>

    #include <string>
    #include <vector>
#endif

#include <set>
#include <string>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <iostream>

#ifdef __LP64__
#define SYSLIBPATH "/system/lib64"
#else
#define SYSLIBPATH "/system/lib"
#endif

namespace {

// Create a len length string containing random characters from [A-Za-z0-9].
static std::string getRandomString(size_t len) {
    char buf[len + 1];
    for (size_t i = 0; i < len; i++) {
        uint32_t r = arc4random() & 0xffff;
        r %= 62;
        if (r < 26) {
            // lowercase
            buf[i] = 'a' + r;
        } else if (r < 52) {
            // uppercase
            buf[i] = 'A' + (r - 26);
        } else {
            // Use a number
            buf[i] = '0' + (r - 52);
        }
    }
    buf[len] = '\0';
    return std::string(buf);
}

// Check if a path exists and attempt to create it if it doesn't.
static bool ensureCacheDirExists(const char *path) {
    if (access(path, R_OK | W_OK | X_OK) == 0) {
        // Done if we can rwx the directory
        return true;
    }
    if (mkdir(path, 0700) == 0) {
        return true;
    }
    return false;
}

// Copy the file named \p srcFile to \p dstFile.
// Return 0 on success and -1 if anything wasn't copied.
static int copyFile(const char *dstFile, const char *srcFile) {
    std::ifstream srcStream(srcFile);
    if (!srcStream) {
        ALOGE("Could not verify or read source file: %s", srcFile);
        return -1;
    }
    std::ofstream dstStream(dstFile);
    if (!dstStream) {
        ALOGE("Could not verify or write destination file: %s", dstFile);
        return -1;
    }
    dstStream << srcStream.rdbuf();
    if (!dstStream) {
        ALOGE("Could not write destination file: %s", dstFile);
        return -1;
    }

    srcStream.close();
    dstStream.close();

    return 0;
}

#define RS_CACHE_DIR "com.android.renderscript.cache"

// Attempt to load the shared library from origName, but then fall back to
// creating a copy of the shared library if necessary (to ensure instancing).
// This function returns the dlopen()-ed handle if successful.
static void *loadSOHelper(const char *origName, const char *cacheDir,
                          const char *resName) {
    // Keep track of which .so libraries have been loaded. Once a library is
    // in the set (per-process granularity), we must instead make a copy of
    // the original shared object (randomly named .so file) and load that one
    // instead. If we don't do this, we end up aliasing global data between
    // the various Script instances (which are supposed to be completely
    // independent).
    static std::set<std::string> LoadedLibraries;

    void *loaded = nullptr;

    // Skip everything if we don't even have the original library available.
    if (access(origName, F_OK) != 0) {
        return nullptr;
    }

    // Common path is that we have not loaded this Script/library before.
    if (LoadedLibraries.find(origName) == LoadedLibraries.end()) {
        loaded = dlopen(origName, RTLD_NOW | RTLD_LOCAL);
        if (loaded) {
            LoadedLibraries.insert(origName);
        }
        return loaded;
    }

    std::string newName(cacheDir);

    // Append RS_CACHE_DIR only if it is not found in cacheDir
    // In driver mode, RS_CACHE_DIR is already appended to cacheDir.
    if (newName.find(RS_CACHE_DIR) == std::string::npos) {
        newName.append("/" RS_CACHE_DIR "/");
    }

    if (!ensureCacheDirExists(newName.c_str())) {
        ALOGE("Could not verify or create cache dir: %s", cacheDir);
        return nullptr;
    }

    // Construct an appropriately randomized filename for the copy.
    newName.append("librs.");
    newName.append(resName);
    newName.append("#");
    newName.append(getRandomString(6));  // 62^6 potential filename variants.
    newName.append(".so");

    int r = copyFile(newName.c_str(), origName);
    if (r != 0) {
        ALOGE("Could not create copy %s -> %s", origName, newName.c_str());
        return nullptr;
    }
    loaded = dlopen(newName.c_str(), RTLD_NOW | RTLD_LOCAL);
    r = unlink(newName.c_str());
    if (r != 0) {
        ALOGE("Could not unlink copy %s", newName.c_str());
    }
    if (loaded) {
        LoadedLibraries.insert(newName.c_str());
    }

    return loaded;
}

static std::string findSharedObjectName(const char *cacheDir,
                                        const char *resName) {

#ifndef RS_SERVER
    std::string scriptSOName(cacheDir);
#ifdef RS_COMPATIBILITY_LIB
    size_t cutPos = scriptSOName.rfind("cache");
    if (cutPos != std::string::npos) {
        scriptSOName.erase(cutPos);
    } else {
        ALOGE("Found peculiar cacheDir (missing \"cache\"): %s", cacheDir);
    }
    scriptSOName.append("/lib/librs.");
#else
    scriptSOName.append("/librs.");
#endif

#else
    std::string scriptSOName("lib");
#endif
    scriptSOName.append(resName);
    scriptSOName.append(".so");

    return scriptSOName;
}

// Load the shared library referred to by cacheDir and resName. If we have
// already loaded this library, we instead create a new copy (in the
// cache dir) and then load that. We then immediately destroy the copy.
// This is required behavior to implement script instancing for the support
// library, since shared objects are loaded and de-duped by name only.
static void *loadSharedLibrary(const char *cacheDir, const char *resName) {
    void *loaded = nullptr;

    std::string scriptSOName = findSharedObjectName(cacheDir, resName);

    // We should check if we can load the library from the standard app
    // location for shared libraries first.
    loaded = loadSOHelper(scriptSOName.c_str(), cacheDir, resName);

    if (loaded == nullptr) {
        ALOGE("Unable to open shared library (%s): %s",
              scriptSOName.c_str(), dlerror());

        // One final attempt to find the library in "/system/lib".
        // We do this to allow bundled applications to use the compatibility
        // library fallback path. Those applications don't have a private
        // library path, so they need to install to the system directly.
        // Note that this is really just a testing path.
        std::string scriptSONameSystem("/system/lib/librs.");
        scriptSONameSystem.append(resName);
        scriptSONameSystem.append(".so");
        loaded = loadSOHelper(scriptSONameSystem.c_str(), cacheDir,
                              resName);
        if (loaded == nullptr) {
            ALOGE("Unable to open system shared library (%s): %s",
                  scriptSONameSystem.c_str(), dlerror());
        }
    }

    return loaded;
}

#ifndef RS_COMPATIBILITY_LIB

static bool is_force_recompile() {
#ifdef RS_SERVER
  return false;
#else
  char buf[PROPERTY_VALUE_MAX];

  // Re-compile if floating point precision has been overridden.
  property_get("debug.rs.precision", buf, "");
  if (buf[0] != '\0') {
    return true;
  }

  // Re-compile if debug.rs.forcerecompile is set.
  property_get("debug.rs.forcerecompile", buf, "0");
  if ((::strcmp(buf, "1") == 0) || (::strcmp(buf, "true") == 0)) {
    return true;
  } else {
    return false;
  }
#endif  // RS_SERVER
}

const static char *BCC_EXE_PATH = "/system/bin/bcc";

static void setCompileArguments(std::vector<const char*>* args,
                                const std::string& bcFileName,
                                const char* cacheDir, const char* resName,
                                const char* core_lib, bool useRSDebugContext,
                                const char* bccPluginName) {
    rsAssert(cacheDir && resName && core_lib);
    args->push_back(BCC_EXE_PATH);
    args->push_back("-unroll-runtime");
    args->push_back("-scalarize-load-store");
    args->push_back("-o");
    args->push_back(resName);
    args->push_back("-output_path");
    args->push_back(cacheDir);
    args->push_back("-bclib");
    args->push_back(core_lib);
    args->push_back("-mtriple");
    args->push_back(DEFAULT_TARGET_TRIPLE_STRING);

    // Enable workaround for A53 codegen by default.
#if defined(__aarch64__) && !defined(DISABLE_A53_WORKAROUND)
    args->push_back("-aarch64-fix-cortex-a53-835769");
#endif

    // Execute the bcc compiler.
    if (useRSDebugContext) {
        args->push_back("-rs-debug-ctx");
    } else {
        // Only load additional libraries for compiles that don't use
        // the debug context.
        if (bccPluginName && strlen(bccPluginName) > 0) {
            args->push_back("-load");
            args->push_back(bccPluginName);
        }
    }

    args->push_back("-fPIC");
    args->push_back("-embedRSInfo");

    args->push_back(bcFileName.c_str());
    args->push_back(nullptr);
}

static bool compileBitcode(const std::string &bcFileName,
                           const char *bitcode,
                           size_t bitcodeSize,
                           const char **compileArguments,
                           const std::string &compileCommandLine) {
    rsAssert(bitcode && bitcodeSize);

    FILE *bcfile = fopen(bcFileName.c_str(), "w");
    if (!bcfile) {
        ALOGE("Could not write to %s", bcFileName.c_str());
        return false;
    }
    size_t nwritten = fwrite(bitcode, 1, bitcodeSize, bcfile);
    fclose(bcfile);
    if (nwritten != bitcodeSize) {
        ALOGE("Could not write %zu bytes to %s", bitcodeSize,
              bcFileName.c_str());
        return false;
    }

    pid_t pid = fork();

    switch (pid) {
    case -1: {  // Error occurred (we attempt no recovery)
        ALOGE("Couldn't fork for bcc compiler execution");
        return false;
    }
    case 0: {  // Child process
        ALOGV("Invoking BCC with: %s", compileCommandLine.c_str());
        execv(BCC_EXE_PATH, (char* const*)compileArguments);

        ALOGE("execv() failed: %s", strerror(errno));
        abort();
        return false;
    }
    default: {  // Parent process (actual driver)
        // Wait on child process to finish compiling the source.
        int status = 0;
        pid_t w = waitpid(pid, &status, 0);
        if (w == -1) {
            ALOGE("Could not wait for bcc compiler");
            return false;
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return true;
        }

        ALOGE("bcc compiler terminated unexpectedly");
        return false;
    }
    }
}

const static char *LD_EXE_PATH = "/system/bin/ld.mc";

static bool createSharedLib(const char *cacheDir, const char *resName) {
    std::string sharedLibName = findSharedObjectName(cacheDir, resName);
    std::string objFileName = cacheDir;
    objFileName.append("/");
    objFileName.append(resName);
    objFileName.append(".o");

    const char *compiler_rt = SYSLIBPATH"/libcompiler_rt.so";
    std::vector<const char *> args = {
        LD_EXE_PATH,
        "-shared",
        "-nostdlib",
        compiler_rt,
        "-mtriple", DEFAULT_TARGET_TRIPLE_STRING,
        "-L", SYSLIBPATH,
        "-lRSDriver", "-lm", "-lc",
        objFileName.c_str(),
        "-o", sharedLibName.c_str(),
        nullptr
    };

    std::string cmdLineStr = bcc::getCommandLine(args.size()-1, args.data());

    pid_t pid = fork();

    switch (pid) {
    case -1: {  // Error occurred (we attempt no recovery)
        ALOGE("Couldn't fork for linker (%s) execution", LD_EXE_PATH);
        return false;
    }
    case 0: {  // Child process
        ALOGV("Invoking ld.mc with args '%s'", cmdLineStr.c_str());
        execv(LD_EXE_PATH, (char* const*) args.data());

        ALOGE("execv() failed: %s", strerror(errno));
        abort();
        return false;
    }
    default: {  // Parent process (actual driver)
        // Wait on child process to finish compiling the source.
        int status = 0;
        pid_t w = waitpid(pid, &status, 0);
        if (w == -1) {
            ALOGE("Could not wait for linker (%s)", LD_EXE_PATH);
            return false;
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return true;
        }

        ALOGE("Linker (%s) terminated unexpectedly", LD_EXE_PATH);
        return false;
    }
    }
}
#endif  // !defined(RS_COMPATIBILITY_LIB)
}  // namespace

namespace android {
namespace renderscript {

#define MAXLINE 500
#define MAKE_STR_HELPER(S) #S
#define MAKE_STR(S) MAKE_STR_HELPER(S)
#define EXPORT_VAR_STR "exportVarCount: "
#define EXPORT_FUNC_STR "exportFuncCount: "
#define EXPORT_FOREACH_STR "exportForEachCount: "
#define OBJECT_SLOT_STR "objectSlotCount: "

// Copy up to a newline or size chars from str -> s, updating str
// Returns s when successful and nullptr when '\0' is finally reached.
static char* strgets(char *s, int size, const char **ppstr) {
    if (!ppstr || !*ppstr || **ppstr == '\0' || size < 1) {
        return nullptr;
    }

    int i;
    for (i = 0; i < (size - 1); i++) {
        s[i] = **ppstr;
        (*ppstr)++;
        if (s[i] == '\0') {
            return s;
        } else if (s[i] == '\n') {
            s[i+1] = '\0';
            return s;
        }
    }

    // size has been exceeded.
    s[i] = '\0';

    return s;
}

RsdCpuScriptImpl::RsdCpuScriptImpl(RsdCpuReferenceImpl *ctx, const Script *s) {
    mCtx = ctx;
    mScript = s;

    mScriptSO = nullptr;

    mInvokeFunctions = nullptr;
    mForEachFunctions = nullptr;
    mFieldAddress = nullptr;
    mFieldIsObject = nullptr;
    mForEachSignatures = nullptr;

#ifndef RS_COMPATIBILITY_LIB
    mCompilerDriver = nullptr;
#endif


    mRoot = nullptr;
    mRootExpand = nullptr;
    mInit = nullptr;
    mFreeChildren = nullptr;


    mBoundAllocs = nullptr;
    mIntrinsicData = nullptr;
    mIsThreadable = true;
}

bool RsdCpuScriptImpl::storeRSInfoFromSO() {
    char line[MAXLINE];
    size_t varCount = 0;
    size_t funcCount = 0;
    size_t forEachCount = 0;
    size_t objectSlotCount = 0;

    mRoot = (RootFunc_t) dlsym(mScriptSO, "root");
    if (mRoot) {
        //ALOGE("Found root(): %p", mRoot);
    }
    mRootExpand = (RootFunc_t) dlsym(mScriptSO, "root.expand");
    if (mRootExpand) {
        //ALOGE("Found root.expand(): %p", mRootExpand);
    }
    mInit = (InvokeFunc_t) dlsym(mScriptSO, "init");
    if (mInit) {
        //ALOGE("Found init(): %p", mInit);
    }
    mFreeChildren = (InvokeFunc_t) dlsym(mScriptSO, ".rs.dtor");
    if (mFreeChildren) {
        //ALOGE("Found .rs.dtor(): %p", mFreeChildren);
    }

    const char *rsInfo = (const char *) dlsym(mScriptSO, ".rs.info");
    if (rsInfo) {
        //ALOGE("Found .rs.info(): %p - %s", rsInfo, rsInfo);
    }

    if (strgets(line, MAXLINE, &rsInfo) == nullptr) {
        goto error;
    }
    if (sscanf(line, EXPORT_VAR_STR "%zu", &varCount) != 1) {
        ALOGE("Invalid export var count!: %s", line);
        goto error;
    }

    mExportedVariableCount = varCount;
    //ALOGE("varCount: %zu", varCount);
    if (varCount > 0) {
        // Start by creating/zeroing this member, since we don't want to
        // accidentally clean up invalid pointers later (if we error out).
        mFieldIsObject = new bool[varCount];
        if (mFieldIsObject == nullptr) {
            goto error;
        }
        memset(mFieldIsObject, 0, varCount * sizeof(*mFieldIsObject));
        mFieldAddress = new void*[varCount];
        if (mFieldAddress == nullptr) {
            goto error;
        }
        for (size_t i = 0; i < varCount; ++i) {
            if (strgets(line, MAXLINE, &rsInfo) == nullptr) {
                goto error;
            }
            char *c = strrchr(line, '\n');
            if (c) {
                *c = '\0';
            }
            mFieldAddress[i] = dlsym(mScriptSO, line);
            if (mFieldAddress[i] == nullptr) {
                ALOGE("Failed to find variable address for %s: %s",
                      line, dlerror());
                // Not a critical error if we don't find a global variable.
            }
            else {
                //ALOGE("Found variable %s at %p", line,
                //mFieldAddress[i]);
            }
        }
    }

    if (strgets(line, MAXLINE, &rsInfo) == nullptr) {
        goto error;
    }
    if (sscanf(line, EXPORT_FUNC_STR "%zu", &funcCount) != 1) {
        ALOGE("Invalid export func count!: %s", line);
        goto error;
    }

    mExportedFunctionCount = funcCount;
    //ALOGE("funcCount: %zu", funcCount);

    if (funcCount > 0) {
        mInvokeFunctions = new InvokeFunc_t[funcCount];
        if (mInvokeFunctions == nullptr) {
            goto error;
        }
        for (size_t i = 0; i < funcCount; ++i) {
            if (strgets(line, MAXLINE, &rsInfo) == nullptr) {
                goto error;
            }
            char *c = strrchr(line, '\n');
            if (c) {
                *c = '\0';
            }

            mInvokeFunctions[i] = (InvokeFunc_t) dlsym(mScriptSO, line);
            if (mInvokeFunctions[i] == nullptr) {
                ALOGE("Failed to get function address for %s(): %s",
                      line, dlerror());
                goto error;
            }
            else {
                //ALOGE("Found InvokeFunc_t %s at %p", line, mInvokeFunctions[i]);
            }
        }
    }

    if (strgets(line, MAXLINE, &rsInfo) == nullptr) {
        goto error;
    }
    if (sscanf(line, EXPORT_FOREACH_STR "%zu", &forEachCount) != 1) {
        ALOGE("Invalid export forEach count!: %s", line);
        goto error;
    }

    if (forEachCount > 0) {

        mForEachSignatures = new uint32_t[forEachCount];
        if (mForEachSignatures == nullptr) {
            goto error;
        }
        mForEachFunctions = new ForEachFunc_t[forEachCount];
        if (mForEachFunctions == nullptr) {
            goto error;
        }
        for (size_t i = 0; i < forEachCount; ++i) {
            unsigned int tmpSig = 0;
            char tmpName[MAXLINE];

            if (strgets(line, MAXLINE, &rsInfo) == nullptr) {
                goto error;
            }
            if (sscanf(line, "%u - %" MAKE_STR(MAXLINE) "s",
                       &tmpSig, tmpName) != 2) {
                ALOGE("Invalid export forEach!: %s", line);
                goto error;
            }

            // Lookup the expanded ForEach kernel.
            strncat(tmpName, ".expand", MAXLINE-1-strlen(tmpName));
            mForEachSignatures[i] = tmpSig;
            mForEachFunctions[i] =
                    (ForEachFunc_t) dlsym(mScriptSO, tmpName);
            if (i != 0 && mForEachFunctions[i] == nullptr) {
                // Ignore missing root.expand functions.
                // root() is always specified at location 0.
                ALOGE("Failed to find forEach function address for %s: %s",
                      tmpName, dlerror());
                goto error;
            }
            else {
                //ALOGE("Found forEach %s at %p", tmpName, mForEachFunctions[i]);
            }
        }
    }

    if (strgets(line, MAXLINE, &rsInfo) == nullptr) {
        goto error;
    }
    if (sscanf(line, OBJECT_SLOT_STR "%zu", &objectSlotCount) != 1) {
        ALOGE("Invalid object slot count!: %s", line);
        goto error;
    }

    if (objectSlotCount > 0) {
        rsAssert(varCount > 0);
        for (size_t i = 0; i < objectSlotCount; ++i) {
            uint32_t varNum = 0;
            if (strgets(line, MAXLINE, &rsInfo) == nullptr) {
                goto error;
            }
            if (sscanf(line, "%u", &varNum) != 1) {
                ALOGE("Invalid object slot!: %s", line);
                goto error;
            }

            if (varNum < varCount) {
                mFieldIsObject[varNum] = true;
            }
        }
    }

    if (varCount > 0) {
        mBoundAllocs = new Allocation *[varCount];
        memset(mBoundAllocs, 0, varCount * sizeof(*mBoundAllocs));
    }

    if (mScriptSO == (void*)1) {
        //rsdLookupRuntimeStub(script, "acos");
    }

    return true;

error:
    delete[] mInvokeFunctions;
    delete[] mForEachFunctions;
    delete[] mFieldAddress;
    delete[] mFieldIsObject;
    delete[] mForEachSignatures;
    delete[] mBoundAllocs;

    return false;
}

bool RsdCpuScriptImpl::init(char const *resName, char const *cacheDir,
                            uint8_t const *bitcode, size_t bitcodeSize,
                            uint32_t flags, char const *bccPluginName) {
    //ALOGE("rsdScriptCreate %p %p %p %p %i %i %p", rsc, resName, cacheDir, bitcode, bitcodeSize, flags, lookupFunc);
    //ALOGE("rsdScriptInit %p %p", rsc, script);

    mCtx->lockMutex();
#ifndef RS_COMPATIBILITY_LIB
    bool useRSDebugContext = false;

    mCompilerDriver = nullptr;

    mCompilerDriver = new bcc::RSCompilerDriver();
    if (mCompilerDriver == nullptr) {
        ALOGE("bcc: FAILS to create compiler driver (out of memory)");
        mCtx->unlockMutex();
        return false;
    }

    // Run any compiler setup functions we have been provided with.
    RSSetupCompilerCallback setupCompilerCallback =
            mCtx->getSetupCompilerCallback();
    if (setupCompilerCallback != nullptr) {
        setupCompilerCallback(mCompilerDriver);
    }

    bcinfo::MetadataExtractor bitcodeMetadata((const char *) bitcode, bitcodeSize);
    if (!bitcodeMetadata.extract()) {
        ALOGE("Could not extract metadata from bitcode");
        mCtx->unlockMutex();
        return false;
    }

    const char* core_lib = findCoreLib(bitcodeMetadata, (const char*)bitcode, bitcodeSize);

    if (mCtx->getContext()->getContextType() == RS_CONTEXT_TYPE_DEBUG) {
        mCompilerDriver->setDebugContext(true);
        useRSDebugContext = true;
    }

    std::string bcFileName(cacheDir);
    bcFileName.append("/");
    bcFileName.append(resName);
    bcFileName.append(".bc");

    std::vector<const char*> compileArguments;
    setCompileArguments(&compileArguments, bcFileName, cacheDir, resName, core_lib,
                        useRSDebugContext, bccPluginName);
    // The last argument of compileArguments ia a nullptr, so remove 1 from the size.
    std::string compileCommandLine =
                bcc::getCommandLine(compileArguments.size() - 1, compileArguments.data());

    if (!is_force_recompile()) {
        mScriptSO = loadSharedLibrary(cacheDir, resName);
    }

    // If we can't, it's either not there or out of date.  We compile the bit code and try loading
    // again.
    if (mScriptSO == nullptr) {
        if (!compileBitcode(bcFileName, (const char*)bitcode, bitcodeSize,
                            compileArguments.data(), compileCommandLine))
        {
            ALOGE("bcc: FAILS to compile '%s'", resName);
            mCtx->unlockMutex();
            return false;
        }

        if (!createSharedLib(cacheDir, resName)) {
            ALOGE("Linker: Failed to link object file '%s'", resName);
            mCtx->unlockMutex();
            return false;
        }

        mScriptSO = loadSharedLibrary(cacheDir, resName);
        if (mScriptSO == nullptr) {
            ALOGE("Unable to load '%s'", resName);
            mCtx->unlockMutex();
            return false;
        }
    }

    // Read RS symbol information from the .so.
    if ( !mScriptSO) {
        goto error;
    }

    if ( !storeRSInfoFromSO()) {
      goto error;
    }
#else  // RS_COMPATIBILITY_LIB is defined

    mScriptSO = loadSharedLibrary(cacheDir, resName);

    if (!mScriptSO) {
        goto error;
    }

    if (!storeRSInfoFromSO()) {
        goto error;
    }
#endif
    mCtx->unlockMutex();
    return true;

error:

    mCtx->unlockMutex();
    if (mScriptSO) {
        dlclose(mScriptSO);
    }
    return false;
}

#ifndef RS_COMPATIBILITY_LIB

const char* RsdCpuScriptImpl::findCoreLib(const bcinfo::MetadataExtractor& ME, const char* bitcode,
                                          size_t bitcodeSize) {
    const char* defaultLib = SYSLIBPATH"/libclcore.bc";

    // If we're debugging, use the debug library.
    if (mCtx->getContext()->getContextType() == RS_CONTEXT_TYPE_DEBUG) {
        return SYSLIBPATH"/libclcore_debug.bc";
    }

    // If a callback has been registered to specify a library, use that.
    RSSelectRTCallback selectRTCallback = mCtx->getSelectRTCallback();
    if (selectRTCallback != nullptr) {
        return selectRTCallback((const char*)bitcode, bitcodeSize);
    }

    // Check for a platform specific library
#if defined(ARCH_ARM_HAVE_NEON) && !defined(DISABLE_CLCORE_NEON)
    enum bcinfo::RSFloatPrecision prec = ME.getRSFloatPrecision();
    if (prec == bcinfo::RS_FP_Relaxed) {
        // NEON-capable ARMv7a devices can use an accelerated math library
        // for all reduced precision scripts.
        // ARMv8 does not use NEON, as ASIMD can be used with all precision
        // levels.
        return SYSLIBPATH"/libclcore_neon.bc";
    } else {
        return defaultLib;
    }
#elif defined(__i386__) || defined(__x86_64__)
    // x86 devices will use an optimized library.
    return SYSLIBPATH"/libclcore_x86.bc";
#else
    return defaultLib;
#endif
}

#endif

void RsdCpuScriptImpl::populateScript(Script *script) {
    // Copy info over to runtime
    script->mHal.info.exportedFunctionCount = mExportedFunctionCount;
    script->mHal.info.exportedVariableCount = mExportedVariableCount;
    script->mHal.info.exportedPragmaCount = 0;
    script->mHal.info.exportedPragmaKeyList = 0;
    script->mHal.info.exportedPragmaValueList = 0;

    // Bug, need to stash in metadata
    if (mRootExpand) {
        script->mHal.info.root = mRootExpand;
    } else {
        script->mHal.info.root = mRoot;
    }
}


typedef void (*rs_t)(const void *, void *, const void *, uint32_t, uint32_t, uint32_t, uint32_t);

void RsdCpuScriptImpl::forEachMtlsSetup(const Allocation ** ains,
                                        uint32_t inLen,
                                        Allocation * aout,
                                        const void * usr, uint32_t usrLen,
                                        const RsScriptCall *sc,
                                        MTLaunchStruct *mtls) {

    memset(mtls, 0, sizeof(MTLaunchStruct));

    for (int index = inLen; --index >= 0;) {
        const Allocation* ain = ains[index];

        // possible for this to occur if IO_OUTPUT/IO_INPUT with no bound surface
        if (ain != nullptr &&
            (const uint8_t *)ain->mHal.drvState.lod[0].mallocPtr == nullptr) {

            mCtx->getContext()->setError(RS_ERROR_BAD_SCRIPT,
                                         "rsForEach called with null in allocations");
            return;
        }
    }

    if (aout &&
        (const uint8_t *)aout->mHal.drvState.lod[0].mallocPtr == nullptr) {

        mCtx->getContext()->setError(RS_ERROR_BAD_SCRIPT,
                                     "rsForEach called with null out allocations");
        return;
    }

    if (inLen > 0) {
        const Allocation *ain0   = ains[0];
        const Type       *inType = ain0->getType();

        mtls->fep.dim.x = inType->getDimX();
        mtls->fep.dim.y = inType->getDimY();
        mtls->fep.dim.z = inType->getDimZ();

        for (int Index = inLen; --Index >= 1;) {
            if (!ain0->hasSameDims(ains[Index])) {
                mCtx->getContext()->setError(RS_ERROR_BAD_SCRIPT,
                  "Failed to launch kernel; dimensions of input and output allocations do not match.");

                return;
            }
        }

    } else if (aout != nullptr) {
        const Type *outType = aout->getType();

        mtls->fep.dim.x = outType->getDimX();
        mtls->fep.dim.y = outType->getDimY();
        mtls->fep.dim.z = outType->getDimZ();

    } else {
        mCtx->getContext()->setError(RS_ERROR_BAD_SCRIPT,
                                     "rsForEach called with null allocations");
        return;
    }

    if (inLen > 0 && aout != nullptr) {
        if (!ains[0]->hasSameDims(aout)) {
            mCtx->getContext()->setError(RS_ERROR_BAD_SCRIPT,
              "Failed to launch kernel; dimensions of input and output allocations do not match.");

            return;
        }
    }

    if (!sc || (sc->xEnd == 0)) {
        mtls->xEnd = mtls->fep.dim.x;
    } else {
        rsAssert(sc->xStart < mtls->fep.dim.x);
        rsAssert(sc->xEnd <= mtls->fep.dim.x);
        rsAssert(sc->xStart < sc->xEnd);
        mtls->xStart = rsMin(mtls->fep.dim.x, sc->xStart);
        mtls->xEnd = rsMin(mtls->fep.dim.x, sc->xEnd);
        if (mtls->xStart >= mtls->xEnd) return;
    }

    if (!sc || (sc->yEnd == 0)) {
        mtls->yEnd = mtls->fep.dim.y;
    } else {
        rsAssert(sc->yStart < mtls->fep.dim.y);
        rsAssert(sc->yEnd <= mtls->fep.dim.y);
        rsAssert(sc->yStart < sc->yEnd);
        mtls->yStart = rsMin(mtls->fep.dim.y, sc->yStart);
        mtls->yEnd = rsMin(mtls->fep.dim.y, sc->yEnd);
        if (mtls->yStart >= mtls->yEnd) return;
    }

    if (!sc || (sc->zEnd == 0)) {
        mtls->zEnd = mtls->fep.dim.z;
    } else {
        rsAssert(sc->zStart < mtls->fep.dim.z);
        rsAssert(sc->zEnd <= mtls->fep.dim.z);
        rsAssert(sc->zStart < sc->zEnd);
        mtls->zStart = rsMin(mtls->fep.dim.z, sc->zStart);
        mtls->zEnd = rsMin(mtls->fep.dim.z, sc->zEnd);
        if (mtls->zStart >= mtls->zEnd) return;
    }

    mtls->xEnd     = rsMax((uint32_t)1, mtls->xEnd);
    mtls->yEnd     = rsMax((uint32_t)1, mtls->yEnd);
    mtls->zEnd     = rsMax((uint32_t)1, mtls->zEnd);
    mtls->arrayEnd = rsMax((uint32_t)1, mtls->arrayEnd);

    rsAssert(inLen == 0 || (ains[0]->getType()->getDimZ() == 0));

    mtls->rsc        = mCtx;
    if (ains) {
        memcpy(mtls->ains, ains, inLen * sizeof(ains[0]));
    }
    mtls->aout[0]    = aout;
    mtls->fep.usr    = usr;
    mtls->fep.usrLen = usrLen;
    mtls->mSliceSize = 1;
    mtls->mSliceNum  = 0;

    mtls->isThreadable  = mIsThreadable;

    if (inLen > 0) {
        mtls->fep.inLen = inLen;
        for (int index = inLen; --index >= 0;) {
            mtls->fep.inPtr[index] = (const uint8_t*)ains[index]->mHal.drvState.lod[0].mallocPtr;
            mtls->fep.inStride[index] = ains[index]->getType()->getElementSizeBytes();
        }
    }

    if (aout != nullptr) {
        mtls->fep.outPtr[0] = (uint8_t *)aout->mHal.drvState.lod[0].mallocPtr;
        mtls->fep.outStride[0] = aout->getType()->getElementSizeBytes();
    }
}


void RsdCpuScriptImpl::invokeForEach(uint32_t slot,
                                     const Allocation ** ains,
                                     uint32_t inLen,
                                     Allocation * aout,
                                     const void * usr,
                                     uint32_t usrLen,
                                     const RsScriptCall *sc) {

    MTLaunchStruct mtls;

    forEachMtlsSetup(ains, inLen, aout, usr, usrLen, sc, &mtls);
    forEachKernelSetup(slot, &mtls);

    RsdCpuScriptImpl * oldTLS = mCtx->setTLS(this);
    mCtx->launchThreads(ains, inLen, aout, sc, &mtls);
    mCtx->setTLS(oldTLS);
}

void RsdCpuScriptImpl::forEachKernelSetup(uint32_t slot, MTLaunchStruct *mtls) {
    mtls->script = this;
    mtls->fep.slot = slot;
    mtls->kernel = reinterpret_cast<ForEachFunc_t>(mForEachFunctions[slot]);
    rsAssert(mtls->kernel != nullptr);
    mtls->sig = mForEachSignatures[slot];
}

int RsdCpuScriptImpl::invokeRoot() {
    RsdCpuScriptImpl * oldTLS = mCtx->setTLS(this);
    int ret = mRoot();
    mCtx->setTLS(oldTLS);
    return ret;
}

void RsdCpuScriptImpl::invokeInit() {
    if (mInit) {
        mInit();
    }
}

void RsdCpuScriptImpl::invokeFreeChildren() {
    if (mFreeChildren) {
        mFreeChildren();
    }
}

void RsdCpuScriptImpl::invokeFunction(uint32_t slot, const void *params,
                                      size_t paramLength) {
    //ALOGE("invoke %i %p %zu", slot, params, paramLength);
    void * ap = nullptr;

#if defined(__x86_64__)
    // The invoked function could have input parameter of vector type for example float4 which
    // requires void* params to be 16 bytes aligned when using SSE instructions for x86_64 platform.
    // So try to align void* params before passing them into RS exported function.

    if ((uint8_t)(uint64_t)params & 0x0F) {
        if ((ap = (void*)memalign(16, paramLength)) != nullptr) {
            memcpy(ap, params, paramLength);
        } else {
            ALOGE("x86_64: invokeFunction memalign error, still use params which is not 16 bytes aligned.");
        }
    }
#endif

    RsdCpuScriptImpl * oldTLS = mCtx->setTLS(this);
    reinterpret_cast<void (*)(const void *, uint32_t)>(
        mInvokeFunctions[slot])(ap? (const void *) ap: params, paramLength);

    mCtx->setTLS(oldTLS);
}

void RsdCpuScriptImpl::setGlobalVar(uint32_t slot, const void *data, size_t dataLength) {
    //rsAssert(!script->mFieldIsObject[slot]);
    //ALOGE("setGlobalVar %i %p %zu", slot, data, dataLength);

    //if (mIntrinsicID) {
        //mIntrinsicFuncs.setVar(dc, script, drv->mIntrinsicData, slot, data, dataLength);
        //return;
    //}

    int32_t *destPtr = reinterpret_cast<int32_t *>(mFieldAddress[slot]);
    if (!destPtr) {
        //ALOGV("Calling setVar on slot = %i which is null", slot);
        return;
    }

    memcpy(destPtr, data, dataLength);
}

void RsdCpuScriptImpl::getGlobalVar(uint32_t slot, void *data, size_t dataLength) {
    //rsAssert(!script->mFieldIsObject[slot]);
    //ALOGE("getGlobalVar %i %p %zu", slot, data, dataLength);

    int32_t *srcPtr = reinterpret_cast<int32_t *>(mFieldAddress[slot]);
    if (!srcPtr) {
        //ALOGV("Calling setVar on slot = %i which is null", slot);
        return;
    }
    memcpy(data, srcPtr, dataLength);
}


void RsdCpuScriptImpl::setGlobalVarWithElemDims(uint32_t slot, const void *data, size_t dataLength,
                                                const Element *elem,
                                                const uint32_t *dims, size_t dimLength) {
    int32_t *destPtr = reinterpret_cast<int32_t *>(mFieldAddress[slot]);
    if (!destPtr) {
        //ALOGV("Calling setVar on slot = %i which is null", slot);
        return;
    }

    // We want to look at dimension in terms of integer components,
    // but dimLength is given in terms of bytes.
    dimLength /= sizeof(int);

    // Only a single dimension is currently supported.
    rsAssert(dimLength == 1);
    if (dimLength == 1) {
        // First do the increment loop.
        size_t stride = elem->getSizeBytes();
        const char *cVal = reinterpret_cast<const char *>(data);
        for (uint32_t i = 0; i < dims[0]; i++) {
            elem->incRefs(cVal);
            cVal += stride;
        }

        // Decrement loop comes after (to prevent race conditions).
        char *oldVal = reinterpret_cast<char *>(destPtr);
        for (uint32_t i = 0; i < dims[0]; i++) {
            elem->decRefs(oldVal);
            oldVal += stride;
        }
    }

    memcpy(destPtr, data, dataLength);
}

void RsdCpuScriptImpl::setGlobalBind(uint32_t slot, Allocation *data) {

    //rsAssert(!script->mFieldIsObject[slot]);
    //ALOGE("setGlobalBind %i %p", slot, data);

    int32_t *destPtr = reinterpret_cast<int32_t *>(mFieldAddress[slot]);
    if (!destPtr) {
        //ALOGV("Calling setVar on slot = %i which is null", slot);
        return;
    }

    void *ptr = nullptr;
    mBoundAllocs[slot] = data;
    if (data) {
        ptr = data->mHal.drvState.lod[0].mallocPtr;
    }
    memcpy(destPtr, &ptr, sizeof(void *));
}

void RsdCpuScriptImpl::setGlobalObj(uint32_t slot, ObjectBase *data) {

    //rsAssert(script->mFieldIsObject[slot]);
    //ALOGE("setGlobalObj %i %p", slot, data);

    int32_t *destPtr = reinterpret_cast<int32_t *>(mFieldAddress[slot]);
    if (!destPtr) {
        //ALOGV("Calling setVar on slot = %i which is null", slot);
        return;
    }

    rsrSetObject(mCtx->getContext(), (rs_object_base *)destPtr, data);
}

RsdCpuScriptImpl::~RsdCpuScriptImpl() {
#ifndef RS_COMPATIBILITY_LIB

    if (mCompilerDriver) {
        delete mCompilerDriver;
    }

#endif

    if (mFieldIsObject) {
        for (size_t i = 0; i < mExportedVariableCount; ++i) {
            if (mFieldIsObject[i]) {
                if (mFieldAddress[i] != nullptr) {
                    rs_object_base *obj_addr =
                        reinterpret_cast<rs_object_base *>(mFieldAddress[i]);
                    rsrClearObject(mCtx->getContext(), obj_addr);
                }
            }
        }
    }

    if (mInvokeFunctions) delete[] mInvokeFunctions;
    if (mForEachFunctions) delete[] mForEachFunctions;
    if (mFieldAddress) delete[] mFieldAddress;
    if (mFieldIsObject) delete[] mFieldIsObject;
    if (mForEachSignatures) delete[] mForEachSignatures;
    if (mBoundAllocs) delete[] mBoundAllocs;
    if (mScriptSO) {
        dlclose(mScriptSO);
    }
}

Allocation * RsdCpuScriptImpl::getAllocationForPointer(const void *ptr) const {
    if (!ptr) {
        return nullptr;
    }

    for (uint32_t ct=0; ct < mScript->mHal.info.exportedVariableCount; ct++) {
        Allocation *a = mBoundAllocs[ct];
        if (!a) continue;
        if (a->mHal.drvState.lod[0].mallocPtr == ptr) {
            return a;
        }
    }
    ALOGE("rsGetAllocation, failed to find %p", ptr);
    return nullptr;
}

void RsdCpuScriptImpl::preLaunch(uint32_t slot, const Allocation ** ains,
                                 uint32_t inLen, Allocation * aout,
                                 const void * usr, uint32_t usrLen,
                                 const RsScriptCall *sc) {}

void RsdCpuScriptImpl::postLaunch(uint32_t slot, const Allocation ** ains,
                                  uint32_t inLen, Allocation * aout,
                                  const void * usr, uint32_t usrLen,
                                  const RsScriptCall *sc) {}


}
}
