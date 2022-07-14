#
# Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.
#
# =============================================================================
# Tbase specific includes for the TAC
# Note that LOCAL_PATH is set to the location of the normal/Android.mk that
# includes this file
# =============================================================================
LOCAL_PATH_PLATFORM    := ../normal/platform/tbase
LOCAL_PATH_PLATFORM_IF := ../interface/platform/tbase

# =============================================================================
LOCAL_C_INCLUDES += \
                $(LOCAL_PATH)/$(LOCAL_PATH_PLATFORM)/inc \
                $(LOCAL_PATH)/$(LOCAL_PATH_PLATFORM_IF) \
                $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_SRC_FILES += \
                $(LOCAL_PATH_PLATFORM)/src/fpc_tbase_tac.c \
                $(LOCAL_PATH_PLATFORM)/src/hw_auth/fpc_tee_hw_auth_tbase.c \

# Path to the Kernel REE device driver sysfs interface
LOCAL_CFLAGS += -DFPC_REE_DEVICE_ALIAS_FILE='"modalias"'
LOCAL_CFLAGS += -DFPC_REE_DEVICE_NAME='"fingerprint"'
ifeq ($(PRODUCT_DEVICE), tank)
	LOCAL_CFLAGS += -DFPC_REE_DEVICE_PATH='"/sys/devices/platform/1100a000.spi0/spi_master/spi0"'
else 
	LOCAL_CFLAGS += -DFPC_REE_DEVICE_PATH='"/sys/devices/platform/1100a000.spi/spi_master/spi0"'
endif

LOCAL_CFLAGS += -DTBASE

LOCAL_SHARED_LIBRARIES += libMcClient

# LOCAL_EXPORT_C_INCLUDE_DIRS += \

# setup external deps
ifndef ANDROID_SYSTEM_ROOT
#If not defined use default folder used in main build git
ANDROID_SYSTEM_ROOT := ~/project/m230_drv
endif
vendor_path := vendor/mediatek/proprietary/trustzone/trustonic/source/external/mobicore/common/400b
t_base_dev_kit=$(ANDROID_SYSTEM_ROOT)/$(vendor_path)
#relative path needed for COMP_PATH_MobiCoreDriverLib (multi-OS compatibility for including library)
COMP_PATH_MobiCore=$(t_base_dev_kit)/ClientLib
TBASE_API_LEVEL := 5

