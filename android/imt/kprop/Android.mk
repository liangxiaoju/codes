LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= kprop.c

LOCAL_STATIC_LIBRARIES := libc libstdc++ libcutils

LOCAL_MODULE:= kprop
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT_SBIN)
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)