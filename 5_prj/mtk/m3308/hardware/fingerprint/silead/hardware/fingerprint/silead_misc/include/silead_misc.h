/******************************************************************************
 * @file   silead_misc.h
 * @brief  Contains fingerprint misc functions header file.
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
 * Calvin Wang  2018/7/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_MISC_H__
#define __SILEAD_MISC_H__

int32_t silfp_ree_misc_update_cfg(uint8_t upd, uint32_t chipid, uint32_t subid, uint32_t vid);

void silfp_ree_misc_set_path(const void *path, uint32_t len);

#endif // __SILEAD_MISC_H__