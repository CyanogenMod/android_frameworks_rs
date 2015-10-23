#include "shared.rsh"

int dimX;
int dimY;

int __attribute__((kernel)) foo(int a) {
    return a * 2;
}

static void validate(rs_allocation out) {
    bool failed = false;

    int i, j;

    for (j = 0; j < dimY; j++) {
        for (i = 0; i < dimX; i++) {
            const int actual = rsGetElementAt_int(out, i, j);
            const int expected = (i + j * dimX) * 4;
            if (actual != expected) {
                failed = true;
                rsDebug("row     ", j);
                rsDebug("column  ", i);
                rsDebug("expects ", expected);
                rsDebug("got     ", actual);
            }
        }
    }

    if (failed) {
        rsDebug("FAILED", 0);
    } else {
        rsDebug("PASSED", 0);
    }

    if (failed) {
        rsSendToClientBlocking(RS_MSG_TEST_FAILED);
    } else {
        rsSendToClientBlocking(RS_MSG_TEST_PASSED);
    }
}

void entrypoint(rs_allocation in, rs_allocation out) {
    int i, j;
    for (i = 0; i < dimX; i++) {
        for (j = 0; j < dimY; j++) {
            rsSetElementAt_int(in, j * dimX + i, i, j);
        }
    }

    rsParallelFor(foo, in, out);
    rsParallelFor(foo, out, out);

    validate(out);
}
