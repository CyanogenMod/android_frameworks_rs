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

#include "rsdIntrinsicInlines.h"

using namespace android;
using namespace android::renderscript;

struct ConvolveParams {
    float f[4];
};


static void One(const RsForEachStubParamStruct *p, uchar4 *out,
                const uchar4 *py, const float* coeff) {
    float4 i = convert_float4(py[0]);

    float4 sum;
    sum.x = i.x * coeff[0] +
            i.y * coeff[4] +
            i.z * coeff[8] +
            i.w * coeff[12];
    sum.y = i.x * coeff[1] +
            i.y * coeff[5] +
            i.z * coeff[9] +
            i.w * coeff[13];
    sum.z = i.x * coeff[2] +
            i.y * coeff[6] +
            i.z * coeff[10] +
            i.w * coeff[14];
    sum.w = i.x * coeff[3] +
            i.y * coeff[7] +
            i.z * coeff[11] +
            i.w * coeff[15];

    sum.x = sum.x < 0 ? 0 : (sum.x > 255 ? 255 : sum.x);
    sum.y = sum.y < 0 ? 0 : (sum.y > 255 ? 255 : sum.y);
    sum.z = sum.z < 0 ? 0 : (sum.z > 255 ? 255 : sum.z);
    sum.w = sum.w < 0 ? 0 : (sum.w > 255 ? 255 : sum.w);

    *out = convert_uchar4(sum);
}

enum {
    BLEND_CLEAR = 0,
    BLEND_SRC = 1,
    BLEND_DST = 2,
    BLEND_SRC_OVER = 3,
    BLEND_DST_OVER = 4,
    BLEND_SRC_IN = 5,
    BLEND_DST_IN = 6,
    BLEND_SRC_OUT = 7,
    BLEND_DST_OUT = 8,
    BLEND_SRC_ATOP = 9,
    BLEND_DST_ATOP = 10,
    BLEND_XOR = 11,

    BLEND_NORMAL = 12,
    BLEND_AVERAGE = 13,
    BLEND_MULTIPLY = 14,
    BLEND_SCREEN = 15,
    BLEND_DARKEN = 16,
    BLEND_LIGHTEN = 17,
    BLEND_OVERLAY = 18,
    BLEND_HARDLIGHT = 19,
    BLEND_SOFTLIGHT = 20,
    BLEND_DIFFERENCE = 21,
    BLEND_NEGATION = 22,
    BLEND_EXCLUSION = 23,
    BLEND_COLOR_DODGE = 24,
    BLEND_INVERSE_COLOR_DODGE = 25,
    BLEND_SOFT_DODGE = 26,
    BLEND_COLOR_BURN = 27,
    BLEND_INVERSE_COLOR_BURN = 28,
    BLEND_SOFT_BURN = 29,
    BLEND_REFLECT = 30,
    BLEND_GLOW = 31,
    BLEND_FREEZE = 32,
    BLEND_HEAT = 33,
    BLEND_ADD = 34,
    BLEND_SUBTRACT = 35,
    BLEND_STAMP = 36,
    BLEND_RED = 37,
    BLEND_GREEN = 38,
    BLEND_BLUE = 39,
    BLEND_HUE = 40,
    BLEND_SATURATION = 41,
    BLEND_COLOR = 42,
    BLEND_LUMINOSITY = 43
};

static void ColorMatrix_uchar4(const RsForEachStubParamStruct *p,
                               uint32_t xstart, uint32_t xend,
                               uint32_t instep, uint32_t outstep) {
    ConvolveParams *cp = (ConvolveParams *)p->usr;
    uchar4 *out = (uchar4 *)p->out;
    uchar4 *in = (uchar4 *)p->in;
    uint32_t x1 = xstart;
    uint32_t x2 = xend;

    in += xstart;
    out += xstart;

    switch (p->slot) {
    case BLEND_CLEAR:
        for (;x1 < x2; x1++, out++) {
            *out = 0;
        }
        break;
    case BLEND_SRC:
        for (;x1 < x2; x1++, out++, in++) {
            uchar4 t = *in;
            t.rgb = (t.rgb * t.a) >> 8;
            *out = t;
        }
        break;
    case BLEND_DST:
        for (;x1 < x2; x1++, out++, in++) {
            uchar4 t = *in;
            t.rgb = (t.rgb * t.a) >> 8;
            *out = t;
        }
        break;




    }

    if(x2 > x1) {
        while(x1 != x2) {
            One(p, out++, in++, cp->f);
            x1++;
        }
    }
}

void * rsdIntrinsic_InitBlend(const android::renderscript::Context *dc,
                              android::renderscript::Script *script,
                              RsdIntriniscFuncs_t *funcs) {

    script->mHal.info.exportedVariableCount = 0;
    funcs->root = ColorMatrix_uchar4;

    ConvolveParams *cp = (ConvolveParams *)calloc(1, sizeof(ConvolveParams));
    return cp;
}


