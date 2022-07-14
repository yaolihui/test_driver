/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.engineering;

import android.os.Handler;
import android.os.RemoteException;

import com.fingerprints.extension.common.FingerprintExtensionBase;
import com.fingerprints.extension.util.FpcError;
import com.fingerprints.extension.util.Logger;

public class FingerprintEngineering extends FingerprintExtensionBase {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private static final String ENGINEERING = "com.fingerprints.extension.engineering.IFingerprintEngineering";
    private IFingerprintEngineering mFingerprintEngineering;
    private ImageSubscriptionCallback mImageSubscriptionCallback;
    private ImageInjectionCallback mImageInjectionCallback;
    private Callback<?> mCaptureCallback;
    private SensorSize mSensorSize;
    private SensorImage.BitsPerPixel mBitsPerPixel = SensorImage.BitsPerPixel.BPP_8;
    private Handler mHandler;

    /**
     * ImageType values represent different stages of processing of the image.
     */
    public enum ImageType {
        /**
         * Raw image (not preprocessed)
         */
        RAW,
        /**
         * Preprocessed image
         */
        PREPROCESSED
    }

    public enum CaptureMode {
        CAPTURE,
        VERIFY,
        ENROLL;
    }

    public enum CacResult {
        CAC_SUCCESS(0),
        CAC_ERROR_GENERAL(1000),
        CAC_ERROR_FINGER_POOR_COVERAGE(1001),
        CAC_ERROR_FINGER_UP_TOO_EARLY(1002),
        CAC_ERROR_CALIBRATION(1003),
        CAC_ERROR_CAL_MAX_ATTEMPTS(1004),
        CAC_ERROR_FINGER_NOT_STABLE(1005),
        CAC_ERROR_NO_FINGER(1006),
        CAC_ERROR_DB_MAX_ATTEMPTS(1007),
        CAC_ERROR_IMAGE_CAPTURE(1008),
        CAC_ERROR_IMAGE_BAD_QUALITY(1009),
        CAC_ERROR_FIFO_UNDERFLOW(1010),
        CAC_ERROR_NOT_A_FINGER(1011),
        CAC_ERROR_MEMORY(1012),
        CAC_ERROR_METADATA(1013),
        CAC_ERROR_HISTO_SATURATED(1014),
        CAC_ERROR_UNSUPPORTED(1015),
        CAC_ERROR_PARAMETER(1016),
        CAC_ERROR_FINGER_QUERY(1017),

        CAC_ERROR_UNKNOWN(-1);

        private final int mCacResult;

        CacResult(int cacResult) {
            mCacResult = cacResult;
        }

        public static CacResult fromInteger(int cacValue) {
            for (CacResult cacResult : CacResult.values()) {
                if (cacResult.mCacResult == cacValue) {
                    return cacResult;
                }
            }
            return CAC_ERROR_UNKNOWN;
        }

        public boolean isFailure() {
            return CAC_SUCCESS != this;
        }
    }

    public class EnrollResult extends FpcError {

        public EnrollResult(int value) {
            super(value);
        }

        public boolean isAccepted() {
            if (isError()) {
                return false;
            }
            switch (getExternalEnum()) {
                case FPC_STATUS_ENROLL_LOW_COVERAGE:
                case FPC_STATUS_ENROLL_LOW_QUALITY:
                case FPC_STATUS_ENROLL_LOW_MOBILITY:
                    return false;
                default:
                    return true;
            }
        }

        public boolean isComplete() {
            return getExternalEnum() == Error.FPC_ERROR_NONE;
        }
    }

    public class ImageCaptureResult extends FpcError {
        private final CacResult mCacResult;

        public ImageCaptureResult(int imageCaptureResult, int cacResult) {
            super(imageCaptureResult);
            mCacResult = CacResult.fromInteger(cacResult);
        }

        public CacResult getCacResult() {
            return mCacResult;
        }

        public boolean isImageCaptured() {
            return getExternalEnum() == FpcError.Error.FPC_ERROR_NONE;
        }
    }

    public abstract class ImageData {
        private final SensorImage mRawImage;
        private final SensorImage mEnhancedImage;
        private final ImageCaptureResult mCaptureResult;

