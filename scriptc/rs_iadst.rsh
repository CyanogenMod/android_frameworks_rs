#ifndef __RS_IADST_RSH__
#define __RS_IADST_RSH__

#if RS_VERSION > 19

extern void iadst4(const rs_allocation input, rs_allocation output, int32_t xoff);

extern void iadst8(const rs_allocation input, rs_allocation output, int32_t xoff);

extern void iadst16(const rs_allocation input, rs_allocation output, int32_t xoff);

#endif

#endif