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

package com.fingerprints.imagetool.activities;

import android.app.Dialog;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.Typeface;
import android.graphics.drawable.ColorDrawable;
import android.os.Bundle;
import android.os.Vibrator;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.view.KeyEvent;
import android.widget.Toast;
import android.os.RemoteException;
import android.content.Intent;

import com.fingerprints.extension.engineering.FingerprintEngineering;
import com.fingerprints.extension.engineering.SensorImage;
import com.fingerprints.extension.engineering.SensorSize;
import com.fingerprints.imagetool.R;
import com.fingerprints.imagetool.utils.Logger;
import com.fingerprints.imagetool.utils.SensorImageBitmap;
import com.fingerprints.extension.util.FpcConstants;
public class MainActivity extends DisabledNavigationActivity {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private static final long VIBRATE_MS_IMAGE = 50;
    private static final long STARTCAPTURE_MS = 600;
    private TextView mMessageView;
    private ImageView mSensorImageView;
    private SensorImageBitmap mSensorImageBitmap;
    private FingerprintEngineering mFingerprintEngineering;
    private Vibrator mVibrator;
    private boolean isCanceled = true;
    private Handler mHandler = new Handler(Looper.getMainLooper());

    private final Runnable mStartCaptureImageRunnable = new Runnable() {
        public void run() {
            if (mFingerprintEngineering != null) {
                mLogger.i("start image capturing...");
                startFingerListener();
                startcaptureimage();
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(MainActivity.this, "Sensor is Ready", Toast.LENGTH_SHORT).show();
                    }
                });
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mLogger.enter("onCreate");
        setContentView(R.layout.activity_main);

        try {
            mFingerprintEngineering = new FingerprintEngineering();
        } catch (Throwable e) {
            showClosingAppDialogAndExit();
        }

        TextView appNameView = (TextView) findViewById(R.id.app_name_text);
        appNameView.setTypeface(Typeface.createFromAsset(getAssets(), "fonts/DINLight.ttf"));

        mSensorImageView = (ImageView) findViewById(R.id.sensor_image);
        mSensorImageView.post(new Runnable() {
            public void run() {
                if (mFingerprintEngineering != null) {
                    int w, h, temp;
                    SensorSize sensorSize = mFingerprintEngineering.getSensorSize();
                    mLogger.d("sensor size "+ "W: " + sensorSize.mWidth + " H: " + sensorSize.mHeight);
                    if (sensorSize.mHeight > sensorSize.mWidth) {
                        //Do rotation for the biggest display image
                        temp = sensorSize.mWidth;
                        sensorSize.mWidth = sensorSize.mHeight;
                        sensorSize.mHeight = temp;
                        mLogger.d("Rotation "+ "W: " + sensorSize.mWidth + " H: " + sensorSize.mHeight);
                    }

                    //Resize to fit the view layout
                    mLogger.d("Imageview "+ "W: " + mSensorImageView.getWidth() + " H: " + mSensorImageView.getHeight());
                    if (mSensorImageView.getWidth() > mSensorImageView.getHeight()) {
                        temp = mSensorImageView.getHeight() * sensorSize.mWidth / sensorSize.mHeight;
                        w = (temp > mSensorImageView.getWidth())?mSensorImageView.getWidth():temp;
                        h = (temp > mSensorImageView.getWidth())?
                            (mSensorImageView.getWidth() * sensorSize.mHeight / sensorSize.mWidth):
                             mSensorImageView.getHeight();
                    } else {
                        temp = mSensorImageView.getWidth() * sensorSize.mHeight / sensorSize.mWidth;
                        w = (temp > mSensorImageView.getHeight())?
                            (mSensorImageView.getHeight() * sensorSize.mWidth / sensorSize.mHeight):
                            mSensorImageView.getWidth();
                        h = (temp > mSensorImageView.getHeight())?mSensorImageView.getHeight():temp;
                    }

                    mSensorImageView.setLayoutParams(new LinearLayout.LayoutParams(w/2, h/2));
                    mSensorImageView.setVisibility(View.VISIBLE);
                    mSensorImageView.invalidate();
                    mLogger.d("W: " + w + " H: " + h);
                }
            }
        });

        mMessageView = (TextView) findViewById(R.id.message_text);
        mMessageView.setTypeface(Typeface.createFromAsset(getAssets(), "fonts/DINLight.ttf"));

