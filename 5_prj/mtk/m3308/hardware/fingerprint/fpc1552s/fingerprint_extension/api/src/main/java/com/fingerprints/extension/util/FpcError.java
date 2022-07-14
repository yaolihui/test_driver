package com.fingerprints.extension.util;

import com.fingerprints.extension.util.FpcClientError;
import com.fingerprints.extension.util.FpcSensorAccessError;
import com.fingerprints.extension.util.FpcSensorBioInterfaceError;
import com.fingerprints.extension.util.FpcSensorConfigError;
import com.fingerprints.extension.util.FpcSensorDriverError;
import com.fingerprints.extension.util.FpcSensorInterfaceError;
import com.fingerprints.extension.util.FpcSensorSettingsError;

public class FpcError {

    public enum Error {
        FPC_STATUS_WAIT_TIME(1),
        FPC_STATUS_FINGER_PRESENT(2),
        FPC_STATUS_FINGER_LOST(3),
        FPC_STATUS_BAD_QUALITY(4),
        FPC_STATUS_FINGER_ALREADY_ENROLLED(5),

        FPC_STATUS_ENROLL_PROGRESS(6),
        FPC_STATUS_ENROLL_LOW_COVERAGE(7),
        FPC_STATUS_ENROLL_TOO_SIMILAR(8),
        FPC_STATUS_ENROLL_LOW_QUALITY(9),
        FPC_STATUS_ENROLL_LOW_MOBILITY(10),

        FPC_ERROR_NONE(0),
        FPC_ERROR_NOT_FOUND(-1),
        FPC_ERROR_CAN_BE_USED_2(-2),
        FPC_ERROR_CAN_BE_USED_3(-3),
        FPC_ERROR_CAN_BE_USED_4(-4),
        FPC_ERROR_PAL(-5),
        FPC_ERROR_IO(-6),
        FPC_ERROR_CANCELLED(-7),
        FPC_ERROR_UNKNOWN(-8),
        FPC_ERROR_MEMORY(-9),
        FPC_ERROR_PARAMETER(-10),
        FPC_ERROR_TEST_FAILED(-11),
        FPC_ERROR_TIMEDOUT(-12),
        FPC_ERROR_SENSOR(-13),
        FPC_ERROR_SPI(-14),
        FPC_ERROR_NOT_SUPPORTED(-15),
        FPC_ERROR_OTP(-16),
        FPC_ERROR_STATE(-17),
        FPC_ERROR_PN(-18),
        FPC_ERROR_DEAD_PIXELS(-19),
        FPC_ERROR_TEMPLATE_CORRUPTED(-20),
        FPC_ERROR_CRC(-21),
        FPC_ERROR_STORAGE(-22),
        FPC_ERROR_MAXIMUM_REACHED(-23),
        FPC_ERROR_MINIMUM_NOT_REACHED(-24),
        FPC_ERROR_SENSOR_LOW_COVERAGE(-25),
        FPC_ERROR_SENSOR_LOW_QUALITY(-26),
        FPC_ERROR_SENSOR_FINGER_NOT_STABLE(-27),
        FPC_ERROR_TOO_MANY_FAILED_ATTEMPTS(-28),
        FPC_ERROR_ALREADY_ENROLLED(-29),

