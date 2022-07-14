#
# Copyright (c) 2020-2021 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.
#
# =============================================================================
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := fpc_tac_common
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := fpc
LOCAL_CFLAGS := -Wall -Wextra -Wshadow -Wmissing-prototypes -Wunused -Wunused-result \
                -Werror -O2 -DLOG_TAG='"fpc_tac"' -std=c11

ifeq ($(FPC_TEE_RUNTIME), TBASE)
LOCAL_CFLAGS += -DTBASE
else ifeq ($(FPC_TEE_RUNTIME), QSEE)
LOCAL_CFLAGS += -DQSEE
else ifeq ($(FPC_TEE_RUNTIME), ISEE)
LOCAL_CFLAGS += -DISEE
else ifeq ($(FPC_TEE_RUNTIME), TRUSTY)
LOCAL_CFLAGS += -DTRUSTY
else ifeq ($(FPC_TEE_RUNTIME), ANDROID)
LOCAL_CFLAGS += -DFPC_TEE_ANDROID
endif

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

ifeq ($(TARGET_PRODUCT),qemu_trusty_arm64)
#Simply bypass the calls to fpc sysfs api, for example for QEMU + Trusty
LOCAL_CFLAGS += -DFPC_CONFIG_SIMPLY_BYPASS_FPC_SYSFS_APIS
endif

ifeq ($(FPC_STORE_IN_DATA_DIR),true)
LOCAL_CFLAGS += -DFPC_STORE_IN_DATA_DIR
endif

ifeq ($(FPC_CONFIG_SWIPE_ENROL),1)
LOCAL_CFLAGS += -DFPC_CONFIG_SWIPE_ENROL
endif

ifeq ($(FPC_CONFIG_TRUSTY_SC),1)
LOCAL_CFLAGS += -DFPC_CONFIG_TRUSTY_SC
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
                    system/core/include
endif
endif

LOCAL_SRC_FILES := \
                   src/fpc_sysfs.c \
                   src/fpc_tee.c \
                   src/navigation/fpc_tee_nav.c \
                   src/sensortest/fpc_tee_sensortest.c \
                   src/engineering/fpc_tee_engineering.c \
                   ../interface/fpc_error_str.c \
                   src/hw_auth/fpc_tee_hw_auth.c \
                   src/fido_auth/fpc_tee_fido_auth.c \
                   src/fpc_tee_host_storage.c \
                   src/fpc_tee_storage.c

ifdef FPC_CONFIG_ENGINEERING
LOCAL_CFLAGS += \
    -DFPC_CONFIG_ENGINEERING
endif

ifdef FPC_CONFIG_NAVIGATION
LOCAL_CFLAGS += \
    -DFPC_CONFIG_NAVIGATION
endif

ifdef FPC_CONFIG_FIDO_AUTH
LOCAL_CFLAGS += \
    -DFPC_CONFIG_FIDO_AUTH
endif

ifdef FPC_CONFIG_SENSORTEST
LOCAL_CFLAGS += \
    -DFPC_CONFIG_SENSORTEST
endif

LOCAL_SRC_FILES += \
    src/kpi/fpc_tee_kpi.c

ifneq ($(FPC_PLATFORM_TARGET),fpc58XX)
LOCAL_SRC_FILES += src/fpc_irq_device.c
endif

ifndef FPC_CONFIG_NO_SENSOR
LOCAL_SRC_FILES  += src/fpc_tee_sensor.c
else
LOCAL_SRC_FILES  += src/fpc_tee_sensor_dummy.c
endif

ifndef FPC_CONFIG_NO_ALGO
LOCAL_SRC_FILES  += src/fpc_tee_bio.c
else
LOCAL_SRC_FILES  += src/fpc_tee_bio_dummy.c
endif

ifdef FPC_CONFIG_NORMAL_SPI_RESET
LOCAL_SRC_FILES += src/fpc_reset_device.c
LOCAL_CFLAGS += -DFPC_CONFIG_NORMAL_SPI_RESET
endif

ifdef FPC_CONFIG_NORMAL_SENSOR_RESET
ifndef FPC_CONFIG_NORMAL_SPI_RESET
LOCAL_SRC_FILES += src/fpc_reset_device.c
endif
LOCAL_CFLAGS += -DFPC_CONFIG_NORMAL_SENSOR_RESET
endif

ifeq ($(FPC_CONFIG_DEBUG),1)
LOCAL_CFLAGS += -DFPC_DEBUG_LOGGING -DFPC_CONFIG_DEBUG
endif

ifneq ($(FPC_CONFIG_RETRY_MATCH_TIMEOUT),)
LOCAL_CFLAGS += -DFPC_CONFIG_RETRY_MATCH_TIMEOUT=$(FPC_CONFIG_RETRY_MATCH_TIMEOUT)
endif

ifeq ($(FPC_CONFIG_ENROL_TIMEOUT),1)
LOCAL_CFLAGS += -DFPC_CONFIG_ENROL_TIMEOUT
endif

LOCAL_SHARED_LIBRARIES := liblog libcutils

LOCAL_EXPORT_C_INCLUDE_DIRS += \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/../interface \
    $(LOCAL_PATH)/inc/hw_auth \
    $(LOCAL_PATH)/inc/fido_auth \
    $(LOCAL_PATH)/inc/sensortest \
    $(LOCAL_PATH)/inc/engineering \
    $(LOCAL_PATH)/inc/kpi \
    $(LOCAL_PATH)/inc/navigation

LOCAL_EXPORT_C_INCLUDES += $(LOCAL_EXPORT_C_INCLUDE_DIRS)

LOCAL_HEADER_LIBRARIES += libhardware_headers

include $(BUILD_STATIC_LIBRARY)
