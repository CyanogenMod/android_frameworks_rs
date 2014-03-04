/*
 * Copyright (C) 2013 The Android Open Source Project
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


#include "rsCpuIntrinsic.h"
#include "rsCpuIntrinsicInlines.h"
#include <sys/syscall.h>
#include "cutils/atomic.h"

#ifdef RS_COMPATIBILITY_LIB
#include "rsCompatibilityLib.h"
#endif

#ifndef RS_COMPATIBILITY_LIB
#include "hardware/gralloc.h"
#endif


#define INLINE      inline

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

#define ROUND_POWER_OF_TWO(value, n) \
    (((value) + (1 << ((n) - 1))) >> (n))


#define MI_SIZE_LOG2 3
#define MI_BLOCK_SIZE_LOG2 (6 - MI_SIZE_LOG2)  // 64 = 2^6

#define MI_SIZE (1 << MI_SIZE_LOG2)  // pixels per mi-unit
#define MI_BLOCK_SIZE (1 << MI_BLOCK_SIZE_LOG2)  // mi-units per max block

#define MI_MASK (MI_BLOCK_SIZE - 1)

#define SIMD_WIDTH          16
#define MAX_LOOP_FILTER     63
#define MAX_SEGMENTS        8
#define MAX_REF_FRAMES      4
#define MAX_MODE_LF_DELTAS  2
#define MB_MODE_COUNT       14
#define BLOCK_SIZES         13


#if (defined(__GNUC__) && __GNUC__) || defined(__SUNPRO_C)
#define DECLARE_ALIGNED(n,typ,val)  typ val __attribute__ ((aligned (n)))
#elif defined(_MSC_VER)
#define DECLARE_ALIGNED(n,typ,val)  __declspec(align(n)) typ val
#else
#warning No alignment directives known for this compiler.
#define DECLARE_ALIGNED(n,typ,val)  typ val
#endif

// block transform size
typedef enum {
    TX_4X4 = 0,                      // 4x4 transform
    TX_8X8 = 1,                      // 8x8 transform
    TX_16X16 = 2,                    // 16x16 transform
    TX_32X32 = 3,                    // 32x32 transform
    TX_SIZES
} TX_SIZE;

typedef enum {
    PLANE_TYPE_Y_WITH_DC,
    PLANE_TYPE_UV,
} PLANE_TYPE;

// This structure holds bit masks for all 8x8 blocks in a 64x64 region.
// Each 1 bit represents a position in which we want to apply the loop filter.
// Left_ entries refer to whether we apply a filter on the border to the
// left of the block.   Above_ entries refer to whether or not to apply a
// filter on the above border.   Int_ entries refer to whether or not to
// apply borders on the 4x4 edges within the 8x8 block that each bit
// represents.
// Since each transform is accompanied by a potentially different type of
// loop filter there is a different entry in the array for each transform size.
struct LoopFilterMask {
    uint64_t left_y[4];
    uint64_t above_y[4];
    uint64_t int_4x4_y;
    unsigned short left_uv[4];
    unsigned short above_uv[4];
    unsigned short int_4x4_uv;
    unsigned char lfl_y[64];
    unsigned char lfl_uv[16];
};

// Need to align this structure so when it is declared and
// passed it can be loaded into vector registers.
struct LoopFilterThresh {
    DECLARE_ALIGNED(SIMD_WIDTH, uint8_t, mblim[SIMD_WIDTH]);
    DECLARE_ALIGNED(SIMD_WIDTH, uint8_t, lim[SIMD_WIDTH]);
    DECLARE_ALIGNED(SIMD_WIDTH, uint8_t, hev_thr[SIMD_WIDTH]);
};

struct LoopFilterInfoN {
    LoopFilterThresh lfthr[MAX_LOOP_FILTER + 1];
    uint8_t lvl[MAX_SEGMENTS][MAX_REF_FRAMES][MAX_MODE_LF_DELTAS];
    uint8_t mode_lf_lut[MB_MODE_COUNT];
};

struct BufferInfo {
    int y_offset;
    int u_offset;
    int v_offset;
    int y_stride;
    int uv_stride;
};

#define MAX_CPU_CORES   32
#define MAX_MB_PLANE    3
#define MAX_SB_COL      32

struct LoopFilterProgressChart {
    int start;
    int stop;
    int num_planes;
    int mi_rows;
    int mi_cols;
    BufferInfo        buf_info;
    uint8_t           *buffer_alloc;
    LoopFilterInfoN   *lf_info;
    LoopFilterMask    *lfms;

    int               wid;
    int               quit;
    int               doing;
    volatile int32_t  chart[MAX_SB_COL];
    int32_t           sb_row_pro;
    pthread_t         *tid;
    pthread_mutex_t   *mutex;
    pthread_cond_t    *start_cond;
    pthread_mutex_t   *hmutex;
    pthread_cond_t    *finish;
};

using namespace android;
using namespace android::renderscript;

namespace android {
namespace renderscript {


class RsdCpuScriptIntrinsicLoopFilter : public RsdCpuScriptIntrinsic {
private:
    LoopFilterProgressChart   mPrch;
    int                mWorkerCount;

public:
    virtual void populateScript(Script *);
    virtual void setGlobalVar(uint32_t slot, const void *data, size_t dataLength);
    virtual void setGlobalObj(uint32_t slot, ObjectBase *data);

    virtual ~RsdCpuScriptIntrinsicLoopFilter();
    RsdCpuScriptIntrinsicLoopFilter(RsdCpuReferenceImpl *ctx, const Script *s,
                                    const Element *e);

protected:
    ObjectBaseRef<Allocation> mLfInfo;
    ObjectBaseRef<Allocation> mLfMasks;
    ObjectBaseRef<Allocation> mFrameBuffer;

    void doLoopFilter();
    static void kernel(const RsForEachStubParamStruct *p,
                       uint32_t xstart, uint32_t xend,
                       uint32_t instep, uint32_t outstep);
};

}
}

void RsdCpuScriptIntrinsicLoopFilter::kernel(const RsForEachStubParamStruct *p,
                                             uint32_t xstart, uint32_t xend,
                                             uint32_t instep, uint32_t outstep) {
    RsdCpuScriptIntrinsicLoopFilter *cp = (RsdCpuScriptIntrinsicLoopFilter*)p->usr;
    memset((void*)&cp->mPrch.chart, 0, sizeof(cp->mPrch.chart));
    cp->mPrch.chart[0] = 0x0fffffff;
    cp->mPrch.sb_row_pro = 0;
    cp->mPrch.doing = cp->mWorkerCount;

    int i = 0;
    for (i = 0; i < cp->mWorkerCount; ++i) {
        pthread_cond_signal(&cp->mPrch.start_cond[i]);
    }
    pthread_mutex_lock(cp->mPrch.hmutex);
    if (cp->mPrch.doing) {
        pthread_cond_wait(cp->mPrch.finish, cp->mPrch.hmutex);
    }
    pthread_mutex_unlock(cp->mPrch.hmutex);
}


void RsdCpuScriptIntrinsicLoopFilter::setGlobalVar(uint32_t slot,
                                                   const void *data,
                                                   size_t dataLength) {
    rsAssert(slot >= 0 && slot < 2);
    const int *dptr = (const int *)data;
    switch (slot) {
    case 0:
        rsAssert(dataLength == sizeof(int) * 5);
        mPrch.start       = dptr[0];
        mPrch.stop        = dptr[1];
        mPrch.num_planes  = dptr[2];
        mPrch.mi_rows     = dptr[3];
        mPrch.mi_cols     = dptr[4];
        break;
    case 1:
        rsAssert(dataLength == sizeof(BufferInfo));
        mPrch.buf_info = *((BufferInfo*)data);
        break;
    default:
        ALOGE("Non-exist global value slot: %d", slot);
        rsAssert(0);
    }
}

void RsdCpuScriptIntrinsicLoopFilter::setGlobalObj(uint32_t slot, ObjectBase *data) {
    rsAssert(slot > 1 && slot < 5);
    if (slot == 2) {
        mLfInfo.set(static_cast<Allocation *>(data));
        mPrch.lf_info = (LoopFilterInfoN *)mLfInfo->mHal.state.userProvidedPtr;
    } else if (slot == 3) {
        mLfMasks.set(static_cast<Allocation *>(data));
        mPrch.lfms = (LoopFilterMask *)mLfMasks->mHal.state.userProvidedPtr;
    } else {
        mFrameBuffer.set(static_cast<Allocation *>(data));
        mPrch.buffer_alloc = (uint8_t *)mFrameBuffer->mHal.state.userProvidedPtr;
    }
}

RsdCpuScriptIntrinsicLoopFilter::~RsdCpuScriptIntrinsicLoopFilter() {
    android_atomic_inc(&mPrch.quit);
    int i = 0;
    for (i = 0; i < mWorkerCount; ++i) {
        pthread_cond_signal(&mPrch.start_cond[i]);
    }
    for (i = 0; i < mWorkerCount; ++i) {
        pthread_join(mPrch.tid[i], NULL);
    }
    free(mPrch.tid);
}

void RsdCpuScriptIntrinsicLoopFilter::populateScript(Script *s) {
    s->mHal.info.exportedVariableCount = 9;
    s->mHal.info.exportedFunctionCount = 1;
}

RsdCpuScriptImpl * rsdIntrinsic_LoopFilter(RsdCpuReferenceImpl *ctx,
                                           const Script *s, const Element *e) {
    return new RsdCpuScriptIntrinsicLoopFilter(ctx, s, e);
}

extern "C" void vp9_lpf_vertical_16_c(uint8_t *s, int pitch,
                                      const uint8_t *blimit,
                                      const uint8_t *limit,
                                      const uint8_t *thresh);
extern "C" void vp9_lpf_vertical_16_neon(uint8_t *s, int pitch,
                                         const uint8_t *blimit,
                                         const uint8_t *limit,
                                         const uint8_t *thresh);
extern "C" void vp9_lpf_vertical_16_dual_c(uint8_t *s, int pitch,
                                           const uint8_t *blimit,
                                           const uint8_t *limit,
                                           const uint8_t *thresh);
extern "C" void vp9_lpf_vertical_16_dual_neon(uint8_t *s, int pitch,
                                              const uint8_t *blimit,
                                              const uint8_t *limit,
                                              const uint8_t *thresh);
extern "C" void vp9_lpf_vertical_8_c(uint8_t *s, int pitch,
                                     const uint8_t *blimit,
                                     const uint8_t *limit,
                                     const uint8_t *thresh,
                                     int count);
extern "C" void vp9_lpf_vertical_8_neon(uint8_t *s, int pitch,
                                        const uint8_t *blimit,
                                        const uint8_t *limit,
                                        const uint8_t *thresh, int count);
extern "C" void vp9_lpf_vertical_8_dual_c(uint8_t *s, int pitch,
                                          const uint8_t *blimit0,
                                          const uint8_t *limit0,
                                          const uint8_t *thresh0,
                                          const uint8_t *blimit1,
                                          const uint8_t *limit1,
                                          const uint8_t *thresh1);
extern "C" void vp9_lpf_vertical_8_dual_neon(uint8_t *s, int pitch,
                                             const uint8_t *blimit0,
                                             const uint8_t *limit0,
                                             const uint8_t *thresh0,
                                             const uint8_t *blimit1,
                                             const uint8_t *limit1,
                                             const uint8_t *thresh1);
extern "C" void vp9_lpf_vertical_4_c(uint8_t *s, int pitch, const uint8_t *blimit,
                                     const uint8_t *limit, const uint8_t *thresh,
                                     int count);
extern "C" void vp9_lpf_vertical_4_neon(uint8_t *s, int pitch,
                                        const uint8_t *blimit,
                                        const uint8_t *limit,
                                        const uint8_t *thresh, int count);
extern "C" void vp9_lpf_vertical_4_dual_c(uint8_t *s, int pitch,
                                          const uint8_t *blimit0,
                                          const uint8_t *limit0,
                                          const uint8_t *thresh0,
                                          const uint8_t *blimit1,
                                          const uint8_t *limit1,
                                          const uint8_t *thresh1);
extern "C" void vp9_lpf_vertical_4_dual_neon(uint8_t *s, int pitch,
                                             const uint8_t *blimit0,
                                             const uint8_t *limit0,
                                             const uint8_t *thresh0,
                                             const uint8_t *blimit1,
                                             const uint8_t *limit1,
                                             const uint8_t *thresh1);
extern "C" void vp9_lpf_horizontal_16_c(uint8_t *s, int pitch,
                                        const uint8_t *blimit,
                                        const uint8_t *limit,
                                        const uint8_t *thresh, int count);
extern "C" void vp9_lpf_horizontal_16_neon(uint8_t *s, int pitch,
                                           const uint8_t *blimit,
                                           const uint8_t *limit,
                                           const uint8_t *thresh, int count);
extern "C" void vp9_lpf_horizontal_8_c(uint8_t *s, int pitch,
                                       const uint8_t *blimit,
                                       const uint8_t *limit,
                                       const uint8_t *thresh, int count);
extern "C" void vp9_lpf_horizontal_8_neon(uint8_t *s, int pitch,
                                          const uint8_t *blimit,
                                          const uint8_t *limit,
                                          const uint8_t *thresh, int count);
extern "C" void vp9_lpf_horizontal_8_dual_c(uint8_t *s, int pitch,
                                            const uint8_t *blimit0,
                                            const uint8_t *limit0,
                                            const uint8_t *thresh0,
                                            const uint8_t *blimit1,
                                            const uint8_t *limit1,
                                            const uint8_t *thresh1);
extern "C" void vp9_lpf_horizontal_8_dual_neon(uint8_t *s, int pitch,
                                               const uint8_t *blimit0,
                                               const uint8_t *limit0,
                                               const uint8_t *thresh0,
                                               const uint8_t *blimit1,
                                               const uint8_t *limit1,
                                               const uint8_t *thresh1);
extern "C" void vp9_lpf_horizontal_4_c(uint8_t *s, int pitch,
                                       const uint8_t *blimit,
                                       const uint8_t *limit,
                                       const uint8_t *thresh, int count);
extern "C" void vp9_lpf_horizontal_4_neon(uint8_t *s, int pitch,
                                          const uint8_t *blimit,
                                          const uint8_t *limit,
                                          const uint8_t *thresh, int count);
extern "C" void vp9_lpf_horizontal_4_dual_c(uint8_t *s, int pitch,
                                            const uint8_t *blimit0,
                                            const uint8_t *limit0,
                                            const uint8_t *thresh0,
                                            const uint8_t *blimit1,
                                            const uint8_t *limit1,
                                            const uint8_t *thresh1);
extern "C" void vp9_lpf_horizontal_4_dual_neon(uint8_t *s, int pitch,
                                               const uint8_t *blimit0,
                                               const uint8_t *limit0,
                                               const uint8_t *thresh0,
                                               const uint8_t *blimit1,
                                               const uint8_t *limit1,
                                               const uint8_t *thresh1);


#if defined(ARCH_ARM_HAVE_VFP)

#define vp9_lpf_vertical_16 vp9_lpf_vertical_16_neon
#define vp9_lpf_vertical_16_dual vp9_lpf_vertical_16_dual_neon
#define vp9_lpf_vertical_8 vp9_lpf_vertical_8_neon
#define vp9_lpf_vertical_8_dual vp9_lpf_vertical_8_dual_neon
#define vp9_lpf_vertical_4 vp9_lpf_vertical_4_neon
#define vp9_lpf_vertical_4_dual vp9_lpf_vertical_4_dual_neon
#define vp9_lpf_horizontal_16 vp9_lpf_horizontal_16_neon
#define vp9_lpf_horizontal_8 vp9_lpf_horizontal_8_neon
#define vp9_lpf_horizontal_8_dual vp9_lpf_horizontal_8_dual_neon
#define vp9_lpf_horizontal_4 vp9_lpf_horizontal_4_neon
#define vp9_lpf_horizontal_4_dual vp9_lpf_horizontal_4_dual_neon

void vp9_lpf_horizontal_8_dual_neon(uint8_t *s, int p /* pitch */,
                                    const uint8_t *blimit0,
                                    const uint8_t *limit0,
                                    const uint8_t *thresh0,
                                    const uint8_t *blimit1,
                                    const uint8_t *limit1,
                                    const uint8_t *thresh1) {
    vp9_lpf_horizontal_8(s, p, blimit0, limit0, thresh0, 1);
    vp9_lpf_horizontal_8(s + 8, p, blimit1, limit1, thresh1, 1);
}

