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
summary: Quaternion routines
description:
end:

function: rsQuaternionAdd
ret: void
arg: rs_quaternion* q, "destination quaternion to add to"
arg: const rs_quaternion* rhs, "right hand side quaternion to add"
summary:
description:
 Add two quaternions
inline:
 q->w *= rhs->w;
 q->x *= rhs->x;
 q->y *= rhs->y;
 q->z *= rhs->z;
test: none
end:

function: rsQuaternionConjugate
ret: void
arg: rs_quaternion* q, "quaternion to conjugate"
summary:
description:
 Conjugates the quaternion
inline:
 q->x = -q->x;
 q->y = -q->y;
 q->z = -q->z;
test: none
end:

function: rsQuaternionDot
ret: float, "dot product between q0 and q1"
arg: const rs_quaternion* q0, "first quaternion"
arg: const rs_quaternion* q1, "second quaternion"
summary:
description:
 Dot product of two quaternions
inline:
 return q0->w*q1->w + q0->x*q1->x + q0->y*q1->y + q0->z*q1->z;
test: none
end:

function: rsQuaternionGetMatrixUnit
ret: void
arg: rs_matrix4x4* m, "resulting matrix"
arg: const rs_quaternion* q, "normalized quaternion"
summary:
description:
 Computes rotation matrix from the normalized quaternion
inline:
 float xx = q->x * q->x;
 float xy = q->x * q->y;
 float xz = q->x * q->z;
 float xw = q->x * q->w;
 float yy = q->y * q->y;
 float yz = q->y * q->z;
 float yw = q->y * q->w;
 float zz = q->z * q->z;
 float zw = q->z * q->w;

 m->m[0]  = 1.0f - 2.0f * ( yy + zz );
 m->m[4]  =        2.0f * ( xy - zw );
 m->m[8]  =        2.0f * ( xz + yw );
 m->m[1]  =        2.0f * ( xy + zw );
 m->m[5]  = 1.0f - 2.0f * ( xx + zz );
 m->m[9]  =        2.0f * ( yz - xw );
 m->m[2]  =        2.0f * ( xz - yw );
 m->m[6]  =        2.0f * ( yz + xw );
 m->m[10] = 1.0f - 2.0f * ( xx + yy );
 m->m[3]  = m->m[7] = m->m[11] = m->m[12] = m->m[13] = m->m[14] = 0.0f;
 m->m[15] = 1.0f;
test: none
end:

function: rsQuaternionLoadRotateUnit
ret: void
arg: rs_quaternion* q, "quaternion to set"
arg: float rot, "rot angle to rotate by"
arg: float x, "component of a vector"
arg: float y, "component of a vector"
arg: float z, "component of a vector"
summary:
description:
 Loads a quaternion that represents a rotation about an arbitrary unit vector
inline:
 rot *= (float)(M_PI / 180.0f) * 0.5f;
 float c = cos(rot);
 float s = sin(rot);

 q->w = c;
 q->x = x * s;
 q->y = y * s;
 q->z = z * s;
test: none
end:

function: rsQuaternionSet
ret: void
arg: rs_quaternion* q, "destination quaternion"
arg: float w, "component"
arg: float x, "component"
arg: float y, "component"
arg: float z, "component"
summary:
description:
 Set the quaternion from components or from another quaternion.
inline:
 q->w = w;
 q->x = x;
 q->y = y;
 q->z = z;
test: none
end:

function: rsQuaternionSet
ret: void
arg: rs_quaternion* q
arg: const rs_quaternion* rhs, "source quaternion"
inline:
 q->w = rhs->w;
 q->x = rhs->x;
 q->y = rhs->y;
 q->z = rhs->z;
test: none
end:

# NOTE: The following inline definitions depend on each other.  The order must be preserved
# for the compilation to work.

