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
 * ACCESS_ERROR
 */
package com.fingerprints.extension.util;

public class FpcSensorAccessError {

    public enum Error {
        ACCESS_ERROR_NOT_SET(0),
        ACCESS_ERROR_NOT_SUPPORTED(1),
        ACCESS_ERROR_POLL_IRQ(2),
        ACCESS_ERROR_POLL_IDLE(3),
        ACCESS_ERROR_READ_HW_ID(4),
        ACCESS_ERROR_READ_CAPT_SIZE(5),
        ACCESS_ERROR_READ_IMG(6),
        ACCESS_ERROR_UNKNOWN_HW_ID(7),
        ACCESS_ERROR_CMD_SYNC(8),
        ACCESS_ERROR_CTX(9),
        ACCESS_ERROR_ADDR(10),
        ACCESS_ERROR_BUFFER(11),
        ACCESS_ERROR_BUFFER_SIZE(12),
        ACCESS_ERROR_REG_COUNT(13),
        ACCESS_ERROR_SPI_PAL(14),
        ACCESS_ERROR_DATA(15),
        ACCESS_ERROR_ADAPT_INSERT(16),
        ACCESS_ERROR_REG_TOO_LONG(17),
        ACCESS_ERROR_CRC_READ(18),
        ACCESS_ERROR_CRC_WRITE(19),
        ACCESS_ERROR_WAIT_IRQ(20),
        ACCESS_ERROR_REG_SIZE_ZERO(21),
        ACCESS_ERROR_UNDEFINED(-1000);

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
            return ACCESS_ERROR_UNDEFINED;
        }
    }

    private int mFpcError;

    public FpcSensorAccessError(int fpcError) {
        this.mFpcError = fpcError;
    }

    public Error getModuleInternalEnum() {
        return Error.fromInteger(mFpcError);
    }

    public boolean isNoSensorConnectedError(int errorCode) {
        return (errorCode >= Error.ACCESS_ERROR_POLL_IRQ.ordinal()
                && errorCode <= Error.ACCESS_ERROR_READ_HW_ID.ordinal());
    }
}
