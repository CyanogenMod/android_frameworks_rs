/*
 * Copyright (C) 2011 The Android Open Source Project
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

#include "rsCpuCore.h"
#include "rsCpuScript.h"
#include "rsScriptGroup.h"
#include "rsCpuScriptGroup.h"
//#include "rsdBcc.h"
//#include "rsdAllocation.h"

using namespace android;
using namespace android::renderscript;

CpuScriptGroupImpl::CpuScriptGroupImpl(RsdCpuReferenceImpl *ctx, const ScriptGroup *sg) {
    mCtx = ctx;
    mSG = sg;
}

CpuScriptGroupImpl::~CpuScriptGroupImpl() {

}

bool CpuScriptGroupImpl::init() {
    return true;
}

void CpuScriptGroupImpl::setInput(const ScriptKernelID *kid, Allocation *a) {
}

void CpuScriptGroupImpl::setOutput(const ScriptKernelID *kid, Allocation *a) {
}


typedef void (*ScriptGroupRootFunc_t)(const RsExpandKernelParams *kparams,
                                      uint32_t xstart, uint32_t xend,
                                      uint32_t outstep);

void CpuScriptGroupImpl::scriptGroupRoot(const RsExpandKernelParams *kparams,
                                         uint32_t xstart, uint32_t xend,
                                         uint32_t outstep) {


    const ScriptList *sl           = (const ScriptList *)kparams->usr;
    RsExpandKernelParams *mkparams = (RsExpandKernelParams *)kparams;

    const void **oldIns  = mkparams->ins;
    uint32_t *oldStrides = mkparams->inEStrides;

    void *localIns[1];
    uint32_t localStride[1];

    mkparams->ins        = (const void**)localIns;
    mkparams->inEStrides = localStride;

    for (size_t ct = 0; ct < sl->count; ct++) {
        ScriptGroupRootFunc_t func;
        func          = (ScriptGroupRootFunc_t)sl->fnPtrs[ct];
        mkparams->usr = sl->usrPtrs[ct];

        if (sl->ins[ct]) {
            localIns[0] = sl->ins[ct]->mHal.drvState.lod[0].mallocPtr;

            localStride[0] = sl->ins[ct]->mHal.state.elementSizeBytes;

            if (sl->inExts[ct]) {
                localIns[0] = (void*)
                  ((const uint8_t *)localIns[0] +
                   sl->ins[ct]->mHal.drvState.lod[0].stride * kparams->y);

            } else if (sl->ins[ct]->mHal.drvState.lod[0].dimY > kparams->lid) {
                localIns[0] = (void*)
                  ((const uint8_t *)localIns[0] +
                   sl->ins[ct]->mHal.drvState.lod[0].stride * kparams->lid);
            }

        } else {
            localIns[0]    = NULL;
            localStride[0] = 0;
        }

        uint32_t ostep;
        if (sl->outs[ct]) {
            mkparams->out =
              (uint8_t *)sl->outs[ct]->mHal.drvState.lod[0].mallocPtr;

            ostep = sl->outs[ct]->mHal.state.elementSizeBytes;

            if (sl->outExts[ct]) {
                mkparams->out =
                  (uint8_t *)mkparams->out +
                  sl->outs[ct]->mHal.drvState.lod[0].stride * kparams->y;

            } else if (sl->outs[ct]->mHal.drvState.lod[0].dimY > kparams->lid) {
                mkparams->out =
                  (uint8_t *)mkparams->out +
                  sl->outs[ct]->mHal.drvState.lod[0].stride * kparams->lid;
            }
        } else {
            mkparams->out = NULL;
            ostep         = 0;
        }

        //ALOGE("kernel %i %p,%p  %p,%p", ct, mp->ptrIn, mp->in, mp->ptrOut, mp->out);
        func(kparams, xstart, xend, ostep);
    }
    //ALOGE("script group root");

    mkparams->ins        = oldIns;
    mkparams->inEStrides = oldStrides;
    mkparams->usr        = sl;
}



void CpuScriptGroupImpl::execute() {
    std::vector<Allocation *> ins;
    std::vector<char> inExts;
    std::vector<Allocation *> outs;
    std::vector<char> outExts;
    std::vector<const ScriptKernelID *> kernels;
    bool fieldDep = false;

    for (size_t ct=0; ct < mSG->mNodes.size(); ct++) {
        ScriptGroup::Node *n = mSG->mNodes[ct];
        Script *s = n->mKernels[0]->mScript;
        if (s->hasObjectSlots()) {
            // Disable the ScriptGroup optimization if we have global RS
            // objects that might interfere between kernels.
            fieldDep = true;
        }

        //ALOGE("node %i, order %i, in %i out %i", (int)ct, n->mOrder, (int)n->mInputs.size(), (int)n->mOutputs.size());

        for (size_t ct2=0; ct2 < n->mInputs.size(); ct2++) {
            if (n->mInputs[ct2]->mDstField.get() && n->mInputs[ct2]->mDstField->mScript) {
                //ALOGE("field %p %zu", n->mInputs[ct2]->mDstField->mScript, n->mInputs[ct2]->mDstField->mSlot);
                s->setVarObj(n->mInputs[ct2]->mDstField->mSlot, n->mInputs[ct2]->mAlloc.get());
            }
        }

        for (size_t ct2=0; ct2 < n->mKernels.size(); ct2++) {
            const ScriptKernelID *k = n->mKernels[ct2];
            Allocation *ain = NULL;
            Allocation *aout = NULL;
            bool inExt = false;
            bool outExt = false;

            for (size_t ct3=0; ct3 < n->mInputs.size(); ct3++) {
                if (n->mInputs[ct3]->mDstKernel.get() == k) {
                    ain = n->mInputs[ct3]->mAlloc.get();
                    break;
                }
            }
            if (ain == NULL) {
                for (size_t ct3=0; ct3 < mSG->mInputs.size(); ct3++) {
                    if (mSG->mInputs[ct3]->mKernel == k) {
                        ain = mSG->mInputs[ct3]->mAlloc.get();
                        inExt = true;
                        break;
                    }
                }
            }

            for (size_t ct3=0; ct3 < n->mOutputs.size(); ct3++) {
                if (n->mOutputs[ct3]->mSource.get() == k) {
                    aout = n->mOutputs[ct3]->mAlloc.get();
                    if(n->mOutputs[ct3]->mDstField.get() != NULL) {
                        fieldDep = true;
                    }
                    break;
                }
            }
            if (aout == NULL) {
                for (size_t ct3=0; ct3 < mSG->mOutputs.size(); ct3++) {
                    if (mSG->mOutputs[ct3]->mKernel == k) {
                        aout = mSG->mOutputs[ct3]->mAlloc.get();
                        outExt = true;
                        break;
                    }
                }
            }

            rsAssert((k->mHasKernelOutput == (aout != NULL)) &&
                     (k->mHasKernelInput == (ain != NULL)));

            ins.push_back(ain);
            inExts.push_back(inExt);
            outs.push_back(aout);
            outExts.push_back(outExt);
            kernels.push_back(k);
        }

    }

    MTLaunchStruct mtls;

    if (fieldDep) {
        for (size_t ct=0; ct < ins.size(); ct++) {
            Script *s = kernels[ct]->mScript;
            RsdCpuScriptImpl *si = (RsdCpuScriptImpl *)mCtx->lookupScript(s);
            uint32_t slot = kernels[ct]->mSlot;

            uint32_t inLen;
            const Allocation **ains;

            if (ins[ct] == NULL) {
                inLen = 0;
                ains  = NULL;

            } else {
                inLen = 1;
                ains  = const_cast<const Allocation**>(&ins[ct]);
            }

            si->forEachMtlsSetup(ains, inLen, outs[ct], NULL, 0, NULL, &mtls);

            si->forEachKernelSetup(slot, &mtls);
            si->preLaunch(slot, ains, inLen, outs[ct], mtls.fep.usr,
                          mtls.fep.usrLen, NULL);

            mCtx->launchThreads(ains, inLen, outs[ct], NULL, &mtls);

            si->postLaunch(slot, ains, inLen, outs[ct], NULL, 0, NULL);
        }
    } else {
        ScriptList sl;

        /*
         * TODO: This is a hacky way of doing this and should be replaced by a
         *       call to std::vector's data() member once we have a C++11
         *       version of the STL.
         */
        sl.ins     = &ins.front();
        sl.outs    = &outs.front();
        sl.kernels = &kernels.front();
        sl.count   = kernels.size();

        uint32_t inLen;
        const Allocation **ains;

        if (ins[0] == NULL) {
            inLen = 0;
            ains  = NULL;

        } else {
            inLen = 1;
            ains  = const_cast<const Allocation**>(&ins[0]);
        }

        std::vector<const void *> usrPtrs;
        std::vector<const void *> fnPtrs;
        std::vector<uint32_t> sigs;
        for (size_t ct=0; ct < kernels.size(); ct++) {
            Script *s = kernels[ct]->mScript;
            RsdCpuScriptImpl *si = (RsdCpuScriptImpl *)mCtx->lookupScript(s);

            si->forEachKernelSetup(kernels[ct]->mSlot, &mtls);
            fnPtrs.push_back((void *)mtls.kernel);
            usrPtrs.push_back(mtls.fep.usr);
            sigs.push_back(mtls.fep.usrLen);
            si->preLaunch(kernels[ct]->mSlot, ains, inLen, outs[ct],
                          mtls.fep.usr, mtls.fep.usrLen, NULL);
        }

        sl.sigs    = &sigs.front();
        sl.usrPtrs = &usrPtrs.front();
        sl.fnPtrs  = &fnPtrs.front();

        sl.inExts  = (bool*)&inExts.front();
        sl.outExts = (bool*)&outExts.front();

        Script *s = kernels[0]->mScript;
        RsdCpuScriptImpl *si = (RsdCpuScriptImpl *)mCtx->lookupScript(s);

        si->forEachMtlsSetup(ains, inLen, outs[0], NULL, 0, NULL, &mtls);

        mtls.script = NULL;
        mtls.kernel = (void (*)())&scriptGroupRoot;
        mtls.fep.usr = &sl;

        mCtx->launchThreads(ains, inLen, outs[0], NULL, &mtls);

        for (size_t ct=0; ct < kernels.size(); ct++) {
            Script *s = kernels[ct]->mScript;
            RsdCpuScriptImpl *si = (RsdCpuScriptImpl *)mCtx->lookupScript(s);
            si->postLaunch(kernels[ct]->mSlot, ains, inLen, outs[ct], NULL, 0,
                           NULL);
        }
    }
}
