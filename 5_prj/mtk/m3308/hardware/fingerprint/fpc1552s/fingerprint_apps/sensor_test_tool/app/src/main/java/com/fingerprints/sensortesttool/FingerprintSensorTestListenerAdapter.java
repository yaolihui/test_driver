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

import com.fingerprints.extension.sensortest.FingerprintSensorTest;
import com.fingerprints.extension.sensortest.SensorTestResult;
import com.fingerprints.sensortesttool.logging.Logger;

public class FingerprintSensorTestListenerAdapter implements FingerprintSensorTest.SensorTestCallback {

    private Logger mLogger;

    public FingerprintSensorTestListenerAdapter() {
        mLogger = new Logger(getClass().getSimpleName());
    }

    @Override
    public void onResult(final SensorTestResult result) {
        mLogger.i("onResult (\" + result + \")");
    }
}
