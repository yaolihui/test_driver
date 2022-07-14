/*
 * Copyright (c) 2016-2021 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "fpc_tac.h"
#include "fpc_log.h"

#include "fpc_tee_internal.h"
#include "fpc_tee_engineering.h"

#include "fpc_ta_interface.h"
#include "fpc_ta_engineering_interface.h"
#include "fpc_ta_targets.h"
#include "fpc_types.h"
#include "fpc_sysfs.h"

#ifndef MIN
#define MIN(A, B) (((A) < (B)) ? (A) : (B))
#endif

/**
 * @brief Inject a raw image to the TA.
 *
 * @param[in]   tee             tee handle.
 * @param[in]   buffer          Buffer to be transferred,
 * @param[in]   buffer_size     Size of the buffer.
 * @return      int             0 on success, otherwise fail.
 */
static int inject_raw_image(fpc_tee_t *tee,
                            const uint8_t *buffer,
                            size_t buffer_size)
{

    int result = FPC_ERROR_NONE;
    const uint32_t payload_size = MIN(buffer_size, MAX_CHUNK);
    const uint32_t message_size = payload_size + sizeof(fpc_ta_engineering_command_t);
    fpc_tac_shared_mem_t *const shared_buffer = fpc_tac_alloc_shared(tee->tac, message_size);
    uint32_t buffer_offset = 0;
    uint32_t remaining_size = buffer_size;

    LOGD("%s", __func__);

    if (!shared_buffer) {
        result = -FPC_ERROR_MEMORY;
        goto exit;
    }

    fpc_ta_engineering_command_t *const command = shared_buffer->addr;
    command->header.target        = TARGET_FPC_TA_ENGINEERING;
    command->header.command       = FPC_TA_ENGINEERING_INJECT_RAW;

    /* Send inject commands until payload is transfered completely */
    while (remaining_size) {
        const uint32_t remaining_payload_size = MIN(MAX_CHUNK, remaining_size);
        command->inject.size = remaining_payload_size;
        memcpy(command->inject.array, &buffer[buffer_offset], remaining_payload_size);

        result = fpc_tac_transfer(tee->tac, shared_buffer);
        if (FAILED(result)) {
            LOGE("%s - Failed to inject %u bytes of data - Result: %d", __func__,
                 remaining_payload_size, result);
            goto free_shared;
        }

        remaining_size -= remaining_payload_size;
        buffer_offset += remaining_payload_size;

        /* TEE side returns the remaining number of bytes */
        if (remaining_size != (uint32_t)result) {
            LOGE("%s - REE and TEE out of sync (%u vs %d) - Abort transfer ", __func__,
                 remaining_size, result);
            result = -FPC_ERROR_STATE;
            goto free_shared;
        }
    }

free_shared:
    fpc_tac_free_shared(shared_buffer);
exit:
    return result;
}

int fpc_tee_debug_disable_check_pixels(fpc_tee_t *tee)
{
    LOGD("%s", __func__);

    fpc_ta_engineering_command_t *command =
        (fpc_ta_engineering_command_t *)tee->shared_buffer->addr;
    pthread_mutex_lock(&tee->shared_mem_lock);
    memset(command, 0, sizeof(*command));

    command->header.target  = TARGET_FPC_TA_ENGINEERING;
    command->header.command = FPC_TA_ENGINEERING_DISABLE_CHECK_PIXELS;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    pthread_mutex_unlock(&tee->shared_mem_lock);
    return status;
}

int fpc_tee_debug_get_retrieve_size(
    fpc_tee_t *tee,
    const uint32_t type,
    uint32_t  *raw_size)
{
    LOGD("%s", __func__);

    fpc_ta_engineering_command_t *command =
        (fpc_ta_engineering_command_t *) tee->shared_buffer->addr;
    pthread_mutex_lock(&tee->shared_mem_lock);
    memset(command, 0, sizeof(*command));

    command->header.target  = TARGET_FPC_TA_ENGINEERING;
    command->header.command = FPC_TA_ENGINEERING_GET_RETRIEVE_SIZE;
    command->retrieve.retrieve_type = type;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    *raw_size = command->raw_size.size;
    pthread_mutex_unlock(&tee->shared_mem_lock);
    return status;
}

