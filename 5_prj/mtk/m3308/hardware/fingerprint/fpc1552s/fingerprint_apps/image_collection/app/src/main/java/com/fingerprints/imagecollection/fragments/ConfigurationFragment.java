/*
 *
 * Copyright (c) 2016-2021 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */

package com.fingerprints.imagecollection.fragments;

import android.app.Activity;
import android.app.Fragment;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.media.AudioManager;
import android.os.RemoteException;
import android.os.Bundle;
import android.os.Environment;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.NumberPicker;
import android.widget.TextView;

import com.fingerprints.imagecollection.R;
import com.fingerprints.imagecollection.scenario.ImageCollectionConfig;
import com.fingerprints.imagecollection.scenario.VerifyConfig;
import com.fingerprints.imagecollection.utils.Logger;
import com.fingerprints.imagecollection.values.Constants;
import com.fingerprints.imagecollection.values.FingerType;
import com.fingerprints.imagecollection.values.Preferences;
import com.fingerprints.extension.sensortest.FingerprintSensorTest;
import com.fingerprints.extension.sensortest.SensorInfo;

import static com.fingerprints.imagecollection.values.FingerType.L0;
import static com.fingerprints.imagecollection.values.FingerType.L1;
import static com.fingerprints.imagecollection.values.FingerType.L2;
import static com.fingerprints.imagecollection.values.FingerType.L3;
import static com.fingerprints.imagecollection.values.FingerType.L4;
import static com.fingerprints.imagecollection.values.FingerType.R0;
import static com.fingerprints.imagecollection.values.FingerType.R1;
import static com.fingerprints.imagecollection.values.FingerType.R2;
import static com.fingerprints.imagecollection.values.FingerType.R3;
import static com.fingerprints.imagecollection.values.FingerType.R4;

public class ConfigurationFragment extends Fragment implements View.OnTouchListener {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private Context mContext;
    private AudioManager mAudioManager;
    private TextView mStoragePath;
    private TextView mFingerMap;
    private CheckBox mVerifyOrthogonalCheckBox;
    private CheckBox mMatchDuringVerifyCheckBox;
    private NumberPicker mVerifyCount;
    private NumberPicker mVerifyOrthogonalCount;
    private ImageView mFingerInvisibleL;
    private ImageView mFingerInvisibleR;
    private ImageView[] mFingerSelect = new ImageView[Constants.FINGER_COUNT];

    private ImageCollectionConfig mConfig;
    private FingerprintSensorTest mFingerprintSensorTest;
    private static int HardWareId = 0;
    public ConfigurationFragment() {
        mLogger.enter("ConfigurationFragment");
        mLogger.exit("ConfigurationFragment");
    }
    public FingerprintSensorTest getSensorTest() throws RemoteException {
        if (mFingerprintSensorTest == null) {
            mFingerprintSensorTest = new FingerprintSensorTest();
        }

        return mFingerprintSensorTest;
    }

    private void createDefaultConfig() {
        mConfig = new ImageCollectionConfig();
        try {
            mFingerprintSensorTest = getSensorTest();
        } catch (Throwable e) {
            mLogger.e("SensorTest library not found, skipping...");
        }
        VerifyConfig defaultVerifyConfig = new VerifyConfig(0, "", null);
        VerifyConfig verifyOrthogonalConfig = new VerifyConfig(90, "Orthogonal", "Rotate %f 90°");
        SensorInfo mSensorInfo = mFingerprintSensorTest.getSensorInfo();
        HardWareId = mSensorInfo.getHardwareId() & 0xFF00;

        defaultVerifyConfig.setPathExtra("");
        verifyOrthogonalConfig.setPathExtra("Orthogonal");

        defaultVerifyConfig.setSkipable(false);
        verifyOrthogonalConfig.setSkipable(false);
        verifyOrthogonalConfig.setEnabled(false);

        if(Constants.HWID_FPC1540_DEVICE_ID == HardWareId ||
           Constants.HWID_FPC1542_DEVICE_ID == HardWareId ||
           Constants.HWID_FPC1542SA_DEVICE_ID == HardWareId ||
           Constants.HWID_FPC1552_DEVICE_ID == HardWareId ||
           Constants.HWID_FPC1553_DEVICE_ID == HardWareId) {
            mConfig.addFinger(L1, L2, R0, R1, R2);
            verifyOrthogonalConfig.setNumberOfImages(Constants.DEFAULT_VERIFY_COUNT);
            defaultVerifyConfig.setNumberOfImages(Constants.DEFAULT_VERIFY_COUNT);
        }else if (Constants.HWID_FPC1510_DEVICE_ID == HardWareId){
            mConfig.addFinger(L1, L2, R1, R2);
            verifyOrthogonalConfig.setNumberOfImages(Constants.DEFAULT_VERIFY_COUNT);
            defaultVerifyConfig.setNumberOfImages(Constants.DEFAULT_VERIFY_COUNT);
        }else if (Constants.HWID_FPC1290_DEVICE_ID == HardWareId){
            mConfig.addFinger(L1, L2, R1, R2);
            verifyOrthogonalConfig.setNumberOfImages(Constants.DEFAULT_VERIFY_COUNT_FOR_1290);
            defaultVerifyConfig.setNumberOfImages(Constants.DEFAULT_VERIFY_COUNT_FOR_1290);
        } else {
            mConfig.addFinger(L0, L1, L2, L3, R0, R1, R2, R3);
            verifyOrthogonalConfig.setNumberOfImages(Constants.DEFAULT_VERIFY_COUNT_FOR_OLD_SENSORS);
            defaultVerifyConfig.setNumberOfImages(Constants.DEFAULT_VERIFY_COUNT_FOR_OLD_SENSORS);
        }
        mConfig.add(defaultVerifyConfig);
        mConfig.add(verifyOrthogonalConfig);

    }

