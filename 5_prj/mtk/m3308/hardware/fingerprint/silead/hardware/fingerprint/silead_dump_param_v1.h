/******************************************************************************
 * @file   silead_dump_param_v1.h
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
 * calvin wang 2018/1/2   0.1.0      Init version
 * Ashely Li   2021/1/15  0.2.0      Update copyright notice
 *
 *****************************************************************************/

#ifndef __SILEAD_DUMP_PARAM_V1_H__
#define __SILEAD_DUMP_PARAM_V1_H__

typedef enum { // v1
    DUMP_IMG_AUTH_SUCC = 0,
    DUMP_IMG_AUTH_FAIL,
    DUMP_IMG_AUTH,
    DUMP_IMG_ENROLL_SUCC,
    DUMP_IMG_ENROLL_FAIL,
    DUMP_IMG_ENROLL,
    DUMP_IMG_NAV_SUCC,
    DUMP_IMG_NAV_FAIL,
    DUMP_IMG_SHOT_SUCC,
    DUMP_IMG_SHOT_FAIL,
    DUMP_IMG_RAW,
    DUMP_IMG_CAL,
    DUMP_IMG_FT_QA,
    DUMP_IMG_AUTH_ORIG,
    DUMP_IMG_ENROLL_ORIG,
    DUMP_IMG_OTHER_ORIG,
    DUMP_IMG_OTHER,
    DUMP_IMG_SNR,
    DUMP_IMG_ENROLL_NEW,
    DUMP_IMG_AUTH_MUL_RAW,
    DUMP_IMG_MAX,
} e_mode_dump_img_t;

#endif /* __SILEAD_DUMP_PARAM_V1_H__ */