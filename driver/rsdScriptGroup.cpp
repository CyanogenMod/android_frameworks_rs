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

#include "rsdCore.h"

#include <bcc/BCCContext.h>
#include <bcc/Renderscript/RSCompilerDriver.h>
#include <bcc/Renderscript/RSExecutable.h>
#include <bcc/Renderscript/RSInfo.h>

#include "rsScript.h"
#include "rsScriptGroup.h"
#include "rsdScriptGroup.h"
#include "rsdBcc.h"
#include "rsdAllocation.h"

using namespace android;
using namespace android::renderscript;


bool rsdScriptGroupInit(const android::renderscript::Context *rsc,
                        const android::renderscript::ScriptGroup *sg) {
    return true;
}

void rsdScriptGroupSetInput(const android::renderscript::Context *rsc,
                            const android::renderscript::ScriptGroup *sg,
                            const android::renderscript::ScriptKernelID *kid,
                            android::renderscript::Allocation *) {
}

void rsdScriptGroupSetOutput(const android::renderscript::Context *rsc,
                             const android::renderscript::ScriptGroup *sg,
                             const android::renderscript::ScriptKernelID *kid,
                             android::renderscript::Allocation *) {
}

struct ScriptList {
    size_t count;
    Allocation *const* ins;
    bool const* inExts;
    Allocation *const* outs;
    bool const* outExts;
    const void *const* usrPtrs;
    size_t const *usrSizes;
    uint32_t const *sigs;
    const void *const* fnPtrs;

    const ScriptKernelID *const* kernels;
};

typedef void (*ScriptGroupRootFunc_t)(const RsForEachStubParamStruct *p,
                                      uint32_t xstart, uint32_t xend,
                                      uint32_t instep, uint32_t outstep);

static void ScriptGroupRoot(const RsForEachStubParamStruct *p,
                            uint32_t xstart, uint32_t xend,
                            uint32_t instep, uint32_t outstep) {

    const ScriptList *sl = (const ScriptList *)p->usr;
    RsForEachStubParamStruct *mp = (RsForEachStubParamStruct *)p;
    const void *oldUsr = p->usr;

    for(size_t ct=0; ct < sl->count; ct++) {
        ScriptGroupRootFunc_t func;
        func = (ScriptGroupRootFunc_t)sl->fnPtrs[ct];
        mp->usr = sl->usrPtrs[ct];

        mp->ptrIn = NULL;
        mp->in = NULL;
        mp->ptrOut = NULL;
        mp->out = NULL;

        if (sl->ins[ct]) {
            DrvAllocation *drv = (DrvAllocation *)sl->ins[ct]->mHal.drv;
            mp->ptrIn = (const uint8_t *)drv->lod[0].mallocPtr;
            mp->in = mp->ptrIn;
            if (sl->inExts[ct]) {
                mp->in = mp->ptrIn + drv->lod[0].stride * p->y;
            } else {
                if (drv->lod[0].dimY > p->lid) {
                    mp->in = mp->ptrIn + drv->lod[0].stride * p->lid;
                }
            }
        }

        if (sl->outs[ct]) {
            DrvAllocation *drv = (DrvAllocation *)sl->outs[ct]->mHal.drv;
            mp->ptrOut = (uint8_t *)drv->lod[0].mallocPtr;
            mp->out = mp->ptrOut;
            if (sl->outExts[ct]) {
                mp->out = mp->ptrOut + drv->lod[0].stride * p->y;
            } else {
                if (drv->lod[0].dimY > p->lid) {
                    mp->out = mp->ptrOut + drv->lod[0].stride * p->lid;
                }
            }
        }

        //ALOGE("kernel %i %p,%p  %p,%p", ct, mp->ptrIn, mp->in, mp->ptrOut, mp->out);
        func(p, xstart, xend, instep, outstep);
    }
    //ALOGE("script group root");

    //ConvolveParams *cp = (ConvolveParams *)p->usr;

    mp->usr = oldUsr;
}


