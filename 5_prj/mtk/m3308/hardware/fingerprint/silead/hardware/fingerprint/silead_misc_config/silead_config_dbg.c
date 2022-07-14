/******************************************************************************
 * @file   silead_config_dbg.c
 * @brief  Contains dump config functions.
 *
 *
 * Copyright (c) 2016-2021 GigaDevice/Silead Inc.
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * GigaDevice/Silead Inc. You shall not disclose the present software and
 * shall use it only in accordance with the terms of the license agreement
 * you entered into with GigaDevice/Silead Inc. This software may be
 * subject to export or import laws in certain countries.
 *
 *
 * ------------------- Revision History ------------------------------
 * <author>    <date>   <version>     <desc>
 * calvin wang     2018/8/2    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "cfg_dbg"
#include "silead_logmsg.h"

#include "silead_util.h"
#include "silead_xml.h"
#include "silead_cmd.h"
#include "silead_config_upd.h"
#include "silead_config_algo_dump_util.h"

#define ALGO_CFG_UPD_CHUNK_SIZE_MAX (60 * 1024)
#define ALGO_CFG_DUMP_CHUNK_SIZE_MAX (60 * 1024)
#define ALGO_CFG_DUMP_CHUNK_STEP_MAX 20
#define ALGO_CFG_DUMP_CHUNK_SUB_STEP_MAX 100

#ifdef SIL_DUMP_IMAGE

#define CFG_DUMP_PARENT_DIR_NAME "config"
#ifndef SIL_CFG_DUMP_PATH
#define SIL_CFG_DUMP_PATH "/data/vendor/silead/config"
#endif

static char m_str_cfg_dbg_path[MAX_PATH_LEN] = {0};
static const char *_cfg_dbg_get_path(void)
{
    if (m_str_cfg_dbg_path[0] != '\0') {
        return m_str_cfg_dbg_path;
    } else {
        return SIL_CFG_DUMP_PATH;
    }
}

void silfp_ree_cfg_dbg_set_path(const void *path, uint32_t len)
{
    int32_t ret = 0;
    const char *subdir = CFG_DUMP_PARENT_DIR_NAME;

    ret = silfp_ree_util_path_cat(m_str_cfg_dbg_path, sizeof(m_str_cfg_dbg_path), path, len, subdir, strlen(subdir));
    if (ret < 0) {
        memset(m_str_cfg_dbg_path, 0, sizeof(m_str_cfg_dbg_path));
    }
    LOG_MSG_VERBOSE("path = %s", _cfg_dbg_get_path());
}

static void _cfg_dbg_data(uint32_t chipid, uint32_t subid, uint32_t vid)
{
    int32_t ret = 0;
    void *buf = NULL;
    uint32_t len = ALGO_CFG_DUMP_CHUNK_SIZE_MAX;
    uint32_t result = 0;

    char name[MAX_PATH_LEN] = {0};

    buf = malloc(len);
    if (buf == NULL) {
        LOG_MSG_VERBOSE("malloc(%u) fail", len);
        return;
    }

    memset(buf, 0, len);
    ret = silfp_ree_cmd_send_cmd_with_buf_and_get(REQ_CMD_SUB_ID_DBG_CFG, buf, &len, &result);
    if (ret >= 0) {
        snprintf(name, sizeof(name), "%08x-%08x-%08x.cfg", chipid, subid, vid);
        silfp_ree_util_file_save(_cfg_dbg_get_path(), name, buf, len);
    }

    free(buf);
    buf = NULL;
}

static int32_t _algo_cfg_dbg_data(void *buf, uint32_t *len, uint32_t *result)
{
    if ((buf == NULL) || (len == NULL) || (result == NULL)) {
        return -1;
    }

    return silfp_ree_cmd_send_cmd_with_buf_and_get(REQ_CMD_SUB_ID_DBG_ALGO_CFG, buf, len, result);
}

static int32_t _algo_cfg_dbg_data_crypted_save(int32_t fd, char *buf, uint32_t len, uint32_t total_size, uint8_t step, uint8_t sub_step)
{
    uint32_t magic = ALGO_CFG_UPD_MAGIC;

    if ((buf == NULL) || (len == 0) || (total_size < len)) {
        return -1;
    }

    if ((step == 0) && (sub_step == 0)) {
        silfp_ree_util_write_file(fd, &magic, sizeof(magic));
    }

    if (sub_step == 0) {
        silfp_ree_util_write_file(fd, &total_size, sizeof(uint32_t));
    }

    silfp_ree_util_write_file(fd, buf, len);

    return 0;
}

static char *m_algo_dump_data = NULL;
static uint32_t m_algo_dump_data_size = 0;
static uint32_t m_algo_dump_data_offset = 0;
static void* _algo_cfg_dbg_item_normal_setup(uint32_t size)
{
    void *buf = malloc(size);
    if (buf == NULL) {
        LOG_MSG_ERROR("malloc (%d) failed", size);
        return NULL;
    }

    memset(buf, 0, size);
    m_algo_dump_data = (char *)buf;
    m_algo_dump_data_size = size;
    m_algo_dump_data_offset = 0;

    return buf;
}

static int32_t _algo_cfg_dbg_item_normal_setup_data(const void *data, uint32_t size)
{
    if (m_algo_dump_data == NULL) {
        LOG_MSG_ERROR("dump buf invalid");
        return -1;
    }

    if (m_algo_dump_data_size < m_algo_dump_data_offset + size) {
        LOG_MSG_ERROR("dump buf invalid, need %d but %d", m_algo_dump_data_offset + size, m_algo_dump_data_size);
        return -1;
    }

    memcpy(m_algo_dump_data + m_algo_dump_data_offset, data, size);
    m_algo_dump_data_offset += size;

    if (m_algo_dump_data_offset == m_algo_dump_data_size) {
        LOG_MSG_VERBOSE("dump buf full");
        return 1;
    }

    return 0;
}

static void _algo_cfg_dbg_item_normal_unsetup(void)
{
    if (m_algo_dump_data != NULL) {
        free(m_algo_dump_data);
        m_algo_dump_data = NULL;
    }
    m_algo_dump_data_size = 0;
    m_algo_dump_data_offset = 0;
}

static int32_t _algo_cfg_dbg_item(int32_t fd, char *buf, uint32_t size, uint8_t step, uint32_t *presult)
{
    int32_t ret = 0;
    uint32_t buf_len = 0;
    uint32_t result = 0;
    uint32_t sub_step = 0;

    do {
        buf_len = size;
        ret = _algo_cfg_dbg_data(buf, &buf_len, &result);
        if (ret < 0) {
            LOG_MSG_ERROR("get algo dump fail %d", ret);
            break;
        }

        LOG_MSG_VERBOSE("ret = %d, dump_len = %d, result = 0x%08x", ret, buf_len, result);

        if ((result >> 24) & 0xFF) {
            _algo_cfg_dbg_data_crypted_save(fd, buf, buf_len, ret, step, sub_step);
        } else {
            if (sub_step == 0) {
                _algo_cfg_dbg_item_normal_setup(ret);
            }
            ret = _algo_cfg_dbg_item_normal_setup_data(buf, buf_len);
            if (ret < 0) {
                break;
            }
            if (ret > 0) {
                if (result & 0xFF00) {
                    LOG_MSG_ERROR("buf full, but still have sub data");
                    ret = -1;
                } else {
                    ret = silfp_algo_cfg_dump_util_save(fd, m_algo_dump_data, m_algo_dump_data_size);
                }
                break;
            }
        }
        sub_step++;
    } while ((result & 0xFF00) && (sub_step < ALGO_CFG_DUMP_CHUNK_SUB_STEP_MAX));

    if (presult != NULL) {
        *presult = result;
    }

    _algo_cfg_dbg_item_normal_unsetup();
    if (sub_step >= ALGO_CFG_DUMP_CHUNK_SUB_STEP_MAX) {
        LOG_MSG_ERROR("too many sub loops %d", sub_step);
        ret = -1;
    }

    return ret;
}

static int32_t _algo_cfg_dbg(int32_t fd)
{
    int32_t ret = 0;
    char *dump_buf = NULL;
    uint32_t dump_size = 0;
    uint32_t result = 0;
    uint32_t step = 0;

    dump_size = ALGO_CFG_DUMP_CHUNK_SIZE_MAX;
    dump_buf = malloc(dump_size);
    if (dump_buf == NULL) {
        LOG_MSG_ERROR("malloc dump_buf(%d) failed", dump_size);
        return -1;
    }

    do {
        memset(dump_buf, 0, dump_size);
        ret = _algo_cfg_dbg_item(fd, dump_buf, dump_size, step, &result);
        if (ret < 0) {
            break;
        }
        step++;
    } while ((result & 0xFF0000) && (step < ALGO_CFG_DUMP_CHUNK_STEP_MAX));

    if (step >= ALGO_CFG_DUMP_CHUNK_STEP_MAX) {
        LOG_MSG_ERROR("too many loops %d", step);
        ret = -1;
    }
    free(dump_buf);

    return ret;
}

static int32_t _algo_cfg_dbg_all(uint32_t chipid, uint32_t subid, uint32_t vid)
{
    int32_t ret = 0;
    int32_t fd = 0;
    char name[MAX_PATH_LEN] = {0};

    snprintf(name, sizeof(name), "%s/%08x-%08x-%08x_algo.cfg", _cfg_dbg_get_path(), chipid, subid, vid);

    silfp_ree_util_file_remove(name);
    fd = silfp_ree_util_open_file(name, 1);
    if (fd >= 0) {
        ret = _algo_cfg_dbg(fd);
        silfp_ree_util_close_file(fd);
    }

    if (ret < 0) {
        silfp_ree_util_file_remove(name);
    }

    return ret;
}

void silfp_ree_cfg_dbg(uint32_t chipid, uint32_t subid, uint32_t vid)
{
    _cfg_dbg_data(chipid, subid, vid);
    _algo_cfg_dbg_all(chipid, subid, vid);
}

#endif /* !SIL_DUMP_IMAGE */

