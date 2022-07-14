/******************************************************************************
 * @file   silead_cal_optic.c
 * @brief  Contains fingerprint calibrate operate functions.
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
 * David Wang  2018/7/2   0.1.0      Init version
 * Ashely Li   2021/1/15  0.2.0      Update copyright notice
 *
 *****************************************************************************/

#define FILE_TAG "silead_cal_opt"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>

#include "silead_const.h"
#include "silead_error.h"
#include "silead_cmd.h"
#include "silead_util.h"
#include "silead_dump.h"
#include "silead_cal.h"

#define FP_FEATURE_RESET_DOWN_FOR_FLASH_MASK 0x1000
#define FP_FEATURE_CAL_DATA_STORE_IN_TA 0x40000

#ifndef SIL_FP_CONFIG_PATH
#define SIL_FP_CONFIG_PATH "/persist/silead"
#endif

#define FP_OPTIC_CAL_NAME "fpcal.dat"
#define FP_OPTIC_CAL_NAME1 "fpcal1.dat"
#define FP_OPTIC_CAL_NAME2 "fpcal2.dat"
#define FP_OPTIC_CAL_NAME3 "fpcal3.dat"
#define FP_OPTIC_CAL_NAME4 "fpcal4.dat"
#define FP_OPTIC_CAL_NAME5 "fpcal5.dat"
#define FP_OPTIC_CAL_NAME6 "fpcal6.dat"

#define MAGIC 0x511EADCA
#define VERSION 1

#define CAL_FILE_SIZE_MAX (1024*1024)
#define BUF_SIZE 801*1024

#define CAL_INVALID 0x00
#define INIT_CAL_VALID 0x01
#define INIT_CAL_INVALID 0x02

#define CAL_MIN_SIZE 10
#define CAL_HEADER_SIZE 64

static int32_t m_reset_down_for_flash = 0;
static int32_t m_cal_data_in_ta = 0;

enum {
    FILE_STATE_NONE,
    FILE_STATE_COMBIN_VALID,
    FILE_STATE_SPLIT_VALID,
    FILE_STATE_ALL_INVALID,
};

enum {
    CAL_DATA_NONE = 0,
    CAL_DATA_COMBIN,
    CAL_DATA_UPDATA,
    CAL_DATA_VALID,
    CAL_DATA_INVALID,
};

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t random;
    uint32_t data_len;
    uint8_t cal_step[4];
    uint32_t cal_len[3];
    uint8_t data[];
} cal_data_t, *p_cal_data_t;

typedef struct {
    uint32_t malloced;
    uint32_t size;
    uint32_t flag;
    void *data;
} mem_buf_t, *p_mem_buf_t;

static p_cal_data_t m_cal_data = NULL;
static uint32_t m_cal_data_size = 0;
static uint8_t m_cal_file_state = FILE_STATE_NONE;
static int8_t m_cal_first_step_init = 0;
static uint8_t m_cal_upd_flag[3];

static char m_str_cal_path[MAX_PATH_LEN] = {0};
static const char* _cal_optic_get_path(void)
{
    if (m_str_cal_path[0] != '\0') {
        return m_str_cal_path;
    } else {
        return SIL_FP_CONFIG_PATH;
    }
}

void silfp_cal_optic_set_path(const void *path, uint32_t len)
{
    int32_t ret = 0;

    ret = silfp_util_path_copy(m_str_cal_path, sizeof(m_str_cal_path), path, len);
    if (ret < 0) {
        memset(m_str_cal_path, 0, sizeof(m_str_cal_path));
    }
    LOG_MSG_VERBOSE("path = %s", m_str_cal_path);
}

static int32_t _cal_optic_is_calibrate_step(uint32_t step)
{
    return ((step >= 1) && (step <= 3));
}

static int32_t _cal_optic_get_step_index(uint32_t step)
{
    int32_t ret = -1;
    int32_t i = 0;

    if (m_cal_data == NULL) {
        return -1;
    }

    for (i = 0; i < 3; i++) {
        if (m_cal_data->cal_step[i] == step) {
            ret = i;
            break;
        }
    }

    if (ret < 0) {
        for (i = 0; i < 3; i++) {
            if (m_cal_data->cal_step[i] == 0) {
                ret = i;
                break;
            }
        }
    }

    LOG_MSG_VERBOSE("step: %u --> %d", step, ret);

    return ret;
}