void rsdScriptGroupExecute(const android::renderscript::Context *rsc,
                           const android::renderscript::ScriptGroup *sg) {

    Vector<Allocation *> ins;
    Vector<bool> inExts;
    Vector<Allocation *> outs;
    Vector<bool> outExts;
    Vector<const ScriptKernelID *> kernels;
    bool fieldDep = false;

    for (size_t ct=0; ct < sg->mNodes.size(); ct++) {
        ScriptGroup::Node *n = sg->mNodes[ct];
        Script *s = n->mKernels[0]->mScript;

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
                    //ALOGE(" link in %p", ain);
                }
            }
            for (size_t ct3=0; ct3 < sg->mInputs.size(); ct3++) {
                if (sg->mInputs[ct3]->mKernel == k) {
                    ain = sg->mInputs[ct3]->mAlloc.get();
                    inExt = true;
                    //ALOGE(" io in %p", ain);
                }
            }

            for (size_t ct3=0; ct3 < n->mOutputs.size(); ct3++) {
                if (n->mOutputs[ct3]->mSource.get() == k) {
                    aout = n->mOutputs[ct3]->mAlloc.get();
                    if(n->mOutputs[ct3]->mDstField.get() != NULL) {
                        fieldDep = true;
                    }
                    //ALOGE(" link out %p", aout);
                }
            }
            for (size_t ct3=0; ct3 < sg->mOutputs.size(); ct3++) {
                if (sg->mOutputs[ct3]->mKernel == k) {
                    aout = sg->mOutputs[ct3]->mAlloc.get();
                    outExt = true;
                    //ALOGE(" io out %p", aout);
                }
            }

            if ((k->mHasKernelOutput == (aout != NULL)) &&
                (k->mHasKernelInput == (ain != NULL))) {
                ins.add(ain);
                inExts.add(inExt);
                outs.add(aout);
                outExts.add(outExt);
                kernels.add(k);
            }
        }

    }

    RsdHal * dc = (RsdHal *)rsc->mHal.drv;
    MTLaunchStruct mtls;

    if(fieldDep) {
        for (size_t ct=0; ct < ins.size(); ct++) {
            Script *s = kernels[ct]->mScript;
            DrvScript *drv = (DrvScript *)s->mHal.drv;
            uint32_t slot = kernels[ct]->mSlot;

            rsdScriptInvokeForEachMtlsSetup(rsc, ins[ct], outs[ct], NULL, 0, NULL, &mtls);
            mtls.script = s;

            if (drv->mIntrinsicID) {
                mtls.kernel = (void (*)())drv->mIntrinsicFuncs.root;
                mtls.fep.usr = drv->mIntrinsicData;
            } else {
                mtls.kernel = reinterpret_cast<ForEachFunc_t>(
                                  drv->mExecutable->getExportForeachFuncAddrs()[slot]);
                rsAssert(mtls.kernel != NULL);
                mtls.sig = drv->mExecutable->getInfo().getExportForeachFuncs()[slot].second;
            }

            rsdScriptLaunchThreads(rsc, s->mHal.info.isThreadable, ins[ct], outs[ct],
                                   NULL, 0, NULL, &mtls);
        }
    } else {
        ScriptList sl;
        sl.ins = ins.array();
        sl.outs = outs.array();
        sl.kernels = kernels.array();
        sl.count = kernels.size();

        Vector<const void *> usrPtrs;
        Vector<const void *> fnPtrs;
        Vector<uint32_t> sigs;
        for (size_t ct=0; ct < kernels.size(); ct++) {
            Script *s = kernels[ct]->mScript;
            DrvScript *drv = (DrvScript *)s->mHal.drv;

            if (drv->mIntrinsicID) {
                fnPtrs.add((void *)drv->mIntrinsicFuncs.root);
                usrPtrs.add(drv->mIntrinsicData);
                sigs.add(0);
            } else {
                int slot = kernels[ct]->mSlot;
                fnPtrs.add((void *)drv->mExecutable->getExportForeachFuncAddrs()[slot]);
                usrPtrs.add(NULL);
                sigs.add(drv->mExecutable->getInfo().getExportForeachFuncs()[slot].second);
            }
        }
        sl.sigs = sigs.array();
        sl.usrPtrs = usrPtrs.array();
        sl.fnPtrs = fnPtrs.array();
        sl.inExts = inExts.array();
        sl.outExts = outExts.array();

        rsdScriptInvokeForEachMtlsSetup(rsc, ins[0], outs[0], NULL, 0, NULL, &mtls);
        mtls.script = NULL;
        mtls.kernel = (void (*)())&ScriptGroupRoot;
        mtls.fep.usr = &sl;
        rsdScriptLaunchThreads(rsc, true, ins[0], outs[0], NULL, 0, NULL, &mtls);
    }

}

void rsdScriptGroupDestroy(const android::renderscript::Context *rsc,
                           const android::renderscript::ScriptGroup *sg) {
}


