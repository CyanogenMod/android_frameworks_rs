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

#define ENTRY(f) .text; .align 4; .globl f; .type f,#function; f:
#define END(f) .size f, .-f;

.set FRACTION_BITS, 7
.set MAX_R, 25


/* A quick way of making a line of code conditional on some other condition.
 * Use `.set cc, 1` or `.set cc, 0` to enable or disable lines prefixed with
 * `ifcc`:
 */
.macro ifcc zzz:vararg
.if cc
            \zzz
.endif
.endm

/* Fetch 16 columns of bytes (regardless of image format), convolve these
 * vertically, and leave them in the register file.  If working near the top or
 * bottom of an image then clamp the addressing while loading the data in.
 *
 * The convolution is fully unrolled for windows up to max_r, with the
 * outermost edges calculated first.  This way it's possible to branch directly
 * into the relevant part of the code for an arbitrary convolution radius.  Two
 * variants of the loop are produced; one eliminates the clamping code for a
 * slight speed advantage.
 *
 * Where the macro is called with reg=x, the specified register is taken to
 * contain a pre-calculated pointer into one of the two loops.
 *
 * Input:
 *      x1 -- src
 *      x2 -- pitch
 *      x5 -- r
 *      x6 -- rup
 *      x7 -- rdn
 *      x12 -- switch index
 *      q0-q3 -- coefficient table
 *      x13 = -pitch
 *      x15 = top-row in
 *      x16 = bottom-row in
 * Output:
 *      x1 += 16
 *      q10,q11 -- 16 convolved columns
 * Modifies:
 *      x10 = upper row pointer
 *      x11 = lower row pointer
 *      q12-q15 = temporary sums
 */
.macro fetch, max_r=MAX_R, labelc=1, labelnc=2, reg=x12 /*{{{*/
  .ifc \reg,x12 ; .set cc, 1 ; .else ; .set cc, 0 ; .endif

            ld1         {v15.16b}, [x1], #16
            mov         x10, x15

            uxtl        v14.8h, v15.8b
//            prfm        PLDL1KEEP,[x1, #16] // TODO: confirm
            uxtl2       v15.8h, v15.16b
  .if \max_r < 16 // approximate
    ifcc    adr         \reg, 1f
  .else
    ifcc    adrp        \reg, 1f
    ifcc    add         \reg, \reg, #:lo12:1f
  .endif

            umull       v12.4s, v14.4h, v0.h[0]
    ifcc    sub         \reg, \reg, x5, LSL #6
            umull2      v13.4s, v14.8h, v0.h[0]
            mov         x11, x16
            umull       v14.4s, v15.4h, v0.h[0]
    ifcc    add         \reg, \reg, x5, LSL #3
            umull2      v15.4s, v15.8h, v0.h[0]
            br          \reg

  .irp rowclamp, 1, 0
    .set cc, \rowclamp
    .align 4
    .irp dreg, 4, 3, 2, 1, 0 ; .irp lane, 7, 6, 5, 4, 3, 2, 1, 0 ; .irp doth, .h
        .set i, \dreg * 8 + \lane
        .if 0 < i && i <= \max_r
            ld1         {v10.16b}, [x10], x2
    ifcc    cmp         x6, #i
            ld1         {v11.16b}, [x11], x13
    ifcc    csel        x10, x15, x10, lo
            uaddl       v16.8h, v10.8b, v11.8b
    ifcc    cmp         x7, #i
            uaddl2      v11.8h, v10.16b, v11.16b
    ifcc    csel        x11, x16, x11, lo
            umlal       v12.4s, v16.4h, v\dreg\doth[\lane]
            umlal2      v13.4s, v16.8h, v\dreg\doth[\lane]
//            prfm        PLDL1KEEP,[x10, #32] // TODO: confirm
nop
            umlal       v14.4s, v11.4h, v\dreg\doth[\lane]
//            prfm        PLDL1KEEP,[x11, #32] // TODO: confirm
nop
            umlal2      v15.4s, v11.8h, v\dreg\doth[\lane]
        .endif
    .endr ; .endr ; .endr
    .if \rowclamp == 1
        1: \labelc :
            b           2f
    .else
        2: \labelnc :
    .endif
  .endr

            uqrshrn     v10.4h, v12.4s, #16 - FRACTION_BITS
            add         x15, x15, #16
            uqrshrn2    v10.8h, v13.4s, #16 - FRACTION_BITS
            add         x16, x16, #16
            uqrshrn     v11.4h, v14.4s, #16 - FRACTION_BITS
            uqrshrn2    v11.8h, v15.4s, #16 - FRACTION_BITS
.endm /*}}}*/

/* Some portion of the convolution window (as much as will fit, and all of it
 * for the uchar1 cases) is kept in the register file to avoid unnecessary
 * memory accesses.  This forces the horizontal loops to be unrolled because
 * there's no indexed addressing into the register file.
 *
 * As in the fetch macro, the operations are ordered from outside to inside, so
 * that jumping into the middle of the block bypasses the unwanted window taps.
 *
 * There are several variants of the macro because of the fixed offets of the
 * taps -- the wider the maximum radius the further the centre tap is from the
 * most recently fetched data.  This means that pre-filling the window requires
 * more data that won't be used and it means that rotating the window involves
 * more mov operations.
 *
 * When the buffer gets too big the buffer at [r9] is used.
 *
 * Input:
 *      q4-q11 -- convoltion window
 *      r9 -- pointer to additional convolution window data
 * Output:
 *      r9 -- updated buffer pointer (if used)
 *      d31 -- result to be stored
 * Modifies:
 *      r12 -- temp buffer pointer
 *      q12-q13 -- temporaries for load and vext operations.
 *      q14-q15 -- intermediate sums
 */
