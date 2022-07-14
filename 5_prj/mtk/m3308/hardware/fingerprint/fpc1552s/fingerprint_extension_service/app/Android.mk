#
# Copyright (c) 2016-2020 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_OWNER := fpc

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := FingerprintExtensionService

PLATFORM_VERSION_MAJOR := $(word 1, $(subst ., ,$(PLATFORM_VERSION)))

ifeq ($(PLATFORM_VERSION_MAJOR),6)
FPC_SYSTEM_DIR := true
endif

ifeq ($(PLATFORM_VERSION_MAJOR),7)
FPC_SYSTEM_DIR := true
endif

ifeq ($(FPC_SYSTEM_DIR),)
# In Android O and later we should place the library in /vendor/app insteadof /system/app/
LOCAL_PROPRIETARY_MODULE := true
endif

LOCAL_CERTIFICATE := platform

LOCAL_JAVA_LIBRARIES := \
    com.fingerprints.extension

LOCAL_DEX_PREOPT := false

# Only for FPC internal
ifeq ($(TARGET_PRODUCT),hikey960)
ifeq ($(PLATFORM_VERSION_MAJOR),9)
LOCAL_PRIVATE_PLATFORM_APIS := true
endif

ifeq ($(PLATFORM_VERSION_MAJOR),10)
LOCAL_PRIVATE_PLATFORM_APIS := true
endif
endif

# Only for FPC internal
ifeq ($(TARGET_PRODUCT),aosp_walleye)
LOCAL_PRIVATE_PLATFORM_APIS := true
endif

# Only for FPC internal
ifeq ($(TARGET_PRODUCT),aosp_taimen)
LOCAL_PRIVATE_PLATFORM_APIS := true
endif

# Only for FPC internal
ifeq ($(TARGET_PRODUCT),qemu_trusty_arm64)
LOCAL_PRIVATE_PLATFORM_APIS := true
endif

include $(BUILD_PACKAGE)
