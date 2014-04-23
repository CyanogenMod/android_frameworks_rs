#
# Copyright (C) 2013 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := $(call my-dir)

# C/LLVM-IR source files for the library
clcore_base_files := \
    rs_allocation.c \
    rs_cl.c \
    rs_core.c \
    rs_element.c \
    rs_mesh.c \
    rs_matrix.c \
    rs_program.c \
    rs_sample.c \
    rs_sampler.c \
    convert.ll \
    allocation.ll \
    rsClamp.ll

clcore_files := \
    $(clcore_base_files) \
    math.ll \
    arch/generic.c

clcore_neon_files := \
    $(clcore_base_files) \
    math.ll \
    arch/neon.ll \
    arch/clamp.c

clcore_x86_files := \
    $(clcore_base_files) \
    arch/generic.c \
    arch/x86_sse2.ll \
    arch/x86_sse3.ll

ifeq "REL" "$(PLATFORM_VERSION_CODENAME)"
  RS_VERSION := $(PLATFORM_SDK_VERSION)
else
  # Increment by 1 whenever this is not a final release build, since we want to
  # be able to see the RS version number change during development.
  # See build/core/version_defaults.mk for more information about this.
  RS_VERSION := "(1 + $(PLATFORM_SDK_VERSION))"
endif

# Build the base version of the library
include $(CLEAR_VARS)

# FIXME for 64-bit
LOCAL_32_BIT_ONLY := true

LOCAL_MODULE := libclcore.bc
LOCAL_SRC_FILES := $(clcore_files)

include $(LOCAL_PATH)/build_bc_lib.mk

# Build a debug version of the library
include $(CLEAR_VARS)

# FIXME for 64-bit
LOCAL_32_BIT_ONLY := true

LOCAL_MODULE := libclcore_debug.bc
rs_debug_runtime := 1
LOCAL_SRC_FILES := $(clcore_files)

include $(LOCAL_PATH)/build_bc_lib.mk

# Build an optimized version of the library for x86 platforms (all have SSE2/3).
ifeq ($(TARGET_ARCH),$(filter $(TARGET_ARCH),x86 x86_64))
include $(CLEAR_VARS)

# FIXME for 64-bit
LOCAL_32_BIT_ONLY := true

LOCAL_MODULE := libclcore_x86.bc
LOCAL_SRC_FILES := $(clcore_x86_files)

include $(LOCAL_PATH)/build_bc_lib.mk
endif

# Build a NEON-enabled version of the library (if possible)
ifeq ($(ARCH_ARM_HAVE_NEON),true)
  include $(CLEAR_VARS)

  # FIXME for 64-bit
  LOCAL_32_BIT_ONLY := true

  LOCAL_MODULE := libclcore_neon.bc
  LOCAL_SRC_FILES := $(clcore_neon_files)
  LOCAL_CFLAGS += -DARCH_ARM_HAVE_NEON

  include $(LOCAL_PATH)/build_bc_lib.mk
endif

### Build new versions (librsrt_<ARCH>.bc) as host shared libraries.
### These will be used with bcc_compat and the support library.

# Build the ARM version of the library
include $(CLEAR_VARS)

# FIXME for 64-bit
LOCAL_32_BIT_ONLY := true

BCC_RS_TRIPLE := armv7-none-linux-gnueabi
LOCAL_MODULE := librsrt_arm.bc
LOCAL_IS_HOST_MODULE := true
LOCAL_SRC_FILES := $(clcore_files)
include $(LOCAL_PATH)/build_bc_lib.mk

# Build the MIPS version of the library
include $(CLEAR_VARS)

# FIXME for 64-bit
LOCAL_32_BIT_ONLY := true

BCC_RS_TRIPLE := mipsel-unknown-linux
LOCAL_MODULE := librsrt_mips.bc
LOCAL_IS_HOST_MODULE := true
LOCAL_SRC_FILES := $(clcore_files)
include $(LOCAL_PATH)/build_bc_lib.mk

# Build the x86 version of the library
include $(CLEAR_VARS)

# FIXME for 64-bit
LOCAL_32_BIT_ONLY := true

BCC_RS_TRIPLE := i686-unknown-linux
LOCAL_MODULE := librsrt_x86.bc
LOCAL_IS_HOST_MODULE := true
LOCAL_SRC_FILES := $(clcore_x86_files)
include $(LOCAL_PATH)/build_bc_lib.mk