static int32_t _cal_optic_is_first_step(uint32_t step)
{
    return (step == 1) ? 1 : 0;
}

static void _cal_optic_flag_init(void)
{
    if (m_cal_data != NULL) {
        memset(m_cal_data, 0, m_cal_data_size + sizeof(cal_data_t));
    }

    m_cal_first_step_init = 0;
    m_cal_file_state = FILE_STATE_NONE;
    m_cal_upd_flag[0] = CAL_DATA_NONE;
    m_cal_upd_flag[1] = CAL_DATA_NONE;
    m_cal_upd_flag[2] = CAL_DATA_NONE;
}

static int32_t _cal_optic_get_data_result(int32_t result, uint8_t init, int32_t size)
{
#ifdef SIL_CODE_COMPATIBLE
    if ((size <= 0) || (init && (result <= CAL_MIN_SIZE))) {
        result = -1;
    }
#else
    if ((result < 0) || (size <= CAL_MIN_SIZE)) {
        result = -1;
    }
#endif

    return result;
}

static int32_t _cal_optic_get_cal_buf(void)
{
    int32_t ret = SL_SUCCESS;

    if (m_cal_data == NULL) {
        m_cal_data = (p_cal_data_t)malloc(CAL_FILE_SIZE_MAX);
        if (m_cal_data == NULL) {
            LOG_MSG_ERROR("malloc (%d) fail", CAL_FILE_SIZE_MAX);
            ret = -SL_ERROR_OUT_OF_MEMORY;
        } else {
            memset(m_cal_data, 0, CAL_FILE_SIZE_MAX);
        }
    }

    return ret;
}

static void _cal_optic_release_cal_buf(void)
{
    if (m_cal_data != NULL) {
        free(m_cal_data);
        m_cal_data = NULL;
    }
    _cal_optic_flag_init();
}

int32_t silfp_cal_optic_init(uint32_t feature)
{
    m_reset_down_for_flash = (feature & FP_FEATURE_RESET_DOWN_FOR_FLASH_MASK) ? 1 : 0;
    m_cal_data_in_ta = (feature & FP_FEATURE_CAL_DATA_STORE_IN_TA) ? 1 : 0;

    m_cal_data = NULL;
    m_cal_data_size = CAL_FILE_SIZE_MAX - sizeof(cal_data_t);

    return 0;
}

int32_t silfp_cal_optic_deinit(void)
{
    _cal_optic_release_cal_buf();
    return 0;
}

static const char* _cal_optic_get_name(uint32_t step, uint8_t combin)
{
    if (combin && _cal_optic_is_calibrate_step(step)) {
        return FP_OPTIC_CAL_NAME;
    }

    switch(step) {
    case 1:
        return FP_OPTIC_CAL_NAME1;
    case 2:
        return FP_OPTIC_CAL_NAME2;
    case 3:
        return FP_OPTIC_CAL_NAME3;
    case 4:
        return FP_OPTIC_CAL_NAME4;
    case 5:
        return FP_OPTIC_CAL_NAME5;
    case 6:
        return FP_OPTIC_CAL_NAME6;
    default:
        return NULL;
    }
}

static const char* _cal_optic_get_file(char *path, uint32_t size, uint32_t step, uint8_t combin)
{
    const char *file_name = NULL;

    if (path == NULL || size == 0) {
        return NULL;
    }

    file_name = _cal_optic_get_name(step, combin);
    if (file_name == NULL) {
        return NULL;
    }

    snprintf(path, size, "%s/%s", _cal_optic_get_path(), file_name);

    return file_name;
}

