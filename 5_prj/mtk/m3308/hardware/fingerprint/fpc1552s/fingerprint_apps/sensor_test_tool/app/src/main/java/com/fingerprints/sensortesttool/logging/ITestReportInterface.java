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
package com.fingerprints.sensortesttool.logging;

import com.fingerprints.extension.sensortest.SensorInfo;
import com.fingerprints.extension.sensortest.SensorTestResult;
import com.fingerprints.sensortesttool.testcases.ATestCase;
import com.google.gson.JsonElement;

public interface ITestReportInterface {
    public void reportError(final String text);

    public void reportOk(final String text);

    public void reportMessage(final String text);

    public void reportText(final String text);

    public void reportJson(final JsonElement json);

    public void testRunStarted(final SensorInfo sensorInfo);

    public void testRunComplete();

    public void reportTestComplete(final ATestCase testCase, final SensorTestResult result);

    public void i(final String text);

    public void e(final String text);

    public void e(final String text, final Throwable throwable);

    public void d(final String text);

    public void enter();

    public void exit();

    public void clear();
}
