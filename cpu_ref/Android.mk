
LOCAL_PATH:=$(call my-dir)

rs_base_CFLAGS := -Werror -Wall -Wno-unused-parameter -Wno-unused-variable
ifeq ($(TARGET_BUILD_PDK), true)
  rs_base_CFLAGS += -D__RS_PDK__
endif

ifneq ($(OVERRIDE_RS_DRIVER),)
  rs_base_CFLAGS += -DOVERRIDE_RS_DRIVER=$(OVERRIDE_RS_DRIVER)
endif

include $(CLEAR_VARS)
LOCAL_CLANG := true
LOCAL_MODULE := libRSCpuRef
LOCAL_MODULE_TARGET_ARCH := arm mips x86 x86_64

LOCAL_SRC_FILES:= \
	rsCpuCore.cpp \
	rsCpuScript.cpp \
	rsCpuRuntimeMath.cpp \
	rsCpuRuntimeStubs.cpp \
	rsCpuScriptGroup.cpp \
	rsCpuIntrinsic.cpp \
	rsCpuIntrinsic3DLUT.cpp \
	rsCpuIntrinsicBlend.cpp \
	rsCpuIntrinsicBlur.cpp \
	rsCpuIntrinsicColorMatrix.cpp \
	rsCpuIntrinsicConvolve3x3.cpp \
	rsCpuIntrinsicConvolve5x5.cpp \
	rsCpuIntrinsicHistogram.cpp \
	rsCpuIntrinsicInterPred.cpp \
	rsCpuIntrinsicLUT.cpp \
	rsCpuIntrinsicYuvToRGB.cpp \
	convolve/convolve.c

LOCAL_CFLAGS_arm64 += -DARCH_ARM_HAVE_NEON
LOCAL_SRC_FILES_arm64 += \
    rsCpuIntrinsics_advsimd_Blend.S \
    rsCpuIntrinsics_advsimd_Blur.S \
    rsCpuIntrinsics_advsimd_YuvToRGB.S

ifeq ($(ARCH_ARM_HAVE_NEON),true)
    LOCAL_CFLAGS_arm += -DARCH_ARM_HAVE_NEON
endif

ifeq ($(ARCH_ARM_HAVE_VFP),true)
    LOCAL_CFLAGS_arm += -DARCH_ARM_HAVE_VFP
    LOCAL_SRC_FILES_arm += \
    rsCpuIntrinsics_neon.S \
    rsCpuIntrinsics_neon_ColorMatrix.S \
    rsCpuIntrinsics_neon_Blend.S \
    rsCpuIntrinsics_neon_Blur.S \
    rsCpuIntrinsics_neon_YuvToRGB.S
    convolve/convolve_copy_neon.s \
    convolve/convolve_avg_neon.s \
    convolve/convolve8_neon.s \
    convolve/convolve8_avg_neon.s \
    convolve/convolve_neon.c
    LOCAL_ASFLAGS_arm := -mfpu=neon
endif

LOCAL_SHARED_LIBRARIES += libRS libcutils libutils liblog libsync
LOCAL_SHARED_LIBRARIES += libbcc libbcinfo

LOCAL_C_INCLUDES += frameworks/compile/libbcc/include
LOCAL_C_INCLUDES += frameworks/rs

LOCAL_CFLAGS += $(rs_base_CFLAGS)

LOCAL_LDLIBS := -lpthread -ldl
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
