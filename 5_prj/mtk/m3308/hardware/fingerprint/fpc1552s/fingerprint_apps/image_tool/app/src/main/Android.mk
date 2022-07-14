#
# Copyright (c) 2015-2020 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, java)

LOCAL_PACKAGE_NAME := ImageTool

LOCAL_CERTIFICATE := platform

LOCAL_MODULE_OWNER := fpc

PLATFORM_VERSION_MAJOR := $(word 1, $(subst ., ,$(PLATFORM_VERSION)))

ifeq ($(PLATFORM_VERSION_MAJOR),6)
FPC_SYSTEM_DIR := true
endif

ifeq ($(PLATFORM_VERSION_MAJOR),7)
FPC_SYSTEM_DIR := true
endif

ifeq ($(FPC_SYSTEM_DIR),)
# Place in /vendor/app in Android O
# since we strive towards only placing things
# on the /vendor partition.
LOCAL_PROPRIETARY_MODULE := true
else
# Place it in the /system/priv-app folder in pre Android O
# If placed in /system/priv-app in Android O we would also
# need a priv-app permission xml file in /etc/permissions/,
# see https://source.android.com/devices/tech/config/perms-whitelist
LOCAL_PRIVILEGED_MODULE := true
endif

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
