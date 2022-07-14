/*
 * Copyright (c) 2016-2021 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <dlfcn.h>

#include "fpc_log.h"
#include "fpc_tee_sensor.h"
#include "fpc_tee.h"
#include "fpc_tee_sensortest.h"
#include "fpc_tee_bio.h"
#include "fpc_tee_kpi.h"
#include "fpc_types.h"
#include "fpc_error_str.h"
#include "fpc_tee_engineering.h"
#include "fpc_tee_hw_auth.h"
#include "fpc_tee_db_blob_test.h"

#define DEBUG_PATH_BASE     "/data/vendor_de/0/fpdata"
#define DEBUG_PATH_ENROL    DEBUG_PATH_BASE"/enroll/"
#define DEBUG_PATH_VERIFY   DEBUG_PATH_BASE"/verify/"
#define DEBUG_TEST_DB       DEBUG_PATH_BASE"/test.db"
#define SENSORTEST_STABILIZATION_MS 500
#define NO_MORE_IMAGE_TO_INJECT     6

typedef int (*TRUSTY_FACTORY_FUNC)(void);
static int trusty_test(void)
{
    void *handle = NULL;
    int ret = 0;
    TRUSTY_FACTORY_FUNC p_func = NULL;
    char s_func[6][16] = {"factory_init", "spi_test", "deadpixel_test", "finger_detect", "interrupt_test", "factory_exit"};
    handle = dlopen("/vendor/lib64/hw/fingerprint.default.so", RTLD_LAZY);
    if (handle == NULL) {
        LOGE("%s dlopen err", __func__);
        return -1;
    }
    for (int i = 0; i < 6; i++) {
        p_func = (TRUSTY_FACTORY_FUNC)dlsym(handle, s_func[i]);
        if (p_func) {
            ret = p_func();
            p_func = NULL;
            LOGE("%s, %s ret = %d", __func__, s_func[i], ret);
        } else {
            LOGE("%s dlsym err %s", __func__, s_func[i]);
        }
    }
    dlclose(handle);
    return 0;
}

static int sensor_test(fpc_tee_sensor_t *sensor, uint32_t *result)
{
    fpc_ta_sensortest_test_params_t params;
    memset(&params, 0, sizeof(params));
    uint32_t image_captured;
    uint32_t log_size;
    int status = fpc_tee_sensortest_run_test(sensor,
                                             FPC_SENSORTEST_SELF_TEST,
                                             0,
                                             &params,
                                             &image_captured,
                                             result,
                                             &log_size);
    printf("%s: %d\n", __func__, *result);
    return status;
}

static int checkerboard_test(fpc_tee_sensor_t *sensor, uint32_t *result)
{
    fpc_ta_sensortest_test_params_t params;
    memset(&params, 0, sizeof(params));
    uint32_t image_captured;
    uint32_t log_size;
    int status = fpc_tee_sensortest_run_test(sensor,
                                             FPC_SENSORTEST_CHECKERBOARD_TEST,
                                             0,
                                             &params,
                                             &image_captured,
                                             result,
                                             &log_size);
    printf("%s: %d\n", __func__, *result);
    return status;
}

static int imagequality_test(fpc_tee_sensor_t *sensor, uint32_t *result)
{
    fpc_ta_sensortest_test_params_t params;
    memset(&params, 0, sizeof(params));
    uint32_t image_captured;
    uint32_t log_size;
    printf("Put rubber stamp against sensor...\n");
    int status = fpc_tee_sensortest_run_test(sensor,
                                             FPC_SENSORTEST_IMAGE_QUALITY_TEST,
                                             SENSORTEST_STABILIZATION_MS,
                                             &params,
                                             &image_captured,
                                             result,
                                             &log_size);
    printf("%s: %d\n", __func__, *result);
    return status;
}

static int resetpixel_test(fpc_tee_sensor_t *sensor, uint32_t *result)
{
    fpc_ta_sensortest_test_params_t params;
    memset(&params, 0, sizeof(params));
    uint32_t image_captured;
    uint32_t log_size;
    int status = fpc_tee_sensortest_run_test(sensor,
                                             FPC_SENSORTEST_RESET_PIXEL_TEST,
                                             0,
                                             &params,
                                             &image_captured,
                                             result,
                                             &log_size);
    printf("%s: %d\n", __func__, *result);
    return status;
}

static int capture_uncalibrated(fpc_tee_sensor_t *sensor)
{
    int status = fpc_tee_sensortest_capture_uncalibrated(sensor);
    printf("%s: %d\n", __func__, status);
    return status;
}

static int auto_inject_image(fpc_tee_t *tee, size_t sensor_size, DIR **directory, char *debugpath)
{
    int ret = 0;
    size_t read_result = 0;
    uint8_t file_buffer[sensor_size];
    FILE *image_file = NULL;
    char filename[100];
    struct dirent *file;
    size_t len = 0;

    if (*directory == NULL) {
        LOGE("%s Error opening directory", __func__);
        return -1;
    }

    while ((file = readdir(*directory)) != NULL) {
        len = strlen(file->d_name);
        if (len >= 4 && memcmp(file->d_name + len - 4, ".raw", 4) == 0) {
            if (strlen(debugpath) > 100 - 1) {
                return -1;
            }
            strcpy(filename, debugpath);
            if (strlen(file->d_name) > 100 - 1 - strlen(filename)) {
                return -1;
            }
            strcat(filename, file->d_name);
            image_file = fopen(filename, "r");
            if (NULL == image_file) {
                return -1;
            }
            read_result = fread(file_buffer, 1, sensor_size, image_file);
            if (read_result != sensor_size) {
                LOGE("%s Entire file not read", __func__);
                fclose(image_file);
                return -1;
            }
            ret = fpc_tee_debug_inject(tee, ENG_RAW_IMAGE, file_buffer, sensor_size);
            fclose(image_file);
            return ret;
        }
    }
    return NO_MORE_IMAGE_TO_INJECT;
}

static int enroll_test(fpc_tee_sensor_t *sensor, fpc_tee_bio_t *bio, fpc_tee_t *tee,
                       int8_t no_sensor)
{
    int status = 0;
    uint32_t remaining_samples = 0;
    uint32_t image_size = 0;
    uint32_t id = 0;
    uint64_t authenticator_id;
    char debugpath[50] = DEBUG_PATH_ENROL;
    DIR *directory;
    directory = opendir(debugpath);
    if (NULL == bio || NULL == tee || NULL == directory) {
        return -1;
    }

    fpc_tee_kpi_start(tee);

    status = fpc_tee_begin_enrol(bio);
    if (status) {
        goto out;
    }

    if (no_sensor) {
        status = fpc_tee_debug_get_retrieve_size(tee, ENG_RAW_IMAGE, &image_size);
        if (status) {
            LOGE("%s: fpc_tee_debug_retrieve: status: %d", __func__, status);
            goto out;
        }
        if (image_size == 0) {
            printf("Error: Cannot get sensor size.\n");
            goto out;
        }
        printf("image size : %u\n", image_size);
    } else {
        printf("Put finger against sensor repeatedly...\n");
    }
    for (;;) {
        if (no_sensor) {
            status = auto_inject_image(tee, image_size, &directory, debugpath);
        } else {
            status = fpc_tee_capture_image(sensor, 1);
            if (status == FPC_STATUS_WAIT_TIME) {
                // This will happen after exceeded maximum attempts
                // of trying to capture an image.
                continue;
            }
        }
        if (status) {
            goto out;
        }

        status = fpc_tee_enrol(bio, &remaining_samples);
        if (status < 0) {
            goto out;
        }

        switch (FPC_ERROR_GET_EXTERNAL_ERROR(status)) {
        case FPC_ERROR_NONE:
            status  = fpc_tee_end_enrol(bio, &id);
            if (status) {
                goto out;
            }

            status = fpc_tee_store_template_db(bio, DEBUG_TEST_DB);

            if (status) {
                printf("Store template failed\n");
                fpc_tee_load_template_db(bio, DEBUG_TEST_DB);
                fpc_tee_load_instance_data(bio);
                goto out;
            }
            status = fpc_tee_get_template_db_id(bio, &authenticator_id);

            if (status) {
                LOGE("%s failed to get auth id %i\n", __func__, status);
                printf("Failed to get auth id %i\n", status);
                authenticator_id = 0;
            }

            status = 0;
            printf("Completed\n");
            goto out;
        case FPC_STATUS_ENROLL_PROGRESS:
            LOGD("%s acquired fingerprint\n", __func__);
            printf("Acquired fingerprint\n");
            break;
        case FPC_STATUS_ENROLL_TOO_SIMILAR:
            LOGE("%s got a too similar fingerprint %i\n", __func__, status);
            printf("Too similar fingerprint\n");
            break;
        case FPC_STATUS_ENROLL_LOW_QUALITY:
            LOGE("%s got image with too low quality %i\n", __func__, status);
            printf("Too low quality\n");
            break;
        case FPC_STATUS_ENROLL_LOW_COVERAGE:
            LOGE("%s got image with too low coverage %i\n", __func__, status);
            printf("Too low coverage\n");
            break;
        }
    }
out:
    if (status) {
        LOGE("%s failed 0x%08x %s\n", __func__, status, fpc_error_str(status));
        switch (FPC_ERROR_GET_EXTERNAL_ERROR(status)){
        case -FPC_ERROR_CANCELLED:
            break;
        case -FPC_ERROR_TIMEDOUT:
            printf("Timed out\n");
            break;
        case -FPC_ERROR_STORAGE:
            printf("Error with storage\n");
            break;
        case -FPC_ERROR_ALREADY_ENROLLED:
            LOGE("%s is already enrolled %i\n", __func__, status);
            printf("Already enrolled\n");
            break;
        case -FPC_ERROR_TOO_MANY_FAILED_ATTEMPTS:
            LOGE("%s Too many failed attempts %i\n", __func__, status);
            printf("Error: Too many failed attempts\n");
            break;
        case NO_MORE_IMAGE_TO_INJECT:
            LOGE("%s enroll failed, need more images for enroll %i\n", __func__, status);
            printf("enroll failed, need more images for enroll\n");
            break;
        default:
            printf("Error HW unavailable\n");
            break;
        }
    }
    closedir(directory);
    fpc_tee_kpi_stop(tee);
    printf("%s: %d\n", __func__, status);
    return status;
}

static int authentication_test(fpc_tee_sensor_t *sensor, fpc_tee_bio_t *bio, fpc_tee_t *tee,
                               int8_t no_sensor)
{
    LOGD("%s", __func__);
    uint32_t image_size = 0;
    int status = 0;
    uint32_t id;
    uint32_t update = 0;
    char debugpath[50] = DEBUG_PATH_VERIFY;
    DIR *directory;
    directory = opendir(debugpath);
    if (NULL == bio || NULL == tee || NULL == directory) {
        return -1;
    }

    fpc_tee_kpi_start_sequence(tee, true);

    if (no_sensor) {
        status = fpc_tee_debug_get_retrieve_size(tee, ENG_RAW_IMAGE, &image_size);
        if (status) {
            LOGE("%s: fpc_tee_debug_retrieve: status: %d", __func__, status);
            goto out;
        }
        if (image_size == 0) {
            printf("Error: Cannot get sensor size.\n");
            goto out;
        }
        printf("image size : %u\n", image_size);
    }

    for (;;) {
        if (no_sensor) {
            status = auto_inject_image(tee, image_size, &directory, debugpath);
        } else {
            printf("Put finger against sensor...\n");
            status = fpc_tee_capture_image(sensor, 1);
            if (status == FPC_STATUS_WAIT_TIME) {
                // This will happen after exceeded maximum attempts
                // of trying to capture an image.
                continue;
           }
        }

        if (status != FPC_ERROR_NONE) {
            goto out;
        }

        LOGD("%s acquired fingerprint, status : %d\n", __func__, status);
        printf("Acquired fingerprint\n");

        status = fpc_tee_identify(bio, &id, NULL);
        if (status) {
            goto out;
        }
        status = fpc_tee_update_template(bio, &update);
        if (status) {
            goto out;
        }
        if (update != 0) {
            fpc_tee_store_template_db(bio, DEBUG_TEST_DB);
        }
        if (id != 0) {
            LOGD("%s authenticated fingerprint\n", __func__);
            printf("Authenticated fingerprint\n");
        } else {
            LOGD("%s did not authenticate fingerprint\n", __func__);
            printf("Did not authenticate fingerprint\n");
        }
    }

out:
    if (status) {
        LOGE("%s failed %s\n", __func__, fpc_error_str(status));
        switch (status) {
        case -FPC_ERROR_CANCELLED:
            break;
        case NO_MORE_IMAGE_TO_INJECT:
            break;
        default:
            printf("Error HW unavailable\n");
            break;
        }
    }
    closedir(directory);
    fpc_tee_kpi_stop_sequence(tee);
    printf("%s: %d\n", __func__, status);
    return status;
}

static int afd_calibration_test(fpc_tee_sensor_t *sensor, uint32_t *result)
{
    fpc_ta_sensortest_test_params_t params;
    memset(&params, 0, sizeof(params));
    uint32_t image_captured;
    uint32_t log_size;
    int status = fpc_tee_sensortest_run_test(sensor,
                                             FPC_SENSORTEST_AFD_CALIBRATION_TEST,
                                             0,
                                             &params,
                                             &image_captured,
                                             result,
                                             &log_size);
    printf("%s: %d\n", __func__, *result);
    return status;
}

#define CTL_LOG  "/data/vendor_de/0/fpdata/ctl_test_log_%04d%02d%02d_%02d%02d%02d_%s.txt"
static int store_capture_image(const char *scenario,  fpc_tee_sensor_t *sensor, uint32_t log_size)
{
    int status = 0;
    char *log;
    FILE *log_file = NULL;
    unsigned long write_result;
    time_t nowtime = time(NULL);
    char log_filename[120];
    if ((int)log_size <= 0) {
        return -1;
    }
    struct tm *now = localtime(&nowtime);
    if (now == NULL) {
        return -1;
    }
    sprintf(log_filename, CTL_LOG, now->tm_year + 1900, now->tm_mon + 1, now->tm_mday,
            now->tm_hour, now->tm_min, now->tm_sec, scenario);
    /* The size of the log includes zero termination, but add one extra
      * byte just to be sure.
      */
    log = calloc(log_size + 1, sizeof(char));
    if (NULL == log) {
        printf("calloc memory failed\n");
        goto err;
    }
    log_file = fopen(log_filename, "w");
    if (NULL == log_file) {
        printf("could not open file\n");
        goto err;
    }
    status = fpc_tee_sensortest_get_log(sensor, &log_size, (uint8_t *)log);
    if (status) {
        printf("%s failed getting log from tee side%d", __func__, status);
        goto err;
    }
    write_result = fwrite(log, 1, log_size, log_file);
    if (write_result != log_size) {
        printf("%s "
               "Entire file not written %lu bytes instead of %u "
               "bytes err: %d",
               __func__, write_result, log_size, ferror(log_file));
    }
err:
    if (NULL != log_file) {
        fclose(log_file);
    }
    free(log);
    return 0;
}

static int defective_pixels_test(fpc_tee_sensor_t *sensor, uint32_t *result)
{
    fpc_ta_sensortest_test_params_t params;
    memset(&params, 0, sizeof(params));
    uint32_t image_captured;
    uint32_t log_size;
    int status = fpc_tee_sensortest_run_test(sensor,
                                             FPC_SENSORTEST_DEFECTIVE_PIXELS_TEST,
                                             0,
                                             &params,
                                             &image_captured,
                                             result,
                                             &log_size);
    printf("%s: %d\n", __func__, *result);
    store_capture_image("defective_pixels_test", sensor, log_size);
    return status;
}

static int module_quality_test(fpc_tee_sensor_t *sensor, uint32_t *result)
{
    int status = 0;
    fpc_module_quality_test_limits_t mqt_limits;
    uint32_t image_fetched;
    fpc_ta_sensortest_test_params_t params;
    uint32_t stabilization_time_ms = 500;
    uint32_t log_size = 0;
    memset(&params, 0, sizeof(params));

    status = fpc_tee_wait_finger_down(sensor);
    if (status) {
        LOGE("%s fpc_tee_wait_finger_down failed %d", __func__, status);
        return -1;
    }
    usleep(stabilization_time_ms * 1000);

    memset(&mqt_limits, 0, sizeof(mqt_limits));
    status = fpc_tee_get_mqt_limits(sensor, &mqt_limits);
    if (!status) {
        params.mqt.snr_limit = mqt_limits.snr_threshold;
        params.mqt.snr_limit_preset = mqt_limits.snr_limit_preset;
        params.mqt.snr_cropping_left = mqt_limits.snr_cropping_left;
        params.mqt.snr_cropping_top = mqt_limits.snr_cropping_top;
        params.mqt.snr_cropping_right = mqt_limits.snr_cropping_right;
        params.mqt.snr_cropping_bottom = mqt_limits.snr_cropping_bottom;
        status = fpc_tee_sensortest_run_test(sensor,
                                             FPC_SENSORTEST_MODULE_QUALITY_TEST,
                                             stabilization_time_ms,
                                             &params,
                                             &image_fetched,
                                             result,
                                             &log_size);
    }
    if (*result == FPC_ERROR_NONE) {
        printf("%s: mqt test passed!\n", __func__);
    } else {
        printf("%s: mqt test fail, result=%d\n", __func__, *result);
    }
    store_capture_image("module_quality_test", sensor, log_size);
    return status;
}

static int otp_validation_test(fpc_tee_sensor_t *sensor, uint32_t *result)
{
    int status = 0;
    fpc_ta_sensortest_test_params_t params;
    memset(&params, 0, sizeof(params));
    uint32_t image_captured;
    uint32_t log_size;
    *result = 1;

    status = fpc_tee_sensortest_run_test(sensor,
                                         FPC_SENSORTEST_OTP_VALIDATION_TEST,
                                         0,
                                         &params,
                                         &image_captured,
                                         result,
                                         &log_size);
    if (*result == FPC_ERROR_NONE) {
        printf("%s: otp validation test passed!\n", __func__);
    } else {
        printf("%s: otp validation test fail, result=%d\n", __func__, *result);
    }
    return status;
}

static int txpulse_checkerboard_test(fpc_tee_sensor_t *sensor, uint32_t *result)
{
    int status = 0;
    int supported = 0;
    fpc_ta_sensortest_test_params_t params;
    memset(&params, 0, sizeof(params));
    uint32_t image_captured;
    uint32_t log_size;
    *result = 1;

    status = fpc_tee_sensortest_is_test_supported(sensor,
                                                  FPC_SENSORTEST_TXPULSE_CHECKERBOARD_TEST,
                                                  &supported);
    if (status || !supported) {
        printf("%s: txpulse checkerboard test is not support with this sensor!\n", __func__);
        goto out;
    }

    status = fpc_tee_sensortest_run_test(sensor,
                                         FPC_SENSORTEST_TXPULSE_CHECKERBOARD_TEST,
                                         0,
                                         &params,
                                         &image_captured,
                                         result,
                                         &log_size);
    if (*result == FPC_ERROR_NONE) {
        printf("%s: txpulse checkerboard test passed!\n", __func__);
    } else {
        printf("%s: txpulse checkerboard test fail, result=%d\n", __func__, *result);
    }

out:
    return status;
}

static int esd_check_test(fpc_tee_sensor_t *sensor, uint32_t *result, fpc_sensortest_test_t op)
{
    int status = 0;
    int supported = 0;
    fpc_ta_sensortest_test_params_t params;
    memset(&params, 0, sizeof(params));
    uint32_t image_captured;
    uint32_t log_size;
    char *str = " ";
    *result = 1;

    status = fpc_tee_sensortest_is_test_supported(sensor,
                                                  op,
                                                  &supported);
    if (op == FPC_SENSORTEST_ESD_CHECK_PRE_TEST) {
        str = " pre";
    } else if (op == FPC_SENSORTEST_ESD_CHECK_POST_TEST) {
        str = " post";
    }

    if (status || !supported) {
        printf("%s: esd check%s test is not supported in current SW! "
               "Please build SW with ESD_IN_STT=y\n", __func__, str);
        goto out;
    }

    status = fpc_tee_sensortest_run_test(sensor,
                                         op,
                                         0,
                                         &params,
                                         &image_captured,
                                         result,
                                         &log_size);
    if (*result == FPC_ERROR_NONE) {
        printf("%s: esd check%s test passed!\n", __func__, str);
    } else {
        printf("%s: esd check%s test fail!\n", __func__, str);
    }

out:
    return status;
}

static int afd_cal_test(fpc_tee_sensor_t *sensor, uint32_t *result)
{
    int status = 0;
    int supported = 0;
    fpc_ta_sensortest_test_params_t params;
    memset(&params, 0, sizeof(params));
    uint32_t image_captured;
    uint32_t log_size;
    *result = 1;

    status = fpc_tee_sensortest_is_test_supported(sensor,
                                                  FPC_SENSORTEST_AFD_CAL_TEST,
                                                  &supported);
    if (status || !supported) {
        printf("%s: afd cal test does not support this sensor!\n", __func__);
        goto out;
    }

    status = fpc_tee_sensortest_run_test(sensor,
                                         FPC_SENSORTEST_AFD_CAL_TEST,
                                         0,
                                         &params,
                                         &image_captured,
                                         result,
                                         &log_size);
    if (*result == FPC_ERROR_NONE) {
        printf("%s: afd cal test passed!\n", __func__);
    } else {
        printf("%s: afd cal test fail!\n", __func__);
    }

out:
    return status;
}

static int usage(char *pname, bool sensortest, bool engineering)
{
    printf("%s usage:\n"
           "-e, --enroll_test                          Runs enroll_test\n"
           "-a, --authentication_test                  Runs authentication_test\n"
           "-d, --db_blob_test                         Runs db_blob_test\n"
           , pname);
    if (sensortest && engineering) {
        printf(
            "-s, --sensor_test                          Runs sensor test\n"
            "-b, --checkerboard_test                    Runs checkerboard_test\n"
            "-q, --imagequality_test                    Runs imagequality_test\n"
            "-p, --resetpixel_test                      Runs resetpixel_test\n"
            "-u, --capture_uncalibrated                 Runs capture_uncalibrated\n"
            "-C, --afd_calibration_test                 Runs afd_calibration_test\n"
            "-E, --enroll_no_sensor_test                Runs enroll_no_sensor_test\n"
            "-A, --authentication_no_sensor_test        Runs authentication_no_sensor_test\n"
            "-D, --defective_pixels_test                Runs defective_pixels_test\n"
            "-m, --module_quality_test                  Runs module_quality_test\n"
            "-o, --otp_validation_test                  Runs otp_validation_test\n"
            "-t, --txpulse_checkerboard_test            Runs txpulse_checkerboard_test\n"
            "-x,  --esd_check_pre_test                  Runs esd_check_pre_test\n"
            "-y,  --esd_check_test                      Runs esd_check_test\n"
            "-z,  --esd_check_post_test                 Runs esd_check_post_test\n"
            "-l,  --afd_cal_test                        Runs afd_cal_test\n");
    }

    return -1;
}

int main(int argc, char **argv)
{
    uint32_t result = 1;
    int opt = 0;
    int long_index = 0;
    int status = -1;
    int8_t no_sensor = 0;
    fpc_tee_bio_t *bio = NULL;
    fpc_tee_sensor_t *sensor = NULL;

    if(argc > 1 && strcmp(argv[1], "trusty") == 0){
        return trusty_test();
    }

    fpc_tee_t *tee = fpc_tee_init();
    if (NULL == tee) {
        LOGE("%s , error creating tee_handle.", __func__);
        goto out;
    }

    sensor = fpc_tee_sensor_init(tee);
    if (NULL == sensor) {
        LOGE("%s , error creating sensor_handle.", __func__);
        goto out;
    }

    bio = fpc_tee_bio_init(tee);
    if (NULL == bio) {
        goto out;
    }
    status = fpc_tee_log_build_info(tee);
    if (status) {
        LOGD("%s, An error(%d) occurred in fpc_tee_log_build_info", __func__, status);
    }

    status = fpc_tee_set_token_validation_enable(tee, false);
    if (status) {
        LOGE("Can't disable token validation");
    }

    if (argc < 2) {
        return usage(argv[0], fpc_tee_sensortest_enabled(tee), fpc_tee_engineering_enabled(tee));
    }

    static struct option long_options[] =
    {
        {"sensor_test",                       no_argument,    0,  's'},
        {"checkerboard_test",                 no_argument,    0,  'b'},
        {"imagequality_test",                 no_argument,    0,  'q'},
        {"resetpixel_test",                   no_argument,    0,  'p'},
        {"capture_uncalibrated",              no_argument,    0,  'u'},
        {"enroll_test",                       no_argument,    0,  'e'},
        {"enroll_no_sensor_test",             no_argument,    0,  'E'},
        {"authentication_test",               no_argument,    0,  'a'},
        {"db_blob_test",                      no_argument,    0,  'd'},
        {"authentication_no_sensor_test",     no_argument,    0,  'A'},
        {"afd_calibration_test",              no_argument,    0,  'C'},
        {"defective_pixels_test",             no_argument,    0,  'D'},
        {"module_quality_test",               no_argument,    0,  'm'},
        {"otp_validation_test",               no_argument,    0,  'o'},
        {"txpulse_checkerboard_test",         no_argument,    0,  't'},
        {"esd_check_pre_test",                no_argument,    0,  'x'},
        {"esd_check_test           ",         no_argument,    0,  'y'},
        {"esd_check_post_test",               no_argument,    0,  'z'},
        {"afd_cal_test",                      no_argument,    0,  'l'},
        {0,                                   0,              0,    0}
    };

    while ((opt = getopt_long(argc, argv, "sbqcpueEadACrRDmotxyzl",
                              long_options, &long_index)) != -1) {
        switch (opt) {
        case 's':
            printf("Starting sensor_test\n");
            status = sensor_test(sensor, &result);
            break;
        case 'b':
            printf("Starting checkerboard_test\n");
            status = checkerboard_test(sensor, &result);
            break;
        case 'q':
            printf("Starting imagequality_test\n");
            status = imagequality_test(sensor, &result);
            break;
        case 'p':
            printf("Starting resetpixel_test\n");
            status = resetpixel_test(sensor, &result);
            break;
        case 'u':
            printf("Starting capture_uncalibrated\n");
            status = capture_uncalibrated(sensor);
            printf("End of test.\n");
            return status;
        case 'C': //Supported by fpc1022, fpc1023, fpc1270
            printf("Starting afd_calibration_test\n");
            status = afd_calibration_test(sensor, &result);
            break;
        case 'e':
            printf("Starting enroll_test\n");
            status = enroll_test(sensor, bio, tee, no_sensor);
            break;
        case 'a':
            printf("Starting authentication_test\n");
            enroll_test(sensor, bio, tee, no_sensor);
            status = authentication_test(sensor, bio, tee, no_sensor);
            break;
        case 'd':
            printf("Starting db blob test\n");
            status = test_fpc_tee_db_blob(tee);
            break;
        case 'E':
            printf("Starting enroll_no_sensor test\n");
            no_sensor = 1;
            status = enroll_test(sensor, bio, tee, no_sensor);
            break;
        case 'A':
            printf("Starting authentication_no_sensor test\n");
            no_sensor = 1;
            status = enroll_test(sensor, bio, tee, no_sensor);
            if (0 == status) {
                printf("enroll finished begin verify\n");
                status = authentication_test(sensor, bio, tee, no_sensor);
            }
            break;
        case 'D':
            printf("Starting defective_pixels_test\n");
            status = defective_pixels_test(sensor, &result);
            break;
        case 'm':
            printf("Starting module_quality_test\n");
            status = module_quality_test(sensor, &result);
            break;
        case 'o':
            printf("Starting otp_validation_test\n");
            status = otp_validation_test(sensor, &result);
            break;
        case 't':
            printf("Starting txpulse_checkerboard_test\n");
            status = txpulse_checkerboard_test(sensor, &result);
            break;
        case 'x':
            printf("Starting esd_check_pre_test\n");
            status = esd_check_test(sensor, &result, FPC_SENSORTEST_ESD_CHECK_PRE_TEST);
            break;
        case 'y':
            printf("Starting esd_check_test\n");
            status = esd_check_test(sensor, &result, FPC_SENSORTEST_ESD_CHECK_TEST);
            break;
        case 'z':
            printf("Starting esd_check_post_test\n");
            status = esd_check_test(sensor, &result, FPC_SENSORTEST_ESD_CHECK_POST_TEST);
            break;
        case 'l':
            printf("Starting afd cal test\n");
            status = afd_cal_test(sensor, &result);
            break;
        default:
            printf("Non handled option '%c'. Close down\n", opt);
            fpc_tee_bio_release(bio);
            fpc_tee_sensor_release(sensor);
            fpc_tee_release(tee);
            return usage(argv[0],
                         fpc_tee_sensortest_enabled(tee),
                         fpc_tee_engineering_enabled(tee));
        }
    }

    fpc_tee_set_token_validation_enable(tee, true);
out:
    fpc_tee_bio_release(bio);
    fpc_tee_sensor_release(sensor);
    fpc_tee_release(tee);
    if (status != 0) {
        LOGE("%s could not perform test\n", __func__);
        return -1;
    }

    printf("End of test\n");
    return (int) result;
}
