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
package com.fingerprints.imagecollection.imagecapture;

import android.content.Context;
import android.os.Vibrator;

import com.fingerprints.extension.engineering.FingerprintEngineering;
import com.fingerprints.extension.util.FpcError;
import com.fingerprints.imagecollection.R;
import com.fingerprints.imagecollection.exceptions.OperationAbortedException;
import com.fingerprints.imagecollection.exceptions.OperationSkippedException;
import com.fingerprints.imagecollection.imageutils.Enroll;
import com.fingerprints.imagecollection.imageutils.IImageToolCallback;
import com.fingerprints.imagecollection.imageutils.ImageData;
import com.fingerprints.imagecollection.utils.Logger;
import com.fingerprints.imagecollection.utils.Utils;
import com.fingerprints.imagecollection.values.FingerType;
import com.fingerprints.imagecollection.values.ImageCollectionState;

/**
 * Tool to synchronize between Google Enroll Apis and Fingerprints Image subscription and provide
 * an easy to use interface for enrollment and image capture.
 */
public class CaptureTool {

    private enum ShutdownFlag {
        NONE, SKIP, SHUTDOWN;

        public boolean isAborted() {
            return this == SHUTDOWN;
        }

        public boolean isSkipped() {
            return this == SKIP;
        }
    }


    private static final int IMAGE_CAPTURE_CANCEL_ERRORCODE = 5;
    private static final long VIBRATE_MS_CAPTURE = 50;
    private FingerprintEngineering mFingerprintEngineering;
    private Logger mLogger;
    private Vibrator mVibrator;
    private Context mContext;
    private ImageCollectionState mImageCollectionState;
    private ShutdownFlag mShutdownFlag = ShutdownFlag.NONE;
    private Object mTaskSync = new Object();
    private IImageToolCallback mCallback;
    private String buildInfo;

    public CaptureTool(final Context context, final IImageToolCallback callback) throws
            Exception {
        mContext = context;
        mLogger = new Logger(getClass().getSimpleName());
        mVibrator = (Vibrator) mContext.getSystemService(Context.VIBRATOR_SERVICE);
        mCallback = callback;
        mFingerprintEngineering = new FingerprintEngineering();
    }

    private void handleCaptureData(ImageData imageData, FingerprintEngineering.ImageData captureData) {
        imageData.setImageCaptureResult(captureData.getCaptureResult());

        if (captureData.isImageCaptured()) {
            imageData.setRawSensorImage(captureData.getRawImage());
            imageData.setPreprocessedSensorImage(captureData.getEnhancedImage());
            imageData.setImageCollectionState(getState());
            mCallback.onImage(Utils.getBitmap(captureData.getEnhancedImage(), true));
            mVibrator.vibrate(VIBRATE_MS_CAPTURE);
            mLogger.i("handleCaptureData: ok");
        } else {
            mLogger.e("handleCaptureData: error");
            mCallback.onNotify(getTextFromErrorCode(captureData.getCaptureResult()));
        }
    }

