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

#include <malloc.h>
#include <string.h>
#include <pthread.h>

#include "RenderScript.h"
#include "rs.h"

#include <dlfcn.h>

#define LOG_NDEBUG 0
#define LOG_TAG "rsC++"

using namespace android;
using namespace RSC;

bool RS::gInitialized = false;
pthread_mutex_t RS::gInitMutex = PTHREAD_MUTEX_INITIALIZER;
void* RS::librs = NULL;
dispatchTable* RS::dispatch = NULL;

static int gInitError = 0;

RS::RS() {
    mDev = NULL;
    mContext = NULL;
    mErrorFunc = NULL;
    mMessageFunc = NULL;
    mMessageRun = false;
    mInit = false;

    memset(&mElements, 0, sizeof(mElements));
}

RS::~RS() {
    if (mInit == true) {
        mMessageRun = false;

        RS::dispatch->ContextDeinitToClient(mContext);

        void *res = NULL;
        int status = pthread_join(mMessageThreadId, &res);

        RS::dispatch->ContextDestroy(mContext);
        mContext = NULL;
        RS::dispatch->DeviceDestroy(mDev);
        mDev = NULL;
    }
}

bool RS::init(bool forceCpu, bool synchronous) {
    return RS::init(RS_VERSION, forceCpu, synchronous);
}

