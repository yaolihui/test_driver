/******************************************************************************
 * @file   silead_xml.cpp
 * @brief  Contains XML parse functions.
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
 * calvin wang  2018/12/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LOG_DBG_VERBOSE
#define LOG_DBG_VERBOSE 0 // disable VERBOSE log
#endif /* LOG_DBG_VERBOSE */

#define FILE_TAG "xml"
#include "silead_logmsg.h"

#include "silead_util.h"
#include "silead_util_ext.h"
#include "silead_xml.h"

#ifdef __cplusplus
}
#endif

#include <inttypes.h>

#include "tinyxml2.h"

using namespace tinyxml2;

#define FP_SYSPARAM_PATH1 "/vendor/etc/silead/sysparms"
#define FP_SYSPARAM_PATH2 "/system/etc/silead/sysparms"
#define FP_SYSPARAM_CONFIG_FILE_NAME "silead_config.xml"
#define FP_SYSPARAM_PARAM_FILE_NAME "silead_param.xml"
#define FP_SYSPARAM_ALGO_FILE_NAME "silead_algo.xml"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#define UINT_MATCH(v1, v2, mask) (((v1) & (mask)) == ((v2) & (mask)))
#define XML_GET_ELEM(list_item, item_name, parent_elem, sub_elem)                       \
    if (sub_seach) {                                                                    \
        if (list_item.item_name == NULL) {                                              \
            element = parent_elem->FirstChildElement(list_item.item);                   \
            sub_seach = 0;                                                              \
        } else {                                                                        \
            if ((sub_elem == NULL) || strcmp(list_item.item_name, sub_elem->Name())) {  \
                sub_elem = parent_elem->FirstChildElement(list_item.item_name);         \
            }                                                                           \
            if (sub_elem == NULL) {                                                     \
                continue;                                                               \
            }                                                                           \
        }                                                                               \
    }

#define XML_PATH_SEPRATE "/"
#define CHUNK_SIZE 1024
#define CHUNK_SIZE_MAX (8*1024)

typedef struct xml_parse_item {
    const char *parent_name;
    const char *sub_parent_name;
    const char *third_parent_name;
    const char *item;
    uint32_t id;
} xml_parse_item_t;

#include "silead_xml_item.h"
#ifdef SIL_FP_CFG_V1_SUPPORT
#include "xml_v1/silead_xml_item_v1.h"
#endif /* SIL_FP_CFG_V1_SUPPORT */

static xml_parse_item_t *m_param_parse_item = m_param_parse_item_default;
static xml_parse_item_t *m_config_parse_item = m_config_parse_item_default;
static xml_parse_item_t *m_algo_parse_item = m_algo_parse_item_default;
static uint32_t m_param_count = ARRAY_SIZE(m_param_parse_item_default);
static uint32_t m_config_count = ARRAY_SIZE(m_config_parse_item_default);
static uint32_t m_algo_count = ARRAY_SIZE(m_algo_parse_item_default);

static char m_str_xml_path[PATH_MAX] = {0};
static const char *_xml_get_path(void)
{
    if (m_str_xml_path[0] != '\0') {
        return m_str_xml_path;
    } else {
        return NULL;
    }
}

extern "C" void silfp_ree_xml_set_path(const void *path, uint32_t len)
{
    int32_t ret = 0;

    ret = silfp_ree_util_path_copy(m_str_xml_path, sizeof(m_str_xml_path), path, len);
    if (ret < 0) {
        memset(m_str_xml_path, 0, sizeof(m_str_xml_path));
    }
    LOG_MSG_VERBOSE("path = %s", m_str_xml_path);
}

extern "C" void silfp_ree_xml_set_version(uint32_t ver)
{
    UNUSED(ver);
#ifdef SIL_FP_CFG_V1_SUPPORT
    if (ver == 1) {
        m_param_parse_item = m_param_parse_v1_item;
        m_config_parse_item = m_config_parse_v1_item;
        m_param_count = ARRAY_SIZE(m_param_parse_v1_item);
        m_config_count = ARRAY_SIZE(m_config_parse_v1_item);
        LOG_MSG_DEBUG("set config update to v1");
    }
#endif /* SIL_FP_CFG_V1_SUPPORT */
}

static int32_t _xml_upd_item_array(void **ppbuf, uint32_t *psize, uint32_t *poffset, uint32_t id, void *array, uint32_t len)
{
    uint8_t *p = NULL;
    uint32_t size = 0;
    uint32_t offset = 0;
    uint32_t item_data_size = 0;

    if ((ppbuf == NULL) || (psize == NULL) || (poffset == NULL)) {
        return -1;
    }

    if (array == NULL) {
        len = 0;
    }

    offset = *poffset;
    size = *psize;

    item_data_size = len + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t);
    if ((*ppbuf == NULL) || (size < offset + item_data_size)) {
        size += ((item_data_size + CHUNK_SIZE - 1) & (~(CHUNK_SIZE - 1)));
        p = (uint8_t *)malloc(size);
        if (p != NULL) {
            memset(p, 0, size);
            if ((*ppbuf) != NULL) {
                memcpy(p, (*ppbuf), offset);
                free(*ppbuf);
            }
            (*ppbuf) = p;
            (*psize) = size;
        } else {
            LOG_MSG_ERROR("buf malloc failed");
        }
    } else {
        p = (uint8_t *)(*ppbuf);
    }

    if (p != NULL) {
        /* item total size */
        memcpy(p + offset, &item_data_size, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        /* item id */
        memcpy(p + offset, &id, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        /* item data size & data */
        memcpy(p + offset, &len, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        if ((array != NULL) && (len > 0)) {
            memcpy(p + offset, array, len);
            offset += len;
        }

        (*poffset) = offset;

        return 0;
    }

    return -1;
}

static int32_t _xml_upd_fill_header(void *pbuf, uint32_t size, uint32_t data_len, uint32_t magic)
{
    uint8_t *data = NULL;

    data = (uint8_t *)pbuf;
    if ((data == NULL) || (size <= sizeof(uint32_t) * 2) || (size < data_len)) {
        return -1;
    }

    memcpy(data, &data_len, sizeof(uint32_t)); // all data size
    memcpy(data + 4, &magic, sizeof(uint32_t)); // miagic data

    return 0;
}

static int32_t _xml_upd_mem_setup(void **ppbuf, uint32_t *psize, uint32_t *poffset)
{
    uint32_t size = 0;

    if ((ppbuf == NULL) || (psize == NULL) || (poffset == NULL)) {
        return -1;
    }

    if (*ppbuf != NULL) {
        return 0;
    }

    size = CHUNK_SIZE;
    *ppbuf = malloc(size);
    if (*ppbuf == NULL) {
        LOG_MSG_ERROR("buf malloc failed");
        return -1;
    }

    memset(*ppbuf, 0, size);
    *psize = size;
    *poffset = sizeof(uint32_t) * 2; // buf size & magic num

    return 0;
}

static void _xml_upd_mem_unsetup(void **ppbuf, uint32_t *psize, uint32_t *poffset, uint8_t freed)
{
    if ((ppbuf != NULL) && (*ppbuf != NULL)) {
        if (freed) {
            free(*ppbuf);
        }
        *ppbuf = NULL;
    }
    if (psize != NULL) {
        *psize = 0;
    }
    if (poffset != NULL) {
        *poffset = 0;
    }
}

static int32_t _xml_upd_append_list(void **ppbuf, uint32_t *psize, uint32_t *poffset, uint32_t id, uint8_t force)
{
    int32_t ret = 0;
    uint32_t magic = CFG_UPD_MAGIC;

    if ((ppbuf == NULL) || (psize == NULL) || (poffset == NULL)) { // buf invalid
        return -1; // no need append
    }

    if ((!force) && (*poffset < CHUNK_SIZE_MAX)) { // not fill enough data, no append
        return 0;
    }

    if (force && (*poffset <= sizeof(uint32_t) * 2)) { // force, but data invalid, free
        _xml_upd_mem_unsetup(ppbuf, psize, poffset, 1);
        return -1;
    }

    if (id == CFG_UPD_TYPE_ALGO) {
        magic = ALGO_CFG_UPD_MAGIC;
    }

    // fill data header and append to list
    ret = _xml_upd_fill_header(*ppbuf, *psize, *poffset, magic);
    if (ret >= 0) {
        ret = silfp_ree_xml_util_item_add(id, *ppbuf, *poffset);
    }

    if (ret < 0) { // append fail, buf free
        _xml_upd_mem_unsetup(ppbuf, psize, poffset, 1);
        return -1;
    }

    _xml_upd_mem_unsetup(ppbuf, psize, poffset, 0);
    return 0;
}

static uint32_t _xml_str_get_character_count(const char *str, const char c)
{
    uint32_t count = 0;
    const char *p = str;

    if (str == NULL) {
        return 0;
    }

    while (*p != 0) {
        if (*p == c) {
            count++;
        }
        p++;
    }

    return count;
}

static uint32_t _xml_str_to_uint32(const char *nptr, char **endptr, int32_t base)
{
    while ((*nptr == ' ') || (*nptr == '\t')) {
        nptr++;
    }
    if (base == 10) {
        if ((nptr[0] == '0') && ((nptr[1] == 'x') || (nptr[1] == 'X'))) {
            base = 16;
        }
    }
    return strtoul(nptr, endptr, base);
}

static int32_t _xml_str_to_int32(const char *nptr, char **endptr, int32_t base)
{
    int32_t ret = 0;

    while ((*nptr == ' ') || (*nptr == '\t')) {
        nptr++;
    }
    if ((nptr[0] == '0') && ((nptr[1] == 'x') || (nptr[1] == 'X'))) {
        ret = _xml_str_to_uint32(nptr, endptr, 16);
    } else {
        base = 10;
        ret = strtol(nptr, endptr, base);
        if (ret == 0x7FFFFFFF) {
            ret = _xml_str_to_uint32(nptr, endptr, base);
        }
    }

    return ret;
}

static float _xml_str_to_float(const char *nptr, char **endptr)
{
    return strtof(nptr, endptr);
}

static double _xml_str_to_double(const char *nptr, char **endptr)
{
    return strtod(nptr, endptr);
}

static uint32_t _xml_str_to_uint32_array(char *str, const char *delims, uint32_t *value, uint32_t size)
{
    uint32_t count = 0;
    char *saveptr = NULL;

    char *sValue = strtok_r(str, delims, &saveptr);
    while (sValue != NULL) {
        if (sValue != NULL) {
            if (count < size) {
                value[count++] = _xml_str_to_uint32(sValue, NULL, 16);
            } else {
                LOG_MSG_ERROR("should fix me? data more then %d", size);
                break;
            }
        }

        sValue = strtok_r(NULL, delims, &saveptr);
    }
    return count;
}

static uint32_t _xml_str_to_int32_array(const char *str, int32_t *value, uint32_t size)
{
    uint32_t count = 0;
    const char *p = str;
    char *end = NULL;

    while ((p != NULL) && (*p != '\0')) {
        if ((*p == '/') && (*(p+1) == '/')) {
            while (!((*p =='\n') || (*p == '\0'))) {
                p++;
            }
        }
        if ((*p >= '0' && *p <= '9') || (*p == '+') || (*p == '-')) {
            if (count < size) {
                value[count++] = _xml_str_to_int32(p, &end, 10);
                p = end;
            } else {
                LOG_MSG_ERROR("should fix me? data more then %d", size);
                break;
            }
        } else {
            p++;
        }
    }

    return count;
}

static uint32_t _xml_str_to_float_array(const char *str, float *value, uint32_t size)
{
    uint32_t count = 0;
    const char *p = str;
    char *end = NULL;

    while ((p != NULL) && (*p != '\0')) {
        if ((*p == '/') && (*(p+1) == '/')) {
            while (!((*p =='\n') || (*p == '\0'))) {
                p++;
            }
        }
        if ((*p >= '0' && *p <= '9') || (*p == '+') || (*p == '-') || (*p == '.')) {
            if (count < size) {
                value[count++] = _xml_str_to_float(p, &end);
                p = end;
            } else {
                LOG_MSG_ERROR("should fix me? data more then %d", size);
                break;
            }
        } else {
            p++;
        }
    }

    return count;
}

static uint32_t _xml_str_to_double_array(const char *str, double *value, uint32_t size)
{
    uint32_t count = 0;
    const char *p = str;
    char *end = NULL;

    while ((p != NULL) && (*p != '\0') && (count < size)) {
        if ((*p == '/') && (*(p+1) == '/')) {
            while (!((*p =='\n') || (*p == '\0'))) {
                p++;
            }
        }
        if ((*p >= '0' && *p <= '9') || (*p == '+') || (*p == '-') || (*p == '.')) {
            if (count < size) {
                value[count++] = _xml_str_to_double(p, &end);
                p = end;
            } else {
                LOG_MSG_ERROR("should fix me? data more then %d", size);
                break;
            }
        } else {
            p++;
        }
    }

    return count;
}

static int32_t _xml_str_to_tag_item_value(const char *str, void *data, uint32_t size, uint32_t *poffset)
{
    const char *p = str;
    char *pbuf = (char *)data;
    uint32_t offset = 0;
    int32_t value = 0;

    if ((p == NULL) || (pbuf == NULL) || (poffset == NULL)) {
        return -1;
    }

    offset = *poffset;
    if (size < (offset + sizeof(uint32_t) * 2)) {
        return -1;
    }

    value = sizeof(uint32_t);
    memcpy(pbuf + offset, &value, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    value = _xml_str_to_int32(p, NULL, 0);
    memcpy(pbuf + offset, &value, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    *poffset = offset;
    return 0;
}

static int32_t _xml_str_to_tag_item_data(const char *str, void *data, uint32_t size, uint32_t *poffset)
{
    const char *p = str;
    char *pbuf = (char *)data;
    uint32_t offset = 0;
    uint32_t tag_len = 0;

    if ((p == NULL) || (pbuf == NULL) || (poffset == NULL)) {
        return -1;
    }

    offset = *poffset;
    if (size < (offset + sizeof(uint32_t))) {
        return -1;
    }

    offset += sizeof(uint32_t);
    while ((p != NULL) && (*p != '\0')) { // skip invalid data
        if ((*p >= '0' && *p <= '9') || (*p >= 'A' && *p <= 'Z') || (*p == '_') || (*p == '-')) {
            break;
        }
        p++;
    }

    while ((*p != '\0') && ((*p >= '0' && *p <= '9') || (*p >= 'A' && *p <= 'Z') || (*p == '_') || (*p == '-'))) {
        if (size < offset + 1) {
            return -1;
        }
        pbuf[offset++] = *(p++);
        tag_len++;
    }

    if (size < offset + 1) {
        return -1;
    }
    pbuf[offset++] = '\0';
    tag_len++;

    memcpy(pbuf + (*poffset), &tag_len, sizeof(uint32_t));
    *poffset = offset;

    return 0;
}

static uint32_t _xml_str_to_tag_item(char *str, const char *delims, void *data, uint32_t size, uint32_t *poffset)
{
    uint32_t count = 0;
    char *saveptr = NULL;
    uint32_t offset = 0;

    offset = *poffset;
    char *value = strtok_r(str, delims, &saveptr);
    while ((value != NULL) && (count < 2)) {
        if (count == 0) {
            if (_xml_str_to_tag_item_data(value, data, size, &offset) < 0) {
                return 0;
            }
        } else {
            if (_xml_str_to_tag_item_value(value, data, size, &offset) < 0) {
                return 0;
            }
        }
        count++;
        value = strtok_r(NULL, delims, &saveptr);
    }

    if (count != 2) {
        return 0;
    }

    *poffset = offset;
    return count;
}

static uint32_t _xml_get_array_number(const XMLElement *element)
{
    uint32_t number = 0;

    if (element == NULL) {
        return 0;
    }

    const XMLAttribute *numAttribute = element->FindAttribute("n");
    if (numAttribute == NULL) {
        return 0;
    }

    const char *sNumber = numAttribute->Value();
    if (sNumber == NULL) {
        return 0;
    }

    number = _xml_str_to_uint32(sNumber, NULL, 10);
    if (number <= 0) {
        return 0;
    }

    return number;
}

static int32_t _xml_get_int32_array(const XMLElement *element, void **buf, uint32_t *size, uint32_t *offset, xml_parse_item_t *parse_item)
{
    uint32_t number = 0;

    if ((element == NULL) || (parse_item == NULL)) {
        return -1;
    }

    number = _xml_get_array_number(element);
    if (number == 0) {
        return -1;
    }

    LOG_MSG_VERBOSE("number:%d", number);
    const char *sParamValues = element->GetText();
    if (sParamValues == NULL) {
        return -1;
    }

    int32_t *params = (int32_t *)malloc(sizeof(int32_t) * number);
    if (params == NULL) {
        return -1;
    }

    memset(params, 0, sizeof(int32_t) * number);

    uint32_t count = _xml_str_to_int32_array(sParamValues, params, number);
    if (count != number) {
        LOG_MSG_ERROR("should fix me? (n mismatch, %d but %d)", count, number);
    }

    if (count > 0) {
        _xml_upd_item_array(buf, size, offset, parse_item->id, params, count * sizeof(int32_t));
        LOG_MSG_VERBOSE("update ArrI: (%s%s%s%s%s%s%s) (0x%08x) count(%d)",
                        (parse_item->parent_name == NULL) ? "" : parse_item->parent_name,
                        (parse_item->sub_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->sub_parent_name == NULL) ? "" : parse_item->sub_parent_name,
                        (parse_item->third_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->third_parent_name == NULL) ? "" : parse_item->third_parent_name,
                        (parse_item->item == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->item == NULL) ? "" : parse_item->item,
                        parse_item->id, count);
    }

    free(params);

    return 0;
}

static int32_t _xml_get_int32_item(const XMLElement *element, void **buf, uint32_t *size, uint32_t *offset, xml_parse_item_t *parse_item)
{
    int32_t valuei32 = 0;
    uint32_t value32 = 0;

    if ((element == NULL) || (parse_item == NULL)) {
        return -1;
    }

    const char *svalue = element->GetText();
    if (svalue == NULL) {
        return -1;
    }

    if ((strlen(svalue) > 2) && (svalue[0] == '0') && (svalue[1] == 'x')) { // 16bit data
        value32 = _xml_str_to_uint32(svalue, NULL, 16);
        _xml_upd_item_array(buf, size, offset, parse_item->id, &value32, sizeof(uint32_t));
        LOG_MSG_VERBOSE("update: (%s%s%s%s%s%s%s) (0x%08x) = 0x%x",
                        (parse_item->parent_name == NULL) ? "" : parse_item->parent_name,
                        (parse_item->sub_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->sub_parent_name == NULL) ? "" : parse_item->sub_parent_name,
                        (parse_item->third_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->third_parent_name == NULL) ? "" : parse_item->third_parent_name,
                        (parse_item->item == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->item == NULL) ? "" : parse_item->item,
                        parse_item->id, value32);
    } else {
        valuei32 = _xml_str_to_int32(svalue, NULL, 10);
        _xml_upd_item_array(buf, size, offset, parse_item->id, &valuei32, sizeof(int32_t));
        LOG_MSG_VERBOSE("update: (%s%s%s%s%s%s%s) (0x%08x) = %d",
                        (parse_item->parent_name == NULL) ? "" : parse_item->parent_name,
                        (parse_item->sub_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->sub_parent_name == NULL) ? "" : parse_item->sub_parent_name,
                        (parse_item->third_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->third_parent_name == NULL) ? "" : parse_item->third_parent_name,
                        (parse_item->item == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->item == NULL) ? "" : parse_item->item,
                        parse_item->id, valuei32);
    }

    return 0;
}

static int32_t _xml_get_uu_array(const XMLElement *element, void **buf, uint32_t *size, uint32_t *offset, xml_parse_item_t *parse_item)
{
    uint32_t value[2] = {0};
    uint32_t count = 0;
    uint32_t number = 0;

    const char *REGS_LIST_DELIM = "\n";
    const char *REG_ITEM_DELIM = ":";

    if ((element == NULL) || (parse_item == NULL)) {
        return -1;
    }

    char *sRegsList = (char *)element->GetText();
    if (sRegsList == NULL) {
        return -1;
    }

    number = _xml_str_get_character_count(sRegsList, REG_ITEM_DELIM[0]);
    if (number <= 0) {
        return -1;
    }

    reg_cfg_t *preg = (reg_cfg_t *)malloc(sizeof(reg_cfg_t) * number);
    if (preg == NULL) {
        return -1;
    }

    memset(preg, 0, sizeof(reg_cfg_t) * number);

    char *saveptr = NULL;
    char *sRegItem = strtok_r(sRegsList, REGS_LIST_DELIM, &saveptr); // get item 0xXXXXXXXX:0xXXXXXXXX
    while (sRegItem != NULL) {
        if (_xml_str_to_uint32_array(sRegItem, REG_ITEM_DELIM, value, ARRAY_SIZE(value)) == ARRAY_SIZE(value)) {
            preg[count].addr = value[0];
            preg[count].val = value[1];
            count++;
        }
        sRegItem = strtok_r(NULL, REGS_LIST_DELIM, &saveptr);
    }

    if (count > 0) {
        _xml_upd_item_array(buf, size, offset, parse_item->id, preg, count * sizeof(reg_cfg_t));
        LOG_MSG_VERBOSE("update ArrUU: (%s%s%s%s%s%s%s) (0x%08x) count(%d)",
                        (parse_item->parent_name == NULL) ? "" : parse_item->parent_name,
                        (parse_item->sub_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->sub_parent_name == NULL) ? "" : parse_item->sub_parent_name,
                        (parse_item->third_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->third_parent_name == NULL) ? "" : parse_item->third_parent_name,
                        (parse_item->item == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->item == NULL) ? "" : parse_item->item,
                        parse_item->id, count);
    }
    free(preg);

    return 0;
}

static int32_t _xml_get_float_array(const XMLElement *element, void **buf, uint32_t *size, uint32_t *offset, xml_parse_item_t *parse_item)
{
    uint32_t number = 0;

    if ((element == NULL) || (parse_item == NULL)) {
        return -1;
    }

    number = _xml_get_array_number(element);
    if (number == 0) {
        return -1;
    }

    LOG_MSG_VERBOSE("number:%d", number);
    const char *sParamValues = element->GetText();
    if (sParamValues == NULL) {
        return -1;
    }

    float *params = (float *)malloc(sizeof(float) * number);
    if (params == NULL) {
        return -1;
    }

    memset(params, 0, sizeof(float) * number);

    uint32_t count = _xml_str_to_float_array(sParamValues, params, number);
    if (count != number) {
        LOG_MSG_ERROR("should fix me? (n mismatch, %d but %d)", count, number);
    }

    if (count > 0) {
        _xml_upd_item_array(buf, size, offset, parse_item->id, params, count * sizeof(float));
        LOG_MSG_VERBOSE("update ArrF: (%s%s%s%s%s%s%s) (0x%08x) count(%d)",
                        (parse_item->parent_name == NULL) ? "" : parse_item->parent_name,
                        (parse_item->sub_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->sub_parent_name == NULL) ? "" : parse_item->sub_parent_name,
                        (parse_item->third_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->third_parent_name == NULL) ? "" : parse_item->third_parent_name,
                        (parse_item->item == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->item == NULL) ? "" : parse_item->item,
                        parse_item->id, count);
    }

    free(params);

    return 0;
}

static int32_t _xml_get_double_array(const XMLElement *element, void **buf, uint32_t *size, uint32_t *offset, xml_parse_item_t *parse_item)
{
    uint32_t number = 0;

    if ((element == NULL) || (parse_item == NULL)) {
        return -1;
    }

    number = _xml_get_array_number(element);
    if (number == 0) {
        return -1;
    }

    LOG_MSG_VERBOSE("number:%d", number);
    const char *sParamValues = element->GetText();
    if (sParamValues == NULL) {
        return -1;
    }

    double *params = (double *)malloc(sizeof(double) * number);
    if (params == NULL) {
        return -1;
    }

    memset(params, 0, sizeof(double) * number);

    uint32_t count = _xml_str_to_double_array(sParamValues, params, number);
    if (count != number) {
        LOG_MSG_ERROR("should fix me? (n mismatch, %d but %d)", count, number);
    }

    if (count > 0) {
        _xml_upd_item_array(buf, size, offset, parse_item->id, params, count * sizeof(double));
        LOG_MSG_VERBOSE("update ArrD: (%s%s%s%s%s%s%s) (0x%08x) count(%d)",
                        (parse_item->parent_name == NULL) ? "" : parse_item->parent_name,
                        (parse_item->sub_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->sub_parent_name == NULL) ? "" : parse_item->sub_parent_name,
                        (parse_item->third_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->third_parent_name == NULL) ? "" : parse_item->third_parent_name,
                        (parse_item->item == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->item == NULL) ? "" : parse_item->item,
                        parse_item->id, count);
    }

    free(params);

    return 0;
}

static int32_t _xml_get_tag_array(const XMLElement *element, void **buf, uint32_t *size, uint32_t *offset, xml_parse_item_t *parse_item)
{
    uint32_t number = 0;
    uint32_t count = 0;

    const char *TAG_LIST_DELIM = "\n";
    const char *TAG_ITEM_DELIM = ":";

    if ((element == NULL) || (parse_item == NULL)) {
        return -1;
    }

    number = _xml_get_array_number(element);
    if (number == 0) {
        return -1;
    }

    LOG_MSG_VERBOSE("number:%d", number);
    char *sParamValues = (char *)element->GetText();
    if (sParamValues == NULL) {
        return -1;
    }

    count = _xml_str_get_character_count(sParamValues, TAG_ITEM_DELIM[0]);
    if (count <= 0) {
        return -1;
    }

    if (count != number) {
        LOG_MSG_ERROR("should fix me? (n mismatch, %d but %d)", count, number);
    }

    uint32_t params_offset = sizeof(uint32_t);
    uint32_t params_size = (128 + sizeof(uint32_t) * 3) * number + sizeof(uint32_t);
    char *params = (char *)malloc(params_size);
    if (params == NULL) {
        return -1;
    }
    memset(params, 0, params_size);

    count = 0;
    char *saveptr = NULL;
    char *sTagItem = strtok_r(sParamValues, TAG_LIST_DELIM, &saveptr); // get item XXXXXXXX:XXXXXXXX
    while ((sTagItem != NULL) && (count < number)) {
        if (_xml_str_to_tag_item(sTagItem, TAG_ITEM_DELIM, params, params_size, &params_offset) > 0) {
            count++;
        }
        sTagItem = strtok_r(NULL, TAG_LIST_DELIM, &saveptr);
    }
    if (count != number) {
        LOG_MSG_ERROR("should fix me? (n mismatch, %d but %d)", count, number);
    }

    if ((count > 0) && (params_offset > sizeof(uint32_t))) {
        memcpy(params, &count, sizeof(uint32_t));
        _xml_upd_item_array(buf, size, offset, parse_item->id, params, params_offset);
        LOG_MSG_VERBOSE("update ArrTI: (%s%s%s%s%s%s%s) (0x%08x) count(%d %d)",
                        (parse_item->parent_name == NULL) ? "" : parse_item->parent_name,
                        (parse_item->sub_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->sub_parent_name == NULL) ? "" : parse_item->sub_parent_name,
                        (parse_item->third_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->third_parent_name == NULL) ? "" : parse_item->third_parent_name,
                        (parse_item->item == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->item == NULL) ? "" : parse_item->item,
                        parse_item->id, count, params_offset);
    }

    free(params);

    return 0;
}

static int32_t _xml_get_param_item_elem_int32(const XMLElement *element, void *data, uint32_t size, uint32_t *poffset, uint32_t id)
{
    int32_t value = 0;
    char *pbuf = (char *)data;
    uint32_t offset = 0;

    if ((element == NULL) || (pbuf == NULL) || (poffset == NULL) || (size < (*poffset) + sizeof(uint32_t) * 3)) {
        return -1;
    }

    const char *svalue = element->GetText();
    if (svalue == NULL) {
        return -1;
    }

    offset = *poffset;
    memcpy(pbuf + offset, &id, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    value = sizeof(uint32_t);
    memcpy(pbuf + offset, &value, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    value = _xml_str_to_int32(svalue, NULL, 0);
    memcpy(pbuf + offset, &value, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    *poffset = offset;
    LOG_MSG_VERBOSE("update I: param item elem (%d), %d HEX(0x%x)", id, value, value);

    return 0;
}

static int32_t _xml_get_param_item_elem_int32_array(const XMLElement *element, void *data, uint32_t size, uint32_t *poffset, uint32_t id, uint32_t number)
{
    uint32_t count = 0;
    char *pbuf = (char *)data;
    uint32_t offset = 0;

    if ((element == NULL) || (pbuf == NULL) || (poffset == NULL) || (size < (*poffset) + sizeof(uint32_t) * (number + 2))) {
        return -1;
    }

    offset = *poffset;
    memset(pbuf + offset, 0, sizeof(uint32_t) * 2);
    if (number == 0) {
        return -1;
    }

    LOG_MSG_VERBOSE("number:%d", number);
    const char *svalue = element->GetText();
    if (svalue == NULL) {
        return -1;
    }

    int32_t *params = (int32_t *)malloc(sizeof(int32_t) * number);
    if (params == NULL) {
        return -1;
    }

    memset(params, 0, sizeof(int32_t) * number);
    count = _xml_str_to_int32_array(svalue, params, number);
    if (count != number) {
        LOG_MSG_ERROR("should fix me? (n mismatch, %d but %d)", count, number);
    }

    if (count > 0) {
        memcpy(pbuf + offset, &id, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        number = count * sizeof(uint32_t);
        memcpy(pbuf + offset, &number, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        memcpy(pbuf + offset, params, number);
        offset += number;

        *poffset = offset;
        LOG_MSG_VERBOSE("update ArrI: param item elem (%d), %d", id, count);
    }

    free(params);

    return 0;
}

static int32_t _xml_get_param_item_elem_float_array(const XMLElement *element, void *data, uint32_t size, uint32_t *poffset, uint32_t id, uint32_t number)
{
    uint32_t count = 0;
    char *pbuf = (char *)data;
    uint32_t offset = 0;

    if ((element == NULL) || (pbuf == NULL) || (poffset == NULL) || (size < (*poffset) + sizeof(float) * number + sizeof(uint32_t) * 2)) {
        return -1;
    }

    offset = *poffset;
    memset(pbuf + offset, 0, sizeof(uint32_t) * 2);
    if (number == 0) {
        return -1;
    }

    LOG_MSG_VERBOSE("number:%d", number);
    const char *svalue = element->GetText();
    if (svalue == NULL) {
        return -1;
    }

    float *params = (float *)malloc(sizeof(float) * number);
    if (params == NULL) {
        return -1;
    }

    memset(params, 0, sizeof(float) * number);
    count = _xml_str_to_float_array(svalue, params, number);
    if (count != number) {
        LOG_MSG_ERROR("should fix me? (n mismatch, %d but %d)", count, number);
    }

    if (count > 0) {
        memcpy(pbuf + offset, &id, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        number = count * sizeof(float);
        memcpy(pbuf + offset, &number, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        memcpy(pbuf + offset, params, number);
        offset += number;

        *poffset = offset;
        LOG_MSG_VERBOSE("update ArrF: param item elem (%d), %d", id, count);
    }

    free(params);

    return 0;
}

static int32_t _xml_get_param_item_elem_double_array(const XMLElement *element, void *data, uint32_t size, uint32_t *poffset, uint32_t id, uint32_t number)
{
    uint32_t count = 0;
    char *pbuf = (char *)data;
    uint32_t offset = 0;

    if ((element == NULL) || (pbuf == NULL) || (poffset == NULL) || (size < (*poffset) + sizeof(double) * number + sizeof(uint32_t) * 2)) {
        return -1;
    }

    offset = *poffset;
    memset(pbuf + offset, 0, sizeof(uint32_t) * 2);
    if (number == 0) {
        return -1;
    }

    LOG_MSG_VERBOSE("number:%d", number);
    const char *svalue = element->GetText();
    if (svalue == NULL) {
        return -1;
    }

    double *params = (double *)malloc(sizeof(double) * number);
    if (params == NULL) {
        return -1;
    }

    memset(params, 0, sizeof(double) * number);
    count = _xml_str_to_double_array(svalue, params, number);
    if (count != number) {
        LOG_MSG_ERROR("should fix me? (n mismatch, %d but %d)", count, number);
    }

    if (count > 0) {
        memcpy(pbuf + offset, &id, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        number = count * sizeof(double);
        memcpy(pbuf + offset, &number, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        memcpy(pbuf + offset, params, number);
        offset += number;

        *poffset = offset;
        LOG_MSG_VERBOSE("update ArrF: param item elem (%d), %d", id, count);
    }

    free(params);

    return 0;
}

static int32_t _xml_get_param_item_elem(const XMLElement *element, void *data, uint32_t size, uint32_t *poffset, uint32_t id, uint32_t num)
{
    const XMLAttribute *type_attribute = NULL;
    char *data_type = NULL;

    if (element == NULL) {
        return -1;
    }

    type_attribute = element->FindAttribute("st");
    if (type_attribute != NULL) {
        data_type = (char *)type_attribute->Value();
    }

    if (data_type != NULL) {
        if (!strcmp("ArrI", data_type)) {
            _xml_get_param_item_elem_int32_array(element, data, size, poffset, id, num);
        } else if (!strcmp("ArrF", data_type)) {
            _xml_get_param_item_elem_float_array(element, data, size, poffset, id, num);
        } else if (!strcmp("ArrD", data_type)) {
            _xml_get_param_item_elem_double_array(element, data, size, poffset, id, num);
        } else {
            data_type = NULL;
        }
    }

    if (data_type == NULL) {
        _xml_get_param_item_elem_int32(element, data, size, poffset, id);
    }

    return 0;
}

static uint32_t _xml_get_param_item_data_info(const XMLElement *element, uint32_t *nums, uint32_t count)
{
    uint32_t size = 0;
    uint32_t i = 0;

    if ((element == NULL) || (nums == NULL) || (count == 0)) {
        return 0;
    }

    if (ARRAY_SIZE(m_algo_param_item) != count) {
        LOG_MSG_ERROR("num count invalid, %d but %d", count, ARRAY_SIZE(m_algo_param_item));
        return 0;
    }

    for (i = 0; i < count; i++) {
        const XMLElement *sub_element = element->FirstChildElement(m_algo_param_item[i].item);
        if (sub_element == NULL) {
            continue;
        }

        char *data_type = NULL;
        const XMLAttribute *type_attribute = sub_element->FindAttribute("st");
        if (type_attribute != NULL) {
            data_type = (char *)type_attribute->Value();
        }
        if (data_type != NULL) {
            if (!strcmp("ArrI", data_type)) {
                nums[i] = _xml_get_array_number(sub_element);
                size += nums[i] * sizeof(uint32_t) + sizeof(uint32_t) * 2;
            } else if (!strcmp("ArrF", data_type)) {
                nums[i] = _xml_get_array_number(sub_element);
                size += nums[i] * sizeof(float) + sizeof(uint32_t) * 2;
            } else if (!strcmp("ArrD", data_type)) {
                nums[i] = _xml_get_array_number(sub_element);
                size += nums[i] * sizeof(double) + sizeof(uint32_t) * 2;
            } else {
                data_type = NULL;
            }
        }
        if (data_type == NULL) {
            nums[i] = 1;
            size += sizeof(uint32_t) * 3;
        }
    }

    return size;
}

static int32_t _xml_get_param_item(const XMLElement *element, void **buf, uint32_t *size, uint32_t *offset, xml_parse_item_t *parse_item)
{
    uint32_t *nums = NULL;
    int32_t i = 0;
    int32_t count = 0;

    if ((element == NULL) || (parse_item == NULL)) {
        return -1;
    }

    const char *name = element->Name();
    if (name == NULL) {
        return -1;
    }
    uint32_t name_len = strlen(name);
    uint32_t name_len_pack = ((name_len >> 2) + 1) << 2;
    if (name_len == 0) {
        return -1;
    }

    count = ARRAY_SIZE(m_algo_param_item);
    if (count == 0) {
        return -1;
    }

    nums = (uint32_t *)malloc(sizeof(uint32_t) * count);
    if (nums == NULL) {
        return -1;
    }
    memset(nums, 0, sizeof(uint32_t) * count);

    uint32_t data_offset = 0;
    uint32_t data_size = _xml_get_param_item_data_info(element, nums, count);
    if (data_size == 0) {
        free(nums);
        return -1;
    }
    data_size += sizeof(uint32_t) + name_len_pack;

    char *data = (char *)malloc(data_size);
    if (data == NULL) {
        free(nums);
        return -1;
    }

    memset(data, 0, data_size);
    memcpy(data + data_offset, &name_len_pack, sizeof(uint32_t));
    data_offset += sizeof(uint32_t);
    memcpy(data + data_offset, name, name_len);
    data_offset += name_len_pack;

    for (i = 0; i < count; i++) {
        const XMLElement *sub_element = element->FirstChildElement(m_algo_param_item[i].item);
        if (sub_element != NULL) {
            _xml_get_param_item_elem(sub_element, data, data_size, &data_offset, m_algo_param_item[i].id, nums[i]);
        }
    }

    if (data_offset > name_len_pack + sizeof(uint32_t)) {
        _xml_upd_item_array(buf, size, offset, parse_item->id, data, data_offset);
        LOG_MSG_VERBOSE("update ArrParam: (%s%s%s%s%s%s%s) %s (0x%08x) size(%d)",
                        (parse_item->parent_name == NULL) ? "" : parse_item->parent_name,
                        (parse_item->sub_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->sub_parent_name == NULL) ? "" : parse_item->sub_parent_name,
                        (parse_item->third_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->third_parent_name == NULL) ? "" : parse_item->third_parent_name,
                        (parse_item->item == NULL) ? "" : XML_PATH_SEPRATE,
                        (parse_item->item == NULL) ? "" : parse_item->item,
                        name, parse_item->id, data_offset);
    }

    free(data);
    free(nums);

    return 0;
}

static int32_t _xml_get_param_array(const XMLElement *element, void **buf, uint32_t *size, uint32_t *offset, xml_parse_item_t *parse_item, uint32_t id)
{
    const XMLElement *sub_element = NULL;

    if ((element == NULL) || (parse_item == NULL)) {
        return -1;
    }

    sub_element = element->FirstChildElement();
    while (sub_element != NULL) {
        _xml_upd_append_list(buf, size, offset, id, 0);
        if (_xml_upd_mem_setup(buf, size, offset) < 0) {
            return -1;
        }
        _xml_get_param_item(sub_element, buf, size, offset, parse_item);
        sub_element = sub_element->NextSiblingElement();
    }

    return 0;
}

static int32_t _xml_param_check_dev_ver(const XMLElement *rootElement, void **buf, uint32_t *size, uint32_t *offset, uint32_t cid, uint32_t sid, uint32_t vid, int32_t dump)
{
    int32_t ret = -1;
    uint32_t id[3] = {0};
    uint32_t id_mask[3] = {0};
    uint32_t count = 0;

    uint32_t dev_ver_size = 0;
    dev_ver_t *p_dev_ver = NULL;
    uint32_t dev_ver_count = 0;
    void *pdev_ver = NULL;

    const char *VER_LIST_DELIM = " ";
    const char *VER_ID_DELIM = "_";

    memset(id, 0, sizeof(id));
    memset(id_mask, 0, sizeof(id_mask));

    if (rootElement == NULL || strcmp("device", rootElement->Name())) {
        return -1;
    }
    if (m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].item == NULL) {
        return -1;
    }

    const XMLAttribute *verAttribute = rootElement->FindAttribute(m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].item);
    if (verAttribute == NULL) {
        return -1;
    }

    char *sVerList = (char *)verAttribute->Value();
    if (sVerList == NULL) {
        return -1;
    }

    // get id mask
    const XMLElement *element = rootElement;
    if (m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].parent_name != NULL) {
        element = element->FirstChildElement(m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].parent_name);
        if (element == NULL) {
            return -1;
        }
    }
    if (m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].sub_parent_name != NULL) {
        element = element->FirstChildElement(m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].sub_parent_name);
        if (element == NULL) {
            return -1;
        }
    }
    if (m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].item != NULL) {
        element = element->FirstChildElement(m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].item);
        if (element == NULL) {
            return -1;
        }
    }
    char *sVerMask = (char *)element->GetText();
    if (sVerMask == NULL) {
        return -1;
    }
    count = _xml_str_to_uint32_array(sVerMask, VER_ID_DELIM, id_mask, ARRAY_SIZE(id_mask));
    if (count != ARRAY_SIZE(id_mask)) {
        LOG_MSG_VERBOSE("id mask invalid, %d but %d", ARRAY_SIZE(id_mask), count);
        return -1;
    }
    if (dump) {
        _xml_upd_item_array(buf, size, offset, m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].id, id_mask, sizeof(id_mask));
        LOG_MSG_VERBOSE("update mask: (%s%s%s%s%s) (0x%08x) %08x:%08x:%08x",
                        (m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].parent_name == NULL) ? "" : m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].parent_name,
                        (m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].sub_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                        (m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].sub_parent_name == NULL) ? "" : m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].sub_parent_name,
                        (m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].item == NULL) ? "" : XML_PATH_SEPRATE,
                        (m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].item == NULL) ? "" : m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].item,
                        m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].id, id_mask[0], id_mask[1], id_mask[2]);
    }

    // get id value and check
    char *saveptr = NULL;
    char *sVerValue = strtok_r(sVerList, VER_LIST_DELIM, &saveptr);
    while (sVerValue != NULL) {
        count = _xml_str_to_uint32_array(sVerValue, VER_ID_DELIM, id, ARRAY_SIZE(id));
        if (count == ARRAY_SIZE(id_mask)) {
            if (dump) {
                if (dev_ver_count + 1 > dev_ver_size) {
                    pdev_ver = p_dev_ver;
                    p_dev_ver = (dev_ver_t *)realloc(p_dev_ver, sizeof(dev_ver_t) * (dev_ver_size + 4));
                    if (p_dev_ver != NULL) {
                        dev_ver_size += 4;
                    } else { /* realloc fail */
                        dev_ver_size = 0;
                        p_dev_ver = 0;
                        free(pdev_ver);
                    }
                }
                if (p_dev_ver != NULL) {
                    p_dev_ver[dev_ver_count].id = id[0];
                    p_dev_ver[dev_ver_count].sid = id[1];
                    p_dev_ver[dev_ver_count].vid = id[2];
                    dev_ver_count++;
                }
                ret = 0;
            } else {
                if (UINT_MATCH(cid, id[0], id_mask[0]) && UINT_MATCH(sid, id[1], id_mask[1]) && UINT_MATCH(vid, id[2], id_mask[2])) {
                    ret = 0;
                    break;
                }
            }
        }
        sVerValue = strtok_r(NULL, VER_LIST_DELIM, &saveptr);
    }

    if (dump) {
        _xml_upd_item_array(buf, size, offset, m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].id, p_dev_ver, dev_ver_count * sizeof(dev_ver_t));
        LOG_MSG_VERBOSE("update dev_ver: (%s%s%s%s%s) (0x%08x) %d",
                        (m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].parent_name == NULL) ? "" : m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].parent_name,
                        (m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].sub_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                        (m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].sub_parent_name == NULL) ? "" : m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].sub_parent_name,
                        (m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].item == NULL) ? "" : XML_PATH_SEPRATE,
                        (m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].item == NULL) ? "" : m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].item,
                        m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].id, dev_ver_count);

        if (p_dev_ver != NULL) {
            free(p_dev_ver);
        }
    }

    return ret;
}