void vp9_lpf_vertical_4_dual_neon(uint8_t *s, int p,
                                  const uint8_t *blimit0,
                                  const uint8_t *limit0,
                                  const uint8_t *thresh0,
                                  const uint8_t *blimit1,
                                  const uint8_t *limit1,
                                  const uint8_t *thresh1) {
    vp9_lpf_vertical_4_neon(s, p, blimit0, limit0, thresh0, 1);
    vp9_lpf_vertical_4_neon(s + 8 * p, p, blimit1, limit1, thresh1, 1);
}

void vp9_lpf_vertical_8_dual_neon(uint8_t *s, int p,
                                  const uint8_t *blimit0,
                                  const uint8_t *limit0,
                                  const uint8_t *thresh0,
                                  const uint8_t *blimit1,
                                  const uint8_t *limit1,
                                  const uint8_t *thresh1) {
    vp9_lpf_vertical_8_neon(s, p, blimit0, limit0, thresh0, 1);
    vp9_lpf_vertical_8_neon(s + 8 * p, p, blimit1, limit1, thresh1, 1);
}

void vp9_lpf_vertical_16_dual_neon(uint8_t *s, int p,
                                   const uint8_t *blimit,
                                   const uint8_t *limit,
                                   const uint8_t *thresh) {
    vp9_lpf_vertical_16_neon(s, p, blimit, limit, thresh);
    vp9_lpf_vertical_16_neon(s + 8 * p, p, blimit, limit, thresh);
}

