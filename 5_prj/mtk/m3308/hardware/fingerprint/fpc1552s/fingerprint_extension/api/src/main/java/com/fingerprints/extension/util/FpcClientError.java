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
 * FPC_LIB_ERROR
 */
package com.fingerprints.extension.util;

public class FpcClientError {

    public enum Error {
        FPC_LIB_ERROR_NOT_SET(0),
        FPC_LIB_ERROR_NOT_SUPPORTED(1),
        FPC_LIB_ERROR_NULL_RESPONSE(2),
        FPC_LIB_ERROR_STORAGE(27),
        FPC_LIB_ERROR_KPI(28),
        FPC_LIB_ERROR_COMMON(29),
        FPC_LIB_ERROR_C_CORE_IMAGE_DATA(30),
        FPC_LIB_ERROR_C_CORE_GENERIC(31),
        FPC_LIB_ERROR_UNDEFINED(-1000);

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
            return FPC_LIB_ERROR_UNDEFINED;
        }
    }

    private int mFpcError;

    public FpcClientError(int fpcError) {
        this.mFpcError = fpcError;
    }

    public Error getModuleInternalEnum() {
        return Error.fromInteger(mFpcError);
    }
}
