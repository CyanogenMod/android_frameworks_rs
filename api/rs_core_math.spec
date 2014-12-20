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

start:
w: 1, 2, 3, 4
t: i8, i16, i32
name: abs
ret: u#2#1
arg: #2#1 v
comment:
 Returns the absolute value of an integer.

 For floats, use fabs().
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: acos
ret: #2#1
arg: #2#1 v range(-1,1)
comment:
 Returns the inverse cosine, in radians.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: acosh
ret: #2#1
arg: #2#1 v
comment:
 Returns the inverse hyperbolic cosine, in radians.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: acospi
ret: #2#1
arg: #2#1 v range(-1,1)
comment:
 Returns the inverse cosine in radians, divided by pi.

 To get an inverse cosine measured in degrees, use acospi(a) * 180.f.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: asin
ret: #2#1
arg: #2#1 v range(-1,1)
comment:
 Returns the inverse sine, in radians.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: asinh
ret: #2#1
arg: #2#1 v
comment:
 Returns the inverse hyperbolic sine, in radians.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: asinpi
ret: #2#1
arg: #2#1 v range(-1,1)
comment:
 Returns the inverse sine in radians, divided by pi.

 To get an inverse sine measured in degrees, use asinpi(a) * 180.f.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: atan
ret: #2#1
arg: #2#1 v range(-1,1)
comment:
 Returns the inverse tangent, in radians.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: atan2
ret: #2#1
arg: #2#1 numerator
arg: #2#1 denominator
comment:
 Returns the inverse tangent of (numerator / denominator), in radians.

 denominator can be 0.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: atan2pi
ret: #2#1
arg: #2#1 numerator
arg: #2#1 denominator
comment:
 Returns the inverse tangent of (numerator / denominator), in radians, divided by pi.

 To get an inverse tangent measured in degrees, use atan2pi(n, d) * 180.f.

 denominator can be 0.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: atanh
ret: #2#1
arg: #2#1 v range(-1,1)
comment:
 Returns the inverse hyperbolic tangent, in radians.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: atanpi
ret: #2#1
arg: #2#1 v range(-1,1)
comment:
 Returns the inverse tangent in radians, divided by pi.

 To get an inverse tangent measured in degrees, use atanpi(a) * 180.f.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: cbrt
ret: #2#1
arg: #2#1 v
comment:
 Returns the cube root.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: ceil
ret: #2#1
arg: #2#1 v
comment:
 Returns the smallest integer not less than a value.

 For example, ceil(1.2f) returns 2.f, and ceil(-1.2f) returns -1.f.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: clamp
ret: #2#1
arg: #2#1 value
arg: #2#1 min_value
arg: #2#1 max_value above(min_value)
comment:
 Clamps a value to a specified high and low bound.

 clamp() returns min_value if value < min_value, max_value if value > max_value, otherwise value.

 If min_value is greater than max_value, the results are undefined.

 @param value Value to be clamped.  Supports 1, 2, 3, 4 components.
 @param min_value Lower bound, must be scalar or matching vector.
 @param max_value High bound, must match the type of low.
version: 9
end:

start:
w: 2, 3, 4
t: f32
name: clamp
ret: #2#1
arg: #2#1 value
arg: #2 min_value
arg: #2 max_value above(min_value)
comment:
 Clamps a value to a specified high and low bound.

 clamp() returns min_value if value < min_value, max_value if value > max_value, otherwise value.

 If min_value is greater than max_value, the results are undefined.

 @param value Value to be clamped.  Supports 1, 2, 3, 4 components.
 @param min_value Lower bound, must be scalar or matching vector.
 @param max_value High bound, must match the type of low.
version: 9
end:

start:
w: 1, 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64
name: clamp
ret: #2#1
arg: #2#1 value
arg: #2#1 min_value
arg: #2#1 max_value above(min_value)
comment:
 Clamps a value to a specified high and low bound.

 clamp() returns min_value if value < min_value, max_value if value > max_value, otherwise value.

 If min_value is greater than max_value, the results are undefined.

 @param value Value to be clamped.  Supports 1, 2, 3, 4 components.
 @param min_value Lower bound, must be scalar or matching vector.
 @param max_value High bound, must match the type of low.
version: 19
end:

start:
w: 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64
name: clamp
ret: #2#1
arg: #2#1 value
arg: #2 min_value
arg: #2 max_value above(min_value)
comment:
 Clamps a value to a specified high and low bound.

 clamp() returns min_value if value < min_value, max_value if value > max_value, otherwise value.

 If min_value is greater than max_value, the results are undefined.

 @param value Value to be clamped.  Supports 1, 2, 3, 4 components.
 @param min_value Lower bound, must be scalar or matching vector.
 @param max_value High bound, must match the type of low.
version: 19
end:

