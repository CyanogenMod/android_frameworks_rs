/*
 * Copyright (C) 2011 The Android Open Source Project
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
#include "rsdSampler.h"

#include "rsContext.h"
#include "rsSampler.h"

#ifndef RS_COMPATIBILITY_LIB
#include "rsProgramVertex.h"
#include "rsProgramFragment.h"

#include <GLES/gl.h>
#include <GLES/glext.h>
#endif

typedef float float2 __attribute__((ext_vector_type(2)));
typedef float float3 __attribute__((ext_vector_type(3)));
typedef float float4 __attribute__((ext_vector_type(4)));
typedef uint8_t uchar4 __attribute__((ext_vector_type(4)));

using namespace android;
using namespace android::renderscript;

#if defined(ARCH_ARM_HAVE_VFP)
    #define LOCAL_CALL __attribute__((pcs("aapcs-vfp")))
#else
    #define LOCAL_CALL
#endif

// 565 Conversion bits taken from SkBitmap
#define SK_R16_BITS     5
#define SK_G16_BITS     6
#define SK_B16_BITS     5

#define SK_R16_SHIFT    (SK_B16_BITS + SK_G16_BITS)
#define SK_G16_SHIFT    (SK_B16_BITS)
#define SK_B16_SHIFT    0

#define SK_R16_MASK     ((1 << SK_R16_BITS) - 1)
#define SK_G16_MASK     ((1 << SK_G16_BITS) - 1)
#define SK_B16_MASK     ((1 << SK_B16_BITS) - 1)

static inline unsigned SkR16ToR32(unsigned r) {
    return (r << (8 - SK_R16_BITS)) | (r >> (2 * SK_R16_BITS - 8));
}

static inline unsigned SkG16ToG32(unsigned g) {
    return (g << (8 - SK_G16_BITS)) | (g >> (2 * SK_G16_BITS - 8));
}

static inline unsigned SkB16ToB32(unsigned b) {
    return (b << (8 - SK_B16_BITS)) | (b >> (2 * SK_B16_BITS - 8));
}

#define SkPacked16ToR32(c)      SkR16ToR32(SkGetPackedR16(c))
#define SkPacked16ToG32(c)      SkG16ToG32(SkGetPackedG16(c))
#define SkPacked16ToB32(c)      SkB16ToB32(SkGetPackedB16(c))

#define SkGetPackedR16(color)   (((unsigned)(color) >> SK_R16_SHIFT) & SK_R16_MASK)
#define SkGetPackedG16(color)   (((unsigned)(color) >> SK_G16_SHIFT) & SK_G16_MASK)
#define SkGetPackedB16(color)   (((unsigned)(color) >> SK_B16_SHIFT) & SK_B16_MASK)

static float3 getFrom565(uint16_t color) {
    float3 result;
    result.x = (float)SkPacked16ToR32(color);
    result.y = (float)SkPacked16ToG32(color);
    result.z = (float)SkPacked16ToB32(color);
    return result;
}



/**
* Allocation sampling
*/
static inline float getElementAt1(const uint8_t *p, int32_t x) {
    float r = p[x];
    return r;
}

static inline float2 getElementAt2(const uint8_t *p, int32_t x) {
    x *= 2;
    float2 r = {p[x], p[x+1]};
    return r;
}

static inline float3 getElementAt3(const uint8_t *p, int32_t x) {
    x *= 4;
    float3 r = {p[x], p[x+1], p[x+2]};
    return r;
}

static inline float4 getElementAt4(const uint8_t *p, int32_t x) {
    x *= 4;
    float4 r = {p[x], p[x+1], p[x+2], p[x+3]};
    return r;
}

static inline float3 getElementAt565(const uint8_t *p, int32_t x) {
    x *= 2;
    float3 r = getFrom565(((const uint16_t *)p)[0]);
    return r;
}

static inline float getElementAt1(const uint8_t *p, size_t stride, int32_t x, int32_t y) {
    p += y * stride;
    float r = p[x];
    return r;
}

static inline float2 getElementAt2(const uint8_t *p, size_t stride, int32_t x, int32_t y) {
    p += y * stride;
    x *= 2;
    float2 r = {p[x], p[x+1]};
    return r;
}

