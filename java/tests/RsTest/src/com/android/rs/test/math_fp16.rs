#include "shared.rsh"

static volatile half h1;
static volatile half2 h2;
static volatile half3 h3;
static volatile half4 h4;

static volatile int i1;
static volatile int2 i2;
static volatile int3 i3;
static volatile int4 i4;

#define TEST_HN_FUNC_HN(fn) \
    h1 = fn(h1);            \
    h2 = fn(h2);            \
    h3 = fn(h3);            \
    h4 = fn(h4);

#define TEST_IN_FUNC_HN(fn) \
    i1 = fn(h1);            \
    i2 = fn(h2);            \
    i3 = fn(h3);            \
    i4 = fn(h4);

#define TEST_HN_FUNC_HN_HN(fn)  \
    h1 = fn(h1, h1);            \
    h2 = fn(h2, h2);            \
    h3 = fn(h3, h3);            \
    h4 = fn(h4, h4);

#define TEST_HN_FUNC_HN_IN(fn)  \
    h1 = fn(h1, i1);            \
    h2 = fn(h2, i2);            \
    h3 = fn(h3, i3);            \
    h4 = fn(h4, i4);

#define TEST_HN_FUNC_HN_PIN(fn) \
    h1 = fn(h1, (int *) &i1);   \
    h2 = fn(h2, (int2 *) &i2);  \
    h3 = fn(h3, (int3 *) &i3);  \
    h4 = fn(h4, (int4 *) &i4);

#define TEST_HN_FUNC_HN_I(fn)  \
    h1 = fn(h1, i1);           \
    h2 = fn(h2, i1);           \
    h3 = fn(h3, i1);           \
    h4 = fn(h4, i1);

#define TEST_HN_FUNC_HN_H(fn)  \
    h1 = fn(h1, h1);           \
    h2 = fn(h2, h1);           \
    h3 = fn(h3, h1);           \
    h4 = fn(h4, h1);

#define TEST_HN_FUNC_H_HN(fn)  \
    h1 = fn(h1, h1);           \
    h2 = fn(h1, h2);           \
    h3 = fn(h1, h3);           \
    h4 = fn(h1, h4);           \

#define TEST_HN_FUNC_HN_PHN(fn) \
    h1 = fn(h1, (half *) &h1);  \
    h2 = fn(h2, (half2 *) &h2); \
    h3 = fn(h3, (half3 *) &h3); \
    h4 = fn(h4, (half4 *) &h4); \

#define TEST_HN_FUNC_HN_HN_HN(fn)   \
    h1 = fn(h1, h1, h1);            \
    h2 = fn(h2, h2, h2);            \
    h3 = fn(h3, h3, h3);            \
    h4 = fn(h4, h4, h4);

#define TEST_HN_FUNC_HN_HN_H(fn)   \
    h1 = fn(h1, h1, h1);           \
    h2 = fn(h2, h2, h1);           \
    h3 = fn(h3, h3, h1);           \
    h4 = fn(h4, h4, h1);

#define TEST_HN_FUNC_HN_HN_PIN(fn) \
    h1 = fn(h1, h1, (int *) &i1);  \
    h2 = fn(h2, h2, (int2 *) &i2); \
    h3 = fn(h3, h3, (int3 *) &i3); \
    h4 = fn(h4, h4, (int4 *) &i4);

