#
# Copyright (c) 2015-2021 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.

LOCAL_PATH := $(call my-dir)

ifeq ($(FPC_HAL_SHARED_LIB),)
include $(CLEAR_VARS)
$(warning "FPC_HAL_SHARED_LIB is no")
#LOCAL_MODULE := android.hardware.biometrics.fingerprint@2.1-service
LOCAL_MODULE := fingerprint.fpc.service_default
LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := android.hardware.biometrics.fingerprint@2.1-service.rc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := fpc
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MULTILIB := first
LOCAL_SRC_FILES := service.cpp fpc_hidl.cpp
LOCAL_CFLAGS := -DLOG_TAG='"fpc_hidl"'

PLATFORM_VERSION_MAJOR := $(word 1, $(subst ., ,$(PLATFORM_VERSION)))

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../fingerprint_common

LOCAL_SHARED_LIBRARIES := liblog \
                          libhidlbase \
                          libutils \
                          android.hardware.biometrics.fingerprint@2.1 \
                          libhwbinder \
                          libcutils

ifeq ($(PLATFORM_VERSION_MAJOR),8)
LOCAL_SHARED_LIBRARIES += libhidltransport
endif

ifeq ($(PLATFORM_VERSION_MAJOR),9)
LOCAL_SHARED_LIBRARIES += libhidltransport
endif

ifeq ($(PLATFORM_VERSION_MAJOR),10)
LOCAL_SHARED_LIBRARIES += libhidltransport
endif

LOCAL_STATIC_LIBRARIES := fpc_hal_common \
                          fpc_tac \
                          fpc_hal_extension \
                          com.fingerprints.extension@1.0

ifeq ($(FPC_TEE_RUNTIME), TBASE)
LOCAL_SHARED_LIBRARIES += libMcClient
else ifeq ($(FPC_TEE_RUNTIME), QSEE)
LOCAL_SHARED_LIBRARIES += libQSEEComAPI libion
else ifeq ($(FPC_TEE_RUNTIME), ANDROID)
LOCAL_SHARED_LIBRARIES += lib_fpc_ta_shared
else ifeq ($(FPC_TEE_RUNTIME), TRUSTY)
LOCAL_SHARED_LIBRARIES += libtrusty
endif

include $(BUILD_EXECUTABLE)
else
$(warning "FPC_HAL_SHARED_LIB defined")
endif

include $(call all-makefiles-under,$(LOCAL_PATH))
