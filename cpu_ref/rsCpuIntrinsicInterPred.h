/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RSD_CPU_SCRIPT_INTRINSIC_INTER_PRED_H
#define RSD_CPU_SCRIPT_INTRINSIC_INTER_PRED_H

#include "rsCpuIntrinsic.h"
#include "rsCpuIntrinsicInlines.h"
#include "rsCpuConvolve.h"

using namespace android;
using namespace android::renderscript;

namespace android {
namespace renderscript {

typedef struct inter_pred_param {
    int pred_mode;

    int src_mv;
    int src_stride;
    int dst_mv;
    int dst_stride;

    int filter_x_mv;
    int x_step_q4;
    int filter_y_mv;
    int y_step_q4;

    int w;
    int h;
}INTER_PRED_PARAM;

static const int16_t inter_pred_filters[512] = {
    0, 0, 0, 128, 0, 0, 0, 0, 0, 1, -5, 126, 8, -3, 1, 0,
    -1, 3, -10, 122, 18, -6, 2, 0, -1, 4, -13, 118, 27, -9, 3, -1,
    -1, 4, -16, 112, 37, -11, 4, -1, -1, 5, -18, 105, 48, -14, 4, -1,
    -1, 5, -19, 97, 58, -16, 5, -1, -1, 6, -19, 88, 68, -18, 5, -1,
    -1, 6, -19, 78, 78, -19, 6, -1, -1, 5, -18, 68, 88, -19, 6, -1,
    -1, 5, -16, 58, 97, -19, 5, -1, -1, 4, -14, 48, 105, -18, 5, -1,
    -1, 4, -11, 37, 112, -16, 4, -1, -1, 3, -9, 27, 118, -13, 4, -1,
    0, 2, -6, 18, 122, -10, 3, -1, 0, 1, -3, 8, 126, -5, 1, 0,
    0, 0, 0, 128, 0, 0, 0, 0, -3, -1, 32, 64, 38, 1, -3, 0,
    -2, -2, 29, 63, 41, 2, -3, 0, -2, -2, 26, 63, 43, 4, -4, 0,
    -2, -3, 24, 62, 46, 5, -4, 0, -2, -3, 21, 60, 49, 7, -4, 0,
    -1, -4, 18, 59, 51, 9, -4, 0, -1, -4, 16, 57, 53, 12, -4, -1,
    -1, -4, 14, 55, 55, 14, -4, -1, -1, -4, 12, 53, 57, 16, -4, -1,
    0, -4, 9, 51, 59, 18, -4, -1, 0, -4, 7, 49, 60, 21, -3, -2,
    0, -4, 5, 46, 62, 24, -3, -2, 0, -4, 4, 43, 63, 26, -2, -2,
    0, -3, 2, 41, 63, 29, -2, -2, 0, -3, 1, 38, 64, 32, -1, -3,
    0, 0, 0, 128, 0, 0, 0, 0, -1, 3, -7, 127, 8, -3, 1, 0,
    -2, 5, -13, 125, 17, -6, 3, -1, -3, 7, -17, 121, 27, -10, 5, -2,
    -4, 9, -20, 115, 37, -13, 6, -2, -4, 10, -23, 108, 48, -16, 8, -3,
    -4, 10, -24, 100, 59, -19, 9, -3, -4, 11, -24, 90, 70, -21, 10, -4,
    -4, 11, -23, 80, 80, -23, 11, -4, -4, 10, -21, 70, 90, -24, 11, -4,
    -3, 9, -19, 59, 100, -24, 10, -4, -3, 8, -16, 48, 108, -23, 10, -4,
    -2, 6, -13, 37, 115, -20, 9, -4, -2, 5, -10, 27, 121, -17, 7, -3,
    -1, 3, -6, 17, 125, -13, 5, -2, 0, 1, -3, 8, 127, -7, 3, -1,
    0, 0, 0, 128, 0, 0, 0, 0, 0, 0, 0, 120, 8, 0, 0, 0,
    0, 0, 0, 112, 16, 0, 0, 0, 0, 0, 0, 104, 24, 0, 0, 0,
    0, 0, 0, 96, 32, 0, 0, 0, 0, 0, 0, 88, 40, 0, 0, 0,
    0, 0, 0, 80, 48, 0, 0, 0, 0, 0, 0, 72, 56, 0, 0, 0,
    0, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 56, 72, 0, 0, 0,
    0, 0, 0, 48, 80, 0, 0, 0, 0, 0, 0, 40, 88, 0, 0, 0,
    0, 0, 0, 32, 96, 0, 0, 0, 0, 0, 0, 24, 104, 0, 0, 0,
    0, 0, 0, 16, 112, 0, 0, 0, 0, 0, 0, 8, 120, 0, 0, 0
};

typedef void (*convolve_fn_t)(const uint8_t *src, ptrdiff_t src_stride,
                              uint8_t *dst, ptrdiff_t dst_stride,
                              const int16_t *filter_x, int x_step_q4,
                              const int16_t *filter_y, int y_step_q4,
                              int w, int h);


class RsdCpuScriptIntrinsicInterPred: public RsdCpuScriptIntrinsic {
public:
    virtual void populateScript(Script *);
    virtual void invokeFreeChildren();

    virtual void setGlobalObj(uint32_t slot, ObjectBase *data);
    virtual void setGlobalVar(uint32_t slot, const void *data, size_t dataLength);
    virtual ~RsdCpuScriptIntrinsicInterPred();
    RsdCpuScriptIntrinsicInterPred(RsdCpuReferenceImpl *ctx, const Script *s, const Element *e);

protected:
    uint8_t *mRef;
    uint8_t *mParam;
    int mFriParamCount;
    int mSecParamCount;
    int mParamOffset;
    int mCount;
    convolve_fn_t mSwitchConvolve[32];
    static void kernel(const RsForEachStubParamStruct *p,
                       uint32_t xstart, uint32_t xend,
                       uint32_t instep, uint32_t outstep);
};

}
}
#endif

