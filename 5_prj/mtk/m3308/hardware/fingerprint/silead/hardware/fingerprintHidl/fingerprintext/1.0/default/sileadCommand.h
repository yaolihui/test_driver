#ifndef __SILEAD_COMMOND_H__
#define __SILEAD_COMMOND_H__

#include <log/log.h>
#include <android/log.h>

typedef void (*silead_notify_t)(uint32_t cmd_id, const uint8_t *result, uint32_t len);

int32_t silext_command_init(void);
int32_t silext_command_deinit(void);
int32_t set_notify(silead_notify_t notify);

int32_t silext_command_request(uint32_t cmdId, const uint8_t *buf, uint32_t size);

#endif // __SILEAD_COMMOND_H__
