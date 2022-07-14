#
# Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := com.fingerprints.extension
LOCAL_MODULE_OWNER := fpc

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_DEX_PREOPT := false

#For debug build we use static for debug purposes in Android Studio

ifeq ($(PLATFORM_VERSION_MAJOR),8)
LOCAL_STATIC_JAVA_LIBRARIES := com.fingerprints.extension-V1.0-java-static
else
LOCAL_STATIC_JAVA_LIBRARIES := com.fingerprints.extension-V1.0-java
endif

LOCAL_STATIC_JAVA_LIBRARIES += android.hidl.base-V1.0-java
LOCAL_STATIC_JAVA_LIBRARIES += android-support-annotations

LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_JAVA_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := com.fingerprints.extension.xml

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_CLASS := ETC

LOCAL_MODULE_OWNER := fpc

ifeq ($(PLATFORM_VERSION_MAJOR),9)
LOCAL_PROPRIETARY_MODULE := true
endif

# This will install the file in /vendor/etc/permissions

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_ETC)/permissions

LOCAL_SRC_FILES := $(LOCAL_MODULE)

include $(BUILD_PREBUILT)
