/*
 * Copyright (c) 2016-2021 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

#include <unistd.h>
#include <linux/limits.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <fpc_error.h>
#include <fpc_error_str.h>
#include <fpc_ta_interface.h>

#include "MobiCoreDriverApi.h"

#include <fpc_tac.h>
#include <fpc_log.h>
#include <fpc_sysfs.h>
#include <fpc_types.h>
#include <fpc_tbase_interface.h>
#include <pthread.h>

#define FPC_TA_TIMEOUT 10000
#define TRUSTLET_PATH "/vendor/app/mcRegistry/04010000000000000000000000000000.tlbin"

const char *const fpc_ree_device_alias_file = FPC_REE_DEVICE_ALIAS_FILE;
const char *const fpc_ree_device_name = FPC_REE_DEVICE_NAME;
const char *const fpc_ree_device_path = FPC_REE_DEVICE_PATH;

struct fpc_tac
{
  mcSessionHandle_t tl_session_handle;
  fpc_tbase_msg_t msg;
  int sysfs_fd;
  pthread_mutex_t trans_lock;
};

static size_t _fpc_tac_load_TA_file(const char* pPath, uint8_t** ppContent)
{
    FILE*    pStream;
    size_t   filesize;
    uint8_t* content = NULL;

    /* Open the file */
    pStream = fopen(pPath, "rb");
    if (!pStream)
    {
        goto error;
    }

    if (fseek(pStream, 0L, SEEK_END) != 0)
    {
        goto error;
    }

    filesize = ftell(pStream);
    if (0 >= filesize)
    {
        goto error;
    }

    /* Set the file pointer at the beginning of the file */
    if (fseek(pStream, 0L, SEEK_SET) != 0)
    {
        goto error;
    }

    /* Allocate a buffer for the content */
    content = (uint8_t*)malloc(filesize);
    if (!content) {
          goto error;
    }

    /* Read data from the file into the buffer */
    if (fread(content, filesize, 1, pStream) != 1) {
        goto error;
    }

    /* Close the file */
    fclose(pStream);
    *ppContent = content;

    /* Return number of bytes read */
    return filesize;

error:
    LOGE("%s failed with error: %s", __func__, strerror(errno));
    if (content != NULL) {
        free(content);
    }

    if (pStream != NULL) fclose(pStream);
    return 0;
}

/**
 * Send setup commands to the TA, at the moment only INIT and EXIT is supported
 */
static int fpc_tac_setup_cmd(fpc_tac_t* tac, uint32_t cmd)
{
    mcResult_t ret = 0;

    tac->msg.msg_type = TBASE_MSG_SETUP;
    tac->msg.setup.command = cmd;

    ret = mcNotify(&tac->tl_session_handle);
    if (MC_DRV_OK != ret)
    {
        LOGE("%s Notify failed: %d", __func__, ret);
        goto out;
    }

    ret = mcWaitNotification(&tac->tl_session_handle, FPC_TA_TIMEOUT);
    if (MC_DRV_OK != ret)
    {
        LOGE("%s wait for notification failed: 0x%02X ", __func__, ret);
    }

out:
    if (ret)
    {
        int errorcode = 0;
        ret = mcGetSessionErrorCode(&tac->tl_session_handle, &errorcode);
        if (ret == MC_DRV_OK)
        {
            LOGE("%s ErrorCode: %d", __func__, errorcode);
        } else {
            LOGE("%s failed to get error code", __func__);
        }

        return -FPC_ERROR_IO;
    }

    if(tac->msg.response)
    {
        LOGE("%s, TA returned message error:%d", __func__, tac->msg.response);
        return -FPC_ERROR_IO;
    }

    if(tac->msg.setup.response)
    {
        LOGE("%s, Setup message error: %d", __func__, tac->msg.setup.response);
        return -FPC_ERROR_IO;
    }

    return 0;
}

/**
 * @brief fpc_tac_open open a connection to the ta.
 * @return the tac or NULL on failure.
 */
