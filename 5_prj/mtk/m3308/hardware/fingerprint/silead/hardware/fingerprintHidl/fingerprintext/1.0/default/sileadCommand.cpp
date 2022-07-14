#define LOG_TAG "vendor.silead.hardware.fingerprintext@1.0-service"

#include "sileadCommand.h"

#include <cutils/sockets.h>
#include <cutils/record_stream.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_COMMAND_BYTES (150 * 1024)
#define SOCKET_NAME_DEFAULT "com_silead_fpext"
#define SOCKET_OPEN_RETRY_DELAY (4*1000*1000)

#define TEST_RESULT_SERVICE_FAILED (-1)

static pthread_t m_tid_receiver = -1;
static pthread_mutex_t m_write_mutex;
static int32_t m_command_fd = -1;
static RecordStream *p_rs = NULL;

static silead_notify_t m_command_notify = NULL;

static void _command_process_response(const void *result, uint32_t len)
{
    uint32_t cmd = -1;
    const uint8_t *buf = (const uint8_t *)result;

    if (len <= 4) {
        return;
    }

    cmd = ((buf[0] & 0xff) << 24) | ((buf[1] & 0xff) << 16) | ((buf[2] & 0xff) << 8) | (buf[3] & 0xff);

    ALOGI("rsp: cmd=0x%u, notify=%s", cmd, (m_command_notify == NULL) ? "NULL":"OK");
    if (m_command_notify != NULL) {
        m_command_notify(cmd, buf + 4, len - 4);
    }
}

static void *_command_receiver(void __unused *param)
{
    int32_t ret = 0;
    void *p_record = NULL;
    size_t recordlen = 0;

    const char *socket_name = SOCKET_NAME_DEFAULT;
    int32_t retry_count = 0;

    for(;;) {
        m_command_fd = socket_local_client(socket_name, ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
        if (m_command_fd < 0) {
            if (retry_count == 8) {
                ALOGE("Couldn't find '%s' socket after %d times, continuing to retry silently", socket_name, retry_count);
            } else if (retry_count >= 0 && retry_count < 8) {
                ALOGI("Couldn't find '%s' socket; retrying after timeout", socket_name);
            }

            usleep(SOCKET_OPEN_RETRY_DELAY);
            retry_count++;

            continue;
        }

        retry_count = 0;

        if (p_rs != NULL) {
            record_stream_free(p_rs);
            p_rs = NULL;
        }
        if (p_rs == NULL) {
            p_rs = record_stream_new(m_command_fd, MAX_COMMAND_BYTES);
        }

        for (;;) {
            ret = record_stream_get_next(p_rs, &p_record, &recordlen);

            if (ret == 0 && p_record == NULL) {
                break;
            } else if (ret < 0 && errno != EAGAIN) {
                break;
            } else if (ret == 0) {
                _command_process_response(p_record, recordlen);
            }
        }

        ALOGI("Disconnected from %s socket", socket_name);
        ALOGI("ret = %d, errno = %d", ret, errno);

        if (m_command_fd >= 0) {
            close(m_command_fd);
            m_command_fd = -1;
        }

        if (p_rs != NULL) {
            record_stream_free(p_rs);
            p_rs = NULL;
        }
    }
    return NULL;
}

int32_t silext_command_init(void)
{
    int32_t ret = 0;

    pthread_mutex_init(&m_write_mutex, NULL);
    ret = pthread_create(&m_tid_receiver, NULL, _command_receiver, NULL);
    if (ret < 0) {
        ALOGE("Failed to create receiver thread errno:%d", errno);
    }

    return ret;
}

int32_t silext_command_deinit(void)
{
    if (m_command_fd >= 0) {
        close(m_command_fd);
        m_command_fd = -1;
    }

    if (p_rs != NULL) {
        record_stream_free(p_rs);
        p_rs = NULL;
    }
    pthread_mutex_destroy(&m_write_mutex);
    return 0;
}

int32_t set_notify(silead_notify_t notify)
{
    m_command_notify = notify;
    return 0;
}

static int32_t _command_data_blocking_write(int32_t fd, const void *buffer, size_t len)
{
    size_t offset = 0;
    const uint8_t *wbuffer;

    wbuffer = (const uint8_t *)buffer;

    while (offset < len) {
        ssize_t written = 0;
        do {
            written = write(fd, wbuffer + offset, len - offset);
        } while (written < 0 && ((errno == EINTR) || (errno == EAGAIN)));

        if (written >= 0) {
            offset += written;
        } else {
            // written < 0
            ALOGE("unexpected error on write errno:%d", errno);
            close(fd);
            return TEST_RESULT_SERVICE_FAILED;
        }
    }

    return 0;
}

static int32_t _command_send_request_raw(uint32_t cmdid, const void *data, size_t data_size)
{
    int32_t ret = 0;
    uint32_t header = 0;
    char p[4] = {0};

    int32_t fd = m_command_fd;
    if (fd < 0) {
        ALOGE("socket is invalid");
        return TEST_RESULT_SERVICE_FAILED;
    }

    if (data_size + sizeof(uint32_t) > MAX_COMMAND_BYTES) {
        ALOGE("packet larger than %u (%u)", MAX_COMMAND_BYTES, (unsigned int )data_size);
        return TEST_RESULT_SERVICE_FAILED;
    }

    ALOGI("request %u", cmdid);

    pthread_mutex_lock(&m_write_mutex);

    p[0] = (cmdid >> 24) & 0xFF;
    p[1] = (cmdid >> 16) & 0xFF;
    p[2] = (cmdid >> 8) & 0xFF;
    p[3] = cmdid & 0xFF;

    header = htonl(data_size + sizeof(uint32_t));
    ret = _command_data_blocking_write(fd, (void *)&header, sizeof(header));
    if (ret >= 0) {
        ret = _command_data_blocking_write(fd, p, sizeof(uint32_t));
        ret |= _command_data_blocking_write(fd, data, data_size);
    }

    pthread_mutex_unlock(&m_write_mutex);

    return ret;
}

int32_t silext_command_request(uint32_t cmdId, const uint8_t *buf, uint32_t size)
{
    return _command_send_request_raw(cmdId, buf, size);
}