static inline float3 getElementAt3(const uint8_t *p, size_t stride, int32_t x, int32_t y) {
    p += y * stride;
    x *= 4;
    float3 r = {p[x], p[x+1], p[x+2]};
    return r;
}

static inline float4 getElementAt4(const uint8_t *p, size_t stride, int32_t x, int32_t y) {
    p += y * stride;
    x *= 4;
    float4 r = {p[x], p[x+1], p[x+2], p[x+3]};
    return r;
}

static inline float3 getElementAt565(const uint8_t *p, size_t stride, int32_t x, int32_t y) {
    p += y * stride;
    x *= 2;
    float3 r = getFrom565(((const uint16_t *)p)[0]);
    return r;
}





static float4 LOCAL_CALL
            getSample1D_A(const uint8_t *p, int32_t iPixel,
                          int32_t next, float w0, float w1) {
    float p0 = getElementAt1(p, iPixel);
    float p1 = getElementAt1(p, next);
    float r = p0 * w0 + p1 * w1;
    r *= (1.f / 255.f);
    float4 ret = {0.f, 0.f, 0.f, r};
    return ret;
}
static float4 LOCAL_CALL
            getSample1D_L(const uint8_t *p, int32_t iPixel,
                          int32_t next, float w0, float w1) {
    float p0 = getElementAt1(p, iPixel);
    float p1 = getElementAt1(p, next);
    float r = p0 * w0 + p1 * w1;
    r *= (1.f / 255.f);
    float4 ret = {r, r, r, 1.f};
    return ret;
}
static float4 LOCAL_CALL
            getSample1D_LA(const uint8_t *p, int32_t iPixel,
                           int32_t next, float w0, float w1) {
    float2 p0 = getElementAt2(p, iPixel);
    float2 p1 = getElementAt2(p, next);
    float2 r = p0 * w0 + p1 * w1;
    r *= (1.f / 255.f);
    float4 ret = {r.x, r.x, r.x, r.y};
    return ret;
}
static float4 LOCAL_CALL
            getSample1D_RGB(const uint8_t *p, int32_t iPixel,
                            int32_t next, float w0, float w1) {
    float3 p0 = getElementAt3(p, iPixel);
    float3 p1 = getElementAt3(p, next);
    float3 r = p0 * w0 + p1 * w1;
    r *= (1.f / 255.f);
    float4 ret = {r.x, r.x, r.z, 1.f};
    return ret;
}
static float4 LOCAL_CALL
            getSample1D_565(const uint8_t *p, int32_t iPixel,
                           int32_t next, float w0, float w1) {
    float3 p0 = getElementAt565(p, iPixel);
    float3 p1 = getElementAt565(p, next);
    float3 r = p0 * w0 + p1 * w1;
    r *= (1.f / 255.f);
    float4 ret = {r.x, r.x, r.z, 1.f};
    return ret;
}
static float4 LOCAL_CALL
            getSample1D_RGBA(const uint8_t *p, int32_t iPixel,
                             int32_t next, float w0, float w1) {
    float4 p0 = getElementAt4(p, iPixel);
    float4 p1 = getElementAt4(p, next);
    float4 r = p0 * w0 + p1 * w1;
    r *= (1.f / 255.f);
    return r;
}


