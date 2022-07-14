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
 * FPC_SENSOR_INTERFACE_ERROR
 */
package com.fingerprints.extension.util;

public class FpcSensorInterfaceError {

    public enum Error {
        SENSOR_INTERFACE_ERROR_NOT_SET(0),
        SENSOR_INTERFACE_ERROR_NOT_SUPPORTED(1),
        SENSOR_INTERFACE_ERROR_NULL_RESPONSE(2),
        SENSOR_INTERFACE_ERROR_IMAGE_DATA(3),
        SENSOR_INTERFACE_ERROR_IRQ(4),
        SENSOR_INTERFACE_ERROR_BUFFER(5),
        SENSOR_INTERFACE_ERROR_CONTEXT(6),
        SENSOR_INTERFACE_ERROR_CTL(7),
        SENSOR_INTERFACE_ERROR_CAPTURE(8),
        SENSOR_INTERFACE_ERROR_UNDEFINED(-1000);

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
            return SENSOR_INTERFACE_ERROR_UNDEFINED;
        }
    }

    private int mFpcError;

    public FpcSensorInterfaceError(int fpcError) {
        this.mFpcError = fpcError;
    }

    public Error getModuleInternalEnum() {
        return Error.fromInteger(mFpcError);
    }
}
