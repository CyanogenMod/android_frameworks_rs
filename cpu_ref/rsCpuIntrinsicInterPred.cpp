#include "rsCpuIntrinsicInterPred.h"

void RsdCpuScriptIntrinsicInterPred::setGlobalObj(uint32_t slot,
                                                  ObjectBase *data) {
    Allocation *alloc = static_cast<Allocation *>(data);
    if (slot == 0) mRef = (uint8_t *)alloc->mHal.state.userProvidedPtr;
    if (slot == 1) mParam = (uint8_t *)alloc->mHal.state.userProvidedPtr;
}

void RsdCpuScriptIntrinsicInterPred::setGlobalVar(uint32_t slot,
                                                  const void *data,
                                                  size_t dataLength) {
    mFriParamCount = ((int32_t *)data)[0];
    mSecParamCount = ((int32_t *)data)[1];
    mParamOffset   = ((int32_t *)data)[2];
}

void RsdCpuScriptIntrinsicInterPred::kernel(const RsForEachStubParamStruct *p,
                                            uint32_t xstart, uint32_t xend,
                                            uint32_t instep, uint32_t outstep) {
    RsdCpuScriptIntrinsicInterPred *cp = (RsdCpuScriptIntrinsicInterPred *)p->usr;
    cp->mCount++;
    const int vp9_convolve_mode[2][2] = {{24, 16}, {8, 0}};
    uint8_t *ref_base = cp->mRef;
    INTER_PRED_PARAM *fri_param = (INTER_PRED_PARAM *)cp->mParam;
    INTER_PRED_PARAM *sec_param = (INTER_PRED_PARAM *)(cp->mParam + cp->mParamOffset);
    int32_t fri_count = cp->mFriParamCount;
    int32_t sec_count = cp->mSecParamCount;
    int mode_num;
    uint8_t *src;
    uint8_t *dst;
    const int16_t *filter_x;
    const int16_t *filter_y;
    for (int i = 0; i < fri_count; i++) {

        mode_num = vp9_convolve_mode[(fri_param[i].x_step_q4 == 16)]
                                    [(fri_param[i].y_step_q4 == 16)];
        src = ref_base + fri_param[i].src_mv;
        dst = ref_base + fri_param[i].dst_mv;

        filter_x = inter_pred_filters + fri_param[i].filter_x_mv;
        filter_y = inter_pred_filters + fri_param[i].filter_y_mv;

        cp->mSwitchConvolve[fri_param[i].pred_mode + mode_num](
            src, fri_param[i].src_stride,
            dst, fri_param[i].dst_stride,
            filter_x, fri_param[i].x_step_q4,
            filter_y, fri_param[i].y_step_q4,
            fri_param[i].w, fri_param[i].h
        );
    }

    for (int i = 0; i < sec_count; i++) {
        mode_num = vp9_convolve_mode[(sec_param[i].x_step_q4 == 16)]
                                    [(sec_param[i].y_step_q4 == 16)];
        src = ref_base + sec_param[i].src_mv;
        dst = ref_base + sec_param[i].dst_mv;

        filter_x = inter_pred_filters + sec_param[i].filter_x_mv;
        filter_y = inter_pred_filters + sec_param[i].filter_y_mv;

        cp->mSwitchConvolve[sec_param[i].pred_mode + mode_num + 1](
            src, sec_param[i].src_stride,
            dst, sec_param[i].dst_stride,
            filter_x, sec_param[i].x_step_q4,
            filter_y, sec_param[i].y_step_q4,
            sec_param[i].w, sec_param[i].h
        );
    }

}

