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

#include <cutils/compiler.h>

#include "rsContext.h"
#include "rsScriptC.h"
#include "rsMatrix4x4.h"
#include "rsMatrix3x3.h"
#include "rsMatrix2x2.h"

#include "rsCpuCore.h"
#include "rsCpuScript.h"


using namespace android;
using namespace android::renderscript;


//////////////////////////////////////////////////////////////////////////////
// Float util
//////////////////////////////////////////////////////////////////////////////


static void SC_MatrixLoadRotate(Matrix4x4 *m, float rot, float x, float y, float z) {
    m->loadRotate(rot, x, y, z);
}
static void SC_MatrixLoadScale(Matrix4x4 *m, float x, float y, float z) {
    m->loadScale(x, y, z);
}
static void SC_MatrixLoadTranslate(Matrix4x4 *m, float x, float y, float z) {
    m->loadTranslate(x, y, z);
}
static void SC_MatrixRotate(Matrix4x4 *m, float rot, float x, float y, float z) {
    m->rotate(rot, x, y, z);
}
static void SC_MatrixScale(Matrix4x4 *m, float x, float y, float z) {
    m->scale(x, y, z);
}
static void SC_MatrixTranslate(Matrix4x4 *m, float x, float y, float z) {
    m->translate(x, y, z);
}

static void SC_MatrixLoadOrtho(Matrix4x4 *m, float l, float r, float b, float t, float n, float f) {
    m->loadOrtho(l, r, b, t, n, f);
}
static void SC_MatrixLoadFrustum(Matrix4x4 *m, float l, float r, float b, float t, float n, float f) {
    m->loadFrustum(l, r, b, t, n, f);
}
static void SC_MatrixLoadPerspective(Matrix4x4 *m, float fovy, float aspect, float near, float far) {
    m->loadPerspective(fovy, aspect, near, far);
}

static bool SC_MatrixInverse_4x4(Matrix4x4 *m) {
    return m->inverse();
}
static bool SC_MatrixInverseTranspose_4x4(Matrix4x4 *m) {
    return m->inverseTranspose();
}
static void SC_MatrixTranspose_4x4(Matrix4x4 *m) {
    m->transpose();
}
static void SC_MatrixTranspose_3x3(Matrix3x3 *m) {
    m->transpose();
}
static void SC_MatrixTranspose_2x2(Matrix2x2 *m) {
    m->transpose();
}

static float SC_randf(float max) {
    float r = (float)rand();
    r *= max;
    r /= RAND_MAX;
    return r;
}

static float SC_randf2(float min, float max) {
    float r = (float)rand();
    r /= RAND_MAX;
    r = r * (max - min) + min;
    return r;
}

static int SC_randi(int max) {
    return (int)SC_randf(max);
}

static int SC_randi2(int min, int max) {
    return (int)SC_randf2(min, max);
}

static float SC_frac(float v) {
    int i = (int)floor(v);
    return fmin(v - i, 0x1.fffffep-1f);
}


static int32_t SC_AtomicCas(volatile int32_t *ptr, int32_t expectedValue, int32_t newValue) {
    int32_t prev;

    do {
        int32_t ret = android_atomic_release_cas(expectedValue, newValue, ptr);
        if (!ret) {
            // The android cas return 0 if it wrote the value.  This means the
            // previous value was the expected value and we can return.
            return expectedValue;
        }
        // We didn't write the value and need to load the "previous" value.
        prev = *ptr;

        // A race condition exists where the expected value could appear after our cas failed
        // above.  In this case loop until we have a legit previous value or the
        // write passes.
        } while (prev == expectedValue);
    return prev;
}


static int32_t SC_AtomicInc(volatile int32_t *ptr) {
    return android_atomic_inc(ptr);
}

static int32_t SC_AtomicDec(volatile int32_t *ptr) {
    return android_atomic_dec(ptr);
}

static int32_t SC_AtomicAdd(volatile int32_t *ptr, int32_t value) {
    return android_atomic_add(value, ptr);
}

static int32_t SC_AtomicSub(volatile int32_t *ptr, int32_t value) {
    int32_t prev, status;
    do {
        prev = *ptr;
        status = android_atomic_release_cas(prev, prev - value, ptr);
    } while (CC_UNLIKELY(status != 0));
    return prev;
}

static int32_t SC_AtomicAnd(volatile int32_t *ptr, int32_t value) {
    return android_atomic_and(value, ptr);
}

static int32_t SC_AtomicOr(volatile int32_t *ptr, int32_t value) {
    return android_atomic_or(value, ptr);
}

static int32_t SC_AtomicXor(volatile int32_t *ptr, int32_t value) {
    int32_t prev, status;
    do {
        prev = *ptr;
        status = android_atomic_release_cas(prev, prev ^ value, ptr);
    } while (CC_UNLIKELY(status != 0));
    return prev;
}

