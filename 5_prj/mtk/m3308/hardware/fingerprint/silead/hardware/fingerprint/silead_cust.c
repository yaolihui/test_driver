/******************************************************************************
 * @file   silead_cust.c
 * @brief  Contains fingerprint customize functions.
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

#define FILE_TAG "silead_cust"
#include "log/logmsg.h"

#include "silead_cust.h"
#include "silead_screen.h"
#include "fingerprint.h"
#include "silead_error.h"
#include "silead_notify.h"

#include <unistd.h>
#include <signal.h>

const char* __WEAK silfp_cust_get_dump_path(void)
{
    // return default dump path
    return NULL;
}

const char* __WEAK silfp_cust_get_cal_path(void)
{
    // return default cal path
    return NULL;
}

const char* __WEAK silfp_cust_get_ta_name(void)
{
    // return default ta name
    return NULL;
}

int32_t __WEAK silfp_cust_is_capture_disable(void)
{
    // capture disabled
    return 0;
}

int32_t __WEAK silfp_cust_need_cancel_notice(void)
{
    // need notify when cancel, if return -1, use default
    return -1;
}

int32_t __WEAK silfp_cust_is_screen_from_drv(void)
{
    // get screen status from driver
    return 1;
}

int32_t __WEAK silfp_cust_get_finger_status_mode(void)
{
    return -1;
}

void __WEAK silfp_cust_finger_down_pre_action(void)
{
    LOG_MSG_VERBOSE("default, not impliment");
}

void __WEAK silfp_cust_finger_down_after_action(void)
{
    LOG_MSG_VERBOSE("default, not impliment");
}

void __WEAK silfp_cust_finger_up_pre_action(void)
{
    LOG_MSG_VERBOSE("default, not impliment");
}

void __WEAK silfp_cust_finger_up_after_action(void)
{
    LOG_MSG_VERBOSE("default, not impliment");
}

int32_t __WEAK silfp_cust_set_hbm_mode(uint32_t __unused mode)
{
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_set_brightness(uint32_t __unused mode)
{
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_restore_hbm(void)
{
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_restore_brightness(void)
{
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

void __WEAK silfp_cust_capture_get_tp_info(uint8_t __unused mode)
{
    // [capture image] get tp info and send to ta, for optic
    LOG_MSG_VERBOSE("default, not impliment");
}

int32_t __WEAK silfp_cust_get_sys_charging_state(void)
{
    // [all] for charging interference
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_tpl_change_action(void)
{
    // [tpl change] action after tpl change (remove, add, load)
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_tp_irq_enable(int32_t __unused enable)
{
    // enable/disable tp irq
    return 0;
}

int32_t __WEAK silfp_cust_clear_ui_ready(void)
{
    // [auth | enroll] clear finger ready
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_wait_ui_ready(void)
{
    // [auth | enroll] wait finger ready when touch down
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_notify_ui_ready(uint32_t __unused addition)
{
    // [auth | enroll] notify finger ready
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_auth_get_retry_times(void)
{
    // [auth] retry times for each auth
    return 2;
}

int32_t __WEAK silfp_cust_auth_mistouch_ignor_screen_off(void)
{
    // [auth] mistouch and screen off, ignor notify
    return 0;
}

int32_t __WEAK silfp_cust_enroll_timeout_sec(void)
{
    // [enroll] enroll timeout (seconds)
    // if return 0, disable timeout check
    return 0;
}

int32_t __WEAK silfp_cust_enroll_cont_err_times(void)
{
    // [enroll] when cont error, report FINGERPRINT_ERROR_UNABLE_TO_PROCESS error
    // if return 0, disabled
    return 0;
}

int32_t __WEAK silfp_cust_enroll_report_remain_when_error(void)
{
    // [enroll] when error, report remain times to UI
    // 1: enable, 0: disabled
    return 0;
}

int32_t __WEAK silfp_cust_trans_notice_code(int32_t code)
{
    // [enroll] notice code trans, such as same area/same finger ect.
/*    if (code < 0) {
        code = -code;
    }
*/
    LOG_MSG_DEBUG("silfp_cust_trans_notice_code code=%d", code);
    switch(code){
    case -SL_ERROR_EROLL_DUPLICATE:
        return chino_FINGERPRINT_ACQUIRED_DUPLICATE_FINGER;
    case -SL_ERROR_SAME_AREA:
        return chino_FINGERPRINT_ACQUIRED_SAME_AREA;
    default:
          if (code < 0) {
            code = -code;
         }
    }
    return code;
}

int32_t __WEAK silfp_cust_otp_parse(void __unused *buf, uint32_t __unused size, uint32_t __unused offset, uint32_t __unused *otp, uint32_t __unused count)
{
    // otp parse
    return offset;
}

int32_t __WEAK silfp_cust_send_quality_notice(int32_t __unused result, int32_t __unused quality)
{
    // result: 0 pass, 1 failed
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_send_aging_notice(int32_t __unused result)
{
    // result: 0 pass, 1 failed
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_send_calibrate_notice(int32_t __unused sub_id, int32_t __unused result)
{
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_send_optic_test_factory_quality_notice(int32_t __unused result, uint32_t __unused snr, uint32_t __unused noise, uint32_t __unused signal)
{
    // [factory test] optic factory quality test, result: 0 pass, 1 failed
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_is_optic(void)
{
    LOG_MSG_VERBOSE("default, not impliment");
    return -1;
}

int32_t __WEAK silfp_cust_esd_support(void)
{
    // [esd] -1: support if not optic, else unsupport, 0: unsupport, 1: support
    return -1;
}

void __WEAK silfp_cust_auth_mismatch(void)
{
    LOG_MSG_VERBOSE("default, not impliment");
}

uint8_t __WEAK silfp_cust_get_quality_threshold(void)
{
    return 0;
}

extern int32_t silfp_common_cancel();
static void _silfp_cust_enroll_timeout()
{
    LOG_MSG_DEBUG("YLH:warning: enroll timeout! back!!");
    silfp_notify_send_error_notice(FINGERPRINT_ERROR_TIMEOUT);
    alarm(0);
    silfp_common_cancel();
}

void __WEAK silfp_cust_enroll_timeout(unsigned int __unused sec)
{
    //LOG_MSG_VERBOSE("default, not impliment");
    static int flag = 0;
    if (!flag ) {
        LOG_MSG_DEBUG("YLH:register signal sigalrm");
        signal(SIGALRM, _silfp_cust_enroll_timeout);
        flag = 1;
    }
    alarm(sec);
}

int32_t __WEAK silfp_cust_load_user_db_action(void __unused *param)
{
    return 0;
}

int32_t __WEAK silfp_cust_support_no_enroll_nav(void)
{
    return 0;
}

int32_t __WEAK silfp_cust_need_wait_finger_up(void)
{
    return 0;
}

int32_t __WEAK silfp_cust_reset_test_skip_pwdn(void)
{
    return 0;
}

void __WEAK silfp_cust_report_remaining(uint32_t __unused gid, uint32_t __unused remaining)
{
    LOG_MSG_VERBOSE("default, not impliment");
}
