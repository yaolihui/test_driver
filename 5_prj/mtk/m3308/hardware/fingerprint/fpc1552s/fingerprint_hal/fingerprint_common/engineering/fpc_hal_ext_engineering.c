/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <cutils/properties.h>

#include "fpc_hal_ext_engineering.h"
#include "fpc_tee_hal.h"
#include "fpc_log.h"
#include "fpc_types.h"
#include "fpc_tee_bio.h"
#include "fpc_worker.h"
#include "fpc_tee_sensor.h"
#include "fpc_tee_engineering.h"
#include "fpc_tee_hal.h"
#include "fpc_hal_private.h"
#include "fpc_ta_engineering_interface.h"

#define TEST_TEMPLATE_DB "/data/fpc/test.db"

typedef struct {
    fpc_engineering_t engineering;

    pthread_mutex_t mutex;
    void *img_subscr_cb_ctx;
    fpc_img_subscr_cb_t img_subscr_cb;
    void *img_inj_cb_ctx;
    fpc_img_inj_cb_t img_inj_cb;
    bool is_inj_wait_for_img;
    bool is_inj_cancel;
    fpc_img_inj_cancel_cb_t img_inj_cancel_cb;
    uint16_t image_width;
    uint16_t image_height;
    fpc_capture_cb_t capture_cb;
    void *capture_cb_ctx;
    fpc_hal_common_t *hal;
} engineering_module_t;

int fpc_hal_ext_get_raw_image(fpc_engineering_t *self, fpc_hal_img_data_t *image_data)
{
    int status = 0;
    engineering_module_t *module = (engineering_module_t *) self;

    LOG_ENTER();

    status = fpc_tee_debug_get_retrieve_size(module->hal->tee_handle,
                                             ENG_RAW_IMAGE,
                                             &image_data->buffer_size);
    if (status) {
        LOGE("%s: fpc_tee_debug_get_retrieve_size: status: %d", __func__,
             status);
        image_data->buffer_size = 0;
        return status;
    }

    image_data->image_type = RAW;
    image_data->buffer = (uint8_t *) malloc(image_data->buffer_size);
    if (!image_data->buffer) {
        status = -FPC_ERROR_MEMORY;
        LOGE("%s: Failed to allocate memory for subscription data: status: %d", __func__, status);
        goto out;
    }

    status = fpc_tee_debug_retrieve(module->hal->tee_handle,
                                    ENG_RAW_IMAGE,
                                    image_data->buffer,
                                    image_data->buffer_size);
    if (status) {
        LOGE("%s: fpc_tee_debug_retrieve: status: %d", __func__,
             status);
        goto out;
    }

out:
    LOG_LEAVE();
    return status;
}

int fpc_hal_ext_get_enhanced_image(fpc_engineering_t *self, fpc_hal_img_data_t *image_data)
{
    int status = 0;
    engineering_module_t *module = (engineering_module_t *) self;

    LOG_ENTER();

    image_data->buffer_size = module->image_width * module->image_height;
    image_data->buffer = (uint8_t *) malloc(image_data->buffer_size);
    if (!image_data->buffer) {
        status = -FPC_ERROR_MEMORY;
        LOGE("%s: Failed to allocate memory for image data: status %d", __func__, status);
        goto out;
    }

    status = fpc_tee_debug_retrieve(module->hal->tee_handle,
                                    ENG_ENHANCED_IMAGE,
                                    image_data->buffer,
                                    image_data->buffer_size);
    if (status) {
        LOGE("%s: fpc_tee_debug_retrieve: status: %d", __func__, status);
        goto out;
    }

    image_data->image_type = PREPROCESSED;

out:
    LOG_LEAVE();
    return status;
}

void fpc_hal_ext_free_image(fpc_hal_img_data_t *image_data)
{
    LOG_ENTER();

    if (image_data != NULL && image_data->buffer != NULL) {
        free(image_data->buffer);
        image_data->buffer = NULL;
        image_data->buffer_size = 0;
    }
    LOG_LEAVE();
}

