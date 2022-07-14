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

package com.fingerprints.imagecollection.imageutils;

import com.fingerprints.extension.engineering.FingerprintEngineering;
import com.fingerprints.extension.engineering.FingerprintEngineering.ImageCaptureResult;
import com.fingerprints.extension.engineering.FingerprintEngineering.EnrollResult;

import com.fingerprints.extension.engineering.SensorImage;
import com.fingerprints.imagecollection.scenario.VerifyConfig;
import com.fingerprints.imagecollection.values.FingerType;
import com.fingerprints.imagecollection.values.ImageCollectionState;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

import static com.fingerprints.imagecollection.imageutils.ImageData.CaptureStatus.ACCEPTED;
import static com.fingerprints.imagecollection.imageutils.ImageData.CaptureStatus.CANCELLED;
import static com.fingerprints.imagecollection.imageutils.ImageData.CaptureStatus.ERROR;
import static com.fingerprints.imagecollection.imageutils.ImageData.CaptureStatus.REJECTED;

public class ImageData {

    public enum CaptureStatus {
        INITED, ACCEPTED, REJECTED, DENIED, ERROR, CANCELLED;

        public boolean isAccepted() {
            return this == ACCEPTED;
        }

        public boolean isRejected() {
            return this == REJECTED;
        }

        public boolean isError() {
            return this == ERROR || this == CANCELLED;
        }

        public boolean isCancelled() {
            return this == CANCELLED;
        }
    }

    private String mTimeString;
    private SensorImage mRawSensorImage = null;
    private SensorImage mPreprocessedSensorImage = null;
    private ImageCollectionState mImageCollectionState;
    private CaptureStatus mCaptureStatus = CaptureStatus.INITED;
    private final FingerType mFingerType;
    private Integer mSampleId;
    private Integer mErrMsgId;
    private Enroll.EnrollSession mEnrollSession = null;
    private VerifyConfig mVerifyConfig;
    private ImageCaptureResult mImageCaptureResult;
    private EnrollResult mEnrollResult = null;
    private Date mDate;

    public static ImageData createImageData(FingerType fingerType, FingerprintEngineering.EnrollData enrollData) {
        ImageData imageData = new ImageData(fingerType);
        imageData.setEnrollResult(enrollData.getEnrollResult());
        if (enrollData.isError()) {
            imageData.setErrorId(enrollData.getError().getErrorCode());
            switch (enrollData.getError().getExternalEnum()) {
                case FPC_ERROR_CANCELLED:
                    imageData.setCaptureStatus(CANCELLED);
                    break;
                default:
                    imageData.setCaptureStatus(ERROR);
                    break;
            }
        } else if (enrollData.isImageCaptured()) {
            if (enrollData.isAccepted()) {
                imageData.setCaptureStatus(ACCEPTED);
            } else {
                imageData.setCaptureStatus(REJECTED);
            }
        } else {
            // No image was captured -> can't create ImageData
            return null;
        }
        return imageData;
    }

    public ImageData(final FingerType fingerType) {
        mFingerType = fingerType;
    }

    public void onCaptureComplete() {
        mDate = new Date();
        mTimeString = new SimpleDateFormat("yyyyMMddHHmmssSSS", Locale.US).format(mDate);
    }

    public Date getDate() {
        return mDate;
    }

    public void setImageCollectionState(final ImageCollectionState imageCollectionState) {
        mImageCollectionState = imageCollectionState;
    }

    public ImageCollectionState getImageCollectionState() {
        return mImageCollectionState;
    }

    public FingerType getFingerType() {
        return mFingerType;
    }

    public void setRawSensorImage(final SensorImage rawSensorImage) {
        mRawSensorImage = rawSensorImage;
    }

    public SensorImage getRawSensorImage() {
        return mRawSensorImage;
    }

    public void setPreprocessedSensorImage(final SensorImage preprocessedSensorImage) {
        mPreprocessedSensorImage = preprocessedSensorImage;
    }

    public SensorImage getPreprocessedSensorImage() {
        return mPreprocessedSensorImage;
    }

    public CaptureStatus getCaptureStatus() {
        return mCaptureStatus;
    }

    public void setCaptureStatus(final CaptureStatus captureStatus) {
        mCaptureStatus = captureStatus;
    }

    public int getSampleId() {
        return mSampleId;
    }

    public void setSampleId(final int sampleId) {
        mSampleId = sampleId;
    }

    public void setErrorId(int errMsgId) {
        mErrMsgId = errMsgId;
    }

    public void setEnrollResult(final EnrollResult enrollrResult) {
        mEnrollResult = enrollrResult;
    }
    public boolean hasEnrollSession() {
        return mEnrollSession != null;
    }

    public void setEnrollSession(final Enroll.EnrollSession enrollSession) {
        mEnrollSession = enrollSession;
    }

    public void setVerifyConfig(final VerifyConfig verifyConfig) {
        mVerifyConfig = verifyConfig;
    }

    public boolean hasVerifyConfig() {
        return mVerifyConfig != null;
    }

    public VerifyConfig getVerifyConfig() {
        return mVerifyConfig;
    }

    public Enroll.EnrollSession getEnrollSession() {
        return mEnrollSession;
    }

    public String getTimeString() {
        return mTimeString;
    }

    public boolean hasErrorMessage() {
        return mErrMsgId != null;
    }

    public int getErrMsgId() {
        return mErrMsgId;
    }

    public EnrollResult getEnrollResult() {
        return mEnrollResult;
    }

    public void setImageCaptureResult(ImageCaptureResult imageCaptureResult) {
        this.mImageCaptureResult = imageCaptureResult;
    }

    public ImageCaptureResult getImageCaptureResult() {
        return mImageCaptureResult;
    }
}
