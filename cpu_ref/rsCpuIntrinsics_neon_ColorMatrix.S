/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include <machine/cpu-features.h>
#include <machine/asm.h>

#define SNIP_START(x) \
	.globl x; x:

#define SNIP_END(x) \
	.globl x##_end; x##_end: \
	.globl x##_len; x##_len: \
    .word x##_end-x

SNIP_START(_N_ColorMatrix_prefix)
    stmfd           sp!, {r4, lr}
    vpush           {q4-q7}
    vld1.16 {q2}, [r2]!
    vld1.16 {q3}, [r2]!
    vld1.16 {d8}, [r2]!
SNIP_END(_N_ColorMatrix_prefix)

SNIP_START(_N_ColorMatrix_postfix1)
    subs r3, r3, #1
    #bne 1b
SNIP_END(_N_ColorMatrix_postfix1)

SNIP_START(_N_ColorMatrix_postfix2)

    #mov r0, #0
    #ldr r0, [r0]

    vpop            {q4-q7}
    ldmfd           sp!, {r4, lr}
    bx              lr
SNIP_END(_N_ColorMatrix_postfix2)

SNIP_START(_N_ColorMatrix_load_u8_4)
    vld4.8 {d0[0],d1[0],d2[0],d3[0]}, [r1]!
    vld4.8 {d0[1],d1[1],d2[1],d3[1]}, [r1]!
    vld4.8 {d0[2],d1[2],d2[2],d3[2]}, [r1]!
    vld4.8 {d0[3],d1[3],d2[3],d3[3]}, [r1]!
SNIP_END(_N_ColorMatrix_load_u8_4)

SNIP_START(_N_ColorMatrix_load_u8_2)
    vld2.8 {d0[0],d1[0]}, [r1]!
    vld2.8 {d0[1],d1[1]}, [r1]!
    vld2.8 {d0[2],d1[2]}, [r1]!
    vld2.8 {d0[3],d1[3]}, [r1]!
SNIP_END(_N_ColorMatrix_load_u8_2)

SNIP_START(_N_ColorMatrix_load_u8_1)
    vld1.32 {d0}, [r1]!
SNIP_END(_N_ColorMatrix_load_u8_1)

SNIP_START(_N_ColorMatrix_store_u8_4)
    vst4.8 {d0[0],d1[0],d2[0],d3[0]}, [r0]!
    vst4.8 {d0[1],d1[1],d2[1],d3[1]}, [r0]!
    vst4.8 {d0[2],d1[2],d2[2],d3[2]}, [r0]!
    vst4.8 {d0[3],d1[3],d2[3],d3[3]}, [r0]!
SNIP_END(_N_ColorMatrix_store_u8_4)

SNIP_START(_N_ColorMatrix_store_u8_2)
    vst2.8 {d0[0],d1[0]}, [r0]!
    vst2.8 {d0[1],d1[1]}, [r0]!
    vst2.8 {d0[2],d1[2]}, [r0]!
    vst2.8 {d0[3],d1[3]}, [r0]!
SNIP_END(_N_ColorMatrix_store_u8_2)

SNIP_START(_N_ColorMatrix_store_u8_1)
    vst1.32 {d0}, [r0]!
SNIP_END(_N_ColorMatrix_store_u8_1)

SNIP_START(_N_ColorMatrix_unpack_u8_4)
    vmovl.u8 q12, d0  /* R */
    vmovl.u8 q13, d1  /* G */
    vmovl.u8 q14, d2  /* B */
    vmovl.u8 q15, d3  /* A */
SNIP_END(_N_ColorMatrix_unpack_u8_4)

SNIP_START(_N_ColorMatrix_unpack_u8_3)
    vmovl.u8 q12, d0  /* R */
    vmovl.u8 q13, d1  /* G */
    vmovl.u8 q14, d2  /* B */
SNIP_END(_N_ColorMatrix_unpack_u8_3)

SNIP_START(_N_ColorMatrix_unpack_u8_2)
    vmovl.u8 q12, d0  /* R */
    vmovl.u8 q13, d1  /* G */
SNIP_END(_N_ColorMatrix_unpack_u8_2)

SNIP_START(_N_ColorMatrix_unpack_u8_1)
    vmovl.u8 q12, d0  /* R */
SNIP_END(_N_ColorMatrix_unpack_u8_1)

SNIP_START(_N_ColorMatrix_pack_u8_4)
    vshrn.i32 d24, q8, #8
    vshrn.i32 d26, q9, #8
    vshrn.i32 d28, q10, #8
    vshrn.i32 d30, q11, #8
    vqmovun.s16 d0, q12
    vqmovun.s16 d1, q13
    vqmovun.s16 d2, q14
    vqmovun.s16 d3, q15
SNIP_END(_N_ColorMatrix_pack_u8_4)

SNIP_START(_N_ColorMatrix_pack_u8_3)
    vshrn.i32 d24, q8, #8
    vshrn.i32 d26, q9, #8
    vshrn.i32 d28, q10, #8
    vqmovun.s16 d0, q12
    vqmovun.s16 d1, q13
    vqmovun.s16 d2, q14
SNIP_END(_N_ColorMatrix_pack_u8_3)

SNIP_START(_N_ColorMatrix_pack_u8_2)
    vshrn.i32 d24, q8, #8
    vshrn.i32 d26, q9, #8
    vqmovun.s16 d0, q12
    vqmovun.s16 d1, q13
SNIP_END(_N_ColorMatrix_pack_u8_2)

SNIP_START(_N_ColorMatrix_pack_u8_1)
    vshrn.i32 d24, q8, #8
    vqmovun.s16 d0, q12
SNIP_END(_N_ColorMatrix_pack_u8_1)

SNIP_START(_N_ColorMatrix_dot)
    vmov.u8 d1, d0
    vmov.u8 d2, d0
SNIP_END(_N_ColorMatrix_dot)

