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

enum IntrinsicEnums {
    INTRINSIC_UNDEFINED,
    INTRINSIC_CONVOLVE_3x3,
    INTRINXIC_COLORMATRIX

};

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

static inline int4 convert_int4(uchar4 i) {
    int4 f4 = {i.x, i.y, i.z, i.w};
    return f4;
}

static inline short4 convert_short4(uchar4 i) {
    short4 f4 = {i.x, i.y, i.z, i.w};
    return f4;
}

static inline float4 convert_float4(uchar4 i) {
    float4 f4 = {i.x, i.y, i.z, i.w};
    return f4;
}

static inline uchar4 convert_uchar4(int4 i) {
    uchar4 f4 = {(uchar)i.x, (uchar)i.y, (uchar)i.z, (uchar)i.w};
    return f4;
}


static inline int4 clamp(int4 amount, int low, int high) {
    int4 r;
    r.x = amount.x < low ? low : (amount.x > high ? high : amount.x);
    r.y = amount.y < low ? low : (amount.y > high ? high : amount.y);
    r.z = amount.z < low ? low : (amount.z > high ? high : amount.z);
    r.w = amount.w < low ? low : (amount.w > high ? high : amount.w);
    return r;
}

static float params[9] = { 0.f,  -1.f,  0.f,
                          -1.f,   5.f, -1.f,
                           0.f,  -1.f,  0.f };
static short iparams[16] = { 0,      -255,   0,
                          -255,   1275,   -255,
                          0,     -255,    0 };

static short imatrix[16] = { 255,    0,   50,    0,
                               0,  200,   50,    0,
                               0,    0,  255,    0,
                               0,    0,    0,  255 };

//        m.set(1, 0, 0.2f);
        //m.set(1, 1, 0.9f);
        //m./set(1, 2, 0.2f);


//static short iparams[16] = { 0,0,0, 0,255,0, 0,0,0};

ObjectBaseRef<Allocation> gAlloc;

static void Convolve3x3_Bind(const Context *dc, const Script *script,
                             void * intrinsicData, uint32_t slot, Allocation *data) {

    //ALOGE("bind %p", data);
    rsAssert(slot == 1);
    gAlloc.set(data);
}

static void Convolve3x3_SetVar(const Context *dc, const Script *script, void * intrinsicData,
                               uint32_t slot, void *data, size_t dataLength) {

    rsAssert(slot == 0);
    memcpy (params, data, dataLength);
    for(int ct=0; ct < 9; ct++) {
        iparams[ct] = (int)(params[ct] * 255.f);
    }
}

extern "C" void TestConvolveK(void *dst, const void *y0, const void *y1, const void *y2, const short *coef, uint32_t count);
extern "C" void rsdIntrinsicColorMatrix4x4K(void *dst, const void *src, const short *coef, uint32_t count);
extern "C" void rsdIntrinsicColorMatrix3x3K(void *dst, const void *src, const short *coef, uint32_t count);


static void ConvolveOne(const RsForEachStubParamStruct *p, uint32_t x, uchar4 *out,
                        const uchar4 *py0, const uchar4 *py1, const uchar4 *py2,
                        const float* coeff) {

    uint32_t x1 = rsMin((int32_t)x+1, (int32_t)p->dimX);
    uint32_t x2 = rsMax((int32_t)x-1, 0);

    float4 px = convert_float4(py0[x1]) * coeff[0] +
                convert_float4(py0[x]) * coeff[1] +
                convert_float4(py0[x2]) * coeff[2] +
                convert_float4(py1[x1]) * coeff[3] +
                convert_float4(py1[x]) * coeff[4] +
                convert_float4(py1[x2]) * coeff[5] +
                convert_float4(py2[x1]) * coeff[6] +
                convert_float4(py2[x]) * coeff[7] +
                convert_float4(py2[x2]) * coeff[8];

    //px = clamp(px, 0.f, 255.f);
    px.x = px.x < 0 ? 0 : (px.x > 255 ? 255 : px.x);
    px.y = px.y < 0 ? 0 : (px.y > 255 ? 255 : px.y);
    px.z = px.z < 0 ? 0 : (px.z > 255 ? 255 : px.z);
    px.w = px.w < 0 ? 0 : (px.w > 255 ? 255 : px.w);

    uchar4 o = {(uchar)px.x, (uchar)px.y, (uchar)px.z, (uchar)px.w};
    *out = o;
}

