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
