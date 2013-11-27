#include "shared.rsh"

uint32_t x;
uint32_t y;
uint32_t z;

rs_element elemNull;
rs_element elemNonNull;
rs_type typeNull;
rs_type typeNonNull;
rs_allocation allocNull;
rs_allocation allocNonNull;
rs_sampler samplerNull;
rs_sampler samplerNonNull;
rs_script scriptNull;
rs_script scriptNonNull;

char *allocPtr;
rs_allocation allocDst;

volatile char c;
volatile char2 c2;
volatile char3 c3;
volatile char4 c4;
volatile uchar uc;
volatile uchar2 uc2;
volatile uchar3 uc3;
volatile uchar4 uc4;
volatile short s;
volatile short2 s2;
volatile short3 s3;
volatile short4 s4;
volatile ushort us;
volatile ushort2 us2;
volatile ushort3 us3;
volatile ushort4 us4;
volatile int i;
volatile int2 i2;
volatile int3 i3;
volatile int4 i4;
volatile uint ui;
volatile uint2 ui2;
volatile uint3 ui3;
volatile uint4 ui4;
volatile long l;
volatile long2 l2;
volatile long3 l3;
volatile long4 l4;
volatile ulong ul;
volatile ulong2 ul2;
volatile ulong3 ul3;
volatile ulong4 ul4;
volatile float f;
volatile float2 f2;
volatile float3 f3;
volatile float4 f4;
volatile double d;
volatile double2 d2;
volatile double3 d3;
volatile double4 d4;

rs_allocation aChar;
rs_allocation aChar2;
rs_allocation aChar3;
rs_allocation aChar4;
rs_allocation aUChar;
rs_allocation aUChar2;
rs_allocation aUChar3;
rs_allocation aUChar4;
rs_allocation aShort;
rs_allocation aShort2;
rs_allocation aShort3;
rs_allocation aShort4;
rs_allocation aUShort;
rs_allocation aUShort2;
rs_allocation aUShort3;
rs_allocation aUShort4;
rs_allocation aInt;
rs_allocation aInt2;
rs_allocation aInt3;
rs_allocation aInt4;
rs_allocation aUInt;
rs_allocation aUInt2;
rs_allocation aUInt3;
rs_allocation aUInt4;
rs_allocation aLong;
rs_allocation aLong2;
rs_allocation aLong3;
rs_allocation aLong4;
rs_allocation aULong;
rs_allocation aULong2;
rs_allocation aULong3;
rs_allocation aULong4;
rs_allocation aFloat;
rs_allocation aFloat2;
rs_allocation aFloat3;
rs_allocation aFloat4;
rs_allocation aDouble;
rs_allocation aDouble2;
rs_allocation aDouble3;
rs_allocation aDouble4;