bool RS::initDispatch(int targetApi) {

    pthread_mutex_lock(&gInitMutex);
    if (gInitError) {
        goto error;
    } else if (gInitialized) {
        return true;
    }
    // pick appropriate lib at some point
    RS::librs = dlopen("libRS.so", RTLD_LAZY | RTLD_LOCAL);
    if (RS::librs == 0) {
        ALOGE("couldn't dlopen libRS, %s", dlerror());
        goto error;
    }
    ALOGE("libRS initialized successfully");

    RS::dispatch = new dispatchTable;


    RS::dispatch->AllocationGetType = (AllocationGetTypeFnPtr)dlsym(RS::librs, "rsaAllocationGetType");
    if (RS::dispatch->AllocationGetType == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->AllocationGetType");
        goto error;
    }
    RS::dispatch->TypeGetNativeData = (TypeGetNativeDataFnPtr)dlsym(RS::librs, "rsaTypeGetNativeData");
    if (RS::dispatch->TypeGetNativeData == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->TypeGetNativeData");
        goto error;
    }
    RS::dispatch->ElementGetNativeData = (ElementGetNativeDataFnPtr)dlsym(RS::librs, "rsaElementGetNativeData");
    if (RS::dispatch->ElementGetNativeData == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ElementGetNativeData");
        goto error;
    }
    RS::dispatch->ElementGetSubElements = (ElementGetSubElementsFnPtr)dlsym(RS::librs, "rsaElementGetSubElements");
    if (RS::dispatch->ElementGetSubElements == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ElementGetSubElements");
        goto error;
    }
    RS::dispatch->DeviceCreate = (DeviceCreateFnPtr)dlsym(RS::librs, "rsDeviceCreate");
    if (RS::dispatch->DeviceCreate == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->DeviceCreate");
        goto error;
    }
    RS::dispatch->DeviceDestroy = (DeviceDestroyFnPtr)dlsym(RS::librs, "rsDeviceDestroy");
    if (RS::dispatch->DeviceDestroy == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->DeviceDestroy");
        goto error;
    }
    RS::dispatch->DeviceSetConfig = (DeviceSetConfigFnPtr)dlsym(RS::librs, "rsDeviceSetConfig");
    if (RS::dispatch->DeviceSetConfig == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->DeviceSetConfig");
        goto error;
    }
    RS::dispatch->ContextCreate = (ContextCreateFnPtr)dlsym(RS::librs, "rsContextCreate");;
    if (RS::dispatch->ContextCreate == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ContextCreate");
        goto error;
    }
    RS::dispatch->ContextDestroy = (ContextDestroyFnPtr)dlsym(RS::librs, "rsContextDestroy");
    if (RS::dispatch->ContextDestroy == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ContextDestroy");
        goto error;
    }
    RS::dispatch->ContextGetMessage = (ContextGetMessageFnPtr)dlsym(RS::librs, "rsContextGetMessage");
    if (RS::dispatch->ContextGetMessage == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ContextGetMessage");
        goto error;
    }
    RS::dispatch->ContextPeekMessage = (ContextPeekMessageFnPtr)dlsym(RS::librs, "rsContextPeekMessage");
    if (RS::dispatch->ContextPeekMessage == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ContextPeekMessage");
        goto error;
    }
    RS::dispatch->ContextSendMessage = (ContextSendMessageFnPtr)dlsym(RS::librs, "rsContextSendMessage");
    if (RS::dispatch->ContextSendMessage == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ContextSendMessage");
        goto error;
    }
    RS::dispatch->ContextInitToClient = (ContextInitToClientFnPtr)dlsym(RS::librs, "rsContextInitToClient");
    if (RS::dispatch->ContextInitToClient == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ContextInitToClient");
        goto error;
    }
    RS::dispatch->ContextDeinitToClient = (ContextDeinitToClientFnPtr)dlsym(RS::librs, "rsContextDeinitToClient");
    if (RS::dispatch->ContextDeinitToClient == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ContextDeinitToClient");
        goto error;
    }
    RS::dispatch->TypeCreate = (TypeCreateFnPtr)dlsym(RS::librs, "rsTypeCreate");
    if (RS::dispatch->TypeCreate == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->TypeCreate");
        goto error;
    }
    RS::dispatch->AllocationCreateTyped = (AllocationCreateTypedFnPtr)dlsym(RS::librs, "rsAllocationCreateTyped");
    if (RS::dispatch->AllocationCreateTyped == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->AllocationCreateTyped");
        goto error;
    }
    RS::dispatch->AllocationCreateFromBitmap = (AllocationCreateFromBitmapFnPtr)dlsym(RS::librs, "rsAllocationCreateFromBitmap");
    if (RS::dispatch->AllocationCreateFromBitmap == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->AllocationCreateFromBitmap");
        goto error;
    }
    RS::dispatch->AllocationCubeCreateFromBitmap = (AllocationCubeCreateFromBitmapFnPtr)dlsym(RS::librs, "rsAllocationCubeCreateFromBitmap");
    if (RS::dispatch->AllocationCubeCreateFromBitmap == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->AllocationCubeCreateFromBitmap");
        goto error;
    }
    RS::dispatch->AllocationGetSurface = (AllocationGetSurfaceFnPtr)dlsym(RS::librs, "rsAllocationGetSurface");
    if (RS::dispatch->AllocationGetSurface == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->AllocationGetSurface");
        goto error;
    }
    RS::dispatch->AllocationSetSurface = (AllocationSetSurfaceFnPtr)dlsym(RS::librs, "rsAllocationSetSurface");
    if (RS::dispatch->AllocationSetSurface == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->AllocationSetSurface");
        goto error;
    }
    RS::dispatch->ContextFinish = (ContextFinishFnPtr)dlsym(RS::librs, "rsContextFinish");
    if (RS::dispatch->ContextFinish == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ContextFinish");
        goto error;
    }
    RS::dispatch->ContextDump = (ContextDumpFnPtr)dlsym(RS::librs, "rsContextDump");
    if (RS::dispatch->ContextDump == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ContextDump");
        goto error;
    }
    RS::dispatch->ContextSetPriority = (ContextSetPriorityFnPtr)dlsym(RS::librs, "rsContextSetPriority");
    if (RS::dispatch->ContextSetPriority == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ContextSetPriority");
        goto error;
    }
    RS::dispatch->AssignName = (AssignNameFnPtr)dlsym(RS::librs, "rsAssignName");
    if (RS::dispatch->AssignName == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->AssignName");
        goto error;
    }
    RS::dispatch->ObjDestroy = (ObjDestroyFnPtr)dlsym(RS::librs, "rsObjDestroy");
    if (RS::dispatch->ObjDestroy == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ObjDestroy");
        goto error;
    }
    RS::dispatch->ElementCreate = (ElementCreateFnPtr)dlsym(RS::librs, "rsElementCreate");
    if (RS::dispatch->ElementCreate == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ElementCreate");
        goto error;
    }
    RS::dispatch->ElementCreate2 = (ElementCreate2FnPtr)dlsym(RS::librs, "rsElementCreate2");
    if (RS::dispatch->ElementCreate2 == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ElementCreate2");
        goto error;
    }
    RS::dispatch->AllocationCopyToBitmap = (AllocationCopyToBitmapFnPtr)dlsym(RS::librs, "rsAllocationCopyToBitmap");
    if (RS::dispatch->AllocationCopyToBitmap == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->AllocationCopyToBitmap");
        goto error;
    }
    RS::dispatch->Allocation1DData = (Allocation1DDataFnPtr)dlsym(RS::librs, "rsAllocation1DData");
    if (RS::dispatch->Allocation1DData == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->Allocation1DData");
        goto error;
    }
    RS::dispatch->Allocation1DElementData = (Allocation1DElementDataFnPtr)dlsym(RS::librs, "rsAllocation1DElementData");
    if (RS::dispatch->Allocation1DElementData == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->Allocation1DElementData");
        goto error;
    }
    RS::dispatch->Allocation2DData = (Allocation2DDataFnPtr)dlsym(RS::librs, "rsAllocation2DData");
    if (RS::dispatch->Allocation2DData == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->Allocation2DData");
        goto error;
    }
    RS::dispatch->Allocation3DData = (Allocation3DDataFnPtr)dlsym(RS::librs, "rsAllocation3DData");
    if (RS::dispatch->Allocation3DData == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->Allocation3DData");
        goto error;
    }
    RS::dispatch->AllocationGenerateMipmaps = (AllocationGenerateMipmapsFnPtr)dlsym(RS::librs, "rsAllocationGenerateMipmaps");
    if (RS::dispatch->AllocationGenerateMipmaps == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->AllocationGenerateMipmaps");
        goto error;
    }
    RS::dispatch->AllocationRead = (AllocationReadFnPtr)dlsym(RS::librs, "rsAllocationRead");
    if (RS::dispatch->AllocationRead == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->AllocationRead");
        goto error;
    }
    RS::dispatch->Allocation1DRead = (Allocation1DReadFnPtr)dlsym(RS::librs, "rsAllocation1DRead");
    if (RS::dispatch->Allocation1DRead == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->Allocation1DRead");
        goto error;
    }
    RS::dispatch->Allocation2DRead = (Allocation2DReadFnPtr)dlsym(RS::librs, "rsAllocation2DRead");
    if (RS::dispatch->Allocation2DRead == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->Allocation2DRead");
        goto error;
    }
    RS::dispatch->AllocationSyncAll = (AllocationSyncAllFnPtr)dlsym(RS::librs, "rsAllocationSyncAll");
    if (RS::dispatch->AllocationSyncAll == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->AllocationSyncAll");
        goto error;
    }
    RS::dispatch->AllocationResize1D = (AllocationResize1DFnPtr)dlsym(RS::librs, "rsAllocationResize1D");
    if (RS::dispatch->AllocationResize1D == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->AllocationResize1D");
        goto error;
    }
    RS::dispatch->AllocationCopy2DRange = (AllocationCopy2DRangeFnPtr)dlsym(RS::librs, "rsAllocationCopy2DRange");
    if (RS::dispatch->AllocationCopy2DRange == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->AllocationCopy2DRange");
        goto error;
    }
    RS::dispatch->AllocationCopy3DRange = (AllocationCopy3DRangeFnPtr)dlsym(RS::librs, "rsAllocationCopy3DRange");
    if (RS::dispatch->AllocationCopy3DRange == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->AllocationCopy3DRange");
        goto error;
    }
    RS::dispatch->SamplerCreate = (SamplerCreateFnPtr)dlsym(RS::librs, "rsSamplerCreate");
    if (RS::dispatch->SamplerCreate == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->SamplerCreate");
        goto error;
    }
    RS::dispatch->ScriptBindAllocation = (ScriptBindAllocationFnPtr)dlsym(RS::librs, "rsScriptBindAllocation");
    if (RS::dispatch->ScriptBindAllocation == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ScriptBindAllocation");
        goto error;
    }
    RS::dispatch->ScriptSetTimeZone = (ScriptSetTimeZoneFnPtr)dlsym(RS::librs, "rsScriptSetTimeZone");
    if (RS::dispatch->ScriptSetTimeZone == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ScriptSetTimeZone");
        goto error;
    }
    RS::dispatch->ScriptInvoke = (ScriptInvokeFnPtr)dlsym(RS::librs, "rsScriptInvoke");
    if (RS::dispatch->ScriptInvoke == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ScriptInvoke");
        goto error;
    }
    RS::dispatch->ScriptInvokeV = (ScriptInvokeVFnPtr)dlsym(RS::librs, "rsScriptInvokeV");
    if (RS::dispatch->ScriptInvokeV == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ScriptInvokeV");
        goto error;
    }
    RS::dispatch->ScriptForEach = (ScriptForEachFnPtr)dlsym(RS::librs, "rsScriptForEach");
    if (RS::dispatch->ScriptForEach == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ScriptForEach");
        goto error;
    }
    RS::dispatch->ScriptSetVarI = (ScriptSetVarIFnPtr)dlsym(RS::librs, "rsScriptSetVarI");
    if (RS::dispatch->ScriptSetVarI == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ScriptSetVarI");
        goto error;
    }
    RS::dispatch->ScriptSetVarObj = (ScriptSetVarObjFnPtr)dlsym(RS::librs, "rsScriptSetVarObj");
    if (RS::dispatch->ScriptSetVarObj == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ScriptSetVarObj");
        goto error;
    }
    RS::dispatch->ScriptSetVarJ = (ScriptSetVarJFnPtr)dlsym(RS::librs, "rsScriptSetVarJ");
    if (RS::dispatch->ScriptSetVarJ == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ScriptSetVarJ");
        goto error;
    }
    RS::dispatch->ScriptSetVarF = (ScriptSetVarFFnPtr)dlsym(RS::librs, "rsScriptSetVarF");
    if (RS::dispatch->ScriptSetVarF == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ScriptSetVarF");
        goto error;
    }
    RS::dispatch->ScriptSetVarD = (ScriptSetVarDFnPtr)dlsym(RS::librs, "rsScriptSetVarD");
    if (RS::dispatch->ScriptSetVarD == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ScriptSetVarD");
        goto error;
    }
    RS::dispatch->ScriptSetVarV = (ScriptSetVarVFnPtr)dlsym(RS::librs, "rsScriptSetVarV");
    if (RS::dispatch->ScriptSetVarV == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ScriptSetVarV");
        goto error;
    }
    RS::dispatch->ScriptGetVarV = (ScriptGetVarVFnPtr)dlsym(RS::librs, "rsScriptGetVarV");
    if (RS::dispatch->ScriptGetVarV == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ScriptGetVarV");
        goto error;
    }
    RS::dispatch->ScriptSetVarVE = (ScriptSetVarVEFnPtr)dlsym(RS::librs, "rsScriptSetVarVE");
    if (RS::dispatch->ScriptSetVarVE == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ScriptSetVarVE");
        goto error;
    }
    RS::dispatch->ScriptCCreate = (ScriptCCreateFnPtr)dlsym(RS::librs, "rsScriptCCreate");
    if (RS::dispatch->ScriptCCreate == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ScriptCCreate");
        goto error;
    }
    RS::dispatch->ScriptIntrinsicCreate = (ScriptIntrinsicCreateFnPtr)dlsym(RS::librs, "rsScriptIntrinsicCreate");
    if (RS::dispatch->ScriptIntrinsicCreate == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ScriptIntrinsicCreate");
        goto error;
    }
    RS::dispatch->ScriptKernelIDCreate = (ScriptKernelIDCreateFnPtr)dlsym(RS::librs, "rsScriptKernelIDCreate");
    if (RS::dispatch->ScriptKernelIDCreate == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ScriptKernelIDCreate");
        goto error;
    }
    RS::dispatch->ScriptFieldIDCreate = (ScriptFieldIDCreateFnPtr)dlsym(RS::librs, "rsScriptFieldIDCreate");
    if (RS::dispatch->ScriptFieldIDCreate == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ScriptFieldIDCreate");
        goto error;
    }
    RS::dispatch->ScriptGroupCreate = (ScriptGroupCreateFnPtr)dlsym(RS::librs, "rsScriptGroupCreate");
    if (RS::dispatch->ScriptGroupCreate == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ScriptGroupCreate");
        goto error;
    }
    RS::dispatch->ScriptGroupSetOutput = (ScriptGroupSetOutputFnPtr)dlsym(RS::librs, "rsScriptGroupSetOutput");
    if (RS::dispatch->ScriptGroupSetOutput == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ScriptGroupSetOutput");
        goto error;
    }
    RS::dispatch->ScriptGroupSetInput = (ScriptGroupSetInputFnPtr)dlsym(RS::librs, "rsScriptGroupSetInput");
    if (RS::dispatch->ScriptGroupSetInput == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ScriptGroupSetInput");
        goto error;
    }
    RS::dispatch->ScriptGroupExecute = (ScriptGroupExecuteFnPtr)dlsym(RS::librs, "rsScriptGroupExecute");
    if (RS::dispatch->ScriptGroupExecute == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->ScriptGroupExecute");
        goto error;
    }
    RS::dispatch->AllocationIoSend = (AllocationIoSendFnPtr)dlsym(RS::librs, "rsAllocationIoSend");
    if (RS::dispatch->AllocationIoSend == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->AllocationIoSend");
        goto error;
    }
    RS::dispatch->AllocationIoReceive = (AllocationIoReceiveFnPtr)dlsym(RS::librs, "rsAllocationIoReceive");
    if (RS::dispatch->AllocationIoReceive == NULL) {
        ALOGE("Couldn't initialize RS::dispatch->AllocationIoReceive");
        goto error;
    }

    gInitialized = true;

    pthread_mutex_unlock(&gInitMutex);
    return true;

 error:
    gInitError = 1;
    pthread_mutex_unlock(&gInitMutex);
    return false;
}

