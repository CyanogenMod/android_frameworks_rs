/*
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

#ifndef RSD_CPU_CONVOLVE_NEON_H
#define RSD_CPU_CONVOLVE_NEON_H

#include <stdint.h>

extern "C" {
#if defined(ARCH_ARM_HAVE_VFP)
void vp9_convolve_copy_neon(const uint8_t *src, ptrdiff_t src_stride,
                            uint8_t *dst, ptrdiff_t dst_stride,
                            const int16_t *filter_x, int x_step_q4,
                            const int16_t *filter_y, int y_step_q4,
                            int w, int h);

void vp9_convolve_avg_neon(const uint8_t *src, ptrdiff_t src_stride,
                           uint8_t *dst, ptrdiff_t dst_stride,
                           const int16_t *filter_x, int x_step_q4,
                           const int16_t *filter_y, int y_step_q4,
                           int w, int h);

void vp9_convolve8_vert_neon(const uint8_t *src, ptrdiff_t src_stride,
                             uint8_t *dst, ptrdiff_t dst_stride,
                             const int16_t *filter_x, int x_step_q4,
                             const int16_t *filter_y, int y_step_q4,
                             int w, int h);

void vp9_convolve8_avg_vert_neon(const uint8_t *src, ptrdiff_t src_stride,
                                 uint8_t *dst, ptrdiff_t dst_stride,
                                 const int16_t *filter_x, int x_step_q4,
                                 const int16_t *filter_y, int y_step_q4,
                                 int w, int h);

void vp9_convolve8_horiz_neon(const uint8_t *src, ptrdiff_t src_stride,
                              uint8_t *dst, ptrdiff_t dst_stride,
                              const int16_t *filter_x, int x_step_q4,
                              const int16_t *filter_y, int y_step_q4,
                              int w, int h);

void vp9_convolve8_avg_horiz_neon(const uint8_t *src, ptrdiff_t src_stride,
                                  uint8_t *dst, ptrdiff_t dst_stride,
                                  const int16_t *filter_x, int x_step_q4,
                                  const int16_t *filter_y, int y_step_q4,
                                  int w, int h);
void vp9_convolve8_neon(const uint8_t *src, ptrdiff_t src_stride,
                        uint8_t *dst, ptrdiff_t dst_stride,
                        const int16_t *filter_x, int x_step_q4,
                        const int16_t *filter_y, int y_step_q4,
                        int w, int h);

void vp9_convolve8_avg_neon(const uint8_t *src, ptrdiff_t src_stride,
                            uint8_t *dst, ptrdiff_t dst_stride,
                            const int16_t *filter_x, int x_step_q4,
                            const int16_t *filter_y, int y_step_q4,
                            int w, int h);

#else
void vp9_convolve_copy_c(const uint8_t *src, ptrdiff_t src_stride,
                         uint8_t *dst, ptrdiff_t dst_stride,
                         const int16_t *filter_x, int x_step_q4,
                         const int16_t *filter_y, int y_step_q4,
                         int w, int h);

void vp9_convolve_avg_c(const uint8_t *src, ptrdiff_t src_stride,
                        uint8_t *dst, ptrdiff_t dst_stride,
                        const int16_t *filter_x, int x_step_q4,
                        const int16_t *filter_y, int y_step_q4,
                        int w, int h);

void vp9_convolve8_vert_c(const uint8_t *src, ptrdiff_t src_stride,
                          uint8_t *dst, ptrdiff_t dst_stride,
                          const int16_t *filter_x, int x_step_q4,
                          const int16_t *filter_y, int y_step_q4,
                          int w, int h);

void vp9_convolve8_avg_vert_c(const uint8_t *src, ptrdiff_t src_stride,
                              uint8_t *dst, ptrdiff_t dst_stride,
                              const int16_t *filter_x, int x_step_q4,
                              const int16_t *filter_y, int y_step_q4,
                              int w, int h);

void vp9_convolve8_horiz_c(const uint8_t *src, ptrdiff_t src_stride,
                           uint8_t *dst, ptrdiff_t dst_stride,
                           const int16_t *filter_x, int x_step_q4,
                           const int16_t *filter_y, int y_step_q4,
                           int w, int h);

void vp9_convolve8_avg_horiz_c(const uint8_t *src, ptrdiff_t src_stride,
                               uint8_t *dst, ptrdiff_t dst_stride,
                               const int16_t *filter_x, int x_step_q4,
                               const int16_t *filter_y, int y_step_q4,
                               int w, int h);

void vp9_convolve8_c(const uint8_t *src, ptrdiff_t src_stride,
                     uint8_t *dst, ptrdiff_t dst_stride,
                     const int16_t *filter_x, int x_step_q4,
                     const int16_t *filter_y, int y_step_q4,
                     int w, int h);

void vp9_convolve8_avg_c(const uint8_t *src, ptrdiff_t src_stride,
                         uint8_t *dst, ptrdiff_t dst_stride,
                         const int16_t *filter_x, int x_step_q4,
                         const int16_t *filter_y, int y_step_q4,
                         int w, int h);
#endif
}
#endif
