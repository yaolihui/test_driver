#define LOG_TAG "vendor.silead.hardware.fingerprintext@1.0-service"

#include <android/log.h>
#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>
#include <vendor/silead/hardware/fingerprintext/1.0/ISileadFingerprint.h>
#include "sileadFingerprint.h"

using vendor::silead::hardware::fingerprintext::V1_0::ISileadFingerprint;
using vendor::silead::hardware::fingerprintext::V1_0::implementation::sileadFingerprint;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::sp;

int main()
{
    android::sp<ISileadFingerprint> fpext = sileadFingerprint::getInstance();

    configureRpcThreadpool(1, true /*callerWillJoin*/);

    if (fpext != nullptr) {
        if (::android::OK != fpext->registerAsService()) {
            return 1;
        }
    } else {
        ALOGE("Can't create instance of sileadFingerprint, nullptr");
    }

    joinRpcThreadpool();

    return 0; // should never get here
}
