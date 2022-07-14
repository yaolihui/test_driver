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

package com.fingerprints.sensortesttool.testcases.manual;

import android.app.Activity;
import android.graphics.Typeface;
import android.os.RemoteException;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.TextView;

import com.fingerprints.extension.common.CanceledException;
import com.fingerprints.extension.sensetouch.CalibrationCallback;
import com.fingerprints.extension.sensortest.SensorTestResult;
import com.fingerprints.sensortesttool.ITestController;
import com.fingerprints.sensortesttool.R;
import com.fingerprints.sensortesttool.testcases.AUITestCase;
import com.fingerprints.sensortesttool.views.ForceView;


public class SenseTouchCalibration extends AUITestCase {

    private static final long CALIBRATE_TIME = 3000;
    private TextView mCalibrateGroundText;
    private TextView mCalibrateThreshold;
    private View mView;
    private ForceView mForceView;

    public SenseTouchCalibration(final ITestController controller) {
        super("SenseTouch Calibration", "sense_touch_calibration", controller);
        setManual(true);
    }

    @Override
    public String getDescription() {
        return "Calibrate threshold values.";
    }

    @Override
    protected void runTest() throws RemoteException {
        boolean supported = getController().getSenseTouch().isSupported();

        if (supported) {
            new Thread(new Runnable() {
                @Override
                public void run() {
                    try {
                        resetUI();

                        float groundstate = getController().getSenseTouch().calibrate(CALIBRATE_TIME,
                                0.05f, 0.0f, new CalibrationCallback() {
                                    @Override
                                    public void onUpdate(final float value, final long remainingTimeMs) {
                                        final String remainingTime = String.format("%.1f", ((remainingTimeMs) / 1000f));

                                        ((Activity) getController().getContext()).runOnUiThread(new Runnable() {
                                            @Override
                                            public void run() {
                                                mCalibrateGroundText.setText(getController().getContext().getString(R.string.calibrating_ground_state)
                                                        + " (" + remainingTime + "s)");
                                            }
                                        });
                                        mForceView.setGroundstate(value);
                                        mForceView.setForce(value);
                                    }

                                    @Override
                                    public void onDone() {
                                        ((Activity) getController().getContext()).runOnUiThread(new Runnable() {
                                            @Override
                                            public void run() {
                                                mCalibrateGroundText.setText(getController().getContext().getString(R.string.calibrating_ground_state)
                                                        + " (DONE)");
                                            }
                                        });
                                    }
                                });

                        getController().vibrate();

                        ((Activity) getController().getContext()).runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                mCalibrateGroundText.setTypeface(null, Typeface.NORMAL);
                                mCalibrateThreshold.setTypeface(null, Typeface.BOLD);
                            }
                        });

                        float threshold = getController().getSenseTouch().calibrate(CALIBRATE_TIME,
                                0.05f, groundstate + 0.05f, new CalibrationCallback() {
                                    @Override
                                    public void onUpdate(final float value, final long remainingTimeMs) {
                                        final String remainingTime = String.format("%.1f", ((remainingTimeMs) / 1000f));

                                        ((Activity) getController().getContext()).runOnUiThread(new Runnable() {
                                            @Override
                                            public void run() {
                                                mCalibrateThreshold.setText(getController().getContext().getString(R.string.calibrating_threshold)
                                                        + " (" + remainingTime + "s)");
                                            }
                                        });
                                        mForceView.setThreshold(value);
                                        mForceView.setForce(value);
                                    }

                                    @Override
                                    public void onDone() {
                                        ((Activity) getController().getContext()).runOnUiThread(new Runnable() {
                                            @Override
                                            public void run() {
                                                mCalibrateThreshold.setText(getController().getContext().getString(R.string.calibrating_threshold)
                                                        + " (DONE)");
                                            }
                                        });
                                    }
                                });

                        boolean finishOk = getController().getSenseTouch().finishCalibration(groundstate, threshold);

                        getLog().reportMessage("values (0.0-1.0): ground: " + groundstate + ", threshold: " + threshold);

                        if (finishOk) {
                            onTestComplete(new SensorTestResult(SensorTestResult.ResultCode.PASS));
                        } else {
                            getLog().reportError("Error finishing calibration.");
                            onTestComplete(new SensorTestResult(SensorTestResult.ResultCode.ERROR));
                        }
                    } catch (CanceledException e) {
                        e.printStackTrace();

                    } catch (Exception e) {
                        e.printStackTrace();
                        onTestComplete(new SensorTestResult(SensorTestResult.ResultCode.ERROR, e.getMessage()));
                    }
                }
            }).start();
        } else {
            onTestComplete(new SensorTestResult(SensorTestResult.ResultCode.NOT_SUPPORTED));
        }
    }

    private void resetUI() {
        mForceView.setThreshold(0.0f);
        mForceView.setGroundstate(0.0f);
        mForceView.setForce(0.0f);

        ((Activity) getController().getContext()).runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mCalibrateGroundText.setTypeface(null, Typeface.BOLD);
                mCalibrateThreshold.setTypeface(null, Typeface.NORMAL);
                mCalibrateGroundText.setText(getController().getContext().getString(R.string.calibrating_ground_state));
                mCalibrateThreshold.setText(getController().getContext().getString(R.string.calibrating_threshold));
            }
        });
    }

    @Override
    public void cancel() {
        try {
            getController().getSenseTouch().cancelCalibration();
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        super.cancel();
    }

    @Override
    public void onTestWillDisplay() {
        super.onTestWillDisplay();
        resetUI();
    }

    @Override
    public View getView() {
        if (mView == null) {
            LayoutInflater li = LayoutInflater.from(getController().getContext());
            mView = li.inflate(R.layout.view_testcase_sensetouch_calibration, null);
            mForceView = (ForceView) mView.findViewById(R.id.force_view);
            mCalibrateGroundText = (TextView) mView.findViewById(R.id.calibrate_ground_text);
            mCalibrateThreshold = (TextView) mView.findViewById(R.id.calibrate_threshold_text);
            return mView;
        }

        return mView;
    }
}