static bool testAPI() {
    TEST_HN_FUNC_HN(acos);
    TEST_HN_FUNC_HN(acosh);
    TEST_HN_FUNC_HN(acospi);

    TEST_HN_FUNC_HN(asin);
    TEST_HN_FUNC_HN(asinh);
    TEST_HN_FUNC_HN(asinpi);

    TEST_HN_FUNC_HN(atan);
    TEST_HN_FUNC_HN_HN(atan2);
    TEST_HN_FUNC_HN_HN(atan2pi);
    TEST_HN_FUNC_HN(atanh);
    TEST_HN_FUNC_HN(atanpi);

    TEST_HN_FUNC_HN(cbrt);
    TEST_HN_FUNC_HN(ceil);
    // TODO add copysign test once function is added

    TEST_HN_FUNC_HN(cos);
    TEST_HN_FUNC_HN(cosh);
    TEST_HN_FUNC_HN(cospi);

    TEST_HN_FUNC_HN(degrees);
    TEST_HN_FUNC_HN(erf);
    TEST_HN_FUNC_HN(erfc);
    TEST_HN_FUNC_HN(exp);
    TEST_HN_FUNC_HN(exp10);
    TEST_HN_FUNC_HN(exp2);
    TEST_HN_FUNC_HN(expm1);

    TEST_HN_FUNC_HN(fabs);
    TEST_HN_FUNC_HN_HN(fdim);
    TEST_HN_FUNC_HN(floor);
    TEST_HN_FUNC_HN_HN_HN(fma);

    TEST_HN_FUNC_HN_HN(fmax);
    TEST_HN_FUNC_HN_H(fmax);
    TEST_HN_FUNC_HN_HN(fmin);
    TEST_HN_FUNC_HN_H(fmin);
    TEST_HN_FUNC_HN_HN(fmod);

    TEST_HN_FUNC_HN_HN(hypot);
    // TODO add ilogb test once function is added
    TEST_HN_FUNC_HN_IN(ldexp);
    TEST_HN_FUNC_HN_I(ldexp);
    TEST_HN_FUNC_HN(lgamma);
    TEST_HN_FUNC_HN_PIN(lgamma);

    TEST_HN_FUNC_HN(log);
    TEST_HN_FUNC_HN(log10);
    TEST_HN_FUNC_HN(log1p);
    TEST_HN_FUNC_HN(log2);
    TEST_HN_FUNC_HN(logb);

    TEST_HN_FUNC_HN_HN_HN(mad);
    TEST_HN_FUNC_HN_HN(max);
    TEST_HN_FUNC_HN_H(max);
    TEST_HN_FUNC_HN_HN(min);
    TEST_HN_FUNC_HN_H(min);
    TEST_HN_FUNC_HN_HN_HN(mix);
    TEST_HN_FUNC_HN_HN_H(mix);

    h1 = nan_half();

    TEST_HN_FUNC_HN(native_acos);
    TEST_HN_FUNC_HN(native_acosh);
    TEST_HN_FUNC_HN(native_acospi);

    TEST_HN_FUNC_HN(native_asin);
    TEST_HN_FUNC_HN(native_asinh);
    TEST_HN_FUNC_HN(native_asinpi);

    TEST_HN_FUNC_HN(native_atan);
    TEST_HN_FUNC_HN_HN(native_atan2);
    TEST_HN_FUNC_HN_HN(native_atan2pi);
    TEST_HN_FUNC_HN(native_atanh);
    TEST_HN_FUNC_HN(native_atanpi);

    TEST_HN_FUNC_HN(native_cbrt);
    TEST_HN_FUNC_HN(native_cos);
    TEST_HN_FUNC_HN(native_cosh);
    TEST_HN_FUNC_HN(native_cospi);

    TEST_HN_FUNC_HN_HN(native_divide);
    TEST_HN_FUNC_HN(native_exp);
    TEST_HN_FUNC_HN(native_exp10);
    TEST_HN_FUNC_HN(native_exp2);
    TEST_HN_FUNC_HN(native_expm1);

    TEST_HN_FUNC_HN_HN(native_hypot);
    TEST_HN_FUNC_HN(native_log);
    TEST_HN_FUNC_HN(native_log10);
    TEST_HN_FUNC_HN(native_log1p);
    TEST_HN_FUNC_HN(native_log2);

    TEST_HN_FUNC_HN_HN(native_powr);
    TEST_HN_FUNC_HN(native_recip);
    TEST_HN_FUNC_HN_IN(native_rootn);
    TEST_HN_FUNC_HN(native_rsqrt);

    TEST_HN_FUNC_HN(native_sin);
    TEST_HN_FUNC_HN_PHN(native_sincos);
    TEST_HN_FUNC_HN(native_sinh);
    TEST_HN_FUNC_HN(native_sinpi);

    TEST_HN_FUNC_HN(native_tan);
    TEST_HN_FUNC_HN(native_tanh);
    TEST_HN_FUNC_HN(native_tanpi);

    // Bug: https://b.corp.google.com/issues/26099914
    // The vector variant of nextafter causes an LLVM crash due to a known
    // issue that has since been fixed upstream.  Enable the test after the fix
    // is pulled into AOSP.
    // TEST_HN_FUNC_HN_HN(nextafter);
    TEST_HN_FUNC_HN_HN(pow);
    TEST_HN_FUNC_HN_IN(pown);
    TEST_HN_FUNC_HN_HN(powr);

    TEST_HN_FUNC_HN(radians);
    TEST_HN_FUNC_HN_HN(remainder);
    TEST_HN_FUNC_HN_HN_PIN(remquo);
    TEST_HN_FUNC_HN(rint);
    TEST_HN_FUNC_HN_IN(rootn);
    TEST_HN_FUNC_HN(round);
    TEST_HN_FUNC_HN(rsqrt);

    TEST_HN_FUNC_HN(sign);
    TEST_HN_FUNC_HN(sin);
    TEST_HN_FUNC_HN_PHN(sincos);
    TEST_HN_FUNC_HN(sinh);
    TEST_HN_FUNC_HN(sinpi);
    TEST_HN_FUNC_HN(sqrt);

    // Bug: https://b.corp.google.com/issues/26099914
    // Some variants of the step functions cause LLVM crash due to a known
    // issue that has since been fixed upstream.  Enable the test after the fix
    // is pulled into AOSP.
    // TEST_HN_FUNC_HN_HN(step);
    // TEST_HN_FUNC_HN_H(step);
    // TEST_HN_FUNC_H_HN(step);

    TEST_HN_FUNC_HN(tan);
    TEST_HN_FUNC_HN(tanh);
    TEST_HN_FUNC_HN(tanpi);

    TEST_HN_FUNC_HN(tgamma);
    TEST_HN_FUNC_HN(trunc);

    // Vector math functions
    h3 = cross(h3, h3);
    h4 = cross(h4, h4);
    return true;
}