static int32_t _xml_param_parse_get(const XMLElement *element, void **buf, uint32_t *size, uint32_t *offset, xml_parse_item_t *parse_item)
{
    const XMLAttribute *type_attribute = NULL;
    char *data_type = NULL;

    if (element == NULL) {
        return -1;
    }

    type_attribute = element->FindAttribute("st");
    if (type_attribute != NULL) {
        data_type = (char *)type_attribute->Value();
    }

    if (data_type != NULL) {
        if (!strcmp("ArrI", data_type)) {
            _xml_get_int32_array(element, buf, size, offset, parse_item);
        } else if (!strcmp("ArrUU", data_type)) {
            _xml_get_uu_array(element, buf, size, offset, parse_item);
        } else {
            data_type = NULL;
        }
    }

    if (data_type == NULL) {
        _xml_get_int32_item(element, buf, size, offset, parse_item);
    }

    return 0;
}

static int32_t _xml_param_parse(const XMLElement *rootElement, void **buf, uint32_t *size, uint32_t *offset)
{
    const XMLElement *parent_element = NULL;
    const XMLElement *sub_parent_element = NULL;
    const XMLElement *third_parent_element = NULL;
    const XMLElement *element = NULL;

    uint32_t i = 0;
    int8_t sub_seach = 1;

    for (i = PARAM_UPD_ITEM_INDEX_START; i < m_param_count; i++) {
        if (m_param_parse_item[i].item == NULL) {
            continue;
        }

        sub_seach = 1;
        XML_GET_ELEM(m_param_parse_item[i], parent_name, rootElement, parent_element);
        XML_GET_ELEM(m_param_parse_item[i], sub_parent_name, parent_element, sub_parent_element);
        XML_GET_ELEM(m_param_parse_item[i], third_parent_name, sub_parent_element, third_parent_element);
        if (sub_seach) {
            element = third_parent_element->FirstChildElement(m_param_parse_item[i].item);
        }

        if (element != NULL) {
            _xml_param_parse_get(element, buf, size, offset, &m_param_parse_item[i]);
        }
    }

    return 0;
}

static int32_t _xml_param_get_algo_type(const XMLElement *rootElement, void **buf, uint32_t *size, uint32_t *offset)
{
    uint32_t algo_type = 0;
    uint32_t value32 = 0;
    const XMLElement *element = rootElement;

    if (element == NULL) {
        return -1;
    }

    element = element->FirstChildElement("SysParam");
    if (element == NULL) {
        return -1;
    }

    element = element->FirstChildElement("SLAlg");
    if (element == NULL) {
        return -1;
    }

    element = element->FirstChildElement("miscellaneous_ctl");
    if (element == NULL) {
        return -1;
    }

    const char *svalue = element->GetText();
    if (svalue == NULL) {
        return -1;
    }

    if ((strlen(svalue) > 2) && (svalue[0] == '0') && (svalue[1] == 'x')) { // 16bit data
        value32 = _xml_str_to_uint32(svalue, NULL, 16);
        algo_type = (value32 & 0x10000000) ? 1 : 0;
    } else {
        value32 = _xml_str_to_uint32(svalue, NULL, 10);
        algo_type = (value32 & 0x10000000) ? 1 : 0;
    }

    _xml_upd_item_array(buf, size, offset, CFG_UPD_ID_ALGO_TYPE, &algo_type, sizeof(algo_type));
    LOG_MSG_VERBOSE("update algo type: 0x%x (0x%x)", algo_type, value32);

    return 0;
}

