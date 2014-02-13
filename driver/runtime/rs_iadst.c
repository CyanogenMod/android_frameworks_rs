#include "rs_iadst.h"
#include "rs_allocation.rsh"

extern void rsIadst4(const rs_allocation input, rs_allocation output, int32_t xoff) {
    int s0, s1, s2, s3, s4, s5, s6, s7;

    int x0 = rsGetElementAt_short(input, xoff);
    int x1 = rsGetElementAt_short(input, xoff + 1);
    int x2 = rsGetElementAt_short(input, xoff + 2);
    int x3 = rsGetElementAt_short(input, xoff + 3);

    if (!(x0 | x1 | x2 | x3)) {
        rsSetElementAt_short(output, 0, xoff);
        rsSetElementAt_short(output, 0, xoff + 1);
        rsSetElementAt_short(output, 0, xoff + 2);
        rsSetElementAt_short(output, 0, xoff + 3);
        return;
    }

    s0 = sinpi_1_9 * x0;
    s1 = sinpi_2_9 * x0;
    s2 = sinpi_3_9 * x1;
    s3 = sinpi_4_9 * x2;
    s4 = sinpi_1_9 * x2;
    s5 = sinpi_2_9 * x3;
    s6 = sinpi_4_9 * x3;
    s7 = x0 - x2 + x3;

    x0 = s0 + s3 + s5;
    x1 = s1 - s4 - s6;
    x2 = sinpi_3_9 * s7;
    x3 = s2;

    s0 = x0 + x3;
    s1 = x1 + x3;
    s2 = x2;
    s3 = x0 + x1 - x3;

    rsSetElementAt_short(output, dct_const_round_shift(s0), xoff);
    rsSetElementAt_short(output, dct_const_round_shift(s1), xoff + 1);
    rsSetElementAt_short(output, dct_const_round_shift(s2), xoff + 2);
    rsSetElementAt_short(output, dct_const_round_shift(s3), xoff + 3);
}

extern void rsIadst8(const rs_allocation input, rs_allocation output, int32_t xoff) {
    int s0, s1, s2, s3, s4, s5, s6, s7;
    int16_t outArr[8];

    int x0 = rsGetElementAt_short(input, xoff + 7);
    int x1 = rsGetElementAt_short(input, xoff);
    int x2 = rsGetElementAt_short(input, xoff + 5);
    int x3 = rsGetElementAt_short(input, xoff + 2);
    int x4 = rsGetElementAt_short(input, xoff + 3);
    int x5 = rsGetElementAt_short(input, xoff + 4);
    int x6 = rsGetElementAt_short(input, xoff + 1);
    int x7 = rsGetElementAt_short(input, xoff + 6);

    if (!(x0 | x1 | x2 | x3 | x4 | x5 | x6 | x7)) {
        rsSetElementAt_short(output, 0, xoff);
        rsSetElementAt_short(output, 0, xoff + 1);
        rsSetElementAt_short(output, 0, xoff + 2);
        rsSetElementAt_short(output, 0, xoff + 3);
        rsSetElementAt_short(output, 0, xoff + 4);
        rsSetElementAt_short(output, 0, xoff + 5);
        rsSetElementAt_short(output, 0, xoff + 6);
        rsSetElementAt_short(output, 0, xoff + 7);
        return;
    }

    // stage 1
    s0 = cospi_2_64  * x0 + cospi_30_64 * x1;
    s1 = cospi_30_64 * x0 - cospi_2_64  * x1;
    s2 = cospi_10_64 * x2 + cospi_22_64 * x3;
    s3 = cospi_22_64 * x2 - cospi_10_64 * x3;
    s4 = cospi_18_64 * x4 + cospi_14_64 * x5;
    s5 = cospi_14_64 * x4 - cospi_18_64 * x5;
    s6 = cospi_26_64 * x6 + cospi_6_64  * x7;
    s7 = cospi_6_64  * x6 - cospi_26_64 * x7;

    x0 = dct_const_round_shift(s0 + s4);
    x1 = dct_const_round_shift(s1 + s5);
    x2 = dct_const_round_shift(s2 + s6);
    x3 = dct_const_round_shift(s3 + s7);
    x4 = dct_const_round_shift(s0 - s4);
    x5 = dct_const_round_shift(s1 - s5);
    x6 = dct_const_round_shift(s2 - s6);
    x7 = dct_const_round_shift(s3 - s7);

    // stage 2
    s0 = x0;
    s1 = x1;
    s2 = x2;
    s3 = x3;
    s4 =  cospi_8_64  * x4 + cospi_24_64 * x5;
    s5 =  cospi_24_64 * x4 - cospi_8_64  * x5;
    s6 = -cospi_24_64 * x6 + cospi_8_64  * x7;
    s7 =  cospi_8_64  * x6 + cospi_24_64 * x7;

    x0 = s0 + s2;
    x1 = s1 + s3;
    x2 = s0 - s2;
    x3 = s1 - s3;
    x4 = dct_const_round_shift(s4 + s6);
    x5 = dct_const_round_shift(s5 + s7);
    x6 = dct_const_round_shift(s4 - s6);
    x7 = dct_const_round_shift(s5 - s7);

    // stage 3
    s2 = cospi_16_64 * (x2 + x3);
    s3 = cospi_16_64 * (x2 - x3);
    s6 = cospi_16_64 * (x6 + x7);
    s7 = cospi_16_64 * (x6 - x7);

    x2 = dct_const_round_shift(s2);
    x3 = dct_const_round_shift(s3);
    x6 = dct_const_round_shift(s6);
    x7 = dct_const_round_shift(s7);

    outArr[0] =  x0;
    outArr[1] = -x4;
    outArr[2] =  x6;
    outArr[3] = -x2;
    outArr[4] =  x3;
    outArr[5] = -x7;
    outArr[6] =  x5;
    outArr[7] = -x1;

    int i;
    for (i = 0; i < 8; ++i) {
        rsSetElementAt_short(output, outArr[i], xoff + i);
    }
}

