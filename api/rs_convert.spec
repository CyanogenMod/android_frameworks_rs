#
# Copyright (C) 2014 The Android Open Source Project
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
summary: Conversion functions
description:
 TODO Add desc.
end:

function: convert_#3#1
version: 9
attrib: const
w: 2, 3, 4
t: u8, u16, u32, i8, i16, i32, f32
t: u8, u16, u32, i8, i16, i32, f32
ret: #3#1
arg: #2#1 v, compatible(#3)
summary: Converts numerical vectors
description:
 Component wise conversion from a numerical type to another.

 Conversions of floating point values to integer will truncate.

 Conversions of numbers too large to fit the destination type yield undefined results.
 For example, converting a float that contains 1.0e18 to a short is undefined.
 Use @clamp() to avoid this.
end:

function: convert_#3#1
version: 21
attrib: const
w: 2, 3, 4
t: u64, i64, f64
t: u64, i64, f64
ret: #3#1
arg: #2#1 v, compatible(#3)
end:

function: convert_#3#1
version: 21
attrib: const
w: 2, 3, 4
t: u64, i64, f64
t: u8, u16, u32, i8, i16, i32, f32
ret: #3#1
arg: #2#1 v, compatible(#3)
end:

function: convert_#3#1
version: 21
attrib: const
w: 2, 3, 4
t: u8, u16, u32, i8, i16, i32, f32
t: u64, i64, f64
ret: #3#1
arg: #2#1 v, compatible(#3)
end:

function: rsPackColorTo8888
attrib: const
ret: uchar4
arg: float r
arg: float g
arg: float b
summary:
description:
 Pack floating point (0-1) RGB values into a uchar4.

 For the float3 variant and the variant that only specifies r, g, b,
 the alpha component is set to 255 (1.0).
test: none
end:

function: rsPackColorTo8888
attrib: const
ret: uchar4
arg: float r
arg: float g
arg: float b
arg: float a
test: none
end:

function: rsPackColorTo8888
attrib: const
ret: uchar4
arg: float3 color
test: none
end:

function: rsPackColorTo8888
attrib: const
ret: uchar4
arg: float4 color
test: none
end:

function: rsUnpackColor8888
attrib: =const
ret: float4
arg: uchar4 c
summary:
description:
 Unpack a uchar4 color to float4.  The resulting float range will be (0-1).
test: none
end:

function: rsYuvToRGBA_#2#1
attrib: const
w: 4
t: u8, f32
ret: #2#1
arg: uchar y
arg: uchar u
arg: uchar v
summary:
description:
 Convert from YUV to RGBA.
test: none
end:
