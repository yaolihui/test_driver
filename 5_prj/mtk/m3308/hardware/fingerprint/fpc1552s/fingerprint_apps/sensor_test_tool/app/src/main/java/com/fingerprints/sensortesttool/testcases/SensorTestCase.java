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
package com.fingerprints.sensortesttool.testcases;

import android.os.RemoteException;

import com.fingerprints.extension.sensortest.SensorTest;
import com.fingerprints.extension.sensortest.SensorTestInput;
import com.fingerprints.extension.sensortest.SensorTestResult;
import com.fingerprints.sensortesttool.FingerprintSensorTestListenerAdapter;
import com.fingerprints.sensortesttool.ITestController;
import com.fingerprints.sensortesttool.logging.Logger;
import com.fingerprints.sensortesttool.tools.Disk;
import com.fingerprints.sensortesttool.values.Constants;

import tools.xml.GVisitor;
import tools.xml.IXMLNode;
import tools.xml.XMLParser;

public class SensorTestCase extends ATestCase {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private SensorTest mSensorTest;

    public SensorTestCase(final SensorTest sensorTest, final ITestController controller) {
        super(sensorTest.name, sensorTest.name.replaceAll(" ", "_").toLowerCase(), controller);
        mSensorTest = sensorTest;
        if (mSensorTest.waitForFingerDown) {
            setManual(true);
        }
    }

    public SensorTest getSensorTest() {
        return mSensorTest;
    }

    public String getDescription() {
        return mSensorTest.description;
    }

    public void runTest() throws RemoteException {
        SensorTestInput sensorTestInput = new SensorTestInput("test");
        final StringBuilder sb = new StringBuilder();

        try {
            String xmlFile = Disk.readExternalTextFile(Constants.SENSORTEST_TEST_LIMITS, getController().getContext());
            IXMLNode root = XMLParser.parseString(xmlFile);

            root.get("Test", new GVisitor<IXMLNode>() {
                @Override
                public void visit(final IXMLNode testNode) {
                    if (mSensorTest.name.equalsIgnoreCase(testNode.getValue("name"))) {
                        testNode.get("Limit", new GVisitor<IXMLNode>() {
                            @Override
                            public void visit(final IXMLNode limitNode) {
                                sb.append(limitNode.getValue("key"));
                                sb.append("=");
                                sb.append(limitNode.getValue("value"));
                                sb.append(";");
                            }
                        });
                    }
                }
            });
        } catch (Exception e) {
            mLogger.w("Exception: " + e);
        }

        sensorTestInput.testLimitsKeyValuePair = sb.toString().toLowerCase();

        getController().getSensorTest().runSensorTest(new FingerprintSensorTestListenerAdapter() {
            @Override
            public void onResult(final SensorTestResult sensorTestResult) {
                onTestComplete(sensorTestResult);
            }
        }, mSensorTest, sensorTestInput);
    }
}
