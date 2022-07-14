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
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "fpc_ta_interface.h"
#include "fpc_tee.h"
#include "fpc_tac.h"
#include "fpc_tee_internal.h"
#include "fpc_log.h"
#include "fpc_types.h"
#include "fpc_error_str.h"
#include "fpc_ta_targets.h"
#include "fpc_tee_bio.h"
#include "fpc_tee_bio_internal.h"
#include "fpc_sysfs.h"

#define MIN(A, B) ((A) < (B) ? (A) : (B))

static int get_db_blob_size(fpc_tee_t* tee, size_t *blob_size)
{
    int status = 0;

    fpc_ta_bio_command_t* command = tee->shared_buffer->addr;
    pthread_mutex_lock(&tee->shared_mem_lock);
    command->header.target    = TARGET_FPC_TA_DB_BLOB;
    command->header.command   = FPC_TA_BIO_GET_DB_SIZE_CMD;
    command->get_db_size.size = 0;

    status = fpc_tac_transfer(tee->tac, tee->shared_buffer);

    *blob_size = command->get_db_size.size;
    pthread_mutex_unlock(&tee->shared_mem_lock);
    return status;
}

static int db_open(fpc_tee_t *tee, uint32_t mode, uint32_t size)
{
    int result;
    fpc_ta_bio_command_t* command = tee->shared_buffer->addr;

    pthread_mutex_lock(&tee->shared_mem_lock);
    command->header.target  = TARGET_FPC_TA_DB_BLOB;
    command->header.command = FPC_TA_BIO_DB_OPEN_CMD;
    command->db_open.mode   = mode;
    command->db_open.size   = size;

    result = fpc_tac_transfer(tee->tac, tee->shared_buffer);

    if (FAILED(result)) {
        LOGE("%s Failed to complete command", __func__);
    }
    pthread_mutex_unlock(&tee->shared_mem_lock);
    return result;
}

static int db_close(fpc_tee_t *tee)
{
    int result;
    fpc_ta_bio_command_t* command = tee->shared_buffer->addr;

    pthread_mutex_lock(&tee->shared_mem_lock);
    command->header.target  = TARGET_FPC_TA_DB_BLOB;
    command->header.command = FPC_TA_BIO_DB_CLOSE_CMD;

    result = fpc_tac_transfer(tee->tac, tee->shared_buffer);

    if (FAILED(result)) {
        LOGE("%s Failed to complete command", __func__);
    }
    pthread_mutex_unlock(&tee->shared_mem_lock);
    return result;
}

static int send_db_read_commands(fpc_tee_t* tee, uint8_t *blob, size_t blob_size)
{
    int result = 0;
    const uint32_t payload_size = MIN(blob_size, MAX_CHUNK);
    const uint32_t message_size = payload_size + sizeof(fpc_ta_bio_command_t);
    fpc_tac_shared_mem_t* shared_buffer = fpc_tac_alloc_shared(tee->tac, message_size);

    if (!shared_buffer) {
        return -FPC_ERROR_MEMORY;
    }

    fpc_ta_bio_command_t* command = shared_buffer->addr;
    command->header.target        = TARGET_FPC_TA_DB_BLOB;
    command->header.command       = FPC_TA_BIO_DB_READ_CMD;
    command->db_read.size         = payload_size;

    uint32_t blob_offset    = 0;
    uint32_t remaining_size = blob_size;

    // Send read commands until complete payload is transfered
    while (remaining_size) {
        const uint32_t remaining_payload_size = MIN(MAX_CHUNK, remaining_size);

        result = fpc_tac_transfer(tee->tac, shared_buffer);
        if (FAILED(result)) {
            goto free_shared;
        }

        remaining_size -= remaining_payload_size;

        // TEE side returns the remaining number of bytes
        if (remaining_size != (uint32_t)result) {
            LOGE("%s - REE and TEE out of sync (%u vs %d). Abort transfer ", __func__, remaining_size, result);
            result = -FPC_ERROR_STATE;
            goto free_shared;
        }

        memcpy(&blob[blob_offset], command->db_read.array, remaining_payload_size);
        blob_offset += remaining_payload_size;
    }

free_shared:
    fpc_tac_free_shared(shared_buffer);
    return result;
}

