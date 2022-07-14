/******************************************************************************
 * @file   silead_stats.h
 * @brief  Contains time statistics functions header file.
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
 * Jack Zhang  2018/4/2   0.1.0      Init version
 * Ashely Li   2021/1/15  0.2.0      Update copyright notice
 *
 *****************************************************************************/

#ifndef __SILEAD_STATS_H__
#define __SILEAD_STATS_H__

void silfp_stats_set_enabled(uint32_t enable);
void silfp_stats_reset(void);
void silfp_stats_start(void);
void silfp_stats_capture_image(void);
void silfp_stats_auth_mismatch(void);
void silfp_stats_auth_match(void);

#endif /* __SILEAD_STATS_H__ */