static uint32_t SC_AtomicUMin(volatile uint32_t *ptr, uint32_t value) {
    uint32_t prev, status;
    do {
        prev = *ptr;
        uint32_t n = rsMin(value, prev);
        status = android_atomic_release_cas((int32_t) prev, (int32_t)n, (volatile int32_t*) ptr);
    } while (CC_UNLIKELY(status != 0));
    return prev;
}

static int32_t SC_AtomicMin(volatile int32_t *ptr, int32_t value) {
    int32_t prev, status;
    do {
        prev = *ptr;
        int32_t n = rsMin(value, prev);
        status = android_atomic_release_cas(prev, n, ptr);
    } while (CC_UNLIKELY(status != 0));
    return prev;
}

static uint32_t SC_AtomicUMax(volatile uint32_t *ptr, uint32_t value) {
    uint32_t prev, status;
    do {
        prev = *ptr;
        uint32_t n = rsMax(value, prev);
        status = android_atomic_release_cas((int32_t) prev, (int32_t) n, (volatile int32_t*) ptr);
    } while (CC_UNLIKELY(status != 0));
    return prev;
}

static int32_t SC_AtomicMax(volatile int32_t *ptr, int32_t value) {
    int32_t prev, status;
    do {
        prev = *ptr;
        int32_t n = rsMax(value, prev);
        status = android_atomic_release_cas(prev, n, ptr);
    } while (CC_UNLIKELY(status != 0));
    return prev;
}



//////////////////////////////////////////////////////////////////////////////
// Class implementation
//////////////////////////////////////////////////////////////////////////////

// llvm name mangling ref
//  <builtin-type> ::= v  # void
//                 ::= b  # bool
//                 ::= c  # char
//                 ::= a  # signed char
//                 ::= h  # unsigned char
//                 ::= s  # short
//                 ::= t  # unsigned short
//                 ::= i  # int
//                 ::= j  # unsigned int
//                 ::= l  # long
//                 ::= m  # unsigned long
//                 ::= x  # long long, __int64
//                 ::= y  # unsigned long long, __int64
//                 ::= f  # float
//                 ::= d  # double