start:
w: 1, 2, 3, 4
t: u8, u16, u32, i8, i16, i32
name: clz
ret: #2#1
arg: #2#1 value
comment:
 Returns the number of leading 0-bits in a value.

 For example, clz((char)0x03) returns 5.
version: 9
end:

start:
w: 2, 3, 4
t: u8, u16, u32, i8, i16, i32, f32
t: u8, u16, u32, i8, i16, i32, f32
name: convert_#3#1
arg: #2#1 v compatible(#3)
ret: #3#1
comment:
 Component wise conversion from #2#1 to #3#1.

 For the convert_* functions, conversions of floating point values to integer will truncate.
 Conversions of numbers too large to fit the destination type yield undefined results.
 For example, converting a float that contains 1.0e18 to a short is undefined.
version: 9
end:

start:
w: 2, 3, 4
t: u64, i64, f64
t: u64, i64, f64
name: convert_#3#1
arg: #2#1 v compatible(#3)
ret: #3#1
comment:
 Component wise conversion from #2#1 to #3#1.

 For the convert_* functions, conversions of floating point values to integer will truncate.
 Conversions of numbers too large to fit the destination type yield undefined results.
 For example, converting a float that contains 1.0e18 to a short is undefined.
version: 21
end:

start:
w: 2, 3, 4
t: u64, i64, f64
t: u8, u16, u32, i8, i16, i32, f32
name: convert_#3#1
arg: #2#1 v compatible(#3)
ret: #3#1
comment:
 Component wise conversion from #2#1 to #3#1.

 For the convert_* functions, conversions of floating point values to integer will truncate.
 Conversions of numbers too large to fit the destination type yield undefined results.
 For example, converting a float that contains 1.0e18 to a short is undefined.
version: 21
end:

start:
w: 2, 3, 4
t: u8, u16, u32, i8, i16, i32, f32
t: u64, i64, f64
name: convert_#3#1
arg: #2#1 v compatible(#3)
ret: #3#1
comment:
 Component wise conversion from #2#1 to #3#1.

 For the convert_* functions, conversions of floating point values to integer will truncate.
 Conversions of numbers too large to fit the destination type yield undefined results.
 For example, converting a float that contains 1.0e18 to a short is undefined.
version: 21
end:

start:
w: 1, 2, 3, 4
t: f32
name: copysign
ret: #2#1
arg: #2#1 magnitude_value
arg: #2#1 sign_value
comment:
 Copies the sign from sign_value to magnitude_value.

 The value returned is either magnitude_value or -magnitude_value.

 For example, copysign(4.0f, -2.7f) returns -4.0f and copysign(-4.0f, 2.7f) returns 4.0f.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: cos
ret: #2#1
arg: #2#1 v
comment:
 Returns the cosine of an angle measured in radians.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: cosh
ret: #2#1
arg: #2#1 v
comment:
 Returns the hypebolic cosine of v, where v is measured in radians.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: cospi
ret: #2#1
arg: #2#1 v
comment:
 Returns the cosine of (v * pi), where (v * pi) is measured in radians.

 To get the cosine of a value measured in degrees, call cospi(v / 180.f).
version: 9
end:

start:
w: 3, 4
t: f32
name: cross
ret: #2#1
arg: #2#1 left_vector
arg: #2#1 right_vector
comment:
 Computes the cross product of two vectors.
version: 9
test: vector
end:

start:
w: 1, 2, 3, 4
t: f32
name: degrees
ret: #2#1
arg: #2#1 v
comment:
 Converts from radians to degrees.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: distance
ret: #2
arg: #2#1 left_vector
arg: #2#1 right_vector
comment:
 Compute the distance between two points.
version: 9
test: vector
end:

start:
w: 1, 2, 3, 4
t: f32
name: dot
ret: #2
arg: #2#1 left_vector
arg: #2#1 right_vector
comment:
 Computes the dot product of two vectors.
version: 9
test: vector
end:

start:
w: 1, 2, 3, 4
t: f32
name: erf
ret: #2#1
arg: #2#1 v
comment:
 Returns the error function.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: erfc
ret: #2#1
arg: #2#1 v
comment:
 Returns the complementary error function.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: exp
ret: #2#1
arg: #2#1 v
comment:
 Returns e raised to v, i.e. e ^ v.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: exp10
ret: #2#1
arg: #2#1 v
comment:
 Returns 10 raised to v, i.e. 10.f ^ v.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: exp2
ret: #2#1
arg: #2#1 v
comment:
 Returns 2 raised to v, i.e. 2.f ^ v.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: expm1
ret: #2#1
arg: #2#1 v
comment:
 Returns e raised to v minus 1, i.e. (e ^ v) - 1.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: fabs
ret: #2#1
arg: #2#1 v
comment:
 Returns the absolute value of the float v.

 For integers, use abs().
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: fast_distance
ret: #2
arg: #2#1 left_vector
arg: #2#1 right_vector
comment:
 Computes the approximate distance between two points.

 The precision is what would be expected from doing the computation using 16 bit floating point values.
