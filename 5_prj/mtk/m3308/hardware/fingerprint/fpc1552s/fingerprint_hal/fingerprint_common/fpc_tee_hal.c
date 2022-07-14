/*
 * Copyright (c) 2015-2022 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <inttypes.h>
#include <fcntl.h>

#include "fpc_log.h"
#include "fpc_types.h"
#include "fpc_algo_ext.h"

#include "fpc_error_str.h"
#include "fpc_sysfs.h"
#include "fpc_ta_bio_interface.h"
#include "fpc_tee.h"
#include "fpc_tee_sensor.h"
#include "fpc_tee_bio.h"
#include "fpc_tee_hw_auth.h"
#include "fpc_tee_hal.h"
#include "fpc_tee_engineering.h"
#include "fpc_tee_kpi.h"

#include "fpc_worker.h"
#include "fpc_hal_private.h"
#include "fpc_hal_input_device.h"
#include "fpc_hal_navigation.h"
#include "fpc_hal_sense_touch.h"
#include "fpc_hal_ext_authenticator_service.h"
#include "fpc_hal_ext_authenticator_2_service.h"
#include "fpc_hal_ext_engineering_service.h"
#include "fpc_hal_ext_navigation_service.h"
#include "fpc_hal_ext_sensortest_service.h"
#include "fpc_hal_ext_sense_touch_service.h"

static bool is_treble_hal(void)
{
#ifdef PRE_TREBLE_HAL
    return false;
#else
    return true;
#endif
}

static bool is_swipe_to_enrol_enabled(void)
{
#ifdef FPC_CONFIG_SWIPE_ENROL
    return true;
#else
    return false;
#endif
}

static void hal_work_func(void *arg)
{
    fpc_hal_common_t *dev = (fpc_hal_common_t *) arg;
    dev->current_task.func(dev->current_task.arg);

    if (fpc_tee_navigation_enabled(dev->tee_handle)) {
        fpc_navigation_resume(dev->ext_navigation);
    }
}

void fingerprint_hal_do_async_work(fpc_hal_common_t *dev,
                                   void (*func)(void *), void *arg,
                                   fpc_task_owner_t owner)
{
    fpc_worker_join_task(dev->worker);

    if (fpc_tee_navigation_enabled(dev->tee_handle)) {
        fpc_navigation_pause(dev->ext_navigation);
    }
    dev->current_task.func = func;
    dev->current_task.arg = arg;
    dev->current_task.owner = owner;
    fpc_worker_run_task(dev->worker, hal_work_func, dev);
}

void fingerprint_hal_goto_idle(fpc_hal_common_t *dev)
{
    fpc_tee_set_cancel(dev->sensor);
    if (fpc_tee_engineering_enabled(dev->tee_handle)) {
        dev->ext_engineering->set_cancel_image_injection(dev->ext_engineering);
    }
    fpc_worker_join_task(dev->worker);
    fpc_tee_clear_cancel(dev->sensor);
    if (fpc_tee_engineering_enabled(dev->tee_handle)) {
        dev->ext_engineering->clear_cancel_image_injection(dev->ext_engineering);
    }
    if (fpc_tee_navigation_enabled(dev->tee_handle)) {
        fpc_navigation_pause(dev->ext_navigation);
    }
}

void fingerprint_hal_resume(fpc_hal_common_t *dev)
{
    fpc_worker_join_task(dev->worker);
    if (fpc_tee_navigation_enabled(dev->tee_handle)) {
        fpc_navigation_resume(dev->ext_navigation);
    }
}

static int sense_touch_wait_for_force(fpc_hal_common_t *dev, int status)
{
    const fpc_sense_touch_config_t *st_config = fpc_sense_touch_get_config();

    if (status == FPC_ERROR_NONE && st_config != NULL && st_config->auth_enable_down_force) {
        int result = fpc_tee_wait_for_button_down_force(dev->sensor,
                                                        st_config->auth_button_timeout_ms,
                                                        st_config->trigger_threshold);
        switch (FPC_ERROR_GET_EXTERNAL_ERROR(result)) {
        case FPC_ERROR_NONE:
            report_input_event(FPC_SENSE_TOUCH_EVENT, FPC_SENSE_TOUCH_AUTH_PRESS,
                               FPC_HAL_INPUT_KEY_DOWN);
            break;
        default:
            status = result;
            break;
        }
    }
    if (status == FPC_ERROR_NONE && st_config != NULL && st_config->auth_enable_up_force) {

        int result = fpc_tee_wait_for_button_up_force(dev->sensor,
                                                      st_config->auth_button_timeout_ms,
                                                      st_config->untrigger_threshold);

        switch (FPC_ERROR_GET_EXTERNAL_ERROR(result)) {
        case FPC_ERROR_NONE:
            report_input_event(FPC_SENSE_TOUCH_EVENT, FPC_SENSE_TOUCH_AUTH_PRESS,
                               FPC_HAL_INPUT_KEY_UP);
            break;
        case -FPC_ERROR_TIMEDOUT:
            report_input_event(FPC_SENSE_TOUCH_EVENT, FPC_SENSE_TOUCH_AUTH_PRESS,
                               FPC_HAL_INPUT_KEY_UP);
            break;
        default:
            status = result;
            break;
        }
    }
    return status;
}

static int32_t capture_image(fpc_hal_common_t *dev, uint8_t wait_for_finger)
{
    int32_t status = 0;

    if (fpc_tee_engineering_enabled(dev->tee_handle) &&
        dev->ext_engineering->is_img_inj_enabled(dev->ext_engineering)) {
        status = dev->ext_engineering->handle_image_injection(dev->ext_engineering);
    } else {
        status = fpc_tee_capture_image(dev->sensor, wait_for_finger);

        if (fpc_tee_force_sensor_enabled(dev->tee_handle) && wait_for_finger) {
            status = sense_touch_wait_for_force(dev, status);
        }
    }

    return status;
}

static int32_t capture_image_swipe(fpc_hal_common_t *dev, uint8_t wait_for_finger)
{
    int32_t status = 0;

    if (fpc_tee_engineering_enabled(dev->tee_handle) &&
        dev->ext_engineering->is_img_inj_enabled(dev->ext_engineering)) {
        status = dev->ext_engineering->handle_image_injection(dev->ext_engineering);
    } else {
        status = fpc_tee_capture_image_swipe(dev->sensor, wait_for_finger);
    }

    return status;
}

static void handle_kpi_statistics(fpc_hal_common_t *dev, int status, uint32_t id)
{
    if (fpc_tee_engineering_enabled(dev->tee_handle)) {
        fpc_ta_bio_identify_statistics_t stat;
        memset(&stat, 0, sizeof(fpc_ta_bio_identify_statistics_t));
        int status_get_statistics = fpc_tee_get_identify_statistics(dev->bio, &stat);
        if (status_get_statistics) {
            LOGD("%s failed to get identify statistics", __func__);
        }
        dev->ext_engineering->handle_image_subscription_auth(dev->ext_engineering, 0, status,
                                                             stat.coverage, stat.quality, id);
    }
}

static int do_template_update(const fpc_hal_common_t *dev)
{
    int update_status = 0;
    uint32_t update = 0;

    update_status = fpc_tee_update_template(dev->bio, &update);

    if (update_status) {
        goto out;
    }

    if (update != 0) {
        fpc_tee_store_template_db(dev->bio, dev->current_db_file);
    }

out:
    return update_status;
}

static void log_capture_status(const char *function_name, const int capture_status)
{

    switch (capture_status) {
    case FPC_STATUS_WAIT_TIME:
        LOGD("%s FPC_STATUS_WAIT_TIME", function_name);
        break;
    case FPC_STATUS_FINGER_LOST:
        LOGD("%s FPC_STATUS_FINGER_LOST", function_name);
        break;
    case FPC_STATUS_BAD_QUALITY:
        LOGD("%s FPC_STATUS_BAD_QUALITY", function_name);
        break;
    case FPC_STATUS_ENROLL_LOW_COVERAGE:
        LOGD("%s FPC_STATUS_ENROLL_LOW_COVERAGE", function_name);
        break;
    case FPC_ERROR_NONE:
        break;
    default:
        LOGE("%s, capture_status: %d", function_name, capture_status);
        break;
    }
}

static int send_notification(const fpc_hal_common_t *dev, uint32_t id,
                             int capture_status, int identification_state,
                             uint32_t *failed_attempts)
{
    int status = 0;
    log_capture_status(__func__, capture_status);

    /* If an identification was made we handle this as an identification attempt. */
    if (identification_state == FPC_IDENTIFICATION_ATTEMPTED) {
        uint8_t hat[69];
        uint8_t *token = NULL;
        uint32_t token_size = 0;

        if (id != 0) {
            token      = hat;
            token_size = sizeof(hat);
            status = fpc_tee_get_auth_result(dev->tee_handle, token, token_size);

            if (status) {
                goto out;
            }
        }

        if (id == 0) {
            (*failed_attempts)++;
        }

        dev->callback->on_acquired(dev->callback_context, HAL_COMPAT_ACQUIRED_GOOD);

        dev->callback->on_authenticated(dev->callback_context,
                                        id, dev->current_gid,
                                        token, token_size);
    } else if (capture_status == FPC_STATUS_FINGER_LOST) {
        dev->callback->on_acquired(dev->callback_context,
                                   HAL_COMPAT_ACQUIRED_TOO_FAST);
    } else {
        dev->callback->on_acquired(dev->callback_context,
                                   HAL_COMPAT_ACQUIRED_INSUFFICIENT);
    }

