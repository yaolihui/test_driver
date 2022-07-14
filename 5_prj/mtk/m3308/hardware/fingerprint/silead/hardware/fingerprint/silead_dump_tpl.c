/******************************************************************************
 * @file   silead_dump_tpl.c
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
 * <author>    <date>     <version>     <desc>
 * Taobb      2021/8/25     0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "dump"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>

#include "silead_const.h"
#include "silead_cmd.h"
#include "silead_util.h"
#include "silead_error.h"
#include "silead_dump_tpl.h"

#ifdef SIL_DUMP_TPL

#define TPL_PACK_SIZE  (950 * 1024)
#define TPL_NAME_LEN   32

/* load user db & sync to ca */
int32_t silfp_dump_tpl(const char *db_path)
{
    int32_t ret, i = 0;
    uint32_t result, tpl_len = 0;
    uint8_t tpl_name[TPL_NAME_LEN] = {0};
    void *buf = NULL;

    LOG_MSG_VERBOSE("enter %s", db_path);

    if(db_path == NULL) {
        return -SL_ERROR_BAD_PARAMS;
    }

    buf = malloc(TPL_PACK_SIZE);
    if (buf == NULL) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    snprintf(buf, TPL_PACK_SIZE, "%s", db_path);
    tpl_len = strlen(buf);
    silfp_cmd_send_cmd_with_buf_and_get(REQ_CMD_SUB_ID_SYNC_DB_PATH, buf, &tpl_len, &result);

    for (i = 0;; i++) {
        tpl_len = TPL_PACK_SIZE;
        memset(buf, 0, tpl_len);
        memcpy(buf, &i, sizeof(i));
        ret = silfp_cmd_send_cmd_with_buf_and_get(REQ_CMD_SUB_ID_SYNC_TPL_TO_CA, buf, &tpl_len, &result);
        if (ret < 0) {
            LOG_MSG_ERROR("silfp_cmd_send_cmd  REQ_CMD_SUB_ID_SYNC_TPL_TO_CA failed (%d)", ret);
            break;
        }
        if (tpl_len > 0) {
            snprintf((char *)tpl_name, sizeof(tpl_name), "%d-syncto_catpl.tpl", i);
            silfp_util_file_save(db_path, (char *)tpl_name, buf, tpl_len);
        }
    }

    free(buf);
    buf = NULL;

    return ret;
}

#endif /* SIL_DUMP_TPL */
