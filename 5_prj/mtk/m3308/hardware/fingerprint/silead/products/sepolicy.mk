SL_FP_ROOT := vendor/mediatek/proprietary/hardware/fingerprint/silead

include $(SL_FP_ROOT)/products/sileadConfig.mk

ifeq ($(strip $(SILEAD_FP_SUPPORT)),yes)
    BOARD_SEPOLICY_DIRS += $(SL_FP_ROOT)/sepolicy/android8/vendor/common

    ifeq ($(strip $(SILEAD_FP_TEE_TYPE)),qsee)
        BOARD_SEPOLICY_DIRS += $(SL_FP_ROOT)/sepolicy/android8/vendor/qsee
    else ifeq ($(strip $(SILEAD_FP_TEE_TYPE)),tee)
        BOARD_SEPOLICY_DIRS += $(SL_FP_ROOT)/sepolicy/android8/vendor/tee
    else ifeq ($(strip $(SILEAD_FP_TEE_TYPE)),gp)
        BOARD_SEPOLICY_DIRS += $(SL_FP_ROOT)/sepolicy/android8/vendor/tee
    else ifeq ($(strip $(SILEAD_FP_TEE_TYPE)),trusty)
        BOARD_SEPOLICY_DIRS += $(SL_FP_ROOT)/sepolicy/android8/vendor/trusty
    endif

    ifeq ($(strip $(SILEAD_FP_TEST_SUPPORT)),yes)
      ifeq ($(strip $(shell expr $(PLATFORM_VERSION) \>= 12)),1)
        BOARD_SEPOLICY_DIRS += $(SL_FP_ROOT)/sepolicy/android8/public
       ifneq ($(strip $(MTK_PLATFORM)),)
        BOARD_SEPOLICY_DIRS += $(SL_FP_ROOT)/sepolicy/android8/private_mtk
       else
        BOARD_SEPOLICY_DIRS += $(SL_FP_ROOT)/sepolicy/android8/private
       endif
      else
        BOARD_SEPOLICY_DIRS += $(SL_FP_ROOT)/sepolicy/android8/public
        BOARD_SEPOLICY_DIRS += $(SL_FP_ROOT)/sepolicy/android8/private
      endif
    endif
endif
