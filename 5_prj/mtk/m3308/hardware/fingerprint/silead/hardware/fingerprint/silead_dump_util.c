/******************************************************************************
 * @file   silead_dump_util.c
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
 * <author>      <date>      <version>     <desc>
 * Calvin Wang 2019/12/29 0.1.0      Init version
 * Ashely Li   2021/1/15  0.2.0      Update copyright notice
 *
 *****************************************************************************/

#define FILE_TAG "dump_util"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>

#include "silead_dump_param.h"
#include "silead_dump_util.h"

#define FRAME_NODE_MAX 40

#define INFO_DATA_MAGIC 0x0511EADF

#define FLAG_FRAME_GET_NORMAL 0
#define FLAG_FRAME_GET_POPUP  1

#define SET_P_INT_VALUE(p, v) \
    if ((p) != NULL) { \
        *(p) = (v); \
    }

#define CHECK_AND_COPY_BUF(p, size, pv, len) \
    if ((size) == (len)) { \
        if (((p) != NULL) && ((pv) != NULL)) { \
            memcpy(p, pv, size); \
        } \
    } else { \
        LOG_MSG_ERROR("data len invalid, %u but %u", (size), (len)); \
    }

#define FRAME_LIST_COUNT_VERBOSE(desc) LOG_MSG_VERBOSE(desc "(list:%u) (%u + %u)", FRAME_LIST_IDX_MAX, m_dump_frame_count[FRAME_LIST_IDX_NORM], m_dump_frame_count[FRAME_LIST_IDX_EXT])
#define FRAME_LIST_COUNT_DEBUG(desc) LOG_MSG_DEBUG(desc "(list:%u) (%u + %u)", FRAME_LIST_IDX_MAX, m_dump_frame_count[FRAME_LIST_IDX_NORM], m_dump_frame_count[FRAME_LIST_IDX_EXT])
#define FRAME_GET_COUNT() (m_dump_frame_count[FRAME_LIST_IDX_NORM] + m_dump_frame_count[FRAME_LIST_IDX_EXT])
#define FRAME_GET_EACH_COUNT() ((m_dump_frame_count[FRAME_LIST_IDX_NORM] > 0) ? m_dump_frame_count[FRAME_LIST_IDX_NORM] : m_dump_frame_count[FRAME_LIST_IDX_EXT])

typedef enum {
    EXT_ID_DIR_TYPE = 0,
    EXT_ID_SUBDIR_TYPE,
    EXT_ID_SUBSTEP_VALUE,
    EXT_ID_TIME_STAMP,
    EXT_ID_SUFFIX_PARAM,
    EXT_ID_EXT_DATA,
} e_ext_data_id_t;

typedef struct _dump_frame_list {
    struct _dump_frame_list *prev;
    struct _dump_frame_list *next;
    dump_frame_t *frame;
} dump_frame_list_t;

enum {
    FRAME_LIST_IDX_NORM = 0,
    FRAME_LIST_IDX_EXT,
    FRAME_LIST_IDX_MAX,
} e_frame_list_idx_t;

static uint8_t m_dump_img_enable = 0;
static dump_frame_list_t *m_dump_frame_header[FRAME_LIST_IDX_MAX] = {NULL, NULL};
static uint8_t m_dump_frame_count[FRAME_LIST_IDX_MAX] = {0, 0};

void silfp_dump_util_set_enable(uint8_t enable)
{
    m_dump_img_enable = enable;
    LOG_MSG_DEBUG("enabel = %u", m_dump_img_enable);
}

static dump_frame_t* _dump_util_frame_item_get(uint8_t popup)
{
    uint32_t idx = 0;
    dump_frame_list_t *frame_item = NULL;
    dump_frame_t *frame = NULL;

    for (idx = 0; idx < FRAME_LIST_IDX_MAX; idx++) {
        if (m_dump_frame_header[idx] != NULL) {
            frame_item = m_dump_frame_header[idx]->prev;
        }

        if (frame_item != NULL) {
            frame = frame_item->frame;
            if (popup != FLAG_FRAME_GET_NORMAL) {
                if (frame_item == m_dump_frame_header[idx]) { // just one item
                    m_dump_frame_header[idx] = NULL;
                } else {
                    frame_item->prev->next = frame_item->next;
                    frame_item->next->prev = frame_item->prev;
                }
                free(frame_item);
                m_dump_frame_count[idx]--;
            }
            break;
        }
    }
    LOG_MSG_VERBOSE("popup: %u", popup);
    FRAME_LIST_COUNT_VERBOSE("frame remain: ");

    return frame;
}

