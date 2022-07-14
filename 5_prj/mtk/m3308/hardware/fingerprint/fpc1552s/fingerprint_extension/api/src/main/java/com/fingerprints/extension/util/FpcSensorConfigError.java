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
 * CONFIG_ERROR
 */
package com.fingerprints.extension.util;

public class FpcSensorConfigError {

    public enum Error {
        CONFIG_ERROR_NOT_SET(0),
        CONFIG_ERROR_NOT_SUPPORTED(1),
        CONFIG_ERROR_UNDEFINED(-1000);

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
            return CONFIG_ERROR_UNDEFINED;
        }
    }

    private int mFpcError;

    public FpcSensorConfigError(int fpcError) {
        this.mFpcError = fpcError;
    }

    public Error getModuleInternalEnum() {
        return Error.fromInteger(mFpcError);
    }
}