static void free_captured_images(fpc_capture_data_t *capture_data)
{
    LOG_ENTER();

    fpc_hal_ext_free_image(&capture_data->raw_image);
    fpc_hal_ext_free_image(&capture_data->enhanced_image);

    LOG_LEAVE();
}

static int handle_subscription_callback(fpc_engineering_t *self, fpc_capture_data_t *data)
{
    engineering_module_t *module = (engineering_module_t *) self;
    int status = 0;

    LOG_ENTER();

    pthread_mutex_lock(&module->mutex);

    status = fpc_hal_ext_get_raw_image(self, &data->raw_image);

    if (status) {
        LOGE("%s: fpc_hal_ext_get_raw_image: status: %d", __func__, status);
        goto out;
    }

    status = fpc_hal_ext_get_enhanced_image(self, &data->enhanced_image);

    if (status) {
        LOGE("%s: fpc_get_enhanced_image: status: %d", __func__, status);
        goto out;
    }

    module->img_subscr_cb(module->img_subscr_cb_ctx, data);

out:

    free_captured_images(data);

    pthread_mutex_unlock(&module->mutex);

    LOG_LEAVE();
    return status;
}

static int handle_image_subscription(fpc_engineering_t *self)
{
    int status = 0;
    engineering_module_t *module = (engineering_module_t *) self;

    LOG_ENTER();

    if (!module->img_subscr_cb) {
        LOGD("%s: No image subscription callback registered\n", __func__);
        goto out;
    }

    fpc_capture_data_t capture_data;
    memset(&capture_data, 0, sizeof(capture_data));
    capture_data.mode = FPC_CAPTURE_MODE_CAPTURE;

    status = handle_subscription_callback(self, &capture_data);

out:
    LOG_LEAVE();
    return status;
}

static int handle_image_subscription_enroll(fpc_engineering_t *self, int capture_result,
                                            int enroll_result, int samples_remaining, uint32_t fid)
{
    int status = 0;
    engineering_module_t *module = (engineering_module_t *) self;

    LOG_ENTER();

    if (!module->img_subscr_cb) {
        LOGD("%s: No image subscription callback registered\n", __func__);
        goto out;
    }

    fpc_capture_data_t capture_data;
    memset(&capture_data, 0, sizeof(capture_data));
    capture_data.mode = FPC_CAPTURE_MODE_ENROLL;
    capture_data.user_id = fid;
    capture_data.capture_result = capture_result;
    capture_data.enroll_result = enroll_result;
    capture_data.samples_remaining = samples_remaining;

    status = handle_subscription_callback(self, &capture_data);

out:
    LOG_LEAVE();
    return status;
}

static int handle_image_subscription_auth(fpc_engineering_t *self, int capture_result,
                                          int identify_result, int32_t coverage, int32_t quality,
                                          uint32_t fid)
{
    int status = 0;
    engineering_module_t *module = (engineering_module_t *) self;

    LOG_ENTER();

    if (!module->img_subscr_cb) {
        LOGD("%s: No image subscription callback registered\n", __func__);
        goto out;
    }

    fpc_capture_data_t capture_data;
    memset(&capture_data, 0, sizeof(capture_data));
    capture_data.mode = FPC_CAPTURE_MODE_VERIFY;
    capture_data.user_id = fid;
    capture_data.coverage = coverage;
    capture_data.quality = quality;
    capture_data.capture_result = capture_result;
    capture_data.identify_result = identify_result;

    status = handle_subscription_callback(self, &capture_data);

out:
    LOG_LEAVE();
    return status;
}

static void get_sensor_size(fpc_engineering_t *self,
                            uint16_t *width, uint16_t *height)
{
    engineering_module_t *module = (engineering_module_t *) self;

    LOG_ENTER();

    pthread_mutex_lock(&module->mutex);

    *width = module->image_width;
    *height = module->image_height;

    pthread_mutex_unlock(&module->mutex);

    LOG_LEAVE();
}

