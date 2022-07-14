/**
 * Copyright (c) 2019 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <utils/Log.h>
#include <time.h>

#include "FingerprintEngineering.h"
#include "fpc_hal_ext_engineering_service.h"
#include "fpc_hal_ext_engineering.h"

#ifndef MIN
#define MIN(x, y) ((x > y) ? y : x)
#endif

/* Set transfer between C++ and Java to 1024*50 */
#define HIDL_MAX_TRANSFER (1024 * 50)
#define TRANSFER_DELAY_US (100 * 1000)
void add_engineering_service(fpc_engineering_t* device) {
    com::fingerprints::extension::V1_0::implementation::FingerprintEngineering::instantiate(device);
}

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

FingerprintEngineering* FingerprintEngineering::sInstance = NULL;
fpc_hal_img_data_t * FingerprintEngineering::sImageInjectData = NULL;

void FingerprintEngineering::instantiate(fpc_engineering_t* device) {
    if (sInstance == NULL) {
        sInstance = new FingerprintEngineering(device);
        if (sInstance->registerAsService() != android::OK) {
            ALOGE("Failed to register FingerprintEngineering");
        }
    }
}

FingerprintEngineering::FingerprintEngineering(fpc_engineering_t* device)
        : mImageSubscriptionCallback(NULL),
          mImageInjectionCallback(NULL),
          mCaptureCallback(NULL),
          mDevice(device),
          mIsCapture(false) {
}

FingerprintEngineering::~FingerprintEngineering() {
}

void FingerprintEngineering::sendCaptureCallback(
        fpc_capture_data_t* captureData,
        sp<IImageCaptureCallback> imageCaptureCallback)
{
    ImageCaptureData imageCaptureData = {};

    imageCaptureData.mode = captureData->mode;
    imageCaptureData.captureResult = captureData->capture_result;
    imageCaptureData.identifyResult = captureData->identify_result;
    imageCaptureData.templateUpdateResult = captureData->template_update_result;
    imageCaptureData.enrollResult = captureData->enroll_result;
    imageCaptureData.cacResult = captureData->cac_result;
    imageCaptureData.userId = captureData->user_id;
    imageCaptureData.remainingSamples = captureData->samples_remaining;
    imageCaptureData.coverage = captureData->coverage;
    imageCaptureData.quality = captureData->quality;

    if (!imageCaptureCallback->onImage(imageCaptureData).isOk()) {
        ALOGE("%s onImage() callback failed", __func__);
    }
    ALOGD("%s: onImage() callback sucess", __func__);

    /* Raw image buffer */
    uint32_t size;
    uint32_t offset = 0;
    uint8_t *src_buffer = captureData->raw_image.buffer;
    uint32_t remaining = captureData->raw_image.buffer_size;
    uint8_t type = 0; /* 0 is raw, 1 is enhanced */

    ALOGD("%s: Transfering RAW: src_buffer: %p", __func__, src_buffer);

    do {
        hidl_vec<uint8_t> shared_buffer;

        size = MIN(remaining, HIDL_MAX_TRANSFER);
        remaining -= size;

        shared_buffer.setToExternal(src_buffer + offset, size);

        if (!imageCaptureCallback->onImageTransferData(type, shared_buffer).isOk()) {
            ALOGE("%s: onImageTransferData() callback failed: type: %d", __func__, type);
            break;
        }
        ALOGD("%s: onImageTransferData() callback sucess: %d, %d, %d, %d", __func__, type, offset, size, remaining);
        offset += size;
    } while (remaining);

    /* Enhanced image buffer */
    offset = 0;
    type = 1; /* 0 is raw, 1 is enhanced */
    src_buffer = captureData->enhanced_image.buffer;
    remaining = captureData->enhanced_image.buffer_size;

    ALOGD("%s: Transfering Enhanced: src_buffer: %p", __func__, src_buffer);

    do {
        hidl_vec<uint8_t> shared_buffer;

        size = MIN(remaining, HIDL_MAX_TRANSFER);
        remaining -= size;
        shared_buffer.setToExternal(src_buffer + offset, size);

        if (!imageCaptureCallback->onImageTransferData(type, shared_buffer).isOk()) {
            ALOGE("%s: onImageTransferData() callback failed: type: %d", __func__, type);
            break;
        }
        ALOGD("%s: onImageTransferData() callback sucess: %d, %d, %d, %d", __func__, type, offset, size, remaining);
        offset += size;
    } while (remaining);

    if (!imageCaptureCallback->onImageFinish().isOk()) {
        ALOGE("%s Sending callback failed", __func__);
    }
    ALOGD("%s: onImageFinish() callback sucess", __func__);
}

