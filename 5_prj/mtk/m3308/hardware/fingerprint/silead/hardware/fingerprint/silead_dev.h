/******************************************************************************
 * @file   silead_dev.h
 * @brief  Contains /dev/silead_fp operate functions header file.
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
 * Jack Zhang  2018/4/2   0.1.0      Init version
 * David Wang  2018/6/5   0.1.1      Support wakelock & pwdn
 * Ashely Li   2021/1/15  0.2.0      Update copyright notice
 *
 *****************************************************************************/

#ifndef __SILEAD_DEV_H__
#define __SILEAD_DEV_H__

#include <sys/ioctl.h>

#include "silead_screen.h"

typedef enum _pwdn_mode {
    SIFP_PWDN_NONE = 0,
    SIFP_PWDN_POWEROFF = 1, /* shutdown the avdd power supply */
    SIFP_PWDN_FLASH = 2, /* shutdown avdd 200ms for H/W full reset */
    SIFP_PWDN_MAX,
} _pwdn_mode_t;

typedef struct _dev_key {
    int32_t key;
    uint32_t value;   /* key down = 1, key up = 0 */
} fp_dev_key_t;

#define DEVNAME_LEN 16
typedef struct _dev_conf {
    uint8_t mode;
    uint8_t bits;
    uint16_t delay;
    uint32_t speed;
    char dev[DEVNAME_LEN];
    uint8_t nl_id;
    uint8_t dev_id;
    uint16_t reserve;
    uint32_t reg;
    char ta[DEVNAME_LEN];
} fp_dev_conf_t;

#define PROC_VND_ID_LEN    32
#define OTP_DATA_LEN       64


#define SIFP_IOC_MAGIC 's' //'k'

#define SIFP_IOC_RESET         _IOW(SIFP_IOC_MAGIC, 10, uint8_t)
#define SIFP_IOC_ENABLE_IRQ    _IO(SIFP_IOC_MAGIC,  11)
#define SIFP_IOC_DISABLE_IRQ   _IO(SIFP_IOC_MAGIC,  12)
#define SIFP_IOC_WAIT_IRQ      _IOR(SIFP_IOC_MAGIC, 13, uint8_t)
#define SIFP_IOC_CLR_IRQ       _IO(SIFP_IOC_MAGIC,  14)
#define SIFP_IOC_KEY_EVENT     _IOW(SIFP_IOC_MAGIC, 15, fp_dev_key_t)
#define SIFP_IOC_INIT          _IOR(SIFP_IOC_MAGIC, 16, fp_dev_conf_t)
#define SIFP_IOC_DEINIT        _IO(SIFP_IOC_MAGIC,  17)
#define SIFP_IOC_IRQ_STATUS    _IOR(SIFP_IOC_MAGIC, 18, uint8_t)
#define SIFP_IOC_SCR_STATUS    _IOR(SIFP_IOC_MAGIC, 20, uint8_t)
#define SIFP_IOC_GET_VER       _IOR(SIFP_IOC_MAGIC, 21, char[10])
#define SIFP_IOC_SET_KMAP      _IOW(SIFP_IOC_MAGIC, 22, uint16_t[7])
#define SIFP_IOC_ACQ_SPI       _IO(SIFP_IOC_MAGIC,  23)
#define SIFP_IOC_RLS_SPI       _IO(SIFP_IOC_MAGIC,  24)
#define SIFP_IOC_PKG_SIZE      _IOR(SIFP_IOC_MAGIC, 25, uint8_t)     /* support from v0.0.5 */
#define SIFP_IOC_DBG_LEVEL     _IOWR(SIFP_IOC_MAGIC,26, uint8_t)     /* support from v0.0.6 */
#define SIFP_IOC_WAKELOCK      _IOW(SIFP_IOC_MAGIC, 27, uint8_t)
#define SIFP_IOC_PWDN          _IOW(SIFP_IOC_MAGIC, 28, uint8_t)
#define SIFP_IOC_PROC_NODE     _IOW(SIFP_IOC_MAGIC, 29, char[PROC_VND_ID_LEN])
#define SIFP_IOC_SET_FEATURE   _IOW(SIFP_IOC_MAGIC, 30, uint8_t)
#define SIFP_IOC_OTP_NODE     _IOW(SIFP_IOC_MAGIC, 31, char[OTP_DATA_LEN])



#define SIL_STUB_IOC_PRINT     _IO(SIFP_IOC_MAGIC, 1)
#define SIL_STUB_IOC_INIT      _IO(SIFP_IOC_MAGIC, 2)
#define SIL_STUB_IOC_DEINIT    _IO(SIFP_IOC_MAGIC, 3)
#define SIL_STUB_IOC_GETDATA   _IOR(SIFP_IOC_MAGIC, 4, int32_t)
#define SIL_STUB_IOC_SETDATA   _IOW(SIFP_IOC_MAGIC, 5, int32_t)

#define FEATURE_FLASH_CS  0x01

int32_t silfp_dev_get_ver(char *ver, uint32_t len);
int32_t silfp_dev_enable(void);
int32_t silfp_dev_disable(void);
int32_t silfp_dev_init(fp_dev_conf_t *t);
int32_t silfp_dev_deinit(void);
int32_t silfp_dev_hw_reset(uint8_t delayms);

int32_t silfp_dev_get_screen_status(uint8_t *status);
int32_t silfp_dev_set_screen_cb(screen_cb listen, void *param);
int32_t silfp_dev_set_finger_status_mode(int32_t mode);

int32_t silfp_dev_wait_finger_status(int32_t irq, int32_t down, int32_t up, int32_t cancel, int32_t timeout);
void silfp_dev_wait_clean(void);
void silfp_dev_cancel(void);
void silfp_dev_sync_finger_down(void);
void silfp_dev_sync_finger_up(void);
int32_t silfp_dev_is_finger_down(void);
void silfp_dev_reset_finger_status(void);

int32_t silfp_dev_send_key(uint32_t key);
int32_t silfp_dev_send_key_state(uint32_t key, int down);
int32_t silfp_dev_set_log_level(uint8_t lvl);
int32_t silfp_dev_set_feature(uint8_t feature);

int32_t silfp_dev_wakelock(uint8_t lock);
int32_t silfp_dev_pwdn(uint8_t avdd_down);
int32_t silfp_dev_create_proc_node(const char *chipname);
void silfp_dev_set_otp(uint32_t *otp);

int32_t silfp_dev_clear_irq(void);


#endif /* __SILEAD_DEV_H__ */
