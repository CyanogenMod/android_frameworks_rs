/*
 * Copyright (C) 2012 The Android Open Source Project
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


#include "rsdCore.h"
#include "rsdIntrinsics.h"
#include "rsdAllocation.h"

typedef uint8_t uchar;
typedef uint16_t ushort;
typedef uint32_t uint;

typedef float float2 __attribute__((ext_vector_type(2)));
typedef float float3 __attribute__((ext_vector_type(3)));
typedef float float4 __attribute__((ext_vector_type(4)));
typedef uchar uchar2 __attribute__((ext_vector_type(2)));
typedef uchar uchar3 __attribute__((ext_vector_type(3)));
typedef uchar uchar4 __attribute__((ext_vector_type(4)));
typedef ushort ushort2 __attribute__((ext_vector_type(2)));
typedef ushort ushort3 __attribute__((ext_vector_type(3)));
typedef ushort ushort4 __attribute__((ext_vector_type(4)));
typedef uint uint2 __attribute__((ext_vector_type(2)));
typedef uint uint3 __attribute__((ext_vector_type(3)));
typedef uint uint4 __attribute__((ext_vector_type(4)));
typedef char char2 __attribute__((ext_vector_type(2)));
typedef char char3 __attribute__((ext_vector_type(3)));
typedef char char4 __attribute__((ext_vector_type(4)));
typedef short short2 __attribute__((ext_vector_type(2)));
typedef short short3 __attribute__((ext_vector_type(3)));
typedef short short4 __attribute__((ext_vector_type(4)));
typedef int int2 __attribute__((ext_vector_type(2)));
typedef int int3 __attribute__((ext_vector_type(3)));
typedef int int4 __attribute__((ext_vector_type(4)));
typedef long long2 __attribute__((ext_vector_type(2)));
typedef long long3 __attribute__((ext_vector_type(3)));
typedef long long4 __attribute__((ext_vector_type(4)));



using namespace android;
using namespace android::renderscript;

/*
typedef struct {
    const void *in;
    void *out;
    const void *usr;
    size_t usr_len;
    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint32_t lod;
    RsAllocationCubemapFace face;
    uint32_t ar[16];
    uint32_t dimX;
    uint32_t dimY;
    uint32_t dimZ;
    size_t strideIn;
    size_t strideOut;
    const void *baseIn;
    void *baseOut;
} RsForEachStubParamStruct;
*/

float4 convert_float4(uchar4 i) {
    float4 f4 = {i.x, i.y, i.z, i.w};
    return f4;
}

uchar4 convert_uchar4(float4 i) {
    uchar4 f4 = {(uchar)i.x, (uchar)i.y, (uchar)i.z, (uchar)i.w};
    return f4;
}

float4 clamp(float4 amount, float low, float high) {
    float4 r;
    r.x = amount.x < low ? low : (amount.x > high ? high : amount.x);
    r.y = amount.y < low ? low : (amount.y > high ? high : amount.y);
    r.z = amount.z < low ? low : (amount.z > high ? high : amount.z);
    r.w = amount.w < low ? low : (amount.w > high ? high : amount.w);
    return r;
}

static float params[9] = { 0.f,  -1.f,  0.f,
                          -1.f,   5.f, -1.f,
                           0.f,  -1.f,  0.f };

ObjectBaseRef<Allocation> gAlloc;

static void Convolve3x3_Bind(const Context *dc, const Script *script,
                             uint32_t slot, Allocation *data) {

    ALOGE("bind %p", data);
    rsAssert(slot == 1);
    gAlloc.set(data);
}

static void Convolve3x3_SetVar(const Context *dc, const Script *script,
                               uint32_t slot, void *data, size_t dataLength) {

    rsAssert(slot == 0);
    memcpy (params, data, dataLength);
}

static void Convolve3x3_uchar4(const RsForEachStubParamStruct *p,
                                    uint32_t xstart, uint32_t xend,
                                    uint32_t instep, uint32_t outstep) {

    DrvAllocation *din = (DrvAllocation *)gAlloc->mHal.drv;
    const uchar *pin = (const uchar *)din->lod[0].mallocPtr;

    uint32_t y1 = rsMin((int32_t)p->y + 1, (int32_t)p->dimY);
    uint32_t y2 = rsMax((int32_t)p->y - 1, 0);
    const uchar4 *py0 = (const uchar4 *)(pin + din->lod[0].stride * y2);
    const uchar4 *py1 = (const uchar4 *)(pin + din->lod[0].stride * p->y);
    const uchar4 *py2 = (const uchar4 *)(pin + din->lod[0].stride * y1);

    uchar4 *out = (uchar4 *)p->out;

    for(uint32_t x = xstart; x < xend; x++) {
        uint32_t x1 = rsMin((int32_t)x+1, (int32_t)p->dimX);
        uint32_t x2 = rsMax((int32_t)x-1, 0);

        float4 p00 = convert_float4(py0[x1]) * params[0];
        float4 p01 = convert_float4(py0[x])  * params[1];
        float4 p02 = convert_float4(py0[x2]) * params[2];
        float4 p10 = convert_float4(py1[x1]) * params[3];
        float4 p11 = convert_float4(py1[x])  * params[4];
        float4 p12 = convert_float4(py1[x2]) * params[5];
        float4 p20 = convert_float4(py2[x1]) * params[6];
        float4 p21 = convert_float4(py2[x])  * params[7];
        float4 p22 = convert_float4(py2[x2]) * params[8];

        p00 += p01;
        p02 += p10;
        p11 += p12;
        p20 += p21;

        p22 += p00;
        p02 += p11;

        p20 += p22;
        p20 += p02;

        p20 = clamp(p20, 0.f, 255.f);
        *out = convert_uchar4(p20);
        out++;
    }

}





bool rsdIntrinsic_Init(const android::renderscript::Context *dc,
                       android::renderscript::Script *script,
                       RsScriptIntrinsicID iid,
                       RsdIntriniscFuncs_t *funcs) {

    script->mHal.info.exportedVariableCount = 2;

    funcs->bind = Convolve3x3_Bind;
    funcs->setVar = Convolve3x3_SetVar;
    funcs->root = Convolve3x3_uchar4;
    return true;
}



