#
# Copyright (c) 2015-2021 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# TODO: Update LOCAL_MODULE to match the correct project setup
# ifeq ($(TARGET_DEVICE),evb6795_64_tee)
# LOCAL_MODULE := fingerprint.mt6795
# else ifeq ($(PRODUCT_MODEL),evb6797_64_teei)
# LOCAL_MODULE := fingerprint.mt6797
# else
# LOCAL_MODULE := fingerprint.$(TARGET_DEVICE)
# LOCAL_MULTILIB := first
# endif

LOCAL_MODULE := fingerprint.fpc.default
LOCAL_MULTILIB := first
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := fpc

PLATFORM_VERSION_MAJOR := $(word 1, $(subst ., ,$(PLATFORM_VERSION)))

ifeq ($(PLATFORM_VERSION_MAJOR),6)
ENABLE_LEGACY_EXTENSION=1
endif

ifeq ($(PLATFORM_VERSION_MAJOR),7)
ENABLE_LEGACY_EXTENSION=1
endif

ifeq ($(ENABLE_LEGACY_EXTENSION),)
LOCAL_PROPRIETARY_MODULE := true
endif

LOCAL_CFLAGS := -std=c99 -Wall -Werror\
    -DLOG_TAG='"fpc_fingerprint_hal"' \

LOCAL_SRC_FILES := fpc_legacy_hal.c

ifeq ($(FPC_CONFIG_DEBUG),1)
LOCAL_CFLAGS += -DFPC_DEBUG_LOGGING
endif

LOCAL_C_INCLUDES += $(LOCAL_PATH)/ \
                    $(LOCAL_PATH)/../../fingerprint_tac/normal/inc/ \
                    $(LOCAL_PATH)/../../fingerprint_tac/interface/ \
                    $(LOCAL_PATH)/../fingerprint_common \
                    $(LOCAL_PATH)/../../fingerprint_hal/fingerprint_common/sensortest \
                    $(LOCAL_PATH)/../../fingerprint_hal/fingerprint_common/engineering \
                    hardware/libhardware/include

LOCAL_SHARED_LIBRARIES := liblog libutils
LOCAL_STATIC_LIBRARIES := fpc_hal_common fpc_tac libcutils

ifeq ($(ENABLE_LEGACY_EXTENSION),1)
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_STATIC_LIBRARIES += fpc_hal_extension_legacy
else
LOCAL_SHARED_LIBRARIES += libhidlbase \
                          libhwbinder \
                          liblog \
                          libutils \
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

LOCAL_STATIC_LIBRARIES += fpc_hal_extension \
                          com.fingerprints.extension@1.0
endif

ifeq ($(FPC_TEE_RUNTIME), TBASE)
LOCAL_SHARED_LIBRARIES += libMcClient
else ifeq ($(FPC_TEE_RUNTIME), QSEE)
LOCAL_SHARED_LIBRARIES += libQSEEComAPI libion
else ifeq ($(FPC_TEE_RUNTIME), ANDROID)
LOCAL_SHARED_LIBRARIES += lib_fpc_ta_shared
else ifeq ($(FPC_TEE_RUNTIME), TRUSTY)
LOCAL_SHARED_LIBRARIES += libtrusty
endif
include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))
