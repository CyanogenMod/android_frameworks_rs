/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
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

#include "vp9_common.h"
#include "vp9_filter.h"
#include <string.h>
#include <stdio.h>

extern void vp9_convolve8_horiz_neon(const uint8_t *src, ptrdiff_t src_stride,
                                     uint8_t *dst, ptrdiff_t dst_stride,
                                     const int16_t *filter_x, int x_step_q4,
                                     const int16_t *filter_y, int y_step_q4,
                                     int w, int h);

extern void vp9_convolve8_vert_neon(const uint8_t *src, ptrdiff_t src_stride,
                                    uint8_t *dst, ptrdiff_t dst_stride,
                                    const int16_t *filter_x, int x_step_q4,
                                    const int16_t *filter_y, int y_step_q4,
                                    int w, int h);

extern void vp9_convolve8_avg_vert_neon(const uint8_t *src, ptrdiff_t src_stride,
                                        uint8_t *dst, ptrdiff_t dst_stride,
                                        const int16_t *filter_x, int x_step_q4,
                                        const int16_t *filter_y, int y_step_q4,
                                        int w, int h);
extern void vp9_convolve8_avg_c(const uint8_t *src, ptrdiff_t src_stride,
                                uint8_t *dst, ptrdiff_t dst_stride,
                                const int16_t *filter_x, int x_step_q4,
                                const int16_t *filter_y, int y_step_q4,
                                int w, int h);

extern void vp9_convolve8_c(const uint8_t *src, ptrdiff_t src_stride,
                            uint8_t *dst, ptrdiff_t dst_stride,
                            const int16_t *filter_x, int x_step_q4,
                            const int16_t *filter_y, int y_step_q4,
                            int w, int h);

void vp9_convolve8_neon(const uint8_t *src, ptrdiff_t src_stride,
                        uint8_t *dst, ptrdiff_t dst_stride,
                        const int16_t *filter_x, int x_step_q4,
                        const int16_t *filter_y, int y_step_q4,
                        int w, int h) {
    /* Given our constraints: w <= 64, h <= 64, taps == 8 we can reduce the
     * maximum buffer size to 64 * 64 + 7 (+ 1 to make it divisible by 4).
     */
    DECLARE_ALIGNED_ARRAY(8, uint8_t, temp, 64 * 72);

    // Account for the vertical phase needing 3 lines prior and 4 lines post
    int intermediate_height = h + 7;

    if (x_step_q4 != 16 || y_step_q4 != 16)
        return vp9_convolve8_c(src, src_stride,
                               dst, dst_stride,
                               filter_x, x_step_q4,
                               filter_y, y_step_q4,
                               w, h);

    /* Filter starting 3 lines back. The neon implementation will ignore the
     * given height and filter a multiple of 4 lines. Since this goes in to
     * the temp buffer which has lots of extra room and is subsequently discarded
     * this is safe if somewhat less than ideal.
     */
    vp9_convolve8_horiz_neon(src - src_stride * 3, src_stride,
                             temp, 64, filter_x, x_step_q4,
                             filter_y, y_step_q4,
                             w, intermediate_height);

    /* Step into the temp buffer 3 lines to get the actual frame data */
    vp9_convolve8_vert_neon(temp + 64 * 3, 64, dst, dst_stride, filter_x,
                            x_step_q4, filter_y, y_step_q4, w, h);
}

void vp9_convolve8_avg_neon(const uint8_t *src, ptrdiff_t src_stride,
                            uint8_t *dst, ptrdiff_t dst_stride,
                            const int16_t *filter_x, int x_step_q4,
                            const int16_t *filter_y, int y_step_q4,
                            int w, int h) {
    DECLARE_ALIGNED_ARRAY(8, uint8_t, temp, 64 * 72);
    int intermediate_height = h + 7;

    if (x_step_q4 != 16 || y_step_q4 != 16)
        return vp9_convolve8_avg_c(src, src_stride, dst, dst_stride,
                                   filter_x, x_step_q4, filter_y, y_step_q4,
                                   w, h);

    /* This implementation has the same issues as above. In addition, we only want
     * to average the values after both passes.
     */
    vp9_convolve8_horiz_neon(src - src_stride * 3, src_stride, temp, 64,
                             filter_x, x_step_q4, filter_y, y_step_q4,
                             w, intermediate_height);
    vp9_convolve8_avg_vert_neon(temp + 64 * 3, 64, dst, dst_stride,
                                filter_x, x_step_q4, filter_y, y_step_q4,
                                w, h);
}
