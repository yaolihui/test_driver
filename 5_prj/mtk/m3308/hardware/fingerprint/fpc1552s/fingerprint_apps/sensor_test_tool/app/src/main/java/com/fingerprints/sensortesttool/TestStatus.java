/*
 *
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */
package com.fingerprints.sensortesttool;

import com.fingerprints.extension.sensortest.SensorTestResult;

public enum TestStatus {
    RUNNING, STOPPED, PENDING, WAITING_FOR_USER, PASSED, FAILED, CANCELLED, NOT_SUPPORTED, ERROR, UNKNOWN;

    public boolean isPassed() {
        return this == PASSED;
    }

    public boolean isFailed() {
        return this == FAILED || this == ERROR || this == NOT_SUPPORTED;
    }

    public boolean isCancelled() {
        return this == CANCELLED;
    }

    public boolean isRunning() {
        return this == RUNNING || this == WAITING_FOR_USER;
    }

    public boolean isWaiting() {
        return this == WAITING_FOR_USER;
    }

    public String getInfoString() {

        switch (this) {
            case STOPPED:
                return "";
            case RUNNING:
                return "Running";
            case PENDING:
                return "Pending";
            case WAITING_FOR_USER:
                return "Awaiting input...";
            case PASSED:
                return "Passed";
            case FAILED:
                return "Failed";
            case CANCELLED:
                return "Cancelled";
            case NOT_SUPPORTED:
                return "Test not supported";
            case ERROR:
                return "Error";
            case UNKNOWN:
                return "Internal error";
        }

        return super.toString();
    }

    public static TestStatus getStatusFromResult(final SensorTestResult sensorTestResult) {
        if (sensorTestResult.resultCode == SensorTestResult.ResultCode.PASS) {
            return PASSED;
        } else if (sensorTestResult.resultCode == SensorTestResult.ResultCode.FAIL) {
            return FAILED;
        } else if (sensorTestResult.resultCode == SensorTestResult.ResultCode.CANCELLED) {
            return CANCELLED;
        } else if (sensorTestResult.resultCode == SensorTestResult.ResultCode.NOT_SUPPORTED) {
            return NOT_SUPPORTED;
        } else if (sensorTestResult.resultCode == SensorTestResult.ResultCode.ERROR) {
            return ERROR;
        }
        return UNKNOWN;
    }
}
