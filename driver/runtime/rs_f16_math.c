/* Implementations for copysign, ilogb, and nextafter for float16 based on
 * corresponding float32 implementations in
 * bionic/libm/upstream-freebsd/lib/msun/src
 */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#include "rs_core.rsh"
#include "rs_f16_util.h"

// Based on bionic/libm/upstream-freebsd/lib/msun/src/s_copysignf.c
extern half __attribute__((overloadable)) copysign(half x, half y) {
    short hx, hy;
    GET_HALF_WORD(hx, x);
    GET_HALF_WORD(hy, y);

    SET_HALF_WORD(hx, (hx & 0x7fff) | (hy & 0x8000));
    return x;
}

// Based on bionic/libm/upstream-freebsd/lib/msun/src/s_ilogbf.c
extern int __attribute__((overloadable)) ilogb(half x) {
    const int RS_INT_MAX = 0x7fffffff;
    const int RS_INT_MIN = 0x80000000;

    short hx, ix;
    GET_HALF_WORD(hx, x);
    hx &= 0x7fff;

    if (hx < 0x0400) { // subnormal
        if (hx == 0)
            return RS_INT_MIN; // for zero
        for (hx <<= 5, ix = -14; hx > 0; hx <<= 1)
            ix -= 1;
        return ix;
    }
    else if (hx < 0x7c00) {
        return (hx >> 10) - 15;
    }
    else { // hx >= 0x7c00
        return RS_INT_MAX; // for NaN and infinity
    }
}

// Based on bionic/libm/upstream-freebsd/lib/msun/src/s_nextafterf.c
extern half __attribute__((overloadable)) nextafter(half x, half y) {
  volatile half t;
  short hx, hy, ix, iy;

  GET_HALF_WORD(hx, x);
  GET_HALF_WORD(hy, y);
  ix = hx & 0x7fff; // |x|
  iy = hy & 0x7fff; // |y|

  if ((ix > 0x7c00) || // x is nan
      (iy > 0x7c00))   // y is nan
    return x + y;      // return nan

  if (x == y) return y; // x == y.  return y
  if (ix == 0) {
    SET_HALF_WORD(x, (hy & 0x8000) | 1);
    return x;
  }

  if (hx >= 0) {  // x >= 0
    if (hx > hy)
      hx -= 1;    // x > y, x -= 1 ulp
    else
      hx += 1;    // x < y, x += 1 ulp
  }
  else {          // x < 0
    if (hy>= 0 || hx > hy)
      hx -= 1;    // x < y, x -= 1 ulp
    else
      hx += 1;
  }

  SET_HALF_WORD(x, hx);
  return x;
}
