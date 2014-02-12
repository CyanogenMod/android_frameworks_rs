#include "rs_types.rsh"
#include "rs_allocation.rsh"
#include "rs_dct.h"

static int fdct_round_shift(int input) {
    int rv = ROUND_POWER_OF_TWO(input, DCT_CONST_BITS);
    return rv;
}

static void fdct8(const int16_t *input, int16_t *output) {
    /*canbe16*/ int s0, s1, s2, s3, s4, s5, s6, s7;
    /*needs32*/ int t0, t1, t2, t3;
    /*canbe16*/ int x0, x1, x2, x3;

    // stage 1
    s0 = input[0] + input[7];
    s1 = input[1] + input[6];
    s2 = input[2] + input[5];
    s3 = input[3] + input[4];
    s4 = input[3] - input[4];
    s5 = input[2] - input[5];
    s6 = input[1] - input[6];
    s7 = input[0] - input[7];

    // fdct4(step, step);
    x0 = s0 + s3;
    x1 = s1 + s2;
    x2 = s1 - s2;
    x3 = s0 - s3;
    t0 = (x0 + x1) * cospi_16_64;
    t1 = (x0 - x1) * cospi_16_64;
    t2 = x2 * cospi_24_64 + x3 * cospi_8_64;
    t3 = -x2 * cospi_8_64 + x3 * cospi_24_64;
    output[0] = fdct_round_shift(t0);
    output[2] = fdct_round_shift(t2);
    output[4] = fdct_round_shift(t1);
    output[6] = fdct_round_shift(t3);

    // Stage 2
    t0 = (s6 - s5) * cospi_16_64;
    t1 = (s6 + s5) * cospi_16_64;
    t2 = fdct_round_shift(t0);
    t3 = fdct_round_shift(t1);

    // Stage 3
    x0 = s4 + t2;
    x1 = s4 - t2;
    x2 = s7 - t3;
    x3 = s7 + t3;

    // Stage 4
    t0 = x0 * cospi_28_64 + x3 * cospi_4_64;
    t1 = x1 * cospi_12_64 + x2 * cospi_20_64;
    t2 = x2 * cospi_12_64 + x1 * -cospi_20_64;
    t3 = x3 * cospi_28_64 + x0 * -cospi_4_64;
    output[1] = fdct_round_shift(t0);
    output[3] = fdct_round_shift(t2);
    output[5] = fdct_round_shift(t1);
    output[7] = fdct_round_shift(t3);
}

static int dct_32_round(int input) {
    int rv = ROUND_POWER_OF_TWO(input, DCT_CONST_BITS);
    return rv;
}

static int half_round_shift(int input) {
    int rv = (input + 1 + (input < 0)) >> 2;
    return rv;
}

