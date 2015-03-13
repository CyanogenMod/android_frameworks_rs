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
summary: TODO Add documentation
description:
 TODO Add documentation
end:

function: rsClamp
# TODO Why always_inline?
attrib: const, always_inline
t: i8, i16, i32, u8, u16, u32
ret: #1
arg: #1 amount, "The value to clamp"
arg: #1 low, "Lower bound"
arg: #1 high, "Upper bound"
summary: Restrain a value to a range
description:
 Clamp a value between low and high.

 Deprecated.  Use @clamp() instead.
test: none
end:

function: rsExtractFrustumPlanes
# TODO Why always_inline?
attrib: always_inline
ret: void
arg: const rs_matrix4x4* viewProj, "matrix to extract planes from"
arg: float4* left, "left plane"
arg: float4* right, "right plane"
arg: float4* top, "top plane"
arg: float4* bottom, "bottom plane"
arg: float4* near, "near plane"
arg: float4* far, "far plane"
summary:
description:
 Computes 6 frustum planes from the view projection matrix
inline:
 // x y z w = a b c d in the plane equation
 left->x = viewProj->m[3] + viewProj->m[0];
 left->y = viewProj->m[7] + viewProj->m[4];
 left->z = viewProj->m[11] + viewProj->m[8];
 left->w = viewProj->m[15] + viewProj->m[12];

 right->x = viewProj->m[3] - viewProj->m[0];
 right->y = viewProj->m[7] - viewProj->m[4];
 right->z = viewProj->m[11] - viewProj->m[8];
 right->w = viewProj->m[15] - viewProj->m[12];

 top->x = viewProj->m[3] - viewProj->m[1];
 top->y = viewProj->m[7] - viewProj->m[5];
 top->z = viewProj->m[11] - viewProj->m[9];
 top->w = viewProj->m[15] - viewProj->m[13];

 bottom->x = viewProj->m[3] + viewProj->m[1];
 bottom->y = viewProj->m[7] + viewProj->m[5];
 bottom->z = viewProj->m[11] + viewProj->m[9];
 bottom->w = viewProj->m[15] + viewProj->m[13];

 near->x = viewProj->m[3] + viewProj->m[2];
 near->y = viewProj->m[7] + viewProj->m[6];
 near->z = viewProj->m[11] + viewProj->m[10];
 near->w = viewProj->m[15] + viewProj->m[14];

 far->x = viewProj->m[3] - viewProj->m[2];
 far->y = viewProj->m[7] - viewProj->m[6];
 far->z = viewProj->m[11] - viewProj->m[10];
 far->w = viewProj->m[15] - viewProj->m[14];

 float len = length(left->xyz);
 *left /= len;
 len = length(right->xyz);
 *right /= len;
 len = length(top->xyz);
 *top /= len;
 len = length(bottom->xyz);
 *bottom /= len;
 len = length(near->xyz);
 *near /= len;
 len = length(far->xyz);
 *far /= len;
test: none
end:

function: rsFrac
attrib: const
ret: float
arg: float v
summary:
description:
 Returns the fractional part of a float
test: none
end:

function: rsIsSphereInFrustum
attrib: always_inline
ret: bool
arg: float4* sphere, "float4 representing the sphere"
arg: float4* left, "left plane"
arg: float4* right, "right plane"
arg: float4* top, "top plane"
arg: float4* bottom, "bottom plane"
arg: float4* near, "near plane"
arg: float4* far, "far plane"
summary:
description:
 Checks if a sphere is withing the 6 frustum planes
inline:
 float distToCenter = dot(left->xyz, sphere->xyz) + left->w;
 if (distToCenter < -sphere->w) {
     return false;
 }
 distToCenter = dot(right->xyz, sphere->xyz) + right->w;
 if (distToCenter < -sphere->w) {
     return false;
 }
 distToCenter = dot(top->xyz, sphere->xyz) + top->w;
 if (distToCenter < -sphere->w) {
     return false;
 }
 distToCenter = dot(bottom->xyz, sphere->xyz) + bottom->w;
 if (distToCenter < -sphere->w) {
     return false;
 }
 distToCenter = dot(near->xyz, sphere->xyz) + near->w;
 if (distToCenter < -sphere->w) {
     return false;
 }
 distToCenter = dot(far->xyz, sphere->xyz) + far->w;
 if (distToCenter < -sphere->w) {
     return false;
 }
 return true;
test: none
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

function: rsRand
ret: int
arg: int max_value
summary:
description:
 Return a random value between 0 (or min_value) and max_malue.
test: none
end:

function: rsRand
ret: int
arg: int min_value
arg: int max_value
test: none
end:

function: rsRand
ret: float
arg: float max_value
test: none
end:

function: rsRand
ret: float
arg: float min_value
arg: float max_value
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
