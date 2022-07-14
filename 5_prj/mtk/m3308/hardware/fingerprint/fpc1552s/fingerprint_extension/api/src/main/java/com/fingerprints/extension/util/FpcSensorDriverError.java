/*
 *
 * Copyright (c) 2018 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */
/*
 * SENSOR_DRIVER_ERROR
 */
package com.fingerprints.extension.util;

public class FpcSensorDriverError {

    public enum Error {
        DRIVER_E_NOT_SET(0),
        DRIVER_E_NOT_SUPPORTED(1),
        DRIVER_E_NULL_RESPONSE(2),
        DRIVER_E_CONFIG_MISSMATCH(3),
        DRIVER_E_CAC_FACTORY(4),
        DRIVER_E_CAC_ERROR(5),
        DRIVER_E_CAC_FASTTAP(6),
        DRIVER_E_CAC_FASTTAP_NES(7),
        DRIVER_E_CAC_SIMPLE(8),
        DRIVER_E_CAC_CUSTOM(9),
        DRIVER_E_CAC_MQT(10),
        DRIVER_E_CAC_PN(11),
        DRIVER_E_CAC_SIZE(12),
        DRIVER_E_CAC_EARLY_STOP(13),
        DRIVER_E_CTX(14),
        DRIVER_E_BUFFER(15),
        DRIVER_E_BUFFER_SIZE(16),
        DRIVER_E_IMAGE_DATA(17),
        DRIVER_E_CAPTURE_TYPE(18),
        DRIVER_E_CAPTURE_CONFIG(19),
        DRIVER_E_SENSOR_TYPE(20),
        DRIVER_E_BN_CREATE(21),
        DRIVER_E_BN_CALIBRATE(22),
        DRIVER_E_BN_MEMORY(23),
        DRIVER_E_COMPANION_TYPE(24),
        DRIVER_E_COATING_TYPE(25),
        DRIVER_E_MODE_TYPE(26),
        DRIVER_E_OTP_STATE(27),
        DRIVER_E_OVERLAY_SIZE(28),
        DRIVER_E_DP_CHECKERBOARD(29),
        DRIVER_E_DP_FPCIMAGE(30),
        DRIVER_E_DP_RESET_PIXEL(31),
        DRIVER_E_DP_NOT_SUPPORTED(32),
        DRIVER_E_DP_IMG_PARAM(33),
        DRIVER_E_PAL_DELAY(34),
        DRIVER_E_AFD_INCOMPLETE(35),
        DRIVER_E_AFD_RUNLEVEL(36),
        DRIVER_E_RRS_REG_SIZE(37),
        DRIVER_E_FNGR_DURING_CAL(38),
        DRIVER_E_DFD_CAL_OVERFLOW(39),
        DRIVER_E_OSC_CAL_UNDERFLOW(40),
        DRIVER_E_OSC_CAL_INVALID(41),
        DRIVER_E_NULL_CONFIG(42),
        DRIVER_E_NULL_HW_DESCRIPTOR(43),
        DRIVER_E_DP_IMAGE_DRIVE(44),
        DRIVER_E_DP_IMAGE_CONSTANT(45),
        DRIVER_E_DP_CONFIG(46),
        DRIVER_ERROR_UNDEFINED(-1000);

        private final int mErrorCode;

        Error(int errorCode) {
            mErrorCode = errorCode;
        }

        public static Error fromInteger(int errorCode) {
            for (Error error : Error.values()) {
                if (error.mErrorCode == errorCode) {
                    return error;
                }
            }
            return DRIVER_ERROR_UNDEFINED;
        }
    }

    private int mFpcError;

    public FpcSensorDriverError(int fpcError) {
        this.mFpcError = fpcError;
    }

    public Error getModuleInternalEnum() {
        return Error.fromInteger(mFpcError);
    }
}