#else

#define vp9_lpf_vertical_16 vp9_lpf_vertical_16_c
#define vp9_lpf_vertical_16_dual vp9_lpf_vertical_16_dual_c
#define vp9_lpf_vertical_8 vp9_lpf_vertical_8_c
#define vp9_lpf_vertical_8_dual vp9_lpf_vertical_8_dual_c
#define vp9_lpf_vertical_4 vp9_lpf_vertical_4_c
#define vp9_lpf_vertical_4_dual vp9_lpf_vertical_4_dual_c
#define vp9_lpf_horizontal_16 vp9_lpf_horizontal_16_c
#define vp9_lpf_horizontal_8 vp9_lpf_horizontal_8_c
#define vp9_lpf_horizontal_8_dual vp9_lpf_horizontal_8_dual_c
#define vp9_lpf_horizontal_4 vp9_lpf_horizontal_4_c
#define vp9_lpf_horizontal_4_dual vp9_lpf_horizontal_4_dual_c

#endif // ARCH_ARM_HAVE_VFP




static INLINE int8_t signed_char_clamp(int t) {
    return (int8_t)clamp(t, -128, 127);
}

// should we apply any filter at all: 11111111 yes, 00000000 no
static INLINE int8_t filter_mask(uint8_t limit, uint8_t blimit,
                                 uint8_t p3, uint8_t p2,
                                 uint8_t p1, uint8_t p0,
                                 uint8_t q0, uint8_t q1,
                                 uint8_t q2, uint8_t q3) {
    int8_t mask = 0;
    mask |= (abs(p3 - p2) > limit) * -1;
    mask |= (abs(p2 - p1) > limit) * -1;
    mask |= (abs(p1 - p0) > limit) * -1;
    mask |= (abs(q1 - q0) > limit) * -1;
    mask |= (abs(q2 - q1) > limit) * -1;
    mask |= (abs(q3 - q2) > limit) * -1;
    mask |= (abs(p0 - q0) * 2 + abs(p1 - q1) / 2 > blimit) * -1;
    return ~mask;
}

static INLINE int8_t flat_mask4(uint8_t thresh,
                                uint8_t p3, uint8_t p2,
                                uint8_t p1, uint8_t p0,
                                uint8_t q0, uint8_t q1,
                                uint8_t q2, uint8_t q3) {
    int8_t mask = 0;
    mask |= (abs(p1 - p0) > thresh) * -1;
    mask |= (abs(q1 - q0) > thresh) * -1;
    mask |= (abs(p2 - p0) > thresh) * -1;
    mask |= (abs(q2 - q0) > thresh) * -1;
    mask |= (abs(p3 - p0) > thresh) * -1;
    mask |= (abs(q3 - q0) > thresh) * -1;
    return ~mask;
}

static INLINE int8_t flat_mask5(uint8_t thresh,
                                uint8_t p4, uint8_t p3,
                                uint8_t p2, uint8_t p1,
                                uint8_t p0, uint8_t q0,
                                uint8_t q1, uint8_t q2,
                                uint8_t q3, uint8_t q4) {
    int8_t mask = ~flat_mask4(thresh, p3, p2, p1, p0, q0, q1, q2, q3);
    mask |= (abs(p4 - p0) > thresh) * -1;
    mask |= (abs(q4 - q0) > thresh) * -1;
    return ~mask;
}

// is there high edge variance internal edge: 11111111 yes, 00000000 no
static INLINE int8_t hev_mask(uint8_t thresh, uint8_t p1, uint8_t p0,
                              uint8_t q0, uint8_t q1) {
    int8_t hev = 0;
    hev |= (abs(p1 - p0) > thresh) * -1;
    hev |= (abs(q1 - q0) > thresh) * -1;
    return hev;
}

