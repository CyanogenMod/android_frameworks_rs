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

#include <algorithm>

#include "rsContext.h"
#include <time.h>

using namespace android;
using namespace android::renderscript;

ScriptGroup::ScriptGroup(Context *rsc) : ObjectBase(rsc) {
}

ScriptGroup::~ScriptGroup() {
    if (mRSC->mHal.funcs.scriptgroup.destroy) {
        mRSC->mHal.funcs.scriptgroup.destroy(mRSC, this);
    }

    for (auto link : mLinks) {
        delete link;
    }
}

ScriptGroup::IO::IO(const ScriptKernelID *kid) {
    mKernel = kid;
}

ScriptGroup::Node::Node(Script *s) {
    mScript = s;
    mSeen = false;
    mOrder = 0;
}

ScriptGroup::Node * ScriptGroup::findNode(Script *s) const {
    for (auto node : mNodes) {
        for (auto kernelRef : node->mKernels) {
            if (kernelRef->mScript == s) {
                return node;
            }
        }
    }

    return NULL;
}

bool ScriptGroup::calcOrderRecurse(Node *node0, int depth) {
    node0->mSeen = true;
    if (node0->mOrder < depth) {
        node0->mOrder = depth;
    }
    bool ret = true;

    for (auto link : node0->mOutputs) {
        Node *node1 = NULL;
        if (link->mDstField.get()) {
            node1 = findNode(link->mDstField->mScript);
        } else {
            node1 = findNode(link->mDstKernel->mScript);
        }
        if (node1->mSeen) {
            return false;
        }
        ret &= calcOrderRecurse(node1, node0->mOrder + 1);
    }

    return ret;
}

bool ScriptGroup::calcOrder() {
    // Make nodes

    for (auto kernelRef : mKernels) {
        const ScriptKernelID *kernel = kernelRef.get();
        Node *node = findNode(kernel->mScript);
        if (node == NULL) {
            node = new Node(kernel->mScript);
            mNodes.push_back(node);
        }
        node->mKernels.push_back(kernel);
    }

    // add links
    for (auto link : mLinks) {
        Node *node = findNode(link->mSource->mScript);
        node->mOutputs.push_back(link);

        if (link->mDstKernel.get()) {
            node = findNode(link->mDstKernel->mScript);
            node->mInputs.push_back(link);
        } else {
            node = findNode(link->mDstField->mScript);
            node->mInputs.push_back(link);
        }
    }

    // Order nodes
    bool ret = true;
    for (auto n0 : mNodes) {
        if (n0->mInputs.size() == 0) {
            for (auto n1 : mNodes) {
                n1->mSeen = false;
            }
            ret &= calcOrderRecurse(n0, 1);
        }
    }

    for (auto kernelRef : mKernels) {
        const ScriptKernelID *kernel = kernelRef.get();
        const Node *node = findNode(kernel->mScript);

        if (kernel->mHasKernelOutput) {
            bool found = false;
            for (auto output : node->mOutputs) {
                if (output->mSource.get() == kernel) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                mOutputs.push_back(new IO(kernel));
            }
        }

        if (kernel->mHasKernelInput) {
            bool found = false;
            for (auto input : node->mInputs) {
                if (input->mDstKernel.get() == kernel) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                mInputs.push_back(new IO(kernel));
            }
        }
    }

    // sort
    std::stable_sort(mNodes.begin(), mNodes.end(),
                     [](const ScriptGroup::Node* lhs,
                        const ScriptGroup::Node* rhs) {
        return lhs->mOrder < rhs->mOrder;
    });

    return ret;
}

ScriptGroup * ScriptGroup::create(Context *rsc,
                           ScriptKernelID ** kernels, size_t kernelsSize,
                           ScriptKernelID ** src, size_t srcSize,
                           ScriptKernelID ** dstK, size_t dstKSize,
                           ScriptFieldID  ** dstF, size_t dstFSize,
                           const Type ** type, size_t typeSize) {

    size_t kernelCount = kernelsSize / sizeof(ScriptKernelID *);
    size_t linkCount = typeSize / sizeof(Type *);

    //ALOGE("ScriptGroup::create kernels=%i  links=%i", (int)kernelCount, (int)linkCount);


    // Start by counting unique kernel sources

    ScriptGroup *sg = new ScriptGroup(rsc);

    sg->mKernels.reserve(kernelCount);
    for (size_t ct=0; ct < kernelCount; ct++) {
        sg->mKernels.push_back(kernels[ct]);
    }

    sg->mLinks.reserve(linkCount);
    for (size_t ct=0; ct < linkCount; ct++) {
        Link *l = new Link();
        l->mType = type[ct];
        l->mSource = src[ct];
        l->mDstField = dstF[ct];
        l->mDstKernel = dstK[ct];
        sg->mLinks.push_back(l);
    }

    sg->calcOrder();

    // allocate links
    for (size_t ct=0; ct < sg->mNodes.size(); ct++) {
        const Node *n = sg->mNodes[ct];
        for (size_t ct2=0; ct2 < n->mOutputs.size(); ct2++) {
            Link *l = n->mOutputs[ct2];
            if (l->mAlloc.get()) {
                continue;
            }
            const ScriptKernelID *k = l->mSource.get();

            Allocation * alloc = Allocation::createAllocation(rsc,
                    l->mType.get(), RS_ALLOCATION_USAGE_SCRIPT);
            l->mAlloc = alloc;

            for (size_t ct3=ct2+1; ct3 < n->mOutputs.size(); ct3++) {
                if (n->mOutputs[ct3]->mSource.get() == l->mSource.get()) {
                    n->mOutputs[ct3]->mAlloc = alloc;
                }
            }
        }
    }

    if (rsc->mHal.funcs.scriptgroup.init) {
        rsc->mHal.funcs.scriptgroup.init(rsc, sg);
    }
    sg->incUserRef();
    return sg;
}

