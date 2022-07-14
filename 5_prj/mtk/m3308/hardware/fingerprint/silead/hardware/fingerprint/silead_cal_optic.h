/******************************************************************************
 * @file   silead_cal_optic.h
 * @brief  Contains fingerprint calibrate operate functions header file.
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

#ifndef __SILEAD_CALIBRATE_OPTIC_H__
#define __SILEAD_CALIBRATE_OPTIC_H__

int32_t silfp_cal_optic_init(uint32_t feature);
int32_t silfp_cal_optic_deinit(void);

void silfp_cal_optic_set_path(const void *path, uint32_t len);
int32_t silfp_cal_optic_step(uint32_t step);
int32_t silfp_cal_optic_calibrate(void);

#endif /* __SILEAD_CALIBRATE_OPTIC_H__ */
