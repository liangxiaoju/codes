LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	external/libnl-headers

LOCAL_STATIC_LIBRARIES := libc

ifeq ($(PLATFORM_VERSION),4.4.4)
	LOCAL_STATIC_LIBRARIES += libnl_2
else
	LOCAL_STATIC_LIBRARIES += libnl
endif

LOCAL_MODULE := klogd

LOCAL_SRC_FILES := \
	klog.c \
	main.c \
	ssr_record.c \
	other_record.c \
	mmc_record.c

LOCAL_MODULE_CLASS := EXECUTABLES

LOCAL_FORCE_STATIC_EXECUTABLE := true

include $(BUILD_EXECUTABLE)

ALL_DEFAULT_INSTALLED_MODULES += klogd