static float4 LOCAL_CALL
            getSample2D_A(const uint8_t *p, size_t stride,
                          int locX, int locY, int nextX, int nextY,
                          float w0, float w1, float w2, float w3) {
    float p0 = getElementAt1(p, stride, locX, locY);
    float p1 = getElementAt1(p, stride, nextX, locY);
    float p2 = getElementAt1(p, stride, locX, nextY);
    float p3 = getElementAt1(p, stride, nextX, nextY);
    float r = p0 * w0 + p1 * w1 + p2 * w2 + p3 * w3;
    r *= (1.f / 255.f);
    float4 ret = {0.f, 0.f, 0.f, r};
    return ret;
}
static float4 LOCAL_CALL
            getSample2D_L(const uint8_t *p, size_t stride,
                         int locX, int locY, int nextX, int nextY,
                         float w0, float w1, float w2, float w3) {
    float p0 = getElementAt1(p, stride, locX, locY);
    float p1 = getElementAt1(p, stride, nextX, locY);
    float p2 = getElementAt1(p, stride, locX, nextY);
    float p3 = getElementAt1(p, stride, nextX, nextY);
    float r = p0 * w0 + p1 * w1 + p2 * w2 + p3 * w3;
    r *= (1.f / 255.f);
    float4 ret = {r, r, r, 1.f};
    return ret;
}
static float4 LOCAL_CALL
            getSample2D_LA(const uint8_t *p, size_t stride,
                         int locX, int locY, int nextX, int nextY,
                         float w0, float w1, float w2, float w3) {
    float2 p0 = getElementAt2(p, stride, locX, locY);
    float2 p1 = getElementAt2(p, stride, nextX, locY);
    float2 p2 = getElementAt2(p, stride, locX, nextY);
    float2 p3 = getElementAt2(p, stride, nextX, nextY);
    float2 r = p0 * w0 + p1 * w1 + p2 * w2 + p3 * w3;
    r *= (1.f / 255.f);
    float4 ret = {r.x, r.x, r.x, r.y};
    return ret;
}
static float4 LOCAL_CALL
            getSample2D_RGB(const uint8_t *p, size_t stride,
                         int locX, int locY, int nextX, int nextY,
                         float w0, float w1, float w2, float w3) {
    float4 p0 = getElementAt4(p, stride, locX, locY);
    float4 p1 = getElementAt4(p, stride, nextX, locY);
    float4 p2 = getElementAt4(p, stride, locX, nextY);
    float4 p3 = getElementAt4(p, stride, nextX, nextY);
    float4 r = p0 * w0 + p1 * w1 + p2 * w2 + p3 * w3;
    r *= (1.f / 255.f);
    float4 ret = {r.x, r.y, r.z, 1.f};
    return ret;
}
static float4 LOCAL_CALL
            getSample2D_RGBA(const uint8_t *p, size_t stride,
                         int locX, int locY, int nextX, int nextY,
                         float w0, float w1, float w2, float w3) {
    float4 p0 = getElementAt4(p, stride, locX, locY);
    float4 p1 = getElementAt4(p, stride, nextX, locY);
    float4 p2 = getElementAt4(p, stride, locX, nextY);
    float4 p3 = getElementAt4(p, stride, nextX, nextY);
    float4 r = p0 * w0 + p1 * w1 + p2 * w2 + p3 * w3;
    r *= (1.f / 255.f);
    return r;
}
static float4 getSample2D_565(const uint8_t *p, size_t stride,
                         int locX, int locY, int nextX, int nextY,
                         float w0, float w1, float w2, float w3) {
    float3 p0 = getElementAt565(p, stride, locX, locY);
    float3 p1 = getElementAt565(p, stride, nextX, locY);
    float3 p2 = getElementAt565(p, stride, locX, nextY);
    float3 p3 = getElementAt565(p, stride, nextX, nextY);
    float3 r = p0 * w0 + p1 * w1 + p2 * w2 + p3 * w3;
    r *= (1.f / 255.f);
    float4 ret = {r.x, r.y, r.z, 1.f};
    return ret;
}


extern "C" {
    typedef float4 Sampler2DFn(const uint8_t *p, size_t stride,
                               int lx, int ly, int nx, int ny,
                               float w0, float w1, float w2, float w3) LOCAL_CALL;

    Sampler2DFn rsdCpuGetSample2D_L_k;
    Sampler2DFn rsdCpuGetSample2D_A_k;
    Sampler2DFn rsdCpuGetSample2D_LA_k;
    Sampler2DFn rsdCpuGetSample2D_RGB_k;
    Sampler2DFn rsdCpuGetSample2D_RGBA_k;
}

#if 0
static Sampler2DFn* GetBilinearSampleTable2D[] = {
    0, 0, 0, 0, 0, 0, 0,
    0,//rsdCpuGetSample2D_L_k,
    0,//rsdCpuGetSample2D_A_k,
    0,//rsdCpuGetSample2D_LA_k,
    0,//rsdCpuGetSample2D_RGB_k,
    rsdCpuGetSample2D_RGBA_k
};