static int set_img_subscr_cb(fpc_engineering_t *self,
                             fpc_img_subscr_cb_t callback, void *ctx)
{
    int status = 0;
    engineering_module_t *module = (engineering_module_t *) self;

    LOG_ENTER();

    pthread_mutex_lock(&module->mutex);

    if (!callback) {
        // NULL pointer callback means we deregister image subscription and ignore ctx parameter
        ctx = NULL;
    }

    module->img_subscr_cb = callback;
    module->img_subscr_cb_ctx = ctx;

    pthread_mutex_unlock(&module->mutex);

    LOG_LEAVE();
    return status;
}

static int save_test_db(engineering_module_t *module)
{
    LOG_ENTER();
    return fpc_tee_store_template_db(module->hal->bio, TEST_TEMPLATE_DB);
}

static int delete_test_db(engineering_module_t *module)
{
    int status;

    LOG_ENTER();

    status = fpc_tee_load_empty_db(module->hal->bio);
    if (status) {
        LOGE("%s: fpc_tee_load_empty_db: status: %d", __func__, status);
        goto exit;
    }

    status = save_test_db(module);
    if (status) {
        LOGE("%s: save_test_db: status: %d", __func__, status);
    }

exit:
    LOG_LEAVE();
    return status;
}

static int load_test_db(engineering_module_t *module)
{
    LOG_ENTER();
    LOGD("%s: current db %s, loading %s", __func__, module->hal->current_db_file, TEST_TEMPLATE_DB);
    int status = fpc_tee_load_template_db(module->hal->bio, TEST_TEMPLATE_DB);
    if (status) {
        LOGE("%s: fpc_tee_load_template_db: status: %d", __func__, status);
    }

    status = fpc_tee_load_instance_data(module->hal->bio);
    if (status) {
        LOGE("%s: fpc_tee_load_instance_data: status: %d", __func__, status);
    }

    LOG_LEAVE();
    return status;
}

static void restore_regular_db(engineering_module_t *module)
{
    LOG_ENTER();
    LOGD("%s: loading %s", __func__, module->hal->current_db_file);
    int status = fpc_tee_load_template_db(module->hal->bio, module->hal->current_db_file);
    if (status) {
        LOGE("%s: fpc_tee_load_template_db: status: %d", __func__, status);
    }

    status = fpc_tee_load_instance_data(module->hal->bio);
    if (status) {
        LOGE("%s: fpc_tee_load_instance_data: status: %d", __func__, status);
    }

    LOG_LEAVE();
}

static bool is_swipe_to_enrol_enabled(void)
{
#ifdef FPC_CONFIG_SWIPE_ENROL
    return true;
#else
    return false;
#endif
}