void FingerprintEngineering::onImage(void* context, fpc_capture_data_t* captureData) {
    FingerprintEngineering * self = static_cast<FingerprintEngineering*>(context);
    if (self->mImageSubscriptionCallback != NULL) {
        sendCaptureCallback(captureData, self->mImageSubscriptionCallback);
        usleep(50 * 1000);
    }
}

void FingerprintEngineering::onCapture(void* context,    fpc_capture_data_t *captureData) {
    FingerprintEngineering * self = static_cast<FingerprintEngineering*>(context);
    sp<IImageCaptureCallback> captureCallback = self->mCaptureCallback;
    if (!captureData->samples_remaining) {
        self->mCaptureCallback = NULL;
    }
    if (captureCallback != NULL) {
        if (captureData->samples_remaining == 0) {
            self->mIsCapture = false;
        }
        sendCaptureCallback(captureData, captureCallback);
    }
}

void FingerprintEngineering::onInjectCallback(const hidl_vec<uint8_t>& imageData) {
    /*
     * This callback is called during the call to self->mImageInjectionCallback->onInject()
     * in FingerprintEngineering::onInject() below.
     */
    if (sImageInjectData != NULL) {
        if (imageData.size() > 0) {
            /*
             * It is the responsibility of the caller of FingerprintEngineering::onInject()
             * to free this buffer.
             */
            sImageInjectData->buffer = (uint8_t*) calloc(1, imageData.size());
            if (sImageInjectData->buffer != NULL) {
                memcpy(sImageInjectData->buffer, imageData.data(), imageData.size());
                sImageInjectData->buffer_size = imageData.size();
            } else {
                ALOGE("Memory allocation failed");
                sImageInjectData->buffer_size = 0;
            }
        } else {
            ALOGD("No image data received");
            sImageInjectData->buffer_size = 0;
            sImageInjectData->buffer = NULL;
        }
    } else {
        ALOGE("sImageInjectData is NULL.");
    }
}

int FingerprintEngineering::onInject(void* context, fpc_hal_img_data_t* imgData) {

    FingerprintEngineering * self = static_cast<FingerprintEngineering*>(context);
    if (self->mImageInjectionCallback != NULL) {
        sImageInjectData = imgData;
        if (!self->mImageInjectionCallback->onInject(FingerprintEngineering::onInjectCallback).isOk()) {
            ALOGE("%s Sending callback failed", __func__);
        }
    }
    return 0;
}

void FingerprintEngineering::onCancel(void* context) {
    FingerprintEngineering * self =
            static_cast<FingerprintEngineering*>(context);
    if (self->mImageInjectionCallback != NULL) {
        if (!self->mImageInjectionCallback->onCancel().isOk()) {
            ALOGE("%s Sending callback failed", __func__);
        }
    }
}

void FingerprintEngineering::serviceDied(uint64_t cookie, const wp<IBase>& who) {
    (void)cookie;
    if (who == mImageSubscriptionCallback) {
        stopImageSubscription();
    } else if (who == mImageInjectionCallback) {
        stopImageInjection();
    } else if (who == mCaptureCallback) {
        cancelCapture();
    }
}

