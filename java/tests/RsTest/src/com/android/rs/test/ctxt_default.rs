#include "shared.rsh"

rs_allocation A;
rs_allocation B;
uint32_t gDimX;
static bool failed = false;

void init_vars(int *out) {
    *out = 7;
}

int RS_KERNEL root(int ain, rs_kernel_context ctxt, uint32_t x) {
    _RS_ASSERT_EQU(rsGetArray0(ctxt), 0);
    _RS_ASSERT_EQU(rsGetArray1(ctxt), 0);
    _RS_ASSERT_EQU(rsGetArray2(ctxt), 0);
    _RS_ASSERT_EQU(rsGetArray3(ctxt), 0);
    _RS_ASSERT_EQU(rsGetFace(ctxt), RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X);
    _RS_ASSERT_EQU(rsGetLod(ctxt), 0);

    _RS_ASSERT_EQU(rsGetDimY(ctxt), 0);
    _RS_ASSERT_EQU(rsGetDimZ(ctxt), 0);
    _RS_ASSERT_EQU(rsGetDimArray0(ctxt), 0);
    _RS_ASSERT_EQU(rsGetDimArray1(ctxt), 0);
    _RS_ASSERT_EQU(rsGetDimArray2(ctxt), 0);
    _RS_ASSERT_EQU(rsGetDimArray3(ctxt), 0);
    _RS_ASSERT_EQU(rsGetDimHasFaces(ctxt), false);
    _RS_ASSERT_EQU(rsGetDimLod(ctxt), 0);

    if (!_RS_ASSERT_EQU(ain, 7))
        rsDebug("root at x", x);
    uint32_t dimX = rsGetDimX(ctxt);
    _RS_ASSERT_EQU(dimX, gDimX);
    return ain + x;
}

static bool test_root_output() {
    bool failed = false;
    int i;

    for (i = 0; i < gDimX; i++) {
        int bElt = rsGetElementAt_int(B, i);
        int aElt = rsGetElementAt_int(A, i);
        if (!_RS_ASSERT_EQU(bElt, (aElt + i)))
            rsDebug("test_root_output at i", i);
    }

    if (failed) {
        rsDebug("ctxt_default test_root_output FAILED", 0);
    }
    else {
        rsDebug("ctxt_default test_root_output PASSED", 0);
    }

    return failed;
}

void verify_root() {
    failed |= test_root_output();
}

void kernel_test() {
    if (failed) {
        rsSendToClientBlocking(RS_MSG_TEST_FAILED);
    }
    else {
        rsSendToClientBlocking(RS_MSG_TEST_PASSED);
    }
}