out:
    return status;
}

static void do_authenticate(void *data)
{
    LOGD("%s", __func__);
    int status = 0;
    fpc_hal_common_t *dev = (fpc_hal_common_t *) data;
    /* failed_attempts variable need to be in sync with androids internal counter mFailedAttempts.
       Android only increments mFailedAttempts when a call to onAuthenticated is made with an
       incorrect id. */
    uint32_t failed_attempts = 0;
    struct timespec start_time;
    int identification_state = FPC_IDENTIFICATION_START;

    status = fpc_tee_set_auth_challenge(dev->tee_handle, dev->challenge);
    if (status) {
        goto out;
    }

    for (;;) {
        uint32_t id = 0;
        int32_t diff_time;
        int identify_status = 0;
        fpc_ta_bio_identify_statistics_t stat;
        struct timespec current_time;

        fpc_tee_kpi_start_sequence(dev->tee_handle,
                                   identification_state == FPC_IDENTIFICATION_START);

        memset(&stat, 0, sizeof(fpc_ta_bio_identify_statistics_t));
        const int capture_status = capture_image(dev,
                                                 identification_state == FPC_IDENTIFICATION_START);

        if (capture_status < 0) {
            status = capture_status;
            goto out;
        }

        /* Check if we have been cancelled. */
        if (fpc_tee_sensor_cancelled(dev->sensor)) {
            status = -FPC_ERROR_CANCELLED;
            goto out;
        }

        /* We are only intrested in a finger present verification the first time. */
        if (identification_state == FPC_IDENTIFICATION_START) {
            clock_gettime(CLOCK_MONOTONIC_RAW, &start_time);
            identification_state = FPC_IDENTIFICATION_RETRY;
        }

        /* Do an identification if we got a successful capture. */
        if (capture_status == FPC_ERROR_NONE) {
            identify_status = fpc_tee_identify(dev->bio, &id, &stat);
            handle_kpi_statistics(dev, status, id);
            if (identify_status < 0) {
                status = identify_status;
                goto out;
            } else if (identify_status != FPC_STATUS_BAD_QUALITY) {
                identification_state = FPC_IDENTIFICATION_ATTEMPTED;
            }
        }

        clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
        diff_time = DIFF_TIME(start_time, current_time);

        LOGD("%s capture_status: %d, identify_status: %d, id: %d", __func__, capture_status,
             identify_status, id);

        /* If we have reached a timeout or we had an successful identifcation or a
         * FPC_CAPTURE_FINGER_LOST we report our result to android. */
        if ((diff_time >= FPC_CONFIG_RETRY_MATCH_TIMEOUT) || id ||
            (capture_status == FPC_STATUS_FINGER_LOST)) {
            send_notification(dev, id, capture_status, identification_state, &failed_attempts);

            /* Restart loop. */
            identification_state = FPC_IDENTIFICATION_START;
        }

        /* Do a template update. This takes some time, so we must do after notifications have been
         * sent to android. */
        if (capture_status == FPC_ERROR_NONE) {
            if (identify_status == FPC_ERROR_NONE) {
                status = do_template_update(dev);
                if (status < 0) {
                    goto out;
                }
            } else if (identify_status == FPC_STATUS_BAD_QUALITY) {
                /* No templates are updated, but instance data is updated in TA. Store the DB. */
                fpc_tee_store_template_db(dev->bio, dev->current_db_file);
            }
        }

        fpc_tee_kpi_stop_sequence(dev->tee_handle); //Won't harm

        /* If we have made a successful match we can exit.*/
        if (id) {
            goto out;
        }

        /* Check if the number of failed identification attempts are more then the max number of
         * failed attempts allowed. If so we exit. */
        if (failed_attempts >= FPC_MAX_IDENTIFY_ATTEMPTS) {
            LOGD("%s failed %d times", __func__, failed_attempts);
            status = -FPC_ERROR_CANCELLED;
            goto out;
        }
    } // for(;;)

out:
    /* Will not harm to stop again even if it's already stopped */
    fpc_tee_kpi_stop_sequence(dev->tee_handle);

    if (status) {
        LOGE("%s failed %s", __func__, fpc_error_str(status));
        switch (FPC_ERROR_GET_EXTERNAL_ERROR(status)) {
        case -FPC_ERROR_CANCELLED:
            break;
        default:
            dev->callback->on_error(dev->callback_context,
                                    HAL_COMPAT_ERROR_HW_UNAVAILABLE);
            break;
        }
    }
    if (fpc_tee_log_to_file_enabled(dev->tee_handle)) {
        fpc_tee_write_ta_log(__func__, dev->tee_handle);
    }
}

