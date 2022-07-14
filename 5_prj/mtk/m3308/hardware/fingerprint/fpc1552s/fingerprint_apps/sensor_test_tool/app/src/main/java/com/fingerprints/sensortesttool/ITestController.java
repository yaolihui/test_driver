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

import android.content.Context;
import android.os.RemoteException;

import com.fingerprints.extension.sensetouch.FingerprintSenseTouch;
import com.fingerprints.extension.sensortest.FingerprintSensorTest;
import com.fingerprints.sensortesttool.logging.ITestReportInterface;
import com.fingerprints.sensortesttool.testcases.ATestCase;

public interface ITestController {
    public ITestReportInterface getReport();

    public FingerprintSensorTest getSensorTest() throws RemoteException;

    public FingerprintSenseTouch getSenseTouch() throws RemoteException;

    public Context getContext();

    public void refresh(final ATestCase testCase);

    public void vibrate();
}