#define TUNED_LIST1 8, 16
.macro hconv1_8/*{{{*/
            umull       v14.4s, v9.4h, v0.h[0]
            umull2      v15.4s, v9.8h, v0.h[0]

            adr         x12, 199f-8
            ldr         x12, [x12, x5, LSL #3]
            br          x12
   199:     .xword 101f
            .xword 102f
            .xword 103f
            .xword 104f
            .xword 105f
            .xword 106f
            .xword 107f
            .xword 108f
            .align      4
    108:    umlal       v14.4s, v8.4h, v1.h[0]
            umlal2      v15.4s, v8.8h, v1.h[0]
            umlal       v14.4s, v10.4h, v1.h[0]
            umlal2      v15.4s, v10.8h, v1.h[0]
    107:    ext         v12.16b, v8.16b, v9.16b, #1*2
            ext         v13.16b, v9.16b, v10.16b, #7*2
            umlal       v14.4s, v12.4h, v0.h[7]
            umlal2      v15.4s, v12.8h, v0.h[7]
            umlal       v14.4s, v13.4h, v0.h[7]
            umlal2      v15.4s, v13.8h, v0.h[7]
    106:    ext         v12.16b, v8.16b, v9.16b, #2*2
            ext         v13.16b, v9.16b, v10.16b, #6*2
            umlal       v14.4s, v12.4h, v0.h[6]
            umlal2      v15.4s, v12.8h, v0.h[6]
            umlal       v14.4s, v13.4h, v0.h[6]
            umlal2      v15.4s, v13.8h, v0.h[6]
    105:    ext         v12.16b, v8.16b, v9.16b, #3*2
            ext         v13.16b, v9.16b, v10.16b, #5*2
            umlal       v14.4s, v12.4h, v0.h[5]
            umlal2      v15.4s, v12.8h, v0.h[5]
            umlal       v14.4s, v13.4h, v0.h[5]
            umlal2      v15.4s, v13.8h, v0.h[5]
    104:    //ext         v12.16b, v8.16b, v9.16b, #4*2
            //ext         v13.16b, v9.16b, v10.16b, #4*2
            umlal2      v14.4s, v8.8h, v0.h[4]
            umlal       v15.4s, v9.4h, v0.h[4]
            umlal2      v14.4s, v9.8h, v0.h[4]
            umlal       v15.4s, v10.4h, v0.h[4]
    103:    ext         v12.16b, v8.16b, v9.16b, #5*2
            ext         v13.16b, v9.16b, v10.16b, #3*2
            umlal       v14.4s, v12.4h, v0.h[3]
            umlal2      v15.4s, v12.8h, v0.h[3]
            umlal       v14.4s, v13.4h, v0.h[3]
            umlal2      v15.4s, v13.8h, v0.h[3]
    102:    ext         v12.16b, v8.16b, v9.16b, #6*2
            ext         v13.16b, v9.16b, v10.16b, #2*2
            umlal       v14.4s, v12.4h, v0.h[2]
            umlal2      v15.4s, v12.8h, v0.h[2]
            umlal       v14.4s, v13.4h, v0.h[2]
            umlal2      v15.4s, v13.8h, v0.h[2]
    101:    ext         v12.16b, v8.16b, v9.16b, #7*2
            ext         v13.16b, v9.16b, v10.16b, #1*2
            umlal       v14.4s, v12.4h, v0.h[1]
            umlal2      v15.4s, v12.8h, v0.h[1]
            umlal       v14.4s, v13.4h, v0.h[1]
            umlal2      v15.4s, v13.8h, v0.h[1]

            uqrshrn     v14.4h, v14.4s, #16
            uqrshrn2    v14.8h, v15.4s, #16
            uqrshrn     v15.8b, v14.8h, #FRACTION_BITS

            mov         v8.16b, v9.16b
            mov         v9.16b, v10.16b
            mov         v10.16b, v11.16b
.endm/*}}}*/

.macro hconv1_16/*{{{*/
            umull       v14.4s, v8.4h, v0.h[0]
            umull2      v15.4s, v8.8h, v0.h[0]

            adr         x12, 199f-8
            ldr         x12, [x12, x5, LSL #3]
            br          x12
   199:     .xword 101f
            .xword 102f
            .xword 103f
            .xword 104f
            .xword 105f
            .xword 106f
            .xword 107f
            .xword 108f
            .xword 109f
            .xword 110f
            .xword 111f
            .xword 112f
            .xword 113f
            .xword 114f
            .xword 115f
            .xword 116f
            .align 4
    116:    //ext         v12.16b, v6.16b, v7.16b, #0*2
            //ext         v13.16b, v10.16b, v11.16b, #0*2
            umlal       v14.4s, v6.4h, v2.h[0]
            umlal2      v15.4s, v6.8h, v2.h[0]
            umlal       v14.4s, v10.4h, v2.h[0]
            umlal2      v15.4s, v10.8h, v2.h[0]
    115:    ext         v12.16b, v6.16b, v7.16b, #1*2
            ext         v13.16b, v9.16b, v10.16b, #7*2
            umlal       v14.4s, v12.4h, v1.h[7]
            umlal2      v15.4s, v12.8h, v1.h[7]
            umlal       v14.4s, v13.4h, v1.h[7]
            umlal2      v15.4s, v13.8h, v1.h[7]
    114:    ext         v12.16b, v6.16b, v7.16b, #2*2
            ext         v13.16b, v9.16b, v10.16b, #6*2
            umlal       v14.4s, v12.4h, v1.h[6]
            umlal2      v15.4s, v12.8h, v1.h[6]
            umlal       v14.4s, v13.4h, v1.h[6]
            umlal2      v15.4s, v13.8h, v1.h[6]
    113:    ext         v12.16b, v6.16b, v7.16b, #3*2
            ext         v13.16b, v9.16b, v10.16b, #5*2
            umlal       v14.4s, v12.4h, v1.h[5]
            umlal2      v15.4s, v12.8h, v1.h[5]
            umlal       v14.4s, v13.4h, v1.h[5]
            umlal2      v15.4s, v13.8h, v1.h[5]
    112:    //ext         v12.16b, v6.16b, v7.16b, #4*2
            //ext         v13.16b, v9.16b, v10.16b, #4*2
            umlal2      v14.4s, v6.8h, v1.h[4]
            umlal       v15.4s, v7.4h, v1.h[4]
            umlal2      v14.4s, v9.8h, v1.h[4]
            umlal       v15.4s, v10.4h, v1.h[4]
    111:    ext         v12.16b, v6.16b, v7.16b, #5*2
            ext         v13.16b, v9.16b, v10.16b, #3*2
            umlal       v14.4s, v12.4h, v1.h[3]
            umlal2      v15.4s, v12.8h, v1.h[3]
            umlal       v14.4s, v13.4h, v1.h[3]
            umlal2      v15.4s, v13.8h, v1.h[3]
    110:    ext         v12.16b, v6.16b, v7.16b, #6*2
            ext         v13.16b, v9.16b, v10.16b, #2*2
            umlal       v14.4s, v12.4h, v1.h[2]
            umlal2      v15.4s, v12.8h, v1.h[2]
            umlal       v14.4s, v13.4h, v1.h[2]
            umlal2      v15.4s, v13.8h, v1.h[2]
    109:    ext         v12.16b, v6.16b, v7.16b, #7*2
            ext         v13.16b, v9.16b, v10.16b, #1*2
            umlal       v14.4s, v12.4h, v1.h[1]
            umlal2      v15.4s, v12.8h, v1.h[1]
            umlal       v14.4s, v13.4h, v1.h[1]
            umlal2      v15.4s, v13.8h, v1.h[1]
    108:    //ext         v12.16b, v7.16b, v8.16b, #0*2
            //ext         v13.16b, v9.16b, v10.16b, #0*2
            umlal       v14.4s, v7.4h, v1.h[0]
            umlal2      v15.4s, v7.8h, v1.h[0]
            umlal       v14.4s, v9.4h, v1.h[0]
            umlal2      v15.4s, v9.8h, v1.h[0]
    107:    ext         v12.16b, v7.16b, v8.16b, #1*2
            ext         v13.16b, v8.16b, v9.16b, #7*2
            umlal       v14.4s, v12.4h, v0.h[7]
            umlal2      v15.4s, v12.8h, v0.h[7]
            umlal       v14.4s, v13.4h, v0.h[7]
            umlal2      v15.4s, v13.8h, v0.h[7]
    106:    ext         v12.16b, v7.16b, v8.16b, #2*2
            ext         v13.16b, v8.16b, v9.16b, #6*2
            umlal       v14.4s, v12.4h, v0.h[6]
            umlal2      v15.4s, v12.8h, v0.h[6]
            umlal       v14.4s, v13.4h, v0.h[6]
            umlal2      v15.4s, v13.8h, v0.h[6]
    105:    ext         v12.16b, v7.16b, v8.16b, #3*2
            ext         v13.16b, v8.16b, v9.16b, #5*2
            umlal       v14.4s, v12.4h, v0.h[5]
            umlal2      v15.4s, v12.8h, v0.h[5]
            umlal       v14.4s, v13.4h, v0.h[5]
            umlal2      v15.4s, v13.8h, v0.h[5]
    104:    //ext         v12.16b, v7.16b, v8.16b, #4*2
            //ext         v13.16b, v8.16b, v9.16b, #4*2
            umlal2      v14.4s, v7.8h, v0.h[4]
            umlal       v15.4s, v8.4h, v0.h[4]
            umlal2      v14.4s, v8.8h, v0.h[4]
            umlal       v15.4s, v9.4h, v0.h[4]
    103:    ext         v12.16b, v7.16b, v8.16b, #5*2
            ext         v13.16b, v8.16b, v9.16b, #3*2
            umlal       v14.4s, v12.4h, v0.h[3]
            umlal2      v15.4s, v12.8h, v0.h[3]
            umlal       v14.4s, v13.4h, v0.h[3]
            umlal2      v15.4s, v13.8h, v0.h[3]
    102:    ext         v12.16b, v7.16b, v8.16b, #6*2
            ext         v13.16b, v8.16b, v9.16b, #2*2
            umlal       v14.4s, v12.4h, v0.h[2]
            umlal2      v15.4s, v12.8h, v0.h[2]
            umlal       v14.4s, v13.4h, v0.h[2]
            umlal2      v15.4s, v13.8h, v0.h[2]
    101:    ext         v12.16b, v7.16b, v8.16b, #7*2
            ext         v13.16b, v8.16b, v9.16b, #1*2
            umlal       v14.4s, v12.4h, v0.h[1]
            umlal2      v15.4s, v12.8h, v0.h[1]
            umlal       v14.4s, v13.4h, v0.h[1]
            umlal2      v15.4s, v13.8h, v0.h[1]

            uqrshrn     v14.4h, v14.4s, #16
            uqrshrn2    v14.8h, v15.4s, #16
            uqrshrn     v15.8b, v14.8h, #FRACTION_BITS

            mov         v6.16b, v7.16b
            mov         v7.16b, v8.16b
            mov         v8.16b, v9.16b
            mov         v9.16b, v10.16b
            mov         v10.16b, v11.16b
.endm/*}}}*/

.macro hconv1_25/*{{{*/
            ext         v12.16b, v6.16b, v7.16b, #7*2
            umull       v14.4s, v12.4h, v0.h[0]
            umull2      v15.4s, v12.8h, v0.h[0]

            adr         x12, 199f-8
            ldr         x12, [x12, x5, LSL #3]
            br          x12
   199:     .xword 101f
            .xword 102f
            .xword 103f
            .xword 104f
            .xword 105f
            .xword 106f
            .xword 107f
            .xword 108f
            .xword 109f
            .xword 110f
            .xword 111f
            .xword 112f
            .xword 113f
            .xword 114f
            .xword 115f
            .xword 116f
            .xword 117f
            .xword 118f
            .xword 119f
            .xword 120f
            .xword 121f
            .xword 122f
            .xword 123f
            .xword 124f
            .xword 125f
            .align 4
    125:    ext         v12.16b, v3.16b, v4.16b, #6*2
            ext         v13.16b, v10.16b, v11.16b, #0*2
            umlal       v14.4s, v12.4h, v3.h[1]
            umlal2      v15.4s, v12.8h, v3.h[1]
            umlal       v14.4s, v13.4h, v3.h[1]
            umlal2      v15.4s, v13.8h, v3.h[1]
    124:    ext         v12.16b, v3.16b, v4.16b, #7*2
            ext         v13.16b, v9.16b, v10.16b, #7*2
            umlal       v14.4s, v12.4h, v3.h[0]
            umlal2      v15.4s, v12.8h, v3.h[0]
            umlal       v14.4s, v13.4h, v3.h[0]
            umlal2      v15.4s, v13.8h, v3.h[0]
    123:    ext         v12.16b, v4.16b, v5.16b, #0*2
            ext         v13.16b, v9.16b, v10.16b, #6*2
            umlal       v14.4s, v12.4h, v2.h[7]
            umlal2      v15.4s, v12.8h, v2.h[7]
            umlal       v14.4s, v13.4h, v2.h[7]
            umlal2      v15.4s, v13.8h, v2.h[7]
    122:    ext         v12.16b, v4.16b, v5.16b, #1*2
            ext         v13.16b, v9.16b, v10.16b, #5*2
            umlal       v14.4s, v12.4h, v2.h[6]
            umlal2      v15.4s, v12.8h, v2.h[6]
            umlal       v14.4s, v13.4h, v2.h[6]
            umlal2      v15.4s, v13.8h, v2.h[6]
    121:    ext         v12.16b, v4.16b, v5.16b, #2*2
            ext         v13.16b, v9.16b, v10.16b, #4*2
            umlal       v14.4s, v12.4h, v2.h[5]
            umlal2      v15.4s, v12.8h, v2.h[5]
            umlal       v14.4s, v13.4h, v2.h[5]
            umlal2      v15.4s, v13.8h, v2.h[5]
    120:    ext         v12.16b, v4.16b, v5.16b, #3*2
            ext         v13.16b, v9.16b, v10.16b, #3*2
            umlal       v14.4s, v12.4h, v2.h[4]
            umlal2      v15.4s, v12.8h, v2.h[4]
            umlal       v14.4s, v13.4h, v2.h[4]
            umlal2      v15.4s, v13.8h, v2.h[4]
    119:    ext         v12.16b, v4.16b, v5.16b, #4*2
            ext         v13.16b, v9.16b, v10.16b, #2*2
            umlal       v14.4s, v12.4h, v2.h[3]
            umlal2      v15.4s, v12.8h, v2.h[3]
            umlal       v14.4s, v13.4h, v2.h[3]
            umlal2      v15.4s, v13.8h, v2.h[3]
    118:    ext         v12.16b, v4.16b, v5.16b, #5*2
            ext         v13.16b, v9.16b, v10.16b, #1*2
            umlal       v14.4s, v12.4h, v2.h[2]
            umlal2      v15.4s, v12.8h, v2.h[2]
            umlal       v14.4s, v13.4h, v2.h[2]
            umlal2      v15.4s, v13.8h, v2.h[2]
    117:    ext         v12.16b, v4.16b, v5.16b, #6*2
            ext         v13.16b, v9.16b, v10.16b, #0*2
            umlal       v14.4s, v12.4h, v2.h[1]
            umlal2      v15.4s, v12.8h, v2.h[1]
            umlal       v14.4s, v13.4h, v2.h[1]
            umlal2      v15.4s, v13.8h, v2.h[1]
    116:    ext         v12.16b, v4.16b, v5.16b, #7*2
            ext         v13.16b, v8.16b, v9.16b, #7*2
            umlal       v14.4s, v12.4h, v2.h[0]
            umlal2      v15.4s, v12.8h, v2.h[0]
            umlal       v14.4s, v13.4h, v2.h[0]
            umlal2      v15.4s, v13.8h, v2.h[0]
    115:    ext         v12.16b, v5.16b, v6.16b, #0*2
            ext         v13.16b, v8.16b, v9.16b, #6*2
            umlal       v14.4s, v12.4h, v1.h[7]
            umlal2      v15.4s, v12.8h, v1.h[7]
            umlal       v14.4s, v13.4h, v1.h[7]
            umlal2      v15.4s, v13.8h, v1.h[7]
    114:    ext         v12.16b, v5.16b, v6.16b, #1*2
            ext         v13.16b, v8.16b, v9.16b, #5*2
            umlal       v14.4s, v12.4h, v1.h[6]
            umlal2      v15.4s, v12.8h, v1.h[6]
            umlal       v14.4s, v13.4h, v1.h[6]
            umlal2      v15.4s, v13.8h, v1.h[6]
    113:    ext         v12.16b, v5.16b, v6.16b, #2*2
            ext         v13.16b, v8.16b, v9.16b, #4*2
            umlal       v14.4s, v12.4h, v1.h[5]
            umlal2      v15.4s, v12.8h, v1.h[5]
            umlal       v14.4s, v13.4h, v1.h[5]
            umlal2      v15.4s, v13.8h, v1.h[5]
    112:    ext         v12.16b, v5.16b, v6.16b, #3*2
            ext         v13.16b, v8.16b, v9.16b, #3*2
            umlal       v14.4s, v12.4h, v1.h[4]
            umlal2      v15.4s, v12.8h, v1.h[4]
            umlal       v14.4s, v13.4h, v1.h[4]
            umlal2      v15.4s, v13.8h, v1.h[4]
    111:    ext         v12.16b, v5.16b, v6.16b, #4*2
            ext         v13.16b, v8.16b, v9.16b, #2*2
            umlal       v14.4s, v12.4h, v1.h[3]
            umlal2      v15.4s, v12.8h, v1.h[3]
            umlal       v14.4s, v13.4h, v1.h[3]
            umlal2      v15.4s, v13.8h, v1.h[3]
    110:    ext         v12.16b, v5.16b, v6.16b, #5*2
            ext         v13.16b, v8.16b, v9.16b, #1*2
            umlal       v14.4s, v12.4h, v1.h[2]
            umlal2      v15.4s, v12.8h, v1.h[2]
            umlal       v14.4s, v13.4h, v1.h[2]
            umlal2      v15.4s, v13.8h, v1.h[2]
    109:    ext         v12.16b, v5.16b, v6.16b, #6*2
            ext         v13.16b, v8.16b, v9.16b, #0*2
            umlal       v14.4s, v12.4h, v1.h[1]
            umlal2      v15.4s, v12.8h, v1.h[1]
            umlal       v14.4s, v13.4h, v1.h[1]
            umlal2      v15.4s, v13.8h, v1.h[1]
    108:    ext         v12.16b, v5.16b, v6.16b, #7*2
            ext         v13.16b, v7.16b, v8.16b, #7*2
            umlal       v14.4s, v12.4h, v1.h[0]
            umlal2      v15.4s, v12.8h, v1.h[0]
            umlal       v14.4s, v13.4h, v1.h[0]
            umlal2      v15.4s, v13.8h, v1.h[0]
    107:    ext         v12.16b, v6.16b, v7.16b, #0*2
            ext         v13.16b, v7.16b, v8.16b, #6*2
            umlal       v14.4s, v12.4h, v0.h[7]
            umlal2      v15.4s, v12.8h, v0.h[7]
            umlal       v14.4s, v13.4h, v0.h[7]
            umlal2      v15.4s, v13.8h, v0.h[7]
    106:    ext         v12.16b, v6.16b, v7.16b, #1*2
            ext         v13.16b, v7.16b, v8.16b, #5*2
            umlal       v14.4s, v12.4h, v0.h[6]
            umlal2      v15.4s, v12.8h, v0.h[6]
            umlal       v14.4s, v13.4h, v0.h[6]
            umlal2      v15.4s, v13.8h, v0.h[6]
    105:    ext         v12.16b, v6.16b, v7.16b, #2*2
            ext         v13.16b, v7.16b, v8.16b, #4*2
            umlal       v14.4s, v12.4h, v0.h[5]
            umlal2      v15.4s, v12.8h, v0.h[5]
            umlal       v14.4s, v13.4h, v0.h[5]
            umlal2      v15.4s, v13.8h, v0.h[5]
    104:    ext         v12.16b, v6.16b, v7.16b, #3*2
            ext         v13.16b, v7.16b, v8.16b, #3*2
            umlal       v14.4s, v12.4h, v0.h[4]
            umlal2      v15.4s, v12.8h, v0.h[4]
            umlal       v14.4s, v13.4h, v0.h[4]
            umlal2      v15.4s, v13.8h, v0.h[4]
    103:    ext         v12.16b, v6.16b, v7.16b, #4*2
            ext         v13.16b, v7.16b, v8.16b, #2*2
            umlal       v14.4s, v12.4h, v0.h[3]
            umlal2      v15.4s, v12.8h, v0.h[3]
            umlal       v14.4s, v13.4h, v0.h[3]
            umlal2      v15.4s, v13.8h, v0.h[3]
    102:    ext         v12.16b, v6.16b, v7.16b, #5*2
            ext         v13.16b, v7.16b, v8.16b, #1*2
            umlal       v14.4s, v12.4h, v0.h[2]
            umlal2      v15.4s, v12.8h, v0.h[2]
            umlal       v14.4s, v13.4h, v0.h[2]
            umlal2      v15.4s, v13.8h, v0.h[2]
    101:    ext         v12.16b, v6.16b, v7.16b, #6*2
            ext         v13.16b, v7.16b, v8.16b, #0*2
            umlal       v14.4s, v12.4h, v0.h[1]
            umlal2      v15.4s, v12.8h, v0.h[1]
            umlal       v14.4s, v13.4h, v0.h[1]
            umlal2      v15.4s, v13.8h, v0.h[1]

            uqrshrn     v14.4h, v14.4s, #16
            uqrshrn2    v14.8h, v15.4s, #16
            uqrshrn     v15.8b, v14.8h, #FRACTION_BITS

            ins         v3.d[1], v4.d[0]
            mov         v4.16b, v5.16b
            mov         v5.16b, v6.16b
            mov         v6.16b, v7.16b
            mov         v7.16b, v8.16b
            mov         v8.16b, v9.16b
            mov         v9.16b, v10.16b
            mov         v10.16b, v11.16b
.endm/*}}}*/

#define TUNED_LIST4 6, 12
.macro hconv4_6/*{{{*/
            umull       v14.4s, v7.4h, v0.h[0]
            umull2      v15.4s, v7.8h, v0.h[0]

            adr         x12, 199f-8
            ldr         x12, [x12, x5, LSL #3]
            br          x12
   199:     .xword 101f
            .xword 102f
            .xword 103f
            .xword 104f
            .xword 105f
            .xword 106f
            .align      4
    106:    umlal       v14.4s, v4.4h,  v0.h[6]
            umlal2      v15.4s, v4.8h,  v0.h[6]
            umlal       v14.4s, v10.4h, v0.h[6]
            umlal2      v15.4s, v10.8h, v0.h[6]
    105:    umlal2      v14.4s, v4.8h,  v0.h[5]
            umlal       v15.4s, v5.4h, v0.h[5]
            umlal2      v14.4s, v9.8h, v0.h[5]
            umlal       v15.4s, v10.4h, v0.h[5]
    104:    umlal       v14.4s, v5.4h, v0.h[4]
            umlal2      v15.4s, v5.8h, v0.h[4]
            umlal       v14.4s, v9.4h, v0.h[4]
            umlal2      v15.4s, v9.8h, v0.h[4]
    103:    umlal2      v14.4s, v5.8h, v0.h[3]
            umlal       v15.4s, v6.4h, v0.h[3]
            umlal2      v14.4s, v8.8h, v0.h[3]
            umlal       v15.4s, v9.4h, v0.h[3]
    102:    umlal       v14.4s, v6.4h, v0.h[2]
            umlal2      v15.4s, v6.8h, v0.h[2]
            umlal       v14.4s, v8.4h, v0.h[2]
            umlal2      v15.4s, v8.8h, v0.h[2]
    101:    umlal2      v14.4s, v6.8h, v0.h[1]
            umlal       v15.4s, v7.4h, v0.h[1]
            umlal2      v14.4s, v7.8h, v0.h[1]
            umlal       v15.4s, v8.4h, v0.h[1]

            uqrshrn     v14.4h, v14.4s, #16
            uqrshrn2    v14.8h, v15.4s, #16
            uqrshrn     v15.8b, v14.8h, #FRACTION_BITS

            mov         v4.16b, v5.16b
            mov         v5.16b, v6.16b
            mov         v6.16b, v7.16b
            mov         v7.16b, v8.16b
            mov         v8.16b, v9.16b
            mov         v9.16b, v10.16b
            mov         v10.16b, v11.16b
.endm/*}}}*/

.macro hconv4_12/*{{{*/
            umull       v14.4s, v4.4h, v0.h[0]
            umull2      v15.4s, v4.8h, v0.h[0]

            adr         x12, 199f-8
            ldr         x12, [x12, x5, LSL #3]
            br          x12
   199:     .xword 101f
            .xword 102f
            .xword 103f
            .xword 104f
            .xword 105f
            .xword 106f
            .xword 107f
            .xword 108f
            .xword 109f
            .xword 110f
            .xword 111f
            .xword 112f
            .align 4
    112:    add         x12, x9, #0x1a0
            bic         x12, x12, #0x200
            ld1         {v12.8h}, [x12]
            umlal       v14.4s, v12.4h, v1.h[4]
            umlal2      v15.4s, v12.8h, v1.h[4]
            umlal       v14.4s, v10.4h, v1.h[4]
            umlal2      v15.4s, v10.8h, v1.h[4]
    111:    add         x12, x9, #0x1a8
            bic         x12, x12, #0x200
            ld1         {v12.4h}, [x12], #8
            bic         x12, x12, #0x200
            ld1         {v13.4h}, [x12]
            umlal       v14.4s, v12.4h, v1.h[3]
            umlal       v15.4s, v13.4h, v1.h[3]
            umlal2      v14.4s, v9.8h, v1.h[3]
            umlal       v15.4s, v10.4h, v1.h[3]
    110:    add         x12, x9, #0x1b0
            bic         x12, x12, #0x200
            ld1         {v12.8h}, [x12]
            umlal       v14.4s, v12.4h, v1.h[2]
            umlal2      v15.4s, v12.8h, v1.h[2]
            umlal       v14.4s, v9.4h, v1.h[2]
            umlal2      v15.4s, v9.8h, v1.h[2]
    109:    add         x12, x9, #0x1b8
            bic         x12, x12, #0x200
            ld1         {v12.4h}, [x12], #8
            bic         x12, x12, #0x200
            ld1         {v13.4h}, [x12]
            umlal       v14.4s, v12.4h, v1.h[1]
            umlal       v15.4s, v13.4h, v1.h[1]
            umlal2      v14.4s, v8.8h, v1.h[1]
            umlal       v15.4s, v9.4h, v1.h[1]
    108:    add         x12, x9, #0x1c0
            bic         x12, x12, #0x200
            ld1         {v12.8h}, [x12]
            umlal       v14.4s, v12.4h, v1.h[0]
            umlal2      v15.4s, v12.8h, v1.h[0]
            umlal       v14.4s, v8.4h, v1.h[0]
            umlal2      v15.4s, v8.8h, v1.h[0]
    107:    add         x12, x9, #0x1c8
            bic         x12, x12, #0x200
            ld1         {v12.4h}, [x12], #8
            bic         x12, x12, #0x200
            ld1         {v13.4h}, [x12]
            umlal       v14.4s, v12.4h, v0.h[7]
            umlal       v15.4s, v13.4h, v0.h[7]
            umlal2      v14.4s, v7.8h, v0.h[7]
            umlal       v15.4s, v8.4h, v0.h[7]
    106:    add         x12, x9, #0x1d0
            bic         x12, x12, #0x200
            ld1         {v12.8h}, [x12]
            umlal       v14.4s, v12.4h, v0.h[6]
            umlal2      v15.4s, v12.8h, v0.h[6]
            umlal       v14.4s, v7.4h, v0.h[6]
            umlal2      v15.4s, v7.8h, v0.h[6]
    105:    add         x12, x9, #0x1d8
            bic         x12, x12, #0x200
            ld1         {v12.4h}, [x12], #8
            bic         x12, x12, #0x200
            ld1         {v13.4h}, [x12]
            umlal       v14.4s, v12.4h, v0.h[5]
            umlal       v15.4s, v13.4h, v0.h[5]
            umlal2      v14.4s, v6.8h, v0.h[5]
            umlal       v15.4s, v7.4h, v0.h[5]
    104:    add         x12, x9, #0x1e0
            bic         x12, x12, #0x200
            ld1         {v12.8h}, [x12]
            umlal       v14.4s, v12.4h, v0.h[4]
            umlal2      v15.4s, v12.8h, v0.h[4]
            umlal       v14.4s, v6.4h, v0.h[4]
            umlal2      v15.4s, v6.8h, v0.h[4]
    103:    add         x12, x9, #0x1e8
            bic         x12, x12, #0x200
            ld1         {v12.4h}, [x12], #8
            bic         x12, x12, #0x200
            ld1         {v13.4h}, [x12]
            umlal       v14.4s, v12.4h, v0.h[3]
            umlal       v15.4s, v13.4h, v0.h[3]
            umlal2      v14.4s, v5.8h, v0.h[3]
            umlal       v15.4s, v6.4h, v0.h[3]
    102:    add         x12, x9, #0x1f0
            bic         x12, x12, #0x200
            ld1         {v12.8h}, [x12]
            umlal       v14.4s, v12.4h, v0.h[2]
            umlal2      v15.4s, v12.8h, v0.h[2]
            umlal       v14.4s, v5.4h, v0.h[2]
            umlal2      v15.4s, v5.8h, v0.h[2]
    101:    add         x12, x9, #0x1f8
            bic         x12, x12, #0x200
            ld1         {v12.4h}, [x12]
            umlal       v14.4s, v12.4h, v0.h[1]
            umlal       v15.4s, v4.4h,  v0.h[1]
            umlal2      v14.4s, v4.8h,  v0.h[1]
            umlal       v15.4s, v5.4h, v0.h[1]

            uqrshrn     v14.4h, v14.4s, #16
            uqrshrn2    v14.8h, v15.4s, #16
            uqrshrn     v15.8b, v14.8h, #FRACTION_BITS

            st1         {v4.16b}, [x9], #16
            bic         x9, x9, #0x200
            mov         v4.16b, v5.16b
            mov         v5.16b, v6.16b
            mov         v6.16b, v7.16b
            mov         v7.16b, v8.16b
            mov         v8.16b, v9.16b
            mov         v9.16b, v10.16b
            mov         v10.16b, v11.16b
.endm/*}}}*/

.macro hconv4_25/*{{{*/
            add         x12, x9, #0x198
            bic         x12, x12, #0x200
            ld1         {v12.4h}, [x12], #8
            bic         x12, x12, #0x200
            ld1         {v13.4h}, [x12]
            umull       v14.4s, v12.4h, v0.h[0]
            umull       v15.4s, v13.4h, v0.h[0]

            adr         x12, 199f-8
            ldr         x12, [x12, x5, LSL #3]
            br          x12
   199:     .xword 101f
            .xword 102f
            .xword 103f
            .xword 104f
            .xword 105f
            .xword 106f
            .xword 107f
            .xword 108f
            .xword 109f
            .xword 110f
            .xword 111f
            .xword 112f
            .xword 113f
            .xword 114f
            .xword 115f
            .xword 116f
            .xword 117f
            .xword 118f
            .xword 119f
            .xword 120f
            .xword 121f
            .xword 122f
            .xword 123f
            .xword 124f
            .xword 125f
            .align 4
    125:    add         x12, x9, #0x0d0
            bic         x12, x12, #0x200
            ld1         {v12.8h}, [x12]
            umlal       v14.4s, v12.4h, v3.h[1]
            umlal2      v15.4s, v12.8h, v3.h[1]
            umlal       v14.4s, v10.4h, v3.h[1]
            umlal2      v15.4s, v10.8h, v3.h[1]
    124:    add         x12, x9, #0x0d8
            bic         x12, x12, #0x200
            ld1         {v12.4h}, [x12], #8
            bic         x12, x12, #0x200
            ld1         {v13.4h}, [x12]
            umlal       v14.4s, v12.4h, v3.h[0]
            umlal       v15.4s, v13.4h, v3.h[0]
            umlal2      v14.4s, v9.8h, v3.h[0]
            umlal       v15.4s, v10.4h, v3.h[0]
    123:    add         x12, x9, #0x0e0
            bic         x12, x12, #0x200
            ld1         {v12.8h}, [x12]
            umlal       v14.4s, v12.4h, v2.h[7]
            umlal2      v15.4s, v12.8h, v2.h[7]
            umlal       v14.4s, v9.4h, v2.h[7]
            umlal2      v15.4s, v9.8h, v2.h[7]
    122:    add         x12, x9, #0x0e8
            bic         x12, x12, #0x200
            ld1         {v12.4h}, [x12], #8
            bic         x12, x12, #0x200
            ld1         {v13.4h}, [x12]
            umlal       v14.4s, v12.4h, v2.h[6]
            umlal       v15.4s, v13.4h, v2.h[6]
            umlal2      v14.4s, v8.8h, v2.h[6]
            umlal       v15.4s, v9.4h, v2.h[6]
    121:    add         x12, x9, #0x0f0
            bic         x12, x12, #0x200
            ld1         {v12.8h}, [x12]
            umlal       v14.4s, v12.4h, v2.h[5]
            umlal2      v15.4s, v12.8h, v2.h[5]
            umlal       v14.4s, v8.4h, v2.h[5]
            umlal2      v15.4s, v8.8h, v2.h[5]
    120:    add         x12, x9, #0x0f8
            bic         x12, x12, #0x200
            ld1         {v12.4h}, [x12], #8
            bic         x12, x12, #0x200
            ld1         {v13.4h}, [x12]
            umlal       v14.4s, v12.4h, v2.h[4]
            umlal       v15.4s, v13.4h, v2.h[4]
            umlal2      v14.4s, v7.8h, v2.h[4]
            umlal       v15.4s, v8.4h, v2.h[4]
    119:    add         x12, x9, #0x100
            bic         x12, x12, #0x200
            ld1         {v12.8h}, [x12]
            umlal       v14.4s, v12.4h, v2.h[3]
            umlal2      v15.4s, v12.8h, v2.h[3]
            umlal       v14.4s, v7.4h, v2.h[3]
            umlal2      v15.4s, v7.8h, v2.h[3]
    118:    add         x12, x9, #0x108
            bic         x12, x12, #0x200
            ld1         {v12.4h}, [x12], #8
            bic         x12, x12, #0x200
            ld1         {v13.4h}, [x12]
            umlal       v14.4s, v12.4h, v2.h[2]
            umlal       v15.4s, v13.4h, v2.h[2]
            umlal2      v14.4s, v6.8h, v2.h[2]
            umlal       v15.4s, v7.4h, v2.h[2]
    117:    add         x12, x9, #0x110
            bic         x12, x12, #0x200
            ld1         {v12.8h}, [x12]
            umlal       v14.4s, v12.4h, v2.h[1]
            umlal2      v15.4s, v12.8h, v2.h[1]
            umlal       v14.4s, v6.4h, v2.h[1]
            umlal2      v15.4s, v6.8h, v2.h[1]
    116:    add         x12, x9, #0x118
            bic         x12, x12, #0x200
            ld1         {v12.4h}, [x12], #8
            bic         x12, x12, #0x200
            ld1         {v13.4h}, [x12]
            umlal       v14.4s, v12.4h, v2.h[0]
            umlal       v15.4s, v13.4h, v2.h[0]
            umlal2      v14.4s, v5.8h, v2.h[0]
            umlal       v15.4s, v6.4h, v2.h[0]
    115:    add         x12, x9, #0x120
            bic         x12, x12, #0x200
            ld1         {v12.8h}, [x12]
            umlal       v14.4s, v12.4h, v1.h[7]
            umlal2      v15.4s, v12.8h, v1.h[7]
            umlal       v14.4s, v5.4h, v1.h[7]
            umlal2      v15.4s, v5.8h, v1.h[7]
    114:    add         x12, x9, #0x128
            bic         x12, x12, #0x200
            ld1         {v12.4h}, [x12], #8
            bic         x12, x12, #0x200
            ld1         {v13.4h}, [x12]
            umlal       v14.4s, v12.4h, v1.h[6]
            umlal       v15.4s, v13.4h, v1.h[6]
            umlal2      v14.4s, v4.8h,  v1.h[6]
            umlal       v15.4s, v5.4h, v1.h[6]
    113:    add         x12, x9, #0x130
            bic         x12, x12, #0x200
            ld1         {v12.8h}, [x12]
            umlal       v14.4s, v12.4h, v1.h[5]
            umlal2      v15.4s, v12.8h, v1.h[5]
            umlal       v14.4s, v4.4h,  v1.h[5]
            umlal2      v15.4s, v4.8h,  v1.h[5]
    112:    add         x12, x9, #0x138
            bic         x12, x12, #0x200
            ld1         {v12.4h}, [x12], #8
            bic         x12, x12, #0x200
            ld1         {v16.4h}, [x12]
                                            add         x12, x9, #0x1f8
                                            bic         x12, x12, #0x200
                                            ld1         {v13.4h}, [x12]
            umlal       v14.4s, v12.4h, v1.h[4]
            umlal       v15.4s, v16.4h, v1.h[4]
            umlal       v14.4s, v13.4h, v1.h[4]   // Could be d7, without the load, right?
            umlal       v15.4s, v4.4h,  v1.h[4]
    111:    add         x12, x9, #0x140
            bic         x12, x12, #0x200
            ld1         {v12.8h}, [x12]
                                            add         x12, x9, #0x1f0
                                            bic         x12, x12, #0x200
                                            ld1         {v13.8h}, [x12]
            umlal       v14.4s, v12.4h, v1.h[3]
            umlal2      v15.4s, v12.8h, v1.h[3]
            umlal       v14.4s, v13.4h, v1.h[3]
            umlal2      v15.4s, v13.8h, v1.h[3]
    110:    add         x12, x9, #0x148
            bic         x12, x12, #0x200
            ld1         {v12.4h}, [x12], #8
            bic         x12, x12, #0x200
            ld1         {v16.4h}, [x12]
                                            add         x12, x9, #0x1e8
                                            bic         x12, x12, #0x200
                                            ld1         {v13.4h}, [x12], #8
                                            bic         x12, x12, #0x200
                                            ld1         {v17.4h}, [x12]
            umlal       v14.4s, v12.4h, v1.h[2]
            umlal       v15.4s, v16.4h, v1.h[2]
            umlal       v14.4s, v13.4h, v1.h[2]
            umlal       v15.4s, v17.4h, v1.h[2]
    109:    add         x12, x9, #0x150
            bic         x12, x12, #0x200
            ld1         {v12.8h}, [x12]
                                            add         x12, x9, #0x1e0
                                            bic         x12, x12, #0x200
                                            ld1         {v13.8h}, [x12]
            umlal       v14.4s, v12.4h, v1.h[1]
            umlal2      v15.4s, v12.8h, v1.h[1]
            umlal       v14.4s, v13.4h, v1.h[1]
            umlal2      v15.4s, v13.8h, v1.h[1]
    108:    add         x12, x9, #0x158
            bic         x12, x12, #0x200
            ld1         {v12.4h}, [x12], #8
            bic         x12, x12, #0x200
            ld1         {v16.4h}, [x12]
                                            add         x12, x9, #0x1d8
                                            bic         x12, x12, #0x200
                                            ld1         {v13.4h}, [x12], #8
                                            bic         x12, x12, #0x200
                                            ld1         {v17.4h}, [x12]
            umlal       v14.4s, v12.4h, v1.h[0]
            umlal       v15.4s, v16.4h, v1.h[0]
            umlal       v14.4s, v13.4h, v1.h[0]
            umlal       v15.4s, v17.4h, v1.h[0]
    107:    add         x12, x9, #0x160
            bic         x12, x12, #0x200
            ld1         {v12.8h}, [x12]
                                            add         x12, x9, #0x1d0
                                            bic         x12, x12, #0x200
                                            ld1         {v13.8h}, [x12]
            umlal       v14.4s, v12.4h, v0.h[7]
            umlal2      v15.4s, v12.8h, v0.h[7]
            umlal       v14.4s, v13.4h, v0.h[7]
            umlal2      v15.4s, v13.8h, v0.h[7]
    106:    add         x12, x9, #0x168
            bic         x12, x12, #0x200
            ld1         {v12.4h}, [x12], #8
            bic         x12, x12, #0x200
            ld1         {v16.4h}, [x12]
                                            add         x12, x9, #0x1c8
                                            bic         x12, x12, #0x200
                                            ld1         {v13.4h}, [x12], #8
                                            bic         x12, x12, #0x200
                                            ld1         {v17.4h}, [x12]
            umlal       v14.4s, v12.4h, v0.h[6]
            umlal       v15.4s, v16.4h, v0.h[6]
            umlal       v14.4s, v13.4h, v0.h[6]
            umlal       v15.4s, v17.4h, v0.h[6]
    105:    add         x12, x9, #0x170
            bic         x12, x12, #0x200
            ld1         {v12.8h}, [x12]
                                            add         x12, x9, #0x1c0
                                            bic         x12, x12, #0x200
                                            ld1         {v13.8h}, [x12]
            umlal       v14.4s, v12.4h, v0.h[5]
            umlal2      v15.4s, v12.8h, v0.h[5]
            umlal       v14.4s, v13.4h, v0.h[5]
            umlal2      v15.4s, v13.8h, v0.h[5]
    104:    add         x12, x9, #0x178
            bic         x12, x12, #0x200
            ld1         {v12.4h}, [x12], #8
            bic         x12, x12, #0x200
            ld1         {v16.4h}, [x12]
                                            add         x12, x9, #0x1b8
                                            bic         x12, x12, #0x200
                                            ld1         {v13.4h}, [x12], #8
                                            bic         x12, x12, #0x200
                                            ld1         {v17.4h}, [x12]
            umlal       v14.4s, v12.4h, v0.h[4]
            umlal       v15.4s, v16.4h, v0.h[4]
            umlal       v14.4s, v13.4h, v0.h[4]
            umlal       v15.4s, v17.4h, v0.h[4]
    103:    add         x12, x9, #0x180
            bic         x12, x12, #0x200
            ld1         {v12.8h}, [x12]
                                            add         x12, x9, #0x1b0
                                            bic         x12, x12, #0x200
                                            ld1         {v13.8h}, [x12]
            umlal       v14.4s, v12.4h, v0.h[3]
            umlal2      v15.4s, v12.8h, v0.h[3]
            umlal       v14.4s, v13.4h, v0.h[3]
            umlal2      v15.4s, v13.8h, v0.h[3]
    102:    add         x12, x9, #0x188
            bic         x12, x12, #0x200
            ld1         {v12.4h}, [x12], #8
            bic         x12, x12, #0x200
            ld1         {v16.4h}, [x12]
                                            add         x12, x9, #0x1a8
                                            bic         x12, x12, #0x200
                                            ld1         {v13.4h}, [x12], #8
                                            bic         x12, x12, #0x200
                                            ld1         {v17.4h}, [x12]
            umlal       v14.4s, v12.4h, v0.h[2]
            umlal       v15.4s, v16.4h, v0.h[2]
            umlal       v14.4s, v13.4h, v0.h[2]
            umlal       v15.4s, v17.4h, v0.h[2]
    101:    add         x12, x9, #0x190
            bic         x12, x12, #0x200
            ld1         {v12.8h}, [x12], #16
            bic         x12, x12, #0x200
            ld1         {v13.8h}, [x12]
            umlal       v14.4s, v12.4h, v0.h[1]
            umlal2      v15.4s, v12.8h, v0.h[1]
            umlal       v14.4s, v13.4h, v0.h[1]
            umlal2      v15.4s, v13.8h, v0.h[1]

            uqrshrn     v14.4h, v14.4s, #16
            uqrshrn2    v14.8h, v15.4s, #16
            uqrshrn     v15.8b, v14.8h, #FRACTION_BITS

            st1         {v4.16b}, [x9], #16
            bic         x9, x9, #0x200
            mov         v4.16b, v5.16b
            mov         v5.16b, v6.16b
            mov         v6.16b, v7.16b
            mov         v7.16b, v8.16b
            mov         v8.16b, v9.16b
            mov         v9.16b, v10.16b
            mov         v10.16b, v11.16b
.endm/*}}}*/

/* Dedicated function wrapper for the fetch macro, for the cases where
 * performance isn't that important, to keep code size down.
 */
ENTRY(fetch_generic_asm)
            stp         x10, x11, [sp, #-16]!
            fetch
            ldp         x10, x11, [sp], #16
            ret
END(fetch_generic_asm)

/* Given values in q10 and q11, and an index in x11, sweep the (x11&15)th value
 * across to fill the rest of the register pair.  Used for filling the right
 * hand edge of the window when starting too close to the right hand edge of
 * the image.
 */
ENTRY(prefetch_clamp1)
            sub         x11, xzr, x11
            sub         x15, x15, x1
            sub         x16, x16, x1
            tbz         x11, #3, 1f
            mov         v11.16b, v10.16b
            sub         x1, x1, #16
1:          mov         v12.16b, v11.16b
            movi        v13.8b, #0xff
            tbz         x11, #2, 1f
            ext         v12.16b, v12.16b, v12.16b, #4*2
            sub         x1, x1, #8
            shl         v13.2d, v13.2d, #32
1:          tbz         x11, #1, 1f
            ext         v12.16b, v12.16b, v12.16b, #6*2
            sub         x1, x1, #4
            shl         v13.2d, v13.2d, #16
1:          tbz         x11, #0, 1f
            ext         v12.16b, v12.16b, v12.16b, #7*2
            sub         x1, x1, #2
            shl         v13.2d, v13.2d, #8
1:          dup         v12.8h, v12.h[6]
            sxtl        v13.8h, v13.8b
            bif         v11.16b, v12.16b, v13.16b
1:          tbz         x11, #3, 1f
            mov         v10.16b, v11.16b
            mov         v11.16b, v12.16b
1:          sub         x11, xzr, x11
            add         x15, x15, x1
            add         x16, x16, x1
            ret
END(prefetch_clamp1)

ENTRY(prefetch_clamp4)
            sub         x11, xzr, x11
            sub         x15, x15, x1
            sub         x16, x16, x1
            tbz         x11, #3, 1f
            sub         x1, x1, #16     // what's this?
            mov         v11.16b, v10.16b
1:          dup         v12.2d, v11.d[1]
            tbz         x11, #2, 1f
            dup         v12.2d, v11.d[0]
            sub         x1, x1, #8
            dup         v11.2d, v11.d[0]
1:          tbz         x11, #3, 1f
            mov         v10.16b, v11.16b
            mov         v11.16b, v12.16b
1:          sub         x11, xzr, x11
            add         x15, x15, x1
            add         x16, x16, x1
            ret
END(prefetch_clamp4)


/* Helpers for prefetch, below.
 */
.macro prefetch_out qa, qb, store, qsa, qsb, qsb_hi
  .if \store > 0
    .ifc \qsa,\qsb
            st1         {\qsa}, [x9], #16
            st1         {\qsb}, [x9], #16
    .else
            st1         {\qsa,\qsb}, [x9], #32
    .endif
  .elseif \store == 0
            mov         \qa, \qsa
            mov         \qb, \qsb
  .else
            ins         \qb, \qsb_hi
  .endif
.endm

.macro prefetch_one  qa, qb, rem, c, store=0, step=1
.set i, (need - 16) - \rem
.if i >= 0
1:          cmp         x10, #i+16
            blo         2f
            prefetch_out \qa, \qb, \store, v9.16b, v9.16b, v9.d[1]
            b           1f
2:          cmp         x11, #i+16
            bls         3f
            prefetch_out \qa, \qb, \store, v10.16b, v11.16b, v11.d[1]
            bl          fetch_generic_asm
            b           2f
3:          bl          prefetch_clamp\step
            prefetch_out \qa, \qb, \store, v10.16b, v11.16b, v11.d[1]
4:          b           4f+4
           //v12 contains pad word from prefetch_clamp call
            prefetch_out \qa, \qb, \store, v12.16b, v12.16b, v12.d[1]
  .if \rem > 0
            b           4f+4
  .else
1:
2:
3:
4:          nop
  .endif
.endif
.endm

/* Fill the convolution window with context data.  The aim here is to load
 * exactly rlf + rrt columns, and in the main loop to read as many columns as
 * will be written.  This is complicated by the need to handle cases when the
 * input starts very close to the left or right (or both) edges of the image,
 * and where these do not fall on 16-byte boundaries.
 *
 * Input:
 *      x1 -- src
 *      x2 -- pitch
 *      x3 -- count
 *      x4 -- inlen
 *      x5 -- r
 *      x6 -- rup
 *      x7 -- rdn
 *      x8 -- rlf
 *      x9 -- buffer (if needed)
 *      x13 = -pitch
 *      x15 = top-row in
 *      x16 = bottom-row in
 * Output:
 *      x1 += rlf + min(count, rrt)
 * Modifies:
 *      x10 -- fill start index in the window
 *      x11 -- fill stop index in the window
 *      x12 -- scratch
 */
.macro prefetch step=1, max_r=25
.set need, ((\max_r + \max_r) * \step + 15) & ~15
  .if \step == 1
            mov         x10, #need - (\max_r * \step)
            sub         x10, x10, x8
  .else
            mov         x10, #need - (\max_r * \step)
            sub         x10, x10, x8, LSL #2
  .endif
            add         x11, x10, x4
            subs        x11, x11, #need
            csel        x11, xzr, x11, hi
            add         x11, x11, #need

            bl          fetch_generic_asm
  .if \step == 1
            dup         v9.8h, v10.h[0]
  .else
            dup         v9.2d, v10.d[0]
  .endif
            tst         x10, #15
            beq         2f
            sub         x12, xzr, x10
            tbz         x10, #3, 1f
            mov         v11.16b, v10.16b
            mov         v10.16b, v9.16b
1:          tbz         x12, #2, 1f
            ext         v11.16b, v10.16b, v11.16b, #4*2
            ext         v10.16b, v9.16b, v10.16b, #4*2
  .if \step == 1
  1:        tbz         x12, #1, 1f
            ext         v11.16b, v10.16b, v11.16b, #2*2
            ext         v10.16b, v9.16b, v10.16b, #2*2
  1:        tbz         x12, #0, 1f
            ext         v11.16b, v10.16b, v11.16b, #1*2
            ext         v10.16b, v9.16b, v10.16b, #1*2
  .endif
1:          sub         x1, x1, x10
            sub         x15, x15, x10
            sub         x16, x16, x10
            bic         x10, x10, #15
            add         x1, x1, x10
            add         x15, x15, x10
            add         x16, x16, x10
2:
  .if \step > 1
            /* it's only in the uchar2 and uchar4 cases where the register file
             * is insufficient (given MAX_R <= 25).
             */
            prefetch_one xx, xx, 192, c=\max_r, step=\step, store=1
            prefetch_one xx, xx, 176, c=\max_r, step=\step, store=1
            prefetch_one xx, xx, 160, c=\max_r, step=\step, store=1
            prefetch_one xx, xx, 144, c=\max_r, step=\step, store=1
            prefetch_one xx, xx, 128, c=\max_r, step=\step, store=1
            prefetch_one xx, xx, 112, c=\max_r, step=\step, store=1
            prefetch_one xx, xx,  96, c=\max_r, step=\step, store=1
            prefetch_one xx, xx,  80, c=\max_r, step=\step, store=1
            prefetch_one xx, xx,  64, c=\max_r, step=\step, store=1
            prefetch_one xx, xx,  48, c=\max_r, step=\step, store=1
  .else
            /* q3 normally contains the coefficient table, but it's not fully
             * used.  In the uchar1, r=25 case the other half of q3 is used for
             * the last two window taps to avoid falling out to memory.
             */
            prefetch_one xx,  v3.d[1], 48, c=\max_r, step=\step, store=-1
  .endif
            prefetch_one v4.16b, v5.16b, 32, c=\max_r, step=\step, store=0
            prefetch_one v6.16b, v7.16b, 16, c=\max_r, step=\step, store=0
            prefetch_one v8.16b, v9.16b,  0, c=\max_r, step=\step, store=0

  .if \step == 1
            add         x10, x8, #\max_r * \step
  .else
            lsl         x10, x8, #2
            add         x10, x10, #\max_r * \step
  .endif
            subs        x4, x4, x10
            csel        x4, xzr, x4, lo
.endm

/* The main loop.
 *
 * Input:
 *      x0 = dst
 *      x1 = src
 *      x2 = pitch
 *      x3 = count
 *      x4 = inlen
 *      x5 = r
 *      x6 = rup
 *      x7 = rdn
 *      x9 = buffer
 *      x13 = -pitch
 *      x15 = top-row in
 *      x16 = bottom-row in
 * Modifies
 *      x8 = fetch code pointer
 */
.macro mainloop core, step=1, max_r=25, labelc="", labelnc=""
            adrp        x8, \labelnc
            add         x8, x8, #:lo12:\labelnc
            sub         x8, x8, x5, LSL #5
            sub         x8, x8, x5, LSL #3
            cmp         x5, x6
            ccmp        x5, x7, #0, eq
            beq         5f

            /* if (r != rup || r != rdn) then the address-clamping table should
             * be used rather than the short-cut version.
             */
            adrp        x8, \labelc
            add         x8, x8, #:lo12:\labelc
            sub         x8, x8, x5, LSL #6
            add         x8, x8, x5, LSL #3
            b           5f
            .align  4
3:          fetch max_r=\max_r, labelc=\labelc, labelnc=\labelnc, reg=x8

            /* For each call to fetch two are made to \core.  It would be
             * preferable to have twice the work done in \core.
             */
            \core
            st1         {v15.8b}, [x0], #8
            \core
            st1         {v15.8b}, [x0], #8

            sub         x3, x3, #16
5:          subs        x4, x4, #16
            bhs         3b
            adds        x4, x4, #16
            bne         1f
  .if \step==1
            dup         v10.8h, v9.h[7]
            dup         v11.8h, v9.h[7]
  .else
            dup         v10.2d, v9.d[1]
            dup         v11.2d, v9.d[1]
  .endif
            b           4f

1:          sub         x1, x1, #16
            sub         x15, x15, #16
            sub         x16, x16, #16
            add         x1, x1, x4
            add         x15, x15, x4
            add         x16, x16, x4
            bl          fetch_generic_asm

  .if \step==1
            dup         v12.8h, v11.h[7]
  .else
            dup         v12.2d, v11.d[1]
  .endif
            sub         x4, xzr, x4
            tbz         x4, #3, 1f
            mov         v10.16b, v11.16b
            mov         v11.16b, v12.16b
1:          tbz         x4, #2, 1f
            ext         v10.16b, v10.16b, v11.16b, #4*2
            ext         v11.16b, v11.16b, v12.16b, #4*2
1:          tbz         x4, #1, 1f
            ext         v10.16b, v10.16b, v11.16b, #2*2
            ext         v11.16b, v11.16b, v12.16b, #2*2
1:          tbz         x4, #0, 4f
            ext         v10.16b, v10.16b, v11.16b, #1*2
            ext         v11.16b, v11.16b, v12.16b, #1*2
4:          cbz         x3, 5f
3:          \core
  .if \step==1
            dup         v11.8h, v11.h[7]
  .else
            dup         v11.2d, v11.d[1]
  .endif
            subs        x3, x3, #8
            blo         4f
            st1         {v15.8b}, [x0], #8
            beq         5f
            b           3b
4:          tbz         x3, #2, 1f
            st1         {v15.s}[0], [x0], #4
            ext         v15.16b, v15.16b, v15.16b, #4*2
1:          tbz         x3, #1, 1f
            st1         {v15.h}[0], [x0], #2
            ext         v15.16b, v15.16b, v15.16b, #2*2
1:          tbz         x3, #0, 5f
            st1         {v15.b}[0], [x0], #1
            ext         v15.16b, v15.16b, v15.16b, #1*2
5:          nop
.endm

.irep r, TUNED_LIST1, 25
ENTRY(convolve1_\r)
            stp         x29,x30, [sp, #-16]!

            prefetch    step=1, max_r=\r

            mainloop    core=hconv1_\r, step=1, max_r=\r, labelc=.Lcnv1_\r, labelnc=.Lcnvnc1_\r

            ldp         x29,x30, [sp], #16
            ret
END(convolve1_\r)
.endr

.irep r, TUNED_LIST4, 25
ENTRY(convolve4_\r)
            sub         x12, sp, #0x200
            bic         x9, x12, #0x3fc
            mov         sp, x9
            stp         x12,x30, [sp, #-16]!

            /* x9 now points to a buffer on the stack whose address has the low
             * 10 bits clear.  This allows easy address calculation in the
             * wrap-around cases.
             */


            prefetch    step=4, max_r=\r

            mainloop    core=hconv4_\r, step=4, max_r=\r, labelc=.Lcnv4_\r, labelnc=.Lcnvnc4_\r

            ldp         x12,x30, [sp]
            add         sp, x12, #0x200
            ret
END(convolve4_\r)
.endr

/* void rsdIntrinsicBlurU1_K(
 *                  void *out,      // x0
 *                  void *in,       // x1
 *                  size_t w,       // x2
 *                  size_t h,       // x3
 *                  size_t p,       // x4
 *                  size_t x,       // x5
 *                  size_t y,       // x6
 *                  size_t count,   // x7
 *                  size_t r,       // [sp]
 *                  uint16_t *tab); // [sp,#8]
 */
ENTRY(rsdIntrinsicBlurU1_K)
            stp         x16,x30, [sp, #-80]!
            stp         x14,x15, [sp, #16]
            stp         x12,x13, [sp, #32]
            stp         x10,x11, [sp, #48]
            stp         x8,x9, [sp, #64]
            sub         x8, sp, #32
            sub         sp, sp, #64
            st1         {v8.1d - v11.1d}, [sp]
            st1         {v12.1d - v15.1d}, [x8]
            mov         x8, x5        // x
            ldr         w5, [sp,#144] // r
            sub         x9, x2, x8
            sub         x10, x3, x6
            mov         x2, x4        // pitch
            mov         x3, x7        // count
            sub         x7, x10, #1
            sub         x9, x9, x3

            ldr         x12, [sp, #152] // tab

            add         x0, x0, x8
            add         x1, x1, x8

            cmp         x6, x5
            csel        x6, x5, x6, hs
            cmp         x7, x5
            csel        x7, x5, x7, hs
            cmp         x8, x5
            csel        x8, x5, x8, hs
            cmp         x9, x5
            csel        x9, x5, x8, hs

            add         x4, x8, x9
            add         x4, x4, x3

            sub         x1, x1, x8

            sub         x13, xzr, x2
            msub        x15, x2, x6, x1
            madd        x16, x2, x7, x1

            ld1         {v0.8h,v1.8h}, [x12], #32
            ld1         {v2.8h,v3.8h}, [x12], #32

            adr         x30, 1f
  .irep r, TUNED_LIST1
            cmp         x5, #\r
            bls         convolve1_\r
  .endr
            b           convolve1_25

1:          ld1         {v8.1d - v11.1d}, [sp], #32
            ld1         {v12.1d - v15.1d}, [sp], #32
            ldp         x8,x9, [sp, #64]
            ldp         x10,x11, [sp, #48]
            ldp         x12,x13, [sp, #32]
            ldp         x14,x15, [sp, #16]
            ldp         x12,x30, [sp], #80
            ret
END(rsdIntrinsicBlurU1_K)

/* void rsdIntrinsicBlurU4_K(
 *                  void *out,      // x0
 *                  void *in,       // x1
 *                  size_t w,       // x2
 *                  size_t h,       // x3
 *                  size_t p,       // x4
 *                  size_t x,       // x5
 *                  size_t y,       // x6
 *                  size_t count,   // x7
 *                  size_t r,       // [sp]
 *                  uint16_t *tab); // [sp,#8]
 */
ENTRY(rsdIntrinsicBlurU4_K)
            stp         x16,x30, [sp, #-80]!
            stp         x14,x15, [sp, #16]
            stp         x12,x13, [sp, #32]
            stp         x10,x11, [sp, #48]
            stp         x8,x9, [sp, #64]
            sub         x8, sp, #32
            sub         sp, sp, #64
            st1         {v8.1d - v11.1d}, [sp]
            st1         {v12.1d - v15.1d}, [x8]
            mov         x8, x5        // x
            ldr         w5, [sp,#144] // r
            sub         x9, x2, x8
            sub         x10, x3, x6
            mov         x2, x4        // pitch
            mov         x3, x7        // count
            sub         x7, x10, #1
            sub         x9, x9, x3

            ldr         x12, [sp, #152]

            add         x0, x0, x8, LSL #2
            add         x1, x1, x8, LSL #2

            cmp         x6, x5
            csel        x6, x5, x6, hs
            cmp         x7, x5
            csel        x7, x5, x7, hs
            cmp         x8, x5
            csel        x8, x5, x8, hs
            cmp         x9, x5
            csel        x9, x5, x9, hs

            lsl         x3, x3, #2
            add         x4, x8, x9
            add         x4, x3, x4, LSL #2

            sub         x1, x1, x8, LSL #2

            sub         x13, xzr, x2
            msub        x15, x2, x6, x1
            madd        x16, x2, x7, x1

            ld1         {v0.8h,v1.8h}, [x12], #32
            ld1         {v2.8h,v3.8h}, [x12], #32

            adr         x30, 1f
  .irep r, TUNED_LIST4
            cmp         x5, #\r
            bls         convolve4_\r
  .endr
            b           convolve4_25

1:          ld1         {v8.1d - v11.1d}, [sp], #32
            ld1         {v12.1d - v15.1d}, [sp], #32
            ldp         x8,x9, [sp, #64]
            ldp         x10,x11, [sp, #48]
            ldp         x12,x13, [sp, #32]
            ldp         x14,x15, [sp, #16]
            ldp         x12,x30, [sp], #80
            ret
END(rsdIntrinsicBlurU4_K)