void silfp_dump_util_frame_clear(dump_frame_t *frame)
{
    if (frame != NULL) {
        if (frame->suffix.data != NULL) {
            if (frame->suffix.resident != MEM_FLAG_RESIDENT) {
                free(frame->suffix.data);
            }
        }
        if (frame->ext.data != NULL) {
            if (frame->ext.resident != MEM_FLAG_RESIDENT) {
                free(frame->ext.data);
            }
        }
        if (frame->image.data != NULL) {
            if (frame->image.resident != MEM_FLAG_RESIDENT) {
                free(frame->image.data);
            }
        }
        free(frame);
    }
}

void silfp_dump_util_clear_all(void)
{
    dump_frame_t *frame = NULL;

    frame = _dump_util_frame_item_get(FLAG_FRAME_GET_POPUP);
    while (frame != NULL) {
        silfp_dump_util_frame_clear(frame);
        frame = _dump_util_frame_item_get(FLAG_FRAME_GET_POPUP);
    }

    FRAME_LIST_COUNT_DEBUG("frame count remain: ");
}

static void _dump_util_sync_data_by_header(uint32_t idx)
{
    dump_frame_list_t *frame_item = NULL;
    dump_frame_t *base_frame = NULL;

    if (idx >= FRAME_LIST_IDX_MAX) {
        LOG_MSG_ERROR("idx (%d) invalid", idx);
        return;
    }

    frame_item = m_dump_frame_header[idx];
    if ((frame_item == NULL) || (frame_item->frame == NULL)) {
        return; // no dump frame
    }

    base_frame = frame_item->frame;
    if ((base_frame->suffix.flag != FRAME_DATA_FLAG_NORMAL) || (base_frame->suffix.data == NULL) || (base_frame->suffix.len == 0)) {
        return; // no suffix data
    }

    // just sync suffix data in current version
    frame_item = frame_item->next;
    while (frame_item != m_dump_frame_header[idx]) {
        if ((frame_item->frame == NULL) || (frame_item->frame->suffix.flag != FRAME_DATA_FLAG_WAIT_SYNC)) {
            break;
        }

        frame_item->frame->suffix.data = base_frame->suffix.data;
        frame_item->frame->suffix.len = base_frame->suffix.len;
        frame_item->frame->suffix.flag = FRAME_DATA_FLAG_NORMAL;
        frame_item->frame->suffix.resident = MEM_FLAG_RESIDENT;

        frame_item = frame_item->next;
    }
}

static int32_t _dump_util_check_frame_and_clear(dump_frame_t *frame)
{
    uint32_t i = 0;

    if (frame == NULL) {
        return 0;
    }

    for (i = 0; i < FRAME_LIST_IDX_MAX; i++) {
        if ((m_dump_frame_header[i] == NULL) || (m_dump_frame_header[i]->frame == NULL)) {
            continue;
        }

        if (m_dump_frame_header[i]->frame->type == frame->type) {
            continue; // same type
        }

        LOG_MSG_VERBOSE("dump new type (%u)%u->%u, clear all", i, m_dump_frame_header[i]->frame->type, frame->type);
        silfp_dump_util_clear_all();
        break;
    }

    return 0;
}

