#include "rs_idct.h"
#include "rs_allocation.rsh"

static void idct4_1d(const int16_t *input, int16_t *output) {
    int16_t step[4];
    int temp1, temp2;
    // stage 1
    temp1 = (input[0] + input[2]) * cospi_16_64;
    temp2 = (input[0] - input[2]) * cospi_16_64;
    step[0] = dct_const_round_shift(temp1);
    step[1] = dct_const_round_shift(temp2);
    temp1 = input[1] * cospi_24_64 - input[3] * cospi_8_64;
    temp2 = input[1] * cospi_8_64 + input[3] * cospi_24_64;
    step[2] = dct_const_round_shift(temp1);
    step[3] = dct_const_round_shift(temp2);

    // stage 2
    output[0] = step[0] + step[3];
    output[1] = step[1] + step[2];
    output[2] = step[1] - step[2];
    output[3] = step[0] - step[3];
}

static void idct4x4_1(const rs_allocation input, rs_allocation dest, int xoff, int yoff) {
    int i, j;
    int a1;
    int16_t out = dct_const_round_shift(rsGetElementAt_short(input, xoff, yoff)
                          * cospi_16_64);
    out = dct_const_round_shift(out * cospi_16_64);
    a1 = ROUND_POWER_OF_TWO(out, 4);

    uint8_t result;
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            result = clip_pixel(rsGetElementAt_uchar(dest, j + xoff, i + yoff) + a1);
            rsSetElementAt_uchar(dest, result, j + xoff, i + yoff);
        }
    }
}

static void idct4x4_16(const rs_allocation input, rs_allocation dest, int xoff, int yoff) {
    int16_t out[4 * 4];
    int16_t *outptr = out;
    int i, j;
    int16_t temp_in[4], temp_out[4];

    int16_t in[4 * 4];
    int16_t *inptr = in;
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            in[j + i * 4] = rsGetElementAt_short(input, j + xoff, i + yoff);
        }
    }

    // Rows
    for (i = 0; i < 4; ++i) {
        idct4_1d(inptr, outptr);
        inptr += 4;
        outptr += 4;
    }

    // Columns
    uint8_t result;
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            temp_in[j] = out[j * 4 + i];
        }
        idct4_1d(temp_in, temp_out);
        for (j = 0; j < 4; ++j) {
            result = clip_pixel(ROUND_POWER_OF_TWO(temp_out[j], 4)
                             + rsGetElementAt_uchar(dest, i + xoff, j + yoff));
            rsSetElementAt_uchar(dest, result, i + xoff, j + yoff);
        }
    }
}

static void idct8_1d(const int16_t *input, int16_t *output) {
    int16_t step1[8], step2[8];
    int temp1, temp2;
    // stage 1
    step1[0] = input[0];
    step1[2] = input[4];
    step1[1] = input[2];
    step1[3] = input[6];
    temp1 = input[1] * cospi_28_64 - input[7] * cospi_4_64;
    temp2 = input[1] * cospi_4_64 + input[7] * cospi_28_64;
    step1[4] = dct_const_round_shift(temp1);
    step1[7] = dct_const_round_shift(temp2);
    temp1 = input[5] * cospi_12_64 - input[3] * cospi_20_64;
    temp2 = input[5] * cospi_20_64 + input[3] * cospi_12_64;
    step1[5] = dct_const_round_shift(temp1);
    step1[6] = dct_const_round_shift(temp2);

    // stage 2 & stage 3 - even half
    idct4_1d(step1, step1);

    // stage 2 - odd half
    step2[4] = step1[4] + step1[5];
    step2[5] = step1[4] - step1[5];
    step2[6] = -step1[6] + step1[7];
    step2[7] = step1[6] + step1[7];

    // stage 3 -odd half
    step1[4] = step2[4];
    temp1 = (step2[6] - step2[5]) * cospi_16_64;
    temp2 = (step2[5] + step2[6]) * cospi_16_64;
    step1[5] = dct_const_round_shift(temp1);
    step1[6] = dct_const_round_shift(temp2);
    step1[7] = step2[7];

    // stage 4
    output[0] = step1[0] + step1[7];
    output[1] = step1[1] + step1[6];
    output[2] = step1[2] + step1[5];
    output[3] = step1[3] + step1[4];
    output[4] = step1[3] - step1[4];
    output[5] = step1[2] - step1[5];
    output[6] = step1[1] - step1[6];
    output[7] = step1[0] - step1[7];
}

static void idct8x8_1(const rs_allocation input, rs_allocation dest, int xoff, int yoff) {
    int i, j;
    int a1;
    int16_t out = dct_const_round_shift(rsGetElementAt_short(input, xoff, yoff)
                          * cospi_16_64);
    out = dct_const_round_shift(out * cospi_16_64);
    a1 = ROUND_POWER_OF_TWO(out, 5);

    uint8_t result;
    for (i = 0; i < 8; ++i) {
        for (j = 0; j < 8; ++j) {
            result = clip_pixel(rsGetElementAt_uchar(dest, j + xoff, i + yoff) + a1);
            rsSetElementAt_uchar(dest, result, j + xoff, i + yoff);
        }
    }
}