#else
static Sampler2DFn* GetBilinearSampleTable2D[] = {
    0, 0, 0, 0, 0, 0, 0,
    &getSample2D_L,
    &getSample2D_A,
    &getSample2D_LA,
    &getSample2D_RGB,
    &getSample2D_RGBA,
};
#endif


static int applyWrapMode(RsSamplerValue mode, int coord, int size) {
    switch (mode) {
    case RS_SAMPLER_WRAP:
        coord = coord % size;
        if (coord < 0) {
            coord += size;
        }
        break;

    case RS_SAMPLER_CLAMP:
        coord = rsMax(0, rsMin(coord, size - 1));
        break;

    case RS_SAMPLER_MIRRORED_REPEAT:
        coord = coord % (size * 2);
        if (coord < 0) {
            coord = (size * 2) + coord;
        }
        if (coord >= size) {
            coord = (size * 2) - coord;
        }
        break;

    default:
        coord = 0;
        rsAssert(0);
    }
    return coord;
}

static float4
        sample_LOD_LinearPixel(Allocation *a, const Type *type,
                               RsDataKind dk, RsDataType dt,
                               Sampler *s,
                               float uv, int32_t lod) {
    RsSamplerValue wrapS = s->mHal.state.wrapS;
    int32_t sourceW = type->mHal.state.lodDimX[lod];
    float pixelUV = uv * (float)(sourceW);
    int32_t iPixel = (int32_t)(pixelUV);
    float frac = pixelUV - (float)iPixel;

    if (frac < 0.5f) {
        iPixel -= 1;
        frac += 0.5f;
    } else {
        frac -= 0.5f;
    }

    float oneMinusFrac = 1.0f - frac;

    int32_t next = applyWrapMode(wrapS, iPixel + 1, sourceW);
    int32_t loc = applyWrapMode(wrapS, iPixel, sourceW);

    const uint8_t *ptr = (const uint8_t *)a->mHal.drvState.lod[lod].mallocPtr;

    if (dt == RS_TYPE_UNSIGNED_5_6_5) {
        return getSample1D_565(ptr, loc, next, next, frac);
    }

    switch(dk) {
    case RS_KIND_PIXEL_L:
        return getSample1D_L(ptr, loc, next, next, frac);
    case RS_KIND_PIXEL_A:
        return getSample1D_A(ptr, loc, next, next, frac);
    case RS_KIND_PIXEL_LA:
        return getSample1D_LA(ptr, loc, next, next, frac);
    case RS_KIND_PIXEL_RGB:
        return getSample1D_RGB(ptr, loc, next, next, frac);
    case RS_KIND_PIXEL_RGBA:
        return getSample1D_RGBA(ptr, loc, next, next, frac);

    case RS_KIND_PIXEL_YUV:
    case RS_KIND_USER:
    case RS_KIND_INVALID:
    case RS_KIND_PIXEL_DEPTH:
        rsAssert(0);
        break;
    }

    return 0.f;
}

static float4
        sample_LOD_NearestPixel(Allocation *a, const Type *type,
                                RsDataKind dk, RsDataType dt,
                                Sampler *s, float uv, int32_t lod) {
    RsSamplerValue wrapS = s->mHal.state.wrapS;
    int32_t sourceW = type->mHal.state.lodDimX[lod];
    int32_t iPixel = (int32_t)(uv * (float)(sourceW));
    int32_t location = applyWrapMode(wrapS, iPixel, sourceW);


    const uint8_t *ptr = (const uint8_t *)a->mHal.drvState.lod[lod].mallocPtr;

    float4 result = {0.f, 0.f, 0.f, 1.f};
    if (dt == RS_TYPE_UNSIGNED_5_6_5) {
        result.xyz = getElementAt565(ptr, iPixel);
       return result;
    }

    switch(dk) {
    case RS_KIND_PIXEL_L:
        {
            float t = getElementAt1(ptr, iPixel);
            result.xyz = t;
        }
        break;
    case RS_KIND_PIXEL_A:
        result.w = getElementAt1(ptr, iPixel);
        break;
    case RS_KIND_PIXEL_LA:
        {
            float2 t = getElementAt2(ptr, iPixel);
            result.xyz = t.x;
            result.w = t.y;
        }
        break;
    case RS_KIND_PIXEL_RGB:
        result.xyz = getElementAt3(ptr, iPixel);
        break;
    case RS_KIND_PIXEL_RGBA:
        result = getElementAt4(ptr, iPixel);
        break;

    case RS_KIND_PIXEL_YUV:
    case RS_KIND_USER:
    case RS_KIND_INVALID:
    case RS_KIND_PIXEL_DEPTH:
        rsAssert(0);
        break;
    }

    return result * (1.f / 255.f);
}


