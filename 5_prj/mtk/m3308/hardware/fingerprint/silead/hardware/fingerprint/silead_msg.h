/******************************************************************************
 * @file   silead_msg.h
 * @brief  Contains pipe communication header file.
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
 * Luke Ma     2018/4/2   0.1.0      Init version
 * Ashely Li   2021/1/15  0.2.0      Update copyright notice
 *
 *****************************************************************************/

#ifndef __SILEAD_MESSAGE_H__
#define __SILEAD_MESSAGE_H__

typedef enum _msg_type {
    SIFP_MSG_UNKNOW = -1,
    SIFP_MSG_CANCEL = 0,
    SIFP_MSG_IRQ = 1,
    SIFP_MSG_DOWN = 2,
    SIFP_MSG_UP = 3,
    SIFP_MSG_TIMEOUT = 4,
    SIFP_MSG_MAX,
} msg_type_t;

int32_t silfp_msg_init(void);
void silfp_msg_deinit(void);

void silfp_msg_send(int32_t type);
void silfp_msg_clean(void);

int32_t silfp_msg_wait_finger_status(int32_t irq, int32_t down, int32_t up, int32_t cancel, int32_t timeout);
int32_t silfp_msg_is_finger_down(void);
void silfp_msg_reset_finger_status(void);

#endif /* __SILEAD_MESSAGE_H__ */