    /**
     * Captures image with metadata, will block the calling thread until complete.
     *
     * @param fingerType
     * @return imagedata or null if user minimized or the operation is canceled
     * @throws OperationAbortedException
     * @throws OperationSkippedException
     */
    public ImageData doCapture(final FingerType fingerType) throws OperationAbortedException, OperationSkippedException {
        setState(ImageCollectionState.VERIFY);

        final ImageData imageData = new ImageData(fingerType);

        synchronized (mTaskSync) {
            mLogger.i("startCapture");
            mFingerprintEngineering.startCapture(new FingerprintEngineering.Callback<FingerprintEngineering.CaptureData>() {
                @Override
                public void onResult(final FingerprintEngineering.CaptureData captureData) {
                    if (captureData.isImageCaptured()) {
                        imageData.setCaptureStatus(ImageData.CaptureStatus.ACCEPTED);
                        mCallback.onNotify(mContext.getString(R.string.acquired));
                    } else {
                        switch (captureData.getError().getExternalEnum()) {
                            case FPC_ERROR_CANCELLED:
                                mLogger.i("Cancel Capture");
                                imageData.setCaptureStatus(ImageData.CaptureStatus.CANCELLED);
                                break;
                            default:
                                mLogger.i("Error Capture:" + captureData.getError().describe());
                                imageData.setCaptureStatus(ImageData.CaptureStatus.ERROR);
                                break;
                        }
                    }
                    handleCaptureData(imageData, captureData);
                    synchronized (mTaskSync) {
                        mTaskSync.notifyAll();
                    }
                }
            });

            try {
                //wait for the VerifyCallback to finish.
                mTaskSync.wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        if (mShutdownFlag.isAborted()) {
            throw new OperationAbortedException();
        } else if (mShutdownFlag.isSkipped()) {
            throw new OperationSkippedException();
        }

        mLogger.i("capture complete");
        imageData.onCaptureComplete();

        return imageData;
    }

    /**
     * Captures image with metadata and verifies towards enrolled data, will block the calling thread until complete.
     *
     * @return imagedata or null if user minimized or the operation is canceled
     * @throws OperationAbortedException
     */
    public ImageData doVerify(final Enroll enroll, final FingerType fingerType) throws OperationAbortedException, OperationSkippedException {

        setState(ImageCollectionState.VERIFY);

        final ImageData imageData = new ImageData(fingerType);

        synchronized (mTaskSync) {
            mLogger.i("startVerify");
            mFingerprintEngineering.startVerify(new FingerprintEngineering.Callback<FingerprintEngineering.VerifyData>() {
                @Override
                public void onResult(final FingerprintEngineering.VerifyData verifyData) {
                    mLogger.i("doVerify: " + verifyData.getError().describe());
                    if (verifyData.isIdentified()) {
                        if (verifyData.getUserId() != FingerprintEngineering.VerifyData.USER_ID_MATCH_FAILED) {
                            if (enroll.getFingerId() == verifyData.getUserId()) {
                                imageData.setCaptureStatus(ImageData.CaptureStatus.ACCEPTED);
                                mCallback.onNotify(mContext.getString(R.string.accepted));
                            } else {
                                imageData.setCaptureStatus(ImageData.CaptureStatus.REJECTED);
                                mCallback.onNotify(mContext.getString(R.string.rejected_wrong_id));
                            }
                        } else {
                            imageData.setCaptureStatus(ImageData.CaptureStatus.REJECTED);
                            mCallback.onNotify(mContext.getString(R.string.rejected));
                        }
                    } else {
                        FpcError result = verifyData.getError();
                        mLogger.i("Verify failed:" + result.describe());

                        switch (result.getExternalEnum()) {
                            case FPC_ERROR_CANCELLED:
                                mLogger.i("Cancel Verify");
                                imageData.setCaptureStatus(ImageData.CaptureStatus.CANCELLED);
                                break;
                            default:
                                mLogger.i("Error Verify");
                                imageData.setCaptureStatus(ImageData.CaptureStatus.ERROR);
                                break;
                        }
                    }

                    handleCaptureData(imageData, verifyData);
                    synchronized (mTaskSync) {
                        mTaskSync.notifyAll();
                    }
                }
            });

            try {
                //wait for the VerifyCallback to finish.
                mTaskSync.wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        if (mShutdownFlag.isAborted()) {
            throw new OperationAbortedException();
        } else if (mShutdownFlag.isSkipped()) {
            throw new OperationSkippedException();
        }

        mLogger.i("verify complete");
        imageData.onCaptureComplete();

        return imageData;
    }

    /**
     * Will block the calling thread until enroll is complete.
     *
     * @return Enroll
     */
    public Enroll doEnroll(final FingerType fingerType, final Enroll enroll) throws OperationAbortedException, OperationSkippedException {

        mLogger.enter("doEnroll");
        setState(ImageCollectionState.ENROLL);
        final Enroll.EnrollSession session = enroll.createNewSession();

        mCallback.onMessage(mContext.getString(R.string.collection_start_message));
        setState(ImageCollectionState.ENROLL);

        synchronized (mTaskSync) {
            mFingerprintEngineering.startEnroll(new FingerprintEngineering.Callback<FingerprintEngineering.EnrollData>() {
                private int mRejected = 0;
                private int mProgress = 0;

                @Override
                public void onResult(final FingerprintEngineering.EnrollData enrollData) {
                    mLogger.i("onResult: " + enrollData.getError().describe());
                    if (enrollData.isImageCaptured()) {
                        final ImageData imageData = ImageData.createImageData(fingerType, enrollData);
                        handleCaptureData(imageData, enrollData);
                        session.addImageData(imageData);
                    }

                    if (enrollData.isError()) {
                        // The enroll function failed for some reason (e.g. memory failure etc)
                        synchronized (mTaskSync) {
                            switch (enrollData.getError().getExternalEnum()) {
                            case FPC_ERROR_CANCELLED:
                                mLogger.d("Cancel Enroll");
                                session.setEnrollStatus(Enroll.EnrollStatus.CANCELLED);
                                break;
                            default:
                                mLogger.d("Error Enroll");
                                session.setEnrollStatus(Enroll.EnrollStatus.FAILED);
                                break;
                            }
                            mTaskSync.notifyAll();
                            return;
                        }
                    } else if (!enrollData.isImageCaptured()) {
                        // No image captured (e.g. finger lost etc)
                        mCallback.onNotify(getTextFromErrorCode(enrollData.getError()));
                        return;
                    } else {
                        // An image was captured without any error
                        if (enrollData.isAccepted()) {
                            if (enrollData.getError().getExternalEnum() == FpcError.Error.FPC_STATUS_ENROLL_TOO_SIMILAR) {
                                mCallback.onNotifyAndHighlight(getTextFromErrorCode(enrollData.getError()));
                            } else {
                                mCallback.onNotify(mContext.getString(R.string.accepted));
                            }

                            mProgress++;
                            mRejected = 0;
                        } else {
                            mCallback.onNotify(getTextFromErrorCode(enrollData.getError()));
                            mRejected++;
                        }
                    }

                    if (enrollData.getRemaining() >= 0) {
                        mCallback.updateEnrollProgress(mProgress + enrollData.getRemaining(), mProgress, mRejected);
                    }

                    if (enrollData.getRemaining() == 0) {
                        if (enrollData.getEnrollResult().isComplete()) {
                            session.setEnrollStatus(Enroll.EnrollStatus.COMPLETED);
                            enroll.setFingerId(enrollData.getUserId());
                        } else {
                            session.setEnrollStatus(Enroll.EnrollStatus.FAILED);
                        }
                        synchronized (mTaskSync) {
                            mTaskSync.notifyAll();
                        }
                    }
                }
            });

            try {
                //wait for the EnrollCallback to finish.
                mTaskSync.wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        if (mShutdownFlag.isAborted()) {
            throw new OperationAbortedException();
        }

        mLogger.exit("doEnroll");


        return enroll;
    }

    public boolean isRunning() {
        return this.getState().isRunning();
    }

    public void start() {
        setState(ImageCollectionState.STARTED);
        mShutdownFlag = ShutdownFlag.NONE;
    }

    public void onPause() {
        mLogger.i("paused");
        if (getState().isVerify() || getState().isEnroll()) {
            mFingerprintEngineering.cancelCapture();
        }
    }

    public void onResume() {
        mLogger.i("resumed");
        mShutdownFlag = ShutdownFlag.NONE;
    }

    private void setState(final ImageCollectionState state) {
        mImageCollectionState = state;
    }

    private ImageCollectionState getState() {
        return mImageCollectionState;
    }

    public void skip() {
        if (!getState().isVerify()) {
            throw new RuntimeException("Can only skip in verify state, current state: " + getState());
        }
        shutdown(ShutdownFlag.SKIP);
    }

    public void shutdown() {
        shutdown(ShutdownFlag.SHUTDOWN);
    }

    private void shutdown(final ShutdownFlag flag) {
        mLogger.i("shutdown image tool");

        mLogger.i("releasing image data complete");

        mShutdownFlag = flag;

        if (getState().isVerify() || getState().isEnroll()) {
            mFingerprintEngineering.cancelCapture();
        }

        if (flag.isAborted()) {
            setState(ImageCollectionState.STOPPED);
        }
    }

    private String getTextFromErrorCode(FpcError error) {
        switch (error.getExternalEnum()) {
            case FPC_STATUS_ENROLL_LOW_QUALITY:
                return mContext.getString(R.string.capture_error_low_quality);
            case FPC_ERROR_CANCELLED:
                return mContext.getString(R.string.capture_error_canceled);
            case FPC_ERROR_ALREADY_ENROLLED:
                return mContext.getString(R.string.capture_error_already_enrolled);
            case FPC_ERROR_TOO_MANY_FAILED_ATTEMPTS:
                return mContext.getString(R.string.capture_error_enroll_failures);
            case FPC_STATUS_ENROLL_LOW_COVERAGE:
                return mContext.getString(R.string.capture_error_low_sensor_coverage);
            case FPC_STATUS_ENROLL_TOO_SIMILAR:
                return mContext.getString(R.string.capture_error_image_too_similar);
            case FPC_STATUS_ENROLL_LOW_MOBILITY:
                return mContext.getString(R.string.capture_error_low_mobility);
            case FPC_STATUS_FINGER_LOST:
                return mContext.getString(R.string.capture_error_acquired_too_fast);
            case FPC_STATUS_WAIT_TIME:
                return mContext.getString(R.string.capture_error_acquired_insufficient);
            default:
                return error.describe();
        }
    }

    public String getBuildInfo() {
        if (buildInfo == null) {
            if (mFingerprintEngineering != null) {
                try {
                    buildInfo = mFingerprintEngineering.getBuildInfo();
                } catch (Exception e) {
                    mLogger.w("getBuildInfo: " + e);
                }
            }
        }
        return buildInfo;
    }
}