static void idct8x8_10(const rs_allocation input, rs_allocation dest, int xoff, int yoff) {
    int16_t out[8 * 8] = { 0 };
    int16_t *outptr = out;
    int i, j;
    int16_t temp_in[8], temp_out[8];

    int16_t in[8 * 8];
    int16_t *inptr = in;
    for (i = 0; i < 8; ++i) {
        for (j = 0; j < 8; ++j) {
            in[j + i * 8] = rsGetElementAt_short(input, j + xoff, i + yoff);
        }
    }

    // First transform rows
    // only first 4 row has non-zero coefs
    for (i = 0; i < 4; ++i) {
        idct8_1d(inptr, outptr);
        inptr += 8;
        outptr += 8;
    }

    // Then transform columns
    uint8_t result;
    for (i = 0; i < 8; ++i) {
        for (j = 0; j < 8; ++j)
            temp_in[j] = out[j * 8 + i];
        idct8_1d(temp_in, temp_out);
        for (j = 0; j < 8; ++j) {
            result = clip_pixel(ROUND_POWER_OF_TWO(temp_out[j], 5)
                             + rsGetElementAt_uchar(dest, i + xoff, j + yoff));
            rsSetElementAt_uchar(dest, result, i + xoff, j + yoff);
        }
    }
}

static void idct8x8_64(const rs_allocation input, rs_allocation dest, int xoff, int yoff) {
    int16_t out[8 * 8];
    int16_t *outptr = out;
    int i, j;
    int16_t temp_in[8], temp_out[8];

    int16_t in[8 * 8];
    int16_t *inptr = in;
    for (i = 0; i < 8; ++i) {
        for (j = 0; j < 8; ++j) {
            in[j + i * 8] = rsGetElementAt_short(input, j + xoff, i + yoff);
        }
    }

    // First transform rows
    for (i = 0; i < 8; ++i) {
        idct8_1d(inptr, outptr);
        inptr += 8;
        outptr += 8;
    }

    // Then transform columns
    uint8_t result;
    for (i = 0; i < 8; ++i) {
        for (j = 0; j < 8; ++j)
            temp_in[j] = out[j * 8 + i];
        idct8_1d(temp_in, temp_out);
        for (j = 0; j < 8; ++j) {
            result = clip_pixel(ROUND_POWER_OF_TWO(temp_out[j], 5)
                             + rsGetElementAt_uchar(dest, i + xoff, j + yoff));
            rsSetElementAt_uchar(dest, result, i + xoff, j + yoff);
        }
    }
}