fpc_tac_t* fpc_tac_open(void)
{
    int ret = 0;
    mcResult_t mc_ret;
    mcVersionInfo_t versionInfo;
    uint8_t* trustlet_data_p = NULL;
    uint32_t trustlet_size;
#ifdef FPC_TA_HW_AUTHENTICATION
    uint8_t* encapsulated_key = NULL;
    uint32_t size_encapsulated_key;
#endif

    fpc_tac_t *tac = (fpc_tac_t*) malloc(sizeof(fpc_tac_t));
    if(!tac)
    {
        ret = -FPC_ERROR_MEMORY;
        goto out;
    }

    memset(tac, 0, sizeof(fpc_tac_t));
    tac->sysfs_fd = -1;
    pthread_mutex_init(&tac->trans_lock, NULL);

    char path[PATH_MAX];
    if (!fpc_sysfs_path_by_attr(
            fpc_ree_device_alias_file,
            fpc_ree_device_name,
            fpc_ree_device_path,
            path,
            PATH_MAX)) {
        LOGE("%s sysfs path not found", __func__);
        ret = -FPC_ERROR_IO;
        goto out;
    }

    tac->sysfs_fd = open(path, O_RDONLY);
    if (tac->sysfs_fd == -1) {
        LOGE("%s open %s failed %i", __func__, path, errno);
        ret = -FPC_ERROR_IO;
        goto out;
    }

    //LOGD("Opening <t-base device " __DATE__ "," __TIME__);
    mc_ret = mcOpenDevice(MC_DEVICE_ID_DEFAULT);
    if (MC_DRV_OK != mc_ret)
    {
        LOGE("Error opening device: %d", mc_ret);
        ret = -FPC_ERROR_IO;
        goto out;
    }

    mc_ret = mcGetMobiCoreVersion(MC_DEVICE_ID_DEFAULT, &versionInfo);
    if (MC_DRV_OK != mc_ret)
    {
        LOGE("mcGetMobiCoreVersion failed %d", mc_ret);
        ret = -FPC_ERROR_IO;
        goto out;
    }

    LOGD("productId        = %s",     versionInfo.productId);
    LOGD("versionMci       = 0x%08X", versionInfo.versionMci);
    LOGD("versionSo        = 0x%08X", versionInfo.versionSo);
    LOGD("versionMclf      = 0x%08X", versionInfo.versionMclf);
    LOGD("versionContainer = 0x%08X", versionInfo.versionContainer);
    LOGD("versionMcConfig  = 0x%08X", versionInfo.versionMcConfig);
    LOGD("versionTlApi     = 0x%08X", versionInfo.versionTlApi);
    LOGD("versionDrApi     = 0x%08X", versionInfo.versionDrApi);
    LOGD("versionCmp       = 0x%08X", versionInfo.versionCmp);
    //LOGD("FPC TAC build  %s %s", __DATE__, __TIME__);


    trustlet_size = _fpc_tac_load_TA_file(TRUSTLET_PATH, &trustlet_data_p);
    if (!trustlet_size)
    {
        LOGE("Trustlet not found " TRUSTLET_PATH);
        ret = -FPC_ERROR_IO;
        goto out;
    }


    LOGD("Opening the session %d", trustlet_size);
    tac->tl_session_handle.deviceId = MC_DEVICE_ID_DEFAULT;
    mc_ret = mcOpenTrustlet(
            &(tac->tl_session_handle),
            MC_SPID_SYSTEM,/// MC_SPID_SYSTEM, /* mcSpid_t */
            trustlet_data_p,
            trustlet_size,
            (uint8_t *) &(tac->msg),
            sizeof(fpc_tbase_msg_t));

    // Whatever the result is, free the buffer
    free(trustlet_data_p);
    trustlet_data_p = NULL;

    if (MC_DRV_OK != mc_ret) {
        LOGE("Open session to the trustlet failed: %d", mc_ret);
        ret = -FPC_ERROR_IO;
        goto out;
    }

    ret = fpc_tac_setup_cmd(tac, TBASE_SETUP_CMD_INIT);
    if (ret) {
        LOGE("failed to send INIT: %d", ret);
        //keep going, we want to turn off the spi clock.
    }

out:
    if (0 > ret) {
        fpc_tac_release(tac);
        tac = NULL;
    }

    LOGD("<--%s returns %d", __func__, ret);
    return tac;

}

/**
 * @brief fpc_tac_release close the connection to the ta.
 */
void fpc_tac_release(fpc_tac_t* tac)
{
    int ret = 0;

    if(tac != NULL) {
        fpc_tac_setup_cmd(tac, TBASE_SETUP_CMD_EXIT);

        pthread_mutex_destroy(&tac->trans_lock);

        LOGD("-->%s Closing the session", __func__);
        ret = mcCloseSession(&tac->tl_session_handle);
        if (MC_DRV_OK != ret)
        {
            LOGE("Closing session to the trustlet failed: %d", ret);
            /* continue even in case of error */
        }
        if (tac->sysfs_fd != -1)
        {
            close(tac->sysfs_fd);
        }
        free(tac);
    }

    LOGD("Closing <t-base device");
    ret = mcCloseDevice(MC_DEVICE_ID_DEFAULT);
    if (MC_DRV_OK != ret)
    {
        LOGE("Closing <t-base device failed: %d", ret);
        /* continue even in case of error */;
    }

    LOGD("<--%s Result: %d", __func__, ret);
}

/* This is set according to Trustonic's suggestion */
#define TBASE_MAX_BLOCK_SIZE (1020 * 1024)

typedef struct
{
    fpc_tac_shared_mem_t shared_mem;
    mcBulkMap_t mem_map;
    fpc_tac_t *tac;
} tbase_mapped_memory_t;