// TODO enable nextafter and ilogb tests once added to API
/*
static bool testNextAfter() {
    // Number of non-NaN fp16 values:
    //     2^11 - 2 (= 2046) different NaNs (10 mantisa bits, minus 1 for all
    //     zero, times two for the signs.
    //
    //     +0 and -0 are equivalent (i.e. nextafter(-0, +inf) skips +0 and
    //     returns the minium positive subnormal.
    //
    // So, number of distinct values when calling nextafter in a loop is
    //     2^16 - 2046 - 1 = 63489
    //

    const unsigned int numDistinctExpected = 63489;
    const unsigned int maxSteps = 65536;

    half posinf, neginf;
    (*(short *) &posinf) = 0x7c00;
    (*(short *) &neginf) = 0xfc00;

    unsigned int numDistinct;
    half h, toward;

    h = neginf;
    toward = posinf;
    numDistinct = 1; // h is the first distinct value
    while (numDistinct < maxSteps && h != toward) {
        h = nextafter(h, toward);
        numDistinct ++;
    }
    // TODO we are not expected to handle subnormals.  This test needs to be
    // revised to test only the normal values in f16.
    if (numDistinct != numDistinctExpected) {
        return false;
    }

    h = posinf;
    toward = neginf;
    numDistinct = 1; // h is the first distinct value
    while (numDistinct < maxSteps && h != toward) {
        h = nextafter(h, toward);
        numDistinct ++;
    }
    if (numDistinct != numDistinctExpected) {
        return false;
    }
    return true;
}

static bool testIlogb() {
    bool failed = false;

    // ilogb(2^n) = n.  Test at the boundary on either side of 2^n.
    _RS_ASSERT_EQU(ilogb((half) 0.24), -3);
    _RS_ASSERT_EQU(ilogb((half) 0.26), -2);
    _RS_ASSERT_EQU(ilogb((half) 0.49), -2);
    _RS_ASSERT_EQU(ilogb((half) 0.51), -1);
    _RS_ASSERT_EQU(ilogb((half) 0.99), -1);
    _RS_ASSERT_EQU(ilogb((half) 1.01), 0);
    _RS_ASSERT_EQU(ilogb((half) 1.99), 0);
    _RS_ASSERT_EQU(ilogb((half) 2.01), 1);
    // _RS_ASSERT_EQU(ilogb((half) 0.00000005960), -24); // min subnormal = 2^-24

    // Result is same irrespective of sign.
    _RS_ASSERT_EQU(ilogb((half) -0.24), -3);
    _RS_ASSERT_EQU(ilogb((half) -0.26), -2);
    _RS_ASSERT_EQU(ilogb((half) -0.49), -2);
    _RS_ASSERT_EQU(ilogb((half) -0.51), -1);
    _RS_ASSERT_EQU(ilogb((half) -0.99), -1);
    _RS_ASSERT_EQU(ilogb((half) -1.01), 0);
    _RS_ASSERT_EQU(ilogb((half) -1.99), 0);
    _RS_ASSERT_EQU(ilogb((half) -2.01), 1);
    // _RS_ASSERT_EQU(ilogb((half) -0.00000005960), -24);

    return !failed;
}
*/

void testFp16Math() {
    bool success = true;

    success &= testAPI();
    // TODO enable nextafter and ilogb tests once added to API
    /*
    success &= testNextAfter();
    success &= testIlogb();
    */

    if (success) {
        rsDebug("PASSED", 0);
    } else {
        rsDebug("FAILED", 0);
    }

    if (success) {
        rsSendToClientBlocking(RS_MSG_TEST_PASSED);
    } else {
        rsSendToClientBlocking(RS_MSG_TEST_FAILED);
    }
}