static int enroll_save_template(fpc_hal_common_t *dev, uint32_t id)
{
    int status = fpc_tee_store_template_db(dev->bio, dev->current_db_file);

    if (status) {
        // Reload the database from filesystem to have matching templates inside TA
        fpc_tee_load_template_db(dev->bio, dev->current_db_file);
        fpc_tee_load_instance_data(dev->bio);
        goto out;
    }

    if (fpc_tee_sensor_cancelled(dev->sensor)) {
        // Enroll was cancelled before notifying upper layers, hence remove template from database
        LOGD("%s cancelled enroll", __func__);
        fpc_tee_delete_template(dev->bio, id);
        fpc_tee_store_template_db(dev->bio, dev->current_db_file);
        status = -FPC_ERROR_CANCELLED;
        goto out;
    }

    status = fpc_tee_get_template_db_id(dev->bio, &dev->authenticator_id);
    if (status) {
        LOGE("%s failed to get auth id %i", __func__, status);
        dev->authenticator_id = 0;
    }
    status = 0;

out:
    return status;
}

static void enroll_notify(fpc_hal_common_t *dev, int enroll_status, int remaining_samples,
                          uint32_t id)
{
    switch (enroll_status) {
    case FPC_ERROR_NONE:
        dev->callback->on_enroll_result(dev->callback_context, id, dev->current_gid, 0);
        break;
    case FPC_STATUS_ENROLL_PROGRESS:
        dev->callback->on_acquired(dev->callback_context, HAL_COMPAT_ACQUIRED_GOOD);
        dev->callback->on_enroll_result(dev->callback_context, 0, dev->current_gid,
                                        remaining_samples);
        break;
    case FPC_STATUS_ENROLL_TOO_SIMILAR:
        dev->callback->on_acquired(dev->callback_context, HAL_COMPAT_ACQUIRED_TOO_SIMILAR);
        dev->callback->on_enroll_result(dev->callback_context, 0, dev->current_gid,
                                        remaining_samples);
        break;
    case FPC_STATUS_ENROLL_LOW_QUALITY:
        dev->callback->on_acquired(dev->callback_context, HAL_COMPAT_ACQUIRED_IMAGER_DIRTY);
        break;
    case FPC_STATUS_ENROLL_LOW_COVERAGE:
        dev->callback->on_acquired(dev->callback_context, HAL_COMPAT_ACQUIRED_PARTIAL);
        break;
    case FPC_STATUS_ENROLL_LOW_MOBILITY:
        dev->callback->on_acquired(dev->callback_context, HAL_COMPAT_ACQUIRED_LOW_MOBILITY);
        break;
    default:
        LOGE("%s, unexpected enroll_status %d", __func__, enroll_status);
        break;
    }

    if (fpc_tee_engineering_enabled(dev->tee_handle)) {
        dev->ext_engineering->handle_image_subscription_enroll(dev->ext_engineering, 0,
                                                               enroll_status, remaining_samples, id);
    }
}

