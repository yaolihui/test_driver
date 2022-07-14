/******************************************************************************
 * @file   silead_ext.c
 * @brief  Contains fingerprint extension operate functions.
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
 * Jack Zhang  2018/7/2   0.1.0      Init version
 * Ashely Li   2021/1/15  0.2.0      Update copyright notice
 *
 *****************************************************************************/

#define FILE_TAG "silead_ext"
#include "log/logmsg.h"

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "silead_const.h"
#include "silead_error.h"
#include "silead_dump.h"
#include "silead_cmd.h"
#include "silead_util.h"
#include "silead_cust.h"
#include "silead_impl.h"

#define BUF_SIZE 132*1024

#ifndef SIL_FP_CONFIG_PATH
#define SIL_FP_CONFIG_PATH "/persist/silead"
#endif
#define FP_OPTIC_DEADPX_NAME "fpdeadpx.dat"

static int32_t m_ext_capture_image_enable = 1;
static char m_str_path[MAX_PATH_LEN] = {0};

void silfp_ext_set_capture_image(int32_t enable)
{
    m_ext_capture_image_enable = enable;
}

static const char *_ext_get_path(void)
{
    if (m_str_path[0] != '\0') {
        return m_str_path;
    } else {
        return SIL_FP_CONFIG_PATH;
    }
}

void silfp_ext_set_path(const void *path, uint32_t len)
{
    int32_t ret = 0;

    ret = silfp_util_path_copy(m_str_path, sizeof(m_str_path), path, len);
    if (ret < 0) {
        memset(m_str_path, 0, sizeof(m_str_path));
    }
    LOG_MSG_VERBOSE("path = %s", m_str_path);
}

int32_t silfp_ext_test_spi(uint32_t *chipid, uint32_t *subid)
{
    silfp_cmd_download_normal();

    return silfp_cmd_get_chipid(chipid, subid);
}

static int32_t _ext_test_sram(uint32_t *result, uint32_t *deadpx, uint32_t *deadpx_center, uint8_t init)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    char *buf = NULL;
    int32_t len = BUF_SIZE;
    uint32_t num = 0;
    char path[MAX_PATH_LEN] = {0};

    if (result == NULL || deadpx == NULL || deadpx_center == NULL) {
        return -SL_ERROR_BAD_PARAMS;
    }

    snprintf(path, sizeof(path), "%s/%s", _ext_get_path(), FP_OPTIC_DEADPX_NAME);

    if (init) {
        len = silfp_util_file_get_size(path);
        LOG_MSG_DEBUG("get %s size %d", path, len);

        if (len < 10) {
            len = BUF_SIZE;
        }
    }

    buf = malloc(len);
    if (NULL == buf) {
        LOG_MSG_ERROR("Malloc fail!!!");
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    memset(buf, 0, len);
    if (init && (len != BUF_SIZE)) {
        ret = silfp_util_file_load(_ext_get_path(), FP_OPTIC_DEADPX_NAME, buf, len);
    }

    ret = silfp_cmd_send_cmd_with_buf_and_get(REQ_CMD_SUB_ID_SRAM_TEST, buf, (uint32_t *)&len, &num);
    silfp_util_file_save(_ext_get_path(), FP_OPTIC_DEADPX_NAME, buf, len);

    *deadpx = (num >> 16) & 0xFFFF;
    *deadpx_center = num & 0xFFFF;

    if (ret < 0) {
        *result = EXT_RESULT_FAIL;
    } else {
        *result = ret;
    }

    snprintf(buf, BUF_SIZE, "total-deadpix=%d, center-deadpix=%d, result=%d\n", *deadpx, *deadpx_center, *result);
    //silfp_dump_test_result("deadpx.dat", buf, strlen(buf));

    free(buf);
    buf = NULL;

    return ret;
}

int32_t silfp_ext_test_deadpx(int32_t optic, uint32_t *result, uint32_t *deadpx, uint32_t *badline)
{
    int32_t ret = 0;

    silfp_cmd_download_normal();

    if (optic) {
        ret = _ext_test_sram(result, deadpx, badline, 0);
    } else {
        ret = silfp_cmd_test_deadpx(result, deadpx, badline);
    }
    return ret;
}

int32_t silfp_ext_test_self(int32_t optic, uint32_t *result)
{
    int32_t ret = 0;
    uint32_t chipId = 0;
    uint32_t subId = 0;

    uint32_t deadpx_result = EXT_RESULT_PASS;
    uint32_t deadpx = 0;
    uint32_t badline = 0;

    ret = silfp_ext_test_spi(&chipId, &subId);
    if (ret >= 0) {
        LOG_MSG_INFO("spi test successed (%x,%x)", chipId, subId);
        ret = silfp_ext_test_deadpx(optic, &deadpx_result, &deadpx, &badline);
        LOG_MSG_INFO("silfp_ext_test_deadpx: result=%d, deadpx=%d, badline=%d", deadpx_result, deadpx, badline);
    }

    if (result != NULL) {
        *result = deadpx_result;
    }

    return ret;
}

