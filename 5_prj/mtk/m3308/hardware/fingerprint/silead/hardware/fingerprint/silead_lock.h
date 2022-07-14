/******************************************************************************
 * @file   silead_nav.h
 * @brief  Contains fingerprint navi operate functions header file.
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

#ifndef __SILEAD_LOCK_H__
#define __SILEAD_LOCK_H__

int32_t silfp_lock_mode_commond(void);
int32_t silfp_esd_mode_support(void);
int32_t silfp_esd_check_commond(void);

#endif // __SILEAD_LOCK_H__