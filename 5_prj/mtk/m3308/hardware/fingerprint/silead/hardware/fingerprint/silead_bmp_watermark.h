/******************************************************************************
 * @file   silead_bmp_watermark.h
 * @brief  water mark header
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
 * MelvinCao   2019/12/12 0.1.0      Init version
 * Ashely Li   2021/1/15  0.2.0      Update copyright notice
 *
 *****************************************************************************/

#ifndef __SILEAD_BMP_WATERMARK__
#define __SILEAD_BMP_WATERMARK__

int32_t silead_bmp_add_watermark(void *img, uint32_t offset, uint16_t w, uint16_t h, int8_t depth, int8_t b_inv);

#endif /* __SILEAD_BMP_WATERMARK__ */