void ScriptGroup::setInput(Context *rsc, ScriptKernelID *kid, Allocation *a) {
    for (auto input : mInputs) {
        if (input->mKernel == kid) {
            input->mAlloc = a;

            if (rsc->mHal.funcs.scriptgroup.setInput) {
                rsc->mHal.funcs.scriptgroup.setInput(rsc, this, kid, a);
            }
            return;
        }
    }
    rsAssert(!"ScriptGroup:setInput kid not found");
}

void ScriptGroup::setOutput(Context *rsc, ScriptKernelID *kid, Allocation *a) {
    for (auto output : mOutputs) {
        if (output->mKernel == kid) {
            output->mAlloc = a;

            if (rsc->mHal.funcs.scriptgroup.setOutput) {
                rsc->mHal.funcs.scriptgroup.setOutput(rsc, this, kid, a);
            }
            return;
        }
    }
    rsAssert(!"ScriptGroup:setOutput kid not found");
}

bool ScriptGroup::validateInputAndOutput(Context *rsc) {
    for(size_t i = 0; i < mInputs.size(); i++) {
        if (mInputs[i]->mAlloc.get() == NULL) {
            rsc->setError(RS_ERROR_BAD_VALUE, "ScriptGroup missing input.");
            return false;
        }
    }

    for(size_t i = 0; i < mOutputs.size(); i++) {
        if (mOutputs[i]->mAlloc.get() == NULL) {
            rsc->setError(RS_ERROR_BAD_VALUE, "ScriptGroup missing output.");
            return false;
        }
    }

    return true;
}

void ScriptGroup::execute(Context *rsc) {

    if (!validateInputAndOutput(rsc)) {
        return;
    }

    //ALOGE("ScriptGroup::execute");
    if (rsc->mHal.funcs.scriptgroup.execute) {
        rsc->mHal.funcs.scriptgroup.execute(rsc, this);
        return;
    }

    for (auto node : mNodes) {
        for (auto kernel : node->mKernels) {
            Allocation *ain  = NULL;
            Allocation *aout = NULL;

            for (auto nodeInput : node->mInputs) {
                if (nodeInput->mDstKernel.get() == kernel) {
                    ain = nodeInput->mAlloc.get();
                }
            }

            for (auto sgInput : mInputs) {
                if (sgInput->mKernel == kernel) {
                    ain = sgInput->mAlloc.get();
                }
            }

            for (auto nodeOutput : node->mOutputs) {
                if (nodeOutput->mDstKernel.get() == kernel) {
                    aout = nodeOutput->mAlloc.get();
                }
            }

            for (auto sgOutput : mOutputs) {
                if (sgOutput->mKernel == kernel) {
                    aout = sgOutput->mAlloc.get();
                }
            }

            if (ain == NULL) {
                node->mScript->runForEach(rsc, kernel->mSlot, NULL, 0, aout,
                                          NULL, 0);
            } else {
                const Allocation *ains[1] = {ain};
                node->mScript->runForEach(rsc, kernel->mSlot, ains,
                                          sizeof(ains) / sizeof(RsAllocation),
                                          aout, NULL, 0);
            }
        }
    }

}

void ScriptGroup::serialize(Context *rsc, OStream *stream) const {
}

RsA3DClassID ScriptGroup::getClassId() const {
    return RS_A3D_CLASS_ID_SCRIPT_GROUP;
}

ScriptGroup::Link::Link() {
}

ScriptGroup::Link::~Link() {
}

namespace android {
namespace renderscript {


RsScriptGroup rsi_ScriptGroupCreate(Context *rsc,
                           RsScriptKernelID * kernels, size_t kernelsSize,
                           RsScriptKernelID * src, size_t srcSize,
                           RsScriptKernelID * dstK, size_t dstKSize,
                           RsScriptFieldID * dstF, size_t dstFSize,
                           const RsType * type, size_t typeSize) {


    return ScriptGroup::create(rsc,
                               (ScriptKernelID **) kernels, kernelsSize,
                               (ScriptKernelID **) src, srcSize,
                               (ScriptKernelID **) dstK, dstKSize,
                               (ScriptFieldID  **) dstF, dstFSize,
                               (const Type **) type, typeSize);
}


void rsi_ScriptGroupSetInput(Context *rsc, RsScriptGroup sg, RsScriptKernelID kid,
        RsAllocation alloc) {
    ScriptGroup *s = (ScriptGroup *)sg;
    s->setInput(rsc, (ScriptKernelID *)kid, (Allocation *)alloc);
}

void rsi_ScriptGroupSetOutput(Context *rsc, RsScriptGroup sg, RsScriptKernelID kid,
        RsAllocation alloc) {
    ScriptGroup *s = (ScriptGroup *)sg;
    s->setOutput(rsc, (ScriptKernelID *)kid, (Allocation *)alloc);
}

void rsi_ScriptGroupExecute(Context *rsc, RsScriptGroup sg) {
    ScriptGroup *s = (ScriptGroup *)sg;
    s->execute(rsc);
}

}
}