        mVibrator = (Vibrator) getSystemService(VIBRATOR_SERVICE);
        mLogger.exit("onCreate");
    }

    public void onAcquired(int acquiredInfo) {
        mLogger.enter("onAcquired");
        mLogger.i("onAcquired acquiredInfo: " + acquiredInfo);
        mLogger.exit("onAcquired");
    }

    public void onError(int error) {
        mLogger.enter("onError");
        mLogger.i("onError error: " + error);
        mLogger.exit("onError");
    }

    /*  We use onPause to determine when the application is not open to the user. */
    @Override
    public void onPause() {
        super.onPause();
        mLogger.enter("onPause");
        if (mFingerprintEngineering != null) {
            mHandler.removeCallbacks(mStartCaptureImageRunnable);
            if (isCanceled == false) {
                mLogger.i("Stop image capturing...");
                mFingerprintEngineering.cancelCapture();
                stopFingerListener();
            } else {
                mLogger.d("Image capture is already canceled");
            }
        }

        mLogger.exit("onPause");
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            // clear the cache buffer manually from android s
            mSensorImageView.setBackgroundResource(R.drawable.sensor_image_background);
            mSensorImageView.setImageResource(R.drawable.ic_icon_sensor_print);
            mSensorImageView.setScaleType(ImageView.ScaleType.CENTER);
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public void onResume() {
        super.onResume();
        mLogger.enter("onResume");
        mHandler.removeCallbacks(mStartCaptureImageRunnable);
        mHandler.postDelayed(mStartCaptureImageRunnable, STARTCAPTURE_MS);
        mLogger.exit("onResume");
    }
    private void displayImage(final SensorImage image) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mLogger.enter("displayImage");
                mSensorImageBitmap = new SensorImageBitmap(image);
                Bitmap bitmap = mSensorImageBitmap.getBitmap();
                mSensorImageView.setImageBitmap(bitmap);
                mSensorImageView.setScaleType(ImageView.ScaleType.FIT_XY);
                mSensorImageView.invalidate();
                mLogger.exit("displayImage");
            }
        });
    }

    private void startFingerListener() {
        mLogger.enter("startFingerListener");
        sendBroadcast(new Intent(FpcConstants.ACTION_FPC_START_FINGER_LISTENER));
        mLogger.exit("startFingerListener");
    }

    private void stopFingerListener() {
        mLogger.enter("stopFingerListener");
        sendBroadcast(new Intent(FpcConstants.ACTION_FPC_STOP_FINGER_LISTENER));
        mLogger.exit("stopFingerListener");
    }

    private void startcaptureimage() {
        mLogger.enter("startcaptureimage");
        if ((mFingerprintEngineering != null) && (isCanceled == true)) {
            mLogger.i("Start capturing");
            isCanceled = false;
            mFingerprintEngineering.startCapture(new FingerprintEngineering.Callback<FingerprintEngineering.CaptureData>() {
                @Override
                public void onResult(final FingerprintEngineering.CaptureData captureData) {
                    mLogger.enter("onResult");
                    if (captureData.isImageCaptured()) {
                        mVibrator.vibrate(VIBRATE_MS_IMAGE);
                        displayImage(captureData.getEnhancedImage());
                        mFingerprintEngineering.startCapture(this);
                    } else {
                        mHandler.removeCallbacks(mStartCaptureImageRunnable);
                        switch (captureData.getError().getExternalEnum()) {
                            case FPC_ERROR_CANCELLED:
                                isCanceled = true;
                                mLogger.i("Cancel Capture");
                                break;
                            default:
                                final String description = captureData.getError().describe();
                                mLogger.i("Error Capture:" + description);
                                mFingerprintEngineering.startCapture(this);
                                runOnUiThread(new Runnable() {
                                    @Override
                                    public void run() {
                                        Toast.makeText(MainActivity.this, "No image captured from sensor (" + description + ").", Toast.LENGTH_SHORT).show();
                                   }
                               });
                               break;
                            }
                    }
                    mLogger.exit("onResult");
                }
            });
        }
        mLogger.exit("startcaptureimage");
    }
    public void showClosingAppDialogAndExit() {
        final Dialog dialog = new Dialog(this);
        dialog.setContentView(R.layout.dialog_closing_app);
        dialog.getWindow().setBackgroundDrawable(new ColorDrawable(Color.TRANSPARENT));
        dialog.setCancelable(false);

        TextView titleText = (TextView) dialog.findViewById(R.id.title_text);
        titleText.setTypeface(Typeface.createFromAsset(getAssets(), "fonts/DINLight.ttf"));
        TextView messageText = (TextView) dialog.findViewById(R.id.message_text);
        messageText.setTypeface(Typeface.createFromAsset(getAssets(), "fonts/DINLight.ttf"));
        Button dialogButton = (Button) dialog.findViewById(R.id.ok_button);
        dialogButton.setTypeface(Typeface.createFromAsset(getAssets(), "fonts/DINLight.ttf"));

        dialogButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                dialog.dismiss();
                System.exit(0);
            }
        });

        dialog.show();
    }
}