int fpc_tee_debug_retrieve(
    fpc_tee_t *const tee,
    const uint32_t type,
    uint8_t *const buffer,
    const uint32_t buffer_size)
{
    int result = FPC_ERROR_NONE;
    uint32_t payload_size = MIN(buffer_size, MAX_CHUNK);
    const uint32_t message_size = payload_size + sizeof(fpc_ta_engineering_command_t);
    fpc_tac_shared_mem_t *const shared_buffer = fpc_tac_alloc_shared(tee->tac, message_size);
    uint32_t buffer_offset = 0;
    uint32_t remaining_size = buffer_size;

    LOGD("%s", __func__);

    if (!shared_buffer) {
        result = -FPC_ERROR_MEMORY;
        goto exit;
    }

    fpc_ta_engineering_command_t *const command = shared_buffer->addr;
    command->header.target  = TARGET_FPC_TA_ENGINEERING;
    command->header.command = FPC_TA_ENGINEERING_RETRIEVE;
    command->retrieve.retrieve_type  = type;

    do {
        payload_size = MIN(remaining_size, MAX_CHUNK);
        command->retrieve.size = payload_size;

        /* Get data from TEE */
        result = fpc_tac_transfer(tee->tac, shared_buffer);
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

        memcpy(buffer + buffer_offset, command->retrieve.array, payload_size);
        buffer_offset += payload_size;
    } while (remaining_size);

exit:
    fpc_tac_free_shared(shared_buffer);
    return result;
}

int fpc_tee_debug_inject(fpc_tee_t *tee,
                         const uint32_t inject_type,
                         const uint8_t *buffer,
                         size_t buffer_size)
{
    int ret = FPC_ERROR_NONE;

    LOGD("%s", __func__);

    fpc_ta_engineering_command_t *command =
        (fpc_ta_engineering_command_t *) tee->shared_buffer->addr;
    pthread_mutex_lock(&tee->shared_mem_lock);
    memset(command, 0, sizeof(*command));

    command->header.target      = TARGET_FPC_TA_ENGINEERING;
    command->header.command     = FPC_TA_ENGINEERING_SET_INJECT_SIZE;
    command->raw_size.size      = buffer_size;
    command->inject.inject_type = inject_type;

    ret = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    pthread_mutex_unlock(&tee->shared_mem_lock);
    if (FAILED(ret)) {
        LOGE("%s - Failed to set blob size to TEE - Result: %d", __func__, ret);
        return ret;
    }

    return inject_raw_image(tee, buffer, buffer_size);
}

int fpc_tee_get_sensor_info(
    fpc_tee_t *tee,
    uint16_t *width,
    uint16_t *height)
{
    LOGD("%s", __func__);

    fpc_ta_engineering_command_t *command =
        (fpc_ta_engineering_command_t *) tee->shared_buffer->addr;
    pthread_mutex_lock(&tee->shared_mem_lock);
    memset(command, 0, sizeof(*command));

    command->header.command = FPC_TA_ENGINEERING_GET_SENSOR_INFO;
    command->header.target  = TARGET_FPC_TA_ENGINEERING;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    *width  = command->sensor_info.width;
    *height = command->sensor_info.height;
    pthread_mutex_unlock(&tee->shared_mem_lock);

    return status;
}

int fpc_tee_get_enhanced_image_size(
    fpc_tee_t* tee,
    uint16_t *const width,
    uint16_t *const height)
{
    LOGD("%s", __func__);

    fpc_ta_engineering_command_t *const command = tee->shared_buffer->addr;
    pthread_mutex_lock(&tee->shared_mem_lock);
    memset(command, 0, sizeof(*command));

    command->header.target  = TARGET_FPC_TA_ENGINEERING;
    command->header.command = FPC_TA_ENGINEERING_GET_RETRIEVE_SIZE;
    command->retrieve.retrieve_type = ENG_ENHANCED_IMAGE;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);

    *width  = command->enhanced_image_size.width;
    *height = command->enhanced_image_size.height;
    pthread_mutex_unlock(&tee->shared_mem_lock);

    return status;
}
