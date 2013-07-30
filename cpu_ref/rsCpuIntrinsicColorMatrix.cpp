/*
 * Copyright (C) 2012 The Android Open Source Project
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

#include <sys/mman.h>
#include <unistd.h>

#include "rsCpuIntrinsic.h"
#include "rsCpuIntrinsicInlines.h"
#include "linkloader/include/MemChunk.h"

#include <sys/mman.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
//#include <utils/StopWatch.h>


using namespace android;
using namespace android::renderscript;

namespace android {
namespace renderscript {


class RsdCpuScriptIntrinsicColorMatrix : public RsdCpuScriptIntrinsic {
public:
    virtual void populateScript(Script *);

    virtual void setGlobalVar(uint32_t slot, const void *data, size_t dataLength);

    virtual ~RsdCpuScriptIntrinsicColorMatrix();
    RsdCpuScriptIntrinsicColorMatrix(RsdCpuReferenceImpl *ctx, const Script *s, const Element *e);

    virtual void preLaunch(uint32_t slot, const Allocation * ain, Allocation * aout,
                           const void * usr, uint32_t usrLen, const RsScriptCall *sc);
    virtual void postLaunch(uint32_t slot, const Allocation * ain, Allocation * aout,
                            const void * usr, uint32_t usrLen, const RsScriptCall *sc);

protected:
    float fp[16];
    float fpa[4];
    short ip[16];
    int ipa[4];

    static void kernel(const RsForEachStubParamStruct *p,
                       uint32_t xstart, uint32_t xend,
                       uint32_t instep, uint32_t outstep);

    int64_t mLastKey;
    unsigned char *mBuf;
    size_t mBufSize;

    int64_t computeKey(const Element *ein, const Element *eout);

    bool build(int64_t key);

    void (*mOptKernel)(void *dst, const void *src, const short *coef, uint32_t count);

};

}
}


#define CM_IN_VEC_SIZE_MASK         0x00000003
#define CM_OUT_VEC_SIZE_MASK        0x0000000c
#define CM_IN_TYPE_SIZE_MASK        0x000000F0
#define CM_OUT_TYPE_SIZE_MASK       0x00000F00
#define CM_DOT_MASK                 0x00001000
#define CM_ADD_MASK                 0x00002000
#define CM_COPY_ALPHA               0x00004000
#define CM_MATRIX_MASK              0xFFFF0000


int64_t RsdCpuScriptIntrinsicColorMatrix::computeKey(
        const Element *ein, const Element *eout) {

    int64_t key = 0;

    // Compute a unique code key for this operation

    // Add to the key the input and output types
    key |= (ein->getVectorSize() - 1);
    key |= (eout->getVectorSize() - 1) << 2;

    bool hasFloat = false;
    if (ein->getType() == RS_TYPE_FLOAT_32) {
        hasFloat = true;
        key |= 1 << 4;
    }
    if (eout->getType() == RS_TYPE_FLOAT_32) {
        hasFloat = true;
        key |= 1 << 8;
    }

    // Mask in the bits indicating which coefficients in the
    // color matrix are needed.
    if (hasFloat) {
        for (uint32_t i=0; i < 16; i++) {
            if (fabs(fp[i]) != 0.f) {
                key |= (uint32_t)(1 << (i + 16));
            }
        }
    } else {
        for (uint32_t i=0; i < 16; i++) {
            if (ip[i] != 0) {
                key |= (uint32_t)(1 << (i + 16));
            }
        }
    }

    // Look for a dot product where the r,g,b colums are the same
    if ((ip[0] == ip[1]) && (ip[0] == ip[2]) &&
        (ip[4] == ip[5]) && (ip[4] == ip[6]) &&
        (ip[8] == ip[9]) && (ip[8] == ip[10]) &&
        (ip[12] == ip[13]) && (ip[12] == ip[14])) {

        key |= CM_DOT_MASK;
    }

    // Is alpha a simple copy
    if (!(key & 0x08880000) && (ip[15] == 256)) {
        key |= CM_COPY_ALPHA;
    }

    //ALOGE("build key %08x, %08x", (int32_t)(key >> 32), (int32_t)key);
    return key;
}

#if defined(ARCH_ARM_HAVE_NEON)

#define DEF_SYM(x)                                  \
    extern "C" uint32_t _N_ColorMatrix_##x;      \
    extern "C" uint32_t _N_ColorMatrix_##x##_end;  \
    extern "C" uint32_t _N_ColorMatrix_##x##_len;

DEF_SYM(prefix)
DEF_SYM(postfix1)
DEF_SYM(postfix2)
DEF_SYM(load_u8_4)
DEF_SYM(load_u8_2)
DEF_SYM(load_u8_1)
DEF_SYM(store_u8_4)
DEF_SYM(store_u8_2)
DEF_SYM(store_u8_1)
DEF_SYM(unpack_u8_4)
DEF_SYM(unpack_u8_3)
DEF_SYM(unpack_u8_2)
DEF_SYM(unpack_u8_1)
DEF_SYM(pack_u8_4)
DEF_SYM(pack_u8_3)
DEF_SYM(pack_u8_2)
DEF_SYM(pack_u8_1)
DEF_SYM(dot)
DEF_SYM(add_0_u8)
DEF_SYM(add_1_u8)
DEF_SYM(add_2_u8)
DEF_SYM(add_3_u8)

#define ADD_CHUNK(x) \
    memcpy(buf, &_N_ColorMatrix_##x, _N_ColorMatrix_##x##_len); \
    buf += _N_ColorMatrix_##x##_len


static uint8_t * addBranch(uint8_t *buf, const uint8_t *target, uint32_t condition) {
    size_t off = (target - buf - 8) >> 2;
    rsAssert(((off & 0xff000000) == 0) ||
           ((off & 0xff000000) == 0xff000000));

    uint32_t op = (condition << 28);
    op |= 0xa << 24;  // branch
    op |= 0xffffff & off;
    ((uint32_t *)buf)[0] = op;
    return buf + 4;
}

static uint32_t encodeSIMDRegs(uint32_t vd, uint32_t vn, uint32_t vm, bool q, bool sz) {
    rsAssert(vd < 32);
    rsAssert(vm < 32);
    rsAssert(vn < 32);

    uint32_t op = ((vd & 0xf) << 12) | (((vd & 0x10) >> 4) << 22);
    op |= (vm & 0xf) | (((vm & 0x10) >> 4) << 5);
    op |= ((vn & 0xf) << 16) | (((vn & 0x10) >> 4) << 7);
    if (q) op |= 1 << 6;
    if (sz) op |= 1 << 8;
    return op;
}

static uint8_t * addVMLAL_S16(uint8_t *buf, uint32_t dest_q, uint32_t src_d1, uint32_t src_d2, uint32_t src_d2_s) {
    //vmlal.s16 Q#1, D#1, D#2[#]
    uint32_t op = 0xf2900240 | encodeSIMDRegs(dest_q << 1, src_d1, src_d2 | (src_d2_s << 3), false, false);
    ((uint32_t *)buf)[0] = op;
    return buf + 4;
}

static uint8_t * addVMULL_S16(uint8_t *buf, uint32_t dest_q, uint32_t src_d1, uint32_t src_d2, uint32_t src_d2_s) {
    //vmull.s16 Q#1, D#1, D#2[#]
    uint32_t op = 0xf2900A40 | encodeSIMDRegs(dest_q << 1, src_d1, src_d2 | (src_d2_s << 3), false, false);
    ((uint32_t *)buf)[0] = op;
    return buf + 4;
}
#endif


bool RsdCpuScriptIntrinsicColorMatrix::build(int64_t key) {
#if defined(ARCH_ARM_HAVE_NEON)
    mBufSize = 4096;
    //StopWatch build_time("rs cm: build time");
    mBuf = (uint8_t *)mmap(0, mBufSize, PROT_READ | PROT_WRITE,
                                  MAP_PRIVATE | MAP_ANON, -1, 0);
    if (!mBuf) {
        return false;
    }

    uint8_t *buf = mBuf;

    // Add the function prefix
    // Store the address for the loop return
    ADD_CHUNK(prefix);
    uint8_t *buf2 = buf;

    // Load the incoming r,g,b,a as needed
    switch(key & CM_IN_VEC_SIZE_MASK) {
    case 3:
        ADD_CHUNK(load_u8_4);
        if (key & CM_COPY_ALPHA) {
            ADD_CHUNK(unpack_u8_3);
        } else {
            ADD_CHUNK(unpack_u8_4);
        }
        break;
    case 2:
        ADD_CHUNK(load_u8_4);
        ADD_CHUNK(unpack_u8_3);
        break;
    case 1:
        ADD_CHUNK(load_u8_2);
        ADD_CHUNK(unpack_u8_2);
        break;
    case 0:
        ADD_CHUNK(load_u8_1);
        ADD_CHUNK(unpack_u8_1);
        break;
    }

    // Add multiply and accumulate
    // use MULL to init the output register,
    // use MLAL from there
    bool linit[4] = {false, false, false, false};
    if (key & (1 << 16)) {
        buf = addVMULL_S16(buf, 8, 24, 4, 0);
        linit[0] = true;
    }
    if (!(key & CM_DOT_MASK)) {
        if (key & (1 << 17)) {
            buf = addVMULL_S16(buf, 9, 24, 4, 1);
            linit[1] = true;
        }
        if (key & (1 << 18)) {
            buf = addVMULL_S16(buf, 10, 24, 4, 2);
            linit[2] = true;
        }
    }
    if (key & (1 << 19)) {
        buf = addVMULL_S16(buf, 11, 24, 4, 3);
        linit[3] = true;
    }

    if (key & (1 << 20)) {
        if (linit[0]) {
            buf = addVMLAL_S16(buf, 8, 26, 5, 0);
        } else {
            buf = addVMULL_S16(buf, 8, 26, 5, 0);
            linit[0] = true;
        }
    }
    if (!(key & CM_DOT_MASK)) {
        if (key & (1 << 21)) {
            if (linit[1]) {
                buf = addVMLAL_S16(buf, 9, 26, 5, 1);
            } else {
                buf = addVMULL_S16(buf, 9, 26, 5, 1);
                linit[1] = true;
            }
        }
        if (key & (1 << 22)) {
            if (linit[2]) {
                buf = addVMLAL_S16(buf, 10, 26, 5, 2);
            } else {
                buf = addVMULL_S16(buf, 10, 26, 5, 2);
                linit[2] = true;
            }
        }
    }
    if (key & (1 << 23)) {
        if (linit[3]) {
            buf = addVMLAL_S16(buf, 11, 26, 5, 3);
        } else {
            buf = addVMULL_S16(buf, 11, 26, 5, 3);
            linit[3] = true;
        }
    }

    if (key & (1 << 24)) {
        if (linit[0]) {
            buf = addVMLAL_S16(buf, 8, 28, 6, 0);
        } else {
            buf = addVMULL_S16(buf, 8, 28, 6, 0);
            linit[0] = true;
        }
    }
    if (!(key & CM_DOT_MASK)) {
        if (key & (1 << 25)) {
            if (linit[1]) {
                buf = addVMLAL_S16(buf, 9, 28, 6, 1);
            } else {
                buf = addVMULL_S16(buf, 9, 28, 6, 1);
                linit[1] = true;
            }
        }
        if (key & (1 << 26)) {
            if (linit[2]) {
                buf = addVMLAL_S16(buf, 10, 28, 6, 2);
            } else {
                buf = addVMULL_S16(buf, 10, 28, 6, 2);
                linit[2] = true;
            }
        }
    }
    if (key & (1 << 27)) {
        if (linit[3]) {
            buf = addVMLAL_S16(buf, 11, 28, 6, 3);
        } else {
            buf = addVMULL_S16(buf, 11, 28, 6, 3);
            linit[3] = true;
        }
    }

    if (key & (1 << 28)) {
        if (linit[0]) {
            buf = addVMLAL_S16(buf, 8, 30, 7, 0);
        } else {
            buf = addVMULL_S16(buf, 8, 30, 7, 0);
            linit[0] = true;
        }
    }
    if (!(key & CM_DOT_MASK)) {
        if (key & (1 << 29)) {
            if (linit[1]) {
                buf = addVMLAL_S16(buf, 9, 30, 7, 1);
            } else {
                buf = addVMULL_S16(buf, 9, 30, 7, 1);
                linit[1] = true;
            }
        }
        if (key & (1 << 30)) {
            if (linit[2]) {
                buf = addVMLAL_S16(buf, 10, 30, 7, 2);
            } else {
                buf = addVMULL_S16(buf, 10, 30, 7, 2);
                linit[2] = true;
            }
        }
    }
    if (!(key & CM_COPY_ALPHA)) {
        if (key & (1 << 31)) {
            if (linit[3]) {
                buf = addVMLAL_S16(buf, 11, 30, 7, 3);
            } else {
                buf = addVMULL_S16(buf, 11, 30, 7, 3);
                linit[3] = true;
            }
        }
    }

    // If we have a dot product, perform the special pack.
    if (key & CM_DOT_MASK) {
        ADD_CHUNK(pack_u8_1);
        ADD_CHUNK(dot);
    } else {
        switch(key & CM_IN_VEC_SIZE_MASK) {
        case 3:
            ADD_CHUNK(pack_u8_4);
            break;
        case 2:
            ADD_CHUNK(pack_u8_3);
            break;
        case 1:
            ADD_CHUNK(pack_u8_2);
            break;
        case 0:
            ADD_CHUNK(pack_u8_1);
            break;
        }
    }

    // Write out result
    switch(key & CM_IN_VEC_SIZE_MASK) {
    case 3:
    case 2:
        ADD_CHUNK(store_u8_4);
        break;
    case 1:
        ADD_CHUNK(store_u8_2);
        break;
    case 0:
        ADD_CHUNK(store_u8_1);
        break;
    }

    // Loop, branch, and cleanup
    ADD_CHUNK(postfix1);
    buf = addBranch(buf, buf2, 0x01);
    ADD_CHUNK(postfix2);

    int ret = mprotect(mBuf, mBufSize, PROT_READ | PROT_EXEC);
    if (ret == -1) {
        ALOGE("mprotect error %i", ret);
        return false;
    }

    cacheflush((long)mBuf, (long)mBuf + mBufSize, 0);
    return true;
#else
    return false;
#endif
}

void RsdCpuScriptIntrinsicColorMatrix::setGlobalVar(uint32_t slot, const void *data,
                                                    size_t dataLength) {
    switch(slot) {
    case 0:
        memcpy (fp, data, dataLength);
        for(int ct=0; ct < 16; ct++) {
            ip[ct] = (short)(fp[ct] * 256.f + 0.5f);
        }
        break;
    case 1:
        memcpy (fpa, data, dataLength);
        for(int ct=0; ct < 4; ct++) {
            ipa[ct] = (int)(fpa[ct] * 256.f + 0.5f);
        }
        break;
    default:
        rsAssert(0);
        break;
    }

    mRootPtr = &kernel;
}


static void One(const RsForEachStubParamStruct *p, uchar4 *out,
                const uchar4 *py, const float* coeff) {
    float4 i = convert_float4(py[0]);

    float4 sum;
    sum.x = i.x * coeff[0] +
            i.y * coeff[4] +
            i.z * coeff[8] +
            i.w * coeff[12];
    sum.y = i.x * coeff[1] +
            i.y * coeff[5] +
            i.z * coeff[9] +
            i.w * coeff[13];
    sum.z = i.x * coeff[2] +
            i.y * coeff[6] +
            i.z * coeff[10] +
            i.w * coeff[14];
    sum.w = i.x * coeff[3] +
            i.y * coeff[7] +
            i.z * coeff[11] +
            i.w * coeff[15];

    sum.x = sum.x < 0 ? 0 : (sum.x > 255 ? 255 : sum.x);
    sum.y = sum.y < 0 ? 0 : (sum.y > 255 ? 255 : sum.y);
    sum.z = sum.z < 0 ? 0 : (sum.z > 255 ? 255 : sum.z);
    sum.w = sum.w < 0 ? 0 : (sum.w > 255 ? 255 : sum.w);

    *out = convert_uchar4(sum);
}

void RsdCpuScriptIntrinsicColorMatrix::kernel(const RsForEachStubParamStruct *p,
                                              uint32_t xstart, uint32_t xend,
                                              uint32_t instep, uint32_t outstep) {
    RsdCpuScriptIntrinsicColorMatrix *cp = (RsdCpuScriptIntrinsicColorMatrix *)p->usr;
    uchar4 *out = (uchar4 *)p->out;
    uchar4 *in = (uchar4 *)p->in;
    uint32_t x1 = xstart;
    uint32_t x2 = xend;

    if(x2 > x1) {
        int32_t len = (x2 - x1) >> 2;
        if((cp->mOptKernel != NULL) && (len > 0)) {
            cp->mOptKernel(out, in, cp->ip, len);
            x1 += len << 2;
            out += len << 2;
            in += len << 2;
        }

        while(x1 != x2) {
            One(p, out++, in++, cp->fp);
            x1++;
        }
    }
}

void RsdCpuScriptIntrinsicColorMatrix::preLaunch(
        uint32_t slot, const Allocation * ain, Allocation * aout,
        const void * usr, uint32_t usrLen, const RsScriptCall *sc) {

    int64_t key = computeKey(ain->mHal.state.type->getElement(),
                             aout->mHal.state.type->getElement());
    if ((mOptKernel == NULL) || (mLastKey != key)) {
        if (mBuf) munmap(mBuf, mBufSize);
        mBuf = NULL;
        mOptKernel = NULL;
        if (build(key)) {
            mOptKernel = (void (*)(void *, const void *, const short *, uint32_t)) mBuf;
            mLastKey = key;
        }
    }
}

void RsdCpuScriptIntrinsicColorMatrix::postLaunch(
        uint32_t slot, const Allocation * ain, Allocation * aout,
        const void * usr, uint32_t usrLen, const RsScriptCall *sc) {


}

RsdCpuScriptIntrinsicColorMatrix::RsdCpuScriptIntrinsicColorMatrix(
            RsdCpuReferenceImpl *ctx, const Script *s, const Element *e)
            : RsdCpuScriptIntrinsic(ctx, s, e, RS_SCRIPT_INTRINSIC_ID_COLOR_MATRIX) {

    mLastKey = 0;
    mBuf = NULL;
    mBufSize = 0;
    mOptKernel = NULL;
    const static float defaultMatrix[] = {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
    const static float defaultAdd[] = {0.f, 0.f, 0.f, 0.f};
    setGlobalVar(0, defaultMatrix, sizeof(defaultMatrix));
    setGlobalVar(1, defaultAdd, sizeof(defaultAdd));
}

RsdCpuScriptIntrinsicColorMatrix::~RsdCpuScriptIntrinsicColorMatrix() {
    if (mBuf) munmap(mBuf, mBufSize);
    mBuf = NULL;
    mOptKernel = NULL;
}

void RsdCpuScriptIntrinsicColorMatrix::populateScript(Script *s) {
    s->mHal.info.exportedVariableCount = 2;
}

RsdCpuScriptImpl * rsdIntrinsic_ColorMatrix(RsdCpuReferenceImpl *ctx,
                                            const Script *s, const Element *e) {

    return new RsdCpuScriptIntrinsicColorMatrix(ctx, s, e);
}



