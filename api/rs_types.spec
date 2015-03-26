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
summary: Standard RenderScript types
description:
  Integers:<ul>
  <li>8 bit: char, int8_t</li>
  <li>16 bit: short, int16_t</li>
  <li>32 bit: int, in32_t</li>
  <li>64 bit: long, long long, int64_t</li></ul>

  Unsigned integers:<ul>
  <li>8 bit: uchar, uint8_t</li>
  <li>16 bit: ushort, uint16_t</li>
  <li>32 bit: uint, uint32_t</li>
  <li>64 bit: ulong, uint64_t</li></ul>

  Floating point:<ul>
  <li>32 bit: float</li>
  <li>64 bit: double</li></ul>

  Vectors of length 2, 3, and 4 are supported for all the types above.
include:
 #include "stdbool.h"

 #define RS_PACKED __attribute__((packed, aligned(4)))
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

constant: M_1_PI
value: 0.318309886183790671537767526745028724f
summary: 1 / pi, as a 32 bit float
description:
 The inverse of pi, as a 32 bit float.
end:

constant: M_2_PI
value: 0.636619772367581343075535053490057448f
summary: 2 / pi, as a 32 bit float
description:
 2 divided by pi, as a 32 bit float.
end:

constant: M_2_PIl
value: 0.636619772367581343075535053490057448f
hidden:
summary: Deprecated.  Use M_2_PI instead.
description:
end:

constant: M_2_SQRTPI
value: 1.128379167095512573896158903121545172f
summary:  2 / sqrt(pi), as a 32 bit float
description:
 2 divided by the square root of pi, as a 32 bit float.
end:

constant: M_E
value: 2.718281828459045235360287471352662498f
summary: e, as a 32 bit float
description:
 The number e, the base of the natural logarithm, as a 32 bit float.
end:

constant: M_LN10
value: 2.302585092994045684017991454684364208f
summary: log_e(10), as a 32 bit float
description:
 The natural logarithm of 10, as a 32 bit float.
end:

constant: M_LN2
value: 0.693147180559945309417232121458176568f
summary: log_e(2), as a 32 bit float
description:
 The natural logarithm of 2, as a 32 bit float.
end:

constant: M_LOG10E
value: 0.434294481903251827651128918916605082f
summary: log_10(e), as a 32 bit float
description:
 The logarithm base 10 of e, as a 32 bit float.
end:

constant: M_LOG2E
value: 1.442695040888963407359924681001892137f
summary: log_2(e), as a 32 bit float
description:
 The logarithm base 2 of e, as a 32 bit float.
end:

constant: M_PI
value: 3.141592653589793238462643383279502884f
summary: pi, as a 32 bit float
description:
 The constant pi, as a 32 bit float.
end:

constant: M_PI_2
value: 1.570796326794896619231321691639751442f
summary: pi / 2, as a 32 bit float
description:
 Pi divided by 2, as a 32 bit float.
end:

constant: M_PI_4
value: 0.785398163397448309615660845819875721f
summary: pi / 4, as a 32 bit float
description:
 Pi divided by 4, as a 32 bit float.
end:

constant: M_SQRT1_2
value: 0.707106781186547524400844362104849039f
summary: 1 / sqrt(2), as a 32 bit float
description:
 The inverse of the square root of 2, as a 32 bit float.
end:

constant: M_SQRT2
value: 1.414213562373095048801688724209698079f
summary: sqrt(2), as a 32 bit float
description:
 The square root of 2, as a 32 bit float.
end:

type: int8_t
simple: char
summary: 8 bit signed integer
description:
 8 bit integer type
end:

type: int16_t
simple: short
summary: 16 bit signed integer
description:
 16 bit integer type
end:

type: int32_t
simple: int
summary: 32 bit signed integer
description:
 32 bit integer type
end:

type: int64_t
version: 9 20
simple: long long
summary: 64 bit signed integer
description:
 64 bit integer type
end:

type: int64_t
version: 21
simple: long
end:

type: uint8_t
simple: unsigned char
summary: 8 bit unsigned integer
description:
 8 bit unsigned integer type
end:

type: uint16_t
simple: unsigned short
summary: 16 bit unsigned integer
description:
 16 bit unsigned integer type
end:

type: uint32_t
simple: unsigned int
summary: 32 bit unsigned integer
description:
 32 bit unsigned integer type
end:

type: uint64_t
version: 9 20
simple: unsigned long long
summary: 64 bit unsigned integer
description:
 64 bit unsigned integer type
end:

type: uint64_t
version: 21
simple: unsigned long
end:

type: uchar
simple: uint8_t
summary: 8 bit unsigned integer
description:
 8 bit unsigned integer type
end:

type: ushort
simple: uint16_t
summary: 16 bit unsigned integer
description:
 16 bit unsigned integer type
end:

type: uint
simple: uint32_t
summary: 32 bit unsigned integer
description:
 32 bit unsigned integer type
end:

type: ulong
simple: uint64_t
summary: 64 bit unsigned integer
description:
 Typedef for unsigned long (use for 64-bit unsigned integers)
end:

type: size_t
size: 64
simple: uint64_t
summary: Unsigned size type
description:
 Typedef for size_t
end:

type: size_t
size: 32
simple: uint32_t
end:

type: ssize_t
size: 64
simple: int64_t
summary: Signed size type
description:
 Typedef for ssize_t
end:

type: ssize_t
size: 32
simple: int32_t
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


type: rs_mesh
size: 32
simple: _RS_HANDLE
summary: Handle to a Mesh
description:
 Opaque handle to a RenderScript mesh object.
 See: android.renderscript.Mesh
end:

type: rs_program_fragment
size: 32
simple: _RS_HANDLE
summary: Handle to a ProgramFragment
description:
 Opaque handle to a RenderScript ProgramFragment object.
 See: android.renderscript.ProgramFragment
end:

type: rs_program_vertex
size: 32
simple: _RS_HANDLE
summary: Handle to a ProgramVertex
description:
 Opaque handle to a RenderScript ProgramVertex object.
 See: android.renderscript.ProgramVertex
end:

type: rs_program_raster
size: 32
simple: _RS_HANDLE
summary: Handle to a ProgramRaster
description:
 Opaque handle to a RenderScript ProgramRaster object.
 See: android.renderscript.ProgramRaster
end:

type: rs_program_store
size: 32
simple: _RS_HANDLE
summary: Handle to a ProgramStore
description:
 Opaque handle to a RenderScript ProgramStore object.
 See: android.renderscript.ProgramStore
end:

type: rs_font
size: 32
simple: _RS_HANDLE
summary: Handle to a Font
description:
 Opaque handle to a RenderScript font object.
 See: android.renderscript.Font
end:

type: float2
simple: float __attribute__((ext_vector_type(2)))
summary: Two 32 bit floats
description:
 Vector version of the basic float type.
 Provides two float fields packed into a single 64 bit field with 64 bit alignment.
end:

type: float3
simple: float __attribute__((ext_vector_type(3)))
summary: Three 32 bit floats
description:
 Vector version of the basic float type.
 Provides three float fields packed into a single 128 bit field with 128 bit alignment.
end:

type: float4
simple: float __attribute__((ext_vector_type(4)))
summary: Four 32 bit floats
description:
 Vector version of the basic float type.
 Provides four float fields packed into a single 128 bit field with 128 bit alignment.
end:


type: double2
simple: double __attribute__((ext_vector_type(2)))
summary: Two 64 bit floats
description:
 Vector version of the basic double type. Provides two double fields packed
 into a single 128 bit field with 128 bit alignment.
end:

type: double3
simple: double __attribute__((ext_vector_type(3)))
summary: Three 64 bit floats
description:
 Vector version of the basic double type. Provides three double fields packed
 into a single 256 bit field with 256 bit alignment.
end:

type: double4
simple: double __attribute__((ext_vector_type(4)))
summary: Four 64 bit floats
description:
 Vector version of the basic double type. Provides four double fields packed
 into a single 256 bit field with 256 bit alignment.
end:


type: uchar2
simple: uchar __attribute__((ext_vector_type(2)))
summary: Two 8 bit unsigned integers
description:
 Vector version of the basic uchar type. Provides two uchar fields packed
 into a single 16 bit field with 16 bit alignment.
end:

type: uchar3
simple: uchar __attribute__((ext_vector_type(3)))
summary: Three 8 bit unsigned integers
description:
 Vector version of the basic uchar type. Provides three uchar fields packed
 into a single 32 bit field with 32 bit alignment.
end:

type: uchar4
simple: uchar __attribute__((ext_vector_type(4)))
summary: Four 8 bit unsigned integers
description:
 Vector version of the basic uchar type. Provides four uchar fields packed
 into a single 32 bit field with 32 bit alignment.