static void idct16_1d(const int16_t *input, int16_t *output) {
    int16_t step1[16], step2[16];
    int temp1, temp2;

    // stage 1
    step1[0] = input[0/2];
    step1[1] = input[16/2];
    step1[2] = input[8/2];
    step1[3] = input[24/2];
    step1[4] = input[4/2];
    step1[5] = input[20/2];
    step1[6] = input[12/2];
    step1[7] = input[28/2];
    step1[8] = input[2/2];
    step1[9] = input[18/2];
    step1[10] = input[10/2];
    step1[11] = input[26/2];
    step1[12] = input[6/2];
    step1[13] = input[22/2];
    step1[14] = input[14/2];
    step1[15] = input[30/2];

    // stage 2
    step2[0] = step1[0];
    step2[1] = step1[1];
    step2[2] = step1[2];
    step2[3] = step1[3];
    step2[4] = step1[4];
    step2[5] = step1[5];
    step2[6] = step1[6];
    step2[7] = step1[7];

    temp1 = step1[8] * cospi_30_64 - step1[15] * cospi_2_64;
    temp2 = step1[8] * cospi_2_64 + step1[15] * cospi_30_64;
    step2[8] = dct_const_round_shift(temp1);
    step2[15] = dct_const_round_shift(temp2);

    temp1 = step1[9] * cospi_14_64 - step1[14] * cospi_18_64;
    temp2 = step1[9] * cospi_18_64 + step1[14] * cospi_14_64;
    step2[9] = dct_const_round_shift(temp1);
    step2[14] = dct_const_round_shift(temp2);

    temp1 = step1[10] * cospi_22_64 - step1[13] * cospi_10_64;
    temp2 = step1[10] * cospi_10_64 + step1[13] * cospi_22_64;
    step2[10] = dct_const_round_shift(temp1);
    step2[13] = dct_const_round_shift(temp2);

    temp1 = step1[11] * cospi_6_64 - step1[12] * cospi_26_64;
    temp2 = step1[11] * cospi_26_64 + step1[12] * cospi_6_64;
    step2[11] = dct_const_round_shift(temp1);
    step2[12] = dct_const_round_shift(temp2);

    // stage 3
    step1[0] = step2[0];
    step1[1] = step2[1];
    step1[2] = step2[2];
    step1[3] = step2[3];

    temp1 = step2[4] * cospi_28_64 - step2[7] * cospi_4_64;
    temp2 = step2[4] * cospi_4_64 + step2[7] * cospi_28_64;
    step1[4] = dct_const_round_shift(temp1);
    step1[7] = dct_const_round_shift(temp2);
    temp1 = step2[5] * cospi_12_64 - step2[6] * cospi_20_64;
    temp2 = step2[5] * cospi_20_64 + step2[6] * cospi_12_64;
    step1[5] = dct_const_round_shift(temp1);
    step1[6] = dct_const_round_shift(temp2);

    step1[8] = step2[8] + step2[9];
    step1[9] = step2[8] - step2[9];
    step1[10] = -step2[10] + step2[11];
    step1[11] = step2[10] + step2[11];
    step1[12] = step2[12] + step2[13];
    step1[13] = step2[12] - step2[13];
    step1[14] = -step2[14] + step2[15];
    step1[15] = step2[14] + step2[15];

    // stage 4
    temp1 = (step1[0] + step1[1]) * cospi_16_64;
    temp2 = (step1[0] - step1[1]) * cospi_16_64;
    step2[0] = dct_const_round_shift(temp1);
    step2[1] = dct_const_round_shift(temp2);
    temp1 = step1[2] * cospi_24_64 - step1[3] * cospi_8_64;
    temp2 = step1[2] * cospi_8_64 + step1[3] * cospi_24_64;
    step2[2] = dct_const_round_shift(temp1);
    step2[3] = dct_const_round_shift(temp2);
    step2[4] = step1[4] + step1[5];
    step2[5] = step1[4] - step1[5];
    step2[6] = -step1[6] + step1[7];
    step2[7] = step1[6] + step1[7];

    step2[8] = step1[8];
    step2[15] = step1[15];
    temp1 = -step1[9] * cospi_8_64 + step1[14] * cospi_24_64;
    temp2 = step1[9] * cospi_24_64 + step1[14] * cospi_8_64;
    step2[9] = dct_const_round_shift(temp1);
    step2[14] = dct_const_round_shift(temp2);
    temp1 = -step1[10] * cospi_24_64 - step1[13] * cospi_8_64;
    temp2 = -step1[10] * cospi_8_64 + step1[13] * cospi_24_64;
    step2[10] = dct_const_round_shift(temp1);
    step2[13] = dct_const_round_shift(temp2);
    step2[11] = step1[11];
    step2[12] = step1[12];

    // stage 5
    step1[0] = step2[0] + step2[3];
    step1[1] = step2[1] + step2[2];
    step1[2] = step2[1] - step2[2];
    step1[3] = step2[0] - step2[3];
    step1[4] = step2[4];
    temp1 = (step2[6] - step2[5]) * cospi_16_64;
    temp2 = (step2[5] + step2[6]) * cospi_16_64;
    step1[5] = dct_const_round_shift(temp1);
    step1[6] = dct_const_round_shift(temp2);
    step1[7] = step2[7];

    step1[8] = step2[8] + step2[11];
    step1[9] = step2[9] + step2[10];
    step1[10] = step2[9] - step2[10];
    step1[11] = step2[8] - step2[11];
    step1[12] = -step2[12] + step2[15];
    step1[13] = -step2[13] + step2[14];
    step1[14] = step2[13] + step2[14];
    step1[15] = step2[12] + step2[15];

    // stage 6
    step2[0] = step1[0] + step1[7];
    step2[1] = step1[1] + step1[6];
    step2[2] = step1[2] + step1[5];
    step2[3] = step1[3] + step1[4];
    step2[4] = step1[3] - step1[4];
    step2[5] = step1[2] - step1[5];
    step2[6] = step1[1] - step1[6];
    step2[7] = step1[0] - step1[7];
    step2[8] = step1[8];
    step2[9] = step1[9];
    temp1 = (-step1[10] + step1[13]) * cospi_16_64;
    temp2 = (step1[10] + step1[13]) * cospi_16_64;
    step2[10] = dct_const_round_shift(temp1);
    step2[13] = dct_const_round_shift(temp2);
    temp1 = (-step1[11] + step1[12]) * cospi_16_64;
    temp2 = (step1[11] + step1[12]) * cospi_16_64;
    step2[11] = dct_const_round_shift(temp1);
    step2[12] = dct_const_round_shift(temp2);
    step2[14] = step1[14];
    step2[15] = step1[15];

    // stage 7
    output[0] = step2[0] + step2[15];
    output[1] = step2[1] + step2[14];
    output[2] = step2[2] + step2[13];
    output[3] = step2[3] + step2[12];
    output[4] = step2[4] + step2[11];
    output[5] = step2[5] + step2[10];
    output[6] = step2[6] + step2[9];
    output[7] = step2[7] + step2[8];
    output[8] = step2[7] - step2[8];
    output[9] = step2[6] - step2[9];
    output[10] = step2[5] - step2[10];
    output[11] = step2[4] - step2[11];
    output[12] = step2[3] - step2[12];
    output[13] = step2[2] - step2[13];
    output[14] = step2[1] - step2[14];
    output[15] = step2[0] - step2[15];
}

static void idct16x16_1(const rs_allocation input, rs_allocation dest, int xoff, int yoff) {
    int i, j;
    int a1;
    int16_t out = dct_const_round_shift(rsGetElementAt_short(input, xoff, yoff)
                          * cospi_16_64);
    out = dct_const_round_shift(out * cospi_16_64);
    a1 = ROUND_POWER_OF_TWO(out, 6);

    uint8_t result;
    for (i = 0; i < 16; ++i) {
        for (j = 0; j < 16; ++j) {
            result = clip_pixel(rsGetElementAt_uchar(dest, j + xoff, i + yoff) + a1);
            rsSetElementAt_uchar(dest, result, j + xoff, i + yoff);
        }
    }
}