// This function just checks that all of the called functions are
// able to be linked. It is not intended to be executed!
void check_api_presence() {
    /********************************
     * DO NOT EXECUTE THIS FUNCTION *
     ********************************/
    rsSendToClientBlocking(RS_MSG_TEST_FAILED);

    c = rsGetElementAt_char(aChar, 0);
    rsSetElementAt_char(aChar, c, 0);
    c = rsGetElementAt_char(aChar, 0, 0);
    rsSetElementAt_char(aChar, c, 0, 0);
    c = rsGetElementAt_char(aChar, 0, 0, 0);
    rsSetElementAt_char(aChar, c, 0, 0, 0);
    c2 = rsGetElementAt_char2(aChar2, 0);
    rsSetElementAt_char2(aChar2, c2, 0);
    c2 = rsGetElementAt_char2(aChar2, 0, 0);
    rsSetElementAt_char2(aChar2, c2, 0, 0);
    c2 = rsGetElementAt_char2(aChar2, 0, 0, 0);
    rsSetElementAt_char2(aChar2, c2, 0, 0, 0);
    c3 = rsGetElementAt_char3(aChar3, 0);
    rsSetElementAt_char3(aChar3, c3, 0);
    c3 = rsGetElementAt_char3(aChar3, 0, 0);
    rsSetElementAt_char3(aChar3, c3, 0, 0);
    c3 = rsGetElementAt_char3(aChar3, 0, 0, 0);
    rsSetElementAt_char3(aChar3, c3, 0, 0, 0);
    c4 = rsGetElementAt_char4(aChar4, 0);
    rsSetElementAt_char4(aChar4, c4, 0);
    c4 = rsGetElementAt_char4(aChar4, 0, 0);
    rsSetElementAt_char4(aChar4, c4, 0, 0);
    c4 = rsGetElementAt_char4(aChar4, 0, 0, 0);
    rsSetElementAt_char4(aChar4, c4, 0, 0, 0);

    uc = rsGetElementAt_uchar(aUChar, 0);
    rsSetElementAt_uchar(aUChar, uc, 0);
    uc = rsGetElementAt_uchar(aUChar, 0, 0);
    rsSetElementAt_uchar(aUChar, uc, 0, 0);
    uc = rsGetElementAt_uchar(aUChar, 0, 0, 0);
    rsSetElementAt_uchar(aUChar, uc, 0, 0, 0);
    uc2 = rsGetElementAt_uchar2(aUChar2, 0);
    rsSetElementAt_uchar2(aUChar2, uc2, 0);
    uc2 = rsGetElementAt_uchar2(aUChar2, 0, 0);
    rsSetElementAt_uchar2(aUChar2, uc2, 0, 0);
    uc2 = rsGetElementAt_uchar2(aUChar2, 0, 0, 0);
    rsSetElementAt_uchar2(aUChar2, uc2, 0, 0, 0);
    uc3 = rsGetElementAt_uchar3(aUChar3, 0);
    rsSetElementAt_uchar3(aUChar3, uc3, 0);
    uc3 = rsGetElementAt_uchar3(aUChar3, 0, 0);
    rsSetElementAt_uchar3(aUChar3, uc3, 0, 0);
    uc3 = rsGetElementAt_uchar3(aUChar3, 0, 0, 0);
    rsSetElementAt_uchar3(aUChar3, uc3, 0, 0, 0);
    uc4 = rsGetElementAt_uchar4(aUChar4, 0);
    rsSetElementAt_uchar4(aUChar4, uc4, 0);
    uc4 = rsGetElementAt_uchar4(aUChar4, 0, 0);
    rsSetElementAt_uchar4(aUChar4, uc4, 0, 0);
    uc4 = rsGetElementAt_uchar4(aUChar4, 0, 0, 0);
    rsSetElementAt_uchar4(aUChar4, uc4, 0, 0, 0);

    s = rsGetElementAt_short(aShort, 0);
    rsSetElementAt_short(aShort, s, 0);
    s = rsGetElementAt_short(aShort, 0, 0);
    rsSetElementAt_short(aShort, s, 0, 0);
    s = rsGetElementAt_short(aShort, 0, 0, 0);
    rsSetElementAt_short(aShort, s, 0, 0, 0);
    s2 = rsGetElementAt_short2(aShort2, 0);
    rsSetElementAt_short2(aShort2, s2, 0);
    s2 = rsGetElementAt_short2(aShort2, 0, 0);
    rsSetElementAt_short2(aShort2, s2, 0, 0);
    s2 = rsGetElementAt_short2(aShort2, 0, 0, 0);
    rsSetElementAt_short2(aShort2, s2, 0, 0, 0);
    s3 = rsGetElementAt_short3(aShort3, 0);
    rsSetElementAt_short3(aShort3, s3, 0);
    s3 = rsGetElementAt_short3(aShort3, 0, 0);
    rsSetElementAt_short3(aShort3, s3, 0, 0);
    s3 = rsGetElementAt_short3(aShort3, 0, 0, 0);
    rsSetElementAt_short3(aShort3, s3, 0, 0, 0);
    s4 = rsGetElementAt_short4(aShort4, 0);
    rsSetElementAt_short4(aShort4, s4, 0);
    s4 = rsGetElementAt_short4(aShort4, 0, 0);
    rsSetElementAt_short4(aShort4, s4, 0, 0);
    s4 = rsGetElementAt_short4(aShort4, 0, 0, 0);
    rsSetElementAt_short4(aShort4, s4, 0, 0, 0);

    us = rsGetElementAt_ushort(aUShort, 0);
    rsSetElementAt_ushort(aUShort, us, 0);
    us = rsGetElementAt_ushort(aUShort, 0, 0);
    rsSetElementAt_ushort(aUShort, us, 0, 0);
    us = rsGetElementAt_ushort(aUShort, 0, 0, 0);
    rsSetElementAt_ushort(aUShort, us, 0, 0, 0);
    us2 = rsGetElementAt_ushort2(aUShort2, 0);
    rsSetElementAt_ushort2(aUShort2, us2, 0);
    us2 = rsGetElementAt_ushort2(aUShort2, 0, 0);
    rsSetElementAt_ushort2(aUShort2, us2, 0, 0);
    us2 = rsGetElementAt_ushort2(aUShort2, 0, 0, 0);
    rsSetElementAt_ushort2(aUShort2, us2, 0, 0, 0);
    us3 = rsGetElementAt_ushort3(aUShort3, 0);
    rsSetElementAt_ushort3(aUShort3, us3, 0);
    us3 = rsGetElementAt_ushort3(aUShort3, 0, 0);
    rsSetElementAt_ushort3(aUShort3, us3, 0, 0);
    us3 = rsGetElementAt_ushort3(aUShort3, 0, 0, 0);
    rsSetElementAt_ushort3(aUShort3, us3, 0, 0, 0);
    us4 = rsGetElementAt_ushort4(aUShort4, 0);
    rsSetElementAt_ushort4(aUShort4, us4, 0);
    us4 = rsGetElementAt_ushort4(aUShort4, 0, 0);
    rsSetElementAt_ushort4(aUShort4, us4, 0, 0);
    us4 = rsGetElementAt_ushort4(aUShort4, 0, 0, 0);
    rsSetElementAt_ushort4(aUShort4, us4, 0, 0, 0);

    i = rsGetElementAt_int(aInt, 0);
    rsSetElementAt_int(aInt, i, 0);
    i = rsGetElementAt_int(aInt, 0, 0);
    rsSetElementAt_int(aInt, i, 0, 0);
    i = rsGetElementAt_int(aInt, 0, 0, 0);
    rsSetElementAt_int(aInt, i, 0, 0, 0);
    i2 = rsGetElementAt_int2(aInt2, 0);
    rsSetElementAt_int2(aInt2, i2, 0);
    i2 = rsGetElementAt_int2(aInt2, 0, 0);
    rsSetElementAt_int2(aInt2, i2, 0, 0);
    i2 = rsGetElementAt_int2(aInt2, 0, 0, 0);
    rsSetElementAt_int2(aInt2, i2, 0, 0, 0);
    i3 = rsGetElementAt_int3(aInt3, 0);
    rsSetElementAt_int3(aInt3, i3, 0);
    i3 = rsGetElementAt_int3(aInt3, 0, 0);
    rsSetElementAt_int3(aInt3, i3, 0, 0);
    i3 = rsGetElementAt_int3(aInt3, 0, 0, 0);
    rsSetElementAt_int3(aInt3, i3, 0, 0, 0);
    i4 = rsGetElementAt_int4(aInt4, 0);
    rsSetElementAt_int4(aInt4, i4, 0);
    i4 = rsGetElementAt_int4(aInt4, 0, 0);
    rsSetElementAt_int4(aInt4, i4, 0, 0);
    i4 = rsGetElementAt_int4(aInt4, 0, 0, 0);
    rsSetElementAt_int4(aInt4, i4, 0, 0, 0);

    ui = rsGetElementAt_uint(aUInt, 0);
    rsSetElementAt_uint(aUInt, ui, 0);
    ui = rsGetElementAt_uint(aUInt, 0, 0);
    rsSetElementAt_uint(aUInt, ui, 0, 0);
    ui = rsGetElementAt_uint(aUInt, 0, 0, 0);
    rsSetElementAt_uint(aUInt, ui, 0, 0, 0);
    ui2 = rsGetElementAt_uint2(aUInt2, 0);
    rsSetElementAt_uint2(aUInt2, ui2, 0);
    ui2 = rsGetElementAt_uint2(aUInt2, 0, 0);
    rsSetElementAt_uint2(aUInt2, ui2, 0, 0);
    ui2 = rsGetElementAt_uint2(aUInt2, 0, 0, 0);
    rsSetElementAt_uint2(aUInt2, ui2, 0, 0, 0);
    ui3 = rsGetElementAt_uint3(aUInt3, 0);
    rsSetElementAt_uint3(aUInt3, ui3, 0);
    ui3 = rsGetElementAt_uint3(aUInt3, 0, 0);
    rsSetElementAt_uint3(aUInt3, ui3, 0, 0);
    ui3 = rsGetElementAt_uint3(aUInt3, 0, 0, 0);
    rsSetElementAt_uint3(aUInt3, ui3, 0, 0, 0);
    ui4 = rsGetElementAt_uint4(aUInt4, 0);
    rsSetElementAt_uint4(aUInt4, ui4, 0);
    ui4 = rsGetElementAt_uint4(aUInt4, 0, 0);
    rsSetElementAt_uint4(aUInt4, ui4, 0, 0);
    ui4 = rsGetElementAt_uint4(aUInt4, 0, 0, 0);
    rsSetElementAt_uint4(aUInt4, ui4, 0, 0, 0);

    l = rsGetElementAt_long(aLong, 0);
    rsSetElementAt_long(aLong, l, 0);
    l = rsGetElementAt_long(aLong, 0, 0);
    rsSetElementAt_long(aLong, l, 0, 0);
    l = rsGetElementAt_long(aLong, 0, 0, 0);
    rsSetElementAt_long(aLong, l, 0, 0, 0);
    l2 = rsGetElementAt_long2(aLong2, 0);
    rsSetElementAt_long2(aLong2, l2, 0);
    l2 = rsGetElementAt_long2(aLong2, 0, 0);
    rsSetElementAt_long2(aLong2, l2, 0, 0);
    l2 = rsGetElementAt_long2(aLong2, 0, 0, 0);
    rsSetElementAt_long2(aLong2, l2, 0, 0, 0);
    l3 = rsGetElementAt_long3(aLong3, 0);
    rsSetElementAt_long3(aLong3, l3, 0);
    l3 = rsGetElementAt_long3(aLong3, 0, 0);
    rsSetElementAt_long3(aLong3, l3, 0, 0);
    l3 = rsGetElementAt_long3(aLong3, 0, 0, 0);
    rsSetElementAt_long3(aLong3, l3, 0, 0, 0);
    l4 = rsGetElementAt_long4(aLong4, 0);
    rsSetElementAt_long4(aLong4, l4, 0);
    l4 = rsGetElementAt_long4(aLong4, 0, 0);
    rsSetElementAt_long4(aLong4, l4, 0, 0);
    l4 = rsGetElementAt_long4(aLong4, 0, 0, 0);
    rsSetElementAt_long4(aLong4, l4, 0, 0, 0);

    ul = rsGetElementAt_ulong(aULong, 0);
    rsSetElementAt_ulong(aULong, ul, 0);
    ul = rsGetElementAt_ulong(aULong, 0, 0);
    rsSetElementAt_ulong(aULong, ul, 0, 0);
    ul = rsGetElementAt_ulong(aULong, 0, 0, 0);
    rsSetElementAt_ulong(aULong, ul, 0, 0, 0);
    ul2 = rsGetElementAt_ulong2(aULong2, 0);
    rsSetElementAt_ulong2(aULong2, ul2, 0);
    ul2 = rsGetElementAt_ulong2(aULong2, 0, 0);
    rsSetElementAt_ulong2(aULong2, ul2, 0, 0);
    ul2 = rsGetElementAt_ulong2(aULong2, 0, 0, 0);
    rsSetElementAt_ulong2(aULong2, ul2, 0, 0, 0);
    ul3 = rsGetElementAt_ulong3(aULong3, 0);
    rsSetElementAt_ulong3(aULong3, ul3, 0);
    ul3 = rsGetElementAt_ulong3(aULong3, 0, 0);
    rsSetElementAt_ulong3(aULong3, ul3, 0, 0);
    ul3 = rsGetElementAt_ulong3(aULong3, 0, 0, 0);
    rsSetElementAt_ulong3(aULong3, ul3, 0, 0, 0);
    ul4 = rsGetElementAt_ulong4(aULong4, 0);
    rsSetElementAt_ulong4(aULong4, ul4, 0);
    ul4 = rsGetElementAt_ulong4(aULong4, 0, 0);
    rsSetElementAt_ulong4(aULong4, ul4, 0, 0);
    ul4 = rsGetElementAt_ulong4(aULong4, 0, 0, 0);
    rsSetElementAt_ulong4(aULong4, ul4, 0, 0, 0);

    f = rsGetElementAt_float(aFloat, 0);
    rsSetElementAt_float(aFloat, f, 0);
    f = rsGetElementAt_float(aFloat, 0, 0);
    rsSetElementAt_float(aFloat, f, 0, 0);
    f = rsGetElementAt_float(aFloat, 0, 0, 0);
    rsSetElementAt_float(aFloat, f, 0, 0, 0);
    f2 = rsGetElementAt_float2(aFloat2, 0);
    rsSetElementAt_float2(aFloat2, f2, 0);
    f2 = rsGetElementAt_float2(aFloat2, 0, 0);
    rsSetElementAt_float2(aFloat2, f2, 0, 0);
    f2 = rsGetElementAt_float2(aFloat2, 0, 0, 0);
    rsSetElementAt_float2(aFloat2, f2, 0, 0, 0);
    f3 = rsGetElementAt_float3(aFloat3, 0);
    rsSetElementAt_float3(aFloat3, f3, 0);
    f3 = rsGetElementAt_float3(aFloat3, 0, 0);
    rsSetElementAt_float3(aFloat3, f3, 0, 0);
    f3 = rsGetElementAt_float3(aFloat3, 0, 0, 0);
    rsSetElementAt_float3(aFloat3, f3, 0, 0, 0);
    f4 = rsGetElementAt_float4(aFloat4, 0);
    rsSetElementAt_float4(aFloat4, f4, 0);
    f4 = rsGetElementAt_float4(aFloat4, 0, 0);
    rsSetElementAt_float4(aFloat4, f4, 0, 0);
    f4 = rsGetElementAt_float4(aFloat4, 0, 0, 0);
    rsSetElementAt_float4(aFloat4, f4, 0, 0, 0);

    d = rsGetElementAt_double(aDouble, 0);
    rsSetElementAt_double(aDouble, d, 0);
    d = rsGetElementAt_double(aDouble, 0, 0);
    rsSetElementAt_double(aDouble, d, 0, 0);
    d = rsGetElementAt_double(aDouble, 0, 0, 0);
    rsSetElementAt_double(aDouble, d, 0, 0, 0);
    d2 = rsGetElementAt_double2(aDouble2, 0);
    rsSetElementAt_double2(aDouble2, d2, 0);
    d2 = rsGetElementAt_double2(aDouble2, 0, 0);
    rsSetElementAt_double2(aDouble2, d2, 0, 0);
    d2 = rsGetElementAt_double2(aDouble2, 0, 0, 0);
    rsSetElementAt_double2(aDouble2, d2, 0, 0, 0);
    d3 = rsGetElementAt_double3(aDouble3, 0);
    rsSetElementAt_double3(aDouble3, d3, 0);
    d3 = rsGetElementAt_double3(aDouble3, 0, 0);
    rsSetElementAt_double3(aDouble3, d3, 0, 0);
    d3 = rsGetElementAt_double3(aDouble3, 0, 0, 0);
    rsSetElementAt_double3(aDouble3, d3, 0, 0, 0);
    d4 = rsGetElementAt_double4(aDouble4, 0);
    rsSetElementAt_double4(aDouble4, d4, 0);
    d4 = rsGetElementAt_double4(aDouble4, 0, 0);
    rsSetElementAt_double4(aDouble4, d4, 0, 0);
    d4 = rsGetElementAt_double4(aDouble4, 0, 0, 0);
    rsSetElementAt_double4(aDouble4, d4, 0, 0, 0);

    uc3.x = rsGetElementAtYuv_uchar_Y(aUChar4, 0, 0);
    uc3.y = rsGetElementAtYuv_uchar_U(aUChar4, 0, 0);
    uc3.z = rsGetElementAtYuv_uchar_V(aUChar4, 0, 0);

    c3.x = *(char*)rsGetElementAt(aChar3, 0);
    c3.y = *(char*)rsGetElementAt(aChar3, 0, 0);
    c3.z = *(char*)rsGetElementAt(aChar3, 0, 0, 0);

    rsAllocationIoSend(aInt);
    rsAllocationIoReceive(aInt);

    elemNonNull = rsAllocationGetElement(aInt);

    rsAllocationCopy1DRange(allocDst, 0, 0, 0, allocDst, 0, 0);
    rsAllocationCopy2DRange(allocDst, 0, 0, 0, 0, 0, 0, allocDst, 0, 0, 0, 0);

    // rsSample routines
    f4 += rsSample(allocDst, samplerNonNull, f);
    f4 += rsSample(allocDst, samplerNonNull, f, f);
    f4 += rsSample(allocDst, samplerNonNull, f2);
    f4 += rsSample(allocDst, samplerNonNull, f2, f);
    /********************************
     * DO NOT EXECUTE THIS FUNCTION *
     ********************************/
}

