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
import android.content.DialogInterface;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.RemoteException;
import android.app.AlertDialog;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.fingerprints.imagecollection.R;
import com.fingerprints.imagecollection.interfaces.IConfigurationManager;
import com.fingerprints.imagecollection.interfaces.IImageCollectionTask;
import com.fingerprints.imagecollection.scenario.VerifyConfig;
import com.fingerprints.imagecollection.tasks.ImageCollectionTask;
import com.fingerprints.imagecollection.utils.AndroidUI;
import com.fingerprints.imagecollection.utils.ImageCollectionSession;
import com.fingerprints.imagecollection.utils.Logger;
import com.fingerprints.imagecollection.utils.Utils;
import com.fingerprints.imagecollection.values.Constants;
import com.fingerprints.imagecollection.values.FingerType;
import com.fingerprints.extension.engineering.FingerprintEngineering;
import com.fingerprints.extension.engineering.SensorImage;
import com.fingerprints.extension.engineering.SensorSize;

import tools.xml.GVisitor;

public class ImageCollectionFragment extends Fragment implements IImageCollectionTask {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private static final long CLEAR_MESSAGE_TIMEOUT = 3000;
    private Context mContext;
    private ImageView mImageView;
    private LinearLayout mImageCollectionRunning;
    private RelativeLayout mImageCollectionInit;
    private TextView mMessageText;
    private TextView mProgressText;
    private TextView mInfoTitleText;
    private TextView mInfoFingerText;
    private TextView mExternalConfigurationText;
    private ProgressBar mProgressBar;
    private Button mStartButton;
    private Button mSkipButton;
    private ImageView[] mFingerSelect = new ImageView[Constants.FINGER_COUNT];
    private ImageCollectionTask mImageCollectionTask;
    private Handler mHandler = new Handler(Looper.getMainLooper());
    private IConfigurationManager mConfigurationManager;
    private ImageCollectionSession mSession;
    private ImageCollectionListener mCallback;
    private FingerprintEngineering mFingerprintEngineering;

    private final Runnable mClearMessageRunnable = new Runnable() {
        public void run() {
            mMessageText.setText(null);
        }
    };

    public interface ImageCollectionListener {
        public void onStartImageCollection();

        public void onStopImageCollection();
    }


    public ImageCollectionFragment() {
        mLogger.enter("ImageCollectionFragment");
        mLogger.exit("ImageCollectionFragment");
    }