static void idct16x16_10(const rs_allocation input, rs_allocation dest, int xoff, int yoff) {
    int16_t out[16 * 16] = { 0 };
    int16_t *outptr = out;
    int i, j;
    int16_t temp_in[16], temp_out[16];

    int16_t in[16 * 16];
    int16_t *inptr = in;
    for (i = 0; i < 16; ++i) {
        for (j = 0; j < 16; ++j) {
            in[j + i * 16] = rsGetElementAt_short(input, j + xoff, i + yoff);
        }
    }

    // First transform rows. Since all non-zero dct coefficients are in
    // upper-left 4x4 area, we only need to calculate first 4 rows here.
    for (i = 0; i < 4; ++i) {
        idct16_1d(inptr, outptr);
        inptr += 16;
        outptr += 16;
    }

    // Then transform columns
    uint8_t result;
    for (i = 0; i < 16; ++i) {
        for (j = 0; j < 16; ++j)
            temp_in[j] = out[j * 16 + i];
        idct16_1d(temp_in, temp_out);
        for (j = 0; j < 16; ++j) {
            result = clip_pixel(ROUND_POWER_OF_TWO(temp_out[j], 6)
                             + rsGetElementAt_uchar(dest, i + xoff,j + yoff));
            rsSetElementAt_uchar(dest, result, i + xoff, j + yoff);
        }
    }
}

static void idct16x16_256(const rs_allocation input, rs_allocation dest, int xoff, int yoff) {
    int16_t out[16 * 16];
    int16_t *outptr = out;
    int i, j;
    int16_t temp_in[16], temp_out[16];

    int16_t in[16 * 16];
    int16_t *inptr = in;
    for (i = 0; i < 16; ++i) {
        for (j = 0; j < 16; ++j) {
            in[j + i * 16] = rsGetElementAt_short(input, j + xoff, i + yoff);
        }
    }

    // First transform rows
    for (i = 0; i < 16; ++i) {
        idct16_1d(inptr, outptr);
        inptr += 16;
        outptr += 16;
    }

    // Then transform columns
    uint8_t result;
    for (i = 0; i < 16; ++i) {
        for (j = 0; j < 16; ++j)
            temp_in[j] = out[j * 16 + i];
        idct16_1d(temp_in, temp_out);
        for (j = 0; j < 16; ++j) {
            result = clip_pixel(ROUND_POWER_OF_TWO(temp_out[j], 6)
                             + rsGetElementAt_uchar(dest, i + xoff, j + yoff));
            rsSetElementAt_uchar(dest, result, i + xoff, j + yoff);
        }
    }
}