static int send_db_write_commands(fpc_tee_t* tee, uint8_t *file_buffer, size_t file_buffer_size)
{
    int result = 0;
    fpc_tac_shared_mem_t* shared_buffer = NULL;

    // Setup write message
    const uint32_t payload_size = MIN(file_buffer_size, MAX_CHUNK);
    const uint32_t message_size = payload_size + sizeof(fpc_ta_bio_command_t);
    shared_buffer = fpc_tac_alloc_shared(tee->tac, message_size);

    if (!shared_buffer) {
        return -FPC_ERROR_MEMORY;
    }

    fpc_ta_bio_command_t* command = shared_buffer->addr;
    command->header.target        = TARGET_FPC_TA_DB_BLOB;
    command->header.command       = FPC_TA_BIO_DB_WRITE_CMD;
    command->db_write.size        = payload_size;

    uint32_t remaining_size = file_buffer_size;
    uint32_t payload_offset = 0;

    // Send write commands until complete payload is transfered
    while (remaining_size) {
        uint32_t remaining_payload_size = MIN(MAX_CHUNK, remaining_size);
        memcpy(command->db_write.array, &file_buffer[payload_offset], remaining_payload_size);

        result = fpc_tac_transfer(tee->tac, shared_buffer);
        if (FAILED(result)) {
            LOGE("%s - Failed to write %u bytes of data. Result %d", __func__, remaining_payload_size, result);
            goto free_shared;
        }

        remaining_size -= remaining_payload_size;
        payload_offset += remaining_payload_size;

        // TEE side returns the remaining number of bytes
        if (remaining_size != (uint32_t)result) {
            LOGE("%s - REE and TEE out of sync (%u vs %d). Abort transfer ", __func__, remaining_size, result);
            result = -1;
            goto free_shared;
        }
    }

free_shared:
    fpc_tac_free_shared(shared_buffer);
    return result;
}

int fpc_tee_store_template_db_host(fpc_tee_bio_t *bio, const char *path)
{
    fpc_tee_t* tee = &bio->tee;
    char temp_path[PATH_MAX];
    int result = FPC_ERROR_NONE;
    size_t blob_size = 0;

    LOG_ENTER();

    if (strlen(path) >= PATH_MAX) {
        LOGE("%s input path too long", __func__);
        result = -FPC_ERROR_PARAMETER;
        goto exit;
    }

    strcpy(temp_path, path);
    char *dir = dirname(temp_path);

    result = mkdir(dir, 0700);
    if (result && (errno != EEXIST)) {
        LOGE("%s - mkdir(%s) failed with error %s", __func__, dir, strerror(errno));
        result = -FPC_ERROR_IO;
        goto exit;
    }

    if (snprintf(temp_path, PATH_MAX, "%s.bak", path) >= PATH_MAX) {
        LOGE("%s input:path too long", __func__);
        result = -FPC_ERROR_PARAMETER;
        goto exit;
    }

    result = get_db_blob_size(tee, &blob_size);
    if (result < 0) {
        goto exit;
    } else if (blob_size == 0) {
        result = -FPC_ERROR_IO;
        goto exit;
    }

    // If open fails ensure that we always close. REE/TEE could get out of sync
    // if the fingerprint daemon crashes.
    result = db_open(tee, FPC_TA_BIO_DB_RDONLY, blob_size);
    if (result < 0) {
        LOGE("%s - transfer_open failed with %d", __func__, result);
        goto close;
    }

    uint8_t *blob = malloc(blob_size);
    if (!blob) {
        result = -FPC_ERROR_MEMORY;
        goto close;
    }

    // Transfer encrypted content from TA (chunk if necessary)
    result = send_db_read_commands(tee, blob, blob_size);
    if (result < 0) {
        goto free;
    }

    result = fpc_write_blob_to_file(temp_path, blob, blob_size);
    if (result < 0) {
        goto free;
    }

    if (rename(temp_path, path) < 0) {
        LOGE("%s - rename failed with error %s", __func__, strerror(errno));
        (void)unlink(temp_path);
        result = -FPC_ERROR_IO;
        goto free;
    }

free:
    if (blob != NULL) {
        free(blob);
    }
close:
    db_close(tee);
exit:
    LOG_LEAVE_TRACE(result);
    return result;
}

int fpc_tee_load_template_db_host(fpc_tee_bio_t *bio, const char *path)
{
    fpc_tee_t* tee = &bio->tee;
    size_t file_size = 0;
    int result = FPC_ERROR_NONE;

    LOG_ENTER();

    LOGD("%s: Database path: %s", __func__, path);

    result = fpc_get_file_size(path, &file_size);
    if (result < 0) {
        goto exit;
    }

    if (file_size == 0) {
        LOGD("%s: Loading empty database", __func__);
        result = fpc_tee_load_empty_db(bio);
        goto exit;
    }

    // If open fails ensure that we always close. REE/TEE could get out of sync
    // if the fingerprint daemon crashes.
    result = db_open(tee, FPC_TA_BIO_DB_WRONLY, file_size);
    if (result < 0) {
        LOGE("Failed to open transfer in write mode with %zu bytes of payload", file_size);
        goto close;
    }

    uint8_t *file_buffer = malloc(file_size);
    if (!file_buffer) {
        result = -FPC_ERROR_MEMORY;
        goto close;
    }

    result = fpc_read_blob_from_file(path, file_buffer, file_size);
    if (result < 0) {
        goto free;
    }

    // Transfer encrypted content to TA (chunk if necessary)
    result = send_db_write_commands(tee, file_buffer, file_size);
    if (result < 0) {
        goto free;
    }

free:
    if (file_buffer != NULL) {
        free(file_buffer);
    }
close:
    db_close(tee);
exit:
    LOG_LEAVE_TRACE(result);
    return result;
}
