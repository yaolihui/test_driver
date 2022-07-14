/*
 * Copyright (c) 2016-2021 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#include <string.h>

#include "fpc_tac.h"
#include "fpc_log.h"

#include "fpc_tee_internal.h"
#include "fpc_tee_sensortest.h"

#include "fpc_ta_interface.h"
#include "fpc_ta_sensortest_interface.h"
#include "fpc_ta_targets.h"
#include "fpc_tee_sensor.h"
#include "fpc_tee_sensor_internal.h"
#include "fpc_types.h"

#define USECS_PER_MSEC              1000

#define _FPC_SELFTEST_IRQ_FAIL     6
#define _FPC_ERROR_SENSOR          7

#ifndef MIN
#define MIN(A, B) (((A) < (B)) ? (A) : (B))
#endif

int fpc_tee_sensortest_is_test_supported(fpc_tee_sensor_t *sensor,
                                         fpc_sensortest_test_t test,
                                         int32_t *is_supported) {
    int status = 0;
    fpc_tee_t* tee = sensor->tee;
    fpc_ta_sensortest_command_t* command = (fpc_ta_sensortest_command_t*) tee->shared_buffer->addr;

    LOG_ENTER();
    LOGD("%s: %d", __func__, test);

    pthread_mutex_lock(&tee->shared_mem_lock);
    command->header.target = TARGET_FPC_TA_SENSORTEST;
    command->header.command = FPC_TA_SENSORTEST_IS_TEST_SUPPORTED;
    command->is_test_supported.test = test;

    *is_supported = 0;

    status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    pthread_mutex_unlock(&tee->shared_mem_lock);

    if (FAILED(status)) {
        goto out;
    }

    *is_supported = status;
    status = FPC_ERROR_NONE;

out:
    LOG_LEAVE_TRACE(status);

    return status;
}

static int fpc_tee_sensortest_test_command(fpc_tee_sensor_t* sensor, int32_t command_id, uint32_t *result)
{
    LOG_ENTER();

    fpc_tee_t* tee = sensor->tee;
    fpc_ta_sensortest_command_t* command = (fpc_ta_sensortest_command_t*) tee->shared_buffer->addr;

    pthread_mutex_lock(&tee->shared_mem_lock);
    memset(command, 0, sizeof(*command));
    command->header.command   = command_id;
    command->header.target    = TARGET_FPC_TA_SENSORTEST;
    command->test.result = *result; //send normal side irq-pin status to secure side
    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    *result = command->test.result;
    pthread_mutex_unlock(&tee->shared_mem_lock);

    LOG_LEAVE_TRACE(status);

    return status;
}

static int fpc_tee_sensortest_run_test_command(fpc_tee_sensor_t* sensor,
                                               fpc_sensortest_test_t test,
                                               fpc_ta_sensortest_test_params_t *params,
                                               uint32_t *image_captured,
                                               uint32_t *result,
                                               uint32_t *log_size)
{
    fpc_tee_t* tee = sensor->tee;
    fpc_ta_sensortest_command_t* command = (fpc_ta_sensortest_command_t*) tee->shared_buffer->addr;

    LOG_ENTER();

    pthread_mutex_lock(&tee->shared_mem_lock);
    memset(command, 0, sizeof(*command));
    command->header.command   = FPC_TA_SENSORTEST_RUN_TEST;
    command->header.target    = TARGET_FPC_TA_SENSORTEST;
    command->test.test        = test;
    command->test.params      = *params;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    *params         = command->test.params;
    *image_captured = command->test.image_captured;
    *result         = command->test.result;
    *log_size       = command->test.log_size;
    pthread_mutex_unlock(&tee->shared_mem_lock);

    LOG_LEAVE_TRACE(status);

    return status;
}

static int fpc_tee_sensortest_self_test(fpc_tee_sensor_t *sensor, uint32_t *result)
{
    int status;
    uint32_t temp_result = 0;

    LOG_ENTER();

    status = fpc_tee_sensortest_test_command(sensor, FPC_TA_SENSORTEST_SELF_TEST_INIT, &temp_result);
    if (status) {
        LOGE("%s FPC_TA_SENSORTEST_SELF_TEST_INIT failed! Status = %d", __func__, status);
        status = _FPC_ERROR_SENSOR;
        goto clean;
    }

    // Status is the value of irq pin, we expect 0 here after the init has been called
    status = fpc_tee_status_irq(sensor);
    if (status < 0) {
        status = _FPC_ERROR_SENSOR;
        LOGE("%s IRQ TEST READ INITIAL -> FPC_ERROR_SENSOR", __func__);
        goto clean;
    } else if (status > 0) {
        LOGE("%s IRQ TEST READ INITIAL -> FPC_SELFTEST_IRQ_FAIL pin = %d should be 0",
             __func__, status);
        *result = _FPC_SELFTEST_IRQ_FAIL;
        status = FPC_ERROR_NONE;
        goto clean;
    }

    // Do selftest including irq tests on secure-side
    status = fpc_tee_sensortest_test_command(sensor, FPC_TA_SENSORTEST_SELF_TEST, result);

    // If failure on TA side skip REE side checks they will give nothing as we don't know IRQ status
    if (*result || status) {
        LOGE("%s Selftest failed status = %d result = %d", __func__, status, *result);
        goto clean;
    }

    // After selftest is executed on TA we expect an active IRQ -> pin high
    status = fpc_tee_status_irq(sensor);
    if (status < 0) {
        status = _FPC_ERROR_SENSOR;
        LOGE("%s IRQ TEST READ AFTER -> FPC_ERROR_SENSOR", __func__);
        goto clean;
    } else if (status == 0) {
        LOGE("%s IRQ TEST READ AFTER -> FPC_SELFTEST_IRQ_FAIL pin = %d should be > 0",
             __func__, status);
        *result = _FPC_SELFTEST_IRQ_FAIL;
        status = FPC_ERROR_NONE;
        goto clean;
    }

    if (fpc_tee_sensortest_test_command(sensor, FPC_TA_SENSORTEST_SELF_TEST_CLEANUP, &temp_result)) {
        LOGE("%s FPC_TA_SENSORTEST_SELF_TEST_CLEANUP failed", __func__);
        status = _FPC_ERROR_SENSOR;
        goto out;
    }

    // After cleanup selftest is executed on TA we expect no active IRQ -> pin low
    status = fpc_tee_status_irq(sensor);
    if (status < 0) {
        status = _FPC_ERROR_SENSOR;
        LOGE("%s IRQ TEST READ END -> FPC_ERROR_SENSOR", __func__);
        goto clean;
    } else if (status > 0) {
        LOGE("%s IRQ TEST READ END -> FPC_SELFTEST_IRQ_FAIL pin = %d should be 0",
             __func__, status);
        *result = _FPC_SELFTEST_IRQ_FAIL;
        status = FPC_ERROR_NONE;
        goto clean;
    }

    goto out;

clean:
    if (fpc_tee_sensortest_test_command(sensor, FPC_TA_SENSORTEST_SELF_TEST_CLEANUP, &temp_result)) {
        LOGE("%s FPC_TA_SENSORTEST_SELF_TEST_CLEANUP failed, ignoring", __func__);
    }

out:
    LOG_LEAVE_TRACE(status);

    return status;
}

int fpc_tee_sensortest_run_test(fpc_tee_sensor_t *sensor,
                                fpc_sensortest_test_t test,
                                uint32_t stabilization_time_ms,
                                fpc_ta_sensortest_test_params_t *params,
                                uint32_t *image_captured,
                                uint32_t *result,
                                uint32_t *log_size)
{
    int status = 0;
    *log_size = 0;
    int finger_lost;

    LOG_ENTER();
    LOGD("%s test: %d", __func__, test);

    if (test == FPC_SENSORTEST_MODULE_QUALITY_TEST) {
        //trigger dead pixel update before waiting finger down.
        fpc_tee_check_finger_lost(sensor);
    }

    if (stabilization_time_ms) {
        status = fpc_tee_wait_finger_down(sensor);
        if (status) {
            LOGE("%s fpc_tee_wait_finger_down failed %d", __func__, status);
            goto out;
        }
        usleep(stabilization_time_ms * USECS_PER_MSEC);
    }

    switch (test) {
    case FPC_SENSORTEST_SELF_TEST:
        *image_captured = 0;
        status = fpc_tee_sensortest_self_test(sensor, result);
        break;
    case FPC_SENSORTEST_CHECKERBOARD_TEST:
    case FPC_SENSORTEST_DEFECTIVE_PIXELS_TEST:
        finger_lost = fpc_tee_check_finger_lost(sensor);
        if (!finger_lost) {
            LOGE("%s: Please move off your finger from sensor!", __func__);
            status = -FPC_ERROR_TEST_FAILED;
            break;
        }
        /* FALLTHROUGH */
    default:
        status = fpc_tee_sensortest_run_test_command(sensor,
                                                     test,
                                                     params,
                                                     image_captured,
                                                     result,
                                                     log_size);
        break;
    }

