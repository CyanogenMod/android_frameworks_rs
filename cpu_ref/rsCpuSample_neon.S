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

/*
        r0 = base pointer
        r1 = image stride
        r2 = w
        r3 = h
        sp = float u
        sp = float v
*/

ENTRY(rsdCpuLinearClamp2D_RGBA_k2)
    push            {r4-r8, r10, r11, lr}
    vpush           {q4-q7}

    // Load uv
    ldr r4, [sp, #32+64]
    ldr r5, [sp, #32+64+4]
    vmov d18, r4, r5                                // d18 = float  u, v


//    float pixelU = (u * w) - 0.5f;
//    float pixelV = (v * h) - 0.5f;
    vmov d16, r2, r3                                // d16 = int  w, h
    vcvt.f32.s32 d17, d16                           // d17 = float w, h
    vmul.f32 d20, d18, d17                          // d20 = pixelUV (uv * wh)

    vld1.f32 d19, =0x3F000000  // 0.5
    vsub.f32 d20, d20, d19                          // d20 = pixelUV (uv * wh) - 0.5f

//    int iu = pixelU;
//    int iv = pixelV;
    vcvt.s32.f32 d21, d20                           // d21 = iPixelUV


    //float fracU = pixelU - iu;
    //float fracV = pixelV - iv;
    vcvt.s32.f32 d19, d20                           //
    vcvt.f32.s32 d19, d19                           // d19 = (float)iuv
    vsub.f32 d0, d20, d19                           // d0 = fract = pixelUV - iuv


    //float oneMinusFracU = 1.0f - fracU;
    //float oneMinusFracV = 1.0f - fracV;
    vld1.f32 d22, =0x3F800000  // 0.5
    vsub.f32 d1, d22, d0                            // d1 = oneMinusFrac


    //float weightsX1 = oneMinusFracU * oneMinusFracV;
    //float weightsY1 = fracU * oneMinusFracV;
    //float weightsX2 = fracV * oneMinusFracU;
    //float weightsY2 = fracU * fracV;
    vmul.f32 d2, d1, d1[1]                          // d2 = 1mu * 1mv , 1mv * 1mv
    vmul.f32 d3, d0, d1[1]                          // d3 = u * 1mv , v * 1mv
    vmul.f32 d4, d1, d0[1]                          // d4 = v * 1mu , v * 1mv
    vmul.f32 d5, d0, d0[1]                          // d5 = u * v,  v * v

    //int nextX = rsMax(0, rsMin(iu + 1, w - 1));
    //int nextY = rsMax(0, rsMin(iv + 1, h - 1));
    //int locationX = rsMax(0, rsMin(iu, w - 1));
    //int locationY = rsMax(0, rsMin(iv, h - 1));
    vmov.u32 d6, #1
    vmov.u32 d8, #0
    vsub.s32 d16, d16, d6                           // d16 = h -1, w -1
    vadd.s32 d7, d6, d21                            // d7 = iuv + 1

    vmin.s32 d7, d7, d16
    vmin.s32 d21, d21, d16
    vmax.s32 d7, d7, d8                             // d7 = next
    vmax.s32 d21, d21, d8                           // d21 = location

    mov r2, #4
    vmov d6, r2, r1                                 // d6 = 4, stride
    vmul.s32 d7, d6                                 // d7 = nextX*4, nextY * stride
    vmul.s32 d21, d6                                // d21 = locationX*4, locationY * stride

    //uchar4 *p0c = (uchar4*)&p[(locationY * stride) + (locationX * 4)];
    //uchar4 *p1c = (uchar4*)&p[(locationY * stride) + (nextX * 4)];
    //uchar4 *p2c = (uchar4*)&p[(nextY * stride) + (locationX * 4)];
    //uchar4 *p3c = (uchar4*)&p[(nextY * stride) + (nextX * 4)];
    vmov r2, r3, d7                                 // r2 = nextX*4,  r3 = nextY *stride
    vmov r4, r5, d21                                // r4 = locX*4,  r5 = locY*Stride
    add r3, r3, r0                                  // r3 = p + nextY*stride
    add r5, r5, r0                                  // r5 = p + locY*stride

    //float4 p0 = convert_float4(*p0c);
    //float4 p1 = convert_float4(*p1c);
    add r1, r5, r4                                  // *p0c
    ldr r0, [r1]
    add r1, r5, r2                                  // *p1c
    ldr r1, [r1]
    vmov d0, r0, r1                                 // d0 = p0, p1

    //float4 p2 = convert_float4(*p2c);
    //float4 p3 = convert_float4(*p3c);
    add r1, r3, r4                                  // *p2c
    ldr r0, [r1]
    add r1, r3, r2                                  // *p3c
    ldr r1, [r1]
    vmov d1, r0, r1                                 // d1 = p0, p1

    //return (p0 * weightsX1 + p1 * weightsY1 + p2 * weightsX2 + p3 * weightsY2) * 0.003921569f;
    vmovl.u8 q3, d0
    vmovl.u8 q4, d1
    vmovl.u16 q3, d6
    vmovl.u16 q4, d7
    vmovl.u16 q5, d8
    vmovl.u16 q6, d9
    vcvt.f32.u32 q3, q3
    vcvt.f32.u32 q4, q4
    vcvt.f32.u32 q5, q5
    vcvt.f32.u32 q6, q6

    //vmul.f32 q3, q3, d2[0]
    //vmla.f32 q3, q4, d3[0]
    //vmla.f32 q3, q5, d4[0]
    //vmla.f32 q3, q6, d5[0]

    vld1.f32 d0, =0x3B808081  // 1.f / 255.f
    vmul.f32 q3, q3, d0[0]

    vmov r0, r1, d6
    vmov r2, r3, d7

    mov r3, #0x3F800000

    /* We're done, bye! */
    vpop            {q4-q7}
    pop             {r4-r8, r10, r11, lr}
    bx              lr
END(rsdCpuLinearClamp2D_RGBA_k2)









/*
        r0 = base pointer
        r1 = image stride
        r2 = iu
        r3 = iv
        sp = w
        sp = h
*/

ENTRY(rsdCpuLinearClamp2D_RGBA_k)
    push            {r4-r8, r10, r11, lr}
    vpush           {q4-q7}

    vmov d2, r2, r3

    add r4, sp, #32+64
    vld1.32 d3, [r4]!
    vld1.32 {q0}, [r4]!


    mov r2, #4
    vmov d6, r2, r1                                 // d6 = 4, stride
    vmul.s32 d30, d6                                // d30 = nextX*4, nextY * stride
    vmul.s32 d31, d6                                // d31 = locationX*4, locationY * stride

    //uchar4 *p0c = (uchar4*)&p[(locationY * stride) + (locationX * 4)];
    //uchar4 *p1c = (uchar4*)&p[(locationY * stride) + (nextX * 4)];
    //uchar4 *p2c = (uchar4*)&p[(nextY * stride) + (locationX * 4)];
    //uchar4 *p3c = (uchar4*)&p[(nextY * stride) + (nextX * 4)];
    vmov r2, r3, d30                                // r2 = nextX*4,  r3 = nextY *stride
    vmov r4, r5, d31                                // r4 = locX*4,  r5 = locY*Stride
    add r3, r3, r0                                  // r3 = p + nextY*stride
    add r5, r5, r0                                  // r5 = p + locY*stride

    //float4 p0 = convert_float4(*p0c);
    //float4 p1 = convert_float4(*p1c);
    add r1, r5, r4                                  // *p0c
    ldr r0, [r1]
    add r1, r5, r2                                  // *p1c
    ldr r1, [r1]
    vmov d30, r0, r1                                 // d0 = p0, p1

    //float4 p2 = convert_float4(*p2c);
    //float4 p3 = convert_float4(*p3c);
    add r1, r3, r4                                  // *p2c
    ldr r0, [r1]
    add r1, r3, r2                                  // *p3c
    ldr r1, [r1]
    vmov d31, r0, r1                                 // d1 = p0, p1

    //return (p0 * weightsX1 + p1 * weightsY1 + p2 * weightsX2 + p3 * weightsY2) * 0.003921569f;
    vmovl.u8 q2, d30
    vmovl.u8 q3, d31
    vmovl.u16 q8, d4
    vmovl.u16 q9, d5
    vmovl.u16 q10, d6
    vmovl.u16 q11, d7
    vcvt.f32.u32 q8, q8, #8
    vcvt.f32.u32 q9, q9, #8
    vcvt.f32.u32 q10, q10, #8
    vcvt.f32.u32 q11, q11, #8

    vmul.f32 q3, q8, d0[0]
    vmla.f32 q3, q9, d0[1]
    vmla.f32 q3, q10, d1[0]
    vmla.f32 q3, q11, d1[1]

///    vld1.f32 d0, =0x3B808081  // 1.f / 255.f
//    vmul.f32 q3, q3, d0[0]

    vmov r0, r1, d6
    vmov r2, r3, d7

    mov r3, #0x3F800000

    /* We're done, bye! */
    vpop            {q4-q7}
    pop             {r4-r8, r10, r11, lr}
    bx              lr
END(rsdCpuLinearClamp2D_RGBA_k)



/*
        r0 = uint8_t *ptr
        r1 = image stride
        r2,r3 = iPixel
        sp0,1 = next
        q0 = weights
*/

ENTRY(rsdCpuGetSample2D_RGBA_k)
    push            {r4-r8, lr}

    ldr r4, [sp, #24]                           // next.x
    ldr r5, [sp, #24+4]                         // next.y

    mul r3, r3, r1                                  // iPixel.y * stride
    mul r5, r5, r1                                  // next.y * stride

    add r2, r0, r2, LSL #2
    add r4, r0, r4, LSL #2

    ldr r0, [r2, r3]                                // r0 = p[(locationY * stride) + (locationX * 4)]
    ldr r1, [r4, r3]                                // r1 = p[(locationY * stride) + (nextX * 4)]
    ldr r2, [r2, r5]                                // r2 = p[(nextY * stride) + (locationX * 4)]
    ldr r3, [r4, r5]                                // r3 = p[(nextY * stride) + (nextX * 4)]

    vmov d30, r0, r1                                 // d30 = p0, p1
    vmov d31, r2, r3                                 // d31 = p2, p3

    vcvt.u32.f32 q0, q0, #8
    vmovn.u32 d0, q0

    //return (p0 * weightsX1 + p1 * weightsY1 + p2 * weightsX2 + p3 * weightsY2) * 0.003921569f;
    vmovl.u8 q2, d30
    vmovl.u8 q3, d31

    vmull.u16 q8, d4, d0[0]
    vmlal.u16 q8, d5, d0[1]
    vmlal.u16 q8, d6, d0[2]
    vmlal.u16 q8, d7, d0[3]

    vcvt.f32.u32 q3, q8, #8

    ldr r1, =0x3B808081  // 1.f / 255.f
    vmov.32 d0[0], r1
    vmul.f32 q0, q3, d0[0]

    /* We're done, bye! */
    pop             {r4-r8, lr}
    bx              lr
END(rsdCpuGetSample2D_RGBA_k)