static int32_t _dump_util_frame_add(uint32_t idx, dump_frame_t *frame)
{
    dump_frame_list_t *frame_item = NULL;

    if (!m_dump_img_enable) {
        LOG_MSG_VERBOSE("not enable dump img");
        return -1;
    }

    if ((idx >= FRAME_LIST_IDX_MAX) || (frame == NULL)) {
        LOG_MSG_ERROR("frame %u data invalid", idx);
        return -1;
    }

    frame_item = (dump_frame_list_t *)malloc(sizeof(dump_frame_list_t));
    if (frame_item == NULL) {
        LOG_MSG_ERROR("malloc (%d) fail", (int32_t)sizeof(dump_frame_list_t));
        return -1;
    }

    FRAME_LIST_COUNT_VERBOSE("add start frame count: ");
    _dump_util_check_frame_and_clear(frame);

    memset(frame_item, 0, sizeof(dump_frame_list_t));
    frame_item->frame = frame;
    if (m_dump_frame_header[idx] == NULL) {
        m_dump_frame_header[idx] = frame_item;
        m_dump_frame_header[idx]->prev = m_dump_frame_header[idx];
        m_dump_frame_header[idx]->next = m_dump_frame_header[idx];
    } else {
        frame_item->next = m_dump_frame_header[idx];
        frame_item->prev = m_dump_frame_header[idx]->prev;
        m_dump_frame_header[idx]->prev->next = frame_item;
        m_dump_frame_header[idx]->prev = frame_item;
        m_dump_frame_header[idx] = frame_item;
    }

    m_dump_frame_count[idx]++;
    _dump_util_sync_data_by_header(idx);

    FRAME_LIST_COUNT_VERBOSE("add end frame count: ");

    return 0;
}

int32_t silfp_dump_util_frame_add(dump_frame_t *frame)
{
    LOG_MSG_VERBOSE("add normal frame");
    return _dump_util_frame_add(FRAME_LIST_IDX_NORM, frame);
}

int32_t silfp_dump_util_frame_add_ext(dump_frame_t *frame)
{
    LOG_MSG_VERBOSE("add ext frame");
    return _dump_util_frame_add(FRAME_LIST_IDX_EXT, frame);
}

dump_frame_t* silfp_dump_util_frame_gen(uint8_t type, uint8_t subtype, uint8_t step, uint8_t bitcount, uint16_t width, uint16_t height, uint64_t timestamp)
{
    dump_frame_t *frame = NULL;
    uint32_t count = 0;

    if (!m_dump_img_enable) {
        LOG_MSG_VERBOSE("not enable dump img");
        return NULL;
    }

    count = FRAME_GET_COUNT();
    if (count >= FRAME_NODE_MAX) {
        LOG_MSG_ERROR("dump list full %u", count);
        return NULL;
    }

    frame = malloc(sizeof(dump_frame_t));
    if (frame == NULL) {
        LOG_MSG_ERROR("malloc (%d) fail", (int32_t)sizeof(dump_frame_t));
        return NULL;
    }

    memset(frame, 0, sizeof(dump_frame_t));
    frame->type = type;
    frame->subtype = subtype;
    frame->step = step;
    frame->bitcount = bitcount;
    frame->width = width;
    frame->height = height;
    frame->timestamp = timestamp;
    frame->suffix.flag = FRAME_DATA_FLAG_NONE;
    frame->ext.flag = FRAME_DATA_FLAG_NONE;
    frame->image.flag = FRAME_DATA_FLAG_NONE;

    LOG_MSG_VERBOSE("gen frame: type=%u, subtype=%u, step=%u, bitcount=%u, width=%u, height=%u, timestamp=%u", \
                    type, subtype, step, bitcount, width, height, (uint32_t)timestamp);

    return frame;
}

static int32_t _dump_util_frame_append_data(dump_frame_data_t *frame_data, void *data, uint32_t len, uint8_t mem_flag, uint8_t force)
{
    void *pdata = NULL;

    if ((frame_data == NULL) || (data == NULL) || (len == 0)) {
        return -1;
    }

    if ((!force) && (frame_data->data != NULL)) {
        LOG_MSG_ERROR("data already set, but not force, ignor");
        return -1;
    }

    if (mem_flag == MEM_FLAG_REMALLOC) {
        pdata = malloc(len + 1);
        if (pdata == NULL) {
            LOG_MSG_ERROR("malloc (%u) fail", len);
            return -1;
        }
        memset(pdata, 0, len + 1);
        memcpy(pdata, data, len);
    } else {
        pdata = data;
    }

    if ((frame_data->data != NULL) && (frame_data->resident != MEM_FLAG_RESIDENT)) {
        LOG_MSG_ERROR("force set, clear the data first");
        free(frame_data->data);
    }
    frame_data->data = pdata;
    frame_data->resident = mem_flag;
    frame_data->flag = FRAME_DATA_FLAG_NORMAL;
    frame_data->len = len;

    return 0;
}