version: 17
test: vector
end:

start:
w: 1, 2, 3, 4
t: f32
name: fast_length
ret: #2
arg: #2#1 v
comment:
 Computes the approximate length of a vector.

 The precision is what would be expected from doing the computation using 16 bit floating point values.
version: 17
test: vector
end:

start:
w: 1, 2, 3, 4
t: f32
name: fast_normalize
ret: #2#1
arg: #2#1 v
comment:
 Approximately normalizes a vector.

 For vectors of size 1, returns -1.f for negative values, 0.f for null values, and 1.f for positive values.

 The precision is what would be expected from doing the computation using 16 bit floating point values.
version: 17
test: vector
end:

start:
w: 1, 2, 3, 4
t: f32
name: fdim
ret: #2#1
arg: #2#1 a
arg: #2#1 b
comment:
 Returns the positive difference between two values.

 If a > b, returns (a - b) otherwise returns 0f.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: floor
ret: #2#1
arg: #2#1 v
comment:
 Returns the smallest integer not greater than a value.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: fma
ret: #2#1
arg: #2#1 multiplicand1
arg: #2#1 multiplicand2
arg: #2#1 offset
comment:
 Multiply and add.  Returns (multiplicand1 * multiplicand2) + offset.

 This function is identical to mad().
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: fmax
ret: #2#1
arg: #2#1 a
arg: #2#1 b
comment:
 Returns the maximum of a and b, i.e. (a < b ? b : a).

 The max() function returns identical results but can be applied to more data types.
version: 9
end:

start:
w: 2, 3, 4
t: f32
name: fmax
ret: #2#1
arg: #2#1 a
arg: #2 b
comment:
 Returns the maximum of a and b, i.e. (a < b ? b : a).

 Unlike the other variants of fmax() and max(), this function compare each element of a to the scalar b.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: fmin
ret: #2#1
arg: #2#1 a
arg: #2#1 b
comment:
 Returns the minimum of a and b, i.e. (a > b ? b : a).

 The min() function returns identical results but can be applied to more data types.
version: 9
end:

start:
w: 2, 3, 4
t: f32
name: fmin
ret: #2#1
arg: #2#1 a
arg: #2 b
comment:
 Returns the minimum of a and b, i.e. (a > b ? b : a)

 Unlike the other variants of fmin() and min(), this function compare each element of a to the scalar b.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: fmod
ret: #2#1
arg: #2#1 numerator
arg: #2#1 denominator
comment:
 Returns the remainder of (numerator / denominator), where the quotient is rounded towards zero.

 The function remainder() is similar but rounds toward the closest interger.
 For example, fmod(-3.8f, 2.f) returns -1.8f (-3.8f - -1.f * 2.f)
 while remainder(-3.8f, 2.f) returns 0.2f (-3.8f - -2.f * 2.f).
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: fract
ret: #2#1
arg: #2#1 v
arg: #2#1 *floor
comment:
 Returns the positive fractional part of v, i.e. v - floor(v).

 For example, fract(1.3f, &val) returns 0.3f and sets val to 1.f.
 fract(-1.3f, &val) returns 0.7f and sets val to -2.f.

 @param v Input value.
 @param floor  If floor is not null, each element of floor will be set to the floor of the corresponding element of v.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: fract
ret: #2#1
arg: #2#1 v
comment:
 Returns the positive fractional part of v, i.e. v - floor(v).

 For example, fract(1.3f, &val) returns 0.3f and sets val to 1.f.
 fract(-1.3f, &val) returns 0.7f and sets val to -2.f.
inline:
    #2#1 unused;
    return fract(v, &unused);
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: frexp
ret: #2#1
arg: #2#1 v
arg: int#1 *exponent
comment:
 Returns the binary mantissa and exponent of v, e.g. v == mantissa * 2 ^ exponent.

 The mantissa is always between 0.5 (inclusive) and 1.0 (exclusive).
 See ldexp() for the reverse operation.

 @param v Supports float, float2, float3, float4.
 @param exponent  If exponent is not null, each element of exponent will be set to the exponent of the corresponding element of v.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: half_recip
ret: #2#1
arg: #2#1 v
comment:
 Returns the approximate reciprocal of a value.

 The precision is that of a 16 bit floating point value.
version: 17
end:

start:
w: 1, 2, 3, 4
t: f32
name: half_rsqrt
ret: #2#1
arg: #2#1 v
comment:
 Returns the approximate value of (1.f / sqrt(value)).

 The precision is that of a 16 bit floating point value.
version: 17
end:

start:
w: 1, 2, 3, 4
t: f32
name: half_sqrt
ret: #2#1
arg: #2#1 v
comment:
 Returns the approximate square root of a value.

 The precision is that of a 16 bit floating point value.
