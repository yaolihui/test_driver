/******************************************************************************
 * @file   silead_dump.c
 * @brief  Contains dump image functions.
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
 * calvin wang 2018/1/2   0.1.0      Init version
 * Ashely Li   2021/1/15  0.2.0      Update copyright notice
 *
 *****************************************************************************/

#define FILE_TAG "dump"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>

#include "silead_const.h"
#include "silead_bmp.h"
#include "silead_cmd.h"
#include "silead_dump_dbg.h"
#include "silead_dump_name.h"
#include "silead_dump_path.h"
#include "silead_dump_mraw.h"
#include "silead_dump.h"

#ifdef SIL_DUMP_IMAGE

#define DUMP_STEP_MAX 70
#define DUMP_EXT_SIZE_MAX 512
#define DUMP_EXT_INFO_SIZE_MAX (3 * 1024)

void __attribute__((weak)) silfp_dump_data_v1(int32_t type, void *buf, uint32_t buf_size, uint32_t name_mode);
static uint8_t m_test_dump_version = DUMP_VER_V1;
static void *m_test_dump_buffer = NULL;
static uint32_t m_test_dump_buffer_size = 0;

static uint32_t m_dump_name_mode = 0;
void silfp_dump_set_name_mode(uint32_t mode, int32_t maxlen, const void *ext, uint32_t len)
{
    m_dump_name_mode = mode;

    silfp_dump_name_set_ext(maxlen, ext, len);
    LOG_MSG_VERBOSE("mode=0x%x", m_dump_name_mode);
}

void silfp_dump_set_path(const void *path, uint32_t len)
{
    silfp_dump_path_set(path, len);
}

uint32_t silfp_dump_get_name_mode(void)
{
    return m_dump_name_mode;
}

static int32_t _dump_is_v2(void)
{
    LOG_MSG_DEBUG("use v%d dump", m_test_dump_version ? 2 : 1);
    return m_test_dump_version;  //(m_test_dump_version == DUMP_VER_V2);
}

void silfp_dump_set_version(uint8_t ver)
{
    m_test_dump_version = ver;
}

static int32_t _dump_init(void)
{
    int32_t ret = 0;
    uint32_t size = 0;
    //uint8_t type_16bit = IMG_16BIT_DATA_NORMAL;

    if (silfp_dump_check_level() < 0) {
        return -1;
    }

    LOG_MSG_INFO("dump support, note: should not be enabled in release version");

    if (m_test_dump_buffer != NULL) { // already malloced
        return 0;
    }

    //ret = silfp_cmd_test_get_image_info(NULL, NULL, &size, NULL, NULL, NULL, &type_16bit);
    ret = silfp_cmd_test_get_image_info(NULL, NULL, &size, NULL, NULL, NULL, NULL);
    if ((size == 0) || (ret < 0)) {
        LOG_MSG_ERROR("get dump cfg invalid: %d %u", ret, size);
        return -1;
    }

    m_test_dump_buffer = malloc(size);
    if (m_test_dump_buffer == NULL) {
        LOG_MSG_ERROR("malloc dump buf (%d) failed", size);
        return -1;
    }

    m_test_dump_buffer_size = size;
    memset(m_test_dump_buffer, 0, m_test_dump_buffer_size);
    //silfp_bmp_set_img_16bit_type(type_16bit);

    silfp_dump_name_init();
    silfp_dump_mraw_init();

    if (silfp_dump_data_v1 == NULL) {
        silfp_dump_set_version(DUMP_VER_V2);
    }

    return 0;
}

void silfp_dump_deinit(void)
{
    silfp_dump_name_deinit();
    silfp_dump_mraw_deinit();

    if (m_test_dump_buffer != NULL) {
        free(m_test_dump_buffer);
    }
    m_test_dump_buffer = NULL;
    m_test_dump_buffer_size = 0;
}

static int32_t _dump_get_info_data(void *buf, uint32_t size)
{
    int32_t ret = 0;
    uint32_t result = 0;
    uint32_t len = size;

    if ((buf == NULL) || (len == 0)) {
        LOG_MSG_ERROR("param invalid: %u", len);
        return -1;
    }

    memset(buf, 0, len);
    ret = silfp_cmd_send_cmd_with_buf_and_get(REQ_CMD_SUB_ID_DUMP_FILE_EXT, buf, &len, &result);
    if (ret >= 0) {
        ret = len;
    }
    LOG_MSG_DEBUG("get ext info %d %u", ret, len);

    return ret;
}

static int32_t _dump_get_ext_data(char *buf, uint32_t size, dump_frame_t *frame)
{
    uint32_t len = size;

    if ((buf == NULL) || (size == 0)) {
        LOG_MSG_VERBOSE("ext buf invalid");
        return 0;
    }

    if ((frame == NULL) ||(frame->ext.data == NULL) || (frame->ext.len == 0)) {
        LOG_MSG_VERBOSE("no have ext data");
        return 0; // not have ext data, return size 0
    }

    memset(buf, 0, size);
    if (len >= frame->ext.len) {
        len = frame->ext.len;
    } else {
        LOG_MSG_ERROR("no enough ext buf: %u but %u, split", frame->ext.len, size);
    }
    memcpy(buf, frame->ext.data, len);

    return len;
}

static void _dump_data_internal(void *info_buf, uint32_t info_size, void *ext_buf, uint32_t ext_size, uint8_t all)
{
    int32_t ret = 0;

    uint32_t w = 0;
    uint32_t h = 0;
    uint32_t imgsize = 0;
    uint8_t bitcount = 0;
    uint32_t dump_step = 0;
    uint32_t dump_remaining = 0;
    uint32_t remaining = 0;

    char index[MAX_PATH_LEN] = {0};
    char path[MAX_PATH_LEN] = {0};
    char name[MAX_PATH_LEN] = {0};
    uint32_t name_mode = 0;

    dump_frame_t *frame = NULL;
    uint8_t type = DUMP_IMG_TYPE_OTHER;
    uint8_t subtype = DUMP_IMG_SUBTYPE_OTHER;

    silfp_dump_mraw_reset();
    do {
        memset(m_test_dump_buffer, 0, m_test_dump_buffer_size);
        memset(path, 0, sizeof(path));
        memset(name, 0, sizeof(name));
        ext_size = DUMP_EXT_SIZE_MAX;
        memset(ext_buf, 0, ext_size);

        ret = _dump_get_info_data(info_buf, info_size);
        if (ret < 0) {
            break;
        }

        frame = silfp_dump_util_info_parse(info_buf, ret);
        if (frame != NULL) {
            type = frame->type;
            subtype = frame->subtype;
        }

        name_mode = m_dump_name_mode;
        if ((DUMP_IMG_TYPE_CAL == type) && (CHECK_FLAG_SET(name_mode, DUMP_NAME_MODE_CAL_TIMESTAMP_NONE))) {
            name_mode |= DUMP_NAME_MODE_TIMESTAMP_NONE;
        }

        ret = silfp_dump_path_get_save_path(path, sizeof(path), type, subtype, name_mode);
        if (ret < 0) {
            LOG_MSG_ERROR("get save path fail: %d", ret);
            break;
        }
        if (dump_step == 0) {
            silfp_dump_name_get_save_index(index, sizeof(index), silfp_dump_path_get(), name_mode);
        }
        ret = silfp_dump_name_get_save_name(name, sizeof(name), index, frame, name_mode);
        if (ret < 0) {
            LOG_MSG_ERROR("get save name fail: %d", ret);
        }

        if (ext_buf != NULL) {
            ext_size = _dump_get_ext_data(ext_buf, ext_size, frame);
        }

        snprintf(m_test_dump_buffer, m_test_dump_buffer_size, "%s/%s", path, name); // send the dump image path to ta
        ret = silfp_cmd_test_dump_data(type, dump_step, m_test_dump_buffer, m_test_dump_buffer_size, &dump_remaining, &imgsize, &w, &h, &bitcount);
        if (imgsize > 0) {
            LOG_MSG_DEBUG("%d (%d:%d:%d)", imgsize, w, h, bitcount);
            silfp_bmp_save(path, name, m_test_dump_buffer, imgsize, w, h, bitcount, ext_buf, ext_size, 0);
            frame->width = w;
            frame->height = h;
            frame->bitcount = bitcount;
            silfp_dump_mraw_add(m_test_dump_buffer, imgsize, frame);
        }

        if (all) {
            remaining = dump_remaining;
        } else {
            if ((remaining != 0) && (remaining <= dump_remaining)) {
                remaining--;
            } else {
                remaining = dump_remaining;
            }
        }
        silfp_dump_util_frame_clear(frame);

        dump_step++;
    } while ((ret >= 0) && (dump_step < DUMP_STEP_MAX) && (remaining > 0));

    silfp_dump_mraw(name_mode, index);
}

void silfp_dump_data2(uint8_t all)
{
    int32_t ret = 0;
    void *ext_buf = NULL;
    uint32_t ext_size = DUMP_EXT_SIZE_MAX;
    void *info_buf = NULL;
    uint32_t info_size = DUMP_EXT_INFO_SIZE_MAX;

    if (!_dump_is_v2()) {
        return;
    }

    ret = _dump_init();
    if (ret < 0) {
        return;
    }

    if (m_test_dump_buffer == NULL) {
        LOG_MSG_ERROR("dump buffer NULL");
        return;
    }

    ext_buf = malloc(ext_size);
    if (ext_buf == NULL) {
        LOG_MSG_ERROR("malloc ext buffer (%d) failed", ext_size);
        return;
    }

    info_buf = malloc(info_size);
    if (info_buf == NULL) {
        LOG_MSG_ERROR("malloc info buffer (%d) failed", info_size);
        free(ext_buf);
        return;
    }

    _dump_data_internal(info_buf, info_size, ext_buf, ext_size, all);

    free(ext_buf);
    free(info_buf);
}

void silfp_dump_data(int32_t type)
{
    int32_t ret = 0;

    if (_dump_is_v2()) {
        silfp_dump_data2(0);
        return;
    }

    if (silfp_dump_data_v1 == NULL) { // not implement v1
        return;
    }

    ret = _dump_init();
    if (ret < 0) {
        return;
    }

    if (m_test_dump_buffer == NULL) {
        LOG_MSG_ERROR("dump buffer NULL");
        return;
    }

    silfp_dump_data_v1(type, m_test_dump_buffer, m_test_dump_buffer_size, m_dump_name_mode);
}

#else
void silfp_dump_data(int32_t type)
{
    UNUSED(type);
}

void silfp_dump_set_version(uint8_t ver)
{
    UNUSED(ver);
}

void silfp_dump_data_ext(int32_t type, uint8_t final)
{
    UNUSED(type);
    UNUSED(final);
}

void silfp_dump_set_path(const void *path, uint32_t len)
{
    UNUSED(path);
    UNUSED(len);
}

void silfp_dump_deinit(void)
{
}

#endif /* SIL_DUMP_IMAGE */
