/******************************************************************************
 * @file   silead_dump_v1.c
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

#ifdef SIL_DUMP_IMAGE

#define FILE_TAG "dump_v1"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>

#include "silead_bmp.h"
#include "silead_cmd.h"
#include "silead_util.h"
#include "silead_dump_param.h"
#include "silead_dump_name.h"
#include "silead_dump_path.h"
#include "silead_dump_v1.h"
#include "silead_dump.h"
#include "silead_const.h"

#define TEST_DUMP_DATA_TYPE_IMG         0x51163731
#define TEST_DUMP_DATA_TYPE_NAV         0x51163732
#define TEST_DUMP_DATA_TYPE_RAW         0x51163733
#define TEST_DUMP_DATA_TYPE_FT_QA       0x51163734
#define TEST_DUMP_DATA_TYPE_SNR         0x51163735
#define TEST_DUMP_DATA_TYPE_ENROLL_NEW  0x51163736
#define TEST_DUMP_DATA_TYPE_CAL         0x51163737
#define TEST_DUMP_DATA_TYPE_AUTH        0x51163738
#define TEST_DUMP_DATA_TYPE_MUL_RAW     0x51163739

#define DUMP_STEP_MAX 70
#define DUMP_EXT_SIZE_MAX 512

static int32_t _dump_get_mode(int32_t type)
{
    int32_t mode = TEST_DUMP_DATA_TYPE_IMG;

    switch(type) {
    case DUMP_IMG_NAV_SUCC:
    case DUMP_IMG_NAV_FAIL: {
        mode = TEST_DUMP_DATA_TYPE_NAV;
        break;
    }
    case DUMP_IMG_RAW:
    case DUMP_IMG_AUTH_ORIG:
    case DUMP_IMG_ENROLL_ORIG:
    case DUMP_IMG_OTHER_ORIG: {
        mode = TEST_DUMP_DATA_TYPE_RAW;
        break;
    }
    case DUMP_IMG_CAL: {
        mode = TEST_DUMP_DATA_TYPE_CAL;
        break;
    }
    case DUMP_IMG_FT_QA: {
        mode = TEST_DUMP_DATA_TYPE_FT_QA;
        break;
    }
    case DUMP_IMG_SNR: {
        mode = TEST_DUMP_DATA_TYPE_SNR;
        break;
    }
    case DUMP_IMG_ENROLL_NEW: {
        mode = TEST_DUMP_DATA_TYPE_ENROLL_NEW;
        break;
    }
    case DUMP_IMG_AUTH_MUL_RAW: {
        mode = TEST_DUMP_DATA_TYPE_MUL_RAW;
        break;
    }
#ifndef SIL_CODE_COMPATIBLE // ????
    case DUMP_IMG_AUTH_SUCC:
    case DUMP_IMG_AUTH_FAIL: {
        mode = TEST_DUMP_DATA_TYPE_AUTH;
        break;
    }
#endif
    default: {
        mode = TEST_DUMP_DATA_TYPE_IMG;
        break;
    }
    }
    return mode;
}

static int32_t _dump_get_v2_type(int32_t type, int32_t *imgtype, int32_t *subtype)
{
    switch (type) {
    case DUMP_IMG_AUTH_SUCC: {
        *imgtype = DUMP_IMG_TYPE_AUTH;
        *subtype = DUMP_IMG_SUBTYPE_RAW_SUCC;
        break;
    }
    case DUMP_IMG_AUTH_FAIL: {
        *imgtype = DUMP_IMG_TYPE_AUTH;
        *subtype = DUMP_IMG_SUBTYPE_RAW_FAIL;
        break;
    }
    case DUMP_IMG_AUTH: {
        *imgtype = DUMP_IMG_TYPE_AUTH;
        *subtype = DUMP_IMG_SUBTYPE_RAW;
        break;
    }
    case DUMP_IMG_ENROLL_SUCC: {
        *imgtype = DUMP_IMG_TYPE_ENROLL;
        *subtype = DUMP_IMG_SUBTYPE_RAW_SUCC;
        break;
    }
    case DUMP_IMG_ENROLL_FAIL: {
        *imgtype = DUMP_IMG_TYPE_ENROLL;
        *subtype = DUMP_IMG_SUBTYPE_RAW_FAIL;
        break;
    }
    case DUMP_IMG_ENROLL: {
        *imgtype = DUMP_IMG_TYPE_ENROLL;
        *subtype = DUMP_IMG_SUBTYPE_RAW;
        break;
    }
    case DUMP_IMG_NAV_SUCC: {
        *imgtype = DUMP_IMG_TYPE_NAV;
        *subtype = DUMP_IMG_SUBTYPE_RAW_SUCC;
        break;
    }
    case DUMP_IMG_NAV_FAIL: {
        *imgtype = DUMP_IMG_TYPE_NAV;
        *subtype = DUMP_IMG_SUBTYPE_RAW_FAIL;
        break;
    }
    case DUMP_IMG_SHOT_SUCC: {
        *imgtype = DUMP_IMG_TYPE_ENROLL;
        *subtype = DUMP_IMG_SUBTYPE_SHOT_FAIL;
        break;
    }
    case DUMP_IMG_SHOT_FAIL: {
        *imgtype = DUMP_IMG_TYPE_ENROLL;
        *subtype = DUMP_IMG_SUBTYPE_SHOT_FAIL;
        break;
    }
    case DUMP_IMG_RAW: {
        *imgtype = DUMP_IMG_TYPE_AUTH;
        *subtype = DUMP_IMG_SUBTYPE_RAW_SUCC;
        break;
    }
    case DUMP_IMG_CAL: {
        *imgtype = DUMP_IMG_TYPE_CAL;
        *subtype = DUMP_IMG_SUBTYPE_ORIG;
        break;
    }
    case DUMP_IMG_FT_QA: {
        *imgtype = DUMP_IMG_TYPE_FT_QA;
        *subtype = DUMP_IMG_SUBTYPE_ORIG;
        break;
    }
    case DUMP_IMG_AUTH_ORIG: {
        *imgtype = DUMP_IMG_TYPE_AUTH;
        *subtype = DUMP_IMG_SUBTYPE_ORIG;
        break;
    }
    case DUMP_IMG_ENROLL_ORIG: {
        *imgtype = DUMP_IMG_TYPE_ENROLL;
        *subtype = DUMP_IMG_SUBTYPE_ORIG;
        break;
    }
    case DUMP_IMG_OTHER_ORIG: {
        *imgtype = DUMP_IMG_TYPE_OTHER;
        *subtype = DUMP_IMG_SUBTYPE_ORIG;
        break;
    }
    case DUMP_IMG_OTHER: {
        *imgtype = DUMP_IMG_TYPE_OTHER;
        *subtype = DUMP_IMG_SUBTYPE_RAW_SUCC;
        break;
    }
    case DUMP_IMG_SNR: {
        *imgtype = DUMP_IMG_TYPE_SNR;
        *subtype = DUMP_IMG_SUBTYPE_ORIG;
        break;
    }
    case DUMP_IMG_ENROLL_NEW: {
        *imgtype = DUMP_IMG_TYPE_ENROLL;
        *subtype = DUMP_IMG_SUBTYPE_NEW;
        break;
    }
    case DUMP_IMG_AUTH_MUL_RAW: {
        *imgtype = DUMP_IMG_TYPE_AUTH;
        *subtype = DUMP_IMG_SUBTYPE_MRAW;
        break;
    }
    }
    return 0;
}

static int32_t _dump_get_ext_info(void *path, uint32_t size, void *extbuf, uint32_t *extsize)
{
    int32_t ret = 0;
    void *buf = NULL;
    uint8_t *data = NULL;
    uint32_t len = 1024;

    uint32_t result = 0;
    uint32_t result_path_len = 0;
    uint32_t result_ext_len = 0;

    if (path == NULL || size == 0) {
        return -1;
    }

    buf = malloc(len);
    if (buf == NULL) {
        LOG_MSG_ERROR("malloc buffer failed");
        return -1;
    }

    memset(buf, 0, len);
    silfp_util_path_copy(buf, len, path, strlen(path));

    ret = silfp_cmd_send_cmd_with_buf_and_get(REQ_CMD_SUB_ID_DUMP_FILE_EXT, buf, &len, &result);
    if ((ret < 0) || (result == 0)) {
        LOG_MSG_ERROR("not have ext info (%d %d)", ret, result);
        ret = -1;
    } else {
        result_path_len = (result & 0x0000FFFF);
        result_ext_len = ((result >> 16) & 0x0000FFFF);
        if (result_path_len + result_ext_len > len) {
            LOG_MSG_ERROR("param invalid, (%u %u:%u)", len, result_path_len, result_ext_len);
            ret = -1;
        } else {
            if (result_path_len > 0) {
                silfp_util_path_copy(path, size, buf, result_path_len);
            }
            if (result_ext_len > 0 && extbuf != NULL && extsize != NULL && *extsize > 0) {
                if (*extsize > result_ext_len) {
                    *extsize = result_ext_len;
                } else {
                    LOG_MSG_ERROR("not have enough buffer (%d but %d), split", result_ext_len, *extsize);
                }
                data = (uint8_t *)buf;
                memcpy(extbuf, data + result_path_len, *extsize);
            } else {
                ret = -1;
            }
        }
    }

    free(buf);
    return ret;
}

static void _dump_data_v1_internal(int32_t type, void *buf, uint32_t buf_size, void *ext_buf, uint32_t ext_size, uint32_t name_mode)
{
    int32_t ret = 0;

    uint32_t w = 0;
    uint32_t h = 0;
    uint32_t imgsize = 0;
    uint8_t bitcount = 0;
    uint32_t dump_step = 0;
    uint32_t dump_remaining = 0;
    uint32_t mode = TEST_DUMP_DATA_TYPE_IMG;

    char path[MAX_PATH_LEN] = {0};
    char name[MAX_PATH_LEN] = {0};
    uint32_t dump_path_mode = 0;
    int32_t imgtype = 0;
    int32_t subtype = 0;

    mode = _dump_get_mode(type);
    _dump_get_v2_type(type, &imgtype, &subtype);

    do {
        memset(buf, 0, buf_size);
        ret = silfp_cmd_test_dump_data(mode, dump_step, buf, buf_size, &dump_remaining, &imgsize, &w, &h, &bitcount);
        if (imgsize > 0) {
            LOG_MSG_DEBUG("%d (%d:%d:%d)", imgsize, w, h, bitcount);

            dump_path_mode = name_mode;
            silfp_dump_path_get_save_path(path, sizeof(path), imgtype, subtype, dump_path_mode);
            if ((TEST_DUMP_DATA_TYPE_CAL == mode) && (CHECK_FLAG_SET(dump_path_mode, DUMP_NAME_MODE_CAL_TIMESTAMP_NONE))) {
                dump_path_mode |= DUMP_NAME_MODE_TIMESTAMP_NONE;
            }

            if (silfp_dump_name_get_save_name_simple(name, sizeof(name), silfp_dump_path_get(), imgtype, subtype, dump_path_mode) >= 0) {
                ext_size = DUMP_EXT_SIZE_MAX;
                memset(ext_buf, 0, ext_size);
                if (_dump_get_ext_info(name, sizeof(name), ext_buf, &ext_size) < 0) {
                    ext_size = 0;
                }
                silfp_bmp_save(path, name, buf, imgsize, w, h, bitcount, ext_buf, ext_size, 0);
            }
        }
        dump_step++;
    } while (ret >= 0 && dump_step < DUMP_STEP_MAX && dump_remaining > 0);
}

void silfp_dump_data_v1(int32_t type, void *buf, uint32_t buf_size, uint32_t name_mode)
{
    void *ext_buf = NULL;
    uint32_t ext_size = DUMP_EXT_SIZE_MAX;

    ext_buf = malloc(ext_size);
    if (ext_buf == NULL) {
        LOG_MSG_ERROR("malloc ext buffer (%d) failed", ext_size);
        return;
    }

    _dump_data_v1_internal(type, buf, buf_size, ext_buf, ext_size, name_mode);

    free(ext_buf);
}

#endif /* SIL_DUMP_IMAGE */