static bool test_obj_api() {
    bool failed = false;

    _RS_ASSERT(!rsIsObject(elemNull));
    _RS_ASSERT(rsIsObject(elemNonNull));
    rsSetObject(&elemNull, elemNonNull);
    _RS_ASSERT(rsIsObject(elemNull));
    rsClearObject(&elemNull);
    _RS_ASSERT(!rsIsObject(elemNull));

    _RS_ASSERT(!rsIsObject(typeNull));
    _RS_ASSERT(rsIsObject(typeNonNull));
    rsSetObject(&typeNull, typeNonNull);
    _RS_ASSERT(rsIsObject(typeNull));
    rsClearObject(&typeNull);
    _RS_ASSERT(!rsIsObject(typeNull));

    _RS_ASSERT(!rsIsObject(allocNull));
    _RS_ASSERT(rsIsObject(allocNonNull));
    rsSetObject(&allocNull, allocNonNull);
    _RS_ASSERT(rsIsObject(allocNull));
    rsClearObject(&allocNull);
    _RS_ASSERT(!rsIsObject(allocNull));

    _RS_ASSERT(!rsIsObject(samplerNull));
    _RS_ASSERT(rsIsObject(samplerNonNull));
    rsSetObject(&samplerNull, samplerNonNull);
    _RS_ASSERT(rsIsObject(samplerNull));
    rsClearObject(&samplerNull);
    _RS_ASSERT(!rsIsObject(samplerNull));

    _RS_ASSERT(!rsIsObject(scriptNull));
    _RS_ASSERT(rsIsObject(scriptNonNull));
    rsSetObject(&scriptNull, scriptNonNull);
    _RS_ASSERT(rsIsObject(scriptNull));
    rsClearObject(&scriptNull);
    _RS_ASSERT(!rsIsObject(scriptNull));

    if (failed) {
        rsDebug("test_obj_api FAILED", -1);
    }
    else {
        rsDebug("test_obj_api PASSED", 0);
    }

    return failed;
}


