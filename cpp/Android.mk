rs_cpp_SRC_FILES := \
	RenderScript.cpp \
	BaseObj.cpp \
	Element.cpp \
	Type.cpp \
	Allocation.cpp \
	Script.cpp \
	ScriptC.cpp \
	ScriptIntrinsics.cpp \
	Sampler.cpp

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk

include frameworks/compile/slang/rs_version.mk
local_cflags_for_rs_cpp += $(RS_VERSION_DEFINE)
local_cflags_for_rs_cpp += -Wno-unused-parameter -std=c++11

LOCAL_SRC_FILES := $(rs_cpp_SRC_FILES)

ifneq ($(HOST_OS),windows)
LOCAL_CLANG := true
endif
LOCAL_CFLAGS += $(local_cflags_for_rs_cpp)

LOCAL_SHARED_LIBRARIES := \
	libz \
	libcutils \
	libutils \
	liblog \
	libdl \

LOCAL_MODULE:= libRScpp

LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES += frameworks/rs
LOCAL_C_INCLUDES += $(intermediates)

include external/stlport/libstlport.mk
include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk

ifneq ($(HOST_OS),windows)
LOCAL_CLANG := true
endif
LOCAL_CFLAGS += $(local_cflags_for_rs_cpp)

ifeq ($(my_32_64_bit_suffix),32)
LOCAL_SDK_VERSION := 8
else
LOCAL_SDK_VERSION := 21
endif
LOCAL_CFLAGS += -DRS_COMPATIBILITY_LIB

LOCAL_SRC_FILES := $(rs_cpp_SRC_FILES)

LOCAL_SRC_FILES += ../rsCompatibilityLib.cpp

LOCAL_WHOLE_STATIC_LIBRARIES := \
	libutils \
	libstlport_static

LOCAL_MODULE:= libRScpp_static

LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES += frameworks/rs
LOCAL_C_INCLUDES += $(intermediates)

LOCAL_LDFLAGS := -llog -lz -ldl
LOCAL_NDK_STL_VARIANT := stlport_static

include $(BUILD_STATIC_LIBRARY)