static INLINE void filter4(int8_t mask, uint8_t thresh, uint8_t *op1,
                           uint8_t *op0, uint8_t *oq0, uint8_t *oq1) {
    int8_t filter1, filter2;

    const int8_t ps1 = (int8_t) *op1 ^ 0x80;
    const int8_t ps0 = (int8_t) *op0 ^ 0x80;
    const int8_t qs0 = (int8_t) *oq0 ^ 0x80;
    const int8_t qs1 = (int8_t) *oq1 ^ 0x80;
    const uint8_t hev = hev_mask(thresh, *op1, *op0, *oq0, *oq1);

    // add outer taps if we have high edge variance
    int8_t filter = signed_char_clamp(ps1 - qs1) & hev;

    // inner taps
    filter = signed_char_clamp(filter + 3 * (qs0 - ps0)) & mask;

    // save bottom 3 bits so that we round one side +4 and the other +3
    // if it equals 4 we'll set to adjust by -1 to account for the fact
    // we'd round 3 the other way
    filter1 = signed_char_clamp(filter + 4) >> 3;
    filter2 = signed_char_clamp(filter + 3) >> 3;

    *oq0 = signed_char_clamp(qs0 - filter1) ^ 0x80;
    *op0 = signed_char_clamp(ps0 + filter2) ^ 0x80;

    // outer tap adjustments
    filter = ROUND_POWER_OF_TWO(filter1, 1) & ~hev;

    *oq1 = signed_char_clamp(qs1 - filter) ^ 0x80;
    *op1 = signed_char_clamp(ps1 + filter) ^ 0x80;
}

void vp9_lpf_horizontal_4_c(uint8_t *s, int p /* pitch */,
                            const uint8_t *blimit, const uint8_t *limit,
                            const uint8_t *thresh, int count) {
    int i;

    // loop filter designed to work using chars so that we can make maximum use
    // of 8 bit simd instructions.
    for (i = 0; i < 8 * count; ++i) {
        const uint8_t p3 = s[-4 * p], p2 = s[-3 * p], p1 = s[-2 * p], p0 = s[-p];
        const uint8_t q0 = s[0 * p],  q1 = s[1 * p],  q2 = s[2 * p],  q3 = s[3 * p];
        const int8_t mask = filter_mask(*limit, *blimit,
                                        p3, p2, p1, p0, q0, q1, q2, q3);
        filter4(mask, *thresh, s - 2 * p, s - 1 * p, s, s + 1 * p);
        ++s;
    }
}

void vp9_lpf_horizontal_4_dual_c(uint8_t *s, int p, const uint8_t *blimit0,
                                 const uint8_t *limit0, const uint8_t *thresh0,
                                 const uint8_t *blimit1, const uint8_t *limit1,
                                 const uint8_t *thresh1) {
    vp9_lpf_horizontal_4_c(s, p, blimit0, limit0, thresh0, 1);
    vp9_lpf_horizontal_4_c(s + 8, p, blimit1, limit1, thresh1, 1);
}

void vp9_lpf_vertical_4_c(uint8_t *s, int pitch, const uint8_t *blimit,
                          const uint8_t *limit, const uint8_t *thresh,
                          int count) {
    int i;

    // loop filter designed to work using chars so that we can make maximum use
    // of 8 bit simd instructions.
    for (i = 0; i < 8 * count; ++i) {
        const uint8_t p3 = s[-4], p2 = s[-3], p1 = s[-2], p0 = s[-1];
        const uint8_t q0 = s[0],  q1 = s[1],  q2 = s[2],  q3 = s[3];
        const int8_t mask = filter_mask(*limit, *blimit,
                                        p3, p2, p1, p0, q0, q1, q2, q3);
        filter4(mask, *thresh, s - 2, s - 1, s, s + 1);
        s += pitch;
    }
}

void vp9_lpf_vertical_4_dual_c(uint8_t *s, int pitch, const uint8_t *blimit0,
                               const uint8_t *limit0, const uint8_t *thresh0,
                               const uint8_t *blimit1, const uint8_t *limit1,
                               const uint8_t *thresh1) {
    vp9_lpf_vertical_4_c(s, pitch, blimit0, limit0, thresh0, 1);
    vp9_lpf_vertical_4_c(s + 8 * pitch, pitch, blimit1, limit1, thresh1, 1);
}

static INLINE void filter8(int8_t mask, uint8_t thresh, uint8_t flat,
                           uint8_t *op3, uint8_t *op2,
                           uint8_t *op1, uint8_t *op0,
                           uint8_t *oq0, uint8_t *oq1,
                           uint8_t *oq2, uint8_t *oq3) {
    if (flat && mask) {
        const uint8_t p3 = *op3, p2 = *op2, p1 = *op1, p0 = *op0;
        const uint8_t q0 = *oq0, q1 = *oq1, q2 = *oq2, q3 = *oq3;

        // 7-tap filter [1, 1, 1, 2, 1, 1, 1]
        *op2 = ROUND_POWER_OF_TWO(p3 + p3 + p3 + 2 * p2 + p1 + p0 + q0, 3);
        *op1 = ROUND_POWER_OF_TWO(p3 + p3 + p2 + 2 * p1 + p0 + q0 + q1, 3);
        *op0 = ROUND_POWER_OF_TWO(p3 + p2 + p1 + 2 * p0 + q0 + q1 + q2, 3);
        *oq0 = ROUND_POWER_OF_TWO(p2 + p1 + p0 + 2 * q0 + q1 + q2 + q3, 3);
        *oq1 = ROUND_POWER_OF_TWO(p1 + p0 + q0 + 2 * q1 + q2 + q3 + q3, 3);
        *oq2 = ROUND_POWER_OF_TWO(p0 + q0 + q1 + 2 * q2 + q3 + q3 + q3, 3);
    } else {
        filter4(mask, thresh, op1, op0, oq0, oq1);
    }
}

void vp9_lpf_horizontal_8_c(uint8_t *s, int p, const uint8_t *blimit,
                            const uint8_t *limit, const uint8_t *thresh,
                            int count) {
    int i;

    // loop filter designed to work using chars so that we can make maximum use
    // of 8 bit simd instructions.
    for (i = 0; i < 8 * count; ++i) {
        const uint8_t p3 = s[-4 * p], p2 = s[-3 * p], p1 = s[-2 * p], p0 = s[-p];
        const uint8_t q0 = s[0 * p], q1 = s[1 * p], q2 = s[2 * p], q3 = s[3 * p];

        const int8_t mask = filter_mask(*limit, *blimit,
                                        p3, p2, p1, p0, q0, q1, q2, q3);
        const int8_t flat = flat_mask4(1, p3, p2, p1, p0, q0, q1, q2, q3);
        filter8(mask, *thresh, flat, s - 4 * p, s - 3 * p, s - 2 * p, s - 1 * p,
                                     s,         s + 1 * p, s + 2 * p, s + 3 * p);
        ++s;
    }
}

void vp9_lpf_horizontal_8_dual_c(uint8_t *s, int p, const uint8_t *blimit0,
                                 const uint8_t *limit0, const uint8_t *thresh0,
                                 const uint8_t *blimit1, const uint8_t *limit1,
                                 const uint8_t *thresh1) {
    vp9_lpf_horizontal_8_c(s, p, blimit0, limit0, thresh0, 1);
    vp9_lpf_horizontal_8_c(s + 8, p, blimit1, limit1, thresh1, 1);
}

void vp9_lpf_vertical_8_c(uint8_t *s, int pitch, const uint8_t *blimit,
                          const uint8_t *limit, const uint8_t *thresh,
                          int count) {
    int i;

    for (i = 0; i < 8 * count; ++i) {
        const uint8_t p3 = s[-4], p2 = s[-3], p1 = s[-2], p0 = s[-1];
        const uint8_t q0 = s[0], q1 = s[1], q2 = s[2], q3 = s[3];
        const int8_t mask = filter_mask(*limit, *blimit,
                                        p3, p2, p1, p0, q0, q1, q2, q3);
        const int8_t flat = flat_mask4(1, p3, p2, p1, p0, q0, q1, q2, q3);
        filter8(mask, *thresh, flat, s - 4, s - 3, s - 2, s - 1,
                                     s,     s + 1, s + 2, s + 3);
        s += pitch;
    }
}