static void idct32_1d(const int16_t *input, int16_t *output) {
    int16_t step1[32], step2[32];
    int temp1, temp2;

    // stage 1
    step1[0] = input[0];
    step1[1] = input[16];
    step1[2] = input[8];
    step1[3] = input[24];
    step1[4] = input[4];
    step1[5] = input[20];
    step1[6] = input[12];
    step1[7] = input[28];
    step1[8] = input[2];
    step1[9] = input[18];
    step1[10] = input[10];
    step1[11] = input[26];
    step1[12] = input[6];
    step1[13] = input[22];
    step1[14] = input[14];
    step1[15] = input[30];

    temp1 = input[1] * cospi_31_64 - input[31] * cospi_1_64;
    temp2 = input[1] * cospi_1_64 + input[31] * cospi_31_64;
    step1[16] = dct_const_round_shift(temp1);
    step1[31] = dct_const_round_shift(temp2);

    temp1 = input[17] * cospi_15_64 - input[15] * cospi_17_64;
    temp2 = input[17] * cospi_17_64 + input[15] * cospi_15_64;
    step1[17] = dct_const_round_shift(temp1);
    step1[30] = dct_const_round_shift(temp2);

    temp1 = input[9] * cospi_23_64 - input[23] * cospi_9_64;
    temp2 = input[9] * cospi_9_64 + input[23] * cospi_23_64;
    step1[18] = dct_const_round_shift(temp1);
    step1[29] = dct_const_round_shift(temp2);

    temp1 = input[25] * cospi_7_64 - input[7] * cospi_25_64;
    temp2 = input[25] * cospi_25_64 + input[7] * cospi_7_64;
    step1[19] = dct_const_round_shift(temp1);
    step1[28] = dct_const_round_shift(temp2);

    temp1 = input[5] * cospi_27_64 - input[27] * cospi_5_64;
    temp2 = input[5] * cospi_5_64 + input[27] * cospi_27_64;
    step1[20] = dct_const_round_shift(temp1);
    step1[27] = dct_const_round_shift(temp2);

    temp1 = input[21] * cospi_11_64 - input[11] * cospi_21_64;
    temp2 = input[21] * cospi_21_64 + input[11] * cospi_11_64;
    step1[21] = dct_const_round_shift(temp1);
    step1[26] = dct_const_round_shift(temp2);

    temp1 = input[13] * cospi_19_64 - input[19] * cospi_13_64;
    temp2 = input[13] * cospi_13_64 + input[19] * cospi_19_64;
    step1[22] = dct_const_round_shift(temp1);
    step1[25] = dct_const_round_shift(temp2);

    temp1 = input[29] * cospi_3_64 - input[3] * cospi_29_64;
    temp2 = input[29] * cospi_29_64 + input[3] * cospi_3_64;
    step1[23] = dct_const_round_shift(temp1);
    step1[24] = dct_const_round_shift(temp2);

    // stage 2
    step2[0] = step1[0];
    step2[1] = step1[1];
    step2[2] = step1[2];
    step2[3] = step1[3];
    step2[4] = step1[4];
    step2[5] = step1[5];
    step2[6] = step1[6];
    step2[7] = step1[7];

    temp1 = step1[8] * cospi_30_64 - step1[15] * cospi_2_64;
    temp2 = step1[8] * cospi_2_64 + step1[15] * cospi_30_64;
    step2[8] = dct_const_round_shift(temp1);
    step2[15] = dct_const_round_shift(temp2);

    temp1 = step1[9] * cospi_14_64 - step1[14] * cospi_18_64;
    temp2 = step1[9] * cospi_18_64 + step1[14] * cospi_14_64;
    step2[9] = dct_const_round_shift(temp1);
    step2[14] = dct_const_round_shift(temp2);

    temp1 = step1[10] * cospi_22_64 - step1[13] * cospi_10_64;
    temp2 = step1[10] * cospi_10_64 + step1[13] * cospi_22_64;
    step2[10] = dct_const_round_shift(temp1);
    step2[13] = dct_const_round_shift(temp2);

    temp1 = step1[11] * cospi_6_64 - step1[12] * cospi_26_64;
    temp2 = step1[11] * cospi_26_64 + step1[12] * cospi_6_64;
    step2[11] = dct_const_round_shift(temp1);
    step2[12] = dct_const_round_shift(temp2);

    step2[16] = step1[16] + step1[17];
    step2[17] = step1[16] - step1[17];
    step2[18] = -step1[18] + step1[19];
    step2[19] = step1[18] + step1[19];
    step2[20] = step1[20] + step1[21];
    step2[21] = step1[20] - step1[21];
    step2[22] = -step1[22] + step1[23];
    step2[23] = step1[22] + step1[23];
    step2[24] = step1[24] + step1[25];
    step2[25] = step1[24] - step1[25];
    step2[26] = -step1[26] + step1[27];
    step2[27] = step1[26] + step1[27];
    step2[28] = step1[28] + step1[29];
    step2[29] = step1[28] - step1[29];
    step2[30] = -step1[30] + step1[31];
    step2[31] = step1[30] + step1[31];

    // stage 3
    step1[0] = step2[0];
    step1[1] = step2[1];
    step1[2] = step2[2];
    step1[3] = step2[3];

    temp1 = step2[4] * cospi_28_64 - step2[7] * cospi_4_64;
    temp2 = step2[4] * cospi_4_64 + step2[7] * cospi_28_64;
    step1[4] = dct_const_round_shift(temp1);
    step1[7] = dct_const_round_shift(temp2);
    temp1 = step2[5] * cospi_12_64 - step2[6] * cospi_20_64;
    temp2 = step2[5] * cospi_20_64 + step2[6] * cospi_12_64;
    step1[5] = dct_const_round_shift(temp1);
    step1[6] = dct_const_round_shift(temp2);

    step1[8] = step2[8] + step2[9];
    step1[9] = step2[8] - step2[9];
    step1[10] = -step2[10] + step2[11];
    step1[11] = step2[10] + step2[11];
    step1[12] = step2[12] + step2[13];
    step1[13] = step2[12] - step2[13];
    step1[14] = -step2[14] + step2[15];
    step1[15] = step2[14] + step2[15];

    step1[16] = step2[16];
    step1[31] = step2[31];
    temp1 = -step2[17] * cospi_4_64 + step2[30] * cospi_28_64;
    temp2 = step2[17] * cospi_28_64 + step2[30] * cospi_4_64;
    step1[17] = dct_const_round_shift(temp1);
    step1[30] = dct_const_round_shift(temp2);
    temp1 = -step2[18] * cospi_28_64 - step2[29] * cospi_4_64;
    temp2 = -step2[18] * cospi_4_64 + step2[29] * cospi_28_64;
    step1[18] = dct_const_round_shift(temp1);
    step1[29] = dct_const_round_shift(temp2);
    step1[19] = step2[19];
    step1[20] = step2[20];
    temp1 = -step2[21] * cospi_20_64 + step2[26] * cospi_12_64;
    temp2 = step2[21] * cospi_12_64 + step2[26] * cospi_20_64;
    step1[21] = dct_const_round_shift(temp1);
    step1[26] = dct_const_round_shift(temp2);
    temp1 = -step2[22] * cospi_12_64 - step2[25] * cospi_20_64;
    temp2 = -step2[22] * cospi_20_64 + step2[25] * cospi_12_64;
    step1[22] = dct_const_round_shift(temp1);
    step1[25] = dct_const_round_shift(temp2);
    step1[23] = step2[23];
    step1[24] = step2[24];
    step1[27] = step2[27];
    step1[28] = step2[28];

    // stage 4
    temp1 = (step1[0] + step1[1]) * cospi_16_64;
    temp2 = (step1[0] - step1[1]) * cospi_16_64;
    step2[0] = dct_const_round_shift(temp1);
    step2[1] = dct_const_round_shift(temp2);
    temp1 = step1[2] * cospi_24_64 - step1[3] * cospi_8_64;
    temp2 = step1[2] * cospi_8_64 + step1[3] * cospi_24_64;
    step2[2] = dct_const_round_shift(temp1);
    step2[3] = dct_const_round_shift(temp2);
    step2[4] = step1[4] + step1[5];
    step2[5] = step1[4] - step1[5];
    step2[6] = -step1[6] + step1[7];
    step2[7] = step1[6] + step1[7];

    step2[8] = step1[8];
    step2[15] = step1[15];
    temp1 = -step1[9] * cospi_8_64 + step1[14] * cospi_24_64;
    temp2 = step1[9] * cospi_24_64 + step1[14] * cospi_8_64;
    step2[9] = dct_const_round_shift(temp1);
    step2[14] = dct_const_round_shift(temp2);
    temp1 = -step1[10] * cospi_24_64 - step1[13] * cospi_8_64;
    temp2 = -step1[10] * cospi_8_64 + step1[13] * cospi_24_64;
    step2[10] = dct_const_round_shift(temp1);
    step2[13] = dct_const_round_shift(temp2);
    step2[11] = step1[11];
    step2[12] = step1[12];

    step2[16] = step1[16] + step1[19];
    step2[17] = step1[17] + step1[18];
    step2[18] = step1[17] - step1[18];
    step2[19] = step1[16] - step1[19];
    step2[20] = -step1[20] + step1[23];
    step2[21] = -step1[21] + step1[22];
    step2[22] = step1[21] + step1[22];
    step2[23] = step1[20] + step1[23];

    step2[24] = step1[24] + step1[27];
    step2[25] = step1[25] + step1[26];
    step2[26] = step1[25] - step1[26];
    step2[27] = step1[24] - step1[27];
    step2[28] = -step1[28] + step1[31];
    step2[29] = -step1[29] + step1[30];
    step2[30] = step1[29] + step1[30];
    step2[31] = step1[28] + step1[31];

    // stage 5
    step1[0] = step2[0] + step2[3];
    step1[1] = step2[1] + step2[2];
    step1[2] = step2[1] - step2[2];
    step1[3] = step2[0] - step2[3];
    step1[4] = step2[4];
    temp1 = (step2[6] - step2[5]) * cospi_16_64;
    temp2 = (step2[5] + step2[6]) * cospi_16_64;
    step1[5] = dct_const_round_shift(temp1);
    step1[6] = dct_const_round_shift(temp2);
    step1[7] = step2[7];

    step1[8] = step2[8] + step2[11];
    step1[9] = step2[9] + step2[10];
    step1[10] = step2[9] - step2[10];
    step1[11] = step2[8] - step2[11];
    step1[12] = -step2[12] + step2[15];
    step1[13] = -step2[13] + step2[14];
    step1[14] = step2[13] + step2[14];
    step1[15] = step2[12] + step2[15];

    step1[16] = step2[16];
    step1[17] = step2[17];
    temp1 = -step2[18] * cospi_8_64 + step2[29] * cospi_24_64;
    temp2 = step2[18] * cospi_24_64 + step2[29] * cospi_8_64;
    step1[18] = dct_const_round_shift(temp1);
    step1[29] = dct_const_round_shift(temp2);
    temp1 = -step2[19] * cospi_8_64 + step2[28] * cospi_24_64;
    temp2 = step2[19] * cospi_24_64 + step2[28] * cospi_8_64;
    step1[19] = dct_const_round_shift(temp1);
    step1[28] = dct_const_round_shift(temp2);
    temp1 = -step2[20] * cospi_24_64 - step2[27] * cospi_8_64;
    temp2 = -step2[20] * cospi_8_64 + step2[27] * cospi_24_64;
    step1[20] = dct_const_round_shift(temp1);
    step1[27] = dct_const_round_shift(temp2);
    temp1 = -step2[21] * cospi_24_64 - step2[26] * cospi_8_64;
    temp2 = -step2[21] * cospi_8_64 + step2[26] * cospi_24_64;
    step1[21] = dct_const_round_shift(temp1);
    step1[26] = dct_const_round_shift(temp2);
    step1[22] = step2[22];
    step1[23] = step2[23];
    step1[24] = step2[24];
    step1[25] = step2[25];
    step1[30] = step2[30];
    step1[31] = step2[31];

    // stage 6
    step2[0] = step1[0] + step1[7];
    step2[1] = step1[1] + step1[6];
    step2[2] = step1[2] + step1[5];
    step2[3] = step1[3] + step1[4];
    step2[4] = step1[3] - step1[4];
    step2[5] = step1[2] - step1[5];
    step2[6] = step1[1] - step1[6];
    step2[7] = step1[0] - step1[7];
    step2[8] = step1[8];
    step2[9] = step1[9];
    temp1 = (-step1[10] + step1[13]) * cospi_16_64;
    temp2 = (step1[10] + step1[13]) * cospi_16_64;
    step2[10] = dct_const_round_shift(temp1);
    step2[13] = dct_const_round_shift(temp2);
    temp1 = (-step1[11] + step1[12]) * cospi_16_64;
    temp2 = (step1[11] + step1[12]) * cospi_16_64;
    step2[11] = dct_const_round_shift(temp1);
    step2[12] = dct_const_round_shift(temp2);
    step2[14] = step1[14];
    step2[15] = step1[15];

    step2[16] = step1[16] + step1[23];
    step2[17] = step1[17] + step1[22];
    step2[18] = step1[18] + step1[21];
    step2[19] = step1[19] + step1[20];
    step2[20] = step1[19] - step1[20];
    step2[21] = step1[18] - step1[21];
    step2[22] = step1[17] - step1[22];
    step2[23] = step1[16] - step1[23];

    step2[24] = -step1[24] + step1[31];
    step2[25] = -step1[25] + step1[30];
    step2[26] = -step1[26] + step1[29];
    step2[27] = -step1[27] + step1[28];
    step2[28] = step1[27] + step1[28];
    step2[29] = step1[26] + step1[29];
    step2[30] = step1[25] + step1[30];
    step2[31] = step1[24] + step1[31];

    // stage 7
    step1[0] = step2[0] + step2[15];
    step1[1] = step2[1] + step2[14];
    step1[2] = step2[2] + step2[13];
    step1[3] = step2[3] + step2[12];
    step1[4] = step2[4] + step2[11];
    step1[5] = step2[5] + step2[10];
    step1[6] = step2[6] + step2[9];
    step1[7] = step2[7] + step2[8];
    step1[8] = step2[7] - step2[8];
    step1[9] = step2[6] - step2[9];
    step1[10] = step2[5] - step2[10];
    step1[11] = step2[4] - step2[11];
    step1[12] = step2[3] - step2[12];
    step1[13] = step2[2] - step2[13];
    step1[14] = step2[1] - step2[14];
    step1[15] = step2[0] - step2[15];

    step1[16] = step2[16];
    step1[17] = step2[17];
    step1[18] = step2[18];
    step1[19] = step2[19];
    temp1 = (-step2[20] + step2[27]) * cospi_16_64;
    temp2 = (step2[20] + step2[27]) * cospi_16_64;
    step1[20] = dct_const_round_shift(temp1);
    step1[27] = dct_const_round_shift(temp2);
    temp1 = (-step2[21] + step2[26]) * cospi_16_64;
    temp2 = (step2[21] + step2[26]) * cospi_16_64;
    step1[21] = dct_const_round_shift(temp1);
    step1[26] = dct_const_round_shift(temp2);
    temp1 = (-step2[22] + step2[25]) * cospi_16_64;
    temp2 = (step2[22] + step2[25]) * cospi_16_64;
    step1[22] = dct_const_round_shift(temp1);
    step1[25] = dct_const_round_shift(temp2);
    temp1 = (-step2[23] + step2[24]) * cospi_16_64;
    temp2 = (step2[23] + step2[24]) * cospi_16_64;
    step1[23] = dct_const_round_shift(temp1);
    step1[24] = dct_const_round_shift(temp2);
    step1[28] = step2[28];
    step1[29] = step2[29];
    step1[30] = step2[30];
    step1[31] = step2[31];

    // final stage
    output[0] = step1[0] + step1[31];
    output[1] = step1[1] + step1[30];
    output[2] = step1[2] + step1[29];
    output[3] = step1[3] + step1[28];
    output[4] = step1[4] + step1[27];
    output[5] = step1[5] + step1[26];
    output[6] = step1[6] + step1[25];
    output[7] = step1[7] + step1[24];
    output[8] = step1[8] + step1[23];
    output[9] = step1[9] + step1[22];
    output[10] = step1[10] + step1[21];
    output[11] = step1[11] + step1[20];
    output[12] = step1[12] + step1[19];
    output[13] = step1[13] + step1[18];
    output[14] = step1[14] + step1[17];
    output[15] = step1[15] + step1[16];
    output[16] = step1[15] - step1[16];
    output[17] = step1[14] - step1[17];
    output[18] = step1[13] - step1[18];
    output[19] = step1[12] - step1[19];
    output[20] = step1[11] - step1[20];
    output[21] = step1[10] - step1[21];
    output[22] = step1[9] - step1[22];
    output[23] = step1[8] - step1[23];
    output[24] = step1[7] - step1[24];
    output[25] = step1[6] - step1[25];
    output[26] = step1[5] - step1[26];
    output[27] = step1[4] - step1[27];
    output[28] = step1[3] - step1[28];
    output[29] = step1[2] - step1[29];
    output[30] = step1[1] - step1[30];
    output[31] = step1[0] - step1[31];
}