version: 17
end:

start:
w: 1, 2, 3, 4
t: f32
name: hypot
ret: #2#1
arg: #2#1 a
arg: #2#1 b
comment:
 Returns the hypotenuse, i.e. sqrt(a * a + b * b).
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: ilogb
ret: int#1
arg: float#1 v
comment:
 Returns the base two exponent of a value, where the mantissa is between 1.f (inclusive) and 2.f (exclusive).

 For example, ilogb(8.5f) returns 3.  Because of the difference in mantissa, this number is one less than
 is returned by frexp().

 logb() is similar but returns a float.
version: 9
test: custom
end:

start:
w: 1, 2, 3, 4
name: ldexp
ret: float#1
arg: float#1 mantissa
arg: int#1 exponent
comment:
 Returns the floating point created from the mantissa and exponent, i.e. (mantissa * 2 ^ exponent).

 See frexp() for the reverse operation.

 @param mantissa Supports float, float2, float3, and float4.
 @param exponent Supports single component or matching vector.
version: 9
end:

start:
w: 2, 3, 4
name: ldexp
ret: float#1
arg: float#1 mantissa
arg: int exponent
comment:
 Returns the floating point created from the mantissa and exponent, i.e. (mantissa * 2 ^ exponent).
 See frexp() for the reverse operation.

 @param mantissa Supports float, float2, float3, and float4.
 @param exponent Supports single component or matching vector.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: length
ret: #2
arg: #2#1 v
comment:
 Computes the length of a vector.
version: 9
test: vector
end:

start:
w: 1, 2, 3, 4
t: f32
name: lgamma
ret: #2#1
arg: #2#1 v
comment:
 Returns the natural logarithm of the absolute value of the gamma function, i.e. log(fabs(gamma(v))).
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: lgamma
ret: #2#1
arg: #2#1 v
arg: int#1 *sign_of_gamma
comment:
 Returns the natural logarithm of the absolute value of the gamma function, i.e. log(fabs(gamma(v))).

 Can also return the sign of the gamma function.

 @param v Input value.
 @param sign_of_gamma  If sign is not null, each element of sign will be set to -1.f if the gamma of the corresponding element of v is negative, otherwise to 1.f.

version: 9
#TODO Temporary until bionic & associated drivers are fixed
test: custom
end:

start:
w: 1, 2, 3, 4
t: f32
name: log
ret: #2#1
arg: #2#1 v
comment:
 Returns the natural logarithm.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: log10
ret: #2#1
arg: #2#1 v
comment:
 Returns the base 10 logarithm.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: log1p
ret: #2#1
arg: #2#1 v
comment:
 Returns the natural logarithm of (v + 1.f).
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: log2
ret: #2#1
arg: #2#1 v
comment:
 Returns the base 2 logarithm.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: logb
ret: #2#1
arg: #2#1 v
comment:
 Returns the base two exponent of a value, where the mantissa is between 1.f (inclusive) and 2.f (exclusive).

 For example, ilogb(8.5f) returns 3.f.  Because of the difference in mantissa, this number is one less than
 is returned by frexp().

 ilogb() is similar but returns an integer.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: mad
ret: #2#1
arg: #2#1 multiplicand1
arg: #2#1 multiplicand2
arg: #2#1 offset
comment:
 Multiply and add.  Returns (multiplicand1 * multiplicand2) + offset.

 This function is identical to fma().
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: max
ret: #2#1
arg: #2#1 a
arg: #2#1 b
comment:
 Returns the maximum value of two arguments.
version: 9
end:

start:
w: 1
t: i8 i16 i32 u8 u16 u32
name: max
ret: #2#1
arg: #2#1 a
arg: #2#1 b
comment:
 Returns the maximum value of two arguments.
inline:
 return (a > b ? a : b);
version: 9 19
end:

start:
w: 2
t: i8 i16 i32 u8 u16 u32
name: max
ret: #2#1
arg: #2#1 a
arg: #2#1 b
comment:
 Returns the maximum value of two arguments.
inline:
 #2#1 tmp;
 tmp.x = (a.x > b.x ? a.x : b.x);
 tmp.y = (a.y > b.y ? a.y : b.y);
 return tmp;
version: 9 19
end:

start:
w: 3
t: i8 i16 i32 u8 u16 u32
name: max
ret: #2#1
arg: #2#1 a
arg: #2#1 b
comment:
 Returns the maximum value of two arguments.
inline:
 #2#1 tmp;
 tmp.x = (a.x > b.x ? a.x : b.x);
 tmp.y = (a.y > b.y ? a.y : b.y);
 tmp.z = (a.z > b.z ? a.z : b.z);
 return tmp;
version: 9 19
end:

start:
w: 4
t: i8 i16 i32 u8 u16 u32
name: max
ret: #2#1
arg: #2#1 a
arg: #2#1 b
comment:
 Returns the maximum value of two arguments.
