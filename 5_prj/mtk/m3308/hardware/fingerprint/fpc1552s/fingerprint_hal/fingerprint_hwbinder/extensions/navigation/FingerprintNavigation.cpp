#include "FingerprintNavigation.h"
#include "fpc_hal_ext_navigation_service.h"

#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>
#include <utils/StrongPointer.h>

void add_navigation_service(fpc_navigation_t* device) {
    com::fingerprints::extension::V1_0::implementation::FingerprintNavigation::instantiate(device);
}

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

FingerprintNavigation* FingerprintNavigation::sInstance = NULL;

void FingerprintNavigation::instantiate(fpc_navigation_t* device) {
    if (sInstance == NULL) {
        sInstance = new FingerprintNavigation(device);
        if (sInstance->registerAsService() != android::OK) {
            ALOGE("Failed to register FingerprintNavigation");
        }
    } else if (device == NULL) {
        ALOGE("FingerprintNavigation::instantiate: clear mDevice");
        sInstance->mDevice = NULL;
    }
}

FingerprintNavigation::FingerprintNavigation(fpc_navigation_t* device)
        : mDevice(device) {
}

FingerprintNavigation::~FingerprintNavigation() {
}

// Methods from ::com::fingerprints::extension::V1_0::IFingerprintNavigation follow.
Return<void> FingerprintNavigation::setNavigation(bool enabled) {
    if (mDevice) {
        mDevice->set_enabled(mDevice, enabled);
    }
    return Void();
}

Return<void> FingerprintNavigation::getNavigationConfig(getNavigationConfig_cb _hidl_cb) {
    if (mDevice) {
        fpc_nav_config_t config;
        mDevice->get_config(mDevice, &config);
        NavigationConfig navigationConfig;
        navigationConfig.tapNoImageMaxThreshold = config.threshold_tap_n_images_max;
        navigationConfig.holdNoImageMinThreshold = config.threshold_hold_n_images_min;
        navigationConfig.doubleClickTimeInterval = config.double_click_time_interval;
        navigationConfig.tapImageTransMaxThreshold = config.threshold_tap_translation_max;
        navigationConfig.swipeImageTransMinThreshold = config.threshold_swipe_translation_min;
        navigationConfig.backGroundAlgo = config.mode;
        _hidl_cb(navigationConfig);
    }
    return Void();
}

Return<void> FingerprintNavigation::setNavigationConfig(const NavigationConfig& navigationConfig) {
    if (mDevice) {
        fpc_nav_config_t config;
        config.threshold_tap_n_images_max = navigationConfig.tapNoImageMaxThreshold;
        config.threshold_hold_n_images_min = navigationConfig.holdNoImageMinThreshold;
        config.double_click_time_interval = navigationConfig.doubleClickTimeInterval;
        config.threshold_tap_translation_max = navigationConfig.tapImageTransMaxThreshold;
        config.threshold_swipe_translation_min = navigationConfig.swipeImageTransMinThreshold;
        config.mode = navigationConfig.backGroundAlgo;

        mDevice->set_config(mDevice, &config);
    }
    return Void();
}

Return<bool> FingerprintNavigation::isEnabled() {
    bool enabled = false;
    if (mDevice) {
        enabled = mDevice->get_enabled(mDevice);
    }
    return enabled;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com