void vp9_lpf_vertical_8_dual_c(uint8_t *s, int pitch, const uint8_t *blimit0,
                               const uint8_t *limit0, const uint8_t *thresh0,
                               const uint8_t *blimit1, const uint8_t *limit1,
                               const uint8_t *thresh1) {
    vp9_lpf_vertical_8_c(s, pitch, blimit0, limit0, thresh0, 1);
    vp9_lpf_vertical_8_c(s + 8 * pitch, pitch, blimit1, limit1, thresh1, 1);
}

static INLINE void filter16(int8_t mask, uint8_t thresh,
                            uint8_t flat, uint8_t flat2,
                            uint8_t *op7, uint8_t *op6,
                            uint8_t *op5, uint8_t *op4,
                            uint8_t *op3, uint8_t *op2,
                            uint8_t *op1, uint8_t *op0,
                            uint8_t *oq0, uint8_t *oq1,
                            uint8_t *oq2, uint8_t *oq3,
                            uint8_t *oq4, uint8_t *oq5,
                            uint8_t *oq6, uint8_t *oq7) {
    if (flat2 && flat && mask) {
        const uint8_t p7 = *op7, p6 = *op6, p5 = *op5, p4 = *op4,
                      p3 = *op3, p2 = *op2, p1 = *op1, p0 = *op0;

        const uint8_t q0 = *oq0, q1 = *oq1, q2 = *oq2, q3 = *oq3,
                      q4 = *oq4, q5 = *oq5, q6 = *oq6, q7 = *oq7;

        // 15-tap filter [1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1]
        *op6 = ROUND_POWER_OF_TWO(p7 * 7 + p6 * 2 + p5 + p4 + p3 + p2 + p1 + p0 +
                                  q0, 4);
        *op5 = ROUND_POWER_OF_TWO(p7 * 6 + p6 + p5 * 2 + p4 + p3 + p2 + p1 + p0 +
                                  q0 + q1, 4);
        *op4 = ROUND_POWER_OF_TWO(p7 * 5 + p6 + p5 + p4 * 2 + p3 + p2 + p1 + p0 +
                                  q0 + q1 + q2, 4);
        *op3 = ROUND_POWER_OF_TWO(p7 * 4 + p6 + p5 + p4 + p3 * 2 + p2 + p1 + p0 +
                                  q0 + q1 + q2 + q3, 4);
        *op2 = ROUND_POWER_OF_TWO(p7 * 3 + p6 + p5 + p4 + p3 + p2 * 2 + p1 + p0 +
                                  q0 + q1 + q2 + q3 + q4, 4);
        *op1 = ROUND_POWER_OF_TWO(p7 * 2 + p6 + p5 + p4 + p3 + p2 + p1 * 2 + p0 +
                                  q0 + q1 + q2 + q3 + q4 + q5, 4);
        *op0 = ROUND_POWER_OF_TWO(p7 + p6 + p5 + p4 + p3 + p2 + p1 + p0 * 2 +
                                  q0 + q1 + q2 + q3 + q4 + q5 + q6, 4);
        *oq0 = ROUND_POWER_OF_TWO(p6 + p5 + p4 + p3 + p2 + p1 + p0 +
                                  q0 * 2 + q1 + q2 + q3 + q4 + q5 + q6 + q7, 4);
        *oq1 = ROUND_POWER_OF_TWO(p5 + p4 + p3 + p2 + p1 + p0 +
                                  q0 + q1 * 2 + q2 + q3 + q4 + q5 + q6 + q7 * 2, 4);
        *oq2 = ROUND_POWER_OF_TWO(p4 + p3 + p2 + p1 + p0 +
                                  q0 + q1 + q2 * 2 + q3 + q4 + q5 + q6 + q7 * 3, 4);
        *oq3 = ROUND_POWER_OF_TWO(p3 + p2 + p1 + p0 +
                                  q0 + q1 + q2 + q3 * 2 + q4 + q5 + q6 + q7 * 4, 4);
        *oq4 = ROUND_POWER_OF_TWO(p2 + p1 + p0 +
                                  q0 + q1 + q2 + q3 + q4 * 2 + q5 + q6 + q7 * 5, 4);
        *oq5 = ROUND_POWER_OF_TWO(p1 + p0 +
                                  q0 + q1 + q2 + q3 + q4 + q5 * 2 + q6 + q7 * 6, 4);
        *oq6 = ROUND_POWER_OF_TWO(p0 +
                                  q0 + q1 + q2 + q3 + q4 + q5 + q6 * 2 + q7 * 7, 4);
    } else {
        filter8(mask, thresh, flat, op3, op2, op1, op0, oq0, oq1, oq2, oq3);
    }
}

void vp9_lpf_horizontal_16_c(uint8_t *s, int p, const uint8_t *blimit,
                             const uint8_t *limit, const uint8_t *thresh,
                             int count) {
    int i;

    // loop filter designed to work using chars so that we can make maximum use
    // of 8 bit simd instructions.
    for (i = 0; i < 8 * count; ++i) {
        const uint8_t p3 = s[-4 * p], p2 = s[-3 * p], p1 = s[-2 * p], p0 = s[-p];
        const uint8_t q0 = s[0 * p], q1 = s[1 * p], q2 = s[2 * p], q3 = s[3 * p];
        const int8_t mask = filter_mask(*limit, *blimit,
                                        p3, p2, p1, p0, q0, q1, q2, q3);
        const int8_t flat = flat_mask4(1, p3, p2, p1, p0, q0, q1, q2, q3);
        const int8_t flat2 = flat_mask5(1, s[-8 * p], s[-7 * p], s[-6 * p], s[-5 * p], p0,
                                        q0, s[4 * p], s[5 * p], s[6 * p], s[7 * p]);

        filter16(mask, *thresh, flat, flat2,
                 s - 8 * p, s - 7 * p, s - 6 * p, s - 5 * p,
                 s - 4 * p, s - 3 * p, s - 2 * p, s - 1 * p,
                 s,         s + 1 * p, s + 2 * p, s + 3 * p,
                 s + 4 * p, s + 5 * p, s + 6 * p, s + 7 * p);
        ++s;
    }
}

static void mb_lpf_vertical_edge_w(uint8_t *s, int p,
                                   const uint8_t *blimit,
                                   const uint8_t *limit,
                                   const uint8_t *thresh,
                                   int count) {
    int i;

    for (i = 0; i < count; ++i) {
        const uint8_t p3 = s[-4], p2 = s[-3], p1 = s[-2], p0 = s[-1];
        const uint8_t q0 = s[0], q1 = s[1], q2 = s[2], q3 = s[3];
        const int8_t mask = filter_mask(*limit, *blimit,
                                        p3, p2, p1, p0, q0, q1, q2, q3);
        const int8_t flat = flat_mask4(1, p3, p2, p1, p0, q0, q1, q2, q3);
        const int8_t flat2 = flat_mask5(1, s[-8], s[-7], s[-6], s[-5], p0,
                                        q0, s[4], s[5], s[6], s[7]);

        filter16(mask, *thresh, flat, flat2,
                 s - 8, s - 7, s - 6, s - 5, s - 4, s - 3, s - 2, s - 1,
                 s,     s + 1, s + 2, s + 3, s + 4, s + 5, s + 6, s + 7);
        s += p;
    }
}

void vp9_lpf_vertical_16_c(uint8_t *s, int p, const uint8_t *blimit,
                           const uint8_t *limit, const uint8_t *thresh) {
    mb_lpf_vertical_edge_w(s, p, blimit, limit, thresh, 8);
}