static int32_t _cfg_update_data(uint32_t id, void *buf, uint32_t len)
{
    UNUSED(id);
    if ((buf == NULL) || (len == 0)) {
        return 0;
    }

    return silfp_ree_cmd_update_cfg(buf, len);
}

static int32_t _cfg_update_algo_data(uint32_t id, void *buf, uint32_t len)
{
    char *pbuf = (char *)buf;
    uint32_t offset = 0;
    uint32_t magic = ALGO_CFG_APPEND_MAGIC;

    if ((pbuf == NULL) || (len == 0)) {
        return -1;
    }

    while (offset < len) {
        if (len <= offset + ALGO_CFG_UPD_CHUNK_SIZE_MAX) {
            _cfg_update_data(id, pbuf + offset, len - offset);
            break;
        }

        _cfg_update_data(id, pbuf + offset, ALGO_CFG_UPD_CHUNK_SIZE_MAX);
        offset += ALGO_CFG_UPD_CHUNK_SIZE_MAX - sizeof(uint32_t) * 2;
        memcpy(pbuf + offset, &len, sizeof(uint32_t));
        memcpy(pbuf + offset + 4, &magic, sizeof(uint32_t));
    }

    return 0;
}

static int32_t _cfg_update_all(void)
{
    xml_upd_item_t* item = NULL;

    item = silfp_ree_xml_util_get_item();
    while(item != NULL) {
        if ((item->buf != NULL) && (item->len > 0)) {
            if (item->id == CFG_UPD_TYPE_NORMAL) {
                _cfg_update_data(item->id, item->buf, item->len);
            } else {
                _cfg_update_algo_data(item->id, item->buf, item->len);
            }
        }
        silfp_ree_xml_util_item_clear(item);
        item = silfp_ree_xml_util_get_item();
    }

    return 0;
}

int32_t silfp_ree_cfg_update(uint32_t chipid, uint32_t subid, uint32_t vid)
{
    int32_t ret = 0;

    ret = silfp_ree_xml_get_sysparams(chipid, subid, vid);
    if (ret >= 0) {
        _cfg_update_all();
    }
    silfp_ree_xml_util_clear_all();

    return ret;
}

