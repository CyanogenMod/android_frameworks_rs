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
summary: Allocation routines
description:
 TODO Adjust documentation.

 Functions that can be used to query the characteristics of an allocation,
 to set and get elements of the allocation.
end:

function: rsAllocationCopy1DRange
version: 14
ret: void
arg: rs_allocation dstAlloc, "Allocation to copy data into."
arg: uint32_t dstOff, "The offset of the first element to be copied in the destination allocation."
arg: uint32_t dstMip, "Mip level in the destination allocation."
arg: uint32_t count, "The number of elements to be copied."
arg: rs_allocation srcAlloc, "The source data allocation."
arg: uint32_t srcOff, "The offset of the first element in data to be copied in the source allocation."
arg: uint32_t srcMip, "Mip level in the source allocation."
summary: Copy consecutive values between allocations
description:
 Copies part of an allocation into another allocation.

 The two allocations must be different.  Using this function to copy whithin
 the same allocation yields undefined results.
test: none
end:

function: rsAllocationCopy2DRange
version: 14
ret: void
arg: rs_allocation dstAlloc, "Allocation to copy data into."
arg: uint32_t dstXoff, "X offset of the region to update in the destination allocation."
arg: uint32_t dstYoff, "Y offset of the region to update in the destination allocation."
arg: uint32_t dstMip, "Mip level in the destination allocation."
arg: rs_allocation_cubemap_face dstFace, "Cubemap face of the destination allocation, ignored for allocations that aren't cubemaps."
arg: uint32_t width, "Width of the incoming region to update."
arg: uint32_t height, "Height of the incoming region to update."
arg: rs_allocation srcAlloc, "The source data allocation."
arg: uint32_t srcXoff, "X offset in data of the source allocation."
arg: uint32_t srcYoff, "Y offset in data of the source allocation."
arg: uint32_t srcMip, "Mip level in the source allocation."
arg: rs_allocation_cubemap_face srcFace, "Cubemap face of the source allocation, ignored for allocations that aren't cubemaps."
summary: Copy a rectangular region between allocations
description:
 Copy a rectangular region into the allocation from another allocation.

 The two allocations must be different.  Using this function to copy whithin
 the same allocation yields undefined results.
test: none
end:

function: rsAllocationVLoadX_#2#1
version: 22
w: 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: #2#1
arg: rs_allocation a
arg: uint32_t x
summary:
description:
 Get a single element from an allocation.
test: none
end:

function: rsAllocationVLoadX_#2#1
version: 22
w: 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: #2#1
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y
test: none
end:

function: rsAllocationVLoadX_#2#1
version: 22
w: 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: #2#1
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y
arg: uint32_t z
test: none
end:

function: rsAllocationVStoreX_#2#1
version: 22
w: 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: void
arg: rs_allocation a
arg: #2#1 val
arg: uint32_t x
summary:
description:
 Set a single element of an allocation.
test: none
end:

function: rsAllocationVStoreX_#2#1
version: 22
w: 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: void
arg: rs_allocation a
arg: #2#1 val
arg: uint32_t x
arg: uint32_t y
test: none
end:

function: rsAllocationVStoreX_#2#1
version: 22
w: 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: void
arg: rs_allocation a
arg: #2#1 val
arg: uint32_t x
arg: uint32_t y
arg: uint32_t z
test: none
end:

function: rsGetElementAt
ret: const void*
arg: rs_allocation a
arg: uint32_t x
summary: Get an element
description:
 Extract a single element from an allocation.
test: none
end:

function: rsGetElementAt
ret: const void*
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y
test: none
end:

function: rsGetElementAt
ret: const void*
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y
arg: uint32_t z
test: none
end:

function: rsGetElementAt_#2#1
version: 9 17
w: 1, 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: #2#1
arg: rs_allocation a
arg: uint32_t x
inline:
 return ((#2#1 *)rsGetElementAt(a, x))[0];
test: none
end:

function: rsGetElementAt_#2#1
version: 9 17
w: 1, 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: #2#1
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y
inline:
 return ((#2#1 *)rsGetElementAt(a, x, y))[0];
test: none
end:

function: rsGetElementAt_#2#1
version: 9 17
w: 1, 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: #2#1
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y
arg: uint32_t z
inline:
 return ((#2#1 *)rsGetElementAt(a, x, y, z))[0];
test: none
end:

function: rsGetElementAt_#2#1
version: 18
w: 1, 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: #2#1
arg: rs_allocation a
arg: uint32_t x
test: none
end:

function: rsGetElementAt_#2#1
version: 18
w: 1, 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: #2#1
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y
test: none
end:

function: rsGetElementAt_#2#1
version: 18
w: 1, 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: #2#1
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y
arg: uint32_t z
test: none
end:

function: rsGetElementAtYuv_uchar_U
version: 18
ret: uchar
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y
summary:
description:
 Extract a single element from an allocation.

 Coordinates are in the dimensions of the Y plane
test: none
end:

function: rsGetElementAtYuv_uchar_V
version: 18
ret: uchar
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y
summary:
description:
 Extract a single element from an allocation.

 Coordinates are in the dimensions of the Y plane
test: none
end:

function: rsGetElementAtYuv_uchar_Y
version: 18
ret: uchar
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y
summary:
description:
 Extract a single element from an allocation.
test: none
end:

function: rsSample
version: 16
ret: float4
arg: rs_allocation a, "allocation to sample from"
arg: rs_sampler s, "sampler state"
arg: float location, "location to sample from"
summary:
description:
 Fetch allocation in a way described by the sampler

 If your allocation is 1D, use the variant with float for location.
 For 2D, use the float2 variant.
test: none
end:

function: rsSample
version: 16
ret: float4
arg: rs_allocation a
arg: rs_sampler s
arg: float location
arg: float lod, "mip level to sample from, for fractional values mip levels will be interpolated if RS_SAMPLER_LINEAR_MIP_LINEAR is used"
test: none
end:

function: rsSample
version: 16
ret: float4
arg: rs_allocation a
arg: rs_sampler s
arg: float2 location
test: none
end:

function: rsSample
version: 16
ret: float4
arg: rs_allocation a
arg: rs_sampler s
arg: float2 location
arg: float lod
test: none
end:

function: rsSetElementAt
version: 18
ret: void
arg: rs_allocation a
arg: void* ptr
arg: uint32_t x
summary: Set an element
description:
 Set single element of an allocation.
test: none
end:

function: rsSetElementAt
version: 18
ret: void
arg: rs_allocation a
arg: void* ptr
arg: uint32_t x
arg: uint32_t y
test: none
end:

function: rsSetElementAt_#2#1
version: 18
w: 1, 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: void
arg: rs_allocation a
arg: #2#1 val
arg: uint32_t x
test: none
end:

function: rsSetElementAt_#2#1
version: 18
w: 1, 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: void
arg: rs_allocation a
arg: #2#1 val
arg: uint32_t x
arg: uint32_t y
test: none
end:

function: rsSetElementAt_#2#1
version: 18
w: 1, 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: void
arg: rs_allocation a
arg: #2#1 val
arg: uint32_t x
arg: uint32_t y
arg: uint32_t z
test: none
end:
