#!/bin/bash

source system/tools/hidl/update-makefiles-helper.sh

do_makefiles_update \
  "vendor.silead.hardware:vendor/mediatek/proprietary/hardware/fingerprint/silead/hardware/fingerprintHidl" \
  "android.hidl:system/libhidl/transport"

