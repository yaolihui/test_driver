#
# Copyright (c) 2016-2021 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.
#
# =============================================================================

LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/fpc_android_common.mk
include $(CLEAR_VARS)
LOCAL_MODULE := fpc_tac
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := fpc
LOCAL_MULTILIB := first
LOCAL_CFLAGS := -Wall -Wextra -Wshadow -Wmissing-prototypes -Wunused -Wunused-result \
                -Werror -O2 -DLOG_TAG='"fpc_tac"' -std=c11

ifneq ($(filter 8.%,$(PLATFORM_VERSION)),)
FPC_STORE_IN_DATA_DIR := true
endif

ifneq ($(filter 7.%,$(PLATFORM_VERSION)),)
FPC_SYSTEM_LIB := true
FPC_STORE_IN_DATA_DIR := true
endif

ifneq ($(filter 6.%,$(PLATFORM_VERSION)),)
FPC_SYSTEM_LIB := true
FPC_STORE_IN_DATA_DIR := true
endif

ifeq ($(FPC_STORE_IN_DATA_DIR),true)
LOCAL_CFLAGS += -DFPC_STORE_IN_DATA_DIR
endif

ifeq ($(FPC_CONFIG_SWIPE_ENROL),1)
LOCAL_CFLAGS += -DFPC_CONFIG_SWIPE_ENROL
endif

#ifeq ($(FPC_CONFIG_DEBUG),)
#FPC_CONFIG_DEBUG=1
#endif

ifeq ($(FPC_CONFIG_DEBUG),1)
LOCAL_CFLAGS += -DFPC_DEBUG_LOGGING -DFPC_CONFIG_DEBUG
endif

ifeq ($(FPC_SYSTEM_LIB),)
# In Android O we should place the library in /vendor/lib64 insteadof /system/lib64
LOCAL_PROPRIETARY_MODULE := true
endif

LOCAL_C_INCLUDES := \
                    $(LOCAL_PATH)/inc/ \
                    $(LOCAL_PATH)/inc/kpi \
                    $(LOCAL_PATH)/inc/hw_auth \
                    $(LOCAL_PATH)/inc/fido_auth \
                    $(LOCAL_PATH)/inc/engineering \
                    $(LOCAL_PATH)/inc/sensortest \
                    $(LOCAL_PATH)/inc/navigation \
                    $(LOCAL_PATH)/../interface

#Not to include these for Android M/N
ifeq ($(filter 6.%,$(PLATFORM_VERSION)),)
ifeq ($(filter 7.%,$(PLATFORM_VERSION)),)
LOCAL_C_INCLUDES += \
                    system/core/include \
                    system/logging/liblog/include
endif
endif

LOCAL_CFLAGS += -DFPC_CONFIG_KEYMASTER_APP_PATH='"$(FPC_CONFIG_KEYMASTER_APP_PATH)"'
LOCAL_CFLAGS += -DFPC_CONFIG_KEYMASTER_NAME='"$(FPC_CONFIG_KEYMASTER_NAME)"'
LOCAL_HEADER_LIBRARIES += libhardware_headers

ifeq ($(FPC_TEE_RUNTIME), TBASE)
ifeq ($(FPC_TBASE_VER), KINIBI)
include $(LOCAL_PATH)/../normal/platform/tbase/kinibi.mk
else
include $(LOCAL_PATH)/../normal/platform/tbase/tbase.mk
endif
else ifeq ($(FPC_TEE_RUNTIME), QSEE)
include $(LOCAL_PATH)/../normal/platform/qsee/qsee.mk
else ifeq ($(FPC_TEE_RUNTIME), ANDROID)
include $(LOCAL_PATH)/../normal/platform/android/droid.mk
else ifeq ($(FPC_TEE_RUNTIME), ISEE)
LOCAL_CFLAGS += -DFPC_CONFIG_TEE_ISEE
include $(LOCAL_PATH)/../normal/platform/isee/isee.mk
else ifeq ($(FPC_TEE_RUNTIME), TRUSTY)
include $(LOCAL_PATH)/../normal/platform/trusty/trusty.mk
else
$(warning "Unknown FPC_TEE_RUNTIME=$(FPC_TEE_RUNTIME)")
endif

LOCAL_WHOLE_STATIC_LIBRARIES  := fpc_tac_common
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

ifneq ($(filter 7.%,$(PLATFORM_VERSION)),)
FPC_SYSTEM_LIB := true
endif

ifneq ($(filter 6.%,$(PLATFORM_VERSION)),)
FPC_SYSTEM_LIB := true
endif

ifeq ($(FPC_SYSTEM_LIB),)
# In Android O we should place the library in /vendor/bin insteadof /system/bin
LOCAL_PROPRIETARY_MODULE := true
endif

LOCAL_MODULE := fpc_tee_test
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := fpc
#LOCAL_MULTILIB := first

LOCAL_C_INCLUDES := \
                    $(LOCAL_PATH)/inc/ \
                    $(LOCAL_PATH)/inc/kpi/ \
                    $(LOCAL_PATH)/../interface \
                    $(LOCAL_PATH)/inc/engineering \
                    $(LOCAL_PATH)/inc/hw_auth \
                    $(LOCAL_PATH)/inc/sensortest

LOCAL_SRC_FILES := \
                   src/test/fpc_tee_test.c \
                   src/test/fpc_tee_db_blob_test.c

LOCAL_CFLAGS := -Wall -Wshadow -Wunused -Wunused-result -std=c11 -Wextra -Werror \
                -Wimplicit-function-declaration -Wconversion -DLOG_TAG='"fpc_tee_test"'

LOCAL_STATIC_LIBRARIES := fpc_tac

LOCAL_SHARED_LIBRARIES := liblog libdl libcutils

LOCAL_PROPRIETARY_MODULE := true
LOCAL_SHARED_LIBRARIES += libMcClient
#ifeq ($(FPC_TEE_RUNTIME), TBASE)
#LOCAL_SHARED_LIBRARIES += libMcClient
#else ifeq ($(FPC_TEE_RUNTIME), QSEE)
#LOCAL_SHARED_LIBRARIES += libQSEEComAPI libion
#else ifeq ($(FPC_TEE_RUNTIME), TRUSTY)
#LOCAL_SHARED_LIBRARIES += libtrusty
#else ifeq ($(FPC_TEE_RUNTIME), ANDROID)
#LOCAL_SHARED_LIBRARIES += lib_fpc_ta_shared
#endif

include $(BUILD_EXECUTABLE)