inline:
 #2#1 tmp;
 tmp.x = (a.x > b.x ? a.x : b.x);
 tmp.y = (a.y > b.y ? a.y : b.y);
 tmp.z = (a.z > b.z ? a.z : b.z);
 tmp.w = (a.w > b.w ? a.w : b.w);
 return tmp;
version: 9 19
end:

start:
w: 1, 2, 3, 4
t: i8 i16 i32 i64 u8 u16 u32 u64
name: max
ret: #2#1
arg: #2#1 a
arg: #2#1 b
comment:
 Returns the maximum value of two arguments.
version: 21
end:

start:
w: 1, 2, 3, 4
t: f32
name: min
ret: #2#1
arg: #2#1 a
arg: #2#1 b
comment:
 Returns the minimum value of two arguments.
version: 9
end:

start:
w: 1
t: i8 i16 i32 u8 u16 u32
name: min
ret: #2#1
arg: #2#1 a
arg: #2#1 b
comment:
 Returns the minimum value of two arguments.
inline:
 return (a < b ? a : b);
version: 9 19
end:

start:
w: 2
t: i8 i16 i32 u8 u16 u32
name: min
ret: #2#1
arg: #2#1 a
arg: #2#1 b
comment:
 Returns the minimum value of two arguments.
inline:
 #2#1 tmp;
 tmp.x = (a.x < b.x ? a.x : b.x);
 tmp.y = (a.y < b.y ? a.y : b.y);
 return tmp;
version: 9 19
end:

start:
w: 3
t: i8 i16 i32 u8 u16 u32
name: min
ret: #2#1
arg: #2#1 a
arg: #2#1 b
comment:
 Returns the minimum value of two arguments.
inline:
 #2#1 tmp;
 tmp.x = (a.x < b.x ? a.x : b.x);
 tmp.y = (a.y < b.y ? a.y : b.y);
 tmp.z = (a.z < b.z ? a.z : b.z);
 return tmp;
version: 9 19
end:

start:
w: 4
t: i8 i16 i32 u8 u16 u32
name: min
ret: #2#1
arg: #2#1 a
arg: #2#1 b
comment:
 Returns the minimum value of two arguments.
inline:
 #2#1 tmp;
 tmp.x = (a.x < b.x ? a.x : b.x);
 tmp.y = (a.y < b.y ? a.y : b.y);
 tmp.z = (a.z < b.z ? a.z : b.z);
 tmp.w = (a.w < b.w ? a.w : b.w);
 return tmp;
version: 9 19
end:

start:
w: 1, 2, 3, 4
t: i8 i16 i32 i64 u8 u16 u32 u64
name: min
ret: #2#1
arg: #2#1 a
arg: #2#1 b
comment:
 Returns the minimum value of two arguments.
version: 21
end:

start:
w: 1, 2, 3, 4
t: f32
name: mix
ret: #2#1
arg: #2#1 start
arg: #2#1 stop
arg: #2#1 fraction
comment:
 Returns start + ((stop - start) * fraction).

 This can be useful for mixing two values.  For example, to create a new color that is 40% color1 and 60% color2, use mix(color1, color2, 0.6f).
version: 9
end:

start:
w: 2, 3, 4
t: f32
name: mix
ret: #2#1
arg: #2#1 start
arg: #2#1 stop
arg: #2 fraction
comment:
 Returns start + ((stop - start) * fraction).

 This can be useful for mixing two values.  For example, to create a new color that is 40% color1 and 60% color2, use mix(color1, color2, 0.6f).
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: modf
ret: #2#1
arg: #2#1 v
arg: #2#1 *integral_part
comment:
 Returns the integral and fractional components of a number.

 Both components will have the same sign as x.  For example, for an input of -3.72f, iret will be set to -3.f and .72f will be returned.

 @param v Source value
 @param integral_part integral_part[0] will be set to the integral portion of the number.
 @return The floating point portion of the value.
version: 9
end:

start:
w: 1
t: f32
name: nan
ret: #2#1
arg: uint#1 v
comment:
 Returns a NaN value (Not a Number).

 @param v Not used.
#TODO We're not using the argument.  Once we do, add this documentation line:
# The argument is embedded into the return value and can be used to distinguish various NaNs.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_acos
ret: #2#1
arg: #2#1 v range(-1,1)
comment:
 Returns the approximate inverse cosine, in radians.
version: 21
# TODO Temporary
test: limited(0.0005)
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_acosh
ret: #2#1
arg: #2#1 v
comment:
 Returns the approximate inverse hyperbolic cosine, in radians.
version: 21
# TODO Temporary
test: limited(0.0005)
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_acospi
ret: #2#1
arg: #2#1 v range(-1,1)
comment:
 Returns the approximate inverse cosine in radians, divided by pi.

 To get an inverse cosine measured in degrees, use acospi(a) * 180.f.