RsdCpuScriptIntrinsicInterPred::RsdCpuScriptIntrinsicInterPred(RsdCpuReferenceImpl *ctx,
                                                               const Script *s, const Element *e)
            : RsdCpuScriptIntrinsic(ctx, s, e, RS_SCRIPT_INTRINSIC_ID_INTER_PRED) {
    mRootPtr = &kernel;
    mCount = 0;
    mParamOffset = 0;
    mFriParamCount = 0;
    mSecParamCount = 0;
    mRef = NULL;
    mParam = NULL;

#if defined(ARCH_ARM_HAVE_VFP)
    mSwitchConvolve[0] = vp9_convolve_copy_neon;
    mSwitchConvolve[1] = vp9_convolve_avg_neon;
    mSwitchConvolve[2] = vp9_convolve8_vert_neon;
    mSwitchConvolve[3] = vp9_convolve8_avg_vert_neon;
    mSwitchConvolve[4] = vp9_convolve8_horiz_neon;
    mSwitchConvolve[5] = vp9_convolve8_avg_horiz_neon;
    mSwitchConvolve[6] = vp9_convolve8_neon;
    mSwitchConvolve[7] = vp9_convolve8_avg_neon;

    mSwitchConvolve[8] = vp9_convolve8_vert_neon;
    mSwitchConvolve[9] = vp9_convolve8_avg_vert_neon;
    mSwitchConvolve[10] = vp9_convolve8_vert_neon;
    mSwitchConvolve[11] = vp9_convolve8_avg_vert_neon;
    mSwitchConvolve[12] = vp9_convolve8_neon;
    mSwitchConvolve[13] = vp9_convolve8_avg_neon;
    mSwitchConvolve[14] = vp9_convolve8_neon;
    mSwitchConvolve[15] = vp9_convolve8_avg_neon;

    mSwitchConvolve[16] = vp9_convolve8_horiz_neon;
    mSwitchConvolve[17] = vp9_convolve8_avg_horiz_neon;
    mSwitchConvolve[18] = vp9_convolve8_neon;
    mSwitchConvolve[19] = vp9_convolve8_avg_neon;
    mSwitchConvolve[20] = vp9_convolve8_horiz_neon;
    mSwitchConvolve[21] = vp9_convolve8_avg_horiz_neon;
    mSwitchConvolve[22] = vp9_convolve8_neon;
    mSwitchConvolve[23] = vp9_convolve8_avg_neon;

    mSwitchConvolve[24] = vp9_convolve8_neon;
    mSwitchConvolve[25] = vp9_convolve8_avg_neon;
    mSwitchConvolve[26] = vp9_convolve8_neon;
    mSwitchConvolve[27] = vp9_convolve8_avg_neon;
    mSwitchConvolve[28] = vp9_convolve8_neon;
    mSwitchConvolve[29] = vp9_convolve8_avg_neon;
    mSwitchConvolve[30] = vp9_convolve8_neon;
    mSwitchConvolve[31] = vp9_convolve8_avg_neon;
#else
    mSwitchConvolve[0] = vp9_convolve_copy_c;
    mSwitchConvolve[1] = vp9_convolve_avg_c;
    mSwitchConvolve[2] = vp9_convolve8_vert_c;
    mSwitchConvolve[3] = vp9_convolve8_avg_vert_c;
    mSwitchConvolve[4] = vp9_convolve8_horiz_c;
    mSwitchConvolve[5] = vp9_convolve8_avg_horiz_c;
    mSwitchConvolve[6] = vp9_convolve8_c;
    mSwitchConvolve[7] = vp9_convolve8_avg_c;

    mSwitchConvolve[8] = vp9_convolve8_vert_c;
    mSwitchConvolve[9] = vp9_convolve8_avg_vert_c;
    mSwitchConvolve[10] = vp9_convolve8_vert_c;
    mSwitchConvolve[11] = vp9_convolve8_avg_vert_c;
    mSwitchConvolve[12] = vp9_convolve8_c;
    mSwitchConvolve[13] = vp9_convolve8_avg_c;
    mSwitchConvolve[14] = vp9_convolve8_c;
    mSwitchConvolve[15] = vp9_convolve8_avg_c;

    mSwitchConvolve[16] = vp9_convolve8_horiz_c;
    mSwitchConvolve[17] = vp9_convolve8_avg_horiz_c;
    mSwitchConvolve[18] = vp9_convolve8_c;
    mSwitchConvolve[19] = vp9_convolve8_avg_c;
    mSwitchConvolve[20] = vp9_convolve8_horiz_c;
    mSwitchConvolve[21] = vp9_convolve8_avg_horiz_c;
    mSwitchConvolve[22] = vp9_convolve8_c;
    mSwitchConvolve[23] = vp9_convolve8_avg_c;

    mSwitchConvolve[24] = vp9_convolve8_c;
    mSwitchConvolve[25] = vp9_convolve8_avg_c;
    mSwitchConvolve[26] = vp9_convolve8_c;
    mSwitchConvolve[27] = vp9_convolve8_avg_c;
    mSwitchConvolve[28] = vp9_convolve8_c;
    mSwitchConvolve[29] = vp9_convolve8_avg_c;
    mSwitchConvolve[30] = vp9_convolve8_c;
    mSwitchConvolve[31] = vp9_convolve8_avg_c;
#endif
}

RsdCpuScriptIntrinsicInterPred::~RsdCpuScriptIntrinsicInterPred() {
}

void RsdCpuScriptIntrinsicInterPred::populateScript(Script *s) {
    s->mHal.info.exportedVariableCount = 3;
}

void RsdCpuScriptIntrinsicInterPred::invokeFreeChildren() {
}


RsdCpuScriptImpl * rsdIntrinsic_InterPred(RsdCpuReferenceImpl *ctx,
                                          const Script *s, const Element *e) {
    return new RsdCpuScriptIntrinsicInterPred(ctx, s, e);
}