        public ImageData(final int captureResult, final int cacResult, final byte[] rawImage,
                         final byte[] enhancedImage) {
            mCaptureResult = new ImageCaptureResult(captureResult, cacResult);
            mRawImage = new SensorImage(mBitsPerPixel, mSensorSize.mWidth, mSensorSize.mHeight, rawImage);
            mEnhancedImage = new SensorImage(mBitsPerPixel, mSensorSize.mWidth, mSensorSize.mHeight, enhancedImage);
        }

        public SensorImage getRawImage() {
            return mRawImage;
        }

        public SensorImage getEnhancedImage() {
            return mEnhancedImage;
        }

        public ImageCaptureResult getCaptureResult() {
            return mCaptureResult;
        }

        /**
         * @return true if an image was successfully captured
         */
        public boolean isImageCaptured() {
            return mCaptureResult.getExternalEnum() == FpcError.Error.FPC_ERROR_NONE;
        }

        public FpcError getError() {
            return mCaptureResult;
        }
    }

    public class CaptureData extends ImageData {
        public CaptureData(final int captureResult, final int cacResult, final byte[] rawImage,
                           final byte[] enhancedImage) {
            super(captureResult, cacResult, rawImage, enhancedImage);
        }
    }

    public class EnrollData extends ImageData {
        private final int mUserId;
        private final EnrollResult mEnrollResult;
        private final int mRemaining;

        public EnrollData(final int captureResult, final int enrollResult, final int cacResult, final int userId, final int remaining, final byte[] rawImage,
                          final byte[] enhancedImage) {

            super(captureResult, cacResult, rawImage, enhancedImage);
            mEnrollResult = new EnrollResult(enrollResult);
            mUserId = userId;
            mRemaining = remaining;
        }

        public int getRemaining() {
            return mRemaining;
        }

        public int getUserId() {
            return mUserId;
        }

        public EnrollResult getEnrollResult() {
            return mEnrollResult;
        }

        /**
         * @return true if an image was captured and accepted by the enroll
         */
        public boolean isAccepted() {
            return getCaptureResult().isImageCaptured() && mEnrollResult.isAccepted();
        }

        /**
         * @return true if there was an error that will abort the enroll sequence
         */
        public boolean isError() {
            return getError().isError();
        }

        /**
         * @return First error, i.e. if capture fails, the capture error else return the
         * enroll error.
         */
        public FpcError getError() {
            if (!isImageCaptured()) {
                return super.getError();
            } else {
                return mEnrollResult;
            }
        }
    }

    public class VerifyData extends ImageData {
        public static final int USER_ID_MATCH_FAILED = 0;
        private final FpcError mIdentifyResult;
        private final int mUserId;
        private final int mCoverage;
        private final int mQuality;

        public VerifyData(final int captureResult, final int identifyResult, final int templateUpdateResult,
                          final int cacResult, final int userId, final int coverage, final int quality,
                          final byte[] rawImage, final byte[] enhancedImage) {
            super(captureResult, cacResult, rawImage, enhancedImage);

            mIdentifyResult = new FpcError(identifyResult);
            mUserId = userId;
            mCoverage = coverage;
            mQuality = quality;
        }

        public FpcError getIdentifyResult() {
            return mIdentifyResult;
        }

        public int getQuality() {
            return mQuality;
        }

        public int getCoverage() {
            return mCoverage;
        }

        public int getUserId() {
            return mUserId;
        }

        /**
         * @return true if an image was captured identification process was successfully done
         */
        public boolean isIdentified() {
            return isImageCaptured() && (!mIdentifyResult.isError());
        }

        /**
         * Return first error, i.e. if capture fails, that is the error, if capture is ok
         * then the mIdentifyResult is the error (which might be ok)
         */
        public FpcError getError() {
            if (super.getError().getExternalEnum() != FpcError.Error.FPC_ERROR_NONE) {
                return super.getError();
            } else {
                return mIdentifyResult;
            }
        }
    }