out:
    LOG_LEAVE_TRACE(status);

    return status;
}

int fpc_tee_sensortest_capture_uncalibrated(fpc_tee_sensor_t *sensor)
{
    LOG_ENTER();

    int result = FPC_ERROR_NONE;
    fpc_tee_t *tee = sensor->tee;
    fpc_ta_sensortest_command_t *command = (fpc_ta_sensortest_command_t *) tee->shared_buffer->addr;

    pthread_mutex_lock(&tee->shared_mem_lock);
    command->header.command   = FPC_TA_SENSORTEST_CAPTURE_UNCALIBRATED;
    command->header.target    = TARGET_FPC_TA_SENSORTEST;
    result = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    pthread_mutex_lock(&tee->shared_mem_lock);
    return result;
}

int fpc_tee_sensortest_get_log(fpc_tee_sensor_t *sensor, uint32_t *log_size, uint8_t *log_buffer)
{
    int result = FPC_ERROR_NONE;
    uint32_t payload_size = MIN(*log_size, MAX_CHUNK);
    uint32_t remaining_size = *log_size;
    const uint32_t message_size = payload_size + sizeof(fpc_ta_sensortest_command_t);
    fpc_tac_shared_mem_t *const shared_buffer = fpc_tac_alloc_shared(sensor->tee->tac, message_size);
    uint32_t buffer_offset = 0;


    LOG_ENTER();

    if (!shared_buffer) {
        return -FPC_ERROR_MEMORY;
    }

    if (!fpc_tee_engineering_enabled(sensor->tee)) {
        result = -FPC_ERROR_NOT_SUPPORTED;
        goto exit;
    }

    fpc_ta_sensortest_command_t *const command = shared_buffer->addr;
    command->header.target    = TARGET_FPC_TA_SENSORTEST;
    command->header.command   = FPC_TA_SENSORTEST_GET_LOG;

    LOGD("%s: Log size: %u bytes", __func__, remaining_size);

    do {
        payload_size = MIN(remaining_size, MAX_CHUNK);
        command->get_log.size = payload_size;

        /* Get data from TEE */
        result = fpc_tac_transfer(sensor->tee->tac, shared_buffer);
        if (FAILED(result)) {
            LOGE("%s: Failed to get %u bytes of data: Result: %d", __func__,
                    remaining_size, result);
            break;
        }
        remaining_size -= payload_size;

        /* TEE side returns the remaining number of bytes */
        if (remaining_size != (uint32_t)result) {
            LOGE("%s: REE and TEE out of sync (%u vs %d): Abort transfer ", __func__,
                    remaining_size, result);
            result = -FPC_ERROR_STATE;
            break;
        }

        LOGD("%s: Transfer with offset: %d", __func__, buffer_offset);

        memcpy(log_buffer + buffer_offset, command->get_log.array, payload_size);
        buffer_offset += payload_size;
    } while (remaining_size);

    LOGD("%s: Transfer complete.", __func__);

exit:
    fpc_tac_free_shared(shared_buffer);

    LOG_LEAVE_TRACE(result);

    return result;
}