end:


type: ushort2
simple: ushort __attribute__((ext_vector_type(2)))
summary: Two 16 bit unsigned integers
description:
 Vector version of the basic ushort type. Provides two ushort fields packed
 into a single 32 bit field with 32 bit alignment.
end:

type: ushort3
simple: ushort __attribute__((ext_vector_type(3)))
summary: Three 16 bit unsigned integers
description:
 Vector version of the basic ushort type. Provides three ushort fields packed
 into a single 64 bit field with 64 bit alignment.
end:

type: ushort4
simple: ushort __attribute__((ext_vector_type(4)))
summary: Four 16 bit unsigned integers
description:
 Vector version of the basic ushort type. Provides four ushort fields packed
 into a single 64 bit field with 64 bit alignment.
end:


type: uint2
simple: uint __attribute__((ext_vector_type(2)))
summary: Two 32 bit unsigned integers
description:
 Vector version of the basic uint type. Provides two uint fields packed into a
 single 64 bit field with 64 bit alignment.
end:

type: uint3
simple: uint __attribute__((ext_vector_type(3)))
summary: Three 32 bit unsigned integers
description:
 Vector version of the basic uint type. Provides three uint fields packed into
 a single 128 bit field with 128 bit alignment.
end:

type: uint4
simple: uint __attribute__((ext_vector_type(4)))
summary: Four 32 bit unsigned integers
description:
 Vector version of the basic uint type. Provides four uint fields packed into
 a single 128 bit field with 128 bit alignment.
end:


type: ulong2
simple: ulong __attribute__((ext_vector_type(2)))
summary: Two 64 bit unsigned integers
description:
 Vector version of the basic ulong type. Provides two ulong fields packed into
 a single 128 bit field with 128 bit alignment.
end:

type: ulong3
simple: ulong __attribute__((ext_vector_type(3)))
summary: Three 64 bit unsigned integers
description:
 Vector version of the basic ulong type. Provides three ulong fields packed
 into a single 256 bit field with 256 bit alignment.
end:

type: ulong4
simple: ulong __attribute__((ext_vector_type(4)))
summary: Four 64 bit unsigned integers
description:
 Vector version of the basic ulong type. Provides four ulong fields packed
 into a single 256 bit field with 256 bit alignment.
end:


type: char2
simple: char __attribute__((ext_vector_type(2)))
summary: Two 8 bit signed integers
description:
 Vector version of the basic char type. Provides two char fields packed into a
 single 16 bit field with 16 bit alignment.
end:

type: char3
simple: char __attribute__((ext_vector_type(3)))
summary: Three 8 bit signed integers
description:
 Vector version of the basic char type. Provides three char fields packed into
 a single 32 bit field with 32 bit alignment.
end:

type: char4
simple: char __attribute__((ext_vector_type(4)))
summary: Four 8 bit signed integers
description:
 Vector version of the basic char type. Provides four char fields packed into
 a single 32 bit field with 32 bit alignment.
end:

type: short2
simple: short __attribute__((ext_vector_type(2)))
summary: Two 16 bit signed integers
description:
 Vector version of the basic short type. Provides two short fields packed into
 a single 32 bit field with 32 bit alignment.
end:

type: short3
simple: short __attribute__((ext_vector_type(3)))
summary: Three 16 bit signed integers
description:
 Vector version of the basic short type. Provides three short fields packed
 into a single 64 bit field with 64 bit alignment.
end:

type: short4
simple: short __attribute__((ext_vector_type(4)))
summary: Four 16 bit signed integers
description:
 Vector version of the basic short type. Provides four short fields packed
 into a single 64 bit field with 64 bit alignment.
end:


type: int2
simple: int __attribute__((ext_vector_type(2)))
summary: Two 32 bit signed integers
description:
 Vector version of the basic int type. Provides two int fields packed into a
 single 64 bit field with 64 bit alignment.
end:

type: int3
simple: int __attribute__((ext_vector_type(3)))
summary: Three 32 bit signed integers
description:
 Vector version of the basic int type. Provides three int fields packed into a
 single 128 bit field with 128 bit alignment.
end:

type: int4
simple: int __attribute__((ext_vector_type(4)))
summary: Four 32 bit signed integers
description:
 Vector version of the basic int type. Provides two four fields packed into a
 single 128 bit field with 128 bit alignment.