    private IImageInjectionCallback mIImageInjectionCallback =
            new IImageInjectionCallback.Stub() {
                public byte[] onInject() {
                    mLogger.enter("onInject");
                    byte[] imageData = null;
                    if (mImageInjectionCallback != null) {
                        ImageInjectionCallback.InjectionError error = null;
                        try {
                            SensorImage image = null;
                            image = mImageInjectionCallback.onInject(mBitsPerPixel,
                                    mSensorSize.mWidth, mSensorSize.mHeight);
                            if (image != null && image.getPixels() != null) {
                                if (image.getBitsPerPixel() != mBitsPerPixel
                                        || image.getWidth() != mSensorSize.mWidth
                                        || image.getHeight() != mSensorSize.mHeight) {
                                    error = ImageInjectionCallback.InjectionError.IMAGE_FORMAT_INCONSISTENT;
                                } else {
                                    imageData = image.getPixels();
                                }
                            } else {
                                error = ImageInjectionCallback.InjectionError.UNSPECIFIED_INJECTION_ERROR;
                            }
                        } catch (Exception e) {
                            error = ImageInjectionCallback.InjectionError.UNSPECIFIED_INJECTION_ERROR;
                            mLogger.e("Exception: " + e);
                        }
                        if (error != null) {
                            mLogger.e("Image injection error occured, code: " + error);
                            onError(error);
                        }
                    }
                    mLogger.exit("onInject");
                    return imageData;
                }

                public void onCancel() {
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            if (mImageInjectionCallback != null) {
                                mImageInjectionCallback.onCancel();
                            }
                        }
                    });
                }

                public void onError(final ImageInjectionCallback.InjectionError errorCode) {
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            if (mImageInjectionCallback != null) {
                                mImageInjectionCallback.onError(errorCode);
                            }
                        }
                    });
                }
            };

    public interface ImageInjectionCallback {

        public enum InjectionError {
            /**
             * Unspecified error during image injection
             */
            UNSPECIFIED_INJECTION_ERROR,
            /**
             * image returned from onInject had inconsistent format
             */
            IMAGE_FORMAT_INCONSISTENT,
            /**
             * image returned from onInject did not match the required
             * format
             */
            IMAGE_FORMAT_NOT_SUPPORTED
        }

        public SensorImage onInject(SensorImage.BitsPerPixel bitsPerPixel, int width, int height);

        public void onCancel();

        public void onError(InjectionError errorCode);
    }

    public interface Callback<T> {
        void onResult(final T data);
    }

    public interface ImageSubscriptionCallback {
        public void onImage(final ImageData data);
    }

    private IImageSubscriptionCallback mIImageSubscriptionCallback = new IImageSubscriptionCallback.Stub() {
        public void onImage(int mode, int captureResult, int identifyResult, int templateUpdateResult, int enrollResult, int cacResult, int userId, int remainingSamples, int coverage, int quality, final byte[] rawImage, final byte[] pngImage) {
            if (mImageSubscriptionCallback != null) {
                ImageData data = null;
                switch (CaptureMode.values()[mode]) {
                    case CAPTURE: {
                        data = new CaptureData(captureResult, cacResult, rawImage, pngImage);
                        break;
                    }
                    case VERIFY: {
                        data = new VerifyData(captureResult, identifyResult, templateUpdateResult, cacResult, userId, coverage, quality, rawImage, pngImage);
                        break;
                    }
                    case ENROLL: {
                        data = new EnrollData(captureResult, enrollResult, cacResult, userId, remainingSamples, rawImage, pngImage);
                        break;
                    }
                }
                mImageSubscriptionCallback.onImage(data);
            }
        }
    };

    private ICaptureCallback mICaptureCallback = new ICaptureCallback.Stub() {
        public void onResult(int mode, int captureResult, int identifyResult, int templateUpdateResult, int enrollResult, int cacResult, int userId, int remainingSamples, int coverage, int quality, final byte[] rawImage, final byte[] pngImage) {
            Callback callback = mCaptureCallback;
            if (remainingSamples == 0) {
                mCaptureCallback = null;
                mLogger.d("onResult callback cleared");
            }

            if (callback != null) {
                if (mode == CaptureMode.CAPTURE.ordinal()) {
                    CaptureData data = new CaptureData(captureResult, cacResult, rawImage, pngImage);
                    callback.onResult(data);
                } else if (mode == CaptureMode.VERIFY.ordinal()) {
                    VerifyData data = new VerifyData(captureResult, identifyResult, templateUpdateResult, cacResult, userId, coverage, quality, rawImage, pngImage);
                    callback.onResult(data);
                } else if (mode == CaptureMode.ENROLL.ordinal()) {
                    EnrollData data = new EnrollData(captureResult, enrollResult, cacResult, userId, remainingSamples, rawImage, pngImage);
                    callback.onResult(data);
                }
            } else {
                mLogger.e("onResult, callback is null");
            }
        }
    };

    public FingerprintEngineering() throws RemoteException {
        mLogger.enter("FingerprintEngineering");
        mHandler = new Handler();
        mFingerprintEngineering = IFingerprintEngineering.Stub.asInterface(
                getFingerprintExtension(ENGINEERING));
        if (mFingerprintEngineering == null) {
            throw new RemoteException("Could not get " + ENGINEERING);
        } else {
            mSensorSize = getSensorSize();
            mLogger.enter("mSensorSize " + mSensorSize.mWidth + " x " + mSensorSize.mHeight);
        }
        mLogger.exit("FingerprintEngineering");
    }

    public SensorSize getSensorSize() {
        mLogger.enter("getSensorSize");
        SensorSize sensorSize = null;
        if (mFingerprintEngineering != null) {
            try {
                sensorSize = mFingerprintEngineering.getSensorSize();
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("getSensorSize");
        return sensorSize;
    }

    public void startCapture(Callback<CaptureData> callback) {
        mLogger.enter("startCapture");
        mCaptureCallback = callback;
        if (mFingerprintEngineering != null) {
            try {
                mFingerprintEngineering.startCapture(mICaptureCallback, CaptureMode.CAPTURE.ordinal());
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("startCapture");
    }

    public long getEnrollChallenge() {
        mLogger.enter("getEnrollChallenge");
        if (mFingerprintEngineering != null) {
            try {
                return mFingerprintEngineering.getEnrollChallenge();
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("getEnrollChallenge");
        return 0;
    }

    public String getBuildInfo() {
        return null;
    }

    public void setEnrollToken(byte[] token) {
        mLogger.enter("setEnrollToken");
        if (mFingerprintEngineering != null) {
            try {
                mFingerprintEngineering.setEnrollToken(token);
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("setEnrollToken");
    }

    public void startVerify(Callback<VerifyData> callback) {
        mLogger.enter("startVerify");
        mCaptureCallback = callback;
        if (mFingerprintEngineering != null) {
            try {
                mFingerprintEngineering.startCapture(mICaptureCallback, CaptureMode.VERIFY.ordinal());
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("startVerify");
    }

    public void startEnroll(Callback<EnrollData> callback) {
        mLogger.enter("startEnroll");
        mCaptureCallback = callback;
        if (mFingerprintEngineering != null) {
            try {
                mFingerprintEngineering.startCapture(mICaptureCallback, CaptureMode.ENROLL.ordinal());
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("startEnroll");
    }

    public void cancelCapture() {
        mLogger.enter("cancelCapture");
        if (mFingerprintEngineering != null) {
            try {
                mFingerprintEngineering.cancelCapture();
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("cancelCapture");
    }

    public void startImageSubscription(ImageSubscriptionCallback callback) {
        mLogger.enter("startImageSubscription");
        mImageSubscriptionCallback = callback;
        if (mFingerprintEngineering != null) {
            try {
                mFingerprintEngineering.startImageSubscription(
                        mIImageSubscriptionCallback);
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("startImageSubscription");
    }

    public void stopImageSubscription() {
        mLogger.enter("stopImageSubscription");
        if (mFingerprintEngineering != null) {
            try {
                mFingerprintEngineering.stopImageSubscription();
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("stopImageSubscription");
    }

    public void startImageInjection(ImageInjectionCallback callback) {
        mLogger.enter("startImageInjection");
        mImageInjectionCallback = callback;
        if (mFingerprintEngineering != null) {
            try {
                mFingerprintEngineering.startImageInjection(mIImageInjectionCallback);
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("startImageInjection");
    }

    public void stopImageInjection() {
        mLogger.enter("stopImageInjection");
        if (mFingerprintEngineering != null) {
            try {
                mFingerprintEngineering.stopImageInjection();
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("stopImageInjection");
    }
}
