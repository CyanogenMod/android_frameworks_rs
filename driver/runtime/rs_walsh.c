#include "rs_types.rsh"
#include "rs_allocation.rsh"

extern void walsh4x4(rs_allocation input, rs_allocation dest, int xoff, int yoff) {
    short output[16];
    int i, j;
    int a1, b1, c1, d1;
    int a2, b2, c2, d2;
    short *op = output;
    int16_t inptr[16];
    int16_t *in = inptr;
    for (i = 0; i < 16; i++) {
        inptr[i] = rsGetElementAt_short(input, xoff + i, yoff);
    }

    for (i = 0; i < 4; i++) {
        a1 = in[0] + in[12];
        b1 = in[4] + in[8];
        c1 = in[4] - in[8];
        d1 = in[0] - in[12];

        op[0] = a1 + b1;
        op[4] = c1 + d1;
        op[8] = a1 - b1;
        op[12] = d1 - c1;
        in++;
        op++;
    }

    in = output;
    op = output;

    for (i = 0; i < 4; i++) {
        a1 = in[0] + in[3];
        b1 = in[1] + in[2];
        c1 = in[1] - in[2];
        d1 = in[0] - in[3];

        a2 = a1 + b1;
        b2 = c1 + d1;
        c2 = a1 - b1;
        d2 = d1 - c1;

        op[0] = (a2 + 3) >> 3;
        op[1] = (b2 + 3) >> 3;
        op[2] = (c2 + 3) >> 3;
        op[3] = (d2 + 3) >> 3;

        in += 4;
        op += 4;
    }

    for (i = 0; i < 16; i++) {
        rsSetElementAt_short(dest, output[i], xoff, yoff + i);
    }

}

extern void walsh4x4_1(rs_allocation input, rs_allocation dest, int xoff, int yoff) {
    int i, j;
    int a1;
    int16_t inptr[16];
    int16_t *in = inptr;
    for (i = 0; i < 16; i++) {
        inptr[i] = rsGetElementAt_short(input, xoff + i, yoff);
    }
    a1 = ((in[0] + 3) >> 3);
    for (i = 0; i < 16; i++) {
        rsSetElementAt_short(dest, a1, xoff, yoff + i);
    }
}