static void do_enroll(void *data)
{
    LOGD("%s", __func__);

    int status = 0;
    uint8_t first_frame = 1;
    fpc_hal_common_t *dev = (fpc_hal_common_t *) data;

    uint32_t remaining_samples = 0;

    status = fpc_tee_authorize_enrol(dev->tee_handle,
                                     dev->hat, sizeof(dev->hat));
    if (status) {
        goto out;
    }
#ifdef FPC_CONFIG_ENROL_TIMEOUT
    status = fpc_tee_check_enrol_timeout(dev->tee_handle, dev->timeout_sec, 1);
    if (status) {
        goto out;
    }
#endif

    if (!dev->kpi_skip) {
        fpc_tee_kpi_start(dev->tee_handle);
    }

    status = fpc_tee_begin_enrol(dev->bio);

    if (status) {
        goto out;
    }

    for (;;) {
        const int capture_status = is_swipe_to_enrol_enabled() ?
                                   capture_image_swipe(dev, first_frame) : capture_image(dev, 1);
        first_frame = 0;
        log_capture_status(__func__, capture_status);

        switch (capture_status) {
        case FPC_STATUS_ENROLL_LOW_COVERAGE:
            dev->callback->on_acquired(dev->callback_context,
                                       HAL_COMPAT_ACQUIRED_PARTIAL);
            break;
        case FPC_STATUS_WAIT_TIME:
            dev->callback->on_acquired(dev->callback_context,
                                       HAL_COMPAT_ACQUIRED_INSUFFICIENT);
            break;
        case FPC_STATUS_FINGER_LOST:
            dev->callback->on_acquired(dev->callback_context,
                                       HAL_COMPAT_ACQUIRED_TOO_FAST);
            break;
        case FPC_ERROR_NONE:
            break;
        default:
            status = capture_status;
            goto out;
        }

        if (capture_status == FPC_ERROR_NONE) {
            uint32_t id = 0;
            int enroll_status = fpc_tee_enrol(dev->bio, &remaining_samples);

            if (enroll_status < 0) {
                status = enroll_status;
                goto out;
            }

            if (enroll_status == FPC_ERROR_NONE) {
#ifdef FPC_CONFIG_ENROL_TIMEOUT
                if (fpc_tee_check_enrol_timeout(dev->tee_handle, dev->timeout_sec, 0)
                    == -FPC_ERROR_TIMEDOUT) {
                    status = -FPC_ERROR_TIMEDOUT;
                    goto out;
                }
#endif
                enroll_status  = fpc_tee_end_enrol(dev->bio, &id);
                if (enroll_status) {
                    status = enroll_status;
                    goto out;
                }

                enroll_status = enroll_save_template(dev, id);
                if (enroll_status) {
                    status = enroll_status;
                    goto out;
                }
            }

            enroll_notify(dev, enroll_status, remaining_samples, id);

            if (enroll_status == FPC_ERROR_NONE) {
                status = enroll_status;
                goto out;
            }
        }
    }

out:

    if (FAILED(status)) {
        LOGE("%s failed %s", __func__, fpc_error_str(status));
        switch (FPC_ERROR_GET_EXTERNAL_ERROR(status)) {
        case -FPC_ERROR_CANCELLED:
            break;
        case -FPC_ERROR_TIMEDOUT:
            dev->callback->on_error(dev->callback_context,
                                    HAL_COMPAT_ERROR_TIMEOUT);
            break;
        case -FPC_ERROR_STORAGE:
            dev->callback->on_error(dev->callback_context,
                                    HAL_COMPAT_ERROR_NO_SPACE);
            break;
        case -FPC_ERROR_PARAMETER:
            dev->callback->on_error(dev->callback_context,
                                    HAL_COMPAT_ERROR_UNABLE_TO_PROCESS);
            break;
        case -FPC_ERROR_ALREADY_ENROLLED:
            dev->callback->on_error(dev->callback_context,
                                    HAL_COMPAT_ERROR_ALREADY_ENROLLED);

            if (fpc_tee_engineering_enabled(dev->tee_handle)) {
                dev->ext_engineering->handle_image_subscription_enroll(dev->ext_engineering,
                                                                       0,
                                                                       status,
                                                                       remaining_samples,
                                                                       0);
            }
            break;
        case -FPC_ERROR_TOO_MANY_FAILED_ATTEMPTS:
            dev->callback->on_error(dev->callback_context,
                                    HAL_COMPAT_ERROR_UNABLE_TO_PROCESS);
            break;
        default:
            dev->callback->on_error(dev->callback_context,
                                    HAL_COMPAT_ERROR_HW_UNAVAILABLE);
            break;
        }
    }

    if (!dev->kpi_skip) {
        fpc_tee_kpi_stop(dev->tee_handle);
    }

    if (fpc_tee_log_to_file_enabled(dev->tee_handle)) {
        fpc_tee_write_ta_log(__func__, dev->tee_handle);
    }
}