version: 21
# TODO Temporary
test: limited(0.0005)
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_asin
ret: #2#1
arg: #2#1 v range(-1,1)
comment:
 Returns the approximate inverse sine, in radians.
version: 21
# TODO Temporary
test: limited(0.0005)
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_asinh
ret: #2#1
arg: #2#1 v
comment:
 Returns the approximate inverse hyperbolic sine, in radians.
version: 21
# TODO Temporary
test: limited(0.0005)
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_asinpi
ret: #2#1
arg: #2#1 v range(-1,1)
comment:
 Returns the approximate inverse sine in radians, divided by pi.

 To get an inverse sine measured in degrees, use asinpi(a) * 180.f.
version: 21
# TODO Temporary
test: limited(0.0005)
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_atan
ret: #2#1
arg: #2#1 v range(-1,1)
comment:
 Returns the approximate inverse tangent, in radians.
version: 21
# TODO Temporary
test: limited(0.0005)
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_atan2
ret: #2#1
arg: #2#1 numerator
arg: #2#1 denominator
comment:
 Returns the approximate inverse tangent of numerator / denominator, in radians.

 denominator can be 0.
version: 21
# TODO Temporary
test: limited(0.0005)
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_atan2pi
ret: #2#1
arg: #2#1 numerator
arg: #2#1 denominator
comment:
 Returns the approximate inverse tangent of numerator / denominator, in radians, divided by pi.

 To get an inverse tangent measured in degrees, use atan2pi(n, d) * 180.f.

 denominator can be 0.
version: 21
# TODO Temporary
test: limited(0.0005)
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_atanh
ret: #2#1
arg: #2#1 v range(-1,1)
comment:
 Returns the approximate inverse hyperbolic tangent, in radians.
version: 21
# TODO Temporary
test: limited(0.0005)
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_atanpi
ret: #2#1
arg: #2#1 v range(-1,1)
comment:
 Returns the approximate inverse tangent in radians, divided by pi.

 To get an inverse tangent measured in degrees, use atanpi(a) * 180.f.
version: 21
# TODO Temporary
test: limited(0.0005)
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_cbrt
ret: #2#1
arg: #2#1 v
comment:
 Returns the approximate cubic root.
version: 21
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_cos
ret: #2#1
arg: #2#1 v
comment:
 Returns the approximate cosine of an angle measured in radians.
version: 21
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_cosh
ret: #2#1
arg: #2#1 v
comment:
 Returns the approximate hypebolic cosine.
version: 21
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_cospi
ret: #2#1
arg: #2#1 v
comment:
 Returns the approximate cosine of (v * pi), where (v * pi) is measured in radians.

 To get the cosine of a value measured in degrees, call cospi(v / 180.f).
version: 21
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_distance
ret: #2
arg: #2#1 left_vector
arg: #2#1 right_vector
comment:
 Computes the approximate distance between two points.
version: 21
test: vector
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_divide
ret: #2#1
arg: #2#1 left_vector
arg: #2#1 right_vector
comment:
 Computes the approximate division result of two values.
version: 21
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_exp
ret: #2#1
arg: #2#1 v range(-86,86)
comment:
 Fast approximate exp.

 It is valid for inputs from -86.f to 86.f.  The precision is no worse than what would be expected from using 16 bit floating point values.
version: 18
test: limited
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_exp10
ret: #2#1
arg: #2#1 v range(-37,37)
comment:
 Fast approximate exp10.

 It is valid for inputs from -37.f to 37.f.  The precision is no worse than what would be expected from using 16 bit floating point values.
version: 18
test: limited
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_exp2
ret: #2#1
arg: #2#1 v range(-125,125)
comment:
 Fast approximate exp2.

 It is valid for inputs from -125.f to 125.f.  The precision is no worse than what would be expected from using 16 bit floating point values.
version: 18
test: limited
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_expm1
ret: #2#1
arg: #2#1 v
comment:
 Returns the approximate (e ^ v) - 1.
version: 21
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_hypot
ret: #2#1
arg: #2#1 a
arg: #2#1 b
comment:
 Returns the approximate native_sqrt(a * a + b * b)
version: 21
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_length
ret: #2
arg: #2#1 v
comment:
 Compute the approximate length of a vector.
version: 21
test: vector
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_log
ret: #2#1
arg: #2#1 v range(10e-10,10e10)
comment:
 Fast approximate log.

 It is not accurate for values very close to zero.
version: 18
test: limited
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_log10
ret: #2#1
arg: #2#1 v range(10e-10,10e10)
comment:
 Fast approximate log10.

 It is not accurate for values very close to zero.
version: 18
test: limited
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_log1p
ret: #2#1
arg: #2#1 v
comment:
 Returns the approximate natural logarithm of (v + 1.0f)
