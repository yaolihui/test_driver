/*
 * Copyright (c) 2015-2021 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "fpc_log.h"
#include "fpc_tee.h"
#include "fpc_tee_internal.h"
#include "fpc_ta_navigation_interface.h"
#include "fpc_types.h"
#include "fpc_tee_nav.h"
#include "fpc_ta_targets.h"
#include "fpc_nav_types.h"

int fpc_tee_nav_poll_data(fpc_tee_t* tee, fpc_nav_data_t* data)
{

    fpc_ta_nav_poll_data_cmd_t* command =
            (fpc_ta_nav_poll_data_cmd_t*) tee->shared_buffer->addr;

    pthread_mutex_lock(&tee->shared_mem_lock);
    command->header.command = FPC_TA_NAVIGATION_POLL_DATA;
    command->header.target = TARGET_FPC_TA_NAVIGATION;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (!status) {
        data->finger_down = command->finger_down;
        data->nav_event = command->nav_event;
        data->request = command->request;
        data->force = command->force;
        data->vec = command->vec;
    }

    pthread_mutex_unlock(&tee->shared_mem_lock);

    return status;
}

int fpc_tee_nav_set_config(fpc_tee_t* tee, const fpc_nav_config_t* config)
{
    int result = FPC_ERROR_NONE;
    fpc_ta_nav_config_cmd_t* command =
            (fpc_ta_nav_config_cmd_t*) tee->shared_buffer->addr;

    pthread_mutex_lock(&tee->shared_mem_lock);
    command->header.command = FPC_TA_NAVIGATION_SET_CONFIG;
    command->header.target = TARGET_FPC_TA_NAVIGATION;
    command->config = *config;

    result = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    pthread_mutex_unlock(&tee->shared_mem_lock);
    return result;
}

int fpc_tee_nav_get_config(fpc_tee_t* tee, fpc_nav_config_t* config)
{
    fpc_ta_nav_config_cmd_t* command =
            (fpc_ta_nav_config_cmd_t*) tee->shared_buffer->addr;

    pthread_mutex_lock(&tee->shared_mem_lock);
    command->header.command = FPC_TA_NAVIGATION_GET_CONFIG;
    command->header.target = TARGET_FPC_TA_NAVIGATION;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (!status) {
        *config = command->config;
    }

    pthread_mutex_unlock(&tee->shared_mem_lock);

    return status;
}

int fpc_tee_nav_init(fpc_tee_t* tee)
{
    int result = FPC_ERROR_NONE;
    fpc_ta_navigation_command_t* command =
            (fpc_ta_navigation_command_t*) tee->shared_buffer->addr;

    pthread_mutex_lock(&tee->shared_mem_lock);
    command->header.command = FPC_TA_NAVIGATION_INIT;
    command->header.target = TARGET_FPC_TA_NAVIGATION;

    result = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    pthread_mutex_unlock(&tee->shared_mem_lock);
    return result;
}

int fpc_tee_nav_exit(fpc_tee_t* tee)
{
    int result = FPC_ERROR_NONE;
    fpc_ta_navigation_command_t* command =
            (fpc_ta_navigation_command_t*) tee->shared_buffer->addr;

    pthread_mutex_lock(&tee->shared_mem_lock);
    command->header.command = FPC_TA_NAVIGATION_EXIT;
    command->header.target = TARGET_FPC_TA_NAVIGATION;

    result = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    pthread_mutex_unlock(&tee->shared_mem_lock);
    return result;
}

int fpc_tee_nav_get_debug_buffer_size(fpc_tee_t *tee, uint32_t *debug_buffer_size)
{
    fpc_ta_navigation_command_t* command =
            (fpc_ta_navigation_command_t*) tee->shared_buffer->addr;

    pthread_mutex_lock(&tee->shared_mem_lock);
    command->header.command = FPC_TA_NAVIGATION_GET_DEBUG_BUFFER_SIZE;
    command->header.target = TARGET_FPC_TA_NAVIGATION;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (!status) {
        *debug_buffer_size = command->get_debug_buffer_size.arg_1;
    }

    pthread_mutex_unlock(&tee->shared_mem_lock);

    return status;
}

int fpc_tee_nav_get_debug_buffer(fpc_tee_t *tee, uint8_t *debug_buffer, uint32_t *debug_buffer_size)
{
    int status = 0;
    fpc_ta_byte_array_msg_t *command = NULL;

    fpc_tac_shared_mem_t *shared_ipc_buffer =
        fpc_tac_alloc_shared(tee->tac, *debug_buffer_size + sizeof(fpc_ta_byte_array_msg_t));

    if (!shared_ipc_buffer) {
        status = -FPC_ERROR_MEMORY;
        LOGE("%s: failed to allocate error buffer ret %d", __func__, status);
        goto exit;
    }

    memset(shared_ipc_buffer->addr, 0, *debug_buffer_size + sizeof(fpc_ta_byte_array_msg_t));

    command = shared_ipc_buffer->addr;
    command->header.target = TARGET_FPC_TA_NAVIGATION;
    command->header.command = FPC_TA_NAVIGATION_GET_DEBUG_BUFFER;
    command->size = *debug_buffer_size;

    status = fpc_tac_transfer(tee->tac, shared_ipc_buffer);
    if (status) {
        goto exit;
    }

    if (command->size > 0 && command->size <= *debug_buffer_size) {
        *debug_buffer_size = command->size;
        memcpy(debug_buffer, command->array, command->size);
        LOGD("%s: got %d size of debug information data", __func__, command->size);
    }
exit:
    if (shared_ipc_buffer) {
        fpc_tac_free_shared(shared_ipc_buffer);
    }
    return status;
}
