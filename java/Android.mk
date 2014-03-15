LOCAL_PATH:=$(call my-dir)

ifneq ($(TARGET_ARCH), arm64)

# Only build our tests if we doing a top-level build. Do not build the
# tests if we are just doing an mm or mmm in frameworks/rs.
ifeq (,$(ONE_SHOT_MAKEFILE))
include $(call all-makefiles-under,$(LOCAL_PATH))
endif

endif