static int32_t _xml_param_get_from_xml(const char *dir, const char *module, void **buf, uint32_t *size, uint32_t *offset, uint32_t cid, uint32_t sid, uint32_t vid, int32_t dump)
{
    XMLDocument doc;
    char path[PATH_MAX] = {0};

    if (dir == NULL) {
        return -1;
    }

    if (module != NULL) {
        sprintf(path, "%s/%s/%s", dir, module, FP_SYSPARAM_PARAM_FILE_NAME);
    } else {
        sprintf(path, "%s/%s", dir, FP_SYSPARAM_PARAM_FILE_NAME);
    }
    LOG_MSG_VERBOSE("path = %s", path);

    doc.LoadFile(path);

    const XMLElement* rootElement = doc.RootElement();
    if (rootElement == NULL || strcmp("device", rootElement->Name())) {
        return -1;
    }

    if (_xml_param_check_dev_ver(rootElement, buf, size, offset, cid, sid, vid, dump) < 0) {
        return -1;
    }

    LOG_MSG_DEBUG("use param file: %s", path);

    _xml_param_get_algo_type(rootElement, buf, size, offset);
    _xml_param_parse(rootElement, buf, size, offset);

    return 0;
}

static int32_t _xml_config_parse(const XMLElement *rootElement, void **buf, uint32_t *size, uint32_t *offset)
{
    const XMLElement *parent_element = NULL;
    const XMLElement *sub_parent_element = NULL;
    const XMLElement *third_parent_element = NULL;
    const XMLElement *element = NULL;

    uint32_t i = 0;
    int8_t sub_seach = 1;

    for (i = 0; i < m_config_count; i++) {
        if (m_config_parse_item[i].item == NULL) {
            continue;
        }

        sub_seach = 1;
        XML_GET_ELEM(m_config_parse_item[i], parent_name, rootElement, parent_element);
        XML_GET_ELEM(m_config_parse_item[i], sub_parent_name, parent_element, sub_parent_element);
        XML_GET_ELEM(m_config_parse_item[i], third_parent_name, sub_parent_element, third_parent_element);
        if (sub_seach) {
            element = third_parent_element->FirstChildElement(m_config_parse_item[i].item);
        }

        if (element != NULL) {
            _xml_get_int32_item(element, buf, size, offset, &m_config_parse_item[i]);
        }
    }

    return 0;
}

