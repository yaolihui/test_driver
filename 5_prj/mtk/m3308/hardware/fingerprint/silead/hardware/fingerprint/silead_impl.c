/******************************************************************************
 * @file   silead_impl.c
 * @brief  Contains CA implements.
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
 * David Wang  2018/4/2   0.1.0      Init version
 * David Wang  2018/5/15  0.1.1      Support get finger status
 * John Zhang  2018/5/15  0.1.2      Support load/save config
 * Jack Zhang  2018/5/17  0.1.3      Change test process to simplify app use
 * Rich Li     2018/5/28  0.1.4      Add get enroll number command ID.
 * Davie Wang  2018/6/1   0.1.5      Add capture image sub command ID.
 * David Wang  2018/6/5   0.1.6      Support wakelock & pwdn
 * Rich Li     2018/6/7   0.1.7      Support dump image
 * Jack Zhang  2018/6/15  0.1.8      Add read OTP I/F.
 * Rich Li     2018/7/2   0.1.9      Add algo set param command ID.
 * Ashely Li   2021/1/15  0.2.0      Update copyright notice
 *
 *****************************************************************************/

#define FILE_TAG "silead_impl"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "silead_const.h"
#include "silead_error.h"
#include "silead_cmd.h"
#include "silead_dev.h"
#include "silead_debug.h"
#include "silead_dump.h"
#include "silead_storage.h"
#include "silead_version.h"
#include "silead_cust.h"
#include "silead_worker.h"
#include "silead_cal.h"
#include "silead_impl.h"
#include "silead_ext.h"
#include "silead_util.h"
#include "silead_ext_skt.h"
#include "silead_bmp.h"
#include "silead_misc.h"

typedef struct fp_ta_feature_bit {
    uint32_t store_normal:1;            // 0x0001, store template in android
    uint32_t tpl_update:1;              // 0x0002, support tpl update after auth success
    uint32_t reinit_after_irq:1;        // 0x0004, need reset after receive irq
    uint32_t need_cal:1;                // 0x0008, need calibrate (capacitive only)
    uint32_t need_cal_finger_up:1;      // 0x0010, need wait finger up (capacitive only)
    uint32_t need_cal2:1;               // 0x0020, need calibrate2 (capacitive only)
    uint32_t finger_loop:1;             // 0x0040, need finger loop to get finger state (capacitive only)
    uint32_t irq_pwdn:1;                // 0x0080, reset down when wait finger down/up (optic only)
    uint32_t irq_pwdn_avdd_down:1;      // 0x0100, avdd down when wait finger down/up (optic only)
    uint32_t is_optic:1;                // 0x0200, optic module
    uint32_t :1;                        // 0x0400, reserved
    uint32_t esd_unsupport:1;           // 0x0800, not support esd (capacitive only)
    uint32_t reset_down_for_flash:1;    // 0x1000, need reset down when operate flash
    uint32_t auth_retry:4;              // 0x2000~10000, auth retry times
    uint32_t watermark:1;               // 0x20000, add watermark on bmp
    uint32_t cal_in_ta:1;               // 0x40000, store cal file in ta
    uint32_t dump_img_mode:1;           // 0x80000, dump img mode(SILEAD RA)
    uint32_t dump_img_encrypt:1;        // 0x100000, dump img encrypt
    uint32_t is_dump_v2:1;              // 0x200000, is dump v2
    uint32_t is_ocp:1;                  // 0x400000, is OCP enabled
} fp_ta_feature_bit_t;

typedef union fp_ta_feature {
    fp_ta_feature_bit_t bit;
    uint32_t feature;
} fp_ta_feature_t;

typedef struct fp_ta_otp_bit {
    uint32_t reserved:14;
    uint32_t year:2;
    uint32_t day:5;
    uint32_t month:4;
    uint32_t ver:3;
    uint32_t vendor_id:4;
    uint32_t reserved32[2];
} fp_ta_otp_bit_t;

typedef union fp_ta_otp {
    fp_ta_otp_bit_t info;
    uint32_t raw[3];
} fp_ta_otp_t;

typedef struct fp_ta_otp_map {
    char    *str;
} fp_ta_otp_map_t;

static const fp_ta_otp_map_t s_otp_vendor[] = {
    {"OFILM\0"},
    {"QTECH\0"},
    {"PRIMAX\0"},
    {"HOLITECH\0"},
    {"HNLENS\0"},
    {"TRULY\0"},
    {"TOPTOUCH\0"},
    {"KIRE\0"},
    {"FINGERCHIP\0"},
    {"HOLITECHINDIA\0"},
    {"FINGERCHIPINDIA\0"},
    {"Unknown\0"},
};

static fp_ta_feature_t m_ta_feature;

static uint32_t m_tpl_max_size = 0;
static uint32_t m_need_finger_loop_bak = 0;

static int32_t m_need_wait_finger_up = 0;
static int32_t m_capture_dump_enable = 0;

static int64_t m_sec_user_id = 0;

static int32_t m_narrow_flag = 0;