static void idct32x32_1(const rs_allocation input, rs_allocation dest, int xoff, int yoff) {
    int i, j;
    int a1;
    int16_t out = dct_const_round_shift(rsGetElementAt_short(input, xoff, yoff)
                          * cospi_16_64);
    out = dct_const_round_shift(out * cospi_16_64);
    a1 = ROUND_POWER_OF_TWO(out, 6);
    uint8_t result;
    for (i = 0; i < 32; ++i) {
        for (j = 0; j < 32; ++j) {
            result = clip_pixel(rsGetElementAt_uchar(dest, j + xoff, i + yoff) + a1);
            rsSetElementAt_uchar(dest, result, j + xoff, i + yoff);
        }
    }
}

static void idct32x32_34(const rs_allocation input, rs_allocation dest, int xoff, int yoff) {
    int16_t out[32 * 32] = { 0 };
    int16_t *outptr = out;
    int i, j;
    int16_t temp_in[32], temp_out[32];

    int16_t in[32 * 32];
    int16_t *inptr = in;
    for (i = 0; i < 32; ++i) {
        for (j = 0; j < 32; ++j) {
            in[j + i * 32] = rsGetElementAt_short(input, j + xoff, i + yoff);
        }
    }

    // Rows
    // only upper-left 8x8 has non-zero coeff
    for (i = 0; i < 8; ++i) {
        idct32_1d(inptr, outptr);
        inptr += 32;
        outptr += 32;
    }

    // Columns
    uint8_t result;
    for (i = 0; i < 32; ++i) {
        for (j = 0; j < 32; ++j)
            temp_in[j] = out[j * 32 + i];
        idct32_1d(temp_in, temp_out);
        for (j = 0; j < 32; ++j) {
            result = clip_pixel(ROUND_POWER_OF_TWO(temp_out[j], 6)
                             + rsGetElementAt_uchar(dest, i + xoff, j + yoff));
            rsSetElementAt_uchar(dest, result, i + xoff, j + yoff);
        }
    }
}