        FPC_ERROR_UNDEFINED(-1000);

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
            return FPC_ERROR_UNDEFINED;
        }
    }

    private static final int FPC_ERROR_MSB_OFFSET_ERROR_MASK = 0x8000000;

    private static final int ERROR_BITS = 1;
    /**
     * < One bit indicating if the result is an error.
     */
    private static final int MODULE_BITS = 5;
    /**
     * < Number of identifying the module.
     */
    private static final int EXTERNAL_BITS = 6;
    /**
     * < Number of identifying the external result.
     */
    private static final int INTERNAL_BITS = 6;
    /**
     * < Number of identifying the internal result.
     */
    private static final int EXTRA_BITS = 14;
    /**
     * < Number of bits left for user defined extra data.
     */

    // Max value for each group
    private static final int MAX_ERROR = ((1 << ERROR_BITS) - 1);
    /**
     * < Max value in error group
     */
    private static final int MAX_MODULE = ((1 << MODULE_BITS) - 1);
    /**
     * < Max value in module group
     */
    private static final int MAX_EXTERNAL = ((1 << EXTERNAL_BITS) - 1);
    /**
     * < Max value in external group
     */
    private static final int MAX_INTERNAL = ((1 << INTERNAL_BITS) - 1);
    /**
     * < Max value in internal group
     */
    private static final int MAX_EXTRA = ((1 << EXTRA_BITS) - 1);
    /**
     * < Max value in extra group
     */

    // Helper macro to mask bits
    private static final int MSB_OFFSET_ERROR = (32 - (ERROR_BITS));
    private static final int MSB_OFFSET_MODULE = (32 - (ERROR_BITS + MODULE_BITS));
    private static final int MSB_OFFSET_EXTERNAL = (32 - (ERROR_BITS + MODULE_BITS + EXTERNAL_BITS));
    private static final int MSB_OFFSET_INTERNAL = (32 - (ERROR_BITS + MODULE_BITS + EXTERNAL_BITS + INTERNAL_BITS));
    private static final int MSB_OFFSET_EXTRA = (32 - (ERROR_BITS + MODULE_BITS + EXTERNAL_BITS + INTERNAL_BITS + EXTRA_BITS));

    // Helper macro to mask bits
    private static final int SHIFT_ERROR = (0);
    private static final int SHIFT_MODULE = (ERROR_BITS);
    private static final int SHIFT_EXTERNAL = (ERROR_BITS + MODULE_BITS);
    private static final int SHIFT_INTERNAL = (ERROR_BITS + MODULE_BITS + EXTERNAL_BITS);
    private static final int SHIFT_EXTRA = (ERROR_BITS + MODULE_BITS + EXTERNAL_BITS + INTERNAL_BITS);

    /* Shifted masks for the defined groups to be used in get macros
     * (Comment assumes 1-5-6-6-14 distribution)
     */
    private static final int ERROR_MASK = (MAX_ERROR << MSB_OFFSET_ERROR);          // 0x80000000
    private static final int MODULE_MASK = (MAX_MODULE << MSB_OFFSET_MODULE);       // 0x7c000000
    private static final int EXTERNAL_MASK = (MAX_EXTERNAL << MSB_OFFSET_EXTERNAL); // 0x03f00000
    private static final int INTERNAL_MASK = (MAX_INTERNAL << MSB_OFFSET_INTERNAL); // 0x000fc000
    private static final int EXTRA_MASK = (MAX_EXTRA << MSB_OFFSET_EXTRA);          // 0x00003fff

    private static final int MODULE_ID_RESERVED_MIN = 0;
    private static final int MODULE_ID_RESERVED = 31;

    /*
     * The definitions of FPC module ID according to the fpc_error.h
     */
    private final int FPC_MODULE_ID_SENSOR = 1;
    private final int FPC_MODULE_ID_SETTINGS = 2;
    private final int FPC_MODULE_ID_ACCESS = 3;
    private final int FPC_MODULE_ID_CONFIG = 4;
    private final int FPC_MODULE_ID_PERIPHIALS = 5;
    private final int FPC_MODULE_ID_DB = 25;
    private final int FPC_MODULE_ID_TA = 26;
    private final int FPC_MODULE_ID_NAVIGATION = 27;
    private final int FPC_MODULE_ID_BIO_INTERFACE = 28;
    private final int FPC_MODULE_ID_SENSOR_INTERFACE = 29;
    private final int FPC_MODULE_ID_CLIENT = 30;

    /*
     * The declarations of FPC Error objects
     */
    private FpcSensorDriverError mFpcSensorDriverError;
    private FpcSensorSettingsError mFpcSensorSettingsError;
    private FpcSensorAccessError mFpcSensorAccessError;
    private FpcSensorConfigError mFpcSensorConfigError;
    private FpcSensorBioInterfaceError mFpcSensorBioInterfaceError;
    private FpcSensorInterfaceError mFpcSensorInterfaceError;
    private FpcClientError mFpcClientError;

    private int mFpcError;

    public FpcError(int fpcError) {
        this.mFpcError = fpcError;
    }

    /**
     * Return true if the error code represented an error
     */
    public boolean isError() {
        return (mFpcError & ERROR_MASK) != 0;
    }

    public Error getExternalEnum() {
        Error result;
        if (isLegacyFormat()) {
            result = Error.fromInteger(mFpcError);
        } else {
            result = Error.fromInteger(isError() ? -getExternal() : getExternal());
        }
        return result;
    }

    /**
     * Get the "external" field of this error
     */
    private int getExternal() {
        return ((((mFpcError)) & EXTERNAL_MASK) >> MSB_OFFSET_EXTERNAL);
    }

    /**
     * Get the "module id" field of this error
     */
    private int getModuleId() {
        return ((((mFpcError)) & MODULE_MASK) >> MSB_OFFSET_MODULE);
    }

    /**
     * Get the "internal" field of this error
     */
    private int getInternal() {
        return ((((mFpcError)) & INTERNAL_MASK) >> MSB_OFFSET_INTERNAL);
    }

    /**
     * Get the "extra" field of this error
     */
    private int getExtra() {
        return ((((mFpcError)) & EXTRA_MASK) >> MSB_OFFSET_EXTRA);
    }

    /**
     * Returns true if the error code was in the legacy format
     */
    private boolean isLegacyFormat() {
        return (getModuleId() == MODULE_ID_RESERVED_MIN) || (getModuleId() == MODULE_ID_RESERVED);
    }

    /**
     * @return The error code used to create this FpcError
     */
    public int getErrorCode() {
        return mFpcError;
    }

    public String toString() {
        return String.format("FpcError(0x%08X): isError:%b, external=%d, module id=%d, " +
                        "internal=%d, extra=%d, value=%s", getErrorCode(), isError(), getExternal(),
                getModuleId(), getInternal(), getExtra(), getExternalEnum().name());
    }

    public String describe() {
        return String.format("%s(%d)", getExternalEnum().name(), getErrorCode());
    }

    public String getExternalErrorCode() {
        if (FPC_MODULE_ID_ACCESS == this.getModuleId()) {
            mFpcSensorAccessError = new FpcSensorAccessError(this.getInternal());
            if (mFpcSensorAccessError.isNoSensorConnectedError(this.getInternal())) {
                return String.format("No sensor connected!");
            }
        }
        return String.format("%s", this.getExternalEnum());
    }

    public String getModuleInternalError() {
        String out = null;
        switch (this.getModuleId()) {
            case FPC_MODULE_ID_SENSOR:
                mFpcSensorDriverError = new FpcSensorDriverError(this.getInternal());
                out = String.format("%s", mFpcSensorDriverError.getModuleInternalEnum().name());
                break;
            case FPC_MODULE_ID_SETTINGS:
                mFpcSensorSettingsError = new FpcSensorSettingsError(this.getInternal());
                out = String.format("%s", mFpcSensorSettingsError.getModuleInternalEnum().name());
                break;
            case FPC_MODULE_ID_ACCESS:
                mFpcSensorAccessError = new FpcSensorAccessError(this.getInternal());
                out = String.format("%s", mFpcSensorAccessError.getModuleInternalEnum().name());
                break;
            case FPC_MODULE_ID_CONFIG:
                mFpcSensorConfigError = new FpcSensorConfigError(this.getInternal());
                out = String.format("%s", mFpcSensorConfigError.getModuleInternalEnum().name());
                break;
            case FPC_MODULE_ID_BIO_INTERFACE:
                mFpcSensorBioInterfaceError = new FpcSensorBioInterfaceError(this.getInternal());
                out = String.format("%s", mFpcSensorBioInterfaceError.getModuleInternalEnum().name());
                break;
            case FPC_MODULE_ID_SENSOR_INTERFACE:
                mFpcSensorInterfaceError = new FpcSensorInterfaceError(this.getInternal());
                out = String.format("%s", mFpcSensorInterfaceError.getModuleInternalEnum().name());
                break;
            case FPC_MODULE_ID_CLIENT:
                mFpcClientError = new FpcClientError(this.getInternal());
                out = String.format("%s", mFpcClientError.getModuleInternalEnum().name());
                break;
            default:
                out = String.format("%s", this.getExternalEnum().name());
                break;
        }
        return out;
    }
}