static char m_str_ta_name[DEVNAME_LEN] = {0};
void silfp_impl_set_ta_name(const void *name, uint32_t len)
{
    int32_t ret = 0;

    ret = silfp_util_strcpy(m_str_ta_name, sizeof(m_str_ta_name), name, len);
    if (ret < 0) {
        memset(m_str_ta_name, 0, sizeof(m_str_ta_name));
    }
    LOG_MSG_VERBOSE("name = %s", m_str_ta_name);
}

void silfp_impl_set_capture_dump_flag(int32_t addition)
{
    if (IS_UI_DUMP_ENABLE(addition)) {
        m_capture_dump_enable = 1;
        silfp_cust_set_hbm_mode(1);
    } else if (IS_UI_DUMP_DISABLE(addition)) {
        m_capture_dump_enable = 0;
        silfp_cust_set_hbm_mode(0);
    }
}

int32_t silfp_impl_set_wait_finger_up_need(int32_t need)
{
    m_need_wait_finger_up = need;
    return 0;
}

int32_t silfp_impl_is_wait_finger_up_need(void)
{
    return m_need_wait_finger_up;
}

inline int32_t silfp_impl_get_screen_status(uint8_t *status)
{
    return silfp_dev_get_screen_status(status);
}

inline int32_t silfp_impl_set_screen_cb(screen_cb listen, void *param)
{
    return silfp_dev_set_screen_cb(listen, param);
}

int32_t silfp_impl_set_finger_status_mode(int32_t mode)
{
    return silfp_dev_set_finger_status_mode(mode);
}

inline void silfp_impl_wait_clean(void)
{
    silfp_dev_wait_clean();
}

inline void silfp_impl_cancel(void)
{
    silfp_dev_cancel();
}

void silfp_impl_sync_finger_status_optic(int32_t down)
{
    if (down) {
        silfp_dev_sync_finger_down();
    } else if (!down) {
        silfp_dev_sync_finger_up();
    }
}

static int32_t _impl_fp_dev_restart()
{
    // Reset 300ms
    silfp_dev_pwdn(SIFP_PWDN_FLASH);
    usleep(1000*300);

    return 0;
}

static int32_t _impl_wait_finger_status_timeout(uint32_t status, int32_t irq, int32_t down, int32_t up, int32_t cancel, int32_t timeout)
{
    int32_t ret = 0;
    int16_t retry = 8;
    int32_t charge_state = silfp_cust_get_sys_charging_state();
    int16_t wait_again = 0;

    LOG_MSG_VERBOSE("wait finger status %d(%d:%d:%d:%d) timeout:%d", status, irq, down, up, cancel, timeout);

    do {
        if (!wait_again) {
            if (m_ta_feature.bit.irq_pwdn) {
                silfp_impl_chip_pwdn();
            } else {
                silfp_impl_download_normal();
                silfp_dev_clear_irq();
                ret = silfp_cmd_wait_finger_status(status, charge_state);
                retry --;
                if (ret == -SL_ERROR_INT_INVALID) {
                    if (retry > 0) {
                        _impl_fp_dev_restart();
                        continue;
                    } else {
                        LOG_MSG_ERROR("SIFP_PWDN_POWEROFF STATE_BREAK silfp_cmd_wait_finger_status ret=%d,retry=%d", ret,retry);
                        silfp_worker_set_to_break_mode();
                        return ret;
                    }
                }
            }

            if ((ret < 0) && (ret != -SL_ERROR_INT_INVALID)) {
                return ret;
            }
        } else {
            LOG_MSG_VERBOSE("detect invalid IRQ, wait again.");
            wait_again = 0;
        }

        silfp_dev_wait_clean();
        if (silfp_worker_is_canceled() && cancel) {
            ret = -SL_ERROR_CANCELED;
            break;
        } else {
            ret = silfp_dev_wait_finger_status(irq, down, up, cancel, timeout);
            if (ret == -SL_ERROR_FINGER_STATUS_TIMEOUT) {
                return ret;
            }
        }

        if (m_ta_feature.bit.reinit_after_irq && (status == IRQ_DOWN || status == IRQ_NAV) && (ret != -SL_ERROR_CANCELED)) {
            silfp_cust_finger_down_pre_action();
            silfp_impl_download_normal();
        } else if (!m_ta_feature.bit.reinit_after_irq && (ret != -SL_ERROR_CANCELED)) {
            ret = silfp_cmd_check_esd();
            if (ret == -SL_ERROR_FAKE_FINGER) {
                wait_again = 1;
                ret = -SL_ERROR_DETECTED_ESD;
                continue;
            }
            if ((ret == -SL_ERROR_DETECTED_ESD) && (retry > 0)) {
                _impl_fp_dev_restart();
                retry = 8; // Fix under ESD test, consective receive ESD cause retry counter continue decrease
                continue;
            }
        }
    } while (((ret == -SL_ERROR_DETECTED_ESD) || (ret == -SL_ERROR_INT_INVALID)) && (retry > 0));

    if ((ret != -SL_ERROR_CANCELED) && (ret != -SL_ERROR_DETECTED_ESD)) {
        if (status == IRQ_DOWN || status == IRQ_NAV) {
            silfp_cal_update_cfg(FP_CONFIG_DOWN_BIT, 0);
        } else {
            silfp_cal_update_cfg(FP_CONFIG_UP_BIT, 0);
        }
    }
    return ret;
}