static int capture_images(fpc_engineering_t *self, fpc_capture_data_t *capture_data,
                          uint8_t wait_for_finger)
{
    uint8_t early_stop = CAC_EARLY_STOP_DISABLE;
    int es_status;
    engineering_module_t *module = (engineering_module_t *) self;

    LOG_ENTER();

    if (self->is_img_inj_enabled(self)) {
        capture_data->capture_result = self->handle_image_injection(self);
    } else {
        es_status = fpc_tee_early_stop_ctrl(module->hal->sensor, &early_stop);
        if (es_status) {
            LOGE("%s: Setting early stop failed: status: %d", __func__, es_status);
        }

        fpc_tee_debug_disable_check_pixels(module->hal->tee_handle);

        capture_data->capture_result = fpc_tee_capture_image(module->hal->sensor, wait_for_finger);

        if (fpc_tee_sensor_cancelled(module->hal->sensor)) {
            LOGE("%s: Capture image has been canceled", __func__);
            capture_data->capture_result = -FPC_ERROR_CANCELLED;
        }

        capture_data->cac_result = fpc_tee_get_last_cac_result(module->hal->sensor);
        if (!es_status) {
            // Restore early stop setting
            es_status = fpc_tee_early_stop_ctrl(module->hal->sensor, &early_stop);
            if (es_status) {
                LOGE("%s: Restoring early stop failed: status: %d", __func__, es_status);
            }
        }
    }

    if (capture_data->capture_result == FPC_ERROR_NONE) {
        // Must call the fpc_hal_ext_get_enhanced_image first to let algo set meta info
        capture_data->capture_result =
            fpc_hal_ext_get_enhanced_image(self, &capture_data->enhanced_image);
        if (capture_data->capture_result) {
            LOGE("%s: Failed getting enhanced image: status: %d",
                 __func__, capture_data->capture_result);
            goto exit;
        }

        capture_data->capture_result = fpc_hal_ext_get_raw_image(self, &capture_data->raw_image);
        if (capture_data->capture_result) {
            LOGE("%s: Failed getting raw image: status: %d",
                 __func__, capture_data->capture_result);
        }
    }

exit:

    LOG_LEAVE();
    return capture_data->capture_result;
}

static int capture_images_swipe(fpc_engineering_t *self, fpc_capture_data_t *capture_data,
                                uint8_t wait_for_finger)
{
    engineering_module_t *module = (engineering_module_t *) self;

    LOG_ENTER();

    if (self->is_img_inj_enabled(self)) {
        capture_data->capture_result = self->handle_image_injection(self);
    } else {
        capture_data->capture_result = fpc_tee_capture_image_swipe(module->hal->sensor,
                wait_for_finger);
        capture_data->cac_result = fpc_tee_get_last_cac_result(module->hal->sensor);
    }

    if (capture_data->capture_result == FPC_ERROR_NONE) {
        capture_data->capture_result = fpc_hal_ext_get_raw_image(self, &capture_data->raw_image);

        if (capture_data->capture_result) {
            LOGE("%s: Failed getting raw image: status: %d", __func__,
                    capture_data->capture_result);
            return capture_data->capture_result;
        }

        capture_data->capture_result = fpc_hal_ext_get_enhanced_image(self,
                &capture_data->enhanced_image);
        if (capture_data->capture_result) {
            LOGE("%s: Failed getting enhanced image: status: %d", __func__,
                    capture_data->capture_result);
        }
    }

    LOG_LEAVE();
    return capture_data->capture_result;
}

static int clear_enroll_challenge(engineering_module_t *module)
{
    int status = 0;
    uint64_t challenge = 0;

    LOG_ENTER();

    status = fpc_tee_get_enrol_challenge(module->hal->tee_handle, &challenge);
    if (status) {
        LOGE("%s: fpc_tee_get_enrol_challenge: status: %d\n", __func__, status);
        status = -EIO;
    }

    LOG_LEAVE();
    return status;
}

static int get_enroll_challenge(fpc_engineering_t *self, uint64_t *challenge)
{
    int status = 0;
    engineering_module_t *module = (engineering_module_t *) self;

    LOG_ENTER();

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);

    status = fpc_tee_get_enrol_challenge(module->hal->tee_handle, challenge);

    if (status) {
        LOGE("%s: fpc_tee_get_enrol_challenge: status: %d\n", __func__, status);
        challenge = 0;
    }

    fingerprint_hal_resume(module->hal);
    pthread_mutex_unlock(&module->hal->lock);

    LOG_LEAVE();
    return status;
}

typedef struct {
    void (*callback)(const char *build_info, void *usr);
    void *usr;
} buildinfo_cb_tuple_t;

static void get_build_info_callback(const fpc_ta_common_build_info_msg_t *build_info, void *usr)
{
    // This is simply just a proxy which calls the callback supplied to get_build_info() below.
    buildinfo_cb_tuple_t *tuple = usr;

    LOG_ENTER();

    if (tuple != NULL && tuple->callback != NULL) {
        const char *str = build_info->array;
        tuple->callback(str, tuple->usr);
    }

    LOG_LEAVE();
}