static float4
        sample_LOD_LinearPixel(Allocation *a, const Type *type,
                               RsDataKind dk, RsDataType dt,
                               Sampler *s, float u, float v, int32_t lod) {
    const RsSamplerValue wrapS = s->mHal.state.wrapS;
    const RsSamplerValue wrapT = s->mHal.state.wrapT;
    const int sourceW = type->mHal.state.lodDimX[lod];
    const int sourceH = type->mHal.state.lodDimY[lod];

    float pixelU = u * (float)sourceW;
    float pixelV = v * (float)sourceH;
    int iPixelU = (int)pixelU;
    int iPixelV = (int)pixelV;

    float fracU = pixelU - iPixelU;
    float fracV = pixelV - iPixelV;

    if (fracU < 0.5f) {
        iPixelU -= 1;
        fracU += 0.5f;
    } else {
        fracU -= 0.5f;
    }
    if (fracV < 0.5f) {
        iPixelV -= 1;
        fracV += 0.5f;
    } else {
        fracV -= 0.5f;
    }
    float oneMinusFracU = 1.0f - fracU;
    float oneMinusFracV = 1.0f - fracV;

    float w1 = oneMinusFracU * oneMinusFracV;
    float w2 = fracU * oneMinusFracV;
    float w3 = oneMinusFracU * fracV;
    float w4 = fracU * fracV;

    int nextX = applyWrapMode(wrapS, iPixelU + 1, sourceW);
    int nextY = applyWrapMode(wrapT, iPixelV + 1, sourceH);
    int locX = applyWrapMode(wrapS, iPixelU, sourceW);
    int locY = applyWrapMode(wrapT, iPixelV, sourceH);

    const uint8_t *ptr = (const uint8_t *)a->mHal.drvState.lod[lod].mallocPtr;
    size_t stride = a->mHal.drvState.lod[lod].stride;

    if (dt == RS_TYPE_UNSIGNED_5_6_5) {
        return getSample2D_565(ptr, stride, locX, locY, nextX, nextY, w1, w2, w3, w4);
    }

    return GetBilinearSampleTable2D[dk](ptr, stride, locX, locY, nextX, nextY, w1, w2, w3, w4);
}

static float4
        sample_LOD_NearestPixel(Allocation *a, const Type *type,
                                RsDataKind dk, RsDataType dt,
                                Sampler *s,
                                float u, float v, int32_t lod) {
    RsSamplerValue wrapS = s->mHal.state.wrapS;
    RsSamplerValue wrapT = s->mHal.state.wrapT;

    int32_t sourceW = type->mHal.state.lodDimX[lod];
    int32_t sourceH = type->mHal.state.lodDimY[lod];

    int locX = applyWrapMode(wrapS, u * sourceW, sourceW);
    int locY = applyWrapMode(wrapT, v * sourceH, sourceH);


    const uint8_t *ptr = (const uint8_t *)a->mHal.drvState.lod[lod].mallocPtr;
    size_t stride = a->mHal.drvState.lod[lod].stride;

    float4 result = {0.f, 0.f, 0.f, 1.f};
    if (dt == RS_TYPE_UNSIGNED_5_6_5) {
        result.xyz = getElementAt565(ptr, stride, locX, locY);
       return result;
    }

    switch(dk) {
    case RS_KIND_PIXEL_L:
        {
            float t = getElementAt1(ptr, stride, locX, locY);
            result.xyz = t;
        }
        break;
    case RS_KIND_PIXEL_A:
        result.w = getElementAt1(ptr, stride, locX, locY);
        break;
    case RS_KIND_PIXEL_LA:
        {
            float2 t = getElementAt2(ptr, stride, locX, locY);
            result.xyz = t.x;
            result.w = t.y;
        }
        break;
    case RS_KIND_PIXEL_RGB:
        result.xyz = getElementAt3(ptr, stride, locX, locY);
        break;
    case RS_KIND_PIXEL_RGBA:
        result = getElementAt4(ptr, stride, locX, locY);
        break;


    case RS_KIND_PIXEL_YUV:
    case RS_KIND_USER:
    case RS_KIND_INVALID:
    case RS_KIND_PIXEL_DEPTH:
        rsAssert(0);
        break;
    }

    return result * (1.f / 255.f);
}



