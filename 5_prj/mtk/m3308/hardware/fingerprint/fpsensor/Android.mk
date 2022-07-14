PACKAGE_PATH := $(call my-dir)

$(shell mkdir -p out/target/product/$(MTK_TARGET_PROJECT)/vendor/app/mcRegistry)
$(shell mkdir -p out/target/product/$(MTK_TARGET_PROJECT)/vendor/etc/init)
$(shell mkdir -p out/target/product/$(MTK_TARGET_PROJECT)/vendor/etc/vintf/manifest)
$(shell mkdir -p out/target/product/$(MTK_TARGET_PROJECT)/vendor/etc/permissions)

$(shell cp -f  $(PACKAGE_PATH)/ta_lib/lib32/05220000000000000000000000000000.tlbin	out/target/product/$(MTK_TARGET_PROJECT)/vendor/app/mcRegistry/05220000000000000000000000000000.tlbin)
$(shell cp -f  $(PACKAGE_PATH)/init.fpsensor.rc										out/target/product/$(MTK_TARGET_PROJECT)/vendor/etc/init/init.fpsensor.rc)
$(shell cp -f  frameworks/native/data/etc/android.hardware.fingerprint.xml			out/target/product/$(MTK_TARGET_PROJECT)/vendor/etc/permissions/android.hardware.fingerprint.xml)