int32_t silfp_dump_util_frame_append_data(dump_frame_t *frame, void *data, uint32_t len, uint8_t mem_flag, uint8_t data_type)
{
    int32_t ret = 0;
    dump_frame_data_t *frame_data = NULL;

    if (!m_dump_img_enable) {
        LOG_MSG_VERBOSE("not enable dump img");
        return -1;
    }

    if ((frame == NULL) || (data == NULL) || (len == 0)) {
        LOG_MSG_ERROR("data invalid");
        return -1;
    }

    if (data_type == FRAME_DATA_TYPE_SUFFIX) {
        frame_data = &(frame->suffix);
    } else if (data_type == FRAME_DATA_TYPE_EXT) {
        frame_data = &(frame->ext);
    } else if (data_type == FRAME_DATA_TYPE_IMG) {
        frame_data = &(frame->image);
    } else {
        LOG_MSG_ERROR("data_type %d invalid, ignor", data_type);
    }

    ret = _dump_util_frame_append_data(frame_data, data, len, mem_flag, 1);
    LOG_MSG_VERBOSE("append data %u, len %u, mem_flag %u, result %d", data_type, len, mem_flag, ret);

    return ret;
}

int32_t silfp_dump_util_frame_append_wait_sync_data(dump_frame_t *frame, uint8_t data_type)
{
    dump_frame_data_t *frame_data = NULL;

    if (!m_dump_img_enable) {
        LOG_MSG_VERBOSE("not enable dump img");
        return -1;
    }

    if (data_type == FRAME_DATA_TYPE_SUFFIX) {
        frame_data = &(frame->suffix);
    } else if (data_type == FRAME_DATA_TYPE_EXT) {
        frame_data = &(frame->ext);
    } else if (data_type == FRAME_DATA_TYPE_IMG) {
        frame_data = &(frame->image);
    } else {
        LOG_MSG_ERROR("data_type %d invalid, ignor", data_type);
    }

    if (frame_data != NULL) {
        frame_data->flag = FRAME_DATA_FLAG_WAIT_SYNC;
        frame_data->len = 0;
        if (frame_data->data != NULL) { // if already set data, free if needed
            if (frame_data->resident != MEM_FLAG_RESIDENT) {
                free(frame_data->data);
            }
            frame_data->data = NULL;
        }
    }

    LOG_MSG_VERBOSE("append wait sync data %d, flag %d", data_type, frame_data->flag);
    return 0;
}

int32_t silfp_dump_util_frame_sync_data(void *data, uint32_t len, uint8_t mem_flag, uint8_t data_type)
{
    int32_t ret = 0;
    dump_frame_list_t *frame_header = NULL;
    dump_frame_data_t *frame_data = NULL;

    if (!m_dump_img_enable) {
        LOG_MSG_VERBOSE("not enable dump img");
        return -1;
    }

    if ((data == NULL) || (len == 0)) {
        LOG_MSG_ERROR("data invalid");
        return -1;
    }

    frame_header = m_dump_frame_header[FRAME_LIST_IDX_NORM];
    if ((frame_header == NULL) || (frame_header->frame == NULL)) {
        return -1; // no dump frame
    }

    if (frame_header->frame->type != DUMP_IMG_TYPE_AUTH) {
        LOG_MSG_ERROR("sync data fail, invalid frame type %u", frame_header->frame->type);
        return -1; // just auth, just in case
    }

    // just sync suffix data in current version
    if (data_type != FRAME_DATA_TYPE_SUFFIX) {
        LOG_MSG_ERROR("sync data fail, invalid type %u", data_type);
        return -1;
    }

    frame_data = &(frame_header->frame->suffix);
    if ((frame_data == NULL) || (frame_data->flag != FRAME_DATA_FLAG_WAIT_SYNC)) {
        return -1;
    }

    ret = _dump_util_frame_append_data(frame_data, data, len, mem_flag, 0);
    _dump_util_sync_data_by_header(FRAME_LIST_IDX_NORM);

    LOG_MSG_VERBOSE("sync data %u, len %u, mem_flag %u, result %d", data_type, len, mem_flag, ret);

    return ret;
}