static void dct32_1d(const int *input, int *output, int round) {
    int step[32];
    // Stage 1
    step[0] = input[0] + input[(32 - 1)];
    step[1] = input[1] + input[(32 - 2)];
    step[2] = input[2] + input[(32 - 3)];
    step[3] = input[3] + input[(32 - 4)];
    step[4] = input[4] + input[(32 - 5)];
    step[5] = input[5] + input[(32 - 6)];
    step[6] = input[6] + input[(32 - 7)];
    step[7] = input[7] + input[(32 - 8)];
    step[8] = input[8] + input[(32 - 9)];
    step[9] = input[9] + input[(32 - 10)];
    step[10] = input[10] + input[(32 - 11)];
    step[11] = input[11] + input[(32 - 12)];
    step[12] = input[12] + input[(32 - 13)];
    step[13] = input[13] + input[(32 - 14)];
    step[14] = input[14] + input[(32 - 15)];
    step[15] = input[15] + input[(32 - 16)];
    step[16] = -input[16] + input[(32 - 17)];
    step[17] = -input[17] + input[(32 - 18)];
    step[18] = -input[18] + input[(32 - 19)];
    step[19] = -input[19] + input[(32 - 20)];
    step[20] = -input[20] + input[(32 - 21)];
    step[21] = -input[21] + input[(32 - 22)];
    step[22] = -input[22] + input[(32 - 23)];
    step[23] = -input[23] + input[(32 - 24)];
    step[24] = -input[24] + input[(32 - 25)];
    step[25] = -input[25] + input[(32 - 26)];
    step[26] = -input[26] + input[(32 - 27)];
    step[27] = -input[27] + input[(32 - 28)];
    step[28] = -input[28] + input[(32 - 29)];
    step[29] = -input[29] + input[(32 - 30)];
    step[30] = -input[30] + input[(32 - 31)];
    step[31] = -input[31] + input[(32 - 32)];

    // Stage 2
    output[0] = step[0] + step[16 - 1];
    output[1] = step[1] + step[16 - 2];
    output[2] = step[2] + step[16 - 3];
    output[3] = step[3] + step[16 - 4];
    output[4] = step[4] + step[16 - 5];
    output[5] = step[5] + step[16 - 6];
    output[6] = step[6] + step[16 - 7];
    output[7] = step[7] + step[16 - 8];
    output[8] = -step[8] + step[16 - 9];
    output[9] = -step[9] + step[16 - 10];
    output[10] = -step[10] + step[16 - 11];
    output[11] = -step[11] + step[16 - 12];
    output[12] = -step[12] + step[16 - 13];
    output[13] = -step[13] + step[16 - 14];
    output[14] = -step[14] + step[16 - 15];
    output[15] = -step[15] + step[16 - 16];

    output[16] = step[16];
    output[17] = step[17];
    output[18] = step[18];
    output[19] = step[19];

    output[20] = dct_32_round((-step[20] + step[27]) * cospi_16_64);
    output[21] = dct_32_round((-step[21] + step[26]) * cospi_16_64);
    output[22] = dct_32_round((-step[22] + step[25]) * cospi_16_64);
    output[23] = dct_32_round((-step[23] + step[24]) * cospi_16_64);

    output[24] = dct_32_round((step[24] + step[23]) * cospi_16_64);
    output[25] = dct_32_round((step[25] + step[22]) * cospi_16_64);
    output[26] = dct_32_round((step[26] + step[21]) * cospi_16_64);
    output[27] = dct_32_round((step[27] + step[20]) * cospi_16_64);

    output[28] = step[28];
    output[29] = step[29];
    output[30] = step[30];
    output[31] = step[31];

    // dump the magnitude by 4, hence the intermediate values are within
    // the range of 16 bits.
    if (round) {
        output[0] = half_round_shift(output[0]);
        output[1] = half_round_shift(output[1]);
        output[2] = half_round_shift(output[2]);
        output[3] = half_round_shift(output[3]);
        output[4] = half_round_shift(output[4]);
        output[5] = half_round_shift(output[5]);
        output[6] = half_round_shift(output[6]);
        output[7] = half_round_shift(output[7]);
        output[8] = half_round_shift(output[8]);
        output[9] = half_round_shift(output[9]);
        output[10] = half_round_shift(output[10]);
        output[11] = half_round_shift(output[11]);
        output[12] = half_round_shift(output[12]);
        output[13] = half_round_shift(output[13]);
        output[14] = half_round_shift(output[14]);
        output[15] = half_round_shift(output[15]);

        output[16] = half_round_shift(output[16]);
        output[17] = half_round_shift(output[17]);
        output[18] = half_round_shift(output[18]);
        output[19] = half_round_shift(output[19]);
        output[20] = half_round_shift(output[20]);
        output[21] = half_round_shift(output[21]);
        output[22] = half_round_shift(output[22]);
        output[23] = half_round_shift(output[23]);
        output[24] = half_round_shift(output[24]);
        output[25] = half_round_shift(output[25]);
        output[26] = half_round_shift(output[26]);
        output[27] = half_round_shift(output[27]);
        output[28] = half_round_shift(output[28]);
        output[29] = half_round_shift(output[29]);
        output[30] = half_round_shift(output[30]);
        output[31] = half_round_shift(output[31]);
    }

    // Stage 3
    step[0] = output[0] + output[(8 - 1)];
    step[1] = output[1] + output[(8 - 2)];
    step[2] = output[2] + output[(8 - 3)];
    step[3] = output[3] + output[(8 - 4)];
    step[4] = -output[4] + output[(8 - 5)];
    step[5] = -output[5] + output[(8 - 6)];
    step[6] = -output[6] + output[(8 - 7)];
    step[7] = -output[7] + output[(8 - 8)];
    step[8] = output[8];
    step[9] = output[9];
    step[10] = dct_32_round((-output[10] + output[13]) * cospi_16_64);
    step[11] = dct_32_round((-output[11] + output[12]) * cospi_16_64);
    step[12] = dct_32_round((output[12] + output[11]) * cospi_16_64);
    step[13] = dct_32_round((output[13] + output[10]) * cospi_16_64);
    step[14] = output[14];
    step[15] = output[15];

    step[16] = output[16] + output[23];
    step[17] = output[17] + output[22];
    step[18] = output[18] + output[21];
    step[19] = output[19] + output[20];
    step[20] = -output[20] + output[19];
    step[21] = -output[21] + output[18];
    step[22] = -output[22] + output[17];
    step[23] = -output[23] + output[16];
    step[24] = -output[24] + output[31];
    step[25] = -output[25] + output[30];
    step[26] = -output[26] + output[29];
    step[27] = -output[27] + output[28];
    step[28] = output[28] + output[27];
    step[29] = output[29] + output[26];
    step[30] = output[30] + output[25];
    step[31] = output[31] + output[24];

    // Stage 4
    output[0] = step[0] + step[3];
    output[1] = step[1] + step[2];
    output[2] = -step[2] + step[1];
    output[3] = -step[3] + step[0];
    output[4] = step[4];
    output[5] = dct_32_round((-step[5] + step[6]) * cospi_16_64);
    output[6] = dct_32_round((step[6] + step[5]) * cospi_16_64);
    output[7] = step[7];
    output[8] = step[8] + step[11];
    output[9] = step[9] + step[10];
    output[10] = -step[10] + step[9];
    output[11] = -step[11] + step[8];
    output[12] = -step[12] + step[15];
    output[13] = -step[13] + step[14];
    output[14] = step[14] + step[13];
    output[15] = step[15] + step[12];

    output[16] = step[16];
    output[17] = step[17];
    output[18] = dct_32_round(step[18] * -cospi_8_64 + step[29] * cospi_24_64);
    output[19] = dct_32_round(step[19] * -cospi_8_64 + step[28] * cospi_24_64);
    output[20] = dct_32_round(step[20] * -cospi_24_64 + step[27] * -cospi_8_64);
    output[21] = dct_32_round(step[21] * -cospi_24_64 + step[26] * -cospi_8_64);
    output[22] = step[22];
    output[23] = step[23];
    output[24] = step[24];
    output[25] = step[25];
    output[26] = dct_32_round(step[26] * cospi_24_64 + step[21] * -cospi_8_64);
    output[27] = dct_32_round(step[27] * cospi_24_64 + step[20] * -cospi_8_64);
    output[28] = dct_32_round(step[28] * cospi_8_64 + step[19] * cospi_24_64);
    output[29] = dct_32_round(step[29] * cospi_8_64 + step[18] * cospi_24_64);
    output[30] = step[30];
    output[31] = step[31];

    // Stage 5
    step[0] = dct_32_round((output[0] + output[1]) * cospi_16_64);
    step[1] = dct_32_round((-output[1] + output[0]) * cospi_16_64);
    step[2] = dct_32_round(output[2] * cospi_24_64 + output[3] * cospi_8_64);
    step[3] = dct_32_round(output[3] * cospi_24_64 - output[2] * cospi_8_64);
    step[4] = output[4] + output[5];
    step[5] = -output[5] + output[4];
    step[6] = -output[6] + output[7];
    step[7] = output[7] + output[6];
    step[8] = output[8];
    step[9] = dct_32_round(output[9] * -cospi_8_64 + output[14] * cospi_24_64);
    step[10] = dct_32_round(output[10] * -cospi_24_64 + output[13] * -cospi_8_64);
    step[11] = output[11];
    step[12] = output[12];
    step[13] = dct_32_round(output[13] * cospi_24_64 + output[10] * -cospi_8_64);
    step[14] = dct_32_round(output[14] * cospi_8_64 + output[9] * cospi_24_64);
    step[15] = output[15];

    step[16] = output[16] + output[19];
    step[17] = output[17] + output[18];
    step[18] = -output[18] + output[17];
    step[19] = -output[19] + output[16];
    step[20] = -output[20] + output[23];
    step[21] = -output[21] + output[22];
    step[22] = output[22] + output[21];
    step[23] = output[23] + output[20];
    step[24] = output[24] + output[27];
    step[25] = output[25] + output[26];
    step[26] = -output[26] + output[25];
    step[27] = -output[27] + output[24];
    step[28] = -output[28] + output[31];
    step[29] = -output[29] + output[30];
    step[30] = output[30] + output[29];
    step[31] = output[31] + output[28];

    // Stage 6
    output[0] = step[0];
    output[1] = step[1];
    output[2] = step[2];
    output[3] = step[3];
    output[4] = dct_32_round(step[4] * cospi_28_64 + step[7] * cospi_4_64);
    output[5] = dct_32_round(step[5] * cospi_12_64 + step[6] * cospi_20_64);
    output[6] = dct_32_round(step[6] * cospi_12_64 + step[5] * -cospi_20_64);
    output[7] = dct_32_round(step[7] * cospi_28_64 + step[4] * -cospi_4_64);
    output[8] = step[8] + step[9];
    output[9] = -step[9] + step[8];
    output[10] = -step[10] + step[11];
    output[11] = step[11] + step[10];
    output[12] = step[12] + step[13];
    output[13] = -step[13] + step[12];
    output[14] = -step[14] + step[15];
    output[15] = step[15] + step[14];

    output[16] = step[16];
    output[17] = dct_32_round(step[17] * -cospi_4_64 + step[30] * cospi_28_64);
    output[18] = dct_32_round(step[18] * -cospi_28_64 + step[29] * -cospi_4_64);
    output[19] = step[19];
    output[20] = step[20];
    output[21] = dct_32_round(step[21] * -cospi_20_64 + step[26] * cospi_12_64);
    output[22] = dct_32_round(step[22] * -cospi_12_64 + step[25] * -cospi_20_64);
    output[23] = step[23];
    output[24] = step[24];
    output[25] = dct_32_round(step[25] * cospi_12_64 + step[22] * -cospi_20_64);
    output[26] = dct_32_round(step[26] * cospi_20_64 + step[21] * cospi_12_64);
    output[27] = step[27];
    output[28] = step[28];
    output[29] = dct_32_round(step[29] * cospi_28_64 + step[18] * -cospi_4_64);
    output[30] = dct_32_round(step[30] * cospi_4_64 + step[17] * cospi_28_64);
    output[31] = step[31];

    // Stage 7
    step[0] = output[0];
    step[1] = output[1];
    step[2] = output[2];
    step[3] = output[3];
    step[4] = output[4];
    step[5] = output[5];
    step[6] = output[6];
    step[7] = output[7];
    step[8] = dct_32_round(output[8] * cospi_30_64 + output[15] * cospi_2_64);
    step[9] = dct_32_round(output[9] * cospi_14_64 + output[14] * cospi_18_64);
    step[10] = dct_32_round(output[10] * cospi_22_64 + output[13] * cospi_10_64);
    step[11] = dct_32_round(output[11] * cospi_6_64 + output[12] * cospi_26_64);
    step[12] = dct_32_round(output[12] * cospi_6_64 + output[11] * -cospi_26_64);
    step[13] = dct_32_round(output[13] * cospi_22_64 + output[10] * -cospi_10_64);
    step[14] = dct_32_round(output[14] * cospi_14_64 + output[9] * -cospi_18_64);
    step[15] = dct_32_round(output[15] * cospi_30_64 + output[8] * -cospi_2_64);

    step[16] = output[16] + output[17];
    step[17] = -output[17] + output[16];
    step[18] = -output[18] + output[19];
    step[19] = output[19] + output[18];
    step[20] = output[20] + output[21];
    step[21] = -output[21] + output[20];
    step[22] = -output[22] + output[23];
    step[23] = output[23] + output[22];
    step[24] = output[24] + output[25];
    step[25] = -output[25] + output[24];
    step[26] = -output[26] + output[27];
    step[27] = output[27] + output[26];
    step[28] = output[28] + output[29];
    step[29] = -output[29] + output[28];
    step[30] = -output[30] + output[31];
    step[31] = output[31] + output[30];

    // Final stage --- outputs indices are bit-reversed.
    output[0]  = step[0];
    output[16] = step[1];
    output[8]  = step[2];
    output[24] = step[3];
    output[4]  = step[4];
    output[20] = step[5];
    output[12] = step[6];
    output[28] = step[7];
    output[2]  = step[8];
    output[18] = step[9];
    output[10] = step[10];
    output[26] = step[11];
    output[6]  = step[12];
    output[22] = step[13];
    output[14] = step[14];
    output[30] = step[15];

    output[1]  = dct_32_round(step[16] * cospi_31_64 + step[31] * cospi_1_64);
    output[17] = dct_32_round(step[17] * cospi_15_64 + step[30] * cospi_17_64);
    output[9]  = dct_32_round(step[18] * cospi_23_64 + step[29] * cospi_9_64);
    output[25] = dct_32_round(step[19] * cospi_7_64 + step[28] * cospi_25_64);
    output[5]  = dct_32_round(step[20] * cospi_27_64 + step[27] * cospi_5_64);
    output[21] = dct_32_round(step[21] * cospi_11_64 + step[26] * cospi_21_64);
    output[13] = dct_32_round(step[22] * cospi_19_64 + step[25] * cospi_13_64);
    output[29] = dct_32_round(step[23] * cospi_3_64 + step[24] * cospi_29_64);
    output[3]  = dct_32_round(step[24] * cospi_3_64 + step[23] * -cospi_29_64);
    output[19] = dct_32_round(step[25] * cospi_19_64 + step[22] * -cospi_13_64);
    output[11] = dct_32_round(step[26] * cospi_11_64 + step[21] * -cospi_21_64);
    output[27] = dct_32_round(step[27] * cospi_27_64 + step[20] * -cospi_5_64);
    output[7]  = dct_32_round(step[28] * cospi_7_64 + step[19] * -cospi_25_64);
    output[23] = dct_32_round(step[29] * cospi_23_64 + step[18] * -cospi_9_64);
    output[15] = dct_32_round(step[30] * cospi_15_64 + step[17] * -cospi_17_64);
    output[31] = dct_32_round(step[31] * cospi_31_64 + step[16] * -cospi_1_64);
}