version: 21
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_log2
ret: #2#1
arg: #2#1 v range(10e-10,10e10)
comment:
 Fast approximate log2.

 It is not accurate for values very close to zero.
version: 18
test: limited
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_normalize
ret: #2#1
arg: #2#1 v
comment:
 Approximately normalizes a vector.
version: 21
test: vector
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_powr
ret: #2#1
arg: #2#1 base range(0,256)
arg: #2#1 exponent range(-15,15)
comment:
 Fast approximate (base ^ exponent).

 @param base Must be between 0.f and 256.f.  The function is not accurate for values very close to zero.
 @param exponent Must be between -15.f and 15.f.
version: 18
test: limited
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_recip
ret: #2#1
arg: #2#1 v
comment:
 Returns the approximate approximate reciprocal of a value.
version: 21
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_rootn
ret: #2#1
arg: #2#1 v
arg: int#1 n
comment:
 Compute the approximate Nth root of a value.
version: 21
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_rsqrt
ret: #2#1
arg: #2#1 v
comment:
 Returns approximate (1 / sqrt(v)).
version: 21
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_sin
ret: #2#1
arg: #2#1 v
comment:
 Returns the approximate sine of an angle measured in radians.
version: 21
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_sincos
ret: #2#1
arg: #2#1 v
arg: #2#1 *cos
comment:
 Returns the approximate sine and cosine of a value.

 @return sine
 @param v The incoming value in radians
 @param *cos cos[0] will be set to the cosine value.
version: 21
# TODO Temporary
test: limited(0.0005)
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_sinh
ret: #2#1
arg: #2#1 v
comment:
 Returns the approximate hyperbolic sine of a value specified in radians.
version: 21
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_sinpi
ret: #2#1
arg: #2#1 v
comment:
 Returns the approximate sine of (v * pi), where (v * pi) is measured in radians.

 To get the sine of a value measured in degrees, call sinpi(v / 180.f).
version: 21
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_sqrt
ret: #2#1
arg: #2#1 v
comment:
 Returns the approximate sqrt(v).
version: 21
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_tan
ret: #2#1
arg: #2#1 v
comment:
 Returns the approximate tangent of an angle measured in radians.
version: 21
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_tanh
ret: #2#1
arg: #2#1 v
comment:
 Returns the approximate hyperbolic tangent of a value.
version: 21
end:

start:
w: 1, 2, 3, 4
t: f32
name: native_tanpi
ret: #2#1
arg: #2#1 v
comment:
 Returns the approximate tangent of (v * pi), where (v * pi) is measured in radians.

 To get the tangent of a value measured in degrees, call tanpi(v / 180.f).
version: 21
end:

start:
w: 1, 2, 3, 4
t: f32
name: nextafter
ret: #2#1
arg: #2#1 v
arg: #2#1 target
comment:
 Returns the next floating point number from v towards target.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: normalize
ret: #2#1
arg: #2#1 v
comment:
 Normalize a vector.

 For vectors of size 1, returns -1.f for negative values, 0.f for null values, and 1.f for positive values.
version: 9
test: vector
end:

start:
w: 1, 2, 3, 4
t: f32
name: pow
ret: #2#1
arg: #2#1 base
arg: #2#1 exponent
comment:
 Returns base raised to the power exponent, i.e. base ^ exponent.

 pown() and powr() are similar.  pown() takes an integer exponent. powr() assumes the base to be non-negative.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: pown
ret: #2#1
arg: #2#1 base
arg: int#1 exponent
comment:
 Returns base raised to the power exponent, i.e. base ^ exponent.

 pow() and powr() are similar.  The both take a float exponent. powr() also assumes the base to be non-negative.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: powr
ret: #2#1
arg: #2#1 base range(0,3000)
arg: #2#1 exponent
comment:
 Returns base raised to the power exponent, i.e. base ^ exponent.  base must be >= 0.

 pow() and pown() are similar.  They both make no assumptions about the base.  pow() takes a float exponent while pown() take an integer.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: radians
ret: #2#1
arg: #2#1 v
comment:
 Converts from degrees to radians.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: remainder
ret: #2#1
arg: #2#1 numerator
arg: #2#1 denominator
comment:
 Returns the remainder of (numerator / denominator), where the quotient is rounded towards the nearest integer.

 The function fmod() is similar but rounds toward the closest interger.
 For example, fmod(-3.8f, 2.f) returns -1.8f (-3.8f - -1.f * 2.f)
 while remainder(-3.8f, 2.f) returns 0.2f (-3.8f - -2.f * 2.f).
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: remquo
ret: #2#1
arg: #2#1 numerator
arg: #2#1 denominator
arg: int#1 *quotient
comment:
 Returns the quotient and the remainder of (numerator / denominator).

 Only the sign and lowest three bits of the quotient are guaranteed to be accurate.

 This function is useful for implementing periodic functions.  The low three bits of the quotient gives the quadrant and the remainder the distance within the quadrant.  For example, an implementation of sin(x) could call remquo(x, PI / 2.f, &quadrant) to reduce very large value of x to something within a limited range.

 Example: remquo(-23.5f, 8.f, &quot) sets the lowest three bits of quot to 3 and the sign negative.  It returns 0.5f.

 @param numerator The numerator.
 @param denominator The denominator.
 @param *quotient quotient[0] will be set to the integer quotient.
 @return The remainder, precise only for the low three bits.