static int32_t _impl_wait_finger_status(uint32_t status, int32_t irq, int32_t down, int32_t up, int32_t cancel)
{
    return _impl_wait_finger_status_timeout(status, irq, down, up, cancel, -1);
}

int32_t silfp_impl_wait_finger_down(void)
{
    int ret = 0;
    ret = _impl_wait_finger_status(IRQ_DOWN, 0, 1, 0, 0);
    if (ret >= 0) {
        silfp_cust_finger_down_after_action();
    }
    return ret;
}

int32_t silfp_impl_wait_finger_down_with_cancel(void)
{
    int32_t ret = 0;
    ret = _impl_wait_finger_status(IRQ_DOWN, 0, 1, 0, 1);
    if (ret >= 0) {
        silfp_cust_finger_down_after_action();
    }

    return ret;
}

int32_t silfp_impl_wait_finger_down_with_cancel_timeout(int32_t timeout)
{
    int32_t ret = 0;
    ret = _impl_wait_finger_status_timeout(IRQ_DOWN, 0, 1, 0, 1, timeout);
    if (ret >= 0) {
        silfp_cust_finger_down_after_action();
    }

    return ret;
}

int32_t silfp_impl_wait_finger_up(void)
{
    int32_t ret = 0;
    ret = _impl_wait_finger_status(IRQ_UP, 0, 0, 1, 0);
    if (ret >= 0) {
        silfp_cust_finger_up_after_action();
        silfp_impl_set_wait_finger_up_need(0);
    }

    return ret;
}

int32_t silfp_impl_wait_finger_up_with_cancel(void)
{
    int32_t ret = 0;
    ret = _impl_wait_finger_status(IRQ_UP, 0, 0, 1, 1);
    if (ret >= 0) {
        silfp_cust_finger_up_after_action();
        silfp_impl_set_wait_finger_up_need(0);
    }

    return ret;
}

int32_t silfp_impl_wait_finger_up_with_cancel_timeout(int32_t timeout)
{
    int32_t ret = 0;
    ret = _impl_wait_finger_status_timeout(IRQ_UP, 0, 0, 1, 1, timeout);
    if (ret >= 0) {
        silfp_cust_finger_up_after_action();
        silfp_impl_set_wait_finger_up_need(0);
    }

    return ret;
}

int32_t silfp_impl_wait_finger_up_with_cancel_impliment(void)
{
    int32_t ret = 0;
    LOG_MSG_DEBUG("release IRQ up");
    ret = _impl_wait_finger_status(IRQ_UP, 0, 0, 1, 1);
    if (ret >= 0) {
        if( silfp_cust_avoid_auth_invalid == NULL ) {
            silfp_cust_finger_up_after_action();
        }
        silfp_impl_set_wait_finger_up_need(0);
    }

    return ret;
}

int32_t silfp_impl_wait_finger_nav(void)
{
    int32_t ret = 0;

    ret = _impl_wait_finger_status(IRQ_NAV, 0, 1, 0, 1);
    if ((ret >= 0) && (silfp_cust_avoid_nav_invalid != NULL)) {
        silfp_cust_finger_down_after_action();
    }

    return ret;
}

int32_t silfp_impl_wait_irq_with_cancel(void)
{
    int32_t ret = 0;

    ret = _impl_wait_finger_status(IRQ_ESD, 1, 0, 0, 1);

    return ret;
}

static int32_t _impl_get_finger_status(uint32_t *status)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    uint32_t addition = 0;

    ret = silfp_cmd_get_finger_status(status, &addition);
    if (ret == -SL_ERROR_DETECTED_ESD) {
        _impl_fp_dev_restart();
        silfp_impl_download_normal();
    }

    if (ret >= 0) {
        if (status) {
            if (*status > 0) {
                silfp_cal_update_cfg(FP_CONFIG_DOWN_BIT, 0);
            } else if (*status == 0) {
                silfp_cal_update_cfg(FP_CONFIG_UP_BIT, 0);
            }
        }
        if (addition & 0x40000000) { // update finish
            m_ta_feature.bit.finger_loop = 0;
        }
        if (addition & 0x80000000) { // need save
            LOG_MSG_DEBUG("addition = 0x%x", addition);
            silfp_cal_update_cfg(FP_CONFIG_CHG_BIT, 1);
        }
    }

    return ret;
}

int32_t silfp_impl_get_finger_down_with_cancel(void)
{
    int32_t ret = -SL_ERROR_CANCELED;
    uint32_t status = 0;

    if (m_ta_feature.bit.finger_loop) {
        silfp_impl_download_normal();

        while (!silfp_worker_is_canceled()) {
            ret = _impl_get_finger_status(&status);
            if (ret < 0) {
                break;
            }

            if (status > 0) {
                silfp_cust_finger_down_after_action();
                break;
            } else {
                ret = -SL_ERROR_CANCELED;
            }
        }
    } else {
        ret = silfp_impl_wait_finger_down_with_cancel();
    }

    return ret;
}

int32_t silfp_impl_is_finger_down(void)
{
    return silfp_dev_is_finger_down();
}

static void _impl_capture_dump(uint8_t orig, int32_t type, uint32_t step, int32_t result)
{
    if (m_capture_dump_enable && ((type == IMG_CAPTURE_ENROLL) || (type == IMG_CAPTURE_AUTH))) {
        silfp_ext_skt_capture_dump(orig, (type == IMG_CAPTURE_ENROLL) ? 1 : 0, step, result);
    }
}

int32_t silfp_impl_capture_image(int32_t type, uint32_t step)
{
    int32_t ret = 0;

    if (silfp_impl_is_optic()) {
        silfp_cust_capture_get_tp_info(0);

        ret = silfp_impl_capture_image_pre();
        if (ret < 0) {
            return ret;
        }

        ret = silfp_impl_capture_image_raw(type, step);
        _impl_capture_dump(1, type, step, 0);
        if (ret < 0) {
            return ret;
        }

        silfp_cust_capture_get_tp_info(1);

        ret = silfp_impl_capture_image_after(type, step);
    } else {
        ret = silfp_cmd_capture_image(type, step);
    }

    if (ret < 0) {
        _impl_capture_dump(0, type, step, ret);
    }

    return ret;
}

int32_t silfp_impl_nav_capture_image(void)
{
    int32_t ret = 0;

    ret = silfp_cmd_nav_capture_image();
    silfp_dump_data((ret >= 0) ? DUMP_IMG_NAV_SUCC : DUMP_IMG_NAV_FAIL);

    return ret;
}

inline int32_t silfp_impl_auth_start(void)
{
    return silfp_cmd_auth_start();
}

inline int32_t silfp_impl_auth_step(uint64_t op_id, uint32_t step, uint32_t is_pay, uint32_t *fid, uint32_t *flag)
{
    int32_t ret = 0;

    ret = silfp_cmd_auth_step(op_id, step, is_pay, fid, flag);
    _impl_capture_dump(0, IMG_CAPTURE_AUTH, step, ret);

    return ret;
}

static int32_t _impl_auth_tpl_upd_normal(void)
{
    int32_t ret = 0;
    void *buf = NULL;
    uint32_t len = m_tpl_max_size;
    uint32_t fid = 0;

    if (m_ta_feature.bit.store_normal && m_ta_feature.bit.tpl_update) {
        do {
            buf = malloc(len);
            if (buf == NULL) {
                ret = -SL_ERROR_OUT_OF_MEMORY;
                break;
            }

            memset(buf, 0, len);
            ret = silfp_cmd_update_template(buf, &len, &fid);
        } while (0);

        if (ret >= 0 && len > 0 && len <= m_tpl_max_size) {
            silfp_storage_update(fid, buf, len);
        }
        free(buf);
        buf = NULL;
    }
    return ret;
}

inline int32_t silfp_impl_get_auth_tpl_upd(void)
{
    int32_t ret = 0;
    uint32_t is_update = 0;
    uint32_t buffer = 0;
    uint32_t len = sizeof(buffer);

    ret = silfp_cmd_send_cmd_with_buf_and_get(REQ_CMD_SUB_ID_JUDGE_UPD_TPL, &buffer, &len, &is_update);

    return ret;
}

inline int32_t silfp_impl_auth_end(void)
{
    int32_t ret = 0;

    _impl_auth_tpl_upd_normal();
    ret = silfp_cmd_auth_end();

    return ret;
}

inline int32_t silfp_impl_get_enroll_num(uint32_t *num)
{
    return silfp_cmd_get_enroll_num(num);
}

inline int32_t silfp_impl_enroll_start(void)
{
    return silfp_cmd_enroll_start();
}

inline int32_t silfp_impl_enroll_step(uint32_t *remaining)
{
    int32_t ret = 0;

    ret = silfp_cmd_enroll_step(remaining);
    _impl_capture_dump(0, IMG_CAPTURE_ENROLL, 0, ret);

    return ret;
}

static int32_t _impl_enroll_tpl_save_normal(uint32_t *fid)
{
    int32_t ret = 0;
    void *buf = NULL;
    uint32_t len = m_tpl_max_size;

    if (m_ta_feature.bit.store_normal) {
        do {
            buf = malloc(len);
            if (buf == NULL) {
                ret = -SL_ERROR_OUT_OF_MEMORY;
                break;
            }

            memset(buf, 0, len);
            ret = silfp_cmd_save_template(buf, &len);
        } while (0);

        if (ret >= 0 && len > 0 && len <= m_tpl_max_size) {
            ret = silfp_storage_save(buf, len, m_sec_user_id, fid);
        } else {
            ret = -SL_ERROR_STO_OP_FAILED;
        }

        if (buf != NULL) {
            free (buf);
        }
    }
    return ret;
}

int32_t silfp_impl_enroll_end(uint32_t *fid)
{
    int32_t ret = 0;
    int32_t status = -1;

    ret = _impl_enroll_tpl_save_normal(fid);
    if (ret >= 0) {
        status = 0; // save ok or no normal storage
    } else {
        status = -1; // save failed
    }

    ret = silfp_cmd_enroll_end(status, fid);
    if (ret >= 0 && status < 0) {
        ret = -SL_ERROR_STO_OP_FAILED;
    }

    return ret;
}

inline int32_t silfp_impl_nav_support(uint32_t *type)
{
    return silfp_cmd_nav_support(type);
}

inline int32_t silfp_impl_nav_start(void)
{
    int32_t ret = 0;

    if (silfp_cal_need_nav_cal()) {
        silfp_impl_download_normal();
    }

    ret = silfp_cmd_nav_start();

    return ret;
}

inline int32_t silfp_impl_nav_step(uint32_t *key)
{
    int32_t ret = 0;

    ret = silfp_cmd_nav_step(key);
    silfp_dump_data((ret >=0) ? DUMP_IMG_NAV_SUCC : DUMP_IMG_NAV_FAIL);

    return ret;
}

inline int32_t silfp_impl_nav_end(void)
{
    return silfp_cmd_nav_end();
}

inline int32_t silfp_impl_send_key(uint32_t key)
{
    return silfp_dev_send_key(key);
}

inline int32_t silfp_impl_nav_set_mode(uint32_t mode)
{
    return silfp_cmd_set_nav_mode(mode);
}

inline int32_t silfp_impl_download_normal()
{
    return silfp_cmd_download_normal();
}

static int32_t _impl_init2()
{
    int32_t ret = 0;
    uint32_t feature = 0;
    uint32_t algoVer = 0;
    uint32_t taVer = 0;

    char device_info[] = "silead@fp";
    uint32_t len = strlen(device_info);

    m_ta_feature.feature = 0;
    ret = silfp_cmd_init2(device_info, len, &feature, &m_tpl_max_size);
    if (ret >= 0) {
        m_ta_feature.feature = feature;
        m_need_finger_loop_bak = m_ta_feature.bit.finger_loop;
        silfp_cal_init(feature, silfp_impl_is_optic());
    }

    // maybe some ta miss the value, should not happend, just in case
    if (m_ta_feature.bit.store_normal && m_tpl_max_size == 0) {
        m_tpl_max_size = (500 * 1024);
    }

    silfp_cmd_get_ta_ver(&algoVer, &taVer);
    LOG_MSG_INFO("ta version:v%d, algo version: v%d", taVer, algoVer);

    LOG_MSG_VERBOSE("store_normal=%d, tpl_max_size=%d, need_reinit_after_irq=%d", m_ta_feature.bit.store_normal, m_tpl_max_size, m_ta_feature.bit.reinit_after_irq);
    LOG_MSG_VERBOSE("finger loop=%d, avdd=%d, optic=%d, esd=%d, auth retry=%d", m_ta_feature.bit.finger_loop, m_ta_feature.bit.irq_pwdn_avdd_down, m_ta_feature.bit.is_optic, !m_ta_feature.bit.esd_unsupport, m_ta_feature.bit.auth_retry);
    LOG_MSG_VERBOSE("ocp=%d", m_ta_feature.bit.is_ocp);
    LOG_MSG_VERBOSE("feature=0x%x", m_ta_feature.feature);

    if (m_ta_feature.bit.reset_down_for_flash) {
        silfp_dev_set_feature(FEATURE_FLASH_CS);
    }
    silfp_bmp_set_img_encrypt(m_ta_feature.bit.dump_img_encrypt);
    silfp_dump_set_version(m_ta_feature.bit.is_dump_v2);

    return ret;
}

static int32_t _impl_get_extend_info_in_flash(void)
{
    int32_t ret = 0;

    if (!m_ta_feature.bit.reset_down_for_flash) {
        return 0;
    }

    LOG_MSG_VERBOSE("need set reset down to read info from flash");

    silfp_dev_pwdn(SIFP_PWDN_NONE);
    ret = silfp_cmd_send_cmd_with_buf(REQ_CMD_SUB_ID_EXTEND_INFO_IN_FLASH, NULL, 0);
    silfp_impl_download_normal();

    return ret;
}

static int32_t _impl_get_ta_feature(void)
{
    int32_t ret = 0;
    uint32_t len = sizeof(m_narrow_flag);
    uint32_t result = 0;

    ret = silfp_cmd_send_cmd_with_buf_and_get(REQ_CMD_SUB_ID_GET_TA_FEATURE, &m_narrow_flag, &len, &result);
    LOG_MSG_VERBOSE("narrow_flag = %d", m_narrow_flag);
    return ret;
}

int32_t silfp_impl_is_narrow_mode(void)
{
    LOG_MSG_VERBOSE("narrow_flag = %d", m_narrow_flag);
    return m_narrow_flag;
}

int32_t _get_version_info(void *buffer, uint32_t offset, uint32_t len, uint8_t *ver_buf, uint32_t ver_buf_len)
{
    uint32_t str_len = 0;

    memcpy(&str_len, (uint8_t *)buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    if ((str_len > 0) && (ver_buf_len >= str_len)) {
        if (len >= str_len + offset) {
            memcpy(ver_buf, (uint8_t *)buffer + offset, str_len);
            LOG_MSG_DEBUG("ver_buf = %s, len=%d", ver_buf, str_len);
        } else {
            LOG_MSG_ERROR("buffer invalid, %d, %d", len, str_len);
            return -1;
        }
    }
    offset += str_len;
    return offset;
}

int32_t silfp_impl_get_version_ext(uint8_t *algo_ver, uint32_t algo_len, uint8_t *ta_ver, uint32_t ta_len)
{
    uint32_t result = 0;
    int32_t ret = 0;
    uint32_t offset = 0;
    uint8_t buffer[1024] = {0};
    uint32_t len = sizeof(buffer);

    if ((algo_ver == NULL) || (algo_len == 0) || (ta_ver == NULL) || (ta_len == 0)) {
        LOG_MSG_ERROR("bad param!");
        return -1;
    }

    memset(algo_ver, 0, algo_len);
    memset(ta_ver, 0, ta_len);
    memset(buffer, 0, len);

    ret = silfp_cmd_send_cmd_with_buf_and_get(REQ_CMD_SUB_ID_VERSION_EXT, buffer, &len, &result);
    LOG_MSG_DEBUG("ret = %d, ext version len = %d", ret, len);
    if ((ret < 0) || (len < 8)) {
        return -1;
    }

    offset = _get_version_info(buffer, offset, len, ta_ver, ta_len);
    offset = _get_version_info(buffer, offset, len, algo_ver, algo_len);

    return offset;
}

static int32_t _impl_init()
{
    int32_t ret = 0;

    uint32_t chipid = 0;
    uint32_t subid = 0;
    uint32_t vid = 0;
    uint32_t update_cfg = 0;

    fp_dev_conf_t dev_init;
    char name[32] = {0};

    memset(&dev_init, 0, sizeof(dev_init));
    ret = silfp_dev_init(&dev_init);
    if (ret < 0) {
        LOG_MSG_ERROR("init dev fail");
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    if (silfp_dev_hw_reset(0) < 0) {
        LOG_MSG_ERROR("reset fingerprint chip fail");
    }

    if (m_str_ta_name[0] != '\0') {
        silfp_cmd_set_env(m_str_ta_name);
    } else {
        silfp_cmd_set_env((dev_init.ta[0] != '\0') ? dev_init.ta : NULL);
    }

    silfp_dbg_update_all_log_level();

    do {
        ret = silfp_cmd_init(&dev_init, &chipid, &subid, &vid, &update_cfg);
        LOG_MSG_VERBOSE("chipid=%x,%x,%x %d (%d)", chipid, subid, vid, update_cfg, ret);
        if (ret < 0) {
            break;
        }

        silfp_ree_misc_update_cfg(update_cfg, chipid, subid, vid);
        ret = _impl_init2();
        if (ret < 0) {
            break;
        }
        _impl_get_ta_feature();
        _impl_get_extend_info_in_flash();
        silfp_impl_get_otp();
    } while (0);

    if (ret >= 0) {
        snprintf(name, sizeof(name), "gsl%04x\n", (chipid >> 16));
        silfp_dev_create_proc_node(name);
    }

    return ret;
}

int32_t silfp_impl_init()
{
    int32_t ret = 0;
    int32_t i = 0;
    const int32_t count = 3;

#ifdef GIT_SHA1_ID
    LOG_MSG_INFO("fp hal version: %s, tag: %s", FP_HAL_VERSION, GIT_SHA1_ID);
#else
    LOG_MSG_INFO("fp hal version: %s", FP_HAL_VERSION);
#endif

    for (i = 0; i < count; i++) {
        ret = _impl_init();
        if (ret >= 0) {
            break;
        }
        usleep(1000*300);
    }

    return ret;
}

int32_t silfp_impl_deinit(void)
{
    silfp_dev_deinit();
    silfp_cmd_deinit();

    if (m_ta_feature.bit.store_normal) {
        silfp_storage_release();
    }
    silfp_dump_deinit();
    silfp_cal_deinit();

    LOG_MSG_VERBOSE("close");
    return SL_SUCCESS;
}

int32_t silfp_impl_set_gid(uint32_t gid)
{
    int32_t ret = 0;
    uint32_t serial = 0xFDCA;

    LOG_MSG_DEBUG("serial = 0x%x", serial);

    ret = silfp_cmd_set_gid(gid, serial);

    return ret;
}

static int32_t _impl_load_user_db_sync_to_ta(const char *db_path)
{
    int32_t ret = 0;

    void *buf = NULL;
    uint8_t *pdata = NULL;
    const uint32_t buf_len = m_tpl_max_size;
    uint32_t tpl_len = 0;
    int32_t load_count = 0;

    int64_t sid = m_sec_user_id;
    uint32_t id[TPL_MAX_ST] = {0};
    int32_t id_count = 0;
    int32_t i = 0;

    buf = malloc(buf_len + sizeof(sid));
    if (buf == NULL) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    silfp_storage_set_tpl_path(db_path);
    pdata = (uint8_t *)buf;
    pdata += sizeof(sid);

    id_count = silfp_storage_get_idlist(id, 1);
    for (i = 0; i < id_count; i++) {
        memset(buf, 0, buf_len);
        ret = silfp_storage_load(id[i], (void *)pdata, buf_len);
        if (ret >= 0) {
            sid = silfp_storage_get_sec_id(id[i]);
            memcpy(buf, &sid, sizeof(sid));
            tpl_len = ret;
            if (tpl_len <= buf_len) {
                ret = silfp_cmd_load_template(id[i], buf, tpl_len + sizeof(sid));
            } else {
                LOG_MSG_ERROR("tpl size failed (%d)", tpl_len);
                ret = -SL_ERROR_TEMPLATE_INVALID;
            }
            silfp_storage_remove(id[i]);
        }
    }

    pdata = (uint8_t *)buf;
    pdata[0] = 0xFF;
    pdata[1] = 0xFF;
    silfp_cmd_load_template(0, buf, 2);
    silfp_storage_remove_all();

    free(buf);
    buf = NULL;

    return load_count;
}

static int32_t _impl_load_user_db_normal(const char *db_path)
{
    int32_t ret = 0;

    void *buf = NULL;
    const uint32_t buf_len = m_tpl_max_size;
    uint32_t tpl_len = 0;
    int32_t load_count = 0;

    uint32_t id[TPL_MAX_ST] = {0};
    int32_t id_count = 0;
    int32_t i = 0;

    buf = malloc(buf_len);
    if (buf == NULL) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    silfp_storage_set_tpl_path(db_path);

    id_count = silfp_storage_get_idlist(id, 1);
    for (i = 0; i < id_count; i ++) {
        memset(buf, 0, buf_len);
        ret = silfp_storage_load(id[i], buf, buf_len);
        if (ret >= 0) {
            tpl_len = ret;
            if (tpl_len <= buf_len) {
                ret = silfp_cmd_load_template(id[i], buf, tpl_len);
            } else {
                LOG_MSG_ERROR("tpl size failed (%d)", tpl_len);
                ret = -SL_ERROR_TEMPLATE_INVALID;
            }

            if (ret == -SL_ERROR_TEMPLATE_INVALID) {
                silfp_storage_inc_fail_count(id[i]);
            } else if (ret >= 0) {
                load_count++;
            }
        }
    }

    free(buf);
    buf = NULL;

    return load_count;
}

int32_t silfp_impl_load_user_db(const char *db_path)
{
    int32_t ret = 0;
    uint32_t sync_ca_tpl = 0;

    if (!m_ta_feature.bit.store_normal) {
        silfp_cust_load_user_db_action((void *)db_path);
    }

    ret = silfp_cmd_load_user_db(db_path, &sync_ca_tpl);
    if (ret < 0) {
        LOG_MSG_ERROR("load user db %s failed (%d)", db_path, ret);
        return -SL_ERROR_STO_OP_FAILED;
    }

    if (m_ta_feature.bit.store_normal) {
        LOG_MSG_VERBOSE("load android tpl");
        ret = _impl_load_user_db_normal(db_path);
    } else if (sync_ca_tpl) {
        LOG_MSG_VERBOSE("sync android tpl");
        ret = _impl_load_user_db_sync_to_ta(db_path);
    }

    return ret;
}

int32_t silfp_impl_remove_finger(uint32_t fid)
{
    int32_t ret = 0;

    if (m_ta_feature.bit.store_normal) {
        ret = silfp_storage_remove(fid);
        if (ret < 0) {
            return ret;
        }
    }

    ret = silfp_cmd_remove_finger(fid);

    return ret;
}

inline int32_t silfp_impl_get_db_count(void)
{
    return silfp_cmd_get_db_count();
}

inline int32_t silfp_impl_get_finger_prints(uint32_t *ids, uint32_t count)
{
    return silfp_cmd_get_finger_prints(ids, count);
}

inline int64_t silfp_impl_load_enroll_challenge(void)
{
    return silfp_cmd_load_enroll_challenge();
}

inline int32_t silfp_impl_set_enroll_challenge(uint64_t challenge)
{
    return silfp_cmd_set_enroll_challenge(challenge);
}

inline int32_t silfp_impl_verify_enroll_challenge(const void *hat, uint32_t size, int64_t sid)
{
    m_sec_user_id = sid;
    return silfp_cmd_verify_enroll_challenge(hat, size);
}

int64_t silfp_impl_load_auth_id(void)
{
    int64_t id = 0;

    id = silfp_cmd_load_auth_id();
    if (m_ta_feature.bit.store_normal) {
        id = silfp_storage_load_db_id();
    }

    return id;
}

inline int32_t silfp_impl_get_hw_auth_obj(void *buffer, uint32_t length)
{
    return silfp_cmd_get_hw_auth_obj(buffer, length);
}

inline int64_t silfp_impl_get_sec_id(uint32_t fid)
{
    return silfp_storage_get_sec_id(fid);
}

inline int32_t silfp_impl_capture_image_pre(void)
{
    return silfp_cmd_capture_image_pre();
}

inline int32_t silfp_impl_capture_image_raw(int32_t type, uint32_t step)
{
    return silfp_cmd_capture_image_raw(type, step);
}

inline int32_t silfp_impl_capture_image_after(int32_t type, uint32_t step)
{
    return silfp_cmd_capture_image_after(type, step);
}

inline int32_t silfp_impl_set_touch_info(void *buffer, uint32_t len)
{
    return silfp_cmd_send_cmd_with_buf(REQ_CMD_SUB_ID_TOUCH_INFO, buffer, len);
}

int32_t silfp_impl_get_otp(void)
{
    int32_t ret = 0;
    uint32_t otp[3] = {0};

    ret = silfp_cmd_get_otp(&otp[0], &otp[1], &otp[2]);
    if (ret >= 0) {
        LOG_MSG_INFO("OTP 0x%08X 0x%08X 0x%08X", otp[0], otp[1], otp[2]);
		silfp_dev_set_otp(otp);
        if ( silfp_cust_otp_setprop != NULL ) {
            silfp_cust_otp_setprop(otp);
        }
    }

    return ret;
}

int32_t silfp_impl_chip_pwdn(void)
{
    int32_t ret = 0;
    uint8_t onoff = 0;

    silfp_dev_pwdn(m_ta_feature.bit.irq_pwdn_avdd_down ? SIFP_PWDN_POWEROFF : SIFP_PWDN_NONE);
    ret = silfp_cmd_send_cmd_with_buf(REQ_CMD_SUB_ID_SPI_CTRL, &onoff, sizeof(onoff));
    //silfp_cmd_deep_sleep_mode(); //????

    return ret;
}

void silfp_impl_chip_pwoff(void)
{
    if (m_ta_feature.bit.is_ocp) {
        silfp_dev_hw_reset(0);
    } else {
        silfp_dev_pwdn(SIFP_PWDN_POWEROFF);
    }
}

inline int32_t silfp_impl_wakelock(uint8_t lock)
{
    return silfp_dev_wakelock(lock);
}

int32_t silfp_impl_is_optic(void)
{
    int32_t ret = silfp_cust_is_optic();
    if (ret < 0) {
        ret = m_ta_feature.bit.is_optic;
    }
    return ret;
}

int32_t silfp_impl_is_unsupport_esd(void)
{
    return m_ta_feature.bit.esd_unsupport;
}

int32_t silfp_impl_calibrate(void)
{
    int32_t ret = silfp_cal_calibrate();
    return ret;
}

int32_t silfp_impl_cal_base_sum(void)
{
    return silfp_cal_base_sum();
}

int32_t silfp_impl_cal_step(uint32_t step)
{
    int32_t ret = silfp_cal_step(step);
    return ret;
}

int32_t silfp_impl_cal_reset(void)
{
    int32_t ret = silfp_cal_reset();
    if (ret >= 0) {
        m_ta_feature.bit.finger_loop = m_need_finger_loop_bak;
    }
    return 0;
}

void silfp_impl_cal_set_path(const void *path, uint32_t len)
{
    silfp_cal_set_path(path, len);
}

int32_t silfp_impl_get_ta_retry_loop(void)
{
    return m_ta_feature.bit.auth_retry;
}

int32_t silfp_impl_is_add_watermark(void)
{
    return m_ta_feature.bit.watermark;
}

int32_t silfp_impl_get_dump_img_mode(void)
{
    int32_t dump_img_num = 1;   //dump_img_num为1表示一张原图对应一张处理后图，为2表示一张原图对应两张处理后图

    if (m_ta_feature.bit.dump_img_mode) {
        dump_img_num = 2;
    }
    return dump_img_num;
}

int32_t silfp_impl_otp_parse(void *buf, uint32_t size, uint32_t offset, uint32_t *otp, uint32_t count)
{
    uint16_t num = NUM_ELEMS(s_otp_vendor);
    uint32_t len;
    fp_ta_otp_t *p_otp = (fp_ta_otp_t *)otp;
    char *pstr = s_otp_vendor[num-1].str;
    char *p = (char *)buf + offset + 1;

    if (!buf || !size || !otp || !count) {
        return -SL_ERROR_BAD_PARAMS;
    }

    if ((p_otp->info.month > 0) && (p_otp->info.month < 13) && (p_otp->info.day > 0)) {
        // parse factory id
        if (p_otp->info.vendor_id < num) {
            pstr = s_otp_vendor[p_otp->info.vendor_id].str;
        }
        sprintf(p, "Verdor: %s\nBatch: %d\nDate: %d-%02d-%02d", pstr, p_otp->info.ver+1, 2020+p_otp->info.year, p_otp->info.month, p_otp->info.day);
    } else {
        sprintf(p, "%s", "NA");
    }

    len = strlen((char *)p);
    offset += len+1;
    *(p-1) = len;
    //LOG_MSG_ERROR("len=%d offset=%d, %s", len, offset, p);

    return offset;
}
