/******************************************************************************
 * @file   silead_config_algo_dump_util.c
 * @brief  Contains Chip config files dump to .h files.
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
 * Calvin Wang  2021/3/10    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "cfg_algo_dump"
#include "silead_logmsg.h"

#include "silead_util.h"
#include "silead_config_algo_dump_util.h"

#define ALGO_PARAM_HEX_PERFIX_SIZE 4
#define ALGO_PARAM_HEX_FMT_LEN_MIN 20

#define DUMP_PARAM_DATA_ARRAY(data, count, type_t, fmt, sfmt) \
    do { \
        uint32_t _i = 0; \
        type_t _value = 0; \
        char _svalue[32] = {0}; \
        char _buffer[256] = {0}; \
        for (_i = 0; _i < count; _i++) { \
            if (_i % count_per_line == 0) { \
                snprintf(_buffer, sizeof(_buffer), "    "); \
                _algo_cfg_dump_to_buf(dump_buf, dump_len, dump_offset, _buffer, strlen(_buffer)); \
            } \
            memcpy(&_value, data + _i * sizeof(type_t), sizeof(type_t)); \
            snprintf(_svalue, sizeof(_svalue), fmt, _value); \
            if (((_i + 1) % count_per_line == 0) || (_i + 1) == count) { \
                _algo_cfg_dump_to_buf(dump_buf, dump_len, dump_offset, _svalue, strlen(_svalue)); \
            } else { \
                snprintf(_buffer, sizeof(_buffer), sfmt, _svalue); \
                _algo_cfg_dump_to_buf(dump_buf, dump_len, dump_offset, _buffer, strlen(_buffer)); \
            } \
            if ((_i + 1) % count_per_line == 0) { \
                snprintf(_buffer, sizeof(_buffer), (const char *)"\n"); \
                _algo_cfg_dump_to_buf(dump_buf, dump_len, dump_offset, _buffer, strlen(_buffer)); \
            } \
        } \
        if (_i % count_per_line != 0) { \
            snprintf(_buffer, sizeof(_buffer), (const char *)"\n"); \
            _algo_cfg_dump_to_buf(dump_buf, dump_len, dump_offset, _buffer, strlen(_buffer)); \
        } \
    } while (0)

#define DUMP_PARAM_HEX_PARSE(fd, data, len, type_t, fmt, sfmt, mul) \
    do { \
        char *dump_buf = NULL; \
        uint32_t dump_len = 0; \
        uint32_t *dump_offset = NULL; \
        uint32_t count = 0; \
        uint32_t data_offset = 0; \
        if ((len) % sizeof(type_t) != 0) { \
            return -1; \
        } \
        count = (len) / sizeof(type_t); \
        dump_offset = &data_offset; \
        dump_len = count * mul; \
        dump_buf = (char *)malloc(dump_len); \
        if (dump_buf == NULL) { \
            LOG_MSG_ERROR("malloc %d failed", dump_len); \
            return -1; \
        } \
        DUMP_PARAM_DATA_ARRAY(data, count, type_t, fmt, sfmt); \
        if (data_offset > 0) { \
            silfp_ree_util_write_file(fd, dump_buf, data_offset); \
        } else { \
            ret = -1; \
        } \
        free(dump_buf); \
    } while (0)

static void _algo_cfg_dump_to_buf(char *dump_buf, uint32_t dump_len, uint32_t *dump_offset, char *buffer, uint32_t len)
{
    if ((dump_buf != NULL) && (dump_len != 0) && (dump_offset != NULL) && (buffer != NULL)) {
        if (dump_len >= (*dump_offset) + len) {
            memcpy(dump_buf + (*dump_offset), buffer, len);
            *dump_offset = (*dump_offset) + len;
        } else {
            LOG_MSG_ERROR("buf is too small (%u: %u + %u)", dump_len, *dump_offset, len);
        }
    }
}

static int32_t _algo_cfg_dump_util_param_parse_hex(int32_t fd, void *buf, uint32_t size, uint32_t type)
{
    int32_t ret = 0;
    uint32_t offset = 0;
    char *pbuf = (char *)buf;

    uint32_t accuracy = 0;
    uint32_t r_align = 0;
    uint32_t align = 0;
    uint32_t count_per_line = 0;
    uint32_t mul = 0;
    char fmt[32] = {0};
    char sfmt[32] = {0};

    if (size <= ALGO_PARAM_HEX_PERFIX_SIZE) {
        return -1;
    }

    accuracy = pbuf[offset++] & 0xFF;
    r_align = pbuf[offset++] & 0xFF;
    align = pbuf[offset++] & 0xFF;
    count_per_line = pbuf[offset++] & 0xFF;

    mul = align + 4;
    if (mul < ALGO_PARAM_HEX_FMT_LEN_MIN) {
        mul = ALGO_PARAM_HEX_FMT_LEN_MIN;
    }

    if (type == ALGO_DUMP_ITEM_TYPE_PARAM_INT) {
        snprintf(fmt, sizeof(fmt), "%s", "%d,");
    } else {
        if (accuracy > 0) {
            if (r_align & 0xF0) {
                snprintf(fmt, sizeof(fmt), "%s%d%s", "%.", accuracy, "lf,");
            } else {
                snprintf(fmt, sizeof(fmt), "%s%d%s", "%.", accuracy, "f,");
            }
        } else {
            if (r_align & 0xF0) {
                snprintf(fmt, sizeof(fmt), "%s", "%lf,");
            } else {
                snprintf(fmt, sizeof(fmt), "%s", "%f,");
            }
        }
    }

    if (align > 0) {
        if (r_align & 0x0F) {
            snprintf(sfmt, sizeof(sfmt), "%s%d%s", "%-", align, "s");
        } else {
            snprintf(sfmt, sizeof(sfmt), "%s%d%s", "%", align, "s");
        }
    } else {
        snprintf(sfmt, sizeof(sfmt), "%s", "%s");
    }

    pbuf += offset;
    if (type == ALGO_DUMP_ITEM_TYPE_PARAM_FLOAT) {
        DUMP_PARAM_HEX_PARSE(fd, pbuf, size - offset, float, fmt, sfmt, mul);
    } else if (type == ALGO_DUMP_ITEM_TYPE_PARAM_DOUBLE) {
        DUMP_PARAM_HEX_PARSE(fd, pbuf, size - offset, double, fmt, sfmt, mul);
    } else if (type == ALGO_DUMP_ITEM_TYPE_PARAM_INT) {
        DUMP_PARAM_HEX_PARSE(fd, pbuf, size - offset, int32_t, fmt, sfmt, mul);
    }
    return ret;
}

static int32_t _algo_cfg_dump_util_param_parse(int32_t fd, void *buf, uint32_t size)
{
    int32_t ret = 0;
    uint32_t offset = 0;
    char *pbuf = (char *)buf;

    uint32_t data_magic = 0;
    uint32_t data_len = 0;
    uint32_t data_type = 0;

    if ((buf == NULL) || (size == 0)) {
        return -1;
    }

    while (offset < size) {
        if (size < offset + sizeof(uint32_t) * 3) {
            ret = -1;
            break;
        }

        memcpy(&data_magic, pbuf + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        memcpy(&data_len, pbuf + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        memcpy(&data_type, pbuf + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        if (size < offset + data_len) {
            ret = -1;
            break;
        }

        if (data_magic != ALGO_DUMP_ITEM_MAGIC) {
            ret = -1;
            break;
        }

        if (data_type == ALGO_DUMP_ITEM_TYPE_STR) {
            silfp_ree_util_write_file(fd, pbuf + offset, data_len);
        } else {
            _algo_cfg_dump_util_param_parse_hex(fd, pbuf + offset, data_len, data_type);
        }

        offset += data_len;
    }

    return ret;
}

int32_t silfp_algo_cfg_dump_util_save(int32_t fd, void *buf, uint32_t size)
{
    int32_t ret = 0;
    uint32_t offset = 0;
    char *pbuf = (char *)buf;

    uint32_t data_magic = 0;
    uint32_t data_len = 0;
    uint32_t data_type = 0;

    if ((buf == NULL) || (size == 0)) {
        return -1;
    }

    while (offset < size) {
        if (size < offset + sizeof(uint32_t) * 3) {
            ret = -1;
            break;
        }

        memcpy(&data_magic, pbuf + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        memcpy(&data_len, pbuf + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        memcpy(&data_type, pbuf + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        if (size < offset + data_len) {
            ret = -1;
            break;
        }

        if (data_magic != ALGO_DUMP_ITEM_MAGIC) {
            ret = -1;
            break;
        }

        if (data_type == ALGO_DUMP_ITEM_TYPE_STR) {
            silfp_ree_util_write_file(fd, pbuf + offset, data_len);
        } else if (data_type == ALGO_DUMP_ITEM_TYPE_PARAM) {
            ret = _algo_cfg_dump_util_param_parse(fd, pbuf + offset, data_len);
            if (ret < 0) {
                break;
            }
        }

        offset += data_len;
    }

    return ret;
}