// return data size
int32_t silfp_dump_util_get_data(void *buf, uint32_t size, uint32_t *w, uint32_t *h, uint8_t *bitcount, uint32_t *remain)
{
    int32_t ret = 0;
    dump_frame_t *frame = NULL;

    if ((buf == NULL) || (size == 0)) {
        LOG_MSG_ERROR("data invalid %u", size);
        return -1;
    }

    frame = _dump_util_frame_item_get(FLAG_FRAME_GET_POPUP);
    if (frame == NULL) {
        LOG_MSG_ERROR("none dump data");
        return -1;
    }

    SET_P_INT_VALUE(w, frame->width);
    SET_P_INT_VALUE(h, frame->height);
    SET_P_INT_VALUE(bitcount, frame->bitcount);
    SET_P_INT_VALUE(remain, FRAME_GET_EACH_COUNT());

    memset(buf, 0, size);
    if ((frame->image.data != NULL) && (frame->image.len > 0)) {
        ret = frame->image.len;
        if (frame->image.len > size) {
            LOG_MSG_ERROR("not enough buf: %u but %u, truncate", frame->image.len, size);
            ret = size;
        }
        memcpy(buf, frame->image.data, ret);
    }

    silfp_dump_util_frame_clear(frame);

    return ret;
}

static int32_t _dump_util_data_byte_pack(void *buf, uint32_t size, uint32_t offset, void *data, uint32_t len, uint8_t id)
{
    uint8_t *p = (uint8_t *)buf;

    if ((buf == NULL) || (data == NULL) || (len == 0)) {
        return offset;
    }

    if ((size < offset + len + 5)) { // id (1 byte) + data len (4 byte) + data
        LOG_MSG_ERROR("append %d, not enough buf, %u but %u", id, offset + len + 5, size);
        return offset;
    }

    p[offset++] = id;
    p[offset++] = (len >> 24) & 0xFF;
    p[offset++] = (len >> 16) & 0xFF;
    p[offset++] = (len >> 8) & 0xFF;
    p[offset++] = len & 0xFF;

    memcpy(p + offset, data, len);
    offset += len;

    return offset;
}

static int32_t _dump_util_data_string_pack(void *buf, uint32_t size, uint32_t offset, void *data, uint32_t len, uint8_t id)
{
    uint8_t *p = (uint8_t *)buf;
    uint32_t data_len = 0;

    if ((buf == NULL) || (data == NULL) || (len == 0)) {
        return offset;
    }

    data_len = len + 1; // data + '\0' (1 byte)
    if ((size < offset + data_len + 5)) { // id (1 byte) + data len (4 byte) + data + '\0' (1 byte)
        LOG_MSG_ERROR("append %d, not enough buf, %u but %u", id, offset + data_len + 5, size);
        return offset;
    }

    p[offset++] = id;
    p[offset++] = (data_len >> 24) & 0xFF;
    p[offset++] = (data_len >> 16) & 0xFF;
    p[offset++] = (data_len >> 8) & 0xFF;
    p[offset++] = data_len & 0xFF;

    memcpy(p + offset, data, len);
    offset += len;
    p[offset++] = '\0';

    return offset;
}

