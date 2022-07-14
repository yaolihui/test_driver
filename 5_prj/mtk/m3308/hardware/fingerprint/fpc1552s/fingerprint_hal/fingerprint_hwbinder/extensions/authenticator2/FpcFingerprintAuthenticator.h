#ifndef COM_FINGERPRINTS_EXTENSION_V1_0_FPCFINGERPRINTAUTHENTICATOR_H
#define COM_FINGERPRINTS_EXTENSION_V1_0_FPCFINGERPRINTAUTHENTICATOR_H

#include <com/fingerprints/extension/1.0/IFpcFingerprintAuthenticator.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include "fpc_hal_ext_authenticator_2_service.h"

namespace com
{
namespace fingerprints
{
namespace extension
{
namespace V1_0
{
namespace implementation
{

using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::com::fingerprints::extension::V1_0::IFpcFingerprintAuthenticator;
using ::com::fingerprints::extension::V1_0::IAuthenticatorCallback;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct FpcFingerprintAuthenticator : public IFpcFingerprintAuthenticator {
    // Methods from ::com::fingerprints::extension::V1_0::IFingerprintAuthenticator follow.
    Return<void> setNotify(const sp<IAuthenticatorCallback> &callback) override;
    Return<void> enroll() override;
    Return<void> authenticate() override;
    Return<void> enumerate() override;
    Return<void> remove(uint32_t id) override;
    Return<void> cancel() override;

    static void instantiate(fpc_authenticator_2_t *device);

private:
    FpcFingerprintAuthenticator(fpc_authenticator_2_t *device);

    static void onEnrollResult(void *context, uint32_t id, uint32_t remaining);
    static void onAuthenticated(void *context, uint32_t id);
    static void onEnumerate(void *context, uint32_t id, uint32_t remaining);
    static void onRemoved(void *context, uint32_t id, uint32_t remaining);
    static void onAcquired(void *context, int32_t code);
    static void onError(void *context, int32_t code);

    static FpcFingerprintAuthenticator *sInstance;
    sp<IAuthenticatorCallback> mCallback;
    fpc_authenticator_compat_callback_t mCompatCallback;
    fpc_authenticator_2_t *mDevice;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com

#endif  // COM_FINGERPRINTS_EXTENSION_V1_0_FPCFINGERPRINTAUTHENTICATOR_H
