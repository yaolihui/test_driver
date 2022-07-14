/******************************************************************************
 * @file   silead_dump.h
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

#ifndef __SILEAD_DUMP_H__
#define __SILEAD_DUMP_H__

#include "silead_dump_param.h"

#define CHECK_FLAG_SET(v, f)    (!!((v) & (f)))
#define CHECK_FLAG_UNSET(v, f)  (!((v) & (f)))

/* maxlen should set to -1, if no length limited (include the ext len) */
void silfp_dump_set_name_mode(uint32_t mode, int32_t maxlen, const void *ext, uint32_t len);
void silfp_dump_set_path(const void *path, uint32_t len);
uint32_t silfp_dump_get_name_mode(void);
void silfp_dump_set_version(uint8_t ver);

void silfp_dump_data(int32_t type);
void silfp_dump_data2(uint8_t all);
void silfp_dump_deinit(void);

#endif /* __SILEAD_DUMP_H__ */