static void idct32x32_1024(const rs_allocation input, rs_allocation dest, int xoff, int yoff) {
    int16_t out[32 * 32];
    int16_t *outptr = out;
    int i, j;
    int16_t temp_in[32], temp_out[32];

    int16_t in[32 * 32];
    int16_t *inptr = in;
    for (i = 0; i < 32; ++i) {
        for (j = 0; j < 32; ++j) {
            in[j + i * 32] = rsGetElementAt_short(input, j + xoff, i + yoff);
        }
    }

    // Rows
    for (i = 0; i < 32; ++i) {
        int16_t zero_coeff[16];
        for (j = 0; j < 16; ++j)
            zero_coeff[j] = inptr[2 * j] | inptr[2 * j + 1];
        for (j = 0; j < 8; ++j)
            zero_coeff[j] = zero_coeff[2 * j] | zero_coeff[2 * j + 1];
        for (j = 0; j < 4; ++j)
            zero_coeff[j] = zero_coeff[2 * j] | zero_coeff[2 * j + 1];
        for (j = 0; j < 2; ++j)
            zero_coeff[j] = zero_coeff[2 * j] | zero_coeff[2 * j + 1];

        if (zero_coeff[0] | zero_coeff[1]) {
            idct32_1d(inptr, outptr);
        } else {
            for (int ct = 0; ct < 32; ct++) {
                outptr[ct] = 0;
            }
        }
        inptr += 32;
        outptr += 32;
    }

    // Columns
    uint8_t result;
    for (i = 0; i < 32; ++i) {
        for (j = 0; j < 32; ++j)
            temp_in[j] = out[j * 32 + i];
        idct32_1d(temp_in, temp_out);
        for (j = 0; j < 32; ++j) {
            result = clip_pixel(ROUND_POWER_OF_TWO(temp_out[j], 6)
                             + rsGetElementAt_uchar(dest, i + xoff, j + yoff));
            rsSetElementAt_uchar(dest, result, i + xoff, j + yoff);
        }
    }
}

