/*
 * Copyright (C) 2012 The Android Open Source Project
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

/*
        r0 = dst
        r1 = y0 base pointer
        r2 = y1 base pointer
        r3 = y2 base pointer
        sp = coeffs
        sp = length / 2
*/

ENTRY(rsdIntrinsicConvolve3x3_K)
        push            {r4-r8, r10, r11, lr}
        vpush           {q4-q7}

        /* Get the coeffs pointer from the stack and load the
           coefficients in the q0, q1 NEON registers */
        ldr r4, [sp, #32+64]
        vld1.16 {q0, q1}, [r4]

        /* Get count from the stack */
        ldr r4, [sp, #36+64]

        /* Load the frequently used immediate in a register */
        mov r5, #8

1:
        /* Load and post-increase the address by r5=#8 */
        vld1.8 {q13}, [r1], r5
        vld1.8 {q14}, [r2], r5
        vld1.8 {q15}, [r3], r5

        /* Signal memory for data that will be used in the loop after the next */
        PLD         (r1, r5)
        PLD         (r2, r5)
        PLD         (r3, r5)

        vmovl.u8 q2, d26
        vmovl.u8 q3, d27
        vmovl.u8 q4, d28
        vmovl.u8 q5, d29
        vmovl.u8 q6, d30
        vmovl.u8 q7, d31

/*
        The two pixel source array is
        d4,  d5,  d6,  d7
        d8,  d9,  d10, d11
        d12, d13, d14, d15
*/

        vmull.s16 q8, d4, d0[0]
        vmlal.s16 q8, d4, d0[3]
        vmlal.s16 q8, d5, d0[1]
        vmlal.s16 q8, d6, d0[2]
        vmlal.s16 q8, d9, d1[0]
        vmlal.s16 q8, d10, d1[1]
        vmlal.s16 q8, d12, d1[2]
        vmlal.s16 q8, d13, d1[3]
        vmlal.s16 q8, d14, d2[0]

        vmull.s16 q9, d5, d0[0]
        vmlal.s16 q9, d6, d0[1]
        vmlal.s16 q9, d7, d0[2]
        vmlal.s16 q9, d9, d0[3]
        vmlal.s16 q9, d10, d1[0]
        vmlal.s16 q9, d11, d1[1]
        vmlal.s16 q9, d13, d1[2]
        vmlal.s16 q9, d14, d1[3]
        vmlal.s16 q9, d15, d2[0]

        vshrn.i32 d16, q8, #8
        vshrn.i32 d17, q9, #8

        vqmovun.s16 d16, q8
        vst1.8 d16, [r0]!

        /* Are we done yet? */
        subs r4, r4, #1
        bne 1b

        /* We're done, bye! */
        vpop            {q4-q7}
        pop             {r4-r8, r10, r11, lr}
        bx              lr
END(TestConvolveK)

/*
        r0 = dst
        r1 = src
        r2 = matrix
        r3 = length
*/
ENTRY(rsdIntrinsicColorMatrix4x4_K)
        .save           {r4, lr}
        stmfd           sp!, {r4, lr}
        vpush           {q4-q7}

        vld1.16 {q2}, [r2]!
        vld1.16 {q3}, [r2]!

1:
        vld4.8 {d0[0],d1[0],d2[0],d3[0]}, [r1]!
        vld4.8 {d0[1],d1[1],d2[1],d3[1]}, [r1]!
        vld4.8 {d0[2],d1[2],d2[2],d3[2]}, [r1]!
        vld4.8 {d0[3],d1[3],d2[3],d3[3]}, [r1]!

        vmovl.u8 q12, d0  /* R */
        vmovl.u8 q13, d1  /* G */
        vmovl.u8 q14, d2  /* B */
        vmovl.u8 q15, d3  /* A */

        vmull.s16 q8,  d24, d4[0]
        vmull.s16 q9,  d24, d4[1]
        vmull.s16 q10, d24, d4[2]
        vmull.s16 q11, d24, d4[3]

        vmlal.s16 q8,  d26, d5[0]
        vmlal.s16 q9,  d26, d5[1]
        vmlal.s16 q10, d26, d5[2]
        vmlal.s16 q11, d26, d5[3]

        vmlal.s16 q8,  d28, d6[0]
        vmlal.s16 q9,  d28, d6[1]
        vmlal.s16 q10, d28, d6[2]
        vmlal.s16 q11, d28, d6[3]

        vmlal.s16 q8,  d30, d7[0]
        vmlal.s16 q9,  d30, d7[1]
        vmlal.s16 q10, d30, d7[2]
        vmlal.s16 q11, d30, d7[3]

        vshrn.i32 d24, q8, #8
        vshrn.i32 d26, q9, #8
        vshrn.i32 d28, q10, #8
        vshrn.i32 d30, q11, #8

        vqmovun.s16 d0, q12
        vqmovun.s16 d1, q13
        vqmovun.s16 d2, q14
        vqmovun.s16 d3, q15

        vst4.8 {d0[0],d1[0],d2[0],d3[0]}, [r0]!
        vst4.8 {d0[1],d1[1],d2[1],d3[1]}, [r0]!
        vst4.8 {d0[2],d1[2],d2[2],d3[2]}, [r0]!
        vst4.8 {d0[3],d1[3],d2[3],d3[3]}, [r0]!

        subs r3, r3, #1
        bne 1b

        vpop            {q4-q7}
        ldmfd           sp!, {r4, lr}
        bx              lr
END(rsdIntrinsicColorMatrix4x4_K)

/*
        r0 = dst
        r1 = src
        r2 = matrix
        r3 = length
*/
ENTRY(rsdIntrinsicColorMatrix3x3_K)
        .save           {r4, lr}
        stmfd           sp!, {r4, lr}
        vpush           {q4-q7}

        vld1.16 {q2}, [r2]!
        vld1.16 {q3}, [r2]!

1:
        vld4.8 {d0[0],d1[0],d2[0],d3[0]}, [r1]!
        vld4.8 {d0[1],d1[1],d2[1],d3[1]}, [r1]!
        vld4.8 {d0[2],d1[2],d2[2],d3[2]}, [r1]!
        vld4.8 {d0[3],d1[3],d2[3],d3[3]}, [r1]!

        vmovl.u8 q12, d0
        vmovl.u8 q13, d1
        vmovl.u8 q14, d2

        vmull.s16 q8,  d24, d4[0]
        vmull.s16 q9,  d24, d4[1]
        vmull.s16 q10, d24, d4[2]

        vmlal.s16 q8,  d26, d5[0]
        vmlal.s16 q9,  d26, d5[1]
        vmlal.s16 q10, d26, d5[2]

        vmlal.s16 q8,  d28, d6[0]
        vmlal.s16 q9,  d28, d6[1]
        vmlal.s16 q10, d28, d6[2]

        vshrn.i32 d24, q8, #8
        vshrn.i32 d26, q9, #8
        vshrn.i32 d28, q10, #8

        vqmovun.s16 d0, q12
        vqmovun.s16 d1, q13
        vqmovun.s16 d2, q14

        vst4.8 {d0[0],d1[0],d2[0],d3[0]}, [r0]!
        vst4.8 {d0[1],d1[1],d2[1],d3[1]}, [r0]!
        vst4.8 {d0[2],d1[2],d2[2],d3[2]}, [r0]!
        vst4.8 {d0[3],d1[3],d2[3],d3[3]}, [r0]!

        subs r3, r3, #1
        bne 1b

        vpop            {q4-q7}
        ldmfd           sp!, {r4, lr}
        bx              lr
END(rsdIntrinsicColorMatrix3x3_K)

/*
        r0 = dst
        r1 = src
        r2 = matrix
        r3 = length
*/
ENTRY(rsdIntrinsicColorMatrixDot_K)
        .save           {r4, lr}
        stmfd           sp!, {r4, lr}
        vpush           {q4-q7}

        vld1.16 {q2}, [r2]!
        vld1.16 {q3}, [r2]!

1:
        vld4.8 {d0[0],d1[0],d2[0],d3[0]}, [r1]!
        vld4.8 {d0[1],d1[1],d2[1],d3[1]}, [r1]!
        vld4.8 {d0[2],d1[2],d2[2],d3[2]}, [r1]!
        vld4.8 {d0[3],d1[3],d2[3],d3[3]}, [r1]!

        vmovl.u8 q12, d0
        vmovl.u8 q13, d1
        vmovl.u8 q14, d2

        vmull.s16 q8,  d24, d4[0]
        vmlal.s16 q8,  d26, d5[0]
        vmlal.s16 q8,  d28, d6[0]
        vshrn.i32 d24, q8, #8
        vqmovun.s16 d0, q12
        vmov.u8 d1, d0
        vmov.u8 d2, d0

        vst4.8 {d0[0],d1[0],d2[0],d3[0]}, [r0]!
        vst4.8 {d0[1],d1[1],d2[1],d3[1]}, [r0]!
        vst4.8 {d0[2],d1[2],d2[2],d3[2]}, [r0]!
        vst4.8 {d0[3],d1[3],d2[3],d3[3]}, [r0]!

        subs r3, r3, #1
        bne 1b

        vpop            {q4-q7}
        ldmfd           sp!, {r4, lr}
        bx              lr
END(rsdIntrinsicColorMatrixDot_K)


/*
static void OneVF(float4 *out, const uchar *ptrIn, int iStride,
                  const float* gPtr, int iradius, int x1, int x2)

    r0 = out
    r1 = pin
    r2 = stride
    r3 = gptr
    r4 = sp, ct
    r5 = sp+4, x1
    r6 = sp+8, x2
*/
ENTRY(rsdIntrinsicBlurVF_K)
        push            {r4-r8, r10, r11, lr}
        vpush           {q4-q7}

        ldr r4, [sp, #32+64]
        ldr r5, [sp, #32+64 + 4]
        ldr r6, [sp, #32+64 + 8]

1:
        veor q0, q0, q0         /* float4 blurredPixel = 0; */
        add r7, r1, r5, lsl #2  /* const uchar *pi = ptrIn + x1 * 4; */
        mov r10, r3

        mov r11, r4

2:
        vld1.32 {d2[0]}, [r7]
        vmovl.u8 q1, d2
        vmovl.u16 q1, d2
        vcvt.f32.s32 q1, q1
        vld1.32 {d6[0]}, [r10]!
        add r7, r7, r2
        vmla.f32 q0, q1, d6[0]
        subs r11, r11, #1
        bne 2b

        vst1.32 {q0}, [r0]!
        add r5, r5, #1
        cmp r5, r6
        bne 1b


        vpop            {q4-q7}
        pop             {r4-r8, r10, r11, lr}
        bx              lr
END(rsdIntrinsicBlurVF_K)

/*
static void OneVF(float4 *out, const uchar *ptrIn, int iStride,
                  const float* gPtr, int iradius, int x1, int x2)

    r0 = out
    r1 = pin
    r2 = gptr
    r3 = ct
    r4 = sp, x1
    r5 = sp+4, x2
*/
ENTRY(rsdIntrinsicBlurHF_K)
        push            {r4-r8, r10, r11, lr}
        vpush           {q4-q7}

        ldr r4, [sp, #32+64]
        ldr r5, [sp, #32+64 + 4]

1:
        veor q0, q0, q0         /* float4 blurredPixel = 0; */
        add r7, r1, r4, lsl #4  /* const uchar *pi = ptrIn + x1 * 4; */
        mov r10, r2
        mov r11, r3

2:
        vld1.32 {q1}, [r7]!
        vld1.32 {d6[0]}, [r10]!
        vmla.f32 q0, q1, d6[0]
        subs r11, r11, #1
        bne 2b

        vcvt.s32.f32 q0, q0
        vmovn.u32 d0, q0
        vmovn.u16 d0, q0

        vst1.32 {d0[0]}, [r0]!
        add r4, r4, #1
        cmp r4, r5
        bne 1b

        vpop            {q4-q7}
        pop             {r4-r8, r10, r11, lr}
        bx              lr
END(rsdIntrinsicBlurHF_K)

