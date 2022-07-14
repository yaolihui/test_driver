/******************************************************************************
 * @file   silead_dump_mraw.h
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
 * Calvin Wang 2020/03/10 0.1.0      Init version
 * Ashely Li   2021/1/15  0.2.0      Update copyright notice
 *
 *****************************************************************************/

#ifndef __SILEAD_DUMP_MRAW_H__
#define __SILEAD_DUMP_MRAW_H__

void silfp_dump_mraw_set_enable(uint8_t enable);

void silfp_dump_mraw_init(void);
void silfp_dump_mraw_deinit(void);

void silfp_dump_mraw_reset(void);
int32_t silfp_dump_mraw_add(void *buf, uint32_t len, dump_frame_t *frame);
int32_t silfp_dump_mraw(uint32_t name_mode, char *index);

#endif /* __SILEAD_DUMP_MRAW_H__ */