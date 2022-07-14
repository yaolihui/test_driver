LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

include $(LOCAL_PATH)/device/device.mk

include $(LOCAL_PATH)/fingerprint_tac/normal/Android.mk \
	$(LOCAL_PATH)/fingerprint_hal/Android.mk \
	$(LOCAL_PATH)/fingerprint_extension/Android.mk \
	$(LOCAL_PATH)/fingerprint_extension_service/Android.mk  \
	$(LOCAL_PATH)/fingerprint_libs/fmi/Android.mk \
	$(LOCAL_PATH)/fingerprint_apps/sensor_test_tool/Android.mk \
	$(LOCAL_PATH)/fingerprint_apps/image_collection/Android.mk 
	# $(LOCAL_PATH)/fingerprint_apps/image_tool/Android.mk \
	$(LOCAL_PATH)/fingerprint_apps/image_collection/Android.mk