static int get_build_info(fpc_engineering_t *self, void (*callback)(const char *build_info,
                                                                    void *usr), void *usr)
{
    int status;
    engineering_module_t *module = (engineering_module_t *) self;
    buildinfo_cb_tuple_t tuple = { .callback = callback, .usr = usr };

    LOG_ENTER();

    status = fpc_tee_get_build_info(module->hal->tee_handle, get_build_info_callback, &tuple);
    if (status) {
        LOGE("%s: fpc_tee_get_build_info: status: %d\n", __func__, status);
    }

    LOG_LEAVE();

    return status;
}

static int set_enroll_token(fpc_engineering_t *self, const uint8_t *token, ssize_t token_length)
{
    int status;
    engineering_module_t *module = (engineering_module_t *) self;

    LOG_ENTER();

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);

    status = fpc_tee_authorize_enrol(module->hal->tee_handle, token, token_length);

    fingerprint_hal_resume(module->hal);
    pthread_mutex_unlock(&module->hal->lock);

    if (status) {
        LOGE("%s: fpc_tee_authorize_enrol: status: %d\n", __func__, status);
    }

    LOG_LEAVE();
    return status;
}

static void execute_capture_callback(engineering_module_t *module,
                                     fpc_capture_data_t *capture_data)
{
    fpc_capture_cb_t capture_cb = module->capture_cb;
    void *capture_cb_ctx = module->capture_cb_ctx;

    LOG_ENTER();

    if (!capture_data->samples_remaining) {
        module->capture_cb_ctx = NULL;
        module->capture_cb = NULL;
    }

    if (capture_cb && capture_cb_ctx) {
        capture_cb(capture_cb_ctx, capture_data);
    } else {
        LOGE("%s: callback was NULL", __func__);
    }

    LOG_LEAVE();
}

static void do_capture(void *data)
{
    int status;
    engineering_module_t *module = (engineering_module_t *) data;
    fpc_capture_data_t capture_data;

    LOG_ENTER();

    memset(&capture_data, 0, sizeof(capture_data));
    capture_data.mode = FPC_CAPTURE_MODE_CAPTURE;

    status = capture_images((fpc_engineering_t *)data, &capture_data, 1);
    if (status) {
        LOGE("%s: capture_images: status: %d", __func__, status);
    }

    execute_capture_callback(module, &capture_data);
    free_captured_images(&capture_data);

    LOG_LEAVE();
}

static void do_verify(void *data)
{
    int status = 0;
    uint32_t id, update;
    engineering_module_t *module = (engineering_module_t *) data;
    fpc_capture_data_t capture_data;
    memset(&capture_data, 0, sizeof(capture_data));

    LOG_ENTER();

    status = load_test_db(module);
    if (status) {
        LOGE("%s: load_test_db: status: %d", __func__, status);
        goto out;
    }

    capture_data.mode = FPC_CAPTURE_MODE_VERIFY;

    status = capture_images((fpc_engineering_t *)data, &capture_data, 1);

    if (status) {
        LOGE("%s: capture_images: status: %d", __func__, status);
        goto out;
    }

    status = fpc_tee_identify(module->hal->bio, &id, NULL);
    capture_data.identify_result = status;

    if (status != FPC_ERROR_NONE) {
        LOGE("%s: fpc_tee_identify: status: %d", __func__, status);
        goto out;
    }

    capture_data.user_id = id;

    status = fpc_tee_update_template(module->hal->bio, &update);

    if (status) {
        LOGE("%s: fpc_tee_update_template: status: %d", __func__, status);
        goto out;
    }

    capture_data.template_update_result = status;

    if (update != 0) {
        save_test_db(module);
    }

out:
    execute_capture_callback(module, &capture_data);

    free_captured_images(&capture_data);
    restore_regular_db(module);

    LOG_LEAVE();
}