static RsdCpuReference::CpuSymbol gSyms[] = {
    { "_Z4acosf", (void *)&acosf, true },
    { "_Z5acoshf", (void *)&acoshf, true },
    { "_Z4asinf", (void *)&asinf, true },
    { "_Z5asinhf", (void *)&asinhf, true },
    { "_Z4atanf", (void *)&atanf, true },
    { "_Z5atan2ff", (void *)&atan2f, true },
    { "_Z5atanhf", (void *)&atanhf, true },
    { "_Z4cbrtf", (void *)&cbrtf, true },
    { "_Z4ceilf", (void *)&ceilf, true },
    { "_Z8copysignff", (void *)&copysignf, true },
    { "_Z3cosf", (void *)&cosf, true },
    { "_Z4coshf", (void *)&coshf, true },
    { "_Z4erfcf", (void *)&erfcf, true },
    { "_Z3erff", (void *)&erff, true },
    { "_Z3expf", (void *)&expf, true },
    { "_Z4exp2f", (void *)&exp2f, true },
    { "_Z5expm1f", (void *)&expm1f, true },
    { "_Z4fdimff", (void *)&fdimf, true },
    { "_Z5floorf", (void *)&floorf, true },
    { "_Z3fmafff", (void *)&fmaf, true },
    { "_Z4fmaxff", (void *)&fmaxf, true },
    { "_Z4fminff", (void *)&fminf, true },  // float fmin(float, float)
    { "_Z4fmodff", (void *)&fmodf, true },
    { "_Z5frexpfPi", (void *)&frexpf, true },
    { "_Z5hypotff", (void *)&hypotf, true },
    { "_Z5ilogbf", (void *)&ilogbf, true },
    { "_Z5ldexpfi", (void *)&ldexpf, true },
    { "_Z6lgammaf", (void *)&lgammaf, true },
    { "_Z6lgammafPi", (void *)&lgammaf_r, true },
    { "_Z3logf", (void *)&logf, true },
    { "_Z5log10f", (void *)&log10f, true },
    { "_Z5log1pf", (void *)&log1pf, true },
    { "_Z4logbf", (void *)&logbf, true },
    { "_Z4modffPf", (void *)&modff, true },
    //{ "_Z3nanj", (void *)&SC_nan, true },
    { "_Z9nextafterff", (void *)&nextafterf, true },
    { "_Z3powff", (void *)&powf, true },
    { "_Z9remainderff", (void *)&remainderf, true },
    { "_Z6remquoffPi", (void *)&remquof, true },
    { "_Z4rintf", (void *)&rintf, true },
    { "_Z5roundf", (void *)&roundf, true },
    { "_Z3sinf", (void *)&sinf, true },
    { "_Z4sinhf", (void *)&sinhf, true },
    { "_Z4sqrtf", (void *)&sqrtf, true },
    { "_Z3tanf", (void *)&tanf, true },
    { "_Z4tanhf", (void *)&tanhf, true },
    { "_Z6tgammaf", (void *)&tgammaf, true },
    { "_Z5truncf", (void *)&truncf, true },

    //{ "smoothstep", (void *)&, true },

    // matrix
    { "_Z18rsMatrixLoadRotateP12rs_matrix4x4ffff", (void *)&SC_MatrixLoadRotate, true },
    { "_Z17rsMatrixLoadScaleP12rs_matrix4x4fff", (void *)&SC_MatrixLoadScale, true },
    { "_Z21rsMatrixLoadTranslateP12rs_matrix4x4fff", (void *)&SC_MatrixLoadTranslate, true },
    { "_Z14rsMatrixRotateP12rs_matrix4x4ffff", (void *)&SC_MatrixRotate, true },
    { "_Z13rsMatrixScaleP12rs_matrix4x4fff", (void *)&SC_MatrixScale, true },
    { "_Z17rsMatrixTranslateP12rs_matrix4x4fff", (void *)&SC_MatrixTranslate, true },

    { "_Z17rsMatrixLoadOrthoP12rs_matrix4x4ffffff", (void *)&SC_MatrixLoadOrtho, true },
    { "_Z19rsMatrixLoadFrustumP12rs_matrix4x4ffffff", (void *)&SC_MatrixLoadFrustum, true },
    { "_Z23rsMatrixLoadPerspectiveP12rs_matrix4x4ffff", (void *)&SC_MatrixLoadPerspective, true },

    { "_Z15rsMatrixInverseP12rs_matrix4x4", (void *)&SC_MatrixInverse_4x4, true },
    { "_Z24rsMatrixInverseTransposeP12rs_matrix4x4", (void *)&SC_MatrixInverseTranspose_4x4, true },
    { "_Z17rsMatrixTransposeP12rs_matrix4x4", (void *)&SC_MatrixTranspose_4x4, true },
    { "_Z17rsMatrixTransposeP12rs_matrix3x3", (void *)&SC_MatrixTranspose_3x3, true },
    { "_Z17rsMatrixTransposeP12rs_matrix2x2", (void *)&SC_MatrixTranspose_2x2, true },

    // RS Math
    { "_Z6rsRandi", (void *)&SC_randi, true },
    { "_Z6rsRandii", (void *)&SC_randi2, true },
    { "_Z6rsRandf", (void *)&SC_randf, true },
    { "_Z6rsRandff", (void *)&SC_randf2, true },
    { "_Z6rsFracf", (void *)&SC_frac, true },

    // Atomics
    { "_Z11rsAtomicIncPVi", (void *)&SC_AtomicInc, true },
    { "_Z11rsAtomicIncPVj", (void *)&SC_AtomicInc, true },
    { "_Z11rsAtomicDecPVi", (void *)&SC_AtomicDec, true },
    { "_Z11rsAtomicDecPVj", (void *)&SC_AtomicDec, true },
    { "_Z11rsAtomicAddPVii", (void *)&SC_AtomicAdd, true },
    { "_Z11rsAtomicAddPVjj", (void *)&SC_AtomicAdd, true },
    { "_Z11rsAtomicSubPVii", (void *)&SC_AtomicSub, true },
    { "_Z11rsAtomicSubPVjj", (void *)&SC_AtomicSub, true },
    { "_Z11rsAtomicAndPVii", (void *)&SC_AtomicAnd, true },
    { "_Z11rsAtomicAndPVjj", (void *)&SC_AtomicAnd, true },
    { "_Z10rsAtomicOrPVii", (void *)&SC_AtomicOr, true },
    { "_Z10rsAtomicOrPVjj", (void *)&SC_AtomicOr, true },
    { "_Z11rsAtomicXorPVii", (void *)&SC_AtomicXor, true },
    { "_Z11rsAtomicXorPVjj", (void *)&SC_AtomicXor, true },
    { "_Z11rsAtomicMinPVii", (void *)&SC_AtomicMin, true },
    { "_Z11rsAtomicMinPVjj", (void *)&SC_AtomicUMin, true },
    { "_Z11rsAtomicMaxPVii", (void *)&SC_AtomicMax, true },
    { "_Z11rsAtomicMaxPVjj", (void *)&SC_AtomicUMax, true },
    { "_Z11rsAtomicCasPViii", (void *)&SC_AtomicCas, true },
    { "_Z11rsAtomicCasPVjjj", (void *)&SC_AtomicCas, true },

    { NULL, NULL, false }
};

const RsdCpuReference::CpuSymbol * RsdCpuScriptImpl::lookupSymbolMath(const char *sym) {
    const RsdCpuReference::CpuSymbol *syms = gSyms;

    while (syms->fnPtr) {
        if (!strcmp(syms->name, sym)) {
            return syms;
        }
        syms++;
    }
    return NULL;
}