void vp9_lpf_vertical_16_dual_c(uint8_t *s, int p, const uint8_t *blimit,
                                const uint8_t *limit, const uint8_t *thresh) {
    mb_lpf_vertical_edge_w(s, p, blimit, limit, thresh, 16);
}


static void filter_selectively_vert_row2(PLANE_TYPE plane_type,
                                         uint8_t *s, int pitch,
                                         unsigned int mask_16x16_l,
                                         unsigned int mask_8x8_l,
                                         unsigned int mask_4x4_l,
                                         unsigned int mask_4x4_int_l,
                                         const LoopFilterInfoN *lfi_n,
                                         const uint8_t *lfl) {
    const int mask_shift = plane_type ? 4 : 8;
    const int mask_cutoff = plane_type ? 0xf : 0xff;
    const int lfl_forward = plane_type ? 4 : 8;

    unsigned int mask_16x16_0 = mask_16x16_l & mask_cutoff;
    unsigned int mask_8x8_0 = mask_8x8_l & mask_cutoff;
    unsigned int mask_4x4_0 = mask_4x4_l & mask_cutoff;
    unsigned int mask_4x4_int_0 = mask_4x4_int_l & mask_cutoff;
    unsigned int mask_16x16_1 = (mask_16x16_l >> mask_shift) & mask_cutoff;
    unsigned int mask_8x8_1 = (mask_8x8_l >> mask_shift) & mask_cutoff;
    unsigned int mask_4x4_1 = (mask_4x4_l >> mask_shift) & mask_cutoff;
    unsigned int mask_4x4_int_1 = (mask_4x4_int_l >> mask_shift) & mask_cutoff;
    unsigned int mask;

    for (mask = mask_16x16_0 | mask_8x8_0 | mask_4x4_0 | mask_4x4_int_0 |
         mask_16x16_1 | mask_8x8_1 | mask_4x4_1 | mask_4x4_int_1;
         mask; mask >>= 1) {
        const LoopFilterThresh *lfi0 = lfi_n->lfthr + *lfl;
        const LoopFilterThresh *lfi1 = lfi_n->lfthr + *(lfl + lfl_forward);

        // TODO(yunqingwang): count in loopfilter functions should be removed.
        if (mask & 1) {
            if ((mask_16x16_0 | mask_16x16_1) & 1) {
                if ((mask_16x16_0 & mask_16x16_1) & 1) {
                    vp9_lpf_vertical_16_dual(s, pitch, lfi0->mblim, lfi0->lim,
                                             lfi0->hev_thr);
                } else if (mask_16x16_0 & 1) {
                    vp9_lpf_vertical_16(s, pitch, lfi0->mblim, lfi0->lim,
                                        lfi0->hev_thr);
                } else {
                    vp9_lpf_vertical_16(s + 8 *pitch, pitch, lfi1->mblim,
                                        lfi1->lim, lfi1->hev_thr);
                }
            }

            if ((mask_8x8_0 | mask_8x8_1) & 1) {
                if ((mask_8x8_0 & mask_8x8_1) & 1) {
                    vp9_lpf_vertical_8_dual(s, pitch, lfi0->mblim, lfi0->lim,
                                            lfi0->hev_thr, lfi1->mblim, lfi1->lim,
                                            lfi1->hev_thr);
                } else if (mask_8x8_0 & 1) {
                    vp9_lpf_vertical_8(s, pitch, lfi0->mblim, lfi0->lim,
                                       lfi0->hev_thr, 1);
                } else {
                    vp9_lpf_vertical_8(s + 8 * pitch, pitch, lfi1->mblim, lfi1->lim,
                                       lfi1->hev_thr, 1);
                }
            }

            if ((mask_4x4_0 | mask_4x4_1) & 1) {
                if ((mask_4x4_0 & mask_4x4_1) & 1) {
                    vp9_lpf_vertical_4_dual(s, pitch, lfi0->mblim, lfi0->lim,
                                            lfi0->hev_thr, lfi1->mblim, lfi1->lim,
                                            lfi1->hev_thr);
                } else if (mask_4x4_0 & 1) {
                    vp9_lpf_vertical_4(s, pitch, lfi0->mblim, lfi0->lim,
                                       lfi0->hev_thr, 1);
                } else {
                    vp9_lpf_vertical_4(s + 8 * pitch, pitch, lfi1->mblim, lfi1->lim,
                                       lfi1->hev_thr, 1);
                }
            }

            if ((mask_4x4_int_0 | mask_4x4_int_1) & 1) {
                if ((mask_4x4_int_0 & mask_4x4_int_1) & 1) {
                    vp9_lpf_vertical_4_dual(s + 4, pitch, lfi0->mblim, lfi0->lim,
                                            lfi0->hev_thr, lfi1->mblim, lfi1->lim,
                                            lfi1->hev_thr);
                } else if (mask_4x4_int_0 & 1) {
                    vp9_lpf_vertical_4(s + 4, pitch, lfi0->mblim, lfi0->lim,
                                       lfi0->hev_thr, 1);
                } else {
                    vp9_lpf_vertical_4(s + 8 * pitch + 4, pitch, lfi1->mblim,
                                       lfi1->lim, lfi1->hev_thr, 1);
                }
            }
        }

        s += 8;
        lfl += 1;
        mask_16x16_0 >>= 1;
        mask_8x8_0 >>= 1;
        mask_4x4_0 >>= 1;
        mask_4x4_int_0 >>= 1;
        mask_16x16_1 >>= 1;
        mask_8x8_1 >>= 1;
        mask_4x4_1 >>= 1;
        mask_4x4_int_1 >>= 1;
    }
}