extern void rsIdct4x4(const rs_allocation input, rs_allocation dest, int eob,
        int xoff, int yoff) {
    if (eob > 1) {
        idct4x4_16(input, dest, xoff, yoff);
    } else {
        idct4x4_1(input, dest, xoff, yoff);
    }
}

extern void rsIdct8x8(const rs_allocation input, rs_allocation dest, int eob,
        int xoff, int yoff) {
    if (eob == 1)
        // DC only DCT coefficient
        idct8x8_1(input, dest, xoff, yoff);
    else if (eob <= 10)
        idct8x8_10(input, dest, xoff, yoff);
    else
        idct8x8_64(input, dest, xoff, yoff);
}

extern void rsIdct16x16(const rs_allocation input, rs_allocation dest, int eob,
        int xoff, int yoff) {
    if (eob == 1)
        /* DC only DCT coefficient. */
        idct16x16_1(input, dest, xoff, yoff);
    else if (eob <= 10)
        idct16x16_10(input, dest, xoff, yoff);
    else
        idct16x16_256(input, dest, xoff, yoff);
}

extern void rsIdct32x32(const rs_allocation input, rs_allocation dest, int eob,
        int xoff, int yoff) {
    if (eob == 1)
        idct32x32_1(input, dest, xoff, yoff);
    else if (eob <= 34)
        // non-zero coeff only in upper-left 8x8
        idct32x32_34(input, dest, xoff, yoff);
    else
        idct32x32_1024(input, dest, xoff, yoff);
}