version: 9
test: custom
end:

start:
w: 1, 2, 3, 4
t: f32
name: rint
ret: #2#1
arg: #2#1 v
comment:
 Rounds to the nearest integral value.

 rint() rounds half values to even.  For example, rint(0.5f) returns 0.f and rint(1.5f) returns 2.f.  Similarly, rint(-0.5f) returns -0.f and rint(-1.5f) returns -2.f.

 round() is similar but rounds away from zero.  trunc() truncates the decimal fraction.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: rootn
ret: #2#1
arg: #2#1 v
arg: int#1 n
comment:
 Compute the Nth root of a value.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: round
ret: #2#1
arg: #2#1 v
comment:
 Round to the nearest integral value.

 round() rounds half values away from zero.  For example, round(0.5f) returns 1.f and round(1.5f) returns 2.f.  Similarly, round(-0.5f) returns -1.f and round(-1.5f) returns -2.f.

 rint() is similar but rounds half values toward even.  trunc() truncates the decimal fraction.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: rsqrt
ret: #2#1
arg: #2#1 v
comment:
 Returns (1 / sqrt(v)).
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: sign
ret: #2#1
arg: #2#1 v
comment:
 Returns the sign of a value.

 if (v < 0) return -1.f;
 else if (v > 0) return 1.f;
 else return 0.f;
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: sin
ret: #2#1
arg: #2#1 v
comment:
 Returns the sine of an angle measured in radians.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: sincos
ret: #2#1
arg: #2#1 v
arg: #2#1 *cos
comment:
 Returns the sine and cosine of a value.

 @return sine of v
 @param v The incoming value in radians
 @param *cos cosptr[0] will be set to the cosine value.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: sinh
ret: #2#1
arg: #2#1 v
comment:
 Returns the hyperbolic sine of v, where v is measured in radians.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: sinpi
ret: #2#1
arg: #2#1 v
comment:
 Returns the sine of (v * pi), where (v * pi) is measured in radians.

 To get the sine of a value measured in degrees, call sinpi(v / 180.f).
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: sqrt
ret: #2#1
arg: #2#1 v
comment:
 Returns the square root of a value.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: step
ret: #2#1
arg: #2#1 edge
arg: #2#1 v
comment:
 Returns 0.f if v < edge, 1.f otherwise.

 This can be useful to create conditional computations without using loops and branching instructions.  For example, instead of computing (a[i] < b[i]) ? 0.f : atan2(a[i], b[i]) for the corresponding elements of a vector, you could instead use step(a, b) * atan2(a, b).
version: 9
end:

start:
w: 2, 3, 4
t: f32
name: step
ret: #2#1
arg: #2#1 edge
arg: #2 v
comment:
 Returns 0.f if v < edge, 1.f otherwise.

 This can be useful to create conditional computations without using loops and branching instructions.  For example, instead of computing (a[i] < b) ? 0.f : atan2(a[i], b) for each element of a vector, you could instead use step(a, b) * atan2(a, b).
version: 9
end:

start:
w: 2, 3, 4
t: f32
name: step
ret: #2#1
arg: #2 edge
arg: #2#1 v
comment:
 Returns 0.f if v < edge, 1.f otherwise.

 This can be useful to create conditional computations without using loops and branching instructions.  For example, instead of computing (a < b[i]) ? 0.f : atan2(a, b[i]) for each element of a vector, you could instead use step(a, b) * atan2(a, b).
version: 21
end:

start:
w: 1, 2, 3, 4
t: f32
name: tan
ret: #2#1
arg: #2#1 v
comment:
 Returns the tangent of an angle measured in radians.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: tanh
ret: #2#1
arg: #2#1 v
comment:
 Returns the hyperbolic tangent of a value.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: tanpi
ret: #2#1
arg: #2#1 v
comment:
 Returns the tangent of (v * pi), where (v * pi) is measured in radians.

 To get the tangent of a value measured in degrees, call tanpi(v / 180.f).
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: tgamma
ret: #2#1
arg: #2#1 v
comment:
 Returns the gamma function of a value.
version: 9
end:

start:
w: 1, 2, 3, 4
t: f32
name: trunc
ret: #2#1
arg: #2#1 v
comment:
 Rounds to integral using truncation.

 For example, trunc(1.7f) returns 1.f and trunc(-1.7f) returns -1.f.

 See rint() and round() for other rounding options.
version: 9
end:
