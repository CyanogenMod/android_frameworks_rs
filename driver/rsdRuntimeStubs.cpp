/*
 * Copyright (C) 2011-2012 The Android Open Source Project
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

#include "rsContext.h"
#include "rsScriptC.h"
#include "rsMatrix4x4.h"
#include "rsMatrix3x3.h"
#include "rsMatrix2x2.h"
#include "rsRuntime.h"

#include "utils/Timers.h"
#include "rsdCore.h"
#include "rsdBcc.h"

#include "rsdPath.h"
#include "rsdAllocation.h"
#include "rsdShaderCache.h"
#include "rsdVertexArray.h"

#include <time.h>

using namespace android;
using namespace android::renderscript;

typedef float float2 __attribute__((ext_vector_type(2)));
typedef float float3 __attribute__((ext_vector_type(3)));
typedef float float4 __attribute__((ext_vector_type(4)));
typedef char char2 __attribute__((ext_vector_type(2)));
typedef char char3 __attribute__((ext_vector_type(3)));
typedef char char4 __attribute__((ext_vector_type(4)));
typedef unsigned char uchar2 __attribute__((ext_vector_type(2)));
typedef unsigned char uchar3 __attribute__((ext_vector_type(3)));
typedef unsigned char uchar4 __attribute__((ext_vector_type(4)));
typedef short short2 __attribute__((ext_vector_type(2)));
typedef short short3 __attribute__((ext_vector_type(3)));
typedef short short4 __attribute__((ext_vector_type(4)));
typedef unsigned short ushort2 __attribute__((ext_vector_type(2)));
typedef unsigned short ushort3 __attribute__((ext_vector_type(3)));
typedef unsigned short ushort4 __attribute__((ext_vector_type(4)));
typedef int32_t int2 __attribute__((ext_vector_type(2)));
typedef int32_t int3 __attribute__((ext_vector_type(3)));
typedef int32_t int4 __attribute__((ext_vector_type(4)));
typedef uint32_t uint2 __attribute__((ext_vector_type(2)));
typedef uint32_t uint3 __attribute__((ext_vector_type(3)));
typedef uint32_t uint4 __attribute__((ext_vector_type(4)));
typedef long long long2 __attribute__((ext_vector_type(2)));
typedef long long long3 __attribute__((ext_vector_type(3)));
typedef long long long4 __attribute__((ext_vector_type(4)));
typedef unsigned long long ulong2 __attribute__((ext_vector_type(2)));
typedef unsigned long long ulong3 __attribute__((ext_vector_type(3)));
typedef unsigned long long ulong4 __attribute__((ext_vector_type(4)));


//////////////////////////////////////////////////////////////////////////////
// Allocation
//////////////////////////////////////////////////////////////////////////////


static void SC_AllocationSyncAll2(Allocation *a, RsAllocationUsageType source) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrAllocationSyncAll(rsc, a, source);
}

static void SC_AllocationSyncAll(Allocation *a) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrAllocationSyncAll(rsc, a, RS_ALLOCATION_USAGE_SCRIPT);
}

static void SC_AllocationCopy1DRange(Allocation *dstAlloc,
                                     uint32_t dstOff,
                                     uint32_t dstMip,
                                     uint32_t count,
                                     Allocation *srcAlloc,
                                     uint32_t srcOff, uint32_t srcMip) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrAllocationCopy1DRange(rsc, dstAlloc, dstOff, dstMip, count,
                             srcAlloc, srcOff, srcMip);
}

static void SC_AllocationCopy2DRange(Allocation *dstAlloc,
                                     uint32_t dstXoff, uint32_t dstYoff,
                                     uint32_t dstMip, uint32_t dstFace,
                                     uint32_t width, uint32_t height,
                                     Allocation *srcAlloc,
                                     uint32_t srcXoff, uint32_t srcYoff,
                                     uint32_t srcMip, uint32_t srcFace) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrAllocationCopy2DRange(rsc, dstAlloc,
                             dstXoff, dstYoff, dstMip, dstFace,
                             width, height,
                             srcAlloc,
                             srcXoff, srcYoff, srcMip, srcFace);
}

static void SC_AllocationIoSend(Allocation *alloc) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsdAllocationIoSend(rsc, alloc);
}


static void SC_AllocationIoReceive(Allocation *alloc) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsdAllocationIoReceive(rsc, alloc);
}



//////////////////////////////////////////////////////////////////////////////
// Context
//////////////////////////////////////////////////////////////////////////////

static void SC_BindTexture(ProgramFragment *pf, uint32_t slot, Allocation *a) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrBindTexture(rsc, pf, slot, a);
}

static void SC_BindVertexConstant(ProgramVertex *pv, uint32_t slot, Allocation *a) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrBindConstant(rsc, pv, slot, a);
}

static void SC_BindFragmentConstant(ProgramFragment *pf, uint32_t slot, Allocation *a) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrBindConstant(rsc, pf, slot, a);
}

static void SC_BindSampler(ProgramFragment *pf, uint32_t slot, Sampler *s) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrBindSampler(rsc, pf, slot, s);
}

static void SC_BindProgramStore(ProgramStore *ps) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrBindProgramStore(rsc, ps);
}

static void SC_BindProgramFragment(ProgramFragment *pf) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrBindProgramFragment(rsc, pf);
}

static void SC_BindProgramVertex(ProgramVertex *pv) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrBindProgramVertex(rsc, pv);
}

static void SC_BindProgramRaster(ProgramRaster *pr) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrBindProgramRaster(rsc, pr);
}

static void SC_BindFrameBufferObjectColorTarget(Allocation *a, uint32_t slot) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrBindFrameBufferObjectColorTarget(rsc, a, slot);
}

static void SC_BindFrameBufferObjectDepthTarget(Allocation *a) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrBindFrameBufferObjectDepthTarget(rsc, a);
}

static void SC_ClearFrameBufferObjectColorTarget(uint32_t slot) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrClearFrameBufferObjectColorTarget(rsc, slot);
}

static void SC_ClearFrameBufferObjectDepthTarget(Context *, Script *) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrClearFrameBufferObjectDepthTarget(rsc);
}

static void SC_ClearFrameBufferObjectTargets(Context *, Script *) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrClearFrameBufferObjectTargets(rsc);
}


//////////////////////////////////////////////////////////////////////////////
// VP
//////////////////////////////////////////////////////////////////////////////

static void SC_VpLoadProjectionMatrix(const rsc_Matrix *m) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrVpLoadProjectionMatrix(rsc, m);
}

static void SC_VpLoadModelMatrix(const rsc_Matrix *m) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrVpLoadModelMatrix(rsc, m);
}

static void SC_VpLoadTextureMatrix(const rsc_Matrix *m) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrVpLoadTextureMatrix(rsc, m);
}

static void SC_PfConstantColor(ProgramFragment *pf, float r, float g, float b, float a) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrPfConstantColor(rsc, pf, r, g, b, a);
}

static void SC_VpGetProjectionMatrix(rsc_Matrix *m) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrVpGetProjectionMatrix(rsc, m);
}


//////////////////////////////////////////////////////////////////////////////
// Drawing
//////////////////////////////////////////////////////////////////////////////

static void SC_DrawQuadTexCoords(float x1, float y1, float z1, float u1, float v1,
                                 float x2, float y2, float z2, float u2, float v2,
                                 float x3, float y3, float z3, float u3, float v3,
                                 float x4, float y4, float z4, float u4, float v4) {
    Context *rsc = RsdCpuReference::getTlsContext();

    if (!rsc->setupCheck()) {
        return;
    }

    RsdHal *dc = (RsdHal *)rsc->mHal.drv;
    if (!dc->gl.shaderCache->setup(rsc)) {
        return;
    }

    //ALOGE("Quad");
    //ALOGE("%4.2f, %4.2f, %4.2f", x1, y1, z1);
    //ALOGE("%4.2f, %4.2f, %4.2f", x2, y2, z2);
    //ALOGE("%4.2f, %4.2f, %4.2f", x3, y3, z3);
    //ALOGE("%4.2f, %4.2f, %4.2f", x4, y4, z4);

    float vtx[] = {x1,y1,z1, x2,y2,z2, x3,y3,z3, x4,y4,z4};
    const float tex[] = {u1,v1, u2,v2, u3,v3, u4,v4};

    RsdVertexArray::Attrib attribs[2];
    attribs[0].set(GL_FLOAT, 3, 12, false, (uint32_t)vtx, "ATTRIB_position");
    attribs[1].set(GL_FLOAT, 2, 8, false, (uint32_t)tex, "ATTRIB_texture0");

    RsdVertexArray va(attribs, 2);
    va.setup(rsc);

    RSD_CALL_GL(glDrawArrays, GL_TRIANGLE_FAN, 0, 4);
}

static void SC_DrawQuad(float x1, float y1, float z1,
                        float x2, float y2, float z2,
                        float x3, float y3, float z3,
                        float x4, float y4, float z4) {
    SC_DrawQuadTexCoords(x1, y1, z1, 0, 1,
                         x2, y2, z2, 1, 1,
                         x3, y3, z3, 1, 0,
                         x4, y4, z4, 0, 0);
}

static void SC_DrawSpriteScreenspace(float x, float y, float z, float w, float h) {
    Context *rsc = RsdCpuReference::getTlsContext();

    ObjectBaseRef<const ProgramVertex> tmp(rsc->getProgramVertex());
    rsc->setProgramVertex(rsc->getDefaultProgramVertex());
    //rsc->setupCheck();

    //GLint crop[4] = {0, h, w, -h};

    float sh = rsc->getHeight();

    SC_DrawQuad(x,   sh - y,     z,
                x+w, sh - y,     z,
                x+w, sh - (y+h), z,
                x,   sh - (y+h), z);
    rsc->setProgramVertex((ProgramVertex *)tmp.get());
}

static void SC_DrawRect(float x1, float y1, float x2, float y2, float z) {
    SC_DrawQuad(x1, y2, z, x2, y2, z, x2, y1, z, x1, y1, z);
}

static void SC_DrawPath(Path *p) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsdPathDraw(rsc, p);
}

static void SC_DrawMesh(Mesh *m) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrDrawMesh(rsc, m);
}

static void SC_DrawMeshPrimitive(Mesh *m, uint32_t primIndex) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrDrawMeshPrimitive(rsc, m, primIndex);
}

static void SC_DrawMeshPrimitiveRange(Mesh *m, uint32_t primIndex, uint32_t start, uint32_t len) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrDrawMeshPrimitiveRange(rsc, m, primIndex, start, len);
}

static void SC_MeshComputeBoundingBox(Mesh *m,
                               float *minX, float *minY, float *minZ,
                               float *maxX, float *maxY, float *maxZ) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrMeshComputeBoundingBox(rsc, m, minX, minY, minZ, maxX, maxY, maxZ);
}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////


static void SC_Color(float r, float g, float b, float a) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrColor(rsc, r, g, b, a);
}

static void SC_Finish() {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsdGLFinish(rsc);
}

static void SC_ClearColor(float r, float g, float b, float a) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrPrepareClear(rsc);
    rsdGLClearColor(rsc, r, g, b, a);
}

static void SC_ClearDepth(float v) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrPrepareClear(rsc);
    rsdGLClearDepth(rsc, v);
}

static uint32_t SC_GetWidth() {
    Context *rsc = RsdCpuReference::getTlsContext();
    return rsrGetWidth(rsc);
}

static uint32_t SC_GetHeight() {
    Context *rsc = RsdCpuReference::getTlsContext();
    return rsrGetHeight(rsc);
}

static void SC_DrawTextAlloc(Allocation *a, int x, int y) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrDrawTextAlloc(rsc, a, x, y);
}

static void SC_DrawText(const char *text, int x, int y) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrDrawText(rsc, text, x, y);
}

static void SC_MeasureTextAlloc(Allocation *a,
                         int32_t *left, int32_t *right, int32_t *top, int32_t *bottom) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrMeasureTextAlloc(rsc, a, left, right, top, bottom);
}

static void SC_MeasureText(const char *text,
                    int32_t *left, int32_t *right, int32_t *top, int32_t *bottom) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrMeasureText(rsc, text, left, right, top, bottom);
}

static void SC_BindFont(Font *f) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrBindFont(rsc, f);
}

static void SC_FontColor(float r, float g, float b, float a) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrFontColor(rsc, r, g, b, a);
}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////

static void SC_SetObject(ObjectBase **dst, ObjectBase * src) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrSetObject(rsc, dst, src);
}

static void SC_ClearObject(ObjectBase **dst) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrClearObject(rsc, dst);
}

static bool SC_IsObject(const ObjectBase *src) {
    Context *rsc = RsdCpuReference::getTlsContext();
    return rsrIsObject(rsc, src);
}




static const Allocation * SC_GetAllocation(const void *ptr) {
    Context *rsc = RsdCpuReference::getTlsContext();
    const Script *sc = RsdCpuReference::getTlsScript();
    return rsdScriptGetAllocationForPointer(rsc, sc, ptr);
}

static void SC_ForEach_SAA(Script *target,
                            Allocation *in,
                            Allocation *out) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrForEach(rsc, target, in, out, NULL, 0, NULL);
}

static void SC_ForEach_SAAU(Script *target,
                            Allocation *in,
                            Allocation *out,
                            const void *usr) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrForEach(rsc, target, in, out, usr, 0, NULL);
}

static void SC_ForEach_SAAUS(Script *target,
                             Allocation *in,
                             Allocation *out,
                             const void *usr,
                             const RsScriptCall *call) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrForEach(rsc, target, in, out, usr, 0, call);
}

static void SC_ForEach_SAAUL(Script *target,
                             Allocation *in,
                             Allocation *out,
                             const void *usr,
                             uint32_t usrLen) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrForEach(rsc, target, in, out, usr, usrLen, NULL);
}

static void SC_ForEach_SAAULS(Script *target,
                              Allocation *in,
                              Allocation *out,
                              const void *usr,
                              uint32_t usrLen,
                              const RsScriptCall *call) {
    Context *rsc = RsdCpuReference::getTlsContext();
    rsrForEach(rsc, target, in, out, usr, usrLen, call);
}



//////////////////////////////////////////////////////////////////////////////
// Time routines
//////////////////////////////////////////////////////////////////////////////

static float SC_GetDt() {
    Context *rsc = RsdCpuReference::getTlsContext();
    const Script *sc = RsdCpuReference::getTlsScript();
    return rsrGetDt(rsc, sc);
}

time_t SC_Time(time_t *timer) {
    Context *rsc = RsdCpuReference::getTlsContext();
    return rsrTime(rsc, timer);
}

tm* SC_LocalTime(tm *local, time_t *timer) {
    Context *rsc = RsdCpuReference::getTlsContext();
    return rsrLocalTime(rsc, local, timer);
}

int64_t SC_UptimeMillis() {
    Context *rsc = RsdCpuReference::getTlsContext();
    return rsrUptimeMillis(rsc);
}

int64_t SC_UptimeNanos() {
    Context *rsc = RsdCpuReference::getTlsContext();
    return rsrUptimeNanos(rsc);
}

//////////////////////////////////////////////////////////////////////////////
// Message routines
//////////////////////////////////////////////////////////////////////////////

static uint32_t SC_ToClient2(int cmdID, void *data, int len) {
    Context *rsc = RsdCpuReference::getTlsContext();
    return rsrToClient(rsc, cmdID, data, len);
}

static uint32_t SC_ToClient(int cmdID) {
    Context *rsc = RsdCpuReference::getTlsContext();
    return rsrToClient(rsc, cmdID, NULL, 0);
}

static uint32_t SC_ToClientBlocking2(int cmdID, void *data, int len) {
    Context *rsc = RsdCpuReference::getTlsContext();
    return rsrToClientBlocking(rsc, cmdID, data, len);
}

static uint32_t SC_ToClientBlocking(int cmdID) {
    Context *rsc = RsdCpuReference::getTlsContext();
    return rsrToClientBlocking(rsc, cmdID, NULL, 0);
}



//////////////////////////////////////////////////////////////////////////////
// Stub implementation
//////////////////////////////////////////////////////////////////////////////

// llvm name mangling ref
//  <builtin-type> ::= v  # void
//                 ::= b  # bool
//                 ::= c  # char
//                 ::= a  # signed char
//                 ::= h  # unsigned char
//                 ::= s  # short
//                 ::= t  # unsigned short
//                 ::= i  # int
//                 ::= j  # unsigned int
//                 ::= l  # long
//                 ::= m  # unsigned long
//                 ::= x  # long long, __int64
//                 ::= y  # unsigned long long, __int64
//                 ::= f  # float
//                 ::= d  # double

static RsdCpuReference::CpuSymbol gSyms[] = {
    // Refcounting
    { "_Z11rsSetObjectP10rs_elementS_", (void *)&SC_SetObject, true },
    { "_Z13rsClearObjectP10rs_element", (void *)&SC_ClearObject, true },
    { "_Z10rsIsObject10rs_element", (void *)&SC_IsObject, true },

    { "_Z11rsSetObjectP7rs_typeS_", (void *)&SC_SetObject, true },
    { "_Z13rsClearObjectP7rs_type", (void *)&SC_ClearObject, true },
    { "_Z10rsIsObject7rs_type", (void *)&SC_IsObject, true },

    { "_Z11rsSetObjectP13rs_allocationS_", (void *)&SC_SetObject, true },
    { "_Z13rsClearObjectP13rs_allocation", (void *)&SC_ClearObject, true },
    { "_Z10rsIsObject13rs_allocation", (void *)&SC_IsObject, true },

    { "_Z11rsSetObjectP10rs_samplerS_", (void *)&SC_SetObject, true },
    { "_Z13rsClearObjectP10rs_sampler", (void *)&SC_ClearObject, true },
    { "_Z10rsIsObject10rs_sampler", (void *)&SC_IsObject, true },

    { "_Z11rsSetObjectP9rs_scriptS_", (void *)&SC_SetObject, true },
    { "_Z13rsClearObjectP9rs_script", (void *)&SC_ClearObject, true },
    { "_Z10rsIsObject9rs_script", (void *)&SC_IsObject, true },

    { "_Z11rsSetObjectP7rs_pathS_", (void *)&SC_SetObject, true },
    { "_Z13rsClearObjectP7rs_path", (void *)&SC_ClearObject, true },
    { "_Z10rsIsObject7rs_path", (void *)&SC_IsObject, true },

    { "_Z11rsSetObjectP7rs_meshS_", (void *)&SC_SetObject, true },
    { "_Z13rsClearObjectP7rs_mesh", (void *)&SC_ClearObject, true },
    { "_Z10rsIsObject7rs_mesh", (void *)&SC_IsObject, true },

    { "_Z11rsSetObjectP19rs_program_fragmentS_", (void *)&SC_SetObject, true },
    { "_Z13rsClearObjectP19rs_program_fragment", (void *)&SC_ClearObject, true },
    { "_Z10rsIsObject19rs_program_fragment", (void *)&SC_IsObject, true },

    { "_Z11rsSetObjectP17rs_program_vertexS_", (void *)&SC_SetObject, true },
    { "_Z13rsClearObjectP17rs_program_vertex", (void *)&SC_ClearObject, true },
    { "_Z10rsIsObject17rs_program_vertex", (void *)&SC_IsObject, true },

    { "_Z11rsSetObjectP17rs_program_rasterS_", (void *)&SC_SetObject, true },
    { "_Z13rsClearObjectP17rs_program_raster", (void *)&SC_ClearObject, true },
    { "_Z10rsIsObject17rs_program_raster", (void *)&SC_IsObject, true },

    { "_Z11rsSetObjectP16rs_program_storeS_", (void *)&SC_SetObject, true },
    { "_Z13rsClearObjectP16rs_program_store", (void *)&SC_ClearObject, true },
    { "_Z10rsIsObject16rs_program_store", (void *)&SC_IsObject, true },

    { "_Z11rsSetObjectP7rs_fontS_", (void *)&SC_SetObject, true },
    { "_Z13rsClearObjectP7rs_font", (void *)&SC_ClearObject, true },
    { "_Z10rsIsObject7rs_font", (void *)&SC_IsObject, true },

    // Allocation ops
    { "_Z21rsAllocationMarkDirty13rs_allocation", (void *)&SC_AllocationSyncAll, true },
    { "_Z20rsgAllocationSyncAll13rs_allocation", (void *)&SC_AllocationSyncAll, false },
    { "_Z20rsgAllocationSyncAll13rs_allocationj", (void *)&SC_AllocationSyncAll2, false },
    { "_Z20rsgAllocationSyncAll13rs_allocation24rs_allocation_usage_type", (void *)&SC_AllocationSyncAll2, false },
    { "_Z15rsGetAllocationPKv", (void *)&SC_GetAllocation, true },
    { "_Z18rsAllocationIoSend13rs_allocation", (void *)&SC_AllocationIoSend, false },
    { "_Z21rsAllocationIoReceive13rs_allocation", (void *)&SC_AllocationIoReceive, false },
    { "_Z23rsAllocationCopy1DRange13rs_allocationjjjS_jj", (void *)&SC_AllocationCopy1DRange, false },
    { "_Z23rsAllocationCopy2DRange13rs_allocationjjj26rs_allocation_cubemap_facejjS_jjjS0_", (void *)&SC_AllocationCopy2DRange, false },

    // Messaging

    { "_Z14rsSendToClienti", (void *)&SC_ToClient, false },
    { "_Z14rsSendToClientiPKvj", (void *)&SC_ToClient2, false },
    { "_Z22rsSendToClientBlockingi", (void *)&SC_ToClientBlocking, false },
    { "_Z22rsSendToClientBlockingiPKvj", (void *)&SC_ToClientBlocking2, false },

    { "_Z22rsgBindProgramFragment19rs_program_fragment", (void *)&SC_BindProgramFragment, false },
    { "_Z19rsgBindProgramStore16rs_program_store", (void *)&SC_BindProgramStore, false },
    { "_Z20rsgBindProgramVertex17rs_program_vertex", (void *)&SC_BindProgramVertex, false },
    { "_Z20rsgBindProgramRaster17rs_program_raster", (void *)&SC_BindProgramRaster, false },
    { "_Z14rsgBindSampler19rs_program_fragmentj10rs_sampler", (void *)&SC_BindSampler, false },
    { "_Z14rsgBindTexture19rs_program_fragmentj13rs_allocation", (void *)&SC_BindTexture, false },
    { "_Z15rsgBindConstant19rs_program_fragmentj13rs_allocation", (void *)&SC_BindFragmentConstant, false },
    { "_Z15rsgBindConstant17rs_program_vertexj13rs_allocation", (void *)&SC_BindVertexConstant, false },

    { "_Z36rsgProgramVertexLoadProjectionMatrixPK12rs_matrix4x4", (void *)&SC_VpLoadProjectionMatrix, false },
    { "_Z31rsgProgramVertexLoadModelMatrixPK12rs_matrix4x4", (void *)&SC_VpLoadModelMatrix, false },
    { "_Z33rsgProgramVertexLoadTextureMatrixPK12rs_matrix4x4", (void *)&SC_VpLoadTextureMatrix, false },

    { "_Z35rsgProgramVertexGetProjectionMatrixP12rs_matrix4x4", (void *)&SC_VpGetProjectionMatrix, false },

    { "_Z31rsgProgramFragmentConstantColor19rs_program_fragmentffff", (void *)&SC_PfConstantColor, false },

    { "_Z11rsgGetWidthv", (void *)&SC_GetWidth, false },
    { "_Z12rsgGetHeightv", (void *)&SC_GetHeight, false },


    { "_Z11rsgDrawRectfffff", (void *)&SC_DrawRect, false },
    { "_Z11rsgDrawQuadffffffffffff", (void *)&SC_DrawQuad, false },
    { "_Z20rsgDrawQuadTexCoordsffffffffffffffffffff", (void *)&SC_DrawQuadTexCoords, false },
    { "_Z24rsgDrawSpriteScreenspacefffff", (void *)&SC_DrawSpriteScreenspace, false },

    { "_Z11rsgDrawMesh7rs_mesh", (void *)&SC_DrawMesh, false },
    { "_Z11rsgDrawMesh7rs_meshj", (void *)&SC_DrawMeshPrimitive, false },
    { "_Z11rsgDrawMesh7rs_meshjjj", (void *)&SC_DrawMeshPrimitiveRange, false },
    { "_Z25rsgMeshComputeBoundingBox7rs_meshPfS0_S0_S0_S0_S0_", (void *)&SC_MeshComputeBoundingBox, false },

    { "_Z11rsgDrawPath7rs_path", (void *)&SC_DrawPath, false },

    { "_Z13rsgClearColorffff", (void *)&SC_ClearColor, false },
    { "_Z13rsgClearDepthf", (void *)&SC_ClearDepth, false },

    { "_Z11rsgDrawTextPKcii", (void *)&SC_DrawText, false },
    { "_Z11rsgDrawText13rs_allocationii", (void *)&SC_DrawTextAlloc, false },
    { "_Z14rsgMeasureTextPKcPiS1_S1_S1_", (void *)&SC_MeasureText, false },
    { "_Z14rsgMeasureText13rs_allocationPiS0_S0_S0_", (void *)&SC_MeasureTextAlloc, false },

    { "_Z11rsgBindFont7rs_font", (void *)&SC_BindFont, false },
    { "_Z12rsgFontColorffff", (void *)&SC_FontColor, false },

    { "_Z18rsgBindColorTarget13rs_allocationj", (void *)&SC_BindFrameBufferObjectColorTarget, false },
    { "_Z18rsgBindDepthTarget13rs_allocation", (void *)&SC_BindFrameBufferObjectDepthTarget, false },
    { "_Z19rsgClearColorTargetj", (void *)&SC_ClearFrameBufferObjectColorTarget, false },
    { "_Z19rsgClearDepthTargetv", (void *)&SC_ClearFrameBufferObjectDepthTarget, false },
    { "_Z24rsgClearAllRenderTargetsv", (void *)&SC_ClearFrameBufferObjectTargets, false },

    { "_Z9rsForEach9rs_script13rs_allocationS0_", (void *)&SC_ForEach_SAA, true },
    { "_Z9rsForEach9rs_script13rs_allocationS0_PKv", (void *)&SC_ForEach_SAAU, true },
    { "_Z9rsForEach9rs_script13rs_allocationS0_PKvPK14rs_script_call", (void *)&SC_ForEach_SAAUS, true },
    { "_Z9rsForEach9rs_script13rs_allocationS0_PKvj", (void *)&SC_ForEach_SAAUL, true },
    { "_Z9rsForEach9rs_script13rs_allocationS0_PKvjPK14rs_script_call", (void *)&SC_ForEach_SAAULS, true },

    // time
    { "_Z6rsTimePi", (void *)&SC_Time, true },
    { "_Z11rsLocaltimeP5rs_tmPKi", (void *)&SC_LocalTime, true },
    { "_Z14rsUptimeMillisv", (void*)&SC_UptimeMillis, true },
    { "_Z13rsUptimeNanosv", (void*)&SC_UptimeNanos, true },
    { "_Z7rsGetDtv", (void*)&SC_GetDt, false },

    // misc
    { "_Z5colorffff", (void *)&SC_Color, false },
    { "_Z9rsgFinishv", (void *)&SC_Finish, false },

    { NULL, NULL, false }
};


extern const RsdCpuReference::CpuSymbol * rsdLookupRuntimeStub(Context * pContext, char const* name) {
    ScriptC *s = (ScriptC *)pContext;
    const RsdCpuReference::CpuSymbol *syms = gSyms;
    const RsdCpuReference::CpuSymbol *sym = NULL;

    if (!sym) {
        while (syms->fnPtr) {
            if (!strcmp(syms->name, name)) {
                return syms;
            }
            syms++;
        }
    }

    return NULL;
}


