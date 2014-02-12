#ifndef __RS_WALSH_RSH__
#define __RS_WALSH_RSH__

#if RS_VERSION > 19

extern void walsh4x4(rs_allocation input, rs_allocation dest, int xoff, int yoff);
extern void walsh4x4_1(rs_allocation input, rs_allocation dest, int xoff, int yoff);

#endif

#endif