static int32_t _xml_config_get_from_xml(const char *dir, const char *module, void **buf, uint32_t *size, uint32_t *offset)
{
    XMLDocument doc;
    char path[PATH_MAX] = {0};

    if (dir == NULL) {
        return -1;
    }

    if (module != NULL) {
        sprintf(path, "%s/%s/%s", dir, module, FP_SYSPARAM_CONFIG_FILE_NAME);
    } else {
        sprintf(path, "%s/%s", dir, FP_SYSPARAM_CONFIG_FILE_NAME);
    }
    LOG_MSG_VERBOSE("path = %s", path);

    doc.LoadFile(path);

    const XMLElement* rootElement = doc.RootElement();
    if (rootElement == NULL || strcmp("device", rootElement->Name())) {
        return -1;
    }

    LOG_MSG_DEBUG("use config file: %s", path);
    _xml_config_parse(rootElement, buf, size, offset);

    return 0;
}

static int32_t _xml_param_and_config_get_from_xml(const char *dir, const char *module, uint32_t cid, uint32_t sid, uint32_t vid, int32_t dump)
{
    int32_t ret = -1;
    void *pbuf = NULL;
    uint32_t size = 0;
    uint32_t offset = 0;

    if (dir == NULL || module == NULL) {
        LOG_MSG_ERROR("path param invalid");
        return -1;
    }

    ret = _xml_upd_mem_setup(&pbuf, &size, &offset);
    if (ret < 0) {
        return -1;
    }

    ret = _xml_param_get_from_xml(dir, module, &pbuf, &size, &offset, cid, sid, vid, dump);
    if (ret >= 0) {
        _xml_config_get_from_xml(dir, module, &pbuf, &size, &offset);
    }

    if (ret >= 0) {
        _xml_upd_append_list(&pbuf, &size, &offset, CFG_UPD_TYPE_NORMAL, 1);
    }

    _xml_upd_mem_unsetup(&pbuf, &size, &offset, 1);

    return ret;
}

