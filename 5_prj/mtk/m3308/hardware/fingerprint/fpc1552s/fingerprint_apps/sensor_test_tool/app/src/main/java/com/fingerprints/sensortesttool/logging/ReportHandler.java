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

public class ReportHandler implements ITestReportInterface {
    private String mHtmlLog = "";
    private StringBuilder mXmlResultLog = new StringBuilder();
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private JsonElement json;

    public ReportHandler() {

    }

    public JsonElement getJson() {
        return json;
    }

    public boolean hasJson() {
        return json != null;
    }

    @Override
    public void testRunStarted(final SensorInfo sensorInfo) {
        if (sensorInfo != null) {
            json = null;
            mXmlResultLog = new StringBuilder();
            mXmlResultLog.append("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
            mXmlResultLog.append("<SensorTestResult>\n");

            mXmlResultLog.append("  <DeviceInfo ");
            for (String key : sensorInfo.getParameterMap().keySet()) {
                Object value = sensorInfo.getParameterMap().get(key);

                if (value instanceof byte[]) {
                    mXmlResultLog.append(key + "=\"" + new String((byte[]) (value)) + "\" ");
                } else {
                    mXmlResultLog.append(key + "=\"" + value + "\" ");
                }
            }

            mXmlResultLog.append("/>\n");
        }
    }

    @Override
    public void testRunComplete() {
        if (mXmlResultLog != null) {
            mXmlResultLog.append("</SensorTestResult>\n");
        }
        i("Automatic Run Complete");
    }

    @Override
    public void reportTestComplete(final ATestCase testCase,
                                   final SensorTestResult sensorTestResult) {
        if (mXmlResultLog != null) {
            mXmlResultLog.append("  <TestCase name=\"" + testCase.getId() +
                    "\" result=\"" + sensorTestResult.resultCode.getString() +
                    "\" resultString=\"" + sensorTestResult.resultString +
                    (sensorTestResult.imageFetched ? ("\" imageLocation=\"" +
                            testCase.getFileName()) : "") +
                    "\" errorCode=\"" + sensorTestResult.errorCode +
                    "\" errorString=\"" + sensorTestResult.errorString + "\"/>\n");

        }
    }

    public StringBuilder getXmlResultLog() {
        return mXmlResultLog;
    }

    public String getHtmlLog() {
        return mHtmlLog;
    }

    public void htmlLog(final String text, final String color) {
        this.mHtmlLog += "<font color=\"" + color + "\">" + text.replaceAll("\n", "<br>").replaceAll(" ", "&nbsp;") + "</font>";
    }

    @Override
    public void reportError(final String text) {
        e(text);
        htmlLog(text + "\n", "red");
    }

    @Override
    public void reportOk(final String text) {
        i(text);
        htmlLog(text + "\n", "green");
    }

    @Override
    public void reportText(final String text) {
        i(text);
        htmlLog(text + "\n", "white");
    }

    @Override
    public void reportJson(final JsonElement json) {
        this.json = json;
    }

    @Override
    public void reportMessage(final String text) {
        d(text);
        htmlLog(text + "\n", "yellow");
    }


    public void e(final String text, final Throwable throwable) {
        mLogger.e(text, throwable);
    }

    @Override
    public void e(final String text) {
        mLogger.e(text + "\n");
    }

    @Override
    public void i(final String text) {
        mLogger.i(text + "\n");
    }

    @Override
    public void d(final String text) {
        mLogger.d(text + "\n");
    }


    @Override
    public void exit() {
        StackTraceElement[] stack = Thread.currentThread().getStackTrace();
        if (stack.length >= 4) {
            StackTraceElement callingMethod = stack[3];
            mLogger.exit(" (" + callingMethod.getFileName() + ":" + callingMethod.getLineNumber() +
                    ") " + callingMethod.getMethodName());
        }
    }

    @Override
    public void clear() {
        mHtmlLog = "";
        mXmlResultLog = new StringBuilder();
    }

    @Override
    public void enter() {
        StackTraceElement[] stack = Thread.currentThread().getStackTrace();
        if (stack.length >= 4) {
            StackTraceElement callingMethod = stack[3];
            mLogger.enter(" (" + callingMethod.getFileName() + ":" + callingMethod.getLineNumber() +
                    ") " + callingMethod.getMethodName());
        }
    }
}
