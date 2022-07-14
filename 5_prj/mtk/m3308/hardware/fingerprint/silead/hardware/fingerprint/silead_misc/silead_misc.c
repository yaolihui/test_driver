/******************************************************************************
 * @file   silead_misc.c
 * @brief  Contains fingerprint misc functions.
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
 * Calin Wang  2018/7/2    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "misc"
#include "silead_logmsg.h"

#include "silead_config_entity.h"
#include "silead_misc.h"

int32_t silfp_ree_misc_update_cfg(uint8_t upd, uint32_t chipid, uint32_t subid, uint32_t vid)
{
    int32_t ret = 0;

    if (upd) { // need config update
        silfp_ree_cfg_update_entity(chipid, subid, vid);
    }

    silfp_ree_cfg_dbg_entity(chipid, subid, vid);

    return ret;
}

void silfp_ree_misc_set_path(const void *path, uint32_t len)
{
    silfp_ree_cfg_dbg_set_path_entity(path, len);
}