// Methods from ::com::fingerprints::extension::V1_0::IFingerprintEngineering follow.
Return<void> FingerprintEngineering::getSensorSize(getSensorSize_cb _hidl_cb) {
    SensorSize sensorSize;
    memset(&sensorSize, 0, sizeof(sensorSize));
    if (mDevice) {
        uint16_t width = 0;
        uint16_t height = 0;
        mDevice->get_sensor_size(mDevice, &width, &height);
        sensorSize.width = width;
        sensorSize.height = height;
    }
    _hidl_cb(sensorSize);
    return Void();
}

Return<void> FingerprintEngineering::startImageSubscription(
        const sp<IImageCaptureCallback>& callback) {
    if (mImageSubscriptionCallback != NULL) {
        mImageSubscriptionCallback->unlinkToDeath(this);
    }
    mImageSubscriptionCallback = callback;
    if (mImageSubscriptionCallback != NULL) {
        mImageSubscriptionCallback->linkToDeath(this, 0);
    }
    if (mDevice) {
        mDevice->set_img_subscr_cb(mDevice, FingerprintEngineering::onImage, this);
    }
    return Void();
}

Return<void> FingerprintEngineering::stopImageSubscription() {
    if (mImageSubscriptionCallback != NULL) {
        mImageSubscriptionCallback->unlinkToDeath(this);
        mImageSubscriptionCallback = NULL;
    }
    if (mDevice) {
        mDevice->set_img_subscr_cb(mDevice, NULL, NULL);
    }
    return Void();
}

Return<void> FingerprintEngineering::startImageInjection(
        const sp<IImageInjectionCallback>& callback) {
    if (mImageInjectionCallback != NULL) {
        mImageInjectionCallback->unlinkToDeath(this);
    }
    mImageInjectionCallback = callback;
    if (mImageInjectionCallback != NULL) {
        mImageInjectionCallback->linkToDeath(this, 0);
    }
    if (mDevice) {
        mDevice->set_img_inj_cb(mDevice, FingerprintEngineering::onInject,
                                FingerprintEngineering::onCancel, this);
    }
    return Void();
}

Return<void> FingerprintEngineering::stopImageInjection() {
    if (mImageInjectionCallback != NULL) {
        mImageInjectionCallback->unlinkToDeath(this);
        mImageInjectionCallback = NULL;
    }

    if (mDevice) {
        mDevice->set_img_inj_cb(mDevice, NULL, NULL, NULL);
    }
    return Void();
}

Return<void> FingerprintEngineering::startCapture(const sp<IImageCaptureCallback>& callback,
                                                  uint32_t mode) {
    if (mCaptureCallback != NULL) {
        mCaptureCallback->unlinkToDeath(this);
    }
    mCaptureCallback = callback;
    if (mCaptureCallback != NULL) {
        mCaptureCallback->linkToDeath(this, 0);
    }

    if (mDevice) {
        mIsCapture = true;
        mDevice->start_capture(mDevice, FingerprintEngineering::onCapture,
                               (fpc_capture_mode_t)mode, this);
    }
    return Void();
}

Return<void> FingerprintEngineering::cancelCapture() {
    if (mDevice && mIsCapture) {
        mDevice->cancel_capture(mDevice);
        mIsCapture = false;
    }

    if (mCaptureCallback != NULL) {
        mCaptureCallback->unlinkToDeath(this);
        mCaptureCallback = NULL;
    }
    return Void();
}

Return<void> FingerprintEngineering::setEnrollToken(const hidl_vec<uint8_t>& token) {
    if (mDevice) {
        mDevice->set_enroll_token(mDevice, token.data(), token.size());
    }
    return Void();
}

Return<uint64_t> FingerprintEngineering::getEnrollChallenge() {
    uint64_t challenge = 0;
    if (mDevice) {
        mDevice->get_enroll_challenge(mDevice, &challenge);
    }
    return challenge;
}

Return<void> FingerprintEngineering::getBuildInfo(getBuildInfo_cb _hidl_cb) {
    hidl_string  build_info = "";
    if (mDevice) {
        mDevice->get_build_info(mDevice, [](const char * build_info, void *usr) {
            *static_cast<hidl_string *>(usr) = build_info;
        }, &build_info);
    }
    _hidl_cb(build_info);
    return Void();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com
