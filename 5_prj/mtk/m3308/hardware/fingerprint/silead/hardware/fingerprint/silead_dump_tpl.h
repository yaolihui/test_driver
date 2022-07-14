/******************************************************************************
 * @file   silead_dump_tpl.h
 * @brief  Contains dump image header file.
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
 * Taobb     2021/8/25   0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_DUMP_TPL_H__
#define __SILEAD_DUMP_TPL_H__


// extra silfp_cmd_send_cmd_with_buf & silfp_cmd_send_cmd_with_buf_and_get cmd id
typedef enum _req_cmd_id_ex {
    REQ_CMD_SUB_ID_SYNC_DB_PATH         = 0x80000007, // get sync tpl path (get)
    REQ_CMD_SUB_ID_SYNC_TPL_TO_CA       = 0x80000008, // save tpl to ca (get)
} req_cmd_id_ex_t;


#endif /* __SILEAD_DUMP_TPL_H__ */