    @Override
    public void onAttach(final Activity activity) {
        super.onAttach(activity);

        mConfig = Preferences.getConfig(activity);

        if (mConfig == null) {
            createDefaultConfig();
            saveConfig();
        }
    }

    private synchronized void saveConfig() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                mLogger.i("Saving configuration...");
                Preferences.saveConfig(getActivity(), mConfig);
                mLogger.i("Saving configuration complete.");
            }
        }).start();
    }

    public ImageCollectionConfig getConfiguration() {
        return mConfig;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {

        final VerifyConfig verifyConfig = mConfig.getVerifyConfigList().get(0);
        final VerifyConfig verifyOrthogonalConfig = mConfig.getVerifyConfigList().get(1);

        mLogger.enter("onCreateView");
        mContext = getActivity();
        View rootView = inflater.inflate(R.layout.fragment_configuration, container, false);
        mAudioManager = (AudioManager) mContext.getSystemService(Context.AUDIO_SERVICE);

        mStoragePath = (TextView) rootView.findViewById(R.id.storage_path);
        String storagePath = mContext.getExternalFilesDir(
                Environment.DIRECTORY_PICTURES).getPath() + "/" + Constants.FPC_FOLDER;
        mStoragePath.setText(storagePath);

        mMatchDuringVerifyCheckBox = (CheckBox) rootView.findViewById(R.id.decisionFeedback);
        mMatchDuringVerifyCheckBox.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(final View v) {
                mConfig.setDecisionFeedback(mMatchDuringVerifyCheckBox.isChecked());
            }
        });
        mMatchDuringVerifyCheckBox.setChecked(mConfig.getDecisionFeedback());

        //VerifyConfig
        mVerifyOrthogonalCheckBox = (CheckBox) rootView.findViewById(R.id.verify_orthogonal);
        mVerifyOrthogonalCheckBox.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                verifyOrthogonalConfig.setEnabled(mVerifyOrthogonalCheckBox.isChecked());
                mVerifyOrthogonalCount.setEnabled(mVerifyOrthogonalCheckBox.isChecked());
            }
        });
        mVerifyOrthogonalCheckBox.setChecked(verifyOrthogonalConfig.isEnabled());

        mVerifyCount = (NumberPicker) rootView.findViewById(R.id.verify_count);
        mVerifyCount.setMinValue(1);
        mVerifyCount.setMaxValue(Constants.MAX_VERIFY_COUNT);
        mVerifyCount.setValue(verifyConfig.getNumberOfImages());
        mVerifyCount.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
            @Override
            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
                verifyConfig.setNumberOfImages(newVal);
            }
        });

        mVerifyOrthogonalCount = (NumberPicker) rootView.findViewById(R.id.verify_orthogonal_count);
        mVerifyOrthogonalCount.setMinValue(1);
        mVerifyOrthogonalCount.setMaxValue(Constants.MAX_VERIFY_COUNT);
        mVerifyOrthogonalCount.setValue(verifyOrthogonalConfig.getNumberOfImages());
        mVerifyOrthogonalCount.setEnabled(verifyOrthogonalConfig.isEnabled());
        mVerifyOrthogonalCount.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
            @Override
            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
                verifyOrthogonalConfig.setNumberOfImages(newVal);
            }
        });

        //FingerMap
        mFingerMap = (TextView) rootView.findViewById(R.id.finger_map);
        mFingerInvisibleL = (ImageView) rootView.findViewById(R.id.finger_invisible_l);
        mFingerInvisibleR = (ImageView) rootView.findViewById(R.id.finger_invisible_r);
        mFingerSelect[0] = (ImageView) rootView.findViewById(R.id.finger_select_l5);
        mFingerSelect[1] = (ImageView) rootView.findViewById(R.id.finger_select_l4);
        mFingerSelect[2] = (ImageView) rootView.findViewById(R.id.finger_select_l3);
        mFingerSelect[3] = (ImageView) rootView.findViewById(R.id.finger_select_l2);
        mFingerSelect[4] = (ImageView) rootView.findViewById(R.id.finger_select_l1);
        mFingerSelect[5] = (ImageView) rootView.findViewById(R.id.finger_select_r1);
        mFingerSelect[6] = (ImageView) rootView.findViewById(R.id.finger_select_r2);
        mFingerSelect[7] = (ImageView) rootView.findViewById(R.id.finger_select_r3);
        mFingerSelect[8] = (ImageView) rootView.findViewById(R.id.finger_select_r4);
        mFingerSelect[9] = (ImageView) rootView.findViewById(R.id.finger_select_r5);

        mFingerInvisibleL.setOnTouchListener(this);
        mFingerInvisibleR.setOnTouchListener(this);
        for (int i = 0; i < Constants.FINGER_COUNT; i++) {
            if (mConfig.hasFinger(FingerType.getFingerType(i))) {
                mFingerSelect[i].setVisibility(View.VISIBLE);
            } else {
                mFingerSelect[i].setVisibility(View.INVISIBLE);
            }
        }

        updateFingerMap();
        mLogger.exit("onCreateView");
        return rootView;
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        mLogger.enter("onTouch");
        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                int x = (int) event.getX();
                int y = (int) event.getY();
                if (mFingerInvisibleL.getId() == v.getId()) {
                    if (isMatchColor(Color.RED, getColor(mFingerInvisibleL, x, y))) {
                        onFingerSelect(L4);
                    } else if (isMatchColor(Color.GREEN, getColor(mFingerInvisibleL, x, y))) {
                        onFingerSelect(L3);
                    } else if (isMatchColor(Color.BLUE, getColor(mFingerInvisibleL, x, y))) {
                        onFingerSelect(L2);
                    } else if (isMatchColor(Color.WHITE, getColor(mFingerInvisibleL, x, y))) {
                        onFingerSelect(L1);
                    } else if (isMatchColor(Color.YELLOW, getColor(mFingerInvisibleL, x, y))) {
                        onFingerSelect(L0);
                    }
                    return true;
                } else if (mFingerInvisibleR.getId() == v.getId()) {
                    if (isMatchColor(Color.RED, getColor(mFingerInvisibleR, x, y))) {
                        onFingerSelect(R4);
                    } else if (isMatchColor(Color.GREEN, getColor(mFingerInvisibleR, x, y))) {
                        onFingerSelect(R3);
                    } else if (isMatchColor(Color.BLUE, getColor(mFingerInvisibleR, x, y))) {
                        onFingerSelect(R2);
                    } else if (isMatchColor(Color.WHITE, getColor(mFingerInvisibleR, x, y))) {
                        onFingerSelect(R1);
                    } else if (isMatchColor(Color.YELLOW, getColor(mFingerInvisibleR, x, y))) {
                        onFingerSelect(R0);
                    }
                    return true;
                }
                break;
        }
        mLogger.exit("onTouch");
        return false;
    }

    private void onFingerSelect(FingerType fingerType) {
        mAudioManager.playSoundEffect(AudioManager.FX_KEY_CLICK);

        if (mConfig.hasFinger(fingerType)) {
            mFingerSelect[fingerType.ordinal()].setVisibility(View.INVISIBLE);
            mConfig.removeFinger(fingerType);
        } else {
            mFingerSelect[fingerType.ordinal()].setVisibility(View.VISIBLE);
            mConfig.addFinger(fingerType);
        }

        updateFingerMap();
    }

    private int getColor(ImageView imageView, int x, int y) {
        imageView.setDrawingCacheEnabled(true);
        Bitmap bitmap = Bitmap.createBitmap(imageView.getDrawingCache());
        imageView.setDrawingCacheEnabled(false);
        return bitmap.getPixel(x, y);
    }

    private boolean isMatchColor(int c1, int c2) {
        int tolerance = 25;
        if ((int) Math.abs(Color.red(c1) - Color.red(c2)) > tolerance) {
            return false;
        }
        if ((int) Math.abs(Color.green(c1) - Color.green(c2)) > tolerance) {
            return false;
        }
        if ((int) Math.abs(Color.blue(c1) - Color.blue(c2)) > tolerance) {
            return false;
        }
        return true;
    }

    private void updateFingerMap() {
        StringBuilder sb = new StringBuilder();
        for (FingerType fingerType : FingerType.values()) {
            if (mConfig.hasFinger(fingerType)) {
                if (!sb.toString().equals("")) {
                    sb.append(" | ");
                    sb.append(fingerType.getId());
                } else {
                    sb.append(fingerType.getId());
                }
            }
        }
        mFingerMap.setText(sb.toString());
    }

    public void setInteractive(boolean enabled) {
        mVerifyOrthogonalCheckBox.setEnabled(enabled);
        mVerifyCount.setEnabled(enabled);
        mVerifyOrthogonalCount.setEnabled(mVerifyOrthogonalCheckBox.isChecked());
        mFingerInvisibleL.setEnabled(enabled);
        mFingerInvisibleR.setEnabled(enabled);
    }

    public void onClosed() {
        saveConfig();
    }
}
