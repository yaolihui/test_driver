/******************************************************************************
 * @file   silead_logmsg.h
 * @brief  Contains log message header file.
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
 * calvin wang  2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_LOG_MSG_H__
#define __SILEAD_LOG_MSG_H__

#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "log/logmsg.h"
#include "silead_const.h"

#define MAX_PATH_LEN 256

#ifndef NUM_ELEMS
#define NUM_ELEMS(a) (sizeof (a) / sizeof (a)[0])
#endif /* NUM_ELEMS */

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif /* UNUSED */

#ifndef __UNUSED
#define __UNUSED __unused
#endif /* __UNUSED */

#ifndef __WEAK
#define __WEAK __attribute__((weak))
#endif /* __WEAK */

#ifndef __PACKED
#define __PACKED __attribute__((packed))
#endif /* __PACKED */

#define FUNC_INVOKE(func, ...) \
    if (func != NULL) { \
        func(__VA_ARGS__); \
    }

#define FUNC_INVOKE_RET(func, def, ...) \
    ((func != NULL) ? func(__VA_ARGS__) : def)

#define FUNC_IS_IMPL(func) (func != NULL)

#define silfp_ree_util_path_copy silfp_util_path_copy
#define silfp_ree_util_dir_get_type silfp_util_dir_get_type
#define silfp_ree_util_open_file silfp_util_open_file
#define silfp_ree_util_write_file silfp_util_write_file
#define silfp_ree_util_close_file silfp_util_close_file
#define silfp_ree_util_file_save silfp_util_file_save
#define silfp_ree_util_file_remove silfp_util_file_remove

#define silfp_ree_cmd_update_cfg silfp_cmd_update_cfg
#define silfp_ree_cmd_send_cmd_with_buf_and_get silfp_cmd_send_cmd_with_buf_and_get
#endif // __SILEAD_LOG_MSG_H__