/******************************************************************************
 * @file   silead_dump_util.h
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
 * <author>       <date>     <version>     <desc>
 * Calvin Wang 2019/12/29 0.1.0      Init version
 * Ashely Li   2021/1/15  0.2.0      Update copyright notice
 *
 *****************************************************************************/

#ifndef __SILEAD_DUMP_UTIL_H__
#define __SILEAD_DUMP_UTIL_H__

enum _dump_frame_data_type {
    FRAME_DATA_TYPE_SUFFIX,
    FRAME_DATA_TYPE_EXT,
    FRAME_DATA_TYPE_IMG,
};

enum _dump_mem_flag {
    MEM_FLAG_NONE,          // no memory
    MEM_FLAG_RESIDENT,      // not need malloc, and not need free when clean item
    MEM_FLAG_REMALLOC,      // need remalloc, and need free when clean item
    MEM_FLAG_MALLOCED,      // buf already malloc, and need free when clean item
};

enum _dump_frame_data_flag {
    FRAME_DATA_FLAG_NONE,
    FRAME_DATA_FLAG_NORMAL,
    FRAME_DATA_FLAG_WAIT_SYNC,
};

typedef struct _dump_data {
    uint16_t resident;      // _dump_mem_flag
    uint16_t flag;          // _dump_frame_data_flag
    uint32_t len;
    void *data;
} dump_frame_data_t;

typedef struct _dump_frame {
    uint8_t type;               // frame type
    uint8_t subtype;            // frame sub type
    uint8_t step;               // step
    uint8_t bitcount;           // img bitcount (8/16/24)
    uint16_t width;             // img width
    uint16_t height;            // img height
    uint64_t timestamp;         // img timestamp
    dump_frame_data_t suffix;   // suffix data
    dump_frame_data_t ext;      // ext data
    dump_frame_data_t image;    // image data
} dump_frame_t;

void silfp_dump_util_set_enable(uint8_t enable);

void silfp_dump_util_frame_clear(dump_frame_t *frame);
void silfp_dump_util_clear_all(void);
int32_t silfp_dump_util_frame_add(dump_frame_t *frame);
int32_t silfp_dump_util_frame_add_ext(dump_frame_t *frame);
dump_frame_t* silfp_dump_util_frame_gen(uint8_t type, uint8_t subtype, uint8_t step, uint8_t bitcount, uint16_t width, uint16_t height, uint64_t timestamp);
int32_t silfp_dump_util_frame_append_data(dump_frame_t *frame, void *data, uint32_t len, uint8_t mem_flag, uint8_t data_type);
int32_t silfp_dump_util_frame_append_wait_sync_data(dump_frame_t *frame, uint8_t data_type);
int32_t silfp_dump_util_frame_sync_data(void *data, uint32_t len, uint8_t mem_flag, uint8_t data_type);

int32_t silfp_dump_util_get_data(void *buf, uint32_t size, uint32_t *w, uint32_t *h, uint8_t *bitcount, uint32_t *remain);
int32_t silfp_dump_util_get_info(void *buf, uint32_t size);
dump_frame_t* silfp_dump_util_info_parse(const void *buf, uint32_t size);

#endif /* __SILEAD_DUMP_UTIL_H__ */