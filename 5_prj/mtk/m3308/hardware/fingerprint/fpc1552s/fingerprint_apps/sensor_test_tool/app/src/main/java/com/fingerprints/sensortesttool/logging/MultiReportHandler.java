/*
 *
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
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

import java.util.ArrayList;


public class MultiReportHandler implements ITestReportInterface {

    private ArrayList<ITestReportInterface> mReportTestInterfaces;

    public MultiReportHandler() {
        mReportTestInterfaces = new ArrayList<>();
    }

    public void addHandler(final ITestReportInterface handler) {
        mReportTestInterfaces.add(handler);
    }

    @Override
    public void reportError(final String text) {
        for (ITestReportInterface rh : mReportTestInterfaces) {
            rh.reportError(text);
        }
    }

    @Override
    public void reportOk(final String text) {
        for (ITestReportInterface rh : mReportTestInterfaces) {
            rh.reportOk(text);
        }
    }

    @Override
    public void reportMessage(final String text) {
        for (ITestReportInterface rh : mReportTestInterfaces) {
            rh.reportMessage(text);
        }
    }

    @Override
    public void reportText(final String text) {
        for (ITestReportInterface rh : mReportTestInterfaces) {
            rh.reportText(text);
        }
    }

    @Override
    public void reportJson(final JsonElement json) {
        for (ITestReportInterface rh : mReportTestInterfaces) {
            rh.reportJson(json);
        }
    }

    @Override
    public void testRunStarted(final SensorInfo sensorInfo) {
        for (ITestReportInterface rh : mReportTestInterfaces) {
            rh.testRunStarted(sensorInfo);
        }
    }

    @Override
    public void testRunComplete() {
        for (ITestReportInterface rh : mReportTestInterfaces) {
            rh.testRunComplete();
        }
    }

    @Override
    public void reportTestComplete(final ATestCase testCase, final SensorTestResult result) {
        for (ITestReportInterface rh : mReportTestInterfaces) {
            rh.reportTestComplete(testCase, result);
        }
    }

    @Override
    public void i(final String text) {
        for (ITestReportInterface rh : mReportTestInterfaces) {
            rh.i(text);
        }
    }

    @Override
    public void e(final String text) {
        for (ITestReportInterface rh : mReportTestInterfaces) {
            rh.e(text);
        }
    }

    @Override
    public void e(final String text, final Throwable throwable) {
        for (ITestReportInterface rh : mReportTestInterfaces) {
            rh.e(text);
        }
    }

    @Override
    public void d(final String text) {
        for (ITestReportInterface rh : mReportTestInterfaces) {
            rh.d(text);
        }
    }

    @Override
    public void enter() {
        for (ITestReportInterface rh : mReportTestInterfaces) {
            rh.enter();
        }
    }

    @Override
    public void exit() {
        for (ITestReportInterface rh : mReportTestInterfaces) {
            rh.exit();
        }
    }

    @Override
    public void clear() {
        for (ITestReportInterface rh : mReportTestInterfaces) {
            rh.clear();
        }
    }
}