extern void rsDct4x4(rs_allocation input, rs_allocation output, int xoff, int yoff) {
    // The 2D transform is done with two passes which are actually pretty
    // similar. In the first one, we transform the columns and transpose
    // the results. In the second one, we transform the rows. To achieve that,
    // as the first pass results are transposed, we tranpose the columns (that
    // is the transposed rows) and transpose the results (so that it goes back
    // in normal/row positions).
    int pass;
    int i, j;
    // We need an intermediate buffer between passes.
    int16_t intermediate[4 * 4];
    int16_t inptr[4 * 4];
    int16_t outptr[4 * 4];
    int16_t *in = inptr;
    int16_t *output_1 = outptr;
    int16_t *out = intermediate;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            inptr[i + j * 4] = rsGetElementAt_short(input, xoff + i, yoff + j);
            outptr[i + j * 4] = rsGetElementAt_short(output, xoff + i, yoff + j);
        }
    }

    // Do the two transform/transpose passes
    for (pass = 0; pass < 2; ++pass) {
        /*canbe16*/ int input[4];
        /*canbe16*/ int step[4];
        /*needs32*/ int temp1, temp2;
        int i;
        for (i = 0; i < 4; ++i) {
            // Load inputs.
            if (0 == pass) {
                input[0] = in[0 * 4] * 16;
                input[1] = in[1 * 4] * 16;
                input[2] = in[2 * 4] * 16;
                input[3] = in[3 * 4] * 16;
                if (i == 0 && input[0]) {
                    input[0] += 1;
                }
            } else {
                input[0] = in[0 * 4];
                input[1] = in[1 * 4];
                input[2] = in[2 * 4];
                input[3] = in[3 * 4];
            }
            // Transform.
            step[0] = input[0] + input[3];
            step[1] = input[1] + input[2];
            step[2] = input[1] - input[2];
            step[3] = input[0] - input[3];
            temp1 = (step[0] + step[1]) * cospi_16_64;
            temp2 = (step[0] - step[1]) * cospi_16_64;
            out[0] = fdct_round_shift(temp1);
            out[2] = fdct_round_shift(temp2);
            temp1 = step[2] * cospi_24_64 + step[3] * cospi_8_64;
            temp2 = -step[2] * cospi_8_64 + step[3] * cospi_24_64;
            out[1] = fdct_round_shift(temp1);
            out[3] = fdct_round_shift(temp2);
            // Do next column (which is a transposed row in second/horizontal pass)
            in++;
            out += 4;
        }
        // Setup in/out for next pass.
        in = intermediate;
        out = output_1;
    }

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            rsSetElementAt_short(output, (output_1[i + j * 4] + 1) >> 2,
                    xoff + i, yoff + j);
        }
    }
}

