/******************************************************************************
 * @file   silead_bmp_watermark.c
 * @brief  char or str in bmp
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
 * Melvin Cao  2019/12/12 0.1.0      Init version
 * Ashely Li   2021/1/15  0.2.0      Update copyright notice
 *
 *****************************************************************************/

#define FILE_TAG "bmp_watermark"
#include "log/logmsg.h"
#include "silead_error.h"
#include "silead_bmp_watermark.h"
#include "silead_impl.h"

#ifndef AARCH64_SUPPORT
#include <string.h>
#include <stdlib.h>
#endif /* !AARCH64_SUPPORT */

#define WATERMARK  (char *)"SILEAD"
#define CHAR_GAP   2
#define FONT_WIDTH 8
#define WM_WIDTH   (6*(FONT_WIDTH+CHAR_GAP))

typedef struct {
    char *name;
    int32_t w;
    int32_t h;
    uint8_t *data;
} sil_font;

typedef struct {
    int32_t w;
    int32_t h;
    int32_t x;
    int32_t y;
    int8_t  depth;
    uint8_t *data;
} sil_bmp_location;

#include "font_8x16.h"

static int32_t _print_char_in_bmp(unsigned char c, sil_bmp_location *location, sil_font *font, int8_t b_inv)
{
    int32_t i, j, k;
    uint8_t tmp;
    uint8_t *bmp_data, *bmp_data_tmp;
    uint8_t *font_data;
    sil_bmp_location *location_tmp = location;
    sil_font *font_tmp = font;

    if (!location || !font) {
        LOG_MSG_ERROR("Alloc templatebuffer fail");
        return -SL_ERROR_BAD_PARAMS;
    }

    bmp_data  = (uint8_t *)location_tmp->data;
    font_data = (uint8_t *)font_tmp->data;
    bmp_data  = (uint8_t *)(bmp_data + location_tmp->w * location_tmp->y * location_tmp->depth);

    font_data = font_data + font_tmp->h * c;
    if (b_inv) {
        font_data += (font_tmp->h-1);
    }

    //LOG_MSG_DEBUG("bmp WxH = %dx%d, x,y = %d,%d", location->w, location->h, location->x, location->y);
    //LOG_MSG_DEBUG("font WxH = %dx%d", font_tmp->w, font_tmp->h);

    for (i = 0; i < font->h; i++) {
        bmp_data_tmp = bmp_data + location_tmp->x * location_tmp->depth;
        tmp = *(font_data);

        for (j = 0; j < font->w; j++) {
            if(tmp & 0x80) {
                for (k = 0; k < location_tmp->depth; k++) {
                    *(bmp_data_tmp + k) = 0xFF;
                }
            }

            bmp_data_tmp += location_tmp->depth;
            tmp <<= 1;
        }
        if (b_inv) {
            font_data --;
        } else {
            font_data ++;
        }
        bmp_data = bmp_data + location_tmp->w * location_tmp->depth;
    }

    return 0;
}

static int32_t _print_str_in_bmp(char *str, unsigned char gap, sil_bmp_location *location, sil_font *font, int8_t b_inv)
{
    int32_t x_bak, sum_x = 0;
    sil_bmp_location *location_tmp;
    sil_font *font_tmp;

    if (!location || !font) {
        LOG_MSG_ERROR("location or font NULL");
        return -SL_ERROR_BAD_PARAMS;
    }

    location_tmp = location;
    font_tmp = font;
    x_bak = location_tmp->x;

    for (; *str; str ++) {
        _print_char_in_bmp(*str, location_tmp, font_tmp, b_inv);
        location_tmp->x = x_bak;
        sum_x += (font_tmp->w + gap);
        location_tmp->x = location_tmp->x + sum_x;
        if (location_tmp->x + (font_tmp->w + gap) > location->w) {
            //LOG_MSG_DEBUG("Exceed width, ignore");
            break;
        }
    }

    location->x = x_bak;
    return 0;
}

int32_t silead_bmp_add_watermark(void *img, uint32_t offset, uint16_t w, uint16_t h, int8_t depth, int8_t b_inv)
{
    sil_bmp_location location = {
        .w = w,
        .h = h,
        .x = 0,
        .y = rand() % h,
        .data = (uint8_t *)img,
    };

    if (!silfp_impl_is_add_watermark()) {
        return SL_SUCCESS;
    }

    if (!img) {
        LOG_MSG_ERROR("image is NULL");
        return -1;
    }

    location.data += offset;
    location.depth = depth;
    if (location.y + font_8x16.h >= h-1) {
        location.y = h - font_8x16.h - 2;
    }

    if (w > WM_WIDTH) {
        location.x = rand() % (w - WM_WIDTH);
    }
    if (location.x > w) {
        location.x = 0;
    }

    return _print_str_in_bmp(WATERMARK, CHAR_GAP, &location, &font_8x16, b_inv);
}