static void filter_selectively_horiz(uint8_t *s, int pitch,
                                     unsigned int mask_16x16,
                                     unsigned int mask_8x8,
                                     unsigned int mask_4x4,
                                     unsigned int mask_4x4_int,
                                     const LoopFilterInfoN *lfi_n,
                                     const uint8_t *lfl) {
    unsigned int mask;
    int count;

    for (mask = mask_16x16 | mask_8x8 | mask_4x4 | mask_4x4_int;
         mask; mask >>= count) {
        const LoopFilterThresh *lfi = lfi_n->lfthr + *lfl;

        count = 1;
        if (mask & 1) {
            if (mask_16x16 & 1) {
                if ((mask_16x16 & 3) == 3) {
                    vp9_lpf_horizontal_16(s, pitch, lfi->mblim, lfi->lim,
                                          lfi->hev_thr, 2);
                    count = 2;
                } else {
                    vp9_lpf_horizontal_16(s, pitch, lfi->mblim, lfi->lim,
                                          lfi->hev_thr, 1);
                }
            } else if (mask_8x8 & 1) {
                if ((mask_8x8 & 3) == 3) {
                    // Next block's thresholds
                    const LoopFilterThresh *lfin = lfi_n->lfthr + *(lfl + 1);

                    vp9_lpf_horizontal_8_dual(s, pitch, lfi->mblim, lfi->lim,
                                              lfi->hev_thr, lfin->mblim, lfin->lim,
                                              lfin->hev_thr);

                    if ((mask_4x4_int & 3) == 3) {
                        vp9_lpf_horizontal_4_dual(s + 4 * pitch, pitch, lfi->mblim,
                                                  lfi->lim, lfi->hev_thr, lfin->mblim,
                                                  lfin->lim, lfin->hev_thr);
                    } else {
                        if (mask_4x4_int & 1)
                            vp9_lpf_horizontal_4(s + 4 * pitch, pitch, lfi->mblim,
                                                 lfi->lim, lfi->hev_thr, 1);
                        else if (mask_4x4_int & 2)
                            vp9_lpf_horizontal_4(s + 8 + 4 * pitch, pitch, lfin->mblim,
                                                 lfin->lim, lfin->hev_thr, 1);
                    }
                    count = 2;
                } else {
                    vp9_lpf_horizontal_8(s, pitch, lfi->mblim, lfi->lim, lfi->hev_thr, 1);

                    if (mask_4x4_int & 1)
                        vp9_lpf_horizontal_4(s + 4 * pitch, pitch, lfi->mblim,
                                             lfi->lim, lfi->hev_thr, 1);
                }
            } else if (mask_4x4 & 1) {
                if ((mask_4x4 & 3) == 3) {
                    // Next block's thresholds
                    const LoopFilterThresh *lfin = lfi_n->lfthr + *(lfl + 1);

                    vp9_lpf_horizontal_4_dual(s, pitch, lfi->mblim, lfi->lim,
                                              lfi->hev_thr, lfin->mblim, lfin->lim,
                                              lfin->hev_thr);
                    if ((mask_4x4_int & 3) == 3) {
                        vp9_lpf_horizontal_4_dual(s + 4 * pitch, pitch, lfi->mblim,
                                                  lfi->lim, lfi->hev_thr, lfin->mblim,
                                                  lfin->lim, lfin->hev_thr);
                    } else {
                        if (mask_4x4_int & 1)
                            vp9_lpf_horizontal_4(s + 4 * pitch, pitch, lfi->mblim,
                                                 lfi->lim, lfi->hev_thr, 1);
                        else if (mask_4x4_int & 2)
                            vp9_lpf_horizontal_4(s + 8 + 4 * pitch, pitch, lfin->mblim,
                                                 lfin->lim, lfin->hev_thr, 1);
                    }
                    count = 2;
                } else {
                    vp9_lpf_horizontal_4(s, pitch, lfi->mblim, lfi->lim, lfi->hev_thr, 1);

                    if (mask_4x4_int & 1)
                        vp9_lpf_horizontal_4(s + 4 * pitch, pitch, lfi->mblim, lfi->lim,
                                             lfi->hev_thr, 1);
                }
            } else if (mask_4x4_int & 1) {
                vp9_lpf_horizontal_4(s + 4 * pitch, pitch, lfi->mblim, lfi->lim,
                                     lfi->hev_thr, 1);
            }
        }
        s += 8 * count;
        lfl += count;
        mask_16x16 >>= count;
        mask_8x8 >>= count;
        mask_4x4 >>= count;
        mask_4x4_int >>= count;
    }
}

static void filter_block_plane_y(LoopFilterInfoN *lf_info,
                                 LoopFilterMask *lfm,
                                 int stride,
                                 uint8_t *buf,
                                 int mi_rows,
                                 int mi_row) {
    uint8_t* dst0 = buf;
    int r;  //, c;

    uint64_t mask_16x16 = lfm->left_y[TX_16X16];
    uint64_t mask_8x8 = lfm->left_y[TX_8X8];
    uint64_t mask_4x4 = lfm->left_y[TX_4X4];
    uint64_t mask_4x4_int = lfm->int_4x4_y;

    // Vertical pass: do 2 rows at one time
    for (r = 0; r < MI_BLOCK_SIZE && mi_row + r < mi_rows; r += 2) {
        unsigned int mask_16x16_l = mask_16x16 & 0xffff;
        unsigned int mask_8x8_l = mask_8x8 & 0xffff;
        unsigned int mask_4x4_l = mask_4x4 & 0xffff;
        unsigned int mask_4x4_int_l = mask_4x4_int & 0xffff;

        // Disable filtering on the leftmost column
        filter_selectively_vert_row2(PLANE_TYPE_Y_WITH_DC, buf, stride,
                mask_16x16_l, mask_8x8_l, mask_4x4_l, mask_4x4_int_l, lf_info,
                &lfm->lfl_y[r << 3]);

        buf += 16 * stride;
        mask_16x16 >>= 16;
        mask_8x8 >>= 16;
        mask_4x4 >>= 16;
        mask_4x4_int >>= 16;
    }

    // Horizontal pass
    buf = dst0;
    mask_16x16 = lfm->above_y[TX_16X16];
    mask_8x8 = lfm->above_y[TX_8X8];
    mask_4x4 = lfm->above_y[TX_4X4];
    mask_4x4_int = lfm->int_4x4_y;

    for (r = 0; r < MI_BLOCK_SIZE && mi_row + r < mi_rows; r++) {
        unsigned int mask_16x16_r;
        unsigned int mask_8x8_r;
        unsigned int mask_4x4_r;

        if (mi_row + r == 0) {
            mask_16x16_r = 0;
            mask_8x8_r = 0;
            mask_4x4_r = 0;
        } else {
            mask_16x16_r = mask_16x16 & 0xff;
            mask_8x8_r = mask_8x8 & 0xff;
            mask_4x4_r = mask_4x4 & 0xff;
        }

        filter_selectively_horiz(buf, stride, mask_16x16_r, mask_8x8_r,
                mask_4x4_r, mask_4x4_int & 0xff, lf_info, &lfm->lfl_y[r << 3]);

        buf += 8 * stride;
        mask_16x16 >>= 8;
        mask_8x8 >>= 8;
        mask_4x4 >>= 8;
        mask_4x4_int >>= 8;
    }
}

static void filter_block_plane_uv(LoopFilterInfoN *lf_info,
                                  LoopFilterMask *lfm,
                                  int stride,
                                  uint8_t *buf,
                                  int mi_rows,
                                  int mi_row) {
    uint8_t* dst0 = buf;
    int r, c;

    uint16_t mask_16x16 = lfm->left_uv[TX_16X16];
    uint16_t mask_8x8 = lfm->left_uv[TX_8X8];
    uint16_t mask_4x4 = lfm->left_uv[TX_4X4];
    uint16_t mask_4x4_int = lfm->int_4x4_uv;

    // Vertical pass: do 2 rows at one time
    for (r = 0; r < MI_BLOCK_SIZE && mi_row + r < mi_rows; r += 4) {

        for (c = 0; c < (MI_BLOCK_SIZE >> 1); c++) {
            lfm->lfl_uv[(r << 1) + c] = lfm->lfl_y[(r << 3) + (c << 1)];
            lfm->lfl_uv[((r + 2) << 1) + c] = lfm->lfl_y[((r + 2) << 3) + (c << 1)];
        }

        {
            unsigned int mask_16x16_l = mask_16x16 & 0xff;
            unsigned int mask_8x8_l = mask_8x8 & 0xff;
            unsigned int mask_4x4_l = mask_4x4 & 0xff;
            unsigned int mask_4x4_int_l = mask_4x4_int & 0xff;

            // Disable filtering on the leftmost column
            filter_selectively_vert_row2(PLANE_TYPE_UV, buf, stride,
                    mask_16x16_l, mask_8x8_l, mask_4x4_l, mask_4x4_int_l,
                    lf_info, &lfm->lfl_uv[r << 1]);

            buf += 16 * stride;
            mask_16x16 >>= 8;
            mask_8x8 >>= 8;
            mask_4x4 >>= 8;
            mask_4x4_int >>= 8;
        }
    }

    // Horizontal pass
    buf = dst0;
    mask_16x16 = lfm->above_uv[TX_16X16];
    mask_8x8 = lfm->above_uv[TX_8X8];
    mask_4x4 = lfm->above_uv[TX_4X4];
    mask_4x4_int = lfm->int_4x4_uv;

    for (r = 0; r < MI_BLOCK_SIZE && mi_row + r < mi_rows; r += 2) {
        int skip_border_4x4_r = mi_row + r == mi_rows - 1;
        unsigned int mask_4x4_int_r = skip_border_4x4_r ? 0 : (mask_4x4_int & 0xf);
        unsigned int mask_16x16_r;
        unsigned int mask_8x8_r;
        unsigned int mask_4x4_r;

        if (mi_row + r == 0) {
            mask_16x16_r = 0;
            mask_8x8_r = 0;
            mask_4x4_r = 0;
        } else {
            mask_16x16_r = mask_16x16 & 0xf;
            mask_8x8_r = mask_8x8 & 0xf;
            mask_4x4_r = mask_4x4 & 0xf;
        }

        filter_selectively_horiz(buf, stride, mask_16x16_r, mask_8x8_r,
                mask_4x4_r, mask_4x4_int_r, lf_info, &lfm->lfl_uv[r << 1]);

        buf += 8 * stride;
        mask_16x16 >>= 4;
        mask_8x8 >>= 4;
        mask_4x4 >>= 4;
        mask_4x4_int >>= 4;
    }
}