extern void rsDct8x8(rs_allocation input, rs_allocation output, int xoff, int yoff) {
    int i, j;
    int16_t intermediate[64];
    int16_t inptr[64];
    int16_t outptr[64];
    int16_t *in = inptr;
    int16_t *output_1 = outptr;
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            inptr[i + j * 8] = rsGetElementAt_short(input, xoff + i, yoff + j);
            outptr[i + j * 8] = rsGetElementAt_short(output, xoff + i, yoff + j);
        }
    }

    // Transform columns
    {
        int16_t *output1 = intermediate;
        /*canbe16*/ int s0, s1, s2, s3, s4, s5, s6, s7;
        /*needs32*/ int t0, t1, t2, t3;
        /*canbe16*/ int x0, x1, x2, x3;

        int i;
        for (i = 0; i < 8; i++) {
            // stage 1
            s0 = (in[0 * 8] + in[7 * 8]) * 4;
            s1 = (in[1 * 8] + in[6 * 8]) * 4;
            s2 = (in[2 * 8] + in[5 * 8]) * 4;
            s3 = (in[3 * 8] + in[4 * 8]) * 4;
            s4 = (in[3 * 8] - in[4 * 8]) * 4;
            s5 = (in[2 * 8] - in[5 * 8]) * 4;
            s6 = (in[1 * 8] - in[6 * 8]) * 4;
            s7 = (in[0 * 8] - in[7 * 8]) * 4;

            // fdct4(step, step);
            x0 = s0 + s3;
            x1 = s1 + s2;
            x2 = s1 - s2;
            x3 = s0 - s3;
            t0 = (x0 + x1) * cospi_16_64;
            t1 = (x0 - x1) * cospi_16_64;
            t2 = x2 * cospi_24_64 + x3 * cospi_8_64;
            t3 = -x2 * cospi_8_64 + x3 * cospi_24_64;
            output1[0 * 8] = fdct_round_shift(t0);
            output1[2 * 8] = fdct_round_shift(t2);
            output1[4 * 8] = fdct_round_shift(t1);
            output1[6 * 8] = fdct_round_shift(t3);

            // Stage 2
            t0 = (s6 - s5) * cospi_16_64;
            t1 = (s6 + s5) * cospi_16_64;
            t2 = fdct_round_shift(t0);
            t3 = fdct_round_shift(t1);

            // Stage 3
            x0 = s4 + t2;
            x1 = s4 - t2;
            x2 = s7 - t3;
            x3 = s7 + t3;

            // Stage 4
            t0 = x0 * cospi_28_64 + x3 * cospi_4_64;
            t1 = x1 * cospi_12_64 + x2 * cospi_20_64;
            t2 = x2 * cospi_12_64 + x1 * -cospi_20_64;
            t3 = x3 * cospi_28_64 + x0 * -cospi_4_64;
            output1[1 * 8] = fdct_round_shift(t0);
            output1[3 * 8] = fdct_round_shift(t2);
            output1[5 * 8] = fdct_round_shift(t1);
            output1[7 * 8] = fdct_round_shift(t3);
            in++;
            output1++;
        }
    }

    // Rows
    for (i = 0; i < 8; ++i) {
        fdct8(&intermediate[i * 8], &output_1[i * 8]);
    }

    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            rsSetElementAt_short(output, output_1[i + j * 8] / 2, xoff + i, yoff + j);
        }
    }

}