uint64_t fpc_pre_enroll(fpc_hal_common_t *dev)
{
    LOGD("%s", __func__);
    pthread_mutex_lock(&dev->lock);
    fingerprint_hal_goto_idle(dev);
    uint64_t challenge = 0;

    int status = fpc_tee_get_enrol_challenge(dev->tee_handle, &challenge);

    if (status) {
        LOGE("%s failed %i", __func__, status);
        challenge = 0;
    }

    LOGD("%s challenge %" PRIu64, __func__, challenge);

    dev->enroll_counter = 0;
    dev->kpi_skip = false;

    fingerprint_hal_resume(dev);
    pthread_mutex_unlock(&dev->lock);

    return challenge;
}

int fpc_post_enroll(fpc_hal_common_t *dev)
{
    LOGD("%s", __func__);
    pthread_mutex_lock(&dev->lock);
    int status = 0;
    uint64_t challenge = 0;
    status = fpc_tee_get_enrol_challenge(dev->tee_handle, &challenge);
    if (status) {
        LOGE("%s failed %i", __func__, status);
        status = -EIO;
    }

    pthread_mutex_unlock(&dev->lock);

    return status;
}

uint64_t fpc_get_authenticator_id(fpc_hal_common_t *dev)
{
    LOGD("%s", __func__);

    pthread_mutex_lock(&dev->lock);
    uint64_t id = dev->authenticator_id;
    pthread_mutex_unlock(&dev->lock);

    return id;
}