fpc_tac_shared_mem_t* fpc_tac_alloc_shared(fpc_tac_t *tac, uint32_t size)
{
    tbase_mapped_memory_t *tbmm = NULL;

    if(TBASE_MAX_BLOCK_SIZE < size)
    {
        LOGE("Error: size does not fit into tbase map block size");
        goto error;
    }

    tbmm = (tbase_mapped_memory_t *)malloc(sizeof(tbase_mapped_memory_t));
    if(!tbmm)
    {
        LOGE("Error: Failed to alloc metadata for shared memory");
        goto error;
    }
    memset(tbmm, 0, sizeof(tbase_mapped_memory_t));

    tbmm->shared_mem.addr = malloc(size);
    if(!tbmm->shared_mem.addr)
    {
        LOGE("Error: Failed to alloc normalworld buffer (malloc)");
        goto error;
    }

    if(MC_DRV_OK != mcMap(
          &tac->tl_session_handle,
          tbmm->shared_mem.addr,
          size,
          &tbmm->mem_map))
    {
        LOGE("Error: Failed to map buffer to secure world");
        goto error;
    }

    tbmm->tac = tac;

    return &tbmm->shared_mem;

error:
    if(tbmm)
    {
        if(tbmm->mem_map.sVirtualAddr &&
            MC_DRV_OK != mcUnmap(
              &tac->tl_session_handle,
              tbmm->shared_mem.addr,
              &tbmm->mem_map))
        {
            LOGE("Failed to unmap shared memory");
        }

        if(tbmm->shared_mem.addr)
        {
            free(tbmm->shared_mem.addr);
        }

        free(tbmm);
    }
    return NULL;
}

void fpc_tac_free_shared(fpc_tac_shared_mem_t *shared_mem)
{
    tbase_mapped_memory_t *tbmm = (tbase_mapped_memory_t *) shared_mem;
    if (!tbmm) {
        LOGE("%s got NULL pointer!", __func__);
    }

    if (tbmm->mem_map.sVirtualAddr &&
        MC_DRV_OK != mcUnmap(
            &tbmm->tac->tl_session_handle,
            tbmm->shared_mem.addr,
            &tbmm->mem_map)) {
        LOGE("Failed to unmap shared memory");
    }

    if (tbmm->shared_mem.addr) {
        free(tbmm->shared_mem.addr);
    }

    free(tbmm);
}

/**
 * transfer control to the ta, the entire content of the shared buffer shall
 * be treated as both input/output.
 */
int fpc_tac_transfer(fpc_tac_t *tac, fpc_tac_shared_mem_t *shared_buffer)
{
    mcResult_t ret = MC_DRV_OK;
    int32_t errorcode;
    int32_t res = 0, cmd_res = 0;

    pthread_mutex_lock(&tac->trans_lock);
    tbase_mapped_memory_t *tbmm = (tbase_mapped_memory_t *)shared_buffer;

    tac->msg.msg_type = TBASE_MSG_COMMAND;
    tac->msg.command.virtual_addr = (uint32_t)tbmm->mem_map.sVirtualAddr;
    tac->msg.command.virtual_addr_len = tbmm->mem_map.sVirtualLen;
    tac->msg.command.response = 0;

    int status = fpc_sysfs_node_write(tac->sysfs_fd, "clk_enable", "1");
    if (status) {
        LOGE("%s, fpc_sysfs_node_write clk_enable failed: %d", __func__, status);
        goto out;
    }

    ret = mcNotify(&tac->tl_session_handle);
    if (MC_DRV_OK != ret) {
        LOGE("%s Notify failed: %d", __func__, ret);
        goto mc_err;
    }

    ret = mcWaitNotification(&tac->tl_session_handle, FPC_TA_TIMEOUT);
    if (MC_DRV_OK != ret) {
        LOGE("%s wait for response notification failed: 0x%02X ",
             __func__, ret);
        goto mc_err;
    }
    res = tac->msg.response;
    cmd_res = tac->msg.command.response;

mc_err:
    if (ret) {
        ret = mcGetSessionErrorCode(&tac->tl_session_handle, &errorcode);
        if (ret == MC_DRV_OK) {
            LOGE("%s ErrorCode: %d", __func__, errorcode);
        } else {
            LOGE("%s failed to get error code", __func__);
        }
        status = -FPC_ERROR_IO;
    }

out:
    tac->msg.command.virtual_addr = 0;
    tac->msg.command.virtual_addr_len = 0;
    fpc_sysfs_node_write(tac->sysfs_fd, "clk_enable", "0");
    pthread_mutex_unlock(&tac->trans_lock);

    if (status) {
        return -FPC_ERROR_IO;
    }

    if (res) {
        LOGE("%s, TA returned message error:%d", __func__, res);
        return -FPC_ERROR_IO;
    }

    if (FAILED(cmd_res)) {
        LOGE("%s TA returned error: 0x%08" PRIx32 " (%s)", __func__, cmd_res,
             fpc_error_str(cmd_res));
    }
    return cmd_res;
}

