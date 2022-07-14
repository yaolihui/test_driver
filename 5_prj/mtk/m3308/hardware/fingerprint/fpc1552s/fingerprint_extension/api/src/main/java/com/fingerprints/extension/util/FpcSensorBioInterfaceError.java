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
 * FPC_BIO_INTERFACE_ERROR
 */
package com.fingerprints.extension.util;

public class FpcSensorBioInterfaceError {

    public enum Error {
        BIO_INTERFACE_ERROR_NOT_SET(0),
        BIO_INTERFACE_ERROR_NOT_SUPPORTED(1),
        BIO_INTERFACE_ERROR_BUFFER(2),
        BIO_INTERFACE_ERROR_OUT_OF_RANGE(3),
        BIO_INTERFACE_ERROR_BIO(63),
        BIO_INTERFACE_ERROR_UNDEFINED(-1000);

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
            return BIO_INTERFACE_ERROR_UNDEFINED;
        }
    }

    private int mFpcError;

    public FpcSensorBioInterfaceError(int fpcError) {
        this.mFpcError = fpcError;
    }

    public Error getModuleInternalEnum() {
        return Error.fromInteger(mFpcError);
    }
}