// return info size
int32_t silfp_dump_util_get_info(void *buf, uint32_t size)
{
    dump_frame_t *frame = NULL;
    int32_t offset = 0;
    uint32_t magic = INFO_DATA_MAGIC;

    if ((buf == NULL) || (size == 0)) {
        LOG_MSG_ERROR("data invalid %u", size);
        return -1;
    }

    frame = _dump_util_frame_item_get(FLAG_FRAME_GET_NORMAL);
    if (frame == NULL) {
        LOG_MSG_ERROR("none dump data");
        return -1;
    }

    memset(buf, 0, size);
    if (size >= sizeof(magic)) {
        memcpy(buf, &magic, sizeof(magic));
        offset += sizeof(magic);
    }

    offset = _dump_util_data_byte_pack(buf, size, offset, &(frame->type), sizeof(frame->type), EXT_ID_DIR_TYPE);
    offset = _dump_util_data_byte_pack(buf, size, offset, &(frame->subtype), sizeof(frame->subtype), EXT_ID_SUBDIR_TYPE);
    offset = _dump_util_data_byte_pack(buf, size, offset, &(frame->step), sizeof(frame->step), EXT_ID_SUBSTEP_VALUE);
    offset = _dump_util_data_byte_pack(buf, size, offset, &(frame->timestamp), sizeof(frame->timestamp), EXT_ID_TIME_STAMP);
    offset = _dump_util_data_string_pack(buf, size, offset, frame->suffix.data, frame->suffix.len, EXT_ID_SUFFIX_PARAM);
    offset = _dump_util_data_byte_pack(buf, size, offset, frame->ext.data, frame->ext.len, EXT_ID_EXT_DATA);

    return offset;
}

dump_frame_t* silfp_dump_util_info_parse(const void *buf, uint32_t size)
{
    dump_frame_t *frame = NULL;
    uint8_t *pdata = (uint8_t *)buf;
    uint32_t offset = 0;
    uint32_t magic = 0;
    uint8_t id = 0;
    uint32_t len = 0;

    if ((buf == NULL) || (size < sizeof(magic))) {
        LOG_MSG_ERROR("data invalid %u", size);
        return NULL;
    }

    memcpy(&magic, buf, sizeof(magic));
    if (magic != INFO_DATA_MAGIC) {
        LOG_MSG_VERBOSE("old info data");
        return NULL;
    }
    offset += sizeof(magic);

    frame = malloc(sizeof(dump_frame_t));
    if (frame == NULL) {
        LOG_MSG_ERROR("malloc (%d) fail", (int32_t)sizeof(dump_frame_t));
        return NULL;
    }
    memset(frame, 0, sizeof(dump_frame_t));

    while (offset + 5 < size) {
        id = pdata[offset++];
        len = ((pdata[offset++] << 24) & 0xFF000000);
        len |= ((pdata[offset++] << 16) & 0x00FF0000);
        len |= ((pdata[offset++] << 8) & 0x0000FF00);
        len |= (pdata[offset++] & 0x000000FF);

        if (len == 0) { // no data
            continue;
        }

        if (len + offset > size) {
            LOG_MSG_ERROR("%d, data invalid, %u but %u", id, len + offset, size);
            break;
        }

        switch (id) {
        case EXT_ID_DIR_TYPE: {
            CHECK_AND_COPY_BUF(&(frame->type), (uint32_t)sizeof(frame->type), pdata + offset, len);
            break;
        }
        case EXT_ID_SUBDIR_TYPE: {
            CHECK_AND_COPY_BUF(&(frame->subtype), (uint32_t)sizeof(frame->subtype), pdata + offset, len);
            break;
        }
        case EXT_ID_SUBSTEP_VALUE: {
            CHECK_AND_COPY_BUF(&(frame->step), (uint32_t)sizeof(frame->step), pdata + offset, len);
            break;
        }
        case EXT_ID_TIME_STAMP: {
            CHECK_AND_COPY_BUF(&(frame->timestamp), (uint32_t)sizeof(frame->timestamp), pdata + offset, len);
            break;
        }
        case EXT_ID_SUFFIX_PARAM: {
            if (len > 1) {
                frame->suffix.len = len - 1;
                frame->suffix.data = pdata + offset;
                frame->suffix.resident = MEM_FLAG_RESIDENT;
            }
            break;
        }
        case EXT_ID_EXT_DATA: {
            frame->ext.data = pdata + offset;
            frame->ext.len = len;
            frame->ext.resident = MEM_FLAG_RESIDENT;
            break;
        }
        }

        offset += len;
    }
    return frame;
}