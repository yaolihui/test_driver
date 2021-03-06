/******************************************************************************
 * @file   silead_ca_impl.h
 * @brief  Contains CA implements header file.
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
 * David Wang  2018/4/2   0.1.0      Init version
 * Ashely Li   2021/1/15  0.2.0      Update copyright notice
 *
 *****************************************************************************/

#ifndef __SILEAD_CA_IMPL_H__
#define __SILEAD_CA_IMPL_H__

typedef struct ca_impl_handle {
    int32_t (*ca_send_modified_command)(uint32_t cmd, void *buffer, uint32_t len, uint32_t flag, uint32_t v1, uint32_t v2, uint32_t *data1, uint32_t *data2);
    int32_t (*ca_send_normal_command)(uint32_t cmd, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4, uint32_t *data1, uint32_t *data2, uint32_t *data3);
    int32_t (*ca_close)(void);
    int32_t (*ca_keymaster_get)(void **buffer);
    int32_t (*ca_sync_ta_log)(int32_t enable);
} ca_impl_handle_t;

#endif /* __SILEAD_CA_IMPL_H__ */