end:


type: long2
simple: long __attribute__((ext_vector_type(2)))
summary: Two 64 bit signed integers
description:
 Vector version of the basic long type. Provides two long fields packed into a
 single 128 bit field with 128 bit alignment.
end:

type: long3
simple: long __attribute__((ext_vector_type(3)))
summary: Three 64 bit signed integers
description:
 Vector version of the basic long type. Provides three long fields packed into
 a single 256 bit field with 256 bit alignment.
end:

type: long4
simple: long __attribute__((ext_vector_type(4)))
summary: Four 64 bit signed integers
description:
 Vector version of the basic long type. Provides four long fields packed into
 a single 256 bit field with 256 bit alignment.
end:

type: rs_matrix4x4
struct:
field: float m[16]
summary: 4x4 matrix of 32 bit floats
description:
 Native holder for RS matrix.  Elements are stored in the array at the
 location [row*4 + col]
end:

type: rs_matrix3x3
struct:
field: float m[9]
summary: 3x3 matrix of 32 bit floats
description:
 Native holder for RS matrix.  Elements are stored in the array at the
 location [row*3 + col]
end:

type: rs_matrix2x2
struct:
field: float m[4]
summary: 2x2 matrix of 32 bit floats
description:
 Native holder for RS matrix.  Elements are stored in the array at the
 location [row*2 + col]
end:

type: rs_quaternion
simple: float4
summary: Quarternion
description:
 Quaternion type for use with the quaternion functions
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
value: RS_ALLOCATION_USAGE_SCRIPT = 0x0001
value: RS_ALLOCATION_USAGE_GRAPHICS_TEXTURE = 0x0002, "Deprecated."
value: RS_ALLOCATION_USAGE_GRAPHICS_VERTEX = 0x0004, "Deprecated."
value: RS_ALLOCATION_USAGE_GRAPHICS_CONSTANTS = 0x0008, "Deprecated."
value: RS_ALLOCATION_USAGE_GRAPHICS_RENDER_TARGET = 0x0010, "Deprecated."
summary: Bitfield to specify the usage types for an allocation
description:
 These values are ORed together to specify which usages or memory spaces are
 relevant to an allocation or an operation on an allocation.
end:

type: rs_primitive
version: 16
size: 32
enum:
value: RS_PRIMITIVE_POINT = 0, "Vertex data will be rendered as a series of points"
value: RS_PRIMITIVE_LINE = 1, "Vertex pairs will be rendered as lines"
value: RS_PRIMITIVE_LINE_STRIP = 2, "Vertex data will be rendered as a connected line strip"
value: RS_PRIMITIVE_TRIANGLE = 3, "Vertices will be rendered as individual triangles"
value: RS_PRIMITIVE_TRIANGLE_STRIP = 4, "Vertices will be rendered as a connected triangle strip defined by the first three vertices with each additional triangle defined by a new vertex"
value: RS_PRIMITIVE_TRIANGLE_FAN = 5, "Vertices will be rendered as a sequence of triangles that all share first vertex as the origin"
value: RS_PRIMITIVE_INVALID = 100, "Invalid primitive"
summary: How to intepret mesh vertex data
description:
 Describes the way mesh vertex data is interpreted when rendering
end:

type: rs_data_type
version: 16
enum:
value: RS_TYPE_NONE             = 0
value: RS_TYPE_FLOAT_32         = 2
value: RS_TYPE_FLOAT_64         = 3
value: RS_TYPE_SIGNED_8         = 4
value: RS_TYPE_SIGNED_16        = 5
value: RS_TYPE_SIGNED_32        = 6
value: RS_TYPE_SIGNED_64        = 7
value: RS_TYPE_UNSIGNED_8       = 8
value: RS_TYPE_UNSIGNED_16      = 9
value: RS_TYPE_UNSIGNED_32      = 10
value: RS_TYPE_UNSIGNED_64      = 11
value: RS_TYPE_BOOLEAN          = 12
value: RS_TYPE_UNSIGNED_5_6_5   = 13
value: RS_TYPE_UNSIGNED_5_5_5_1 = 14
value: RS_TYPE_UNSIGNED_4_4_4_4 = 15
value: RS_TYPE_MATRIX_4X4       = 16
value: RS_TYPE_MATRIX_3X3       = 17
value: RS_TYPE_MATRIX_2X2       = 18
value: RS_TYPE_ELEMENT          = 1000
value: RS_TYPE_TYPE             = 1001
value: RS_TYPE_ALLOCATION       = 1002
value: RS_TYPE_SAMPLER          = 1003
value: RS_TYPE_SCRIPT           = 1004
value: RS_TYPE_MESH             = 1005
value: RS_TYPE_PROGRAM_FRAGMENT = 1006
value: RS_TYPE_PROGRAM_VERTEX   = 1007
value: RS_TYPE_PROGRAM_RASTER   = 1008
value: RS_TYPE_PROGRAM_STORE    = 1009
value: RS_TYPE_FONT             = 1010
value: RS_TYPE_INVALID          = 10000
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