static void do_enroll(void *data)
{
    int status = 0;
    uint8_t first_frame = 1;
    engineering_module_t *module = (engineering_module_t *) data;
    fpc_capture_data_t capture_data;
    memset(&capture_data, 0, sizeof(capture_data));

    LOG_ENTER();

    status = delete_test_db(module);
    if (status) {
        goto error;
    }
    load_test_db(module);
    capture_data.mode = FPC_CAPTURE_MODE_ENROLL;

    status = fpc_tee_set_token_validation_enable(module->hal->tee_handle, false);
    if (status) {
        LOGE("%s: fpc_tee_set_token_validation_enable: status: %d", __func__, status);
        goto error;
    }
    status = fpc_tee_begin_enrol(module->hal->bio);

    if (status) {
        capture_data.enroll_result = status;
        LOGE("%s: fpc_tee_begin_enrol: status: %d", __func__, status);
        goto error;
    }

    while (true) {
        bool image_captured = false;
        while (!image_captured) {
            memset(&capture_data, 0, sizeof(capture_data));
            capture_data.mode = FPC_CAPTURE_MODE_ENROLL;

            if (is_swipe_to_enrol_enabled()) {
                int finger_lost = fpc_tee_check_finger_lost(module->hal->sensor);

                LOGD("%s: fpc_tee_check_finger_lost: finger_lost: %d", __func__, finger_lost);
                if (finger_lost == 0 && !first_frame) {
                    status = capture_images_swipe((fpc_engineering_t *)data, &capture_data, 0);
                } else {
                    status = capture_images_swipe((fpc_engineering_t *)data, &capture_data, 1);
                }
            } else {
                status = capture_images((fpc_engineering_t *)data, &capture_data, 1);
            }
            first_frame = 0;
            LOGD("%s: capture_images: status: %d", __func__, status);
            switch (status) {
            case FPC_ERROR_NONE:
                // Image captured OK and will be sent below.
                image_captured = true;
                break;
            case FPC_STATUS_WAIT_TIME:
                LOGD("%s: FPC_STATUS_WAIT_TIME", __func__);
                capture_data.samples_remaining = REMAINING_UNKNOWN;
                execute_capture_callback(module, &capture_data);
                continue;
            case FPC_STATUS_ENROLL_LOW_COVERAGE:
                LOGD("%s: FPC_STATUS_ENROLL_LOW_COVERAGE", __func__);
                capture_data.samples_remaining = REMAINING_UNKNOWN;
                execute_capture_callback(module, &capture_data);
                continue;
            case FPC_STATUS_FINGER_LOST:
                capture_data.samples_remaining = REMAINING_UNKNOWN;
                execute_capture_callback(module, &capture_data);
                continue;
            default:
                goto error;
            }
        }

        status = fpc_tee_enrol(module->hal->bio, (uint32_t *)&capture_data.samples_remaining);
        capture_data.enroll_result = status;
        LOGD("%s: fpc_tee_enrol: samples_remaining: %d", __func__, capture_data.samples_remaining);

        if (status < 0) {
            LOGE("%s: fpc_tee_enrol: status: %d", __func__, status);
            goto error;
        }

        uint32_t id = 0;
        if (status == FPC_ERROR_NONE) {
            status  = fpc_tee_end_enrol(module->hal->bio, &id);
            if (status) {
                LOGE("%s: fpc_tee_end_enrol: status: %d", __func__, status);
                capture_data.enroll_result = status;
                goto error;
            }

            status = save_test_db(module);

            if (status) {
                LOGE("%s: save_test_db: status: %d", __func__, status);
                capture_data.enroll_result = status;
                goto error;
            }
            capture_data.user_id = id;
        }
        execute_capture_callback(module, &capture_data);
        free_captured_images(&capture_data);

        if (capture_data.samples_remaining == 0) {
            goto out;
        }
    }
error:
    capture_data.samples_remaining = 0;
    execute_capture_callback(module, &capture_data);
out:
    free_captured_images(&capture_data);
    restore_regular_db(module);
    status = fpc_tee_set_token_validation_enable(module->hal->tee_handle, true);
    clear_enroll_challenge(module);
    LOG_LEAVE();
}

static int start_capture(fpc_engineering_t *self, fpc_capture_cb_t callback,
                         fpc_capture_mode_t mode, void *ctx)
{
    int status = 0;
    engineering_module_t *module = (engineering_module_t *) self;

    LOG_ENTER();

    pthread_mutex_lock(&module->hal->lock);

    module->capture_cb = callback;
    module->capture_cb_ctx = ctx;

    LOGD("%s: Using capture mode: %d", __func__, mode);

    if (mode == FPC_CAPTURE_MODE_CAPTURE) {
        fingerprint_hal_goto_idle(module->hal);
        fingerprint_hal_do_async_work(module->hal, do_capture, (void *) module, FPC_TASK_HAL_EXT);
    } else if (mode == FPC_CAPTURE_MODE_VERIFY) {
        fingerprint_hal_goto_idle(module->hal);
        fingerprint_hal_do_async_work(module->hal, do_verify, (void *) module, FPC_TASK_HAL_EXT);
    } else if (mode == FPC_CAPTURE_MODE_ENROLL) {
        fingerprint_hal_goto_idle(module->hal);
        fingerprint_hal_do_async_work(module->hal, do_enroll, (void *) module, FPC_TASK_HAL_EXT);
    }

    pthread_mutex_unlock(&module->hal->lock);

    LOG_LEAVE();
    return status;
}

static int cancel_capture(fpc_engineering_t *self)
{
    int status = 0;
    engineering_module_t *module = (engineering_module_t *) self;

    LOG_ENTER();

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);
    fingerprint_hal_resume(module->hal);
    pthread_mutex_unlock(&module->hal->lock);

    LOG_LEAVE();
    return status;
}

static bool is_img_inj_enabled(fpc_engineering_t *self)
{
    engineering_module_t *module = (engineering_module_t *) self;
    bool enabled = false;

    LOG_ENTER();

    pthread_mutex_lock(&module->mutex);

    if (module->img_inj_cb) {
        enabled = true;
    }

    pthread_mutex_unlock(&module->mutex);

    LOG_LEAVE();
    return enabled;
}

static int set_img_inj_cb(fpc_engineering_t *self,
                          fpc_img_inj_cb_t injection_callback,
                          fpc_img_inj_cancel_cb_t cancel_callback, void *ctx)
{
    int status = 0;
    engineering_module_t *module = (engineering_module_t *) self;

    LOG_ENTER();

    pthread_mutex_lock(&module->mutex);

    if (injection_callback && cancel_callback) {
        module->img_inj_cb = injection_callback;
        module->img_inj_cancel_cb = cancel_callback;
        module->img_inj_cb_ctx = ctx;
    } else {
        module->img_inj_cb = NULL;
        module->img_inj_cancel_cb = NULL;
        module->img_inj_cb_ctx = NULL;
    }

    pthread_mutex_unlock(&module->mutex);

    LOG_LEAVE();
    return status;
}

static int handle_image_injection(fpc_engineering_t *self)
{
    int status = -1;
    fpc_hal_img_data_t image_data;
    engineering_module_t *module = (engineering_module_t *) self;

    LOG_ENTER();

    memset(&image_data, 0, sizeof(fpc_hal_img_data_t));

    pthread_mutex_lock(&module->mutex);

    if (module->img_inj_cb) {
        module->is_inj_wait_for_img = true;
        pthread_mutex_unlock(&module->mutex);

        // TODO: Fix module->img_inj_cb could be altered after mutex is unlocked
        status = module->img_inj_cb(module->img_inj_cb_ctx, &image_data);

        pthread_mutex_lock(&module->mutex);
        module->is_inj_wait_for_img = false;
        if (module->is_inj_cancel) {
            if (module->img_inj_cancel_cb) {
                module->img_inj_cancel_cb(module->img_inj_cb_ctx);
            }
            LOGE("%s: Canceled\n", __func__);
            status = -FPC_ERROR_CANCELLED;
        }
    }
    pthread_mutex_unlock(&module->mutex);

    if (status == 0) {
        if (image_data.buffer != NULL) {
            status = fpc_tee_debug_inject(module->hal->tee_handle,
                                          ENG_RAW_IMAGE,
                                          image_data.buffer,
                                          image_data.buffer_size);
        } else {
            status = -FPC_ERROR_IO;
            LOGE("%s: Failed to get fpc image data: status: %d\n", __func__, status);
        }
    } else if (status != -FPC_ERROR_CANCELLED) {
        status = -FPC_ERROR_IO;
    }

    if (image_data.buffer != NULL) {
        free(image_data.buffer);
    }

    LOG_LEAVE();
    return status;
}

static void set_cancel_image_injection(fpc_engineering_t *self)
{
    engineering_module_t *module = (engineering_module_t *) self;
    LOG_ENTER();

    pthread_mutex_lock(&module->mutex);
    module->is_inj_cancel = true;
    pthread_mutex_unlock(&module->mutex);
}

static void clear_cancel_image_injection(fpc_engineering_t *self)
{
    engineering_module_t *module = (engineering_module_t *) self;
    LOG_ENTER();

    pthread_mutex_lock(&module->mutex);
    module->is_inj_cancel = false;
    pthread_mutex_unlock(&module->mutex);

    LOG_LEAVE();
}

fpc_engineering_t *fpc_engineering_new(fpc_hal_common_t *hal)
{
    engineering_module_t *module;

    LOG_ENTER();

    module = malloc(sizeof(engineering_module_t));
    if (!module) {
        return NULL;
    }

    memset(module, 0, sizeof(engineering_module_t));
    module->hal = hal;
    module->is_inj_cancel = false;

    pthread_mutex_lock(&module->hal->lock);

    int status = fpc_tee_get_enhanced_image_size(module->hal->tee_handle,
                                                 &module->image_width,
                                                 &module->image_height);

    pthread_mutex_unlock(&module->hal->lock);

    if (status) {
        LOGE("%s: fpc_tee_get_enhanced_image_size: status: %d\n", __func__, status);
        goto err;
    }

    module->engineering.get_sensor_size = get_sensor_size;
    module->engineering.set_img_subscr_cb = set_img_subscr_cb;
    module->engineering.handle_image_subscription = handle_image_subscription;
    module->engineering.handle_image_subscription_enroll = handle_image_subscription_enroll;
    module->engineering.handle_image_subscription_auth = handle_image_subscription_auth;
    module->engineering.is_img_inj_enabled = is_img_inj_enabled;
    module->engineering.set_img_inj_cb = set_img_inj_cb;
    module->engineering.handle_image_injection = handle_image_injection;
    module->engineering.set_cancel_image_injection = set_cancel_image_injection;
    module->engineering.clear_cancel_image_injection = clear_cancel_image_injection;
    module->engineering.start_capture = start_capture;
    module->engineering.cancel_capture = cancel_capture;
    module->engineering.set_enroll_token = set_enroll_token;
    module->engineering.get_enroll_challenge = get_enroll_challenge;
    module->engineering.get_build_info = get_build_info;
    pthread_mutex_init(&module->mutex, NULL);

    return (fpc_engineering_t *) module;

err:
    free(module);
    LOG_LEAVE();
    return NULL;
}

void fpc_engineering_destroy(fpc_engineering_t *self)
{
    engineering_module_t *module = (engineering_module_t *) self;

    LOG_ENTER();

    if (!self) {
        return;
    }

    pthread_mutex_destroy(&module->mutex);
    free(self);

    LOG_LEAVE();
}