int fpc_set_active_group(fpc_hal_common_t *dev, uint32_t gid,
                         const char *store_path)
{
    int status = 0;

    LOGD("%s", __func__);

    pthread_mutex_lock(&dev->lock);
    fingerprint_hal_goto_idle(dev);
    int length = snprintf(dev->current_db_file,
                          sizeof(dev->current_db_file), "%s%s",
                          store_path, TEMPLATE_POSTFIX);

    if (length < 0 || (unsigned) length >= sizeof(dev->current_db_file)) {
        status = -EINVAL;
        goto out;
    }

    status = fpc_tee_load_template_db(dev->bio, dev->current_db_file);
    if (status != 0) {
        LOGE("%s: fpc_tac_load_user_db failed with error %s", __func__, fpc_error_str(status));
        status = -1;
        goto out;
    }

    status = fpc_tee_load_instance_data(dev->bio);
    if (status != 0) {
        LOGE("%s: fpc_tee_load_instance_data failed with error %s", __func__, fpc_error_str(status));
        goto out;
    }

    status = fpc_tee_set_gid(dev->bio, gid);
    if (status) {
        goto out;
    }

    dev->current_gid = gid;

    status = fpc_tee_get_template_db_id(dev->bio, &dev->authenticator_id);

    if (status) {
        LOGE("%s failed to get auth id %i", __func__, status);
        dev->authenticator_id = 0;
    }

out:
    if (status) {
        LOGE("%s failed %s", __func__, fpc_error_str(status));
        status = -1;
    }

    fingerprint_hal_resume(dev);
    pthread_mutex_unlock(&dev->lock);

    return status;
}

int fpc_authenticate(fpc_hal_common_t *dev,
                     uint64_t operation_id, uint32_t gid)
{
    LOGD("%s operation_id %" PRIu64, __func__, operation_id);

    int status = 0;
    pthread_mutex_lock(&dev->lock);

    if (gid != dev->current_gid) {
        LOGD("%s finger.gid != current_gid", __func__);
        status = -1;
        goto out;
    }

    fingerprint_hal_goto_idle(dev);
    dev->challenge = operation_id;
    fingerprint_hal_do_async_work(dev, do_authenticate, dev, FPC_TASK_HAL);
out:

    pthread_mutex_unlock(&dev->lock);
    return status;
}

int fpc_enroll(fpc_hal_common_t *dev, const uint8_t *hat, uint32_t size_hat,
               uint32_t gid, uint32_t timeout_sec)
{
    LOGD("%s: timeout_sec: %d", __func__, timeout_sec);
#ifdef FPC_CONFIG_ENROL_TIMEOUT
    dev->timeout_sec = timeout_sec;
#else
    (void)timeout_sec; // Unused
#endif
    LOGD("%s", __func__);

    int status = 0;
    int status_get_file_size = 0;
    size_t file_size = 0;
    pthread_mutex_lock(&dev->lock);

    if (gid != dev->current_gid) {
        LOGD("%s finger.gid != current_gid", __func__);
        status = -1;
        goto out;
    }

    if (size_hat != sizeof(dev->hat)) {
        LOGD("%s hat size mismatch %u", __func__, size_hat);
        status = -1;
        goto out;
    }

    memcpy(dev->hat, hat, size_hat);

    status_get_file_size = fpc_get_file_size(dev->current_db_file, &file_size);

    if (!(dev->enroll_counter) && status_get_file_size >= 0 &&
        file_size < FPC_DB_FILE_SIZE_OF_NO_TEMPLATES) {
        LOGD("%s, kpi_skip is set, file size %zu", __func__, file_size);
        dev->kpi_skip = true;
    } else {
        dev->kpi_skip = false;
    }

    dev->enroll_counter++;

    fingerprint_hal_goto_idle(dev);
    fingerprint_hal_do_async_work(dev, do_enroll, dev, FPC_TASK_HAL);

out:
    pthread_mutex_unlock(&dev->lock);
    return status;
}

int fpc_cancel(fpc_hal_common_t *dev)
{
    LOGD("%s", __func__);

    pthread_mutex_lock(&dev->lock);
    if (dev->current_task.owner == FPC_TASK_HAL) {
        fingerprint_hal_goto_idle(dev);
        fingerprint_hal_resume(dev);
    }

    dev->callback->on_error(dev->callback_context,
                            HAL_COMPAT_ERROR_CANCELED);

    pthread_mutex_unlock(&dev->lock);
    return 0;
}