extern void rsDct16x16(rs_allocation input, rs_allocation output, int xoff, int yoff) {
    // The 2D transform is done with two passes which are actually pretty
    // similar. In the first one, we transform the columns and transpose
    // the results. In the second one, we transform the rows. To achieve that,
    // as the first pass results are transposed, we tranpose the columns (that
    // is the transposed rows) and transpose the results (so that it goes back
    // in normal/row positions).
    int pass;
    int i, j;
    // We need an intermediate buffer between passes.
    int16_t intermediate[256];
    int16_t *out = intermediate;
    int16_t inptr[256];
    int16_t outptr[256];
    int16_t *in = inptr;
    int16_t *output_1 = outptr;
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 16; j++) {
            inptr[i + j * 16] = rsGetElementAt_short(input, xoff + i, yoff + j);
            outptr[i + j * 16] = rsGetElementAt_short(output, xoff + i, yoff + j);
        }
    }

    // Do the two transform/transpose passes
    for (pass = 0; pass < 2; ++pass) {
        /*canbe16*/ int step1[8];
        /*canbe16*/ int step2[8];
        /*canbe16*/ int step3[8];
        /*canbe16*/ int input[8];
        /*needs32*/ int temp1, temp2;
        int i;
        for (i = 0; i < 16; i++) {
            if (0 == pass) {
                // Calculate input for the first 8 results.
                input[0] = (in[0 * 16] + in[15 * 16]) * 4;
                input[1] = (in[1 * 16] + in[14 * 16]) * 4;
                input[2] = (in[2 * 16] + in[13 * 16]) * 4;
                input[3] = (in[3 * 16] + in[12 * 16]) * 4;
                input[4] = (in[4 * 16] + in[11 * 16]) * 4;
                input[5] = (in[5 * 16] + in[10 * 16]) * 4;
                input[6] = (in[6 * 16] + in[ 9 * 16]) * 4;
                input[7] = (in[7 * 16] + in[ 8 * 16]) * 4;
                // Calculate input for the next 8 results.
                step1[0] = (in[7 * 16] - in[ 8 * 16]) * 4;
                step1[1] = (in[6 * 16] - in[ 9 * 16]) * 4;
                step1[2] = (in[5 * 16] - in[10 * 16]) * 4;
                step1[3] = (in[4 * 16] - in[11 * 16]) * 4;
                step1[4] = (in[3 * 16] - in[12 * 16]) * 4;
                step1[5] = (in[2 * 16] - in[13 * 16]) * 4;
                step1[6] = (in[1 * 16] - in[14 * 16]) * 4;
                step1[7] = (in[0 * 16] - in[15 * 16]) * 4;
            } else {
                // Calculate input for the first 8 results.
                input[0] = ((in[0 * 16] + 1) >> 2) + ((in[15 * 16] + 1) >> 2);
                input[1] = ((in[1 * 16] + 1) >> 2) + ((in[14 * 16] + 1) >> 2);
                input[2] = ((in[2 * 16] + 1) >> 2) + ((in[13 * 16] + 1) >> 2);
                input[3] = ((in[3 * 16] + 1) >> 2) + ((in[12 * 16] + 1) >> 2);
                input[4] = ((in[4 * 16] + 1) >> 2) + ((in[11 * 16] + 1) >> 2);
                input[5] = ((in[5 * 16] + 1) >> 2) + ((in[10 * 16] + 1) >> 2);
                input[6] = ((in[6 * 16] + 1) >> 2) + ((in[ 9 * 16] + 1) >> 2);
                input[7] = ((in[7 * 16] + 1) >> 2) + ((in[ 8 * 16] + 1) >> 2);
                // Calculate input for the next 8 results.
                step1[0] = ((in[7 * 16] + 1) >> 2) - ((in[ 8 * 16] + 1) >> 2);
                step1[1] = ((in[6 * 16] + 1) >> 2) - ((in[ 9 * 16] + 1) >> 2);
                step1[2] = ((in[5 * 16] + 1) >> 2) - ((in[10 * 16] + 1) >> 2);
                step1[3] = ((in[4 * 16] + 1) >> 2) - ((in[11 * 16] + 1) >> 2);
                step1[4] = ((in[3 * 16] + 1) >> 2) - ((in[12 * 16] + 1) >> 2);
                step1[5] = ((in[2 * 16] + 1) >> 2) - ((in[13 * 16] + 1) >> 2);
                step1[6] = ((in[1 * 16] + 1) >> 2) - ((in[14 * 16] + 1) >> 2);
                step1[7] = ((in[0 * 16] + 1) >> 2) - ((in[15 * 16] + 1) >> 2);
            }
            // Work on the first eight values; fdct8(input, even_results);
            {
                /*canbe16*/ int s0, s1, s2, s3, s4, s5, s6, s7;
                /*needs32*/ int t0, t1, t2, t3;
                /*canbe16*/ int x0, x1, x2, x3;

                // stage 1
                s0 = input[0] + input[7];
                s1 = input[1] + input[6];
                s2 = input[2] + input[5];
                s3 = input[3] + input[4];
                s4 = input[3] - input[4];
                s5 = input[2] - input[5];
                s6 = input[1] - input[6];
                s7 = input[0] - input[7];

                // fdct4(step, step);
                x0 = s0 + s3;
                x1 = s1 + s2;
                x2 = s1 - s2;
                x3 = s0 - s3;
                t0 = (x0 + x1) * cospi_16_64;
                t1 = (x0 - x1) * cospi_16_64;
                t2 = x3 * cospi_8_64 + x2 * cospi_24_64;
                t3 = x3 * cospi_24_64 - x2 * cospi_8_64;
                out[0] = fdct_round_shift(t0);
                out[4] = fdct_round_shift(t2);
                out[8] = fdct_round_shift(t1);
                out[12] = fdct_round_shift(t3);

                // Stage 2
                t0 = (s6 - s5) * cospi_16_64;
                t1 = (s6 + s5) * cospi_16_64;
                t2 = fdct_round_shift(t0);
                t3 = fdct_round_shift(t1);

                // Stage 3
                x0 = s4 + t2;
                x1 = s4 - t2;
                x2 = s7 - t3;
                x3 = s7 + t3;

                // Stage 4
                t0 = x0 * cospi_28_64 + x3 * cospi_4_64;
                t1 = x1 * cospi_12_64 + x2 * cospi_20_64;
                t2 = x2 * cospi_12_64 + x1 * -cospi_20_64;
                t3 = x3 * cospi_28_64 + x0 * -cospi_4_64;
                out[2] = fdct_round_shift(t0);
                out[6] = fdct_round_shift(t2);
                out[10] = fdct_round_shift(t1);
                out[14] = fdct_round_shift(t3);
            }
            // Work on the next eight values; step1 -> odd_results
            {
                // step 2
                temp1 = (step1[5] - step1[2]) * cospi_16_64;
                temp2 = (step1[4] - step1[3]) * cospi_16_64;
                step2[2] = fdct_round_shift(temp1);
                step2[3] = fdct_round_shift(temp2);
                temp1 = (step1[4] + step1[3]) * cospi_16_64;
                temp2 = (step1[5] + step1[2]) * cospi_16_64;
                step2[4] = fdct_round_shift(temp1);
                step2[5] = fdct_round_shift(temp2);
                // step 3
                step3[0] = step1[0] + step2[3];
                step3[1] = step1[1] + step2[2];
                step3[2] = step1[1] - step2[2];
                step3[3] = step1[0] - step2[3];
                step3[4] = step1[7] - step2[4];
                step3[5] = step1[6] - step2[5];
                step3[6] = step1[6] + step2[5];
                step3[7] = step1[7] + step2[4];
                // step 4
                temp1 = step3[1] * -cospi_8_64 + step3[6] * cospi_24_64;
                temp2 = step3[2] * -cospi_24_64 - step3[5] * cospi_8_64;
                step2[1] = fdct_round_shift(temp1);
                step2[2] = fdct_round_shift(temp2);
                temp1 = step3[2] * -cospi_8_64 + step3[5] * cospi_24_64;
                temp2 = step3[1] * cospi_24_64 + step3[6] * cospi_8_64;
                step2[5] = fdct_round_shift(temp1);
                step2[6] = fdct_round_shift(temp2);
                // step 5
                step1[0] = step3[0] + step2[1];
                step1[1] = step3[0] - step2[1];
                step1[2] = step3[3] - step2[2];
                step1[3] = step3[3] + step2[2];
                step1[4] = step3[4] + step2[5];
                step1[5] = step3[4] - step2[5];
                step1[6] = step3[7] - step2[6];
                step1[7] = step3[7] + step2[6];
                // step 6
                temp1 = step1[0] * cospi_30_64 + step1[7] * cospi_2_64;
                temp2 = step1[1] * cospi_14_64 + step1[6] * cospi_18_64;
                out[1] = fdct_round_shift(temp1);
                out[9] = fdct_round_shift(temp2);
                temp1 = step1[2] * cospi_22_64 + step1[5] * cospi_10_64;
                temp2 = step1[3] * cospi_6_64 + step1[4] * cospi_26_64;
                out[5] = fdct_round_shift(temp1);
                out[13] = fdct_round_shift(temp2);
                temp1 = step1[3] * -cospi_26_64 + step1[4] * cospi_6_64;
                temp2 = step1[2] * -cospi_10_64 + step1[5] * cospi_22_64;
                out[3] = fdct_round_shift(temp1);
                out[11] = fdct_round_shift(temp2);
                temp1 = step1[1] * -cospi_18_64 + step1[6] * cospi_14_64;
                temp2 = step1[0] * -cospi_2_64 + step1[7] * cospi_30_64;
                out[7] = fdct_round_shift(temp1);
                out[15] = fdct_round_shift(temp2);
            }
            // Do next column (which is a transposed row in second/horizontal pass)
            in++;
            out += 16;
        }
        // Setup in/out for next pass.
        in = intermediate;
        out = output_1;
    }

    for (i = 0; i < 16; i++) {
        for (j = 0; j < 16; j++) {
            rsSetElementAt_short(output, output_1[i + j * 16], xoff + i, yoff + j);
        }
    }
}

