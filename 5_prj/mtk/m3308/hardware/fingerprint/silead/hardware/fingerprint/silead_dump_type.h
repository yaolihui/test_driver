/******************************************************************************
 * @file   silead_dump_type.h
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
 * Calvin Wang 2019/12/29 0.1.0      Init version
 * Ashely Li   2021/1/15  0.2.0      Update copyright notice
 *
 *****************************************************************************/

#ifndef __SILEAD_DUMP_TYPE_H__
#define __SILEAD_DUMP_TYPE_H__

enum {
    DUMP_IMG_TYPE_AUTH,
    DUMP_IMG_TYPE_ENROLL,
    DUMP_IMG_TYPE_NAV,
    DUMP_IMG_TYPE_CAL,
    DUMP_IMG_TYPE_FT_QA,
    DUMP_IMG_TYPE_SNR,
    DUMP_IMG_TYPE_OTHER
};

enum {
    DUMP_IMG_SUBTYPE_ORIG,
    DUMP_IMG_SUBTYPE_SHOT_FAIL,
    DUMP_IMG_SUBTYPE_SHOT_FAIL_DIFF,
    DUMP_IMG_SUBTYPE_RAW_SUCC,
    DUMP_IMG_SUBTYPE_RAW_FAIL,
    DUMP_IMG_SUBTYPE_RAW_SUCC_DIFF,
    DUMP_IMG_SUBTYPE_RAW_FAIL_DIFF,
    DUMP_IMG_SUBTYPE_RAW,
    DUMP_IMG_SUBTYPE_MRAW,
    DUMP_IMG_SUBTYPE_NEW,
    DUMP_IMG_SUBTYPE_NEW_DIFF,
    DUMP_IMG_SUBTYPE_OTHER
};

#endif /* __SILEAD_DUMP_TYPE_H__ */