static void do_remove(void *device)
{
    fpc_hal_common_t *dev = (fpc_hal_common_t *) device;
    uint32_t max_number_of_templates = fpc_tee_get_max_number_of_templates(dev->tee_handle);

    uint32_t ids[max_number_of_templates];
    uint32_t size = max_number_of_templates;

    int status = 0;

    if (dev->remove_fid == 0) {
        status = fpc_tee_get_finger_ids(dev->bio, &size, ids);
        if (status) {
            status = -EIO;
            goto out;
        }
        if (!size) {
            dev->callback->on_removed(dev->callback_context, 0, dev->current_gid, 0);
            return;
        }
    } else {
        ids[0] = dev->remove_fid;
        size = 1;
    }

    for (unsigned i = 0; i < size; ++i) {

        status = fpc_tee_delete_template(dev->bio, ids[i]);
        if (status != 0 && FPC_ERROR_GET_EXTERNAL_ERROR(status) != -FPC_ERROR_NOT_FOUND) {
            LOGE("%s delete_template failed %i", __func__, status);
            status = -EIO;
            goto out;
        }

        if (FPC_ERROR_GET_EXTERNAL_ERROR(status) != -FPC_ERROR_NOT_FOUND) {
            status = fpc_tee_store_template_db(dev->bio,
                                               dev->current_db_file);
            if (status) {
                LOGE("%s store_template_db failed %i", __func__, status);
                status = -EIO;
                goto out;
            }
        }

        status = 0;
        dev->callback->on_removed(dev->callback_context,
                                  ids[i], dev->current_gid,
                                  size - 1 - i);
    }

    // We do not want to change behaviour in android 7 or earlier.
    if (!is_treble_hal() || dev->remove_fid == 0) {
        dev->callback->on_removed(dev->callback_context, 0, dev->current_gid, 0);
    }

out:

    if (status) {
        dev->callback->on_error(dev->callback_context,
                                HAL_COMPAT_ERROR_UNABLE_TO_REMOVE);

        LOGE("%s failed %i, reloading db", __func__, status);
        status = fpc_tee_load_template_db(dev->bio, dev->current_db_file);
        if (status != 0) {
            LOGE("%s: fpc_tac_load_user_db failed with error %s", __func__,
                 fpc_error_str(status));
        }

        status = fpc_tee_load_instance_data(dev->bio);
        if (status != 0) {
            LOGE("%s: fpc_tee_load_instance_data failed with error %s", __func__,
                 fpc_error_str(status));
        }
    }

}

int fpc_remove(fpc_hal_common_t *dev, uint32_t gid, uint32_t fid)
{
    int status = 0;

    LOGD("%s(fid=%u gid=%u)", __func__, fid, gid);

    pthread_mutex_lock(&dev->lock);

    if (gid != dev->current_gid) {
        LOGD("%s gid != current_gid, nothing to remove", __func__);
        status = -2;
        goto out;
    }

    dev->remove_fid = fid;

    fingerprint_hal_goto_idle(dev);
    fingerprint_hal_do_async_work(dev, do_remove, dev, FPC_TASK_HAL);


out:
    pthread_mutex_unlock(&dev->lock);

    return status;
}

static void do_enumerate(void *device)
{
    fpc_hal_common_t *dev = (fpc_hal_common_t *) device;

    uint32_t max_number_of_templates = fpc_tee_get_max_number_of_templates(dev->tee_handle);

    uint32_t ids[max_number_of_templates];
    uint32_t size = max_number_of_templates;

    int status = fpc_tee_get_finger_ids(dev->bio, &size, ids);
    if (status) {
        dev->callback->on_error(dev->callback_context,
                                HAL_COMPAT_ERROR_UNABLE_TO_PROCESS);
        LOGE("%s failed %s", __func__, fpc_error_str(status));
        return;
    }

    if (size == 0) {
        dev->callback->on_enumerate(dev->callback_context, 0,
                                    dev->current_gid, 0);
        return;
    }

    for (unsigned i = 0; i < size; ++i) {
        dev->callback->on_enumerate(dev->callback_context, ids[i],
                                    dev->current_gid, (size - 1) - i);
    }

}

int fpc_enumerate(fpc_hal_common_t *dev)
{
    LOGD("%s", __func__);

    pthread_mutex_lock(&dev->lock);

    fingerprint_hal_goto_idle(dev);
    fingerprint_hal_do_async_work(dev, do_enumerate, dev, FPC_TASK_HAL);

    pthread_mutex_unlock(&dev->lock);

    return 0;
}

