/*
 * Copyright (C) 2015 The Android Open Source Project
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

// Warning: This header file is shared with the core runtime, which is written in C;
// hence, this header file must be written in C.

#ifndef RSD_CPU_CORE_RUNTIME_H
#define RSD_CPU_CORE_RUNTIME_H

struct RsExpandKernelParams {

    // Used by kernels
    const void **ins;
    uint32_t *inEStrides;
    void *out;
    uint32_t y;
    uint32_t z;
    uint32_t lid;

    // Used by ScriptGroup and user kernels.
    const void *usr;

    // Used by intrinsics
    uint32_t dimX;
    uint32_t dimY;
    uint32_t dimZ;

    /*
     * FIXME: This is only used by the blend intrinsic.  If possible, we should
     *        modify blur to not need it.
     */
    uint32_t slot;
};

#endif