extern void rsIadst16(const rs_allocation input, rs_allocation output, int32_t xoff) {
    int s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15;
    int16_t outArr[16];
    int i;

    int x0 = rsGetElementAt_short(input, xoff + 15);
    int x1 = rsGetElementAt_short(input, xoff);
    int x2 = rsGetElementAt_short(input, xoff + 13);
    int x3 = rsGetElementAt_short(input, xoff + 2);
    int x4 = rsGetElementAt_short(input, xoff + 11);
    int x5 = rsGetElementAt_short(input, xoff + 4);
    int x6 = rsGetElementAt_short(input, xoff + 9);
    int x7 = rsGetElementAt_short(input, xoff + 6);
    int x8 = rsGetElementAt_short(input, xoff + 7);
    int x9 = rsGetElementAt_short(input, xoff + 8);
    int x10 = rsGetElementAt_short(input, xoff + 5);
    int x11 = rsGetElementAt_short(input, xoff + 10);
    int x12 = rsGetElementAt_short(input, xoff + 3);
    int x13 = rsGetElementAt_short(input, xoff + 12);
    int x14 = rsGetElementAt_short(input, xoff + 1);
    int x15 = rsGetElementAt_short(input, xoff + 14);

    if (!(x0 | x1 | x2 | x3 | x4 | x5 | x6 | x7 | x8
            | x9 | x10 | x11 | x12 | x13 | x14 | x15)) {
        for (i = 0; i < 16; ++i) {
            rsSetElementAt_short(output, 0, xoff + i);
        }
        return;
    }

    // stage 1
    s0 = x0 * cospi_1_64  + x1 * cospi_31_64;
    s1 = x0 * cospi_31_64 - x1 * cospi_1_64;
    s2 = x2 * cospi_5_64  + x3 * cospi_27_64;
    s3 = x2 * cospi_27_64 - x3 * cospi_5_64;
    s4 = x4 * cospi_9_64  + x5 * cospi_23_64;
    s5 = x4 * cospi_23_64 - x5 * cospi_9_64;
    s6 = x6 * cospi_13_64 + x7 * cospi_19_64;
    s7 = x6 * cospi_19_64 - x7 * cospi_13_64;
    s8 = x8 * cospi_17_64 + x9 * cospi_15_64;
    s9 = x8 * cospi_15_64 - x9 * cospi_17_64;
    s10 = x10 * cospi_21_64 + x11 * cospi_11_64;
    s11 = x10 * cospi_11_64 - x11 * cospi_21_64;
    s12 = x12 * cospi_25_64 + x13 * cospi_7_64;
    s13 = x12 * cospi_7_64  - x13 * cospi_25_64;
    s14 = x14 * cospi_29_64 + x15 * cospi_3_64;
    s15 = x14 * cospi_3_64  - x15 * cospi_29_64;

    x0 = dct_const_round_shift(s0 + s8);
    x1 = dct_const_round_shift(s1 + s9);
    x2 = dct_const_round_shift(s2 + s10);
    x3 = dct_const_round_shift(s3 + s11);
    x4 = dct_const_round_shift(s4 + s12);
    x5 = dct_const_round_shift(s5 + s13);
    x6 = dct_const_round_shift(s6 + s14);
    x7 = dct_const_round_shift(s7 + s15);
    x8  = dct_const_round_shift(s0 - s8);
    x9  = dct_const_round_shift(s1 - s9);
    x10 = dct_const_round_shift(s2 - s10);
    x11 = dct_const_round_shift(s3 - s11);
    x12 = dct_const_round_shift(s4 - s12);
    x13 = dct_const_round_shift(s5 - s13);
    x14 = dct_const_round_shift(s6 - s14);
    x15 = dct_const_round_shift(s7 - s15);

    // stage 2
    s0 = x0;
    s1 = x1;
    s2 = x2;
    s3 = x3;
    s4 = x4;
    s5 = x5;
    s6 = x6;
    s7 = x7;
    s8 =    x8 * cospi_4_64   + x9 * cospi_28_64;
    s9 =    x8 * cospi_28_64  - x9 * cospi_4_64;
    s10 =   x10 * cospi_20_64 + x11 * cospi_12_64;
    s11 =   x10 * cospi_12_64 - x11 * cospi_20_64;
    s12 = - x12 * cospi_28_64 + x13 * cospi_4_64;
    s13 =   x12 * cospi_4_64  + x13 * cospi_28_64;
    s14 = - x14 * cospi_12_64 + x15 * cospi_20_64;
    s15 =   x14 * cospi_20_64 + x15 * cospi_12_64;

    x0 = s0 + s4;
    x1 = s1 + s5;
    x2 = s2 + s6;
    x3 = s3 + s7;
    x4 = s0 - s4;
    x5 = s1 - s5;
    x6 = s2 - s6;
    x7 = s3 - s7;
    x8 = dct_const_round_shift(s8 + s12);
    x9 = dct_const_round_shift(s9 + s13);
    x10 = dct_const_round_shift(s10 + s14);
    x11 = dct_const_round_shift(s11 + s15);
    x12 = dct_const_round_shift(s8 - s12);
    x13 = dct_const_round_shift(s9 - s13);
    x14 = dct_const_round_shift(s10 - s14);
    x15 = dct_const_round_shift(s11 - s15);

    // stage 3
    s0 = x0;
    s1 = x1;
    s2 = x2;
    s3 = x3;
    s4 = x4 * cospi_8_64  + x5 * cospi_24_64;
    s5 = x4 * cospi_24_64 - x5 * cospi_8_64;
    s6 = - x6 * cospi_24_64 + x7 * cospi_8_64;
    s7 =   x6 * cospi_8_64  + x7 * cospi_24_64;
    s8 = x8;
    s9 = x9;
    s10 = x10;
    s11 = x11;
    s12 = x12 * cospi_8_64  + x13 * cospi_24_64;
    s13 = x12 * cospi_24_64 - x13 * cospi_8_64;
    s14 = - x14 * cospi_24_64 + x15 * cospi_8_64;
    s15 =   x14 * cospi_8_64  + x15 * cospi_24_64;

    x0 = s0 + s2;
    x1 = s1 + s3;
    x2 = s0 - s2;
    x3 = s1 - s3;
    x4 = dct_const_round_shift(s4 + s6);
    x5 = dct_const_round_shift(s5 + s7);
    x6 = dct_const_round_shift(s4 - s6);
    x7 = dct_const_round_shift(s5 - s7);
    x8 = s8 + s10;
    x9 = s9 + s11;
    x10 = s8 - s10;
    x11 = s9 - s11;
    x12 = dct_const_round_shift(s12 + s14);
    x13 = dct_const_round_shift(s13 + s15);
    x14 = dct_const_round_shift(s12 - s14);
    x15 = dct_const_round_shift(s13 - s15);

    // stage 4
    s2 = (- cospi_16_64) * (x2 + x3);
    s3 = cospi_16_64 * (x2 - x3);
    s6 = cospi_16_64 * (x6 + x7);
    s7 = cospi_16_64 * (- x6 + x7);
    s10 = cospi_16_64 * (x10 + x11);
    s11 = cospi_16_64 * (- x10 + x11);
    s14 = (- cospi_16_64) * (x14 + x15);
    s15 = cospi_16_64 * (x14 - x15);

    x2 = dct_const_round_shift(s2);
    x3 = dct_const_round_shift(s3);
    x6 = dct_const_round_shift(s6);
    x7 = dct_const_round_shift(s7);
    x10 = dct_const_round_shift(s10);
    x11 = dct_const_round_shift(s11);
    x14 = dct_const_round_shift(s14);
    x15 = dct_const_round_shift(s15);

    outArr[0] =  x0;
    outArr[1] = -x8;
    outArr[2] =  x12;
    outArr[3] = -x4;
    outArr[4] =  x6;
    outArr[5] =  x14;
    outArr[6] =  x10;
    outArr[7] =  x2;
    outArr[8] =  x3;
    outArr[9] =  x11;
    outArr[10] =  x15;
    outArr[11] =  x7;
    outArr[12] =  x5;
    outArr[13] = -x13;
    outArr[14] =  x9;
    outArr[15] = -x1;

    for (i = 0; i < 16; ++i) {
        rsSetElementAt_short(output, outArr[i], xoff + i);
    }
}
