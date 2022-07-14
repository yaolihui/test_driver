#
# Copyright (c) 2017-2020 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := fpc_hal_extension
LOCAL_MODULE_OWNER := fpc

PLATFORM_VERSION_MAJOR := $(word 1, $(subst ., ,$(PLATFORM_VERSION)))

LOCAL_PROPRIETARY_MODULE := true

LOCAL_CFLAGS := -Wall -Werror \
    -DLOG_TAG='"fpc_hal_extension"' \

LOCAL_SRC_FILES  += authenticator/FingerprintAuthenticator.cpp \
                    authenticator2/FpcFingerprintAuthenticator.cpp \
                    engineering/FingerprintEngineering.cpp \
                    navigation/FingerprintNavigation.cpp \
                    sensetouch/FingerprintSenseTouch.cpp \
                    sensortest/FingerprintSensorTest.cpp

COMMON_INCLUDE_PATH := $(LOCAL_PATH)/../../fingerprint_common

LOCAL_C_INCLUDES += $(COMMON_INCLUDE_PATH)/ \
                    $(COMMON_INCLUDE_PATH)/authenticator \
                    $(COMMON_INCLUDE_PATH)/authenticator2 \
                    $(COMMON_INCLUDE_PATH)/engineering \
                    $(COMMON_INCLUDE_PATH)/navigation \
                    $(COMMON_INCLUDE_PATH)/sensetouch \
                    $(COMMON_INCLUDE_PATH)/sensortest

LOCAL_SHARED_LIBRARIES := liblog \
                          libutils \
                          libhidlbase \
                          com.fingerprints.extension@1.0

ifeq ($(PLATFORM_VERSION_MAJOR),8)
LOCAL_SHARED_LIBRARIES += libhidltransport
endif

ifeq ($(PLATFORM_VERSION_MAJOR),9)
LOCAL_SHARED_LIBRARIES += libhidltransport
endif

ifeq ($(PLATFORM_VERSION_MAJOR),10)
LOCAL_SHARED_LIBRARIES += libhidltransport
endif

include $(BUILD_STATIC_LIBRARY)
