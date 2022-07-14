#
# Copyright (c) 2015-2021 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := fpc_hal_common
LOCAL_MODULE_OWNER := fpc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MULTILIB := first
LOCAL_CFLAGS := -Wall -Werror \
    -DLOG_TAG='"fpc_fingerprint_hal"' \

LOCAL_CONLYFLAGS := -std=c99

LOCAL_SRC_FILES := fpc_tee_hal.c \
                   fpc_worker.c \
                   navigation/fpc_hal_input_device.c \
                   engineering/fpc_hal_ext_engineering.c \
                   sensortest/fpc_hal_ext_sensortest.c \
                   authenticator/fpc_hal_ext_authenticator.c \
                   authenticator2/fpc_hal_ext_authenticator_2.c \
                   navigation/fpc_hal_navigation.c \
                   sensetouch/fpc_hal_ext_sense_touch.c \
                   sensetouch/fpc_hal_sense_touch.c

ifneq ($(filter 7.%,$(PLATFORM_VERSION)),)
LOCAL_CFLAGS += -DPRE_TREBLE_HAL
endif

ifneq ($(filter 6.%,$(PLATFORM_VERSION)),)
LOCAL_CFLAGS += -DPRE_TREBLE_HAL
endif

ifeq ($(FPC_CONFIG_DEBUG),1)
LOCAL_CFLAGS += -DFPC_DEBUG_LOGGING
endif

ifeq ($(FPC_CONFIG_ENGINEERING),1)
LOCAL_CFLAGS += -DFPC_CONFIG_ENGINEERING
endif

# Enable authentication framework support
ifeq ($(FPC_CONFIG_FIDO_AUTH),1)
LOCAL_CFLAGS         += -DFPC_CONFIG_FIDO_AUTH
    ifeq ($(FPC_CONFIG_FIDO_AUTH_VER_GMRZ),1)
    LOCAL_CFLAGS         += -DFPC_CONFIG_FIDO_AUTH_VER_GMRZ
    else
    LOCAL_CFLAGS         += -DFPC_CONFIG_FIDO_AUTH_VER_GENERIC
    endif
endif

# Retry on no match feature
ifneq ($(FPC_CONFIG_RETRY_MATCH_TIMEOUT),)
LOCAL_CFLAGS         += -DFPC_CONFIG_RETRY_MATCH_TIMEOUT=$(FPC_CONFIG_RETRY_MATCH_TIMEOUT)
endif

ifeq ($(FPC_CONFIG_SWIPE_ENROL),1)
LOCAL_CFLAGS += -DFPC_CONFIG_SWIPE_ENROL
endif

ifeq ($(FPC_CONFIG_ENROL_TIMEOUT),1)
LOCAL_CFLAGS    += -DFPC_CONFIG_ENROL_TIMEOUT
endif

# Sense Touch Calibration
LOCAL_CFLAGS         += -DSENSE_TOUCH_CALIBRATION_PATH='"$(FPC_CONFIG_SENSE_TOUCH_CALIBRATION_PATH)"'

LOCAL_C_INCLUDES += $(LOCAL_PATH) \
                    $(LOCAL_PATH)/engineering \
                    $(LOCAL_PATH)/sensortest \
                    $(LOCAL_PATH)/authenticator \
                    $(LOCAL_PATH)/authenticator2 \
                    $(LOCAL_PATH)/navigation \
                    $(LOCAL_PATH)/sensetouch \
                    $(LOCAL_PATH)/../../fingerprint_tac/interface \
                    $(LOCAL_PATH)/../../fingerprint_tac/normal/inc \
                    $(LOCAL_PATH)/../../fingerprint_tac/normal/inc/hw_auth \
                    $(LOCAL_PATH)/../../fingerprint_tac/normal/inc/fido_auth \
                    $(LOCAL_PATH)/../../fingerprint_tac/normal/inc/sensortest \
                    $(LOCAL_PATH)/../../fingerprint_tac/normal/inc/engineering \
                    $(LOCAL_PATH)/../../fingerprint_tac/normal/inc/kpi \
                    $(LOCAL_PATH)/../../fingerprint_tac/normal/inc/navigation

LOCAL_SHARED_LIBRARIES := liblog \
                          libutils

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)
ifeq ($(FPC_TEE_RUNTIME), QSEE)
LOCAL_SHARED_LIBRARIES +=  libQSEEComAPI
endif

LOCAL_STATIC_LIBRARIES += libcutils fpc_tac

include $(BUILD_STATIC_LIBRARY)
