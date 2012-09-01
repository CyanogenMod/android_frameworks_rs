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

#ifndef RSD_INTRINSICS_H
#define RSD_INTRINSICS_H

#include <rs_hal.h>

typedef struct RsdIntriniscFuncs_rec {

    void (*bind)(const android::renderscript::Context *dc,
                 const android::renderscript::Script *script,
                 void * intrinsicData,
                 uint32_t slot, android::renderscript::Allocation *data);
    void (*setVar)(const android::renderscript::Context *dc,
                   const android::renderscript::Script *script,
                   void * intrinsicData,
                   uint32_t slot, void *data, size_t dataLength);
    void (*root)(const android::renderscript::RsForEachStubParamStruct *,
                 uint32_t x1, uint32_t x2, uint32_t instep, uint32_t outstep);

} RsdIntriniscFuncs_t;

bool rsdIntrinsic_Init(const android::renderscript::Context *dc,
                       android::renderscript::Script *script,
                       RsScriptIntrinsicID id, RsdIntriniscFuncs_t *funcs);



#endif // RSD_INTRINSICS_H