static int32_t _cal_optic_load_file(uint32_t step, void *buf, int32_t size, uint8_t combin)
{
    int32_t ret = 0;
    const char *file_name = NULL;
    char path[MAX_PATH_LEN] = {0};
    int32_t len = 0;

    file_name = _cal_optic_get_file(path, sizeof(path), step, combin);
    if (file_name == NULL) {
        return -SL_ERROR_CONFIG_INVALID;
    }

    len = silfp_util_file_get_size(path);
    LOG_MSG_DEBUG("get %s size %d", path, len);

    if (len < CAL_MIN_SIZE) {
        LOG_MSG_ERROR("data invalid %d", len);
        return -SL_ERROR_CONFIG_INVALID;
    }
    if (len > size) {
        LOG_MSG_ERROR("no enough buf, %d but %d", len, size);
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    ret = silfp_util_file_load(_cal_optic_get_path(), file_name, buf, len);
    if (ret != len) {
        LOG_MSG_ERROR("load data fail, %d but %d", len, ret);
        return -SL_ERROR_CONFIG_INVALID;
    }

    return ret;
}

static int32_t _cal_optic_get_base_sum_data(p_mem_buf_t mem, uint32_t step, uint8_t load)
{
    int32_t ret = 0;
    int32_t size = 0;

    if (mem == NULL) {
        LOG_MSG_ERROR("mem param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }
    memset(mem, 0, sizeof(mem_buf_t));

    if (_cal_optic_is_calibrate_step(step)) {
        LOG_MSG_ERROR("base sum step %d invalid", step);
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _cal_optic_get_cal_buf();
    if (ret < 0) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    memset(m_cal_data, 0, m_cal_data_size + sizeof(cal_data_t));

    size = m_cal_data_size;
    if (size > BUF_SIZE) {
        size = BUF_SIZE;
    }

    if (!load) { // get base sum in ta, not need load
        mem->flag = CAL_INVALID;
        mem->size = size;
    } else {
        ret = _cal_optic_load_file(step, m_cal_data->data, size, 0);
        if (ret < 0) { // file invalid
            mem->flag = INIT_CAL_INVALID;
            mem->size = size;
        } else {
            mem->flag = INIT_CAL_VALID;
            mem->size = ret;
        }
    }

    mem->malloced = 0;
    mem->data = m_cal_data->data;

    return SL_SUCCESS;
}

static void _cal_optic_save_base_sum_data(int32_t result, uint32_t step, uint8_t init, void *buf, int32_t size)
{
    int32_t ret = 0;

    if (_cal_optic_is_calibrate_step(step)) {
        LOG_MSG_ERROR("base sum step %d invalid", step);
        return;
    }

    if (buf == NULL) {
        LOG_MSG_ERROR("data buf invalid");
        return;
    }

    if (result >= 0 && size > 0) {
        result = _cal_optic_get_data_result(result, init, size);
        if (result >= 0) {
            silfp_util_file_save(_cal_optic_get_path(), _cal_optic_get_name(step, 0), buf, size);
        }
    }

    _cal_optic_release_cal_buf();
}

// new calibrate, not need load
static int32_t _cal_optic_get_calibrate_data_new(p_mem_buf_t mem, uint32_t step)
{
    int32_t ret = 0;
    int32_t size = 0;

    if (mem == NULL) {
        LOG_MSG_ERROR("mem param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }
    memset(mem, 0, sizeof(mem_buf_t));

    if (!_cal_optic_is_calibrate_step(step)) {
        LOG_MSG_ERROR("calibrate step %d invalid", step);
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _cal_optic_get_cal_buf();
    if (ret < 0) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    if (m_cal_data->data_len >= m_cal_data_size) {
        LOG_MSG_ERROR("not have enough buf");
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    size = m_cal_data_size - m_cal_data->data_len;
    if (size > BUF_SIZE) {
        size = BUF_SIZE;
    }

    mem->flag = CAL_INVALID;
    mem->size = size;
    mem->malloced = 0;
    mem->data = m_cal_data->data + m_cal_data->data_len;

    return SL_SUCCESS;
}

static int32_t _cal_optic_check_data_valid(void *buf, uint32_t len)
{
    p_cal_data_t cal_data = (p_cal_data_t)buf;
    uint32_t step_mask = 0;

    if (cal_data == NULL) {
        LOG_MSG_ERROR("buf invalid");
        return -1;
    }

    if (cal_data->magic != MAGIC) {
        LOG_MSG_ERROR("data invalid %08x", cal_data->magic);
        return -1;
    }

    if (cal_data->data_len > m_cal_data_size) {
        LOG_MSG_ERROR("data len invalid %d %d", cal_data->data_len, m_cal_data_size);
        return -1;
    }

    if (len != sizeof(cal_data_t) + cal_data->data_len) {
        LOG_MSG_ERROR("data len invalid %d:%d %d", sizeof(cal_data_t), cal_data->data_len, len);
        return -1;
    }

    if (cal_data->cal_len[0] + cal_data->cal_len[1] + cal_data->cal_len[2] != cal_data->data_len) {
        LOG_MSG_ERROR("data len invalid %d:%d:%d %d", cal_data->cal_len[0], cal_data->cal_len[1], cal_data->cal_len[2], cal_data->data_len);
        return -1;
    }

    if ((cal_data->cal_len[0] == 0) || ((cal_data->cal_len[1] == 0) && (cal_data->cal_len[2] != 0))) {
        LOG_MSG_ERROR("data len invalid %d:%d:%d", cal_data->cal_len[0], cal_data->cal_len[1], cal_data->cal_len[2]);
        return -1;
    }

    //step_mask = (1 << (cal_data->cal_step[0] - 1)) | (1 << (cal_data->cal_step[1] - 1)) | (1 << (cal_data->cal_step[2] - 1));

    LOG_MSG_DEBUG("step %d:%d:%d", cal_data->cal_step[0], cal_data->cal_step[1], cal_data->cal_step[2]);

    return 0;
}

static int32_t _cal_optic_get_calibrate_data_load_split(p_mem_buf_t mem, uint32_t step)
{
    int32_t ret = 0;
    int32_t size = 0;
    int32_t index = 0;

    if (mem == NULL) {
        LOG_MSG_ERROR("mem param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }
    memset(mem, 0, sizeof(mem_buf_t));

    if (m_cal_data->data_len >= m_cal_data_size) {
        LOG_MSG_ERROR("not have enough buf");
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    index = _cal_optic_get_step_index(step);
    if (index < 0) {
        LOG_MSG_ERROR("can't get valid index %d:%d:%d", m_cal_upd_flag[0], m_cal_upd_flag[1], m_cal_upd_flag[2]);
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    size = m_cal_data_size - m_cal_data->data_len;
    if (size > BUF_SIZE) {
        size = BUF_SIZE;
    }

    mem->data = (uint8_t *)malloc(size + CAL_HEADER_SIZE);
    if (mem->data == NULL) {
        LOG_MSG_ERROR("malloc (%d) fail", size);
        return -SL_ERROR_OUT_OF_MEMORY;
    }
    memset(mem->data, 0, size + CAL_HEADER_SIZE);
    mem->malloced = 1;

    ret = _cal_optic_load_file(step, mem->data, size, 0);
    if (ret < 0) { // file invalid
        mem->flag = INIT_CAL_INVALID;
        mem->size = size;
    } else {
        memcpy(m_cal_data->data + m_cal_data->data_len, mem->data, ret);
        m_cal_data->cal_step[index] = step;
        m_cal_data->cal_len[index] = ret + CAL_HEADER_SIZE;
        m_cal_data->data_len += ret + CAL_HEADER_SIZE;

        mem->flag = INIT_CAL_VALID;
        mem->size = ret + CAL_HEADER_SIZE;
    }

    return SL_SUCCESS;
}

// load calibrate data
static int32_t _cal_optic_get_calibrate_data_load(p_mem_buf_t mem, uint32_t step)
{
    int32_t ret = 0;
    int32_t size = 0;
    int32_t offset = 0;
    uint8_t invalid = 0;
    int32_t index = 0;

    if (mem == NULL) {
        LOG_MSG_ERROR("mem param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }
    memset(mem, 0, sizeof(mem_buf_t));

    if (!_cal_optic_is_calibrate_step(step)) {
        LOG_MSG_ERROR("calibrate step %d invalid", step);
        return -SL_ERROR_BAD_PARAMS;;
    }

    ret = _cal_optic_get_cal_buf();
    if (ret < 0) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    if (_cal_optic_is_first_step(step)) { // load combin cal file at first step
        ret = _cal_optic_load_file(step, m_cal_data, m_cal_data_size + sizeof(cal_data_t), 1);
        if ((ret > 0) && (_cal_optic_check_data_valid(m_cal_data, ret) >= 0)) {
            m_cal_file_state = FILE_STATE_COMBIN_VALID; // load combin file successful
        } else {
            m_cal_file_state = FILE_STATE_SPLIT_VALID; // assume spilt file is valid
            memset(m_cal_data, 0, m_cal_data_size + sizeof(cal_data_t));
        }
    }

    if (m_cal_file_state == FILE_STATE_COMBIN_VALID) {
        index = _cal_optic_get_step_index(step);
        if (index < 0) {
            LOG_MSG_ERROR("can't get valid index %d:%d:%d", m_cal_upd_flag[0], m_cal_upd_flag[1], m_cal_upd_flag[2]);
            return -SL_ERROR_BAD_PARAMS;
        }

        size = m_cal_data->cal_len[index];
        if (size <= 0) { // no data for this step
            size = BUF_SIZE;
            invalid = 1;
        }

        mem->data = malloc(size + CAL_HEADER_SIZE);
        if (mem->data == NULL) {
            LOG_MSG_ERROR("malloc (%d) fail", size);
            return -SL_ERROR_OUT_OF_MEMORY;
        }
        memset(mem->data, 0, size + CAL_HEADER_SIZE);
        mem->malloced = 1;

        if (!invalid) {
            offset = 0;
            if (index >= 1) {
                offset += m_cal_data->cal_len[0];
            }
            if (index == 2) {
                offset += m_cal_data->cal_len[1];
            }
            memcpy(mem->data, m_cal_data->data + offset, size);
            mem->flag = INIT_CAL_VALID;
        } else {
            mem->flag = INIT_CAL_INVALID;
        }
        mem->size = size + CAL_HEADER_SIZE;

        return SL_SUCCESS;
    }

    return _cal_optic_get_calibrate_data_load_split(mem, step);
}

static void _cal_optic_save_calibrate_data_and_clear(uint32_t step, uint8_t upd)
{
    int32_t ret = 0;
    int32_t i = 0;
    char path[MAX_PATH_LEN] = {0};
    int32_t count = 0;

    m_cal_data->magic = MAGIC;
    m_cal_data->version = VERSION;

    ret = _cal_optic_check_data_valid(m_cal_data, sizeof(cal_data_t) + m_cal_data->data_len);
    if (ret < 0) { // calibrate invalid
        return;
    }

    silfp_util_file_save(_cal_optic_get_path(), _cal_optic_get_name(step, 1), (void *)m_cal_data, sizeof(cal_data_t) + m_cal_data->data_len);

    if (upd) {
        count = 6;
    } else {
        count = 3;
    }

    // clear all base-sum data & split calibrate data
    for (i = 1; i <= count; i++) {
        if (_cal_optic_get_file(path, sizeof(path), i, 0) != NULL) {
            silfp_util_file_remove(path);
        }
    }
}

static int32_t _cal_optic_get_save_flag(void)
{
    if ((m_cal_upd_flag[0] == CAL_DATA_NONE) || (m_cal_upd_flag[1] == CAL_DATA_NONE) || (m_cal_upd_flag[2] == CAL_DATA_NONE)) {
        return CAL_DATA_NONE;
    }

    if ((m_cal_upd_flag[0] == CAL_DATA_INVALID) || (m_cal_upd_flag[1] == CAL_DATA_INVALID) || (m_cal_upd_flag[2] == CAL_DATA_INVALID)) {
        return CAL_DATA_INVALID;
    }

    if ((m_cal_upd_flag[0] == CAL_DATA_UPDATA) || (m_cal_upd_flag[1] == CAL_DATA_UPDATA) || (m_cal_upd_flag[2] == CAL_DATA_UPDATA)) {
        return CAL_DATA_UPDATA;
    }

    if ((m_cal_upd_flag[0] == CAL_DATA_COMBIN) || (m_cal_upd_flag[1] == CAL_DATA_COMBIN) || (m_cal_upd_flag[2] == CAL_DATA_COMBIN)) {
        return CAL_DATA_COMBIN;
    }

    return CAL_DATA_VALID;
}

static void _cal_optic_save_calibrate_data(int32_t result, uint32_t step, uint8_t init, void *buf, int32_t size)
{
    int32_t ret = 0;
    int32_t offset = 0;
    int32_t flag = CAL_DATA_NONE;
    int32_t index = 0;

    if (!_cal_optic_is_calibrate_step(step)) {
        LOG_MSG_ERROR("calibrate step %d invalid", step);
        return;
    }

    if (m_cal_data == NULL) {
        LOG_MSG_ERROR("data buf invalid");
        return;
    }

    index = _cal_optic_get_step_index(step);
    if (index < 0) {
        LOG_MSG_ERROR("can't get valid index %d:%d:%d", m_cal_upd_flag[0], m_cal_upd_flag[1], m_cal_upd_flag[2]);
        return;
    }

    if (result >= 0 && size >= 0) {
        result = _cal_optic_get_data_result(result, init, size);
        if (result >= 0) {
            if (m_cal_file_state == FILE_STATE_NONE) { // new calibrate
                if (size > (int32_t)(m_cal_data_size - m_cal_data->data_len)) {
                    LOG_MSG_ERROR("new: no enough remain buf %d but %d", size, m_cal_data_size - m_cal_data->data_len);
                } else {
                    memcpy(m_cal_data->data + m_cal_data->data_len, buf, size);
                    m_cal_data->cal_step[index] = step;
                    m_cal_data->cal_len[index] = size + CAL_HEADER_SIZE;
                    m_cal_data->data_len += size + CAL_HEADER_SIZE;
                }
            } else if (m_cal_file_state == FILE_STATE_COMBIN_VALID) { // combin mode
                if (size > (int32_t)m_cal_data->cal_len[index] && m_cal_data->cal_step[index] == step) {
                    LOG_MSG_ERROR("combin: cal db should upgrade %d %d", size, m_cal_data->cal_len[index]);
                } else {
                    if (index >= 1) {
                        offset += m_cal_data->cal_len[0];
                    }
                    if (index == 2) {
                        offset += m_cal_data->cal_len[1];
                    }
                    memcpy(m_cal_data->data + offset, buf, size);
                    if (m_cal_data->cal_step[index] != step) {
                        m_cal_data->cal_step[index] = step;
                        m_cal_data->cal_len[index] = size + CAL_HEADER_SIZE;
                        m_cal_data->data_len += size + CAL_HEADER_SIZE;
                    }
                }
            } else {
                if (size > (int32_t)(m_cal_data_size - m_cal_data->data_len)) {
                    LOG_MSG_ERROR("no enough remain buf %d but %d", size, m_cal_data_size - m_cal_data->data_len);
                } else {
                    memcpy(m_cal_data->data + m_cal_data->data_len, buf, size);
                    m_cal_data->cal_step[index] = step;
                    m_cal_data->cal_len[index] = size + CAL_HEADER_SIZE;
                    m_cal_data->data_len += size + CAL_HEADER_SIZE;
                }
            }
            m_cal_upd_flag[step-1] = CAL_DATA_UPDATA;
        } else if (m_cal_file_state == FILE_STATE_SPLIT_VALID) { // spit data & valid
            m_cal_upd_flag[step-1] = CAL_DATA_COMBIN;
        } else if (m_cal_file_state == FILE_STATE_COMBIN_VALID) { // already combin data & valid
            m_cal_upd_flag[step-1] = CAL_DATA_VALID;
        } else { // new calibrate
            m_cal_upd_flag[step-1] = CAL_DATA_VALID;
        }
    } else {
        m_cal_upd_flag[step-1] = CAL_DATA_INVALID;
    }

    flag = _cal_optic_get_save_flag();
    if (flag != CAL_DATA_NONE) { // last calibrate
        if (flag == CAL_DATA_UPDATA) {
            _cal_optic_save_calibrate_data_and_clear(step, 1);
        } else if (flag == CAL_DATA_COMBIN) {
            _cal_optic_save_calibrate_data_and_clear(step, 0);
        }
        _cal_optic_release_cal_buf();
    }
}

static int32_t _cal_optic_get_data_in_ta(p_mem_buf_t mem, uint8_t init)
{
    if (mem == NULL) {
        LOG_MSG_ERROR("mem param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }
    memset(mem, 0, sizeof(mem_buf_t));

    mem->data = malloc(MAX_PATH_LEN);
    if (mem->data == NULL) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    mem->malloced = 1;
    if (init)  {
        mem->flag = INIT_CAL_INVALID;
    } else {
        mem->flag = CAL_INVALID;
    }

    mem->size = MAX_PATH_LEN;
    snprintf(mem->data, MAX_PATH_LEN, "%s", _cal_optic_get_path());

    return SL_SUCCESS;
}

static int32_t _cal_optic_get_data(p_mem_buf_t mem, uint32_t step, uint8_t init)
{
    int32_t ret = 0;

    if (m_cal_data_in_ta) {
        ret = _cal_optic_get_data_in_ta(mem, init);
    } else if (_cal_optic_is_calibrate_step(step)) {
        if (_cal_optic_is_first_step(step)) { // first step, clear all data
            _cal_optic_flag_init();
            m_cal_first_step_init = 1;
        } else if (!m_cal_first_step_init) {
            LOG_MSG_ERROR("not run first step before %d", step);
            return -SL_ERROR_BAD_PARAMS;
        }
        if (init) {
            ret = _cal_optic_get_calibrate_data_load(mem, step);
        } else {
            ret = _cal_optic_get_calibrate_data_new(mem, step);
        }
    } else {
        ret = _cal_optic_get_base_sum_data(mem, step, init);
    }

    return ret;
}

static int32_t _cal_optic_save_data(int32_t result, uint32_t step, uint8_t init, p_mem_buf_t mem, int32_t size)
{
    LOG_MSG_DEBUG("result %d, step %d, size %d", result, step, size);

    if (mem == NULL) {
        LOG_MSG_ERROR("mem param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    if (!m_cal_data_in_ta) {
        if (_cal_optic_is_calibrate_step(step)) {
            _cal_optic_save_calibrate_data(result, step, init, mem->data, size);
        } else {
            _cal_optic_save_base_sum_data(result, step, init, mem->data, size);
        }
    }

    if (mem->malloced) {
        if (mem->data != NULL) {
            free(mem->data);
        }
    }

    return result;
}

static int32_t _cal_optic_load_data_in_flash(uint32_t step)
{
    int32_t ret = 0;
    uint8_t cal_step = step;

    if (!m_reset_down_for_flash) {
        return 0;
    }

    LOG_MSG_VERBOSE("need set reset down to read cal data from flash: %u", step);

    silfp_dev_pwdn(SIFP_PWDN_NONE);
    ret = silfp_cmd_send_cmd_with_buf(REQ_CMD_SUB_ID_LOAD_CAL_IN_FLASH, &cal_step, sizeof(cal_step));

    return ret;
}

static int32_t _cal_optic_save_data_in_flash(uint32_t step)
{
    int32_t ret = 0;
    uint8_t cal_step = step;

    if (!m_reset_down_for_flash) {
        return 0;
    }

    LOG_MSG_VERBOSE("need set reset down to save cal data from flash: %u", step);

    silfp_dev_pwdn(SIFP_PWDN_NONE);
    ret = silfp_cmd_send_cmd_with_buf(REQ_CMD_SUB_ID_SAVE_CAL_IN_FLASH, &cal_step, sizeof(cal_step));
    //silfp_cmd_download_normal();

    return ret;
}

static int32_t _cal_optic_step(uint32_t step, uint8_t init)
{
    int32_t ret = 0;
    mem_buf_t mem;
    int32_t len = 0;

    if (step < 1 || step > 6) { // step 1~5
        return ret;
    }

    memset(&mem, 0, sizeof(mem));
    ret = _cal_optic_get_data(&mem, step, init);
    if (ret < 0) {
        return ret;
    }

    if ((step >= 1) && (step <= 3)) {
        silfp_cmd_download_normal();
    }
    LOG_MSG_DEBUG("step %d, buf_flag %d", step, mem.flag);

    len = mem.size;
    ret = silfp_cmd_calibrate_optic(step, mem.data, (uint32_t *)&len, mem.flag);
    _cal_optic_save_data(ret, step, init, &mem, len);

    if ((step >= 1) && (step <= 3)) { // flash store
        if (ret >= 0) {
            ret = _cal_optic_save_data_in_flash(step);
        }
    }

    if ((step >= 1) && (step <= 3)) { // dump
        silfp_dump_data(DUMP_IMG_CAL);
    }

    return ret;
}

int32_t silfp_cal_optic_step(uint32_t step)
{
    int32_t ret = _cal_optic_step(step, 0);;

    if (step == 5) {
        ret |= _cal_optic_step(step+1, 0);
    }
    return ret;
}

int32_t silfp_cal_optic_calibrate(void)
{
    int32_t ret = 0;

    _cal_optic_load_data_in_flash(1);
    ret = _cal_optic_step(1, 1);

    _cal_optic_load_data_in_flash(2);
    ret |= _cal_optic_step(2, 1);

    _cal_optic_load_data_in_flash(3);
    ret |= _cal_optic_step(3, 1);

    ret |= _cal_optic_step(4, 1);
    ret |= _cal_optic_step(5, 1);
    ret |= _cal_optic_step(6, 1);

    return ret;
}
