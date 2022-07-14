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

package com.fingerprints.imagecollection.tasks;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.Bitmap;
import android.view.WindowManager;
import android.os.Handler;
import android.os.Looper;
import android.app.AlertDialog;
import android.support.annotation.NonNull;

import com.fingerprints.extension.util.FpcError;
import com.fingerprints.imagecollection.R;
import com.fingerprints.imagecollection.exceptions.OperationAbortedException;
import com.fingerprints.imagecollection.exceptions.OperationSkippedException;
import com.fingerprints.imagecollection.imagecapture.CaptureTool;
import com.fingerprints.imagecollection.imagecapture.ImageWriter;
import com.fingerprints.imagecollection.imageutils.Enroll;
import com.fingerprints.imagecollection.imageutils.IImageToolCallback;
import com.fingerprints.imagecollection.imageutils.ImageData;
import com.fingerprints.imagecollection.imageutils.VerifyStatistics;
import com.fingerprints.imagecollection.interfaces.IImageCollectionTask;
import com.fingerprints.imagecollection.scenario.VerifyConfig;
import com.fingerprints.imagecollection.utils.ImageCollectionSession;
import com.fingerprints.imagecollection.utils.Logger;
import com.fingerprints.imagecollection.utils.ArrayUtils;
import com.fingerprints.imagecollection.values.Constants;
import com.fingerprints.imagecollection.values.FingerType;
import com.fingerprints.imagecollection.values.Preferences;

import java.lang.reflect.Method;
import java.util.ArrayList;

public class ImageCollectionTask extends Thread {
    private enum State {
        STOPPED,
        RUNNING,
        PAUSED;

        public boolean isStopped() {
            return this == STOPPED;
        }

        public boolean isPaused() {
            return this == PAUSED;
        }
    }

    private State mState = State.STOPPED;
    private Object mStateSync = new Object();
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private Context mContext;
    private IImageCollectionTask mController;
    private Handler mMainThreadHandler;
    private CaptureTool mCaptureTool;
    private ImageCollectionSession mSession;
    private static final long ONRESUME_DELAY_MS = 600;

    private final Runnable mOnResumeDelayRunnable = new Runnable() {
        public void run() {
            mLogger.d("mOnResumeDelayRunnable.run()");
            setState(State.RUNNING);
            mCaptureTool.onResume();
        }
    };

    private ImageWriter.IImageWriterProgressCallback mIImageWriterProgressCallback = new ImageWriter.IImageWriterProgressCallback() {

        private ProgressDialog mProgressDialog;

        @Override
        public void beganWriting(int max) {
            if (max > 1) {
                mMainThreadHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        mProgressDialog = new ProgressDialog(mContext);
                        mProgressDialog.setTitle(getString(R.string.image_saving_alert_title));
                        mProgressDialog.setMessage(getString(R.string.image_saving_alert_message));
                        mProgressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
                        mProgressDialog.setCancelable(false);
                        mProgressDialog.show();
                    }
                });
            }
        }

        @Override
        public void doneWriting() {
            mMainThreadHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mProgressDialog != null) {
                        mProgressDialog.dismiss();
                        mProgressDialog = null;
                    }
                }
            });
        }

        @Override
        public void writingImage(final int current, final int max) {
            if (mProgressDialog != null) {
                mMainThreadHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (mProgressDialog != null) {
                            mProgressDialog.setMax(max);
                            mProgressDialog.setProgress(current);
                        }
                    }
                });
            }
        }
    };

    public ImageCollectionTask(final Context context, final IImageCollectionTask controller, final ImageCollectionSession session) throws
            Exception {
        mLogger.enter("ImageCollectionTask");
        mContext = context;
        mMainThreadHandler = new Handler(mContext.getMainLooper());
        mController = controller;
        mSession = session;

        mCaptureTool = new CaptureTool(mContext, new IImageToolCallback() {
            @Override
            public void onError(final String message) {
                mController.onError(message);
            }

            @Override
            public void onReset() {
                clearMessage();
                clearFingerprint();
            }

            @Override
            public void onMessage(final String message) {
                mController.setMessage(message);
            }

            @Override
            public void onNotify(final String s) {
                mController.showNotification(s);
            }

            @Override
            public void onNotifyAndHighlight(final String s) {
                mController.showHighlightNotification(s);
            }

            @Override
            public void clearMessage() {
                mController.clearMessage();
            }

            @Override
            public void updateEnrollProgress(final int max, final int progress, final int rejected) {
                mController.updateProgress(max, progress, rejected);
            }

            @Override
            public void clearProgress() {
                mMainThreadHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        mController.clearProgress();
                    }
                });
            }

            @Override
            public void onImage(final Bitmap bitmap) {
                mMainThreadHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        mController.updateFingerprint(bitmap);
                    }
                });
            }
        });

        mLogger.exit("ImageCollectionTask");
    }

    public boolean isStopped() {
        return mState.isStopped();
    }

    public boolean isRunning() {
        return mCaptureTool.isRunning();
    }

    public boolean isPaused() {
        return mState.isPaused();
    }

    private String getString(int resId, Object... objs) {
        return mContext.getResources().getString(resId, objs);
    }

    public void setState(State state) {
        synchronized (mStateSync) {
            mState = state;
            mStateSync.notifyAll();
        }
    }

    public Enroll enroll(final FingerType fingerType) throws Exception, OperationAbortedException {
        Enroll enroll = new Enroll();

        mController.onStartedEnroll();

        for (int i = 0; i < Constants.MAX_ENROLL_RETRY; i++) {
            clearProgress();
            mLogger.i("Enroll");
            mController.setTitle("Enroll", fingerType);

            enroll = mCaptureTool.doEnroll(fingerType, enroll);

            mLogger.i("Enroll result " + enroll.getLastSession().getStatus());

            clearFingerprint();

            waitForApplicationRunning();

            boolean lastTry = (i == Constants.MAX_ENROLL_RETRY - 1);

            if (enroll.getLastSession().getStatus().isComplete() || lastTry) {
                break;
            } else {
                showDialogAndWait(R.string.enroll_alert_title,
                        R.string.enroll_alert_failed_message,
                        R.string.enroll_alert_positive);
            }
        }

        if (enroll.getLastSession().getStatus().isComplete()) {
            showDialogAndWait(R.string.enroll_alert_title,
                    R.string.enroll_alert_success_message,
                    R.string.enroll_alert_positive);
        } else {
            showDialogAndWait(R.string.enroll_alert_title,
                    getString(R.string.enroll_alert_max_retry_message,
                            "" + Constants.MAX_ENROLL_RETRY), R.string.enroll_alert_positive);
        }

        return enroll;
    }

    private void waitForApplicationRunning() {
        synchronized (mStateSync) {
            while (mState.isPaused()) {
                try {
                    mStateSync.wait();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private VerifyStatistics verify(final Enroll enroll, final VerifyConfig verifyConfig, final FingerType fingerType) throws OperationAbortedException, OperationSkippedException, Exception {
        clearFingerprint();
        clearProgress();

        if (verifyConfig.getDescription() != null) {
            showDialogAndWait(verifyConfig.getInfo(), verifyConfig.getFormattedDescription(fingerType, mContext), "Ok");
        }

        mController.onStartedVerify(verifyConfig, fingerType);

        VerifyStatistics verifyStatistics = new VerifyStatistics(mSession.getConfig().getDecisionFeedback());
        int cacFailCount = 0;

        for (int i = 0; i < verifyConfig.getNumberOfImages(); i++) {
            ImageData imageData = null;

            do {
                waitForApplicationRunning();

                if (verifyStatistics.hasDecisionFeedback()) {
                    if (enroll == null) {
                        throw new Exception("Missing enroll data");
                    }
                    imageData = mCaptureTool.doVerify(enroll, fingerType);
                } else {
                    imageData = mCaptureTool.doCapture(fingerType);
                }
                if (imageData.getImageCaptureResult().getCacResult().isFailure()) {
                    cacFailCount++;
                    mLogger.i("Capture failed with CAC error code: " + imageData.getImageCaptureResult().getCacResult());
                }

                if (imageData.getCaptureStatus().isCancelled()) {
                    setState(State.PAUSED);
                }

            } while (imageData.getCaptureStatus().isError());

            imageData.setSampleId(i);
            imageData.setVerifyConfig(verifyConfig);
            ImageWriter.getInstance().write(imageData, mContext, mIImageWriterProgressCallback, mSession.getTaskLog(), mCaptureTool.getBuildInfo());
            updateProgress(verifyConfig.getNumberOfImages(), i + 1, 0);
            verifyStatistics.addData(imageData);
        }

        mLogger.i("FTA=" + cacFailCount + "/" + (verifyConfig.getNumberOfImages() + cacFailCount));
        mSession.getTaskLog().log("CACCalls", verifyConfig.getNumberOfImages() + cacFailCount);
        mSession.getTaskLog().log("CACFailures", cacFailCount);
        mSession.getTaskLog().log("CACSuccesses", verifyConfig.getNumberOfImages());
        return verifyStatistics;
    }

    private void setActiveFingerAndWaitForOk(final FingerType fingerType) {
        mLogger.i("fingerType: " + fingerType.getId() + " " + true);
        mMainThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                mController.clearFingerprint();
                mController.clearProgress();
                mController.updateFinger(fingerType);
            }
        });

        showDialogAndWait(getString(R.string.change_finger_alert_title),
                getString(R.string.change_finger_alert_message, getString(fingerType.getNameResourceId())),
                getString(R.string.change_finger_alert_positive));
    }

    private void runConfiguration() throws OperationAbortedException, Exception {
        for (final FingerType fingerType : FingerType.values()) {
            if (mSession.getConfig().hasFinger(fingerType)) {

                mSession.getTaskLog().open("Collection").log("fingerType", fingerType);

                try {
                    setActiveFingerAndWaitForOk(fingerType);

                    if (!isCaptureModeEnabled()) {

                        mSession.getTaskLog().open("Enroll");

                        Enroll enroll = enroll(fingerType);

                        ImageWriter.getInstance().write(enroll, mContext, mIImageWriterProgressCallback, mSession.getTaskLog(), mCaptureTool.getBuildInfo());

                        mSession.getTaskLog().log("status", enroll.getLastSession().getStatus());
                        mSession.getTaskLog().close();

                        if (enroll.getLastSession().getStatus().isComplete()) {
                            Preferences.addAppErolledFingerprint(enroll.getFingerId(), mContext);

                            for (VerifyConfig verifyConfig : mSession.getConfig().getVerifyConfigList()) {
                                if (verifyConfig.isEnabled()) {
                                    try {
                                        mSession.startVerifySession(verifyConfig);

                                        VerifyStatistics verifyStatistics = verify(enroll, verifyConfig, fingerType);

                                        mSession.endVerifySession(verifyStatistics);
                                    } catch (OperationSkippedException e) {
                                        mSession.getTaskLog().add("Skipped");
                                        //ignore and continue to next.
                                    } catch (Exception e) {
                                        throw e;
                                    } finally {
                                        mSession.getTaskLog().close();
                                    }
                                }
                            }
                        } else if (enroll.getLastSession().getStatus().isCancelled()) {
                            mLogger.d("runConfiguration: break;");
                            break;
                        }
                    } else {
                        //verify only
                        for (VerifyConfig verifyConfig : mSession.getConfig().getVerifyConfigList()) {
                            if (verifyConfig.isEnabled()) {
                                try {
                                    mSession.getTaskLog().open("VerifyConfig");
                                    verify(null, verifyConfig, fingerType);
                                } catch (OperationSkippedException e) {
                                    mSession.getTaskLog().add("Skipped");
                                    //ignore and continue to next.
                                } catch (Exception e) {
                                    throw e;
                                } finally {
                                    mSession.getTaskLog().close();
                                }
                            }
                        }
                    }

                } catch (Exception e) {
                    throw e;
                } finally {
                    mSession.getTaskLog().close("Collection");
                }
            }
        }
    }

    @Override
    public void run() {
        mLogger.enter("run");

        setState(State.RUNNING);

        mSession.start();
        mCaptureTool.start();

        try {
            runConfiguration();

            if (isStopped()){
                onCanceled();
            }
            else {
                onComplete();
            }
        } catch (OperationAbortedException e) {
            mSession.getTaskLog().add("Aborted");
            onCanceled();
        } catch (Exception e) {
            mSession.getTaskLog().add("Error").log("message", e.getMessage()).log("type", e.getClass().getSimpleName());
            e.printStackTrace();
            showDialogAndWait(R.string.generic_error_title, getString(R.string.generic_error_message, e.getMessage()));
            onCanceled();
        } finally {
            mCaptureTool.shutdown();
        }

        mSession.getTaskLog().close();
        mSession.end();

        setState(State.STOPPED);

        mLogger.exit("run");
    }

    public void onComplete() {
        mController.clearMessage();
        mMainThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                mController.onFinishedCollection(false);
            }
        });
    }

    public void onCanceled() {
        mController.clearMessage();
        mMainThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                mController.onFinishedCollection(true);
            }
        });
    }

    public void onPause() {
        mLogger.enter("onPause");
        setState(State.PAUSED);
        mMainThreadHandler.removeCallbacks(mOnResumeDelayRunnable);
        mCaptureTool.onPause();
        mLogger.exit("onPause");
    }

    public void onResume() {
        mLogger.enter("onResume");
        /*
         * Workaround solution for SWISLAY-1577.
         * The postDelayed is added here to avoid terminating Android authentication.
         * Otherwise, the finger listener will be turned off, which causes the ICTA
         * or authenticator becomes malfunction.
         */
        mMainThreadHandler.removeCallbacks(mOnResumeDelayRunnable);
        mMainThreadHandler.postDelayed(mOnResumeDelayRunnable, ONRESUME_DELAY_MS);
        mLogger.exit("onResume");
    }

    public void requestStop() {
        mLogger.enter("requestStop");
        mCaptureTool.shutdown();
        mLogger.exit("requestStop");
    }

    public void requestSkip() {
        mLogger.enter("requestSkip");
        mCaptureTool.skip();
        mLogger.exit("requestSkip");
    }

    private void updateProgress(final int max, final int progress, final int rejected) {
        mMainThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                mController.updateProgress(max, progress, rejected);
            }
        });
    }

    private void clearFingerprint() {
        mMainThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                mController.clearFingerprint();
            }
        });
    }

    private void clearProgress() {
        mMainThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                mController.clearProgress();
            }
        });
    }

    private void showDialogAndWait(final int titleId, final String message, final int buttonTextId) {
        showDialogAndWait(getString(titleId), message, getString(buttonTextId));
    }

    private void showDialogAndWait(final int titleId, final int messageId, final int buttonTextId) {
        showDialogAndWait(getString(titleId), getString(messageId), getString(buttonTextId));
    }

    private void showDialogAndWait(final int titleId, final int messageId) {
        showDialogAndWait(getString(titleId), getString(messageId), getString(R.string.ok));
    }

    private void showDialogAndWait(final int titleId, final String messageId) {
        showDialogAndWait(getString(titleId), messageId, getString(R.string.ok));
    }

    private void showDialogAndWait(final String title, final String message, final String buttonText) {

        boolean isOnUiThread = Thread.currentThread() == Looper.getMainLooper().getThread();

        if (isOnUiThread) {
            throw new RuntimeException("Should not be called from the UI thread");
        }

        final Object waitSync = new Object();

        synchronized (waitSync) {
            mMainThreadHandler.post(new Runnable() {
                @Override
                public void run() {
                    AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
                    builder.setTitle(title);
                    builder.setMessage(message);
                    builder.setPositiveButton(buttonText,
                            new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    synchronized (waitSync) {
                                        waitSync.notifyAll();
                                    }
                                }
                            });
                    final AlertDialog dialog = builder.create();
                    dialog.setCancelable(false);
                    dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY);
                    dialog.show();
                }
            });

            try {
                //wait until user pressed ok.
                waitSync.wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

            mCaptureTool.onResume();
        }
    }

    private boolean isCaptureModeEnabled() {
        try {
            final Class systemProperties = Class.forName("android.os.SystemProperties");
            if (systemProperties != null) {
                final Method getBoolean = systemProperties.getMethod("getBoolean", String.class,
                        boolean.class);
                if (getBoolean != null) {
                    return (boolean) getBoolean.invoke(null, Constants.PROPERTY_FINGERPRINTS_CAPTURE_ENABLED, false);
                }
            }
        } catch (Exception ignore) {
        }
        return false;
    }
}