extern void rsDct32x32(rs_allocation input, rs_allocation out, int xoff, int yoff) {
    int i, j;
    int output[32 * 32];
    int16_t inptr[1024];
    int16_t outptr[1024];
    int16_t *in = inptr;
    int16_t *output_1 = outptr;
    for (i = 0; i < 32; i++) {
        for (j = 0; j < 32; j++) {
            inptr[i + j * 32] = rsGetElementAt_short(input, xoff + i, yoff + j);
            outptr[i + j * 32] = rsGetElementAt_short(out, xoff + i, yoff + j);
        }
    }

    // Columns
    for (i = 0; i < 32; ++i) {
        int temp_in[32], temp_out[32];
        for (j = 0; j < 32; ++j)
            temp_in[j] = in[j * 32 + i] * 4;
        dct32_1d(temp_in, temp_out, 0);
        for (j = 0; j < 32; ++j)
            output[j * 32 + i] = (temp_out[j] + 1 + (temp_out[j] > 0)) >> 2;
    }

    // Rows
    for (i = 0; i < 32; ++i) {
        int temp_in[32], temp_out[32];
        for (j = 0; j < 32; ++j)
            temp_in[j] = output[j + i * 32];
        dct32_1d(temp_in, temp_out, 0);
        for (j = 0; j < 32; ++j)
            output_1[j + i * 32] = (temp_out[j] + 1 + (temp_out[j] < 0)) >> 2;
    }

    for (i = 0; i < 32; i++) {
        for (j = 0; j < 32; j++) {
            rsSetElementAt_short(out, output_1[i + j * 32], xoff + i, yoff + j);
        }
    }
}

