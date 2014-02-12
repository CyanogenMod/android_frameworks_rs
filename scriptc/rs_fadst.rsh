#ifndef __RS_FADST_RSH__
#define __RS_FADST_RSH__

#if RS_VERSION > 19

extern void fadst4(const rs_allocation input, rs_allocation output, int32_t xoff);

extern void fadst8(const rs_allocation input, rs_allocation output, int32_t xoff);

extern void fadst16(const rs_allocation input, rs_allocation output, int32_t xoff);

#endif

#endif