static void *vp9_loop_filter_rows_work_proc(void *data) {
    LoopFilterProgressChart *param = (LoopFilterProgressChart *)data;
    int wid = android_atomic_inc(&param->wid);
    int sb_row;
    int mi_row, mi_col;
    int lfm_idx;
    uint8_t *buf_start[MAX_MB_PLANE];
    uint8_t *buf[MAX_MB_PLANE];
    BufferInfo *buf_info = &param->buf_info;

    while (!android_atomic_release_load(&param->quit)) {
        pthread_mutex_lock(&param->mutex[wid]);
        pthread_cond_wait(&param->start_cond[wid], &param->mutex[wid]);
        pthread_mutex_unlock(&param->mutex[wid]);

        if (android_atomic_release_load(&param->quit)) return NULL;

        buf_start[0] = param->buffer_alloc + buf_info->y_offset;
        buf_start[1] = param->buffer_alloc + buf_info->u_offset;
        buf_start[2] = param->buffer_alloc + buf_info->v_offset;
        sb_row = android_atomic_inc(&param->sb_row_pro);
        mi_row = (sb_row * MI_BLOCK_SIZE) + param->start;

        while (mi_row < param->stop) {
            buf[0] = buf_start[0] + (mi_row * buf_info->y_stride << 3);
            buf[1] = buf_start[1] + (mi_row * buf_info->uv_stride << 2);
            buf[2] = buf_start[2] + (mi_row * buf_info->uv_stride << 2);
            lfm_idx = sb_row * ((param->mi_cols + 7) >> 3);
            for (mi_col = 0; mi_col < param->mi_cols; mi_col += MI_BLOCK_SIZE) {

                while (param->chart[sb_row+1] + 2 > android_atomic_release_load(&param->chart[sb_row])) {
                    usleep(1);
                }

                filter_block_plane_y(param->lf_info, param->lfms + lfm_idx,
                                     buf_info->y_stride, buf[0], param->mi_rows,
                                     mi_row);
                mi_col += MI_BLOCK_SIZE;
                if (mi_col < param->mi_cols) {
                    lfm_idx++;
                    buf[0] += MI_BLOCK_SIZE * MI_BLOCK_SIZE;
                    filter_block_plane_y(param->lf_info, param->lfms + lfm_idx,
                                         buf_info->y_stride, buf[0],
                                         param->mi_rows, mi_row);
                }
                buf[0] += MI_BLOCK_SIZE * MI_BLOCK_SIZE;
                if (param->num_planes > 1) {
                    lfm_idx--;
                    filter_block_plane_uv(param->lf_info, param->lfms + lfm_idx,
                                          buf_info->uv_stride, buf[1],
                                          param->mi_rows, mi_row);
                    filter_block_plane_uv(param->lf_info, param->lfms + lfm_idx,
                                          buf_info->uv_stride, buf[2],
                                          param->mi_rows, mi_row);
                    if (mi_col < param->mi_cols) {
                        lfm_idx++;
                        buf[1] += MI_BLOCK_SIZE * MI_BLOCK_SIZE >> 1;
                        buf[2] += MI_BLOCK_SIZE * MI_BLOCK_SIZE >> 1;
                        filter_block_plane_uv(param->lf_info,
                                              param->lfms + lfm_idx,
                                              buf_info->uv_stride, buf[1],
                                              param->mi_rows, mi_row);
                        filter_block_plane_uv(param->lf_info,
                                              param->lfms + lfm_idx,
                                              buf_info->uv_stride, buf[2],
                                              param->mi_rows, mi_row);
                    }
                    buf[1] += MI_BLOCK_SIZE * MI_BLOCK_SIZE >> 1;
                    buf[2] += MI_BLOCK_SIZE * MI_BLOCK_SIZE >> 1;
                }
                lfm_idx++;
                android_atomic_inc(&param->chart[sb_row+1]);
            }
            android_atomic_inc(&param->chart[sb_row+1]);
            sb_row = android_atomic_inc(&param->sb_row_pro);
            mi_row = (sb_row << 3) + param->start;
        }

        pthread_mutex_lock(param->hmutex);
        if ((--param->doing) == 0)
            pthread_cond_signal(param->finish);
        pthread_mutex_unlock(param->hmutex);
    }

    return NULL;
}

RsdCpuScriptIntrinsicLoopFilter::RsdCpuScriptIntrinsicLoopFilter(
            RsdCpuReferenceImpl *ctx, const Script *s, const Element *e)
            : RsdCpuScriptIntrinsic(ctx, s, e, RS_SCRIPT_INTRINSIC_ID_YUV_TO_RGB) {
    mRootPtr = &kernel;
    mWorkerCount = sysconf(_SC_NPROCESSORS_ONLN);
    mPrch.quit = 0;
    mPrch.wid = 0;
    mPrch.sb_row_pro = 0;
    mPrch.doing = mWorkerCount;
    int size = mWorkerCount * sizeof(pthread_t) +
               mWorkerCount * sizeof(pthread_mutex_t) +
               mWorkerCount * sizeof(pthread_cond_t) +
               sizeof(pthread_mutex_t) + sizeof(pthread_cond_t);
    uint8_t *ptr = (uint8_t *)malloc(size);
    rsAssert(ptr);
    mPrch.tid = (pthread_t *)ptr;
    mPrch.mutex = (pthread_mutex_t *) (mPrch.tid + mWorkerCount);
    mPrch.start_cond  = (pthread_cond_t *) (mPrch.mutex + mWorkerCount);
    mPrch.hmutex = (pthread_mutex_t *) (mPrch.start_cond + mWorkerCount);
    mPrch.finish = (pthread_cond_t *) (mPrch.hmutex + 1);
    int i = 0;
    int rv = 0;
    pthread_mutex_init(mPrch.hmutex, NULL);
    pthread_cond_init(mPrch.finish, NULL);
    for (i = 0; i < mWorkerCount; ++i) {
        pthread_mutex_init(&mPrch.mutex[i], NULL);
        pthread_cond_init(&mPrch.start_cond[i], NULL);
    }
    for (i = 0; i < mWorkerCount; ++i) {
        rv = pthread_create(&mPrch.tid[i], NULL, &vp9_loop_filter_rows_work_proc, &mPrch);
        rsAssert(rv == 0);
    }
}

