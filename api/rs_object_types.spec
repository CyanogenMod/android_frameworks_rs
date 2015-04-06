#
# Copyright (C) 2015 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

header:
summary: Object Types
description:
 The types below are used to manipulate RenderScript objects like allocations,
 samplers, elements, and scripts.  Most of these object are created using the Java
 RenderScript APIs.
include:
 #define NULL ((void *)0)

 // Opaque handle to a RenderScript object. Do not use this directly.
 #ifndef __LP64__
 #define _RS_HANDLE \
 struct {\
   const int* const p;\
 } __attribute__((packed, aligned(4)))
 #else
 #define _RS_HANDLE \
 struct {\
   const long* const p;\
   const long* const r;\
   const long* const v1;\
   const long* const v2;\
 }
 #endif
end:

type: rs_element
simple: _RS_HANDLE
summary: Handle to an element
description:
 Opaque handle to a RenderScript element.
 See: android.renderscript.Element
end:

type: rs_type
simple: _RS_HANDLE
summary: Handle to a Type
description:
 Opaque handle to a RenderScript type.
 See: android.renderscript.Type
end:

type: rs_allocation
simple: _RS_HANDLE
summary: Handle to an allocation
description:
 Opaque handle to a RenderScript allocation.
 See: android.renderscript.Allocation
end:

type: rs_sampler
simple: _RS_HANDLE
summary: Handle to a Sampler
description:
 Opaque handle to a RenderScript sampler object.
 See: android.renderscript.Sampler
end:

type: rs_script
simple: _RS_HANDLE
summary: Handle to a Script
description:
 Opaque handle to a RenderScript script object.
 See: android.renderscript.ScriptC
end:

type: rs_allocation_cubemap_face
version: 14
enum:
value: RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X = 0
value: RS_ALLOCATION_CUBEMAP_FACE_NEGATIVE_X = 1
value: RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_Y = 2
value: RS_ALLOCATION_CUBEMAP_FACE_NEGATIVE_Y = 3
value: RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_Z = 4
value: RS_ALLOCATION_CUBEMAP_FACE_NEGATIVE_Z = 5
summary: Enum for selecting cube map faces
description:
end:

type: rs_allocation_usage_type
version: 14
enum:
value: RS_ALLOCATION_USAGE_SCRIPT = 0x0001, "Allocation is bound to and accessed by scripts."
value: RS_ALLOCATION_USAGE_GRAPHICS_TEXTURE = 0x0002, "Deprecated."
value: RS_ALLOCATION_USAGE_GRAPHICS_VERTEX = 0x0004, "Deprecated."
value: RS_ALLOCATION_USAGE_GRAPHICS_CONSTANTS = 0x0008, "Deprecated."
value: RS_ALLOCATION_USAGE_GRAPHICS_RENDER_TARGET = 0x0010, "Deprecated."
value: RS_ALLOCATION_USAGE_IO_INPUT = 0x0020, "Allocation is used as a Surface consumer."
value: RS_ALLOCATION_USAGE_IO_OUTPUT = 0x0040, "Allocation is used as a Surface producer."
value: RS_ALLOCATION_USAGE_SHARED = 0x0080, "Allocation's backing store is shared with another object (usually a Bitmap).  Copying to or from the original source Bitmap will cause a synchronization rather than a full copy."
summary: Bitfield to specify the usage types for an allocation
description:
 These values are ORed together to specify which usages or memory spaces are
 relevant to an allocation or an operation on an allocation.
end:

type: rs_data_type
version: 16
enum:
value: RS_TYPE_NONE = 0
value: RS_TYPE_FLOAT_32 = 2
value: RS_TYPE_FLOAT_64 = 3
value: RS_TYPE_SIGNED_8 = 4
value: RS_TYPE_SIGNED_16 = 5
value: RS_TYPE_SIGNED_32 = 6
value: RS_TYPE_SIGNED_64 = 7
value: RS_TYPE_UNSIGNED_8 = 8
value: RS_TYPE_UNSIGNED_16 = 9
value: RS_TYPE_UNSIGNED_32 = 10
value: RS_TYPE_UNSIGNED_64 = 11
value: RS_TYPE_BOOLEAN = 12
value: RS_TYPE_UNSIGNED_5_6_5 = 13
value: RS_TYPE_UNSIGNED_5_5_5_1 = 14
value: RS_TYPE_UNSIGNED_4_4_4_4 = 15
value: RS_TYPE_MATRIX_4X4 = 16
value: RS_TYPE_MATRIX_3X3 = 17
value: RS_TYPE_MATRIX_2X2 = 18
value: RS_TYPE_ELEMENT = 1000
value: RS_TYPE_TYPE = 1001
value: RS_TYPE_ALLOCATION = 1002
value: RS_TYPE_SAMPLER = 1003
value: RS_TYPE_SCRIPT = 1004
value: RS_TYPE_MESH = 1005
value: RS_TYPE_PROGRAM_FRAGMENT = 1006
value: RS_TYPE_PROGRAM_VERTEX = 1007
value: RS_TYPE_PROGRAM_RASTER = 1008
value: RS_TYPE_PROGRAM_STORE = 1009
value: RS_TYPE_FONT = 1010
value: RS_TYPE_INVALID = 10000
summary: Element data types
description:
 DataType represents the basic type information for a basic element.  The
 naming convention follows.  For numeric types it is FLOAT,
 SIGNED, or UNSIGNED followed by the _BITS where BITS is the
 size of the data.  BOOLEAN is a true / false (1,0)
 represented in an 8 bit container.  The UNSIGNED variants
 with multiple bit definitions are for packed graphical data
 formats and represent vectors with per vector member sizes
 which are treated as a single unit for packing and alignment
 purposes.

 MATRIX the three matrix types contain FLOAT_32 elements and are treated
 as 32 bits for alignment purposes.

 RS_* objects.  32 bit opaque handles.
end:

type: rs_data_kind
version: 16
enum:
value: RS_KIND_USER         = 0
value: RS_KIND_PIXEL_L      = 7
value: RS_KIND_PIXEL_A      = 8
value: RS_KIND_PIXEL_LA     = 9
value: RS_KIND_PIXEL_RGB    = 10
value: RS_KIND_PIXEL_RGBA   = 11
value: RS_KIND_PIXEL_DEPTH  = 12
value: RS_KIND_PIXEL_YUV    = 13
value: RS_KIND_INVALID      = 100
summary: Element data kind
description:
 The special interpretation of the data if required.  This is primarly
 useful for graphical data.  USER indicates no special interpretation is
 expected.  PIXEL is used in conjunction with the standard data types for
 representing texture formats.
end:

type: rs_sampler_value
version: 16
enum:
value: RS_SAMPLER_NEAREST = 0
value: RS_SAMPLER_LINEAR = 1
value: RS_SAMPLER_LINEAR_MIP_LINEAR = 2
value: RS_SAMPLER_WRAP = 3
value: RS_SAMPLER_CLAMP = 4
value: RS_SAMPLER_LINEAR_MIP_NEAREST = 5
value: RS_SAMPLER_MIRRORED_REPEAT = 6
value: RS_SAMPLER_INVALID = 100
summary: Sampler wrap T value
description:
end:
