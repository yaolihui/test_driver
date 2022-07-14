/*
 * Copyright (c) 2021 Fingerprint Cards AB <tech@fingerprints.com>
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
#include <time.h>

#include "fpc_hal_ext_authenticator_2.h"
#include "fpc_tee_hal.h"
#include "fpc_log.h"
#include "fpc_types.h"
#include "fpc_tee_fido_auth.h"
#include "fpc_tee_bio.h"
#include "fpc_hal_private.h"

#define AUTHENTICATOR_DB "/data/fpc/authenticator.db"
#define FPC_AUTH_TEST_APP "fpctest_fidoverify_app"

typedef struct {
    fpc_authenticator_2_t authenticator;
    uint32_t remove_id;
    const fpc_authenticator_compat_callback_t *callback;
    void *callback_context;
    fpc_hal_common_t *hal;
    int8_t  async_ongoing;
} authenticator_module_t;

static int32_t load_test_db(authenticator_module_t *module);

static void init(fpc_authenticator_2_t *self,
                 const fpc_authenticator_compat_callback_t *callback,
                 void *callback_context)
{
    LOG_ENTER();
    if (!self) {
        return;
    }

    authenticator_module_t *module = (authenticator_module_t *) self;
    module->async_ongoing = 0;

    pthread_mutex_lock(&module->hal->lock);
    module->callback = callback;
    module->callback_context = callback_context;
    pthread_mutex_unlock(&module->hal->lock);

    LOG_LEAVE();
}

static int32_t load_test_db(authenticator_module_t *module)
{
    LOGD("%s: current db %s, loading %s", __func__, module->hal->current_db_file, AUTHENTICATOR_DB);
    int32_t status = fpc_tee_load_template_db(module->hal->bio, AUTHENTICATOR_DB);
    if (status != 0) {
        LOGE("%s: failed to load_template_db", __func__);
        goto err;
    }

    status = fpc_tee_load_instance_data(module->hal->bio);
    if (FAILED(status)) {
        LOGE("%s: failed to load_instance_data", __func__);
    }

err:
    return status;
}

static void auth_restore_regular_db(authenticator_module_t *module)
{
    LOGD("%s: loading %s", __func__, module->hal->current_db_file);
    int32_t status = fpc_tee_load_template_db(module->hal->bio, module->hal->current_db_file);
    if (FAILED(status)) {
        LOGE("%s: fpc_tee_load_template_db failed, status: %d", __func__, status);
    }

    status = fpc_tee_load_instance_data(module->hal->bio);
    if (FAILED(status)) {
        LOGE("%s: fpc_tee_load_instance_data failed, status: %d", __func__, status);
    }
}

static void do_enroll(void *data)
{
    int32_t status = FPC_ERROR_NONE;
    uint32_t remaining_samples = 0;
    authenticator_module_t *module = (authenticator_module_t *) data;

    LOG_ENTER();

    module->async_ongoing = 1;
    status = load_test_db(module);
    if (FAILED(status)) {
        LOGE("%s: load_test_db failure (%d)", __func__, status);
        goto error;
    }

    status = fpc_tee_set_token_validation_enable(module->hal->tee_handle, false);
    if (FAILED(status)) {
        LOGE("%s: fpc_tee_set_token_validation_enable failure (%d)", __func__, status);
        goto error;
    }

    status = fpc_tee_begin_enrol(module->hal->bio);
    if (FAILED(status)) {
        LOGE("%s: fpc_tee_begin_enrol failure (%d)", __func__, status);
        goto error;
    }

    for (;;) {
        int32_t capture_status = 0;
        int32_t enroll_status = 0;

        capture_status = fpc_tee_capture_image(module->hal->sensor, 1);
        if (FAILED(capture_status)) {
            status = capture_status;
            LOGE("%s: fpc_tee_capture_image failure (%d)", __func__, status);
            goto error;
        }

        if (capture_status == FPC_ERROR_NONE) {
            uint32_t id = 0;

            enroll_status = fpc_tee_enrol(module->hal->bio, &remaining_samples);
            if (FAILED(enroll_status)) {
                status = enroll_status;
                LOGE("%s: fpc_tee_enrol failure (%d)", __func__, status);
                goto error;
            }

            if (enroll_status == FPC_ERROR_NONE) {
                enroll_status = fpc_tee_end_enrol(module->hal->bio, &id);
                if (FAILED(enroll_status)) {
                    status = enroll_status;
                    LOGE("%s: fpc_tee_end_enrol failure (%d)", __func__, status);
                    goto error;
                }

                enroll_status = fpc_tee_store_template_db(module->hal->bio, AUTHENTICATOR_DB);
                if (FAILED(enroll_status)) {
                    status = enroll_status;
                    LOGE("%s: fpc_tee_store_template_db failure (%d)", __func__, status);
                    goto error;
                }
            }

            switch (enroll_status) {
            case FPC_ERROR_NONE:
                module->callback->on_acquired(module->callback_context, capture_status);
                module->callback->on_enroll_result(module->callback_context, id, 0);
                break;
            case FPC_STATUS_ENROLL_PROGRESS:
                module->callback->on_acquired(module->callback_context, capture_status);
                module->callback->on_enroll_result(module->callback_context, 0, remaining_samples);
                break;
            case FPC_STATUS_ENROLL_TOO_SIMILAR:
                module->callback->on_acquired(module->callback_context, enroll_status);
                module->callback->on_enroll_result(module->callback_context, 0, remaining_samples);
                break;
            case FPC_STATUS_ENROLL_LOW_QUALITY:
            case FPC_STATUS_ENROLL_LOW_COVERAGE:
            case FPC_STATUS_ENROLL_LOW_MOBILITY:
                module->callback->on_acquired(module->callback_context, enroll_status);
                break;
            default:
                LOGE("%s: Unexpected enroll_status %d", __func__, enroll_status);
                break;
            }

            if (enroll_status == FPC_ERROR_NONE) {
                status = enroll_status;
                break;
            }
        } else {
            module->callback->on_acquired(module->callback_context, capture_status);
        }
    }

error:
    if (FAILED(status)) {
        module->callback->on_error(module->callback_context, status);
    }

    auth_restore_regular_db(module);
    fpc_tee_set_token_validation_enable(module->hal->tee_handle, true);
    module->async_ongoing = 0;

    LOG_LEAVE_TRACE(status);
}

static void hal_enroll(fpc_authenticator_2_t *self)
{
    LOG_ENTER();
    if (!self) {
        return;
    }

    authenticator_module_t *module = (authenticator_module_t *) self;

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);
    fingerprint_hal_do_async_work(module->hal, do_enroll, module, FPC_TASK_HAL_EXT);
    pthread_mutex_unlock(&module->hal->lock);

    LOG_LEAVE();
}

static void do_authenticate(void *data)
{
    int32_t status = FPC_ERROR_NONE;
    authenticator_module_t *module = (authenticator_module_t *) data;
    struct timespec start_time;
    int32_t identification_state = FPC_IDENTIFICATION_START;

    LOG_ENTER();

    module->async_ongoing = 1;
    status = load_test_db(module);
    if (FAILED(status)) {
        LOGE("%s: load_test_db failure (%d)", __func__, status);
        goto error;
    }

    for (;;) {
        uint32_t id = 0;
        int32_t diff_time;
        int32_t capture_status = 0;
        int32_t identify_status = 0;
        struct timespec current_time;

        capture_status = fpc_tee_capture_image(module->hal->sensor,
                                               identification_state == FPC_IDENTIFICATION_START);
        if (FAILED(capture_status)) {
            status = capture_status;
            LOGE("%s: fpc_tee_capture_image failure (%d)", __func__, status);
            goto error;
        }

        if (identification_state == FPC_IDENTIFICATION_START) {
            clock_gettime(CLOCK_MONOTONIC_RAW, &start_time);
            identification_state = FPC_IDENTIFICATION_RETRY;
        }

        if (capture_status == FPC_ERROR_NONE) {
            identify_status = fpc_tee_identify(module->hal->bio, &id, NULL);
            if (FAILED(identify_status)) {
                status = identify_status;
                LOGE("%s: fpc_tee_identify failure (%d)", __func__, status);
                goto error;
            }
            if (identify_status != FPC_STATUS_BAD_QUALITY) {
                identification_state = FPC_IDENTIFICATION_ATTEMPTED;
            }
        }

        clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
        diff_time = DIFF_TIME(start_time, current_time);

        if ((diff_time >= FPC_CONFIG_RETRY_MATCH_TIMEOUT) || id ||
            (capture_status == FPC_STATUS_FINGER_LOST)) {
            module->callback->on_acquired(module->callback_context, capture_status);

            if (identification_state == FPC_IDENTIFICATION_ATTEMPTED) {
                module->callback->on_authenticated(module->callback_context, id);
            }

            identification_state = FPC_IDENTIFICATION_START;
        }

        if (capture_status == FPC_ERROR_NONE) {
            uint32_t update = 0;

            status = fpc_tee_update_template(module->hal->bio, &update);
            if (FAILED(status)) {
                LOGE("%s: fpc_tee_update_template failure (%d)", __func__, status);
                goto error;
            }

            status = fpc_tee_store_template_db(module->hal->bio, AUTHENTICATOR_DB);
            if (FAILED(status)) {
                LOGE("%s: fpc_tee_store_template_db failure (%d)", __func__, status);
                goto error;
            }
        }
    }

error:
    if (FAILED(status)) {
        module->callback->on_error(module->callback_context, status);
    }

    auth_restore_regular_db(module);
    module->async_ongoing = 0;

    LOG_LEAVE_TRACE(status);
}

static void hal_authenticate(fpc_authenticator_2_t *self)
{
    LOG_ENTER();
    if (!self) {
        return;
    }

    authenticator_module_t *module = (authenticator_module_t *) self;

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);
    fingerprint_hal_do_async_work(module->hal, do_authenticate, module, FPC_TASK_HAL_EXT);
    pthread_mutex_unlock(&module->hal->lock);

    LOG_LEAVE();
}

static void do_enumerate(void *data)
{
    int32_t status = FPC_ERROR_NONE;
    authenticator_module_t *module = (authenticator_module_t *) data;

    LOG_ENTER();

    module->async_ongoing = 1;

    uint32_t max_number_of_templates = fpc_tee_get_max_number_of_templates(module->hal->tee_handle);
    uint32_t ids[max_number_of_templates];
    uint32_t size = max_number_of_templates;

    status = load_test_db(module);
    if (FAILED(status)) {
        LOGE("%s: load_test_db failure (%d)", __func__, status);
        goto error;
    }

    status = fpc_tee_get_finger_ids(module->hal->bio, &size, ids);
    if (FAILED(status)) {
        goto error;
    }

    if (size == 0) {
        module->callback->on_enumerate(module->callback_context, 0, 0);
        goto error;
    }

    for (uint32_t index = 0; index < size; index++) {
        module->callback->on_enumerate(module->callback_context, ids[index], (size - 1) - index);
    }

error:
    if (FAILED(status)) {
        module->callback->on_error(module->callback_context, status);
    }

    auth_restore_regular_db(module);
    module->async_ongoing = 0;

    LOG_LEAVE_TRACE(status);
    return;
}

static void hal_enumerate(fpc_authenticator_2_t *self)
{
    LOG_ENTER();
    if (!self) {
        return;
    }

    authenticator_module_t *module = (authenticator_module_t *) self;

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);
    fingerprint_hal_do_async_work(module->hal, do_enumerate, module, FPC_TASK_HAL_EXT);
    pthread_mutex_unlock(&module->hal->lock);

    LOG_LEAVE();
}

static void do_remove(void *data)
{
    int32_t status = FPC_ERROR_NONE;
    authenticator_module_t *module = (authenticator_module_t *) data;

    LOG_ENTER();

    module->async_ongoing = 0;

    uint32_t max_number_of_templates = fpc_tee_get_max_number_of_templates(module->hal->tee_handle);
    uint32_t ids[max_number_of_templates];
    uint32_t size = max_number_of_templates;

    status = load_test_db(module);
    if (FAILED(status)) {
        LOGE("%s: fpc auth load_test_db failure (%d)", __func__, status);
        goto error;
    }

    if (module->remove_id == 0) {
        status = fpc_tee_get_finger_ids(module->hal->bio, &size, ids);
        if (FAILED(status)) {
            LOGE("%s: fpc auth get finger ids failure (%d)", __func__, status);
            goto error;
        }

        if (!size) {
            module->callback->on_removed(module->callback_context, 0, 0);
            LOGE("%s: fpc auth size==0 failure size:(%d)", __func__, size);
            goto error;
        }
    } else {
        ids[0] = module->remove_id;
        size = 1;
    }

    for (uint32_t index = 0; index < size; index++) {
        status = fpc_tee_delete_template(module->hal->bio, ids[index]);
        if (FAILED(status)) {
            LOGE("%s: auth fpc_tee_delete_template failure (%d)", __func__, status);
            goto error;
        }

        status = fpc_tee_store_template_db(module->hal->bio, AUTHENTICATOR_DB);
        if (FAILED(status)) {
            LOGE("%s: auth fpc_tee_store_template_db failure (%d)", __func__, status);
            goto error;
        }

        module->callback->on_removed(module->callback_context, ids[index], size - 1 - index);
    }

error:
    if (FAILED(status)) {
        LOGE("%s: error callback failure (%d)", __func__, status);
        module->callback->on_error(module->callback_context, status);
    }

    auth_restore_regular_db(module);
    module->async_ongoing = 0;

    LOG_LEAVE_TRACE(status);
    return;
}

static void hal_remove(fpc_authenticator_2_t *self, uint32_t id)
{
    LOG_ENTER();
    if (!self) {
        return;
    }

    authenticator_module_t *module = (authenticator_module_t *) self;

    pthread_mutex_lock(&module->hal->lock);
    module->remove_id = id;
    fingerprint_hal_goto_idle(module->hal);
    fingerprint_hal_do_async_work(module->hal, do_remove, module, FPC_TASK_HAL_EXT);
    pthread_mutex_unlock(&module->hal->lock);

    LOG_LEAVE();
}

static void hal_cancel(fpc_authenticator_2_t *self)
{
    LOG_ENTER();
    if (!self) {
        return;
    }

    authenticator_module_t *module = (authenticator_module_t *) self;

    if (!module->async_ongoing/*module->hal->current_task.owner == FPC_TASK_HAL*/) {
        return;
    }

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);
    fingerprint_hal_resume(module->hal);
    pthread_mutex_unlock(&module->hal->lock);

    LOG_LEAVE();
}

fpc_authenticator_2_t *fpc_authenticator_2_new(fpc_hal_common_t *hal)
{
    authenticator_module_t *module = malloc(sizeof(authenticator_module_t));
    if (!module) {
        return NULL;
    }

    memset(module, 0, sizeof(authenticator_module_t));
    module->hal = hal;
    module->authenticator.init = init;
    module->authenticator.enroll = hal_enroll;
    module->authenticator.authenticate = hal_authenticate;
    module->authenticator.enumerate = hal_enumerate;
    module->authenticator.remove = hal_remove;
    module->authenticator.cancel = hal_cancel;

    return (fpc_authenticator_2_t *) module;
}

void fpc_authenticator_2_destroy(fpc_authenticator_2_t *self)
{
    if (self) {
        free(self);
    }
}