// Note that although we use dct_32_round in dct32_1d computation flow,
// this 2d fdct32x32 for rate-distortion optimization loop is operating
// within 16 bits precision.
extern void rsDct32x32_rd(rs_allocation input, rs_allocation out, int xoff, int yoff) {
    int i, j;
    int output[32 * 32];
    int16_t inptr[1024];
    int16_t outptr[1024];
    int16_t *in = inptr;
    int16_t *output_1 = outptr;
    for (i = 0; i < 32; i++) {
        for (j = 0; j < 32; j++) {
            inptr[i + j * 32] = rsGetElementAt_short(input, xoff + i, yoff + j);
            outptr[i + j * 32] = rsGetElementAt_short(out, xoff + i, yoff + j);
        }
    }

    // Columns
    for (i = 0; i < 32; ++i) {
        int temp_in[32], temp_out[32];
        for (j = 0; j < 32; ++j)
            temp_in[j] = in[j * 32 + i] * 4;
        dct32_1d(temp_in, temp_out, 0);
        for (j = 0; j < 32; ++j)
            // TODO(cd): see quality impact of only doing
            //           output[j * 32 + i] = (temp_out[j] + 1) >> 2;
            //           PS: also change code in vp9/encoder/x86/vp9_dct_sse2.c
            output[j * 32 + i] = (temp_out[j] + 1 + (temp_out[j] > 0)) >> 2;
    }

    // Rows
    for (i = 0; i < 32; ++i) {
        int temp_in[32], temp_out[32];
        for (j = 0; j < 32; ++j)
            temp_in[j] = output[j + i * 32];
        dct32_1d(temp_in, temp_out, 1);
        for (j = 0; j < 32; ++j)
            output_1[j + i * 32] = temp_out[j];
    }

    for (i = 0; i < 32; i++) {
        for (j = 0; j < 32; j++) {
            rsSetElementAt_short(out, output_1[i + j * 32], xoff + i, yoff + j);
        }
    }
}
