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
#ifndef VP9_COMMON_VP9_FILTER_H_
#define VP9_COMMON_VP9_FILTER_H_

#define FILTER_BITS 7

#define SUBPEL_BITS 4
#define SUBPEL_MASK ((1 << SUBPEL_BITS) - 1)
#define SUBPEL_SHIFTS (1 << SUBPEL_BITS)
#define SUBPEL_TAPS 8

typedef enum {
    EIGHTTAP = 0,
    EIGHTTAP_SMOOTH = 1,
    EIGHTTAP_SHARP = 2,
    BILINEAR = 3,
    SWITCHABLE = 4  /* should be the last one */
} INTERPOLATION_TYPE;

typedef int16_t subpel_kernel[SUBPEL_TAPS];

struct subpix_fn_table {
    const subpel_kernel *filter_x;
    const subpel_kernel *filter_y;
};

const subpel_kernel *vp9_get_filter_kernel(INTERPOLATION_TYPE type);
extern const subpel_kernel vp9_bilinear_filters[SUBPEL_SHIFTS];
extern const subpel_kernel vp9_sub_pel_filters_8[SUBPEL_SHIFTS];
extern const subpel_kernel vp9_sub_pel_filters_8s[SUBPEL_SHIFTS];
extern const subpel_kernel vp9_sub_pel_filters_8lp[SUBPEL_SHIFTS];

// The VP9_BILINEAR_FILTERS_2TAP macro returns a pointer to the bilinear
// filter kernel as a 2 tap filter.
#define BILINEAR_FILTERS_2TAP(x) \
    (vp9_bilinear_filters[(x)] + SUBPEL_TAPS/2 - 1)

#endif  // VP9_COMMON_VP9_FILTER_H_