static float4 GenericSample1D(Allocation *a, Sampler *s, float u, float lod) {
    const Type *type = a->getType();
    const Element *elem = type->getElement();
    const RsDataKind dk = elem->getKind();
    const RsDataType dt = elem->getType();

    if (dk == RS_KIND_USER || (dt != RS_TYPE_UNSIGNED_8 && dt != RS_TYPE_UNSIGNED_5_6_5)) {
        return 0.f;
    }

    if (!(a->mHal.state.usageFlags & RS_ALLOCATION_USAGE_GRAPHICS_TEXTURE)) {
        const Context *rsc = RsdCpuReference::getTlsContext();
        rsc->setError(RS_ERROR_BAD_VALUE, "Sampling from texture witout USAGE_GRAPHICS_TEXTURE.");
        return 0.f;
    }

    if (lod <= 0.0f) {
        if (s->mHal.state.magFilter == RS_SAMPLER_NEAREST) {
            return sample_LOD_NearestPixel(a, type, dk, dt, s, u, 0);
        }
        return sample_LOD_LinearPixel(a, type, dk, dt, s, u, 0);
    }

    if (s->mHal.state.minFilter == RS_SAMPLER_LINEAR_MIP_NEAREST) {
        int32_t maxLOD = type->mHal.state.lodCount - 1;
        lod = rsMin(lod, (float)maxLOD);
        int32_t nearestLOD = (int32_t)round(lod);
        return sample_LOD_LinearPixel(a, type, dk, dt, s, u, nearestLOD);
    }

    if (s->mHal.state.minFilter == RS_SAMPLER_LINEAR_MIP_LINEAR) {
        int32_t lod0 = (int32_t)floor(lod);
        int32_t lod1 = (int32_t)ceil(lod);
        int32_t maxLOD = type->mHal.state.lodCount - 1;
        lod0 = rsMin(lod0, maxLOD);
        lod1 = rsMin(lod1, maxLOD);
        float4 sample0 = sample_LOD_LinearPixel(a, type, dk, dt, s, u, lod0);
        float4 sample1 = sample_LOD_LinearPixel(a, type, dk, dt, s, u, lod1);
        float frac = lod - (float)lod0;
        return sample0 * (1.0f - frac) + sample1 * frac;
    }

    return sample_LOD_NearestPixel(a, type, dk, dt, s, u, 0);
}

