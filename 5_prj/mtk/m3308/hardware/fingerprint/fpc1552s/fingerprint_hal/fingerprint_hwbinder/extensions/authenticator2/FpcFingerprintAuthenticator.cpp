/**
 * Copyright (c) 2021 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <utils/Log.h>

#include "FpcFingerprintAuthenticator.h"

void add_authenticator_service_2(fpc_authenticator_2_t* device) {
    ALOGE("fpc fingerprint %s", __func__);
    com::fingerprints::extension::V1_0::implementation::FpcFingerprintAuthenticator::instantiate(
            device);
}

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

FpcFingerprintAuthenticator* FpcFingerprintAuthenticator::sInstance = NULL;

void FpcFingerprintAuthenticator::instantiate(fpc_authenticator_2_t* device) {
    if (sInstance == NULL) {
        sInstance = new FpcFingerprintAuthenticator(device);
        if (sInstance->registerAsService() != android::OK) {
            ALOGE("Failed to register FpcFingerprintAuthenticator");
        }
    }
}

FpcFingerprintAuthenticator::FpcFingerprintAuthenticator(fpc_authenticator_2_t* device)
        : mCallback(NULL),
          mDevice(device) {
    mCompatCallback.on_enroll_result = onEnrollResult;
    mCompatCallback.on_authenticated = onAuthenticated;
    mCompatCallback.on_enumerate = onEnumerate;
    mCompatCallback.on_removed = onRemoved;
    mCompatCallback.on_acquired = onAcquired;
    mCompatCallback.on_error = onError;

    if (mDevice) {
        mDevice->init(mDevice, &mCompatCallback, this);
    } else {
        ALOGE("%s: device not yet registered", __func__);
    }
}

void FpcFingerprintAuthenticator::onEnrollResult(void *context, uint32_t id, uint32_t remaining) {
    FpcFingerprintAuthenticator * self = static_cast<FpcFingerprintAuthenticator*>(context);
    if (self->mCallback == NULL) {
        ALOGE("%s: callback not yet registered", __func__);
        return;
    }

    if (!self->mCallback->onEnrollResult(id, remaining).isOk()) {
        ALOGE("%s: call onEnrollResult failed", __func__);
    }
}

void FpcFingerprintAuthenticator::onAuthenticated(void *context, uint32_t id) {
    FpcFingerprintAuthenticator * self = static_cast<FpcFingerprintAuthenticator*>(context);
    if (self->mCallback == NULL) {
        ALOGE("%s: callback not yet registered", __func__);
        return;
    }

    if (!self->mCallback->onAuthenticated(id).isOk()) {
        ALOGE("%s: call onAuthenticated failed", __func__);
    }
}

void FpcFingerprintAuthenticator::onEnumerate(void *context, uint32_t id, uint32_t remaining) {
    FpcFingerprintAuthenticator * self = static_cast<FpcFingerprintAuthenticator*>(context);
    if (self->mCallback == NULL) {
        ALOGE("%s: callback not yet registered", __func__);
        return;
    }

    if (!self->mCallback->onEnumerate(id, remaining).isOk()) {
        ALOGE("%s: call onEnumerate failed", __func__);
    }
}

void FpcFingerprintAuthenticator::onRemoved(void *context, uint32_t id, uint32_t remaining) {
    FpcFingerprintAuthenticator * self = static_cast<FpcFingerprintAuthenticator*>(context);
    if (self->mCallback == NULL) {
        ALOGE("%s: callback not yet registered", __func__);
        return;
    }

    if (!self->mCallback->onRemoved(id, remaining).isOk()) {
        ALOGE("%s: call onRemoved failed", __func__);
    }
}

void FpcFingerprintAuthenticator::onAcquired(void* context, int32_t code) {
    FpcFingerprintAuthenticator *self = static_cast<FpcFingerprintAuthenticator*>(context);
    if (self->mCallback == NULL) {
        ALOGE("%s: callback not yet registered", __func__);
        return;
    }

    if (!self->mCallback->onAcquired(code).isOk()) {
        ALOGE("%s: call onAcquired failed", __func__);
    }
}

void FpcFingerprintAuthenticator::onError(void* context, int32_t code) {
    FpcFingerprintAuthenticator *self = static_cast<FpcFingerprintAuthenticator*>(context);
    if (self->mCallback == NULL) {
        ALOGE("%s: callback not yet registered", __func__);
        return;
    }

    if (!self->mCallback->onError(code).isOk()) {
        ALOGE("%s: call onError failed", __func__);
    }
}

Return<void> FpcFingerprintAuthenticator::setNotify(const sp<IAuthenticatorCallback>& callback)
{
    mCallback = callback;
    return Void();
}

Return<void> FpcFingerprintAuthenticator::enroll() {
    if (mDevice) {
        mDevice->enroll(mDevice);
    } else {
        ALOGE("%s: device not yet registered", __func__);
    }
    return Void();
}

Return<void> FpcFingerprintAuthenticator::authenticate() {
    if (mDevice) {
        mDevice->authenticate(mDevice);
    } else {
        ALOGE("%s: device not yet registered", __func__);
    }
    return Void();
}

Return<void> FpcFingerprintAuthenticator::enumerate() {
    if (mDevice) {
        mDevice->enumerate(mDevice);
    } else {
        ALOGE("%s: device not yet registered", __func__);
    }
    return Void();
}

Return<void> FpcFingerprintAuthenticator:: remove(uint32_t id) {
    if (mDevice) {
        mDevice->remove(mDevice, id);
    } else {
        ALOGE("%s: device not yet registered", __func__);
    }
    return Void();
}

Return<void> FpcFingerprintAuthenticator::cancel() {
    if (mDevice) {
        mDevice->cancel(mDevice);
    } else {
        ALOGE("%s: device not yet registered", __func__);
    }
    return Void();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com