    @Override
    public void onAttach(final Activity activity) {
        super.onAttach(activity);
        mConfigurationManager = (IConfigurationManager) activity;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        mLogger.enter("onCreateView");
        mContext = getActivity();
        View rootView = inflater.inflate(R.layout.fragment_image_collection, container, false);
        getActivity().setTitle(R.string.image_collection_title);

        mImageView = (ImageView) rootView.findViewById(R.id.image_view);
        mMessageText = (TextView) rootView.findViewById(R.id.message_text);
        mProgressText = (TextView) rootView.findViewById(R.id.progress_text);
        mInfoTitleText = (TextView) rootView.findViewById(R.id.info_title_text);
        mInfoFingerText = (TextView) rootView.findViewById(R.id.info_finger_text);
        mExternalConfigurationText = (TextView) rootView.findViewById(R.id.external_configuration_text);
        mProgressBar = (ProgressBar) rootView.findViewById(R.id.progress_bar);
        mStartButton = (Button) rootView.findViewById(R.id.start_button);
        mImageCollectionRunning = (LinearLayout) rootView.findViewById(R.id.image_collection_running);
        mImageCollectionInit = (RelativeLayout) rootView.findViewById(R.id.image_collection_init);
        mSkipButton = (Button) rootView.findViewById(R.id.skip_button);

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

        mStartButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                getActivity().runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        try {
                            if (startCollection()) {
                                mImageCollectionInit.setVisibility(View.GONE);
                                mImageCollectionRunning.setVisibility(View.VISIBLE);
                            }
                        } catch (Exception e) {
                            mLogger.e("General failure", e);
                            AndroidUI.showDialogAndWait(R.string.generic_error_title, getString(R.string.generic_error_message, e.getMessage()), ImageCollectionFragment.this.getActivity());
                        }
                    }
                });
            }
        });

        mSkipButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(final View v) {
                skip();
            }
        });
        try {
            mFingerprintEngineering = new FingerprintEngineering();
        } catch (Throwable e) {
            mLogger.e("Failed to get FingerprintEngineering", e);
        }

        mImageView.post(new Runnable() {
            public void run() {
                int w, h, temp;
                if (mFingerprintEngineering != null) {
                    SensorSize sensorSize = mFingerprintEngineering.getSensorSize();
                    mLogger.i("sensor size "+ "W: " + sensorSize.mWidth + " H: " + sensorSize.mHeight);
                    if (sensorSize.mHeight > sensorSize.mWidth) {
                        //Do rotation for the biggest display image
                        temp = sensorSize.mWidth;
                        sensorSize.mWidth = sensorSize.mHeight;
                        sensorSize.mHeight = temp;
                        mLogger.i("Rotation "+ "W: " + sensorSize.mWidth + " H: " + sensorSize.mHeight);
                    }
                    //Resize to fit the view layout
                    if (mImageView.getWidth() > mImageView.getHeight()) {
                        temp = mImageView.getHeight() * sensorSize.mWidth / sensorSize.mHeight;
                        w = (temp > mImageView.getWidth())?mImageView.getWidth():temp;
                        h = (temp > mImageView.getWidth())?
                            (mImageView.getWidth() * sensorSize.mHeight / sensorSize.mWidth):
                             mImageView.getHeight();
                    } else {
                        temp = mImageView.getWidth() * sensorSize.mHeight / sensorSize.mWidth;
                        w = (temp > mImageView.getHeight())?
                            (mImageView.getHeight() * sensorSize.mWidth / sensorSize.mHeight):
                            mImageView.getWidth();
                        h = (temp > mImageView.getHeight())?mImageView.getHeight():temp;
                    }
                } else {
                    mLogger.d("Failed to get FingerprintEngineering, using the default layout size");
                    w = mImageView.getWidth();
                    h = mImageView.getHeight();
                }
                mImageView.setLayoutParams(new LinearLayout.LayoutParams(w/2, h/2));
                mImageView.setVisibility(View.INVISIBLE);
                mImageView.invalidate();
                mLogger.i("W: " + w + " H: " + h);
            }
        });

        mImageCollectionRunning.setVisibility(View.GONE);
        mImageCollectionInit.setVisibility(View.VISIBLE);
        mSkipButton.setVisibility(View.GONE);
        mCallback = (ImageCollectionListener) getActivity();

        resetUI();
        mLogger.exit("onCreateView");
        return rootView;
    }

    private void skip() {
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        builder.setTitle(R.string.skip_pressed_alert_title);
        builder.setMessage(R.string.skip_pressed_alert_message);
        builder.setPositiveButton(R.string.skip_pressed_alert_positive,
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        mImageCollectionTask.requestSkip();
                    }
                });
        builder.setNegativeButton(R.string.skip_pressed_alert_negative, null);
        builder.setCancelable(false);
        builder.show();
        mSkipButton.clearFocus(); //if navigation is enabled
    }

    public void abort() {
        mLogger.i("cancel collection");
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        builder.setTitle(R.string.cancel_pressed_alert_title);
        builder.setMessage(R.string.cancel_pressed_alert_message);
        builder.setPositiveButton(R.string.cancel_pressed_alert_positive,
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        mLogger.i("cancel key, stop collection");
                        mImageCollectionInit.setVisibility(View.VISIBLE);
                        mImageCollectionRunning.setVisibility(View.INVISIBLE);
                        if (mImageCollectionTask != null) {
                            mImageCollectionTask.requestStop();
                        }
                    }
                });
        builder.setNegativeButton(R.string.cancel_pressed_alert_negative, null);
        builder.setCancelable(false);
        builder.show();
    }

    @Override
    public void onPause() {
        super.onPause();
        mLogger.enter("onPause");
        if (mImageCollectionTask != null) {
            mImageCollectionTask.onPause();
        }
        mLogger.exit("onPause");
    }

    @Override
    public void onResume() {
        super.onResume();
        mLogger.enter("onResume");
        if (mImageCollectionTask != null) {
            mImageCollectionTask.onResume();
        }
        mLogger.exit("onResume");
    }

    @Override
    public void clearStatus() {
        clearMessage();
        clearFingerprint();
        clearProgress();
    }

    public void onBackPressed() {
        mLogger.enter("onBackPressed");
        if (mImageCollectionTask == null || !mImageCollectionTask.isRunning()) {
            getActivity().finish();
        } else {
            AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
            builder.setTitle(R.string.back_pressed_alert_title);
            builder.setMessage(R.string.back_pressed_alert_message);
            builder.setPositiveButton(R.string.back_pressed_alert_positive,
                    new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            mLogger.i("back key, close application");
                            mImageCollectionTask.requestStop();
                            getActivity().finish();
                        }
                    });
            builder.setNegativeButton(R.string.back_pressed_alert_negative, null);
            builder.setCancelable(false);
            builder.show();
        }
        mLogger.exit("onBackPressed");
    }

    public void clearProgress() {
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mProgressText.setText(null);
                mProgressBar.setProgress(0);
            }
        });
    }

    @Override
    public void onError(final String message) {
        setMessage(message);
    }

    @Override
    public void showNotification(final String message) {
        showMessageAndClear(message);
    }

    @Override
    public void showHighlightNotification(final String message) {
        showHighlightMessageAndClear(message);
    }

    @Override
    public void setTitle(final String title, final FingerType finger) {
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mInfoTitleText.setText(title);
                mInfoFingerText.setText(getString(finger.getNameResourceId()));
            }
        });
    }

    @Override
    public void clearTitle() {
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mInfoTitleText.setText("");
                mInfoFingerText.setText("");
            }
        });
    }

    @Override
    public void setMessage(final String message) {
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mHandler.removeCallbacks(mClearMessageRunnable);
                mMessageText.setText(message);
                mMessageText.setTextSize(15);
                mMessageText.setTextColor(getResources().getColor(android.R.color.holo_orange_dark));
            }
        });
    }

    public void updateProgress(final int max, final int progress, final int rejected) {
        mLogger.enter("updateProgress");
        mLogger.d("task: count: " + progress + " max: " + max);
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                String s = "(" + progress + "/" + max + ")";
                if (rejected > 0) {
                    s += " (rejected: " + rejected + ")";
                }
                mProgressText.setText(s);
                mProgressBar.setProgress((int) (progress / (double) max * 100));
                mLogger.exit("updateProgress");
            }
        });
    }

    public void updateFinger(FingerType fingerType) {
        mLogger.enter("updateFinger");
        if (mImageCollectionTask != null) {
            for (int i = 0; i < Constants.FINGER_COUNT; i++) {
                mFingerSelect[i].setVisibility(View.INVISIBLE);
            }
            mFingerSelect[fingerType.ordinal()].setVisibility(View.VISIBLE);
        }
        mLogger.exit("updateFinger");
    }

    public void clearFingerprint() {
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mImageView.setVisibility(View.INVISIBLE);
            }
        });
    }

    public void updateFingerprint(Bitmap bitmap) {
        if (mSession.getConfig().getImageDisplay()) {
            mImageView.setImageBitmap(bitmap);
            mImageView.setVisibility(View.VISIBLE);
            mImageView.invalidate();
        }
    }

    public void clearMessage() {
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mMessageText.setText(null);
            }
        });

    }

    public void showMessage(final String message) {
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mHandler.removeCallbacks(mClearMessageRunnable);
                mMessageText.setText(message);
                mMessageText.setTextSize(15);
                mMessageText.setTextColor(getResources().getColor(android.R.color.holo_orange_dark));
            }
        });
    }

    public void showMessageAndClear(final String message) {
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mMessageText.setText(message);
                mMessageText.setTextSize(15);
                mMessageText.setTextColor(getResources().getColor(android.R.color.holo_orange_dark));
                mHandler.removeCallbacks(mClearMessageRunnable);
                mHandler.postDelayed(mClearMessageRunnable, CLEAR_MESSAGE_TIMEOUT);
            }
        });
    }

    public void showHighlightMessageAndClear(final String message) {
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mMessageText.setText(message);
                mMessageText.setTextSize(20);
                mMessageText.setTextColor(getResources().getColor(android.R.color.holo_red_light));
                mHandler.removeCallbacks(mClearMessageRunnable);
                mHandler.postDelayed(mClearMessageRunnable, CLEAR_MESSAGE_TIMEOUT);
            }
        });
    }

    private boolean startCollection() throws Exception {
        mLogger.enter("startCollection");

        mSession = new ImageCollectionSession(mConfigurationManager.getConfiguration(), getActivity());

        if (!mSession.getConfig().hasFingers()) {
            mLogger.w("mCurrentFinger == null");
            AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
            builder.setTitle(R.string.select_finger_alert_title);
            builder.setMessage(R.string.select_finger_alert_message);
            builder.setPositiveButton(R.string.select_finger_alert_positive, null);
            builder.setCancelable(false);
            builder.show();
            return false;
        }
        if (!Utils.makeDir(mContext)) {
            return false;
        }
        mCallback.onStartImageCollection();
        if (mImageCollectionTask != null) {
            mImageCollectionTask = null;
        }

        try {
            mImageCollectionTask = new ImageCollectionTask(mContext, this, mSession);
            mImageCollectionTask.start();
            getActivity().getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        } catch (RemoteException e) {
            AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
            builder.setTitle(R.string.closing_application_alert_title);
            builder.setMessage("Could not get Fingerprint Extensions");
            builder.setPositiveButton(R.string.closing_application_alert_positive,
                    new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            getActivity().finish();
                            System.exit(0);
                        }
                    });
            builder.setCancelable(false);
            builder.show();
            return false;
        }
        mLogger.exit("startCollection");
        return true;
    }

    @Override
    public void onStartedEnroll() {
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mSkipButton.setVisibility(View.INVISIBLE);
            }
        });
    }

    @Override
    public void onStartedVerify(final VerifyConfig verifyConfig, final FingerType finger) {
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mSkipButton.setVisibility(verifyConfig.isSkipable() ? View.VISIBLE : View.INVISIBLE);
            }
        });
        setTitle(verifyConfig.getInfo(), finger);
        setMessage(mContext.getResources().getString(R.string.collection_start_message));
    }

    public void onFinishedCollection(boolean isCancel) {
        mLogger.enter("onFinishedCollection");
        getActivity().getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        mCallback.onStopImageCollection();
        resetUI();

        if (!isCancel) {
            AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
            builder.setTitle(R.string.image_collection_completed_alert_title);
            builder.setMessage(R.string.image_collection_completed_alert_message);
            builder.setPositiveButton(R.string.image_collection_completed_alert_positive, null);
            builder.setCancelable(false);
            builder.show();
        }
        mLogger.exit("onFinishedCollection");
    }

    public void resetUI() {
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mLogger.enter("resetUI");
                mImageView.setVisibility(View.INVISIBLE);
                mImageView.setImageResource(0);
                mHandler.removeCallbacks(mClearMessageRunnable);
                mClearMessageRunnable.run();
                mProgressText.setText(null);
                mProgressBar.setProgress(0);
                mInfoTitleText.setText("");
                mInfoFingerText.setText("");
                mSkipButton.setVisibility(View.INVISIBLE);
                mImageCollectionInit.setVisibility(View.VISIBLE);
                mImageCollectionRunning.setVisibility(View.INVISIBLE);
                for (int i = 0; i < Constants.FINGER_COUNT; i++) {
                    mFingerSelect[i].setVisibility(View.INVISIBLE);
                }
                mExternalConfigurationText.setVisibility(mConfigurationManager.hasExternalConfiguration() ? View.VISIBLE : View.GONE);

                mLogger.exit("resetUI");
            }
        });
    }
}
