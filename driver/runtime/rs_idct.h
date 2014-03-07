#ifndef _RS_IDCT_H_
#define _RS_IDCT_H_

#include "rs_types.rsh"

#define DCT_CONST_BITS 14

// Constants:
//  for (int i = 1; i< 32; ++i)
//      printf("static const int cospi_%d_64 = %.0f;\n", i,
//           round(16384 * cos(i*M_PI/64)));
// Note: sin(k*Pi/64) = cos((32-k)*Pi/64)
static const int cospi_1_64  = 16364;
static const int cospi_2_64  = 16305;
static const int cospi_3_64  = 16207;
static const int cospi_4_64  = 16069;
static const int cospi_5_64  = 15893;
static const int cospi_6_64  = 15679;
static const int cospi_7_64  = 15426;
static const int cospi_8_64  = 15137;
static const int cospi_9_64  = 14811;
static const int cospi_10_64 = 14449;
static const int cospi_11_64 = 14053;
static const int cospi_12_64 = 13623;
static const int cospi_13_64 = 13160;
static const int cospi_14_64 = 12665;
static const int cospi_15_64 = 12140;
static const int cospi_16_64 = 11585;
static const int cospi_17_64 = 11003;
static const int cospi_18_64 = 10394;
static const int cospi_19_64 = 9760;
static const int cospi_20_64 = 9102;
static const int cospi_21_64 = 8423;
static const int cospi_22_64 = 7723;
static const int cospi_23_64 = 7005;
static const int cospi_24_64 = 6270;
static const int cospi_25_64 = 5520;
static const int cospi_26_64 = 4756;
static const int cospi_27_64 = 3981;
static const int cospi_28_64 = 3196;
static const int cospi_29_64 = 2404;
static const int cospi_30_64 = 1606;
static const int cospi_31_64 = 804;

#define ROUND_POWER_OF_TWO(value, n) \
    (((value) + (1 << ((n) - 1))) >> (n))



static uint8_t clip_pixel(int val) {
    return (val > 255) ? 255u : (val < 0) ? 0u : val;
}

static int dct_const_round_shift(int input) {
    int rv = ROUND_POWER_OF_TWO(input, DCT_CONST_BITS);
    return (int16_t)rv;
}
#endif