void fpc_hal_close(fpc_hal_common_t *device)
{
    LOGD("%s", __func__);

    if (!device) {
        return;
    }

    fpc_hal_common_t *dev = (fpc_hal_common_t *) device;

    pthread_mutex_lock(&dev->lock);

    if (dev->tee_handle) {
        if (dev->sensor) {
            fpc_tee_set_cancel(dev->sensor);
        }
        if (fpc_tee_engineering_enabled(dev->tee_handle) && dev->ext_engineering) {
            dev->ext_engineering->set_cancel_image_injection(dev->ext_engineering);
        }
        if (dev->worker) {
            fpc_worker_join_task(dev->worker);
        }
        if (dev->sensor) {
            fpc_tee_clear_cancel(dev->sensor);
        }
        if (fpc_tee_engineering_enabled(dev->tee_handle) && dev->ext_engineering) {
            dev->ext_engineering->clear_cancel_image_injection(dev->ext_engineering);
        }
        if (fpc_tee_navigation_enabled(dev->tee_handle) && dev->ext_navigation) {
            fpc_navigation_pause(dev->ext_navigation);
        }
        if (fpc_tee_sensortest_enabled(dev->tee_handle)) {
            fpc_sensortest_destroy(dev->ext_sensortest);
        }
        if (fpc_tee_engineering_enabled(dev->tee_handle)) {
            fpc_engineering_destroy(dev->ext_engineering);
        }
        if (fpc_tee_authenticator_enabled(dev->tee_handle)) {
            fpc_authenticator_destroy(dev->ext_authenticator);
        }
        if (fpc_tee_authenticator_2_enabled(dev->tee_handle)) {
            fpc_authenticator_2_destroy(dev->ext_authenticator_2);
        }
        if (fpc_tee_navigation_enabled(dev->tee_handle)) {
            add_navigation_service(NULL);
            fpc_navigation_destroy(dev->ext_navigation);
        }
        if (fpc_tee_sensetouch_enabled(dev->tee_handle)) {
            fpc_sense_touch_destroy(dev->ext_sensetouch);
        }
    }

    destroy_input_device();
    fpc_worker_destroy(dev->worker);

    fpc_tee_sensor_release(dev->sensor);

    fpc_tee_bio_release(dev->bio);

    fpc_tee_release(dev->tee_handle);

    pthread_mutex_unlock(&dev->lock);
    pthread_mutex_destroy(&dev->lock);
    free(device);
}

int fpc_hal_open(fpc_hal_common_t **device, const fpc_hal_compat_callback_t *callback,
                 void *callback_context)
{
    LOGD("%s", __func__);

    *device = NULL;

    fpc_hal_common_t *dev = (fpc_hal_common_t *) malloc(sizeof(fpc_hal_common_t));

    if (!dev) {
        return -FPC_ERROR_MEMORY;
    }

    memset(dev, 0, sizeof(fpc_hal_common_t));

    dev->callback = callback;
    dev->callback_context = callback_context;
    pthread_mutex_init(&dev->lock, NULL);

    dev->tee_handle = fpc_tee_init();
    if (!dev->tee_handle) {
        goto err;
    }

    if (fpc_tee_log_build_info(dev->tee_handle)) {
        LOGD("%s, An error occured get build info.", __func__);
    }

    dev->sensor = fpc_tee_sensor_init(dev->tee_handle);
    if (!dev->sensor) {
        goto err;
    }

    dev->bio = fpc_tee_bio_init(dev->tee_handle);
    if (!dev->bio) {
        goto err;
    }

    if (fpc_tee_navigation_enabled(dev->tee_handle)) {
        if (create_input_device()) {
            LOGE("%s Failed to create input device.", __func__);
            goto err;
        }
    }

    if (fpc_tee_init_hw_auth(dev->tee_handle)) {
        goto err;
    }

    dev->worker = fpc_worker_new();
    if (!dev->worker) {
        goto err;
    }

    if (fpc_tee_engineering_enabled(dev->tee_handle)) {
        dev->ext_engineering = fpc_engineering_new(dev);
        if (!dev->ext_engineering) {
            goto err;
        }
        add_engineering_service(dev->ext_engineering);
    }

    if (fpc_tee_sensortest_enabled(dev->tee_handle)) {
        dev->ext_sensortest = fpc_sensortest_new(dev);
        if (!dev->ext_sensortest) {
            goto err;
        }
        add_sensortest_service(dev->ext_sensortest);
    }
    if (fpc_tee_authenticator_enabled(dev->tee_handle)) {
        dev->ext_authenticator = fpc_authenticator_new(dev);
        if (!dev->ext_authenticator) {
            goto err;
        }
        add_authenticator_service(dev->ext_authenticator);
    }
    if (fpc_tee_authenticator_2_enabled(dev->tee_handle)) {
        dev->ext_authenticator_2 = fpc_authenticator_2_new(dev);
        if (!dev->ext_authenticator_2) {
            goto err;
        }
        add_authenticator_service_2(dev->ext_authenticator_2);
    }
    if (fpc_tee_sensetouch_enabled(dev->tee_handle)) {
        dev->ext_sensetouch = fpc_sense_touch_new(dev);
        fpc_sense_touch_load_config();
        if (!dev->ext_sensetouch) {
            goto err;
        }
        add_sense_touch_service(dev->ext_sensetouch);
    }
    if (fpc_tee_navigation_enabled(dev->tee_handle)) {
        dev->ext_navigation = fpc_navigation_new(dev);
        if (!dev->ext_navigation) {
            goto err;
        }
        add_navigation_service(dev->ext_navigation);
    }

    *device = dev;

    fingerprint_hal_resume(dev);

    return 0;
err:
    LOGE("%s failed", __func__);
    fpc_hal_close(dev);
    return -1;
}
