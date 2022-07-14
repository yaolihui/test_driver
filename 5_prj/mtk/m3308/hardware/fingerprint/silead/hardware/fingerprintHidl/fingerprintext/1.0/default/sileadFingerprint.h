#ifndef VENDOE_SILEAD_HARDWARE_FINGERPRINTEXT_V1_0_SILEADFINGERPRINT_H
#define VENDOE_SILEAD_HARDWARE_FINGERPRINTEXT_V1_0_SILEADFINGERPRINT_H

#include <log/log.h>
#include <android/log.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <vendor/silead/hardware/fingerprintext/1.0/ISileadFingerprint.h>

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

using ::vendor::silead::hardware::fingerprintext::V1_0::ISileadFingerprint;
using ::vendor::silead::hardware::fingerprintext::V1_0::ISileadFingerprintCallback;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_vec;
using ::android::sp;

struct sileadFingerprint : public ISileadFingerprint {
public:
    sileadFingerprint();
    ~sileadFingerprint();

    static ISileadFingerprint* getInstance();

    Return<int32_t> requestCmd(uint32_t cmdId, const hidl_vec<uint8_t>& param) override;
    Return<int32_t> setNotify(const sp<ISileadFingerprintCallback>& clientCallback) override;

private:
    static void notify(uint32_t cmd_id, const uint8_t *result, uint32_t len);

    static sileadFingerprint* sInstance;

    sp<ISileadFingerprintCallback> mClientCallback;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace fingerprintext
}  // namespace hardware
}  // namespace silead
}  // namespace vendor

#endif  // VENDOE_SILEAD_HARDWARE_FINGERPRINTEXT_V1_0_SILEADFINGERPRINT_H
