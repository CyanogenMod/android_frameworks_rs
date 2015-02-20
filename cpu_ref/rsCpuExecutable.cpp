#include "rsCpuExecutable.h"
#include "rsCppUtils.h"

#include <fstream>
#include <set>
#include <memory>

#ifdef RS_COMPATIBILITY_LIB
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#else
#include "bcc/Config/Config.h"
#include <bcc/Renderscript/RSInfo.h>
#include <sys/wait.h>
#endif

#include <dlfcn.h>

namespace android {
namespace renderscript {

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

static std::string findSharedObjectName(const char *cacheDir,
                                        const char *resName) {
#ifndef RS_SERVER
    std::string scriptSOName(cacheDir);
#if defined(RS_COMPATIBILITY_LIB) && !defined(__LP64__)
    size_t cutPos = scriptSOName.rfind("cache");
    if (cutPos != std::string::npos) {
        scriptSOName.erase(cutPos);
    } else {
        ALOGE("Found peculiar cacheDir (missing \"cache\"): %s", cacheDir);
    }
    scriptSOName.append("/lib/librs.");
#else
    scriptSOName.append("/librs.");
#endif // RS_COMPATIBILITY_LIB

#else
    std::string scriptSOName("lib");
#endif // RS_SERVER
    scriptSOName.append(resName);
    scriptSOName.append(".so");

    return scriptSOName;
}

}  // anonymous namespace

const char* SharedLibraryUtils::LD_EXE_PATH = "/system/bin/ld.mc";
const char* SharedLibraryUtils::RS_CACHE_DIR = "com.android.renderscript.cache";

#ifndef RS_COMPATIBILITY_LIB

bool SharedLibraryUtils::createSharedLibrary(const char *cacheDir, const char *resName) {
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

    std::unique_ptr<const char> joined(
        rsuJoinStrings(args.size()-1, args.data()));
    std::string cmdLineStr (joined.get());

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

#endif  // RS_COMPATIBILITY_LIB

const char* RsdCpuScriptImpl::BCC_EXE_PATH = "/system/bin/bcc";

void* SharedLibraryUtils::loadSharedLibrary(const char *cacheDir,
                                            const char *resName,
                                            const char *nativeLibDir) {
    void *loaded = nullptr;

#if defined(RS_COMPATIBILITY_LIB) && defined(__LP64__)
    std::string scriptSOName = findSharedObjectName(nativeLibDir, resName);
#else
    std::string scriptSOName = findSharedObjectName(cacheDir, resName);
#endif

    // We should check if we can load the library from the standard app
    // location for shared libraries first.
    loaded = loadSOHelper(scriptSOName.c_str(), cacheDir, resName);

    if (loaded == nullptr) {
        ALOGE("Unable to open shared library (%s): %s",
              scriptSOName.c_str(), dlerror());

#ifdef RS_COMPATIBILITY_LIB
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
#endif
    }

    return loaded;
}

void* SharedLibraryUtils::loadSOHelper(const char *origName, const char *cacheDir,
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
        newName.append("/");
        newName.append(RS_CACHE_DIR);
        newName.append("/");
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

#define MAXLINE 500
#define MAKE_STR_HELPER(S) #S
#define MAKE_STR(S) MAKE_STR_HELPER(S)
#define EXPORT_VAR_STR "exportVarCount: "
#define EXPORT_FUNC_STR "exportFuncCount: "
#define EXPORT_FOREACH_STR "exportForEachCount: "
#define OBJECT_SLOT_STR "objectSlotCount: "
#define PRAGMA_STR "pragmaCount: "
#define THREADABLE_STR "isThreadable: "
#define CHECKSUM_STR "buildChecksum: "

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

ScriptExecutable* ScriptExecutable::createFromSharedObject(
    Context* RSContext, void* sharedObj) {
    char line[MAXLINE];

    size_t varCount = 0;
    size_t funcCount = 0;
    size_t forEachCount = 0;
    size_t objectSlotCount = 0;
    size_t pragmaCount = 0;
    bool isThreadable = true;

    void** fieldAddress = nullptr;
    bool* fieldIsObject = nullptr;
    char** fieldName = nullptr;
    InvokeFunc_t* invokeFunctions = nullptr;
    ForEachFunc_t* forEachFunctions = nullptr;
    uint32_t* forEachSignatures = nullptr;
    const char ** pragmaKeys = nullptr;
    const char ** pragmaValues = nullptr;
    char *checksum = nullptr;

    const char *rsInfo = (const char *) dlsym(sharedObj, ".rs.info");

    if (strgets(line, MAXLINE, &rsInfo) == nullptr) {
        return nullptr;
    }
    if (sscanf(line, EXPORT_VAR_STR "%zu", &varCount) != 1) {
        ALOGE("Invalid export var count!: %s", line);
        return nullptr;
    }

    fieldAddress = new void*[varCount];
    if (fieldAddress == nullptr) {
        return nullptr;
    }

    fieldIsObject = new bool[varCount];
    if (fieldIsObject == nullptr) {
        goto error;
    }

    fieldName = new char*[varCount];
    if (fieldName == nullptr) {
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
        void* addr = dlsym(sharedObj, line);
        if (addr == nullptr) {
            ALOGE("Failed to find variable address for %s: %s",
                  line, dlerror());
            // Not a critical error if we don't find a global variable.
        }
        fieldAddress[i] = addr;
        fieldIsObject[i] = false;
        fieldName[i] = new char[strlen(line)+1];
        strcpy(fieldName[i], line);
    }

    if (strgets(line, MAXLINE, &rsInfo) == nullptr) {
        goto error;
    }
    if (sscanf(line, EXPORT_FUNC_STR "%zu", &funcCount) != 1) {
        ALOGE("Invalid export func count!: %s", line);
        goto error;
    }

    invokeFunctions = new InvokeFunc_t[funcCount];
    if (invokeFunctions == nullptr) {
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

        invokeFunctions[i] = (InvokeFunc_t) dlsym(sharedObj, line);
        if (invokeFunctions[i] == nullptr) {
            ALOGE("Failed to get function address for %s(): %s",
                  line, dlerror());
            goto error;
        }
    }

    if (strgets(line, MAXLINE, &rsInfo) == nullptr) {
        goto error;
    }
    if (sscanf(line, EXPORT_FOREACH_STR "%zu", &forEachCount) != 1) {
        ALOGE("Invalid export forEach count!: %s", line);
        goto error;
    }

    forEachFunctions = new ForEachFunc_t[forEachCount];
    if (forEachFunctions == nullptr) {
        goto error;
    }

    forEachSignatures = new uint32_t[forEachCount];
    if (forEachSignatures == nullptr) {
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
        forEachSignatures[i] = tmpSig;
        forEachFunctions[i] =
            (ForEachFunc_t) dlsym(sharedObj, tmpName);
        if (i != 0 && forEachFunctions[i] == nullptr &&
            strcmp(tmpName, "root.expand")) {
            // Ignore missing root.expand functions.
            // root() is always specified at location 0.
            ALOGE("Failed to find forEach function address for %s: %s",
                  tmpName, dlerror());
            goto error;
        }
    }

    if (strgets(line, MAXLINE, &rsInfo) == nullptr) {
        goto error;
    }
    if (sscanf(line, OBJECT_SLOT_STR "%zu", &objectSlotCount) != 1) {
        ALOGE("Invalid object slot count!: %s", line);
        goto error;
    }

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
            fieldIsObject[varNum] = true;
        }
    }

#ifndef RS_COMPATIBILITY_LIB
    // Do not attempt to read pragmas or isThreadable flag in compat lib path.
    // Neither is applicable for compat lib

    if (strgets(line, MAXLINE, &rsInfo) == nullptr) {
        goto error;
    }

    if (sscanf(line, PRAGMA_STR "%zu", &pragmaCount) != 1) {
        ALOGE("Invalid pragma count!: %s", line);
        goto error;
    }

    pragmaKeys = new const char*[pragmaCount];
    if (pragmaKeys == nullptr) {
        goto error;
    }

    pragmaValues = new const char*[pragmaCount];
    if (pragmaValues == nullptr) {
        goto error;
    }

    bzero(pragmaKeys, sizeof(char*) * pragmaCount);
    bzero(pragmaValues, sizeof(char*) * pragmaCount);

    for (size_t i = 0; i < pragmaCount; ++i) {
        if (strgets(line, MAXLINE, &rsInfo) == nullptr) {
            ALOGE("Unable to read pragma at index %zu!", i);
            goto error;
        }
        char key[MAXLINE];
        char value[MAXLINE] = ""; // initialize in case value is empty

        // pragmas can just have a key and no value.  Only check to make sure
        // that the key is not empty
        if (sscanf(line, "%" MAKE_STR(MAXLINE) "s - %" MAKE_STR(MAXLINE) "s",
                   key, value) == 0 ||
            strlen(key) == 0)
        {
            ALOGE("Invalid pragma value!: %s", line);

            goto error;
        }

        char *pKey = new char[strlen(key)+1];
        strcpy(pKey, key);
        pragmaKeys[i] = pKey;

        char *pValue = new char[strlen(value)+1];
        strcpy(pValue, value);
        pragmaValues[i] = pValue;
        //ALOGE("Pragma %zu: Key: '%s' Value: '%s'", i, pKey, pValue);
    }

    if (strgets(line, MAXLINE, &rsInfo) == nullptr) {
        goto error;
    }

    char tmpFlag[4];
    if (sscanf(line, THREADABLE_STR "%4s", tmpFlag) != 1) {
        ALOGE("Invalid threadable flag!: %s", line);
        goto error;
    }
    if (strcmp(tmpFlag, "yes") == 0) {
        isThreadable = true;
    } else if (strcmp(tmpFlag, "no") == 0) {
        isThreadable = false;
    } else {
        ALOGE("Invalid threadable flag!: %s", tmpFlag);
        goto error;
    }

    if (strgets(line, MAXLINE, &rsInfo) != nullptr) {
        if (strncmp(line, CHECKSUM_STR, strlen(CHECKSUM_STR)) != 0) {
            ALOGE("Invalid checksum flag!: %s", line);
            goto error;
        }

        // consume trailing newline character
        char *c = strrchr(line, '\n');
        if (c) {
            *c = '\0';
        }

        char *checksumStart = &line[strlen(CHECKSUM_STR)];
        checksum = new char[strlen(checksumStart) + 1];
        strcpy(checksum, checksumStart);
    } else {
        ALOGE("Missing checksum in shared obj file");
        goto error;
    }

#endif  // RS_COMPATIBILITY_LIB

    return new ScriptExecutable(
        RSContext, fieldAddress, fieldIsObject, fieldName, varCount,
        invokeFunctions, funcCount,
        forEachFunctions, forEachSignatures, forEachCount,
        pragmaKeys, pragmaValues, pragmaCount,
        isThreadable, checksum);

error:

#ifndef RS_COMPATIBILITY_LIB
    delete[] checksum;

    for (size_t idx = 0; idx < pragmaCount; ++idx) {
        delete [] pragmaKeys[idx];
        delete [] pragmaValues[idx];
    }

    delete[] pragmaValues;
    delete[] pragmaKeys;
#endif  // RS_COMPATIBILITY_LIB

    delete[] forEachSignatures;
    delete[] forEachFunctions;

    delete[] invokeFunctions;

    for (size_t i = 0; i < varCount; i++) {
        delete[] fieldName[i];
    }
    delete[] fieldName;
    delete[] fieldIsObject;
    delete[] fieldAddress;

    return nullptr;
}

void* ScriptExecutable::getFieldAddress(const char* name) const {
    // TODO: improve this by using a hash map.
    for (size_t i = 0; i < mExportedVarCount; i++) {
        if (strcmp(name, mFieldName[i]) == 0) {
            return mFieldAddress[i];
        }
    }
    return nullptr;
}

}  // namespace renderscript
}  // namespace android