bool RS::init(int targetApi, bool forceCpu, bool synchronous) {
    if (initDispatch(targetApi) == false) {
        ALOGE("Couldn't initialize dispatch table");
        return false;
    }

    mDev = RS::dispatch->DeviceCreate();
    if (mDev == 0) {
        ALOGE("Device creation failed");
        return false;
    }

    mContext = RS::dispatch->ContextCreate(mDev, 0, targetApi, RS_CONTEXT_TYPE_NORMAL, forceCpu, synchronous);
    if (mContext == 0) {
        ALOGE("Context creation failed");
        return false;
    }

    pid_t mNativeMessageThreadId;

    int status = pthread_create(&mMessageThreadId, NULL, threadProc, this);
    if (status) {
        ALOGE("Failed to start RS message thread.");
        return false;
    }
    // Wait for the message thread to be active.
    while (!mMessageRun) {
        usleep(1000);
    }

    mInit = true;

    return true;
}

void RS::throwError(const char *err) const {
    ALOGE("RS CPP error: %s", err);
    int * v = NULL;
    v[0] = 0;
}


void * RS::threadProc(void *vrsc) {
    RS *rs = static_cast<RS *>(vrsc);
    size_t rbuf_size = 256;
    void * rbuf = malloc(rbuf_size);

    RS::dispatch->ContextInitToClient(rs->mContext);
    rs->mMessageRun = true;

    while (rs->mMessageRun) {
        size_t receiveLen = 0;
        uint32_t usrID = 0;
        uint32_t subID = 0;
        RsMessageToClientType r = RS::dispatch->ContextPeekMessage(rs->mContext,
                                                                   &receiveLen, sizeof(receiveLen),
                                                                   &usrID, sizeof(usrID));

        if (receiveLen >= rbuf_size) {
            rbuf_size = receiveLen + 32;
            rbuf = realloc(rbuf, rbuf_size);
        }
        if (!rbuf) {
            ALOGE("RS::message handler realloc error %zu", rbuf_size);
            // No clean way to recover now?
        }
        RS::dispatch->ContextGetMessage(rs->mContext, rbuf, rbuf_size, &receiveLen, sizeof(receiveLen),
                            &subID, sizeof(subID));

        switch(r) {
        case RS_MESSAGE_TO_CLIENT_ERROR:
            ALOGE("RS Error %s", (const char *)rbuf);

            if(rs->mMessageFunc != NULL) {
                rs->mErrorFunc(usrID, (const char *)rbuf);
            }
            break;
        case RS_MESSAGE_TO_CLIENT_NONE:
        case RS_MESSAGE_TO_CLIENT_EXCEPTION:
        case RS_MESSAGE_TO_CLIENT_RESIZE:
            // teardown. But we want to avoid starving other threads during
            // teardown by yielding until the next line in the destructor can
            // execute to set mRun = false. Note that the FIFO sends an
            // empty NONE message when it reaches its destructor.
            usleep(1000);
            break;
        case RS_MESSAGE_TO_CLIENT_USER:
            if(rs->mMessageFunc != NULL) {
                rs->mMessageFunc(usrID, rbuf, receiveLen);
            } else {
                ALOGE("Received a message from the script with no message handler installed.");
            }
            break;

        default:
            ALOGE("RS unknown message type %i", r);
        }
    }

    if (rbuf) {
        free(rbuf);
    }
    ALOGE("RS Message thread exiting.");
    return NULL;
}

void RS::setErrorHandler(ErrorHandlerFunc_t func) {
    mErrorFunc = func;
}

void RS::setMessageHandler(MessageHandlerFunc_t func) {
    mMessageFunc  = func;
}

void RS::finish() {
    RS::dispatch->ContextFinish(mContext);
}