static int32_t _xml_algo_parse_get(const XMLElement *element, void **buf, uint32_t *size, uint32_t *offset, xml_parse_item_t *parse_item)
{
    const XMLAttribute *type_attribute = NULL;
    char *data_type = NULL;

    if (element == NULL) {
        return -1;
    }

    type_attribute = element->FindAttribute("st");
    if (type_attribute != NULL) {
        data_type = (char *)type_attribute->Value();
    }

    if (data_type != NULL) {
        if (!strcmp("ArrI", data_type)) {
            _xml_get_int32_array(element, buf, size, offset, parse_item);
        } else if (!strcmp("ArrF", data_type)) {
            _xml_get_float_array(element, buf, size, offset, parse_item);
        } else if (!strcmp("ArrD", data_type)) {
            _xml_get_double_array(element, buf, size, offset, parse_item);
        } else if (!strcmp("ArrTI", data_type)) {
            _xml_get_tag_array(element, buf, size, offset, parse_item);
        } else if (!strcmp("ArrParam", data_type)) {
            _xml_get_param_array(element, buf, size, offset, parse_item, CFG_UPD_TYPE_ALGO);
        } else {
            data_type = NULL;
        }
    }

    if (data_type == NULL) {
        _xml_get_int32_item(element, buf, size, offset, parse_item);
    }

    return 0;
}

static int32_t _xml_algo_parse(const XMLElement *rootElement)
{
    const XMLElement *parent_element = NULL;
    const XMLElement *sub_parent_element = NULL;
    const XMLElement *third_parent_element = NULL;
    const XMLElement *element = NULL;
    void *pbuf = NULL;
    uint32_t size = 0;
    uint32_t offset = 0;

    uint32_t i = 0;
    int8_t sub_seach = 1;

    for (i = 0; i < m_algo_count; i++) {
        if (m_algo_parse_item[i].item == NULL) {
            continue;
        }

        _xml_upd_append_list(&pbuf, &size, &offset, CFG_UPD_TYPE_ALGO, 0);
        if (_xml_upd_mem_setup(&pbuf, &size, &offset) < 0) {
            return -1;
        }

        sub_seach = 1;
        XML_GET_ELEM(m_algo_parse_item[i], parent_name, rootElement, parent_element);
        XML_GET_ELEM(m_algo_parse_item[i], sub_parent_name, parent_element, sub_parent_element);
        XML_GET_ELEM(m_algo_parse_item[i], third_parent_name, sub_parent_element, third_parent_element);
        if (sub_seach) {
            element = third_parent_element->FirstChildElement(m_algo_parse_item[i].item);
        }

        if (element != NULL) {
            _xml_algo_parse_get(element, &pbuf, &size, &offset, &m_algo_parse_item[i]);
        }
    }

    _xml_upd_append_list(&pbuf, &size, &offset, CFG_UPD_TYPE_ALGO, 1);

    return 0;
}

static int32_t _xml_algo_get_from_xml(const char *dir, const char *module)
{
    XMLDocument doc;
    char path[PATH_MAX] = {0};

    if (dir == NULL) {
        return -1;
    }

    if (module != NULL) {
        sprintf(path, "%s/%s/%s", dir, module, FP_SYSPARAM_ALGO_FILE_NAME);
    } else {
        sprintf(path, "%s/%s", dir, FP_SYSPARAM_ALGO_FILE_NAME);
    }
    LOG_MSG_VERBOSE("path = %s", path);

    doc.LoadFile(path);

    const XMLElement* rootElement = doc.RootElement();
    if (rootElement == NULL || strcmp("device", rootElement->Name())) {
        return -1;
    }

    LOG_MSG_DEBUG("use algo file: %s", path);
    _xml_algo_parse(rootElement);

    return 0;
}

static int32_t _xml_parase(const char *dir, const char *module, uint32_t cid, uint32_t sid, uint32_t vid, int32_t dump)
{
    int32_t ret = -1;

    if (dir == NULL || module == NULL) {
        LOG_MSG_ERROR("path param invalid");
        return ret;
    }

    ret = _xml_param_and_config_get_from_xml(dir, module, cid, sid, vid, dump);
    if (ret >= 0) {
        _xml_algo_get_from_xml(dir, module);
    }

    return ret;
}

extern "C" int32_t silfp_ree_xml_get_sysparams(uint32_t cid, uint32_t sid, uint32_t vid)
{
    int32_t ret = -1;
    DIR *pDir = NULL;
    struct dirent *pEntry = NULL;
    const char *dirs[] = {FP_SYSPARAM_PATH1, FP_SYSPARAM_PATH2};
    const char *path = NULL;
    uint32_t i = 0;

    for (i = 0; i <= ARRAY_SIZE(dirs); i++) {
        if (i == 0) {
            path = _xml_get_path();
        } else {
            path = dirs[i - 1];
        }
        if (path == NULL) {
            continue;
        }

        pDir = opendir(path);
        if (pDir == NULL) {
            continue;
        }

        while((pEntry = readdir(pDir)) != NULL) {
            if (strcmp(pEntry->d_name, ".") == 0 || strcmp(pEntry->d_name, "..") == 0) {
                continue;
            } else if (silfp_ree_util_dir_get_type((char *)path, pEntry) == 4) { // dir
                ret = _xml_parase(path, pEntry->d_name, cid, sid, vid, 0);
                if (ret >= 0) {
                    break;
                }
            }
        }

        closedir(pDir);
        if (ret >= 0) {
            break;
        }
    }

    return ret;
}

extern "C" int32_t silfp_ree_xml_dump(const char *dir, const char *module)
{
    return _xml_parase(dir, module, 0, 0, 0, 1);
}