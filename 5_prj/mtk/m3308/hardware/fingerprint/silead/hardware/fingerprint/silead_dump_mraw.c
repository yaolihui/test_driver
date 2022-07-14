/******************************************************************************
 * @file   silead_dump_path.c
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
 * Calvin Wang 2020/03/10 0.1.0      Init version
 * Ashely Li   2021/1/15  0.2.0      Update copyright notice
 *
 *****************************************************************************/

#ifdef SIL_DUMP_IMAGE

#define FILE_TAG "dump_path"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>

#include "silead_bmp.h"
#include "silead_util.h"
#include "silead_dump_param.h"
#include "silead_dump_path.h"
#include "silead_dump_name.h"
#include "silead_dump_mraw.h"

#define DUM_AUTH_MRAW_IMG_MAX 3
#define MAX_PATH_LEN 256

static uint8_t m_dump_mraw_enable = 1;

static uint32_t m_dump_mraw_offset = 0;
static dump_frame_t *m_dump_mraw_frame = NULL;

void silfp_dump_mraw_set_enable(uint8_t enable)
{
    m_dump_mraw_enable = enable;
}

void silfp_dump_mraw_init(void)
{
    m_dump_mraw_frame = NULL;
    m_dump_mraw_offset = 0;
}

void silfp_dump_mraw_deinit(void)
{
    silfp_dump_mraw_reset();
}

void silfp_dump_mraw_reset(void)
{
    silfp_dump_util_frame_clear(m_dump_mraw_frame);
    m_dump_mraw_frame = NULL;
    m_dump_mraw_offset = 0;
}

static int32_t _dump_mraw_frame_gen(dump_frame_t *frame, uint32_t img_size, uint32_t suffix_size)
{
    int32_t ret = 0;
    void *buf = NULL;

    // already gen the frame
    if (m_dump_mraw_frame != NULL) {
        return 0;
    }

    if (frame == NULL) {
        return -1;
    }

    m_dump_mraw_frame = silfp_dump_util_frame_gen(frame->type, frame->subtype, 0xFF, frame->bitcount, frame->width, frame->height, frame->timestamp);
    if (m_dump_mraw_frame == NULL) {
        LOG_MSG_ERROR("gen mraw frame fail");
        return -1;
    }

    buf = malloc(img_size);
    if (buf == NULL) {
        LOG_MSG_ERROR("malloc(%u) fail", img_size);
        return -1;
    }

    memset(buf, 0, img_size);
    ret = silfp_dump_util_frame_append_data(m_dump_mraw_frame, buf, img_size, MEM_FLAG_MALLOCED, FRAME_DATA_TYPE_IMG);
    if (ret < 0) {
        LOG_MSG_ERROR("append image buf fail");
        free(buf);
        return -1;
    }

    buf = malloc(suffix_size);
    if (buf == NULL) {
        LOG_MSG_ERROR("malloc(%u) fail", suffix_size);
        return -1;
    }

    memset(buf, 0, suffix_size);
    ret = silfp_dump_util_frame_append_data(m_dump_mraw_frame, buf, suffix_size - 1, MEM_FLAG_MALLOCED, FRAME_DATA_TYPE_SUFFIX);
    if (ret < 0) {
        LOG_MSG_ERROR("append image buf fail");
        free(buf);
        return -1;
    }

    return 0;
}

int32_t silfp_dump_mraw_add(void *buf, uint32_t len, dump_frame_t *frame)
{
    int32_t ret = 0;

    if (!m_dump_mraw_enable) {
        return 0;
    }

    if ((buf == NULL) || (len == 0) || (frame == NULL)) {
        return -1;
    }

    LOG_MSG_DEBUG("type = %d, subtype = %d", frame->type, frame->subtype);

    if ((frame->type != DUMP_IMG_TYPE_AUTH) || (frame->subtype != DUMP_IMG_SUBTYPE_ORIG)) {
        return -1;
    }

    ret = _dump_mraw_frame_gen(frame, DUM_AUTH_MRAW_IMG_MAX * len, MAX_PATH_LEN);
    if (ret < 0) {
        silfp_dump_mraw_reset();
        return -1;
    }

    if ((frame->bitcount != m_dump_mraw_frame->bitcount) || (frame->width != m_dump_mraw_frame->width) || (frame->height != m_dump_mraw_frame->height)) {
        LOG_MSG_ERROR("invalid frame %u:%u:%u but %u:%u:%u", m_dump_mraw_frame->width, m_dump_mraw_frame->height, m_dump_mraw_frame->bitcount,
                      frame->width, frame->height, frame->bitcount);
        return -1;
    }

    if (m_dump_mraw_offset + len <= m_dump_mraw_frame->image.len) {
        memcpy((uint8_t *)(m_dump_mraw_frame->image.data) + m_dump_mraw_offset, buf, len);
        m_dump_mraw_offset += len;
    } else {
        LOG_MSG_ERROR("not enough buf: %u + %u but %d", m_dump_mraw_offset, len, m_dump_mraw_frame->image.len);
        return -1;
    }

    if ((frame->suffix.data != NULL) && (frame->suffix.len <= m_dump_mraw_frame->suffix.len)) {
        memset(m_dump_mraw_frame->suffix.data, 0, m_dump_mraw_frame->suffix.len);
        memcpy(m_dump_mraw_frame->suffix.data, frame->suffix.data, frame->suffix.len);
        LOG_MSG_ERROR("set suffix: %s", (char *)(m_dump_mraw_frame->suffix.data));
    } else {
        LOG_MSG_ERROR("not enough suffix buf: %u but %u", m_dump_mraw_frame->suffix.len, frame->suffix.len);
    }

    if (frame->timestamp != 0) {
        m_dump_mraw_frame->timestamp = frame->timestamp;
    }

    return 0;
}

int32_t silfp_dump_mraw(uint32_t name_mode, char *index)
{
    int32_t ret = 0;
    char path[MAX_PATH_LEN] = {0};
    char name[MAX_PATH_LEN] = {0};

    if (!m_dump_mraw_enable) {
        return 0;
    }

    if ((m_dump_mraw_frame == NULL) || (m_dump_mraw_offset == 0)) {
        return -1;
    }

    ret = silfp_dump_path_get_save_path(path, sizeof(path), DUMP_IMG_TYPE_AUTH, DUMP_IMG_SUBTYPE_MRAW, name_mode);
    if (ret < 0) {
        LOG_MSG_ERROR("get save path fail: %d", ret);
        return ret;
    }

    m_dump_mraw_frame->subtype = DUMP_IMG_SUBTYPE_MRAW;
    ret = silfp_dump_name_get_save_name(name, sizeof(name), index, m_dump_mraw_frame, name_mode);
    if (ret < 0) {
        LOG_MSG_ERROR("get save name fail: %d", ret);
        return ret;
    }

    silfp_bmp_save(path, name, m_dump_mraw_frame->image.data, m_dump_mraw_offset, m_dump_mraw_frame->width, m_dump_mraw_frame->height, m_dump_mraw_frame->bitcount, NULL, 0, 0);
    silfp_dump_mraw_reset();

    return 0;
}

#endif /* SIL_DUMP_IMAGE */