static void Convolve3x3_uchar4(const RsForEachStubParamStruct *p,
                                    uint32_t xstart, uint32_t xend,
                                    uint32_t instep, uint32_t outstep) {

//    ALOGE("Convolve3x3_uchar4 y = %i  dim=%i", p->y, p->dimY);

    DrvAllocation *din = (DrvAllocation *)gAlloc->mHal.drv;
    const uchar *pin = (const uchar *)din->lod[0].mallocPtr;

    uint32_t y1 = rsMin((int32_t)p->y + 1, (int32_t)(p->dimY-1));
    uint32_t y2 = rsMax((int32_t)p->y - 1, 0);
    const uchar4 *py0 = (const uchar4 *)(pin + din->lod[0].stride * y2);
    const uchar4 *py1 = (const uchar4 *)(pin + din->lod[0].stride * p->y);
    const uchar4 *py2 = (const uchar4 *)(pin + din->lod[0].stride * y1);

    uchar4 *out = (uchar4 *)p->out;
    uint32_t x1 = xstart;
    uint32_t x2 = xend;
    if(x1 == 0) {
        ConvolveOne(p, 0, out, py0, py1, py2, params);
        x1 ++;
        out++;
    }

    if(x2 > x1) {
        int32_t len = (x2 - x1 - 1) >> 1;
        if(len > 0) {
            TestConvolveK(out, &py0[x1], &py1[x1], &py2[x1], &iparams[0], len);
            x1 += len << 1;
            out += len << 1;
        }

        while(x1 != x2) {
            ConvolveOne(p, x1, out, py0, py1, py2, params);
            out++;
            x1++;
        }
    }

    rsdIntrinsicColorMatrix4x4K((uchar4 *)p->out, (uchar4 *)p->out, &imatrix[0], (xend - xstart)>>2);


/*
    for(uint32_t x = xstart; x < xend; x+=2) {
        //uint32_t x1 = rsMin((int32_t)x+1, (int32_t)p->dimX);
        //uint32_t x2 = rsMax((int32_t)x-1, 0);

        //int4 p00 = convert_int4(py0[x1]);
        //int4 p01 = convert_int4(py0[x]);
        //int4 p02 = convert_int4(py0[x2]);
        //int4 p10 = convert_int4(py1[x1]);
        int4 p11 = convert_int4(py1[x]);
        //int4 p12 = convert_int4(py1[x2]);
        //int4 p20 = convert_int4(py2[x1]);
        //int4 p21 = convert_int4(py2[x]);
        //int4 p22 = convert_int4(py2[x2]);

//        ALOGE("rgbx %i %i %i", p11.x, p11.y, p11.z);

        int4 px = //p00 * (int4)(iparams[0]) +
                  //p01 * (int4)iparams[1] +
                  //p02 * (int4)iparams[2] +
                  //p10 * (int4)iparams[3] +
                  p11 * (int4)(iparams[4]) ;
                  //p12 * (int4)iparams[5] +
                  //p20 * (int4)iparams[6] +
                  //p21 * (int4)iparams[7] +
                  //p22 * (int4)iparams[8];

        //ALOGE("ip %i %i %i  %i %i %i   %i %i %i", iparams[0], iparams[1], iparams[2], iparams[3], iparams[4], iparams[5], iparams[6], iparams[7], iparams[8]);

  //      ALOGE("rgb %i %i %i", px.x, px.y, px.z);
        int4 shift = 12;
        px >>= shift;

    //    ALOGE("rgb %i %i %i", px.x, px.y, px.z);
        //p20 = clamp(p20, 0, 255);

        *out = convert_uchar4(px);
        out++;
    }
*/

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