type: rs_depth_func
version: 16
size: 32
enum:
value: RS_DEPTH_FUNC_ALWAYS        = 0, "Always drawn"
value: RS_DEPTH_FUNC_LESS          = 1, "Drawn if the incoming depth value is less than that in the depth buffer"
value: RS_DEPTH_FUNC_LEQUAL        = 2, "Drawn if the incoming depth value is less or equal to that in the depth buffer"
value: RS_DEPTH_FUNC_GREATER       = 3, "Drawn if the incoming depth value is greater than that in the depth buffer"
value: RS_DEPTH_FUNC_GEQUAL        = 4, "Drawn if the incoming depth value is greater or equal to that in the depth buffer"
value: RS_DEPTH_FUNC_EQUAL         = 5, "Drawn if the incoming depth value is equal to that in the depth buffer"
value: RS_DEPTH_FUNC_NOTEQUAL      = 6, "Drawn if the incoming depth value is not equal to that in the depth buffer"
value: RS_DEPTH_FUNC_INVALID       = 100, "Invalid depth function"
summary: Depth function
description:
 Specifies conditional drawing depending on the comparison of the incoming
 depth to that found in the depth buffer.
end:

type: rs_blend_src_func
version: 16
size: 32
enum:
value: RS_BLEND_SRC_ZERO                   = 0
value: RS_BLEND_SRC_ONE                    = 1
value: RS_BLEND_SRC_DST_COLOR              = 2
value: RS_BLEND_SRC_ONE_MINUS_DST_COLOR    = 3
value: RS_BLEND_SRC_SRC_ALPHA              = 4
value: RS_BLEND_SRC_ONE_MINUS_SRC_ALPHA    = 5
value: RS_BLEND_SRC_DST_ALPHA              = 6
value: RS_BLEND_SRC_ONE_MINUS_DST_ALPHA    = 7
value: RS_BLEND_SRC_SRC_ALPHA_SATURATE     = 8
value: RS_BLEND_SRC_INVALID                = 100
summary: Blend source function
description:
end:

type: rs_blend_dst_func
version: 16
size: 32
enum:
value: RS_BLEND_DST_ZERO                   = 0
value: RS_BLEND_DST_ONE                    = 1
value: RS_BLEND_DST_SRC_COLOR              = 2
value: RS_BLEND_DST_ONE_MINUS_SRC_COLOR    = 3
value: RS_BLEND_DST_SRC_ALPHA              = 4
value: RS_BLEND_DST_ONE_MINUS_SRC_ALPHA    = 5
value: RS_BLEND_DST_DST_ALPHA              = 6
value: RS_BLEND_DST_ONE_MINUS_DST_ALPHA    = 7
value: RS_BLEND_DST_INVALID                = 100
summary: Blend destination function
description:
end:

type: rs_cull_mode
version: 16
size: 32
enum:
value: RS_CULL_BACK     = 0
value: RS_CULL_FRONT    = 1
value: RS_CULL_NONE     = 2
value: RS_CULL_INVALID  = 100
summary: Culling mode
description:
end:

type: rs_sampler_value
version: 16
enum:
value: RS_SAMPLER_NEAREST              = 0
value: RS_SAMPLER_LINEAR               = 1
value: RS_SAMPLER_LINEAR_MIP_LINEAR    = 2
value: RS_SAMPLER_WRAP                 = 3
value: RS_SAMPLER_CLAMP                = 4
value: RS_SAMPLER_LINEAR_MIP_NEAREST   = 5
value: RS_SAMPLER_MIRRORED_REPEAT      = 6
value: RS_SAMPLER_INVALID              = 100
summary: Sampler wrap T value
description:
end:
