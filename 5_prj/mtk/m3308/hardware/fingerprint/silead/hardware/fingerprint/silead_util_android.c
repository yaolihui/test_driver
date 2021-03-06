/******************************************************************************
 * @file   silead_util_android.c
 * @brief  Contains fingerprint utilities functions file.
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

#define FILE_TAG "silead_util"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>
#include <cutils/properties.h>

#define NUM_BASE_10 10
#define NUM_BASE_16 16

int32_t silfp_util_get_str_value(const char *name, uint8_t v)
{
    char buf[PROPERTY_VALUE_MAX] = {0};
    int32_t prop = 0;

    if (name == NULL) {
        LOG_MSG_ERROR("param invalid");
        return -1;
    }

    property_get(name, buf, "");

    if (v != 0xFF && buf[0] == v && strlen(buf) >= 4) {
        prop = (buf[0] & 0x000000FF);
        prop |= ((buf[1] << 8) & 0x0000FF00);
        prop |= ((buf[2] << 16) & 0x00FF0000);
        prop |= ((buf[3] << 24) & 0xFF000000);
    } else if (buf[0] == '0' && (buf[1] == 'x' || buf[1] == 'X')) {
        prop = strtol(buf, NULL, NUM_BASE_16);
    } else {
        prop = strtol(buf, NULL, NUM_BASE_10);
    }

    return prop;
}

int32_t silfp_util_set_str_value(const char *name, uint8_t v)
{
    char buf[PROPERTY_VALUE_MAX] = {0};

    if (name == NULL) {
        LOG_MSG_ERROR("param invalid");
        return -1;
    }

    snprintf(buf, sizeof(buf), "%d", v);
    property_set(name, buf);

    return 0;
}