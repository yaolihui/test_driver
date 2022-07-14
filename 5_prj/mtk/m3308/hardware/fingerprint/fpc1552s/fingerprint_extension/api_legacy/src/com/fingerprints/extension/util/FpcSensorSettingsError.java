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
 * SETTINGS_ERROR
 */
package com.fingerprints.extension.util;

public class FpcSensorSettingsError {

    public enum Error {
        SETTINGS_ERROR_NOT_SET(0),
        SETTINGS_ERROR_NOT_SUPPORTED(1),
        SETTINGS_ERROR_INVALID_ARGUMENT(2),
        SETTINGS_ERROR_NULL_ARGUMENT(3),
        SETTINGS_ERROR_INSUFFICIENT_MEMORY(4),
        SETTINGS_ERROR_INVALID_SENSOR(5),
        SETTINGS_ERROR_INVALID_COMPANION(6),
        SETTINGS_ERROR_INVALID_COMPANION_FAB(7),
        SETTINGS_ERROR_INVALID_COMPANION_REV(8),
        SETTINGS_ERROR_INVALID_COATING(9),
        SETTINGS_ERROR_DESCRIPTOR_NOT_SUPPORTED(10),
        SETTINGS_ERROR_INVALID_SETTINGS_SRC(11),
        SETTINGS_ERROR_INVALID_GROUP(12),
        SETTINGS_ERROR_INVALID_REGISTER(13),
        SETTINGS_ERROR_NO_LIMITS_DATA(14),
        SETTINGS_ERROR_INVALID_LIMITS_GROUP(15),
        SETTINGS_ERROR_INVALID_LIMITS_VALUE(16),
        SETTINGS_ERROR_UNDEFINED(-1000);

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
            return SETTINGS_ERROR_UNDEFINED;
        }
    }

    private int mFpcError;

    public FpcSensorSettingsError(int fpcError) {
        this.mFpcError = fpcError;
    }

    public Error getModuleInternalEnum() {
        return Error.fromInteger(mFpcError);
    }
}
