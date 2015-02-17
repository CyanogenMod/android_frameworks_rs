/*
 * Copyright (C) 2013 The Android Open Source Project
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

#ifndef RENDER_SCRIPT_DEFINES_H
#define RENDER_SCRIPT_DEFINES_H

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////
//

typedef void * RsAsyncVoidPtr;

typedef void * RsAdapter1D;
typedef void * RsAdapter2D;
typedef void * RsAllocation;
typedef void * RsAnimation;
typedef void * RsClosure;
typedef void * RsContext;
typedef void * RsDevice;
typedef void * RsElement;
typedef void * RsFile;
typedef void * RsFont;
typedef void * RsSampler;
typedef void * RsScript;
typedef void * RsScriptKernelID;
typedef void * RsScriptInvokeID;
typedef void * RsScriptFieldID;
typedef void * RsScriptMethodID;
typedef void * RsScriptGroup;
typedef void * RsScriptGroup2;
typedef void * RsMesh;
typedef void * RsPath;
typedef void * RsType;
typedef void * RsObjectBase;

typedef void * RsProgram;
typedef void * RsProgramVertex;
typedef void * RsProgramFragment;
typedef void * RsProgramStore;
typedef void * RsProgramRaster;

typedef void * RsNativeWindow;

typedef void (* RsBitmapCallback_t)(void *);

typedef struct {
    float m[16];
} rs_matrix4x4;

typedef struct {
    float m[9];
} rs_matrix3x3;

typedef struct {
    float m[4];
} rs_matrix2x2;

enum RsDeviceParam {
    RS_DEVICE_PARAM_FORCE_SOFTWARE_GL,
    RS_DEVICE_PARAM_COUNT
};

enum RsContextType {
    RS_CONTEXT_TYPE_NORMAL,
    RS_CONTEXT_TYPE_DEBUG,
    RS_CONTEXT_TYPE_PROFILE
};


enum RsAllocationUsageType {
    RS_ALLOCATION_USAGE_SCRIPT = 0x0001,
    RS_ALLOCATION_USAGE_GRAPHICS_TEXTURE = 0x0002,
    RS_ALLOCATION_USAGE_GRAPHICS_VERTEX = 0x0004,
    RS_ALLOCATION_USAGE_GRAPHICS_CONSTANTS = 0x0008,
    RS_ALLOCATION_USAGE_GRAPHICS_RENDER_TARGET = 0x0010,
    RS_ALLOCATION_USAGE_IO_INPUT = 0x0020,
    RS_ALLOCATION_USAGE_IO_OUTPUT = 0x0040,
    RS_ALLOCATION_USAGE_SHARED = 0x0080,

    RS_ALLOCATION_USAGE_ALL = 0x00FF
};

enum RsAllocationMipmapControl {
    RS_ALLOCATION_MIPMAP_NONE = 0,
    RS_ALLOCATION_MIPMAP_FULL = 1,
    RS_ALLOCATION_MIPMAP_ON_SYNC_TO_TEXTURE = 2
};

enum RsAllocationCubemapFace {
    RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X = 0,
    RS_ALLOCATION_CUBEMAP_FACE_NEGATIVE_X = 1,
    RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_Y = 2,
    RS_ALLOCATION_CUBEMAP_FACE_NEGATIVE_Y = 3,
    RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_Z = 4,
    RS_ALLOCATION_CUBEMAP_FACE_NEGATIVE_Z = 5
};

enum RsDataType {
    RS_TYPE_NONE,
    RS_TYPE_FLOAT_16,
    RS_TYPE_FLOAT_32,
    RS_TYPE_FLOAT_64,
    RS_TYPE_SIGNED_8,
    RS_TYPE_SIGNED_16,
    RS_TYPE_SIGNED_32,
    RS_TYPE_SIGNED_64,
    RS_TYPE_UNSIGNED_8,
    RS_TYPE_UNSIGNED_16,
    RS_TYPE_UNSIGNED_32,
    RS_TYPE_UNSIGNED_64,

    RS_TYPE_BOOLEAN,

    RS_TYPE_UNSIGNED_5_6_5,
    RS_TYPE_UNSIGNED_5_5_5_1,
    RS_TYPE_UNSIGNED_4_4_4_4,

    RS_TYPE_MATRIX_4X4,
    RS_TYPE_MATRIX_3X3,
    RS_TYPE_MATRIX_2X2,

    RS_TYPE_ELEMENT = 1000,
    RS_TYPE_TYPE,
    RS_TYPE_ALLOCATION,
    RS_TYPE_SAMPLER,
    RS_TYPE_SCRIPT,
    RS_TYPE_MESH,
    RS_TYPE_PROGRAM_FRAGMENT,
    RS_TYPE_PROGRAM_VERTEX,
    RS_TYPE_PROGRAM_RASTER,
    RS_TYPE_PROGRAM_STORE,
    RS_TYPE_FONT,

    RS_TYPE_INVALID = 10000,
};

enum RsDataKind {
    RS_KIND_USER,

    RS_KIND_PIXEL_L = 7,
    RS_KIND_PIXEL_A,
    RS_KIND_PIXEL_LA,
    RS_KIND_PIXEL_RGB,
    RS_KIND_PIXEL_RGBA,
    RS_KIND_PIXEL_DEPTH,
    RS_KIND_PIXEL_YUV,

    RS_KIND_INVALID = 100,
};

enum RsSamplerParam {
    RS_SAMPLER_MIN_FILTER,
    RS_SAMPLER_MAG_FILTER,
    RS_SAMPLER_WRAP_S,
    RS_SAMPLER_WRAP_T,
    RS_SAMPLER_WRAP_R,
    RS_SAMPLER_ANISO
};

enum RsSamplerValue {
    RS_SAMPLER_NEAREST,
    RS_SAMPLER_LINEAR,
    RS_SAMPLER_LINEAR_MIP_LINEAR,
    RS_SAMPLER_WRAP,
    RS_SAMPLER_CLAMP,
    RS_SAMPLER_LINEAR_MIP_NEAREST,
    RS_SAMPLER_MIRRORED_REPEAT,

    RS_SAMPLER_INVALID = 100,
};

enum RsDimension {
    RS_DIMENSION_X,
    RS_DIMENSION_Y,
    RS_DIMENSION_Z,
    RS_DIMENSION_LOD,
    RS_DIMENSION_FACE,

    RS_DIMENSION_ARRAY_0 = 100,
    RS_DIMENSION_ARRAY_1,
    RS_DIMENSION_ARRAY_2,
    RS_DIMENSION_ARRAY_3,
    RS_DIMENSION_MAX = RS_DIMENSION_ARRAY_3
};


enum RsError {
    RS_ERROR_NONE = 0,
    RS_ERROR_BAD_SHADER = 1,
    RS_ERROR_BAD_SCRIPT = 2,
    RS_ERROR_BAD_VALUE = 3,
    RS_ERROR_OUT_OF_MEMORY = 4,
    RS_ERROR_DRIVER = 5,

    // Errors that only occur in the debug context.
    RS_ERROR_FATAL_DEBUG = 0x0800,

    RS_ERROR_FATAL_UNKNOWN = 0x1000,
    RS_ERROR_FATAL_DRIVER = 0x1001,
    RS_ERROR_FATAL_PROGRAM_LINK = 0x1002
};

enum RsForEachStrategy {
    RS_FOR_EACH_STRATEGY_SERIAL = 0,
    RS_FOR_EACH_STRATEGY_DONT_CARE = 1,
    RS_FOR_EACH_STRATEGY_DST_LINEAR = 2,
    RS_FOR_EACH_STRATEGY_TILE_SMALL = 3,
    RS_FOR_EACH_STRATEGY_TILE_MEDIUM = 4,
    RS_FOR_EACH_STRATEGY_TILE_LARGE = 5
};

// Script to Script
typedef struct {
    enum RsForEachStrategy strategy;
    uint32_t xStart;
    uint32_t xEnd;
    uint32_t yStart;
    uint32_t yEnd;
    uint32_t zStart;
    uint32_t zEnd;
    uint32_t arrayStart;
    uint32_t arrayEnd;
    uint32_t array2Start;
    uint32_t array2End;
    uint32_t array3Start;
    uint32_t array3End;
    uint32_t array4Start;
    uint32_t array4End;

} RsScriptCall;

enum RsContextFlags {
    RS_CONTEXT_SYNCHRONOUS      = 0x0001,
    RS_CONTEXT_LOW_LATENCY      = 0x0002,
    RS_CONTEXT_LOW_POWER        = 0x0004
};

enum RsBlasTranspose {
    RsBlasNoTrans=111,
    RsBlasTrans=112,
    RsBlasConjTrans=113
};

enum RsBlasUplo {
    RsBlasUpper=121,
    RsBlasLower=122
};

enum RsBlasDiag {
    RsBlasNonUnit=131,
    RsBlasUnit=132
};

enum RsBlasSide {
    RsBlasLeft=141,
    RsBlasRight=142
};

enum RsBlasFunction {
    RsBlas_nop = 0,
    RsBlas_sdsdot,
    RsBlas_dsdot,
    RsBlas_sdot,
    RsBlas_ddot,
    RsBlas_cdotu_sub,
    RsBlas_cdotc_sub,
    RsBlas_zdotu_sub,
    RsBlas_zdotc_sub,
    RsBlas_snrm2,
    RsBlas_sasum,
    RsBlas_dnrm2,
    RsBlas_dasum,
    RsBlas_scnrm2,
    RsBlas_scasum,
    RsBlas_dznrm2,
    RsBlas_dzasum,
    RsBlas_isamax,
    RsBlas_idamax,
    RsBlas_icamax,
    RsBlas_izamax,
    RsBlas_sswap,
    RsBlas_scopy,
    RsBlas_saxpy,
    RsBlas_dswap,
    RsBlas_dcopy,
    RsBlas_daxpy,
    RsBlas_cswap,
    RsBlas_ccopy,
    RsBlas_caxpy,
    RsBlas_zswap,
    RsBlas_zcopy,
    RsBlas_zaxpy,
    RsBlas_srotg,
    RsBlas_srotmg,
    RsBlas_srot,
    RsBlas_srotm,
    RsBlas_drotg,
    RsBlas_drotmg,
    RsBlas_drot,
    RsBlas_drotm,
    RsBlas_sscal,
    RsBlas_dscal,
    RsBlas_cscal,
    RsBlas_zscal,
    RsBlas_csscal,
    RsBlas_zdscal,
    RsBlas_sgemv,
    RsBlas_sgbmv,
    RsBlas_strmv,
    RsBlas_stbmv,
    RsBlas_stpmv,
    RsBlas_strsv,
    RsBlas_stbsv,
    RsBlas_stpsv,
    RsBlas_dgemv,
    RsBlas_dgbmv,
    RsBlas_dtrmv,
    RsBlas_dtbmv,
    RsBlas_dtpmv,
    RsBlas_dtrsv,
    RsBlas_dtbsv,
    RsBlas_dtpsv,
    RsBlas_cgemv,
    RsBlas_cgbmv,
    RsBlas_ctrmv,
    RsBlas_ctbmv,
    RsBlas_ctpmv,
    RsBlas_ctrsv,
    RsBlas_ctbsv,
    RsBlas_ctpsv,
    RsBlas_zgemv,
    RsBlas_zgbmv,
    RsBlas_ztrmv,
    RsBlas_ztbmv,
    RsBlas_ztpmv,
    RsBlas_ztrsv,
    RsBlas_ztbsv,
    RsBlas_ztpsv,
    RsBlas_ssymv,
    RsBlas_ssbmv,
    RsBlas_sspmv,
    RsBlas_sger,
    RsBlas_ssyr,
    RsBlas_sspr,
    RsBlas_ssyr2,
    RsBlas_sspr2,
    RsBlas_dsymv,
    RsBlas_dsbmv,
    RsBlas_dspmv,
    RsBlas_dger,
    RsBlas_dsyr,
    RsBlas_dspr,
    RsBlas_dsyr2,
    RsBlas_dspr2,
    RsBlas_chemv,
    RsBlas_chbmv,
    RsBlas_chpmv,
    RsBlas_cgeru,
    RsBlas_cgerc,
    RsBlas_cher,
    RsBlas_chpr,
    RsBlas_cher2,
    RsBlas_chpr2,
    RsBlas_zhemv,
    RsBlas_zhbmv,
    RsBlas_zhpmv,
    RsBlas_zgeru,
    RsBlas_zgerc,
    RsBlas_zher,
    RsBlas_zhpr,
    RsBlas_zher2,
    RsBlas_zhpr2,
    RsBlas_sgemm,
    RsBlas_ssymm,
    RsBlas_ssyrk,
    RsBlas_ssyr2k,
    RsBlas_strmm,
    RsBlas_strsm,
    RsBlas_dgemm,
    RsBlas_dsymm,
    RsBlas_dsyrk,
    RsBlas_dsyr2k,
    RsBlas_dtrmm,
    RsBlas_dtrsm,
    RsBlas_cgemm,
    RsBlas_csymm,
    RsBlas_csyrk,
    RsBlas_csyr2k,
    RsBlas_ctrmm,
    RsBlas_ctrsm,
    RsBlas_zgemm,
    RsBlas_zsymm,
    RsBlas_zsyrk,
    RsBlas_zsyr2k,
    RsBlas_ztrmm,
    RsBlas_ztrsm,
    RsBlas_chemm,
    RsBlas_cherk,
    RsBlas_cher2k,
    RsBlas_zhemm,
    RsBlas_zherk,
    RsBlas_zher2k
};

// custom complex types because of NDK support
typedef struct {
    float r;
    float i;
} RsFloatComplex;

typedef struct {
    double r;
    double i;
} RsDoubleComplex;

typedef union { 
    float f;
    RsFloatComplex c;
    double d;
    RsDoubleComplex z;
} RsBlasScalar;

typedef struct {
    RsBlasFunction func;
    RsBlasTranspose transA;
    RsBlasTranspose transB;
    RsBlasUplo uplo;
    RsBlasDiag diag;
    RsBlasSide side;
    int M;
    int N;
    int K;
    RsBlasScalar alpha;
    RsBlasScalar beta;
    int incX;
    int incY;
    int KL;
    int KU;
} RsBlasCall;
          
#ifdef __cplusplus
};
#endif

#endif // RENDER_SCRIPT_DEFINES_H
