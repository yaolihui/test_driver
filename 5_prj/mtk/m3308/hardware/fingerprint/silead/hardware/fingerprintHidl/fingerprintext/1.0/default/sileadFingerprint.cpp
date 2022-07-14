#define LOG_TAG "vendor.silead.hardware.fingerprintext@1.0-service"
#define LOG_VERBOSE "vendor.silead.hardware.fingerprintext@1.0-service"

#include "sileadFingerprint.h"
#include "sileadCommand.h"

#include <inttypes.h>
#include <unistd.h>

namespace vendor
{
namespace silead
{
namespace hardware
{
namespace fingerprintext
{
namespace V1_0
{
namespace implementation
{

sileadFingerprint *sileadFingerprint::sInstance = nullptr;

sileadFingerprint::sileadFingerprint() : mClientCallback(nullptr)
{
    sInstance = this;
    silext_command_init();
    set_notify(sileadFingerprint::notify);
}

sileadFingerprint::~sileadFingerprint()
{
    ALOGV("~BiometricsFingerprint()");
    silext_command_deinit();
}

ISileadFingerprint* sileadFingerprint::getInstance()
{
    if (!sInstance) {
        sInstance = new sileadFingerprint();
    }
    return sInstance;
}

Return<int32_t> sileadFingerprint::requestCmd(uint32_t cmdId, const hidl_vec<uint8_t>& param)
{
    return silext_command_request(cmdId, param.data(), param.size());
}

Return<int32_t> sileadFingerprint::setNotify(const sp<ISileadFingerprintCallback>& clientCallback)
{
    mClientCallback = clientCallback;
    return 0;
}

void sileadFingerprint::notify(uint32_t cmd_id, const uint8_t *result, uint32_t len)
{
    sileadFingerprint* thisPtr = static_cast<sileadFingerprint*>(sileadFingerprint::getInstance());
    if (thisPtr == nullptr || thisPtr->mClientCallback == nullptr) {
        ALOGE("Receiving callbacks before the silead client callback is registered.");
        return;
    }

    hidl_vec<uint8_t> data;
    data.setToExternal((unsigned char*)(result), len);
    thisPtr->mClientCallback->onCmdResponse(cmd_id, data);
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace fingerprintext
}  // namespace hardware
}  // namespace silead
}  // namespace vendor