static bool test_rs_alloc_api() {
    bool failed = false;
    rs_allocation a = rsGetAllocation(allocPtr);
    _RS_ASSERT(rsIsObject(a));
    _RS_ASSERT(rsAllocationGetDimX(a) == x);
    _RS_ASSERT(rsAllocationGetDimY(a) == y);
    _RS_ASSERT(rsAllocationGetDimZ(a) == z);
    _RS_ASSERT(rsAllocationGetDimLOD(a) == 0);
    _RS_ASSERT(rsAllocationGetDimFaces(a) == 0);

    rsSetElementAt_char(a, 5, 1, 0);
    rsAllocationCopy1DRange(allocDst, 0, 0, x, a, 0, 0);
    _RS_ASSERT(rsGetElementAt_char(allocDst, 1, 0) == 5);

    if (failed) {
        rsDebug("test_obj_api FAILED", -1);
    }
    else {
        rsDebug("test_obj_api PASSED", 0);
    }

    return failed;
}


void api_test() {
    bool failed = false;
    failed |= test_obj_api();
    failed |= test_rs_alloc_api();

    if (failed) {
        rsSendToClientBlocking(RS_MSG_TEST_FAILED);
    }
    else {
        rsSendToClientBlocking(RS_MSG_TEST_PASSED);
    }
}