int32_t silfp_ext_test_flash(void)
{
    uint32_t tmp = 0;
    uint32_t len = sizeof(tmp);
    uint32_t result = 0;

    silfp_cmd_download_normal();

    return silfp_cmd_send_cmd_with_buf_and_get(REQ_CMD_SUB_ID_FLASH_TEST, (uint8_t *)&tmp, &len, &result);
}

int32_t silfp_ext_test_otp(uint32_t *otp1, uint32_t *otp2, uint32_t *otp3)
{
    silfp_cmd_download_normal();
    return silfp_cmd_get_otp(otp1, otp2, otp3);
}

int32_t silfp_ext_test_get_image_info(uint32_t *w, uint32_t *h, uint32_t *max_size, uint32_t *w_ori, uint32_t *h_ori, uint8_t *bitcount, uint8_t *bitcount_orig)
{
    return silfp_cmd_test_get_image_info(w, h, max_size, w_ori, h_ori, bitcount, bitcount_orig);
}

int32_t silfp_ext_test_image_test_init(uint32_t mode, uint32_t count, uint32_t step)
{
    int32_t ret = 0;
    uint8_t data[3] = {0};

    data[0] = (mode & 0xFF);
    data[1] = (count & 0xFF);
    data[2] = (step & 0xFF);

    ret = silfp_cmd_send_cmd_with_buf(REQ_CMD_SUB_ID_TEST_IMAGE_INIT, data, sizeof(data));
    return ret;
}

int32_t silfp_ext_test_image_test(uint32_t mode, void *buffer, uint32_t *len, uint8_t *quality, uint8_t *area, uint8_t *istpl, uint8_t *greyavg, uint8_t *greymax)
{
    int32_t ret = 0;

    uint32_t temp = 0;
    uint32_t size = sizeof(temp);

#ifdef SIL_FP_EXT_CAPTURE_ENABLE
    if (!m_ext_capture_image_enable) {
        mode &= (~EXT_IMG_FEATURE_DATA_MASK);
    }
#else
    mode &= (~EXT_IMG_FEATURE_DATA_MASK);
#endif

    LOG_MSG_DEBUG("mode: 0x%x", mode);

    if (buffer == NULL || len == 0 || *len == 0) {
        ret = silfp_cmd_test_image_capture(mode, (void *)(&temp), &size, quality, area, istpl, greyavg, greymax);
    } else {
        ret = silfp_cmd_test_image_capture(mode, buffer, len, quality, area, istpl, greyavg, greymax);
    }
    return ret;
}

int32_t silfp_ext_test_send_group_image(uint32_t orig, uint32_t frr, uint32_t imgtype, void *buffer, uint32_t *len)
{
    return silfp_cmd_test_send_group_image(orig, frr, imgtype, buffer, len);
}

int32_t silfp_ext_test_image_finish(void)
{
    return silfp_cmd_test_image_finish();
}

int32_t silfp_ext_test_get_ta_ver(uint32_t *algoVer, uint32_t *taVer)
{
    return silfp_cmd_get_ta_ver(algoVer, taVer);
}

int32_t silfp_ext_optic_test_factory_quality(uint32_t *result, uint32_t *quality, uint32_t *length)
{
    int32_t ret = 0;
    uint32_t qa_result[3] = {0};
    uint32_t len = sizeof(qa_result);
    uint32_t qa_ret = 0;

    silfp_cmd_download_normal();

    ret = silfp_cmd_send_cmd_with_buf_and_get(REQ_CMD_SUB_ID_OPTIC_FACTORY_QUALITY, qa_result, &len, &qa_ret);
    LOG_MSG_DEBUG("ret: %d, result: %u, quality:%u, length:%u", ret, qa_ret, qa_result[0], qa_result[1]);

    silfp_dump_data(DUMP_IMG_FT_QA);

    if (NULL != result) {
        *result = qa_ret;
    }
    if (NULL != quality) {
        *quality = qa_result[0];
    }
    if (NULL != length) {
        *length = qa_result[1];
    }

    return ret;
}

int32_t silfp_ext_optic_test_snr(uint32_t *result, uint32_t *snr, uint32_t *noise, uint32_t *signal)
{
    int32_t ret = 0;
    uint32_t snr_result[8] = {0};
    uint32_t len = sizeof(snr_result);
    uint32_t snr_ret = 0;
    char *buf = NULL;

#ifdef SIL_CODE_COMPATIBLE // ????
    uint32_t snr_finish_result[4] = {0};
    uint32_t snr_finish_len = sizeof(snr_finish_result);
    uint32_t finish_result = 0;
#endif

    silfp_cmd_download_normal();

    ret = silfp_cmd_send_cmd_with_buf_and_get(REQ_CMD_SUB_ID_SNR_TEST, (uint8_t *)snr_result, &len, &snr_ret);
    LOG_MSG_DEBUG("ret: %d, result:%u, snr:%u, noise:%u, signal:%u", ret, snr_ret, snr_result[0], snr_result[1], snr_result[2]);

    silfp_dump_data(DUMP_IMG_SNR);

#ifdef SIL_CODE_COMPATIBLE // ????
    silfp_cmd_send_cmd_with_buf_and_get(0x80000001, (uint8_t *)snr_finish_result, &snr_finish_len, &finish_result);
#else
    silfp_cmd_send_cmd_with_buf(REQ_CMD_SUB_ID_SNR_TEST_FINISH, NULL, 0);
#endif

    if (NULL != result) {
        *result = snr_ret;
    }
    if (NULL != snr) {
        *snr = snr_result[0];
    }
    if (NULL != noise) {
        *noise = snr_result[1];
    }
    if (NULL != signal) {
        *signal = snr_result[2];
    }

    buf = malloc(1024);
    if (buf != NULL) {
        snprintf(buf, 1024, "result=%u, snr=%u, noise=%u, signal=%u %d[%x,%x,%x,%x,%x]\n", snr_ret, snr_result[0], snr_result[1], snr_result[2],
                 (int32_t)(len/sizeof(uint32_t)) - 3, snr_result[3], snr_result[4], snr_result[5], snr_result[6], snr_result[7]);
        //silfp_dump_test_result("snr.dat", buf, strlen(buf));
        free(buf);
    }

    return ret;
}

int32_t silfp_ext_test_speed(void *buffer, uint32_t *len)
{
    return silfp_cmd_test_speed(buffer, len);
}

int32_t silfp_ext_test_reset(void)
{
    int32_t ret = 0;
    uint32_t chip_id = 0;
    uint32_t sub_id = 0;

    LOG_MSG_DEBUG("reset test");
    if (!silfp_cust_reset_test_skip_pwdn()) {
        silfp_dev_pwdn(SIFP_PWDN_NONE);
        ret = silfp_cmd_get_chipid(&chip_id, &sub_id);
        if (ret >= 0) {
            LOG_MSG_DEBUG("test fail, reset down but id (0x%x:0x%x)", chip_id, sub_id);
            ret = -SL_ERROR_BAD_PARAMS;
        } else {
            silfp_cmd_download_normal();
            ret = silfp_cmd_get_chipid(&chip_id, &sub_id);
            if (ret >= 0) {
                ret = 0;
                LOG_MSG_DEBUG("test success");
            } else {
                LOG_MSG_DEBUG("test fail, reset up but not id");
            }
        }
    } else {
        silfp_cmd_download_normal();
        ret = silfp_cmd_get_chipid(&chip_id, &sub_id);
        if (ret >= 0) {
            ret = 0;
            LOG_MSG_DEBUG("test success");
        } else {
            LOG_MSG_DEBUG("test fail, reset up but not id");
        }
    }
    LOG_MSG_DEBUG("test reset ret= %d", ret);

    return ret;
}

static int32_t _ext_image_test_need_capture(int32_t id)
{
    if ((TEST_IMAGE_ID_CAPTURE == id) || (TEST_IMAGE_ID_QUALITY == id)) {
        return 1;
    }
    return 0;
}

static int32_t _ext_image_test_need_quality(int32_t id)
{
    if (TEST_IMAGE_ID_QUALITY == id) {
        return 1;
    }
    return 0;
}

static uint8_t _ext_image_test_check_quality(uint8_t quality)
{
    uint8_t quality_thr = silfp_cust_get_quality_threshold();
    if (quality < quality_thr) {
        return -1;
    }
    return 0;
}

int32_t silfp_ext_capture_image_test(int32_t id, uint8_t *p_quality, uint8_t *p_result)
{
    int32_t ret = 0;
    uint8_t quality = 0;
    uint8_t result = 0;

    ret = silfp_impl_wait_finger_up_with_cancel();
    if (ret >= 0) {
        ret = silfp_impl_get_finger_down_with_cancel();
    }

    if ((ret >= 0) && _ext_image_test_need_capture(id)) {
        silfp_cust_set_hbm_mode(1);
        ret = silfp_impl_capture_image(IMG_CAPTURE_QUALITY_TEST, 0);
        silfp_cust_restore_hbm();
        if (ret < 0) {
            LOG_MSG_DEBUG("capture image fail %d", ret);
        }
    }

    if ((ret >= 0) && _ext_image_test_need_quality(id)) {
        ret = silfp_ext_test_image_test(EXT_IMG_FEATURE_QAULITY_MASK, NULL, NULL, &quality, NULL, NULL, NULL, NULL);
        if (ret >= 0) {
            result = _ext_image_test_check_quality(quality);
        }
        LOG_MSG_DEBUG("ret=%d, quality=%d, result=%d", ret, quality, result);
    }

    if (p_result != NULL) {
        *p_result = result;
    }
    if (p_quality != NULL) {
        *p_quality = quality;
    }

    return ret;
}