static float4 GenericSample2D(Allocation *a, Sampler *s, float u, float v, float lod) {
    const Type *type = a->getType();
    const Element *elem = type->getElement();
    const RsDataKind dk = elem->getKind();
    const RsDataType dt = elem->getType();

    if (dk == RS_KIND_USER || (dt != RS_TYPE_UNSIGNED_8 && dt != RS_TYPE_UNSIGNED_5_6_5)) {
        return 0.f;
    }

    if (!(a->mHal.state.usageFlags & RS_ALLOCATION_USAGE_GRAPHICS_TEXTURE)) {
        const Context *rsc = RsdCpuReference::getTlsContext();
        rsc->setError(RS_ERROR_BAD_VALUE, "Sampling from texture witout USAGE_GRAPHICS_TEXTURE.");
        return 0.f;
    }

    if (lod <= 0.0f) {
        if (s->mHal.state.magFilter == RS_SAMPLER_NEAREST) {
            return sample_LOD_NearestPixel(a, type, dk, dt, s, u, v, 0);
        }
        return sample_LOD_LinearPixel(a, type, dk, dt, s, u, v, 0);
    }

    if (s->mHal.state.minFilter == RS_SAMPLER_LINEAR_MIP_NEAREST) {
        int32_t maxLOD = type->mHal.state.lodCount - 1;
        lod = rsMin(lod, (float)maxLOD);
        int32_t nearestLOD = (int32_t)round(lod);
        return sample_LOD_LinearPixel(a, type, dk, dt, s, u, v, nearestLOD);
    }

    if (s->mHal.state.minFilter == RS_SAMPLER_LINEAR_MIP_LINEAR) {
        int32_t lod0 = (int32_t)floor(lod);
        int32_t lod1 = (int32_t)ceil(lod);
        int32_t maxLOD = type->mHal.state.lodCount - 1;
        lod0 = rsMin(lod0, maxLOD);
        lod1 = rsMin(lod1, maxLOD);
        float4 sample0 = sample_LOD_LinearPixel(a, type, dk, dt, s, u, v, lod0);
        float4 sample1 = sample_LOD_LinearPixel(a, type, dk, dt, s, u, v, lod1);
        float frac = lod - (float)lod0;
        return sample0 * (1.0f - frac) + sample1 * frac;
    }

    return sample_LOD_NearestPixel(a, type, dk, dt, s, u, v, 0);
}




// Must match pixel kind in rsDefines.h
static void * NearestWrap[] = {
    (void *) GenericSample1D,                // L,
    (void *) GenericSample1D,                // A,
    (void *) GenericSample1D,                // LA,
    (void *) GenericSample1D,                // RGB,
    (void *) GenericSample1D,                // RGBA,
    0,
    (void *) GenericSample1D,                // YUV

    (void *) GenericSample2D,                // L,
    (void *) GenericSample2D,                // A,
    (void *) GenericSample2D,                // LA,
    (void *) GenericSample2D,                // RGB,
    (void *) GenericSample2D,                // RGBA,
    0,
    (void *) GenericSample2D,                // YUV
};

static void * NearestClamp[] = {
    (void *) GenericSample1D,                // L,
    (void *) GenericSample1D,                // A,
    (void *) GenericSample1D,                // LA,
    (void *) GenericSample1D,                // RGB,
    (void *) GenericSample1D,                // RGBA,
    0,
    (void *) GenericSample1D,                // YUV

    (void *) GenericSample2D,                // L,
    (void *) GenericSample2D,                // A,
    (void *) GenericSample2D,                // LA,
    (void *) GenericSample2D,                // RGB,
    (void *) GenericSample2D,                // RGBA,
    0,
    (void *) GenericSample2D,                // YUV
};

static void * NearestMirroredRepeat[] = {
    (void *) GenericSample1D,                // L,
    (void *) GenericSample1D,                // A,
    (void *) GenericSample1D,                // LA,
    (void *) GenericSample1D,                // RGB,
    (void *) GenericSample1D,                // RGBA,
    0,
    (void *) GenericSample1D,                // YUV

    (void *) GenericSample2D,                // L,
    (void *) GenericSample2D,                // A,
    (void *) GenericSample2D,                // LA,
    (void *) GenericSample2D,                // RGB,
    (void *) GenericSample2D,                // RGBA,
    0,
    (void *) GenericSample2D,                // YUV
};

// Must match pixel kind in rsDefines.h
static void * LinearWrap[] = {
    (void *) GenericSample1D,                // L,
    (void *) GenericSample1D,                // A,
    (void *) GenericSample1D,                // LA,
    (void *) GenericSample1D,                // RGB,
    (void *) GenericSample1D,                // RGBA,
    0,
    (void *) GenericSample1D,                // YUV

    (void *) GenericSample2D,                // L,
    (void *) GenericSample2D,                // A,
    (void *) GenericSample2D,                // LA,
    (void *) GenericSample2D,                // RGB,
    (void *) GenericSample2D,                // RGBA,
    0,
    (void *) GenericSample2D,                // YUV
};

// Must match pixel kind in rsDefines.h
static void * LinearClamp[] = {
    (void *) GenericSample1D,                // L,
    (void *) GenericSample1D,                // A,
    (void *) GenericSample1D,                // LA,
    (void *) GenericSample1D,                // RGB,
    (void *) GenericSample1D,                // RGBA,
    0,
    (void *) GenericSample1D,                // YUV

    (void *) GenericSample2D,                // L,
    (void *) GenericSample2D,                // A,
    (void *) GenericSample2D,                // LA,
    (void *) GenericSample2D,                // RGB,
    (void *) GenericSample2D,                // RGBA,
    0,
    (void *) GenericSample2D,                // YUV
};

// Must match pixel kind in rsDefines.h
static void * LinearMirroredRepeat[] = {
    (void *) GenericSample1D,                // L,
    (void *) GenericSample1D,                // A,
    (void *) GenericSample1D,                // LA,
    (void *) GenericSample1D,                // RGB,
    (void *) GenericSample1D,                // RGBA,
    0,
    (void *) GenericSample1D,                // YUV

    (void *) GenericSample2D,                // L,
    (void *) GenericSample2D,                // A,
    (void *) GenericSample2D,                // LA,
    (void *) GenericSample2D,                // RGB,
    (void *) GenericSample2D,                // RGBA,
    0,
    (void *) GenericSample2D,                // YUV
};

// Must match pixel kind in rsDefines.h
static void * Generic[] = {
    (void *) GenericSample1D,                // L,
    (void *) GenericSample1D,                // A,
    (void *) GenericSample1D,                // LA,
    (void *) GenericSample1D,                // RGB,
    (void *) GenericSample1D,                // RGBA,
    0,
    (void *) GenericSample1D,                // YUV

    (void *) GenericSample2D,                // L,
    (void *) GenericSample2D,                // A,
    (void *) GenericSample2D,                // LA,
    (void *) GenericSample2D,                // RGB,
    (void *) GenericSample2D,                // RGBA,
    0,
    (void *) GenericSample2D,                // YUV
};

bool rsdSamplerInit(const Context *, const Sampler *s) {
    s->mHal.drv = Generic;

    if ((s->mHal.state.minFilter == s->mHal.state.magFilter) &&
        (s->mHal.state.wrapS == s->mHal.state.wrapT)) {
        // We have fast paths for these.

        switch(s->mHal.state.minFilter) {
        case RS_SAMPLER_NEAREST:
            switch(s->mHal.state.wrapS) {
            case RS_SAMPLER_WRAP:
                s->mHal.drv = NearestWrap;
                break;
            case RS_SAMPLER_CLAMP:
                s->mHal.drv = NearestClamp;
                break;
            case RS_SAMPLER_MIRRORED_REPEAT:
                s->mHal.drv = NearestMirroredRepeat;
                break;
            default:
                break;
            }
            break;
        case RS_SAMPLER_LINEAR:
            switch(s->mHal.state.wrapS) {
            case RS_SAMPLER_WRAP:
                s->mHal.drv = LinearWrap;
                break;
            case RS_SAMPLER_CLAMP:
                s->mHal.drv = LinearClamp;
                break;
            case RS_SAMPLER_MIRRORED_REPEAT:
                s->mHal.drv = LinearMirroredRepeat;
                break;
            default:
                break;
            }
            break;
        case RS_SAMPLER_LINEAR_MIP_LINEAR:
            switch(s->mHal.state.wrapS) {
            case RS_SAMPLER_WRAP:
                s->mHal.drv = LinearWrap;
                break;
            case RS_SAMPLER_CLAMP:
                s->mHal.drv = LinearClamp;
                break;
            case RS_SAMPLER_MIRRORED_REPEAT:
                s->mHal.drv = LinearMirroredRepeat;
                break;
            default:
                break;
            }
            break;
        default:
            rsAssert(0);
            break;
        }

    }

    return true;
}

void rsdSamplerDestroy(const android::renderscript::Context *rsc,
                       const android::renderscript::Sampler *s) {
}
