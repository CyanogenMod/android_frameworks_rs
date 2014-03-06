/*
 *  Copyright (c) 2011 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 *
 * Copyright (C) 2014 The Android Open Source Project
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
#ifndef VP9_COMMON_VP9_COMMON_H_
#define VP9_COMMON_VP9_COMMON_H_
#include <stdint.h>

#define DECLARE_ALIGNED_ARRAY(a,typ,val,n)\
  typ val##_[(n)+(a)/sizeof(typ)+1];\
  typ *val = (typ*)((((intptr_t)val##_)+(a)-1)&((intptr_t)-(a)))
/* Interface header for common constant data structures and lookup tables */

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

#define ROUND_POWER_OF_TWO(value, n) \
    (((value) + (1 << ((n) - 1))) >> (n))

#define ALIGN_POWER_OF_TWO(value, n) \
    (((value) + ((1 << (n)) - 1)) & ~((1 << (n)) - 1))

// Only need this for fixed-size arrays, for structs just assign.
#define vp9_copy(dest, src) {            \
    memcpy(dest, src, sizeof(src));  \
  }

// Use this for variably-sized arrays.
#define vp9_copy_array(dest, src, n) {       \
    memcpy(dest, src, n * sizeof(*src)); \
  }

#define vp9_zero(dest) memset(&dest, 0, sizeof(dest))
#define vp9_zero_array(dest, n) memset(dest, 0, n * sizeof(*dest))

static inline uint8_t clip_pixel(int val) {
  return (val > 255) ? 255u : (val < 0) ? 0u : val;
}

static inline int clamp(int value, int low, int high) {
  return value < low ? low : (value > high ? high : value);
}

static inline double fclamp(double value, double low, double high) {
  return value < low ? low : (value > high ? high : value);
}

#define VP9_SYNC_CODE_0 0x49
#define VP9_SYNC_CODE_1 0x83
#define VP9_SYNC_CODE_2 0x42

#define VP9_FRAME_MARKER 0x2


#endif  // VP9_COMMON_VP9_COMMON_H_
