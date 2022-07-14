/******************************************************************************
 * @file   silead_dump_dbg.c
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

#define FILE_TAG "dump_dbg"
#include "log/logmsg.h"

#include "silead_util.h"
#include "silead_cmd.h"
#include "silead_dump_dbg.h"
#include "silead_const.h"
#include "silead_dump_util.h"

#define DUMP_ENABLED 0x64736377

#ifdef SIL_DUMP_IMAGE_DYNAMIC
static int32_t _dump_get_level(char *log, char *lvl)
{
    int32_t dump_data_support = -1;
    static char m_test_dump_prop[32] = {0};
    int32_t prop = 0;

    if (m_test_dump_prop[0] == 0) {
#ifndef SIL_DUMP_IMAGE_SWITCH_PROP
        snprintf(m_test_dump_prop, sizeof(m_test_dump_prop), "%s.%s.%s.%s.%s", "persist", "log", "tag", log, lvl);
#else
        snprintf(m_test_dump_prop, sizeof(m_test_dump_prop), "%s", SIL_DUMP_IMAGE_SWITCH_PROP);
        UNUSED(log);
        UNUSED(lvl);
#endif
    }

#ifndef SIL_DUMP_IMAGE_SWITCH_PROP
    prop = silfp_util_get_str_value(m_test_dump_prop + 8, (uint8_t)DUMP_ENABLED);
#endif
    if (prop == 0) {
        prop = silfp_util_get_str_value(m_test_dump_prop, (uint8_t)DUMP_ENABLED);
    }
    if (prop == DUMP_ENABLED) {
        dump_data_support = 1;
    }

    return dump_data_support;
}

int32_t silfp_dump_check_level(void)
{
    return _dump_get_level("data", "dump");
}

#else /* !SIL_DUMP_IMAGE_DYNAMIC */

int32_t silfp_dump_check_level(void)
{
    return 1;
}

#endif /* SIL_DUMP_IMAGE_DYNAMIC */

void silfp_dump_dbg_update(void)
{
    static int32_t s_dump_debug = -1;
    uint8_t enable = 0;
    uint32_t size = sizeof(enable);
    uint32_t result = 0;
    int32_t ret = 0;

    int32_t debug = silfp_dump_check_level();
    if (s_dump_debug != debug) {
        enable = debug;
        ret = silfp_cmd_send_cmd_with_buf_and_get(REQ_CMD_SUB_ID_ENABLE_DUMP_IMG, &enable, &size, &result);
        if (ret >= 0) { // if send fail, not update
            silfp_dump_util_set_enable(enable);
            s_dump_debug = debug;
        }
    }
}

#endif /* SIL_DUMP_IMAGE */