function: rsQuaternionLoadRotate
ret: void
arg: rs_quaternion* q, "quaternion to set"
arg: float rot, "angle to rotate by"
arg: float x, "component of a vector"
arg: float y, "component of a vector"
arg: float z, "component of a vector"
summary:
description:
 Loads a quaternion that represents a rotation about an arbitrary vector
 (doesn't have to be unit)
inline:
 const float len = x*x + y*y + z*z;
 if (len != 1) {
     const float recipLen = 1.f / sqrt(len);
     x *= recipLen;
     y *= recipLen;
     z *= recipLen;
 }
 rsQuaternionLoadRotateUnit(q, rot, x, y, z);
test: none
end:

function: rsQuaternionNormalize
ret: void
arg: rs_quaternion* q, "quaternion to normalize"
summary:
description:
 Normalizes the quaternion
inline:
 const float len = rsQuaternionDot(q, q);
 if (len != 1) {
     const float recipLen = 1.f / sqrt(len);
     q->w *= recipLen;
     q->x *= recipLen;
     q->y *= recipLen;
     q->z *= recipLen;
 }
test: none
end:

function: rsQuaternionMultiply
ret: void
arg: rs_quaternion* q, "destination quaternion"
arg: float s, "scalar"
summary:
description:
 Multiply quaternion by a scalar or another quaternion
inline:
 q->w *= s;
 q->x *= s;
 q->y *= s;
 q->z *= s;
test: none
end:

function: rsQuaternionMultiply
ret: void
arg: rs_quaternion* q
arg: const rs_quaternion* rhs, "right hand side quaternion to multiply by"
inline:
 rs_quaternion qtmp;
 rsQuaternionSet(&qtmp, q);

 q->w = qtmp.w*rhs->w - qtmp.x*rhs->x - qtmp.y*rhs->y - qtmp.z*rhs->z;
 q->x = qtmp.w*rhs->x + qtmp.x*rhs->w + qtmp.y*rhs->z - qtmp.z*rhs->y;
 q->y = qtmp.w*rhs->y + qtmp.y*rhs->w + qtmp.z*rhs->x - qtmp.x*rhs->z;
 q->z = qtmp.w*rhs->z + qtmp.z*rhs->w + qtmp.x*rhs->y - qtmp.y*rhs->x;
 rsQuaternionNormalize(q);
test: none
end:

function: rsQuaternionSlerp
ret: void
arg: rs_quaternion* q, "result quaternion from interpolation"
arg: const rs_quaternion* q0, "first param"
arg: const rs_quaternion* q1, "second param"
arg: float t, "how much to interpolate by"
summary:
description:
 Performs spherical linear interpolation between two quaternions
inline:
 if (t <= 0.0f) {
     rsQuaternionSet(q, q0);
     return;
 }
 if (t >= 1.0f) {
     rsQuaternionSet(q, q1);
     return;
 }

 rs_quaternion tempq0, tempq1;
 rsQuaternionSet(&tempq0, q0);
 rsQuaternionSet(&tempq1, q1);

 float angle = rsQuaternionDot(q0, q1);
 if (angle < 0) {
     rsQuaternionMultiply(&tempq0, -1.0f);
     angle *= -1.0f;
 }

 float scale, invScale;
 if (angle + 1.0f > 0.05f) {
     if (1.0f - angle >= 0.05f) {
         float theta = acos(angle);
         float invSinTheta = 1.0f / sin(theta);
         scale = sin(theta * (1.0f - t)) * invSinTheta;
         invScale = sin(theta * t) * invSinTheta;
     } else {
         scale = 1.0f - t;
         invScale = t;
     }
 } else {
     rsQuaternionSet(&tempq1, tempq0.z, -tempq0.y, tempq0.x, -tempq0.w);
     scale = sin(M_PI * (0.5f - t));
     invScale = sin(M_PI * t);
 }

 rsQuaternionSet(q, tempq0.w*scale + tempq1.w*invScale, tempq0.x*scale + tempq1.x*invScale,
                     tempq0.y*scale + tempq1.y*invScale, tempq0.z*scale + tempq1.z*invScale);
test: none
end:
