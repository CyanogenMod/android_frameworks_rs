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


#ifdef __cplusplus
};
#endif

#endif // RENDER_SCRIPT_DEFINES_H
