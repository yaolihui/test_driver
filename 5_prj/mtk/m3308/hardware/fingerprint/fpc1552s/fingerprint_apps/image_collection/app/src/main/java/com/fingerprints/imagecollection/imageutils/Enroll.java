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

import java.util.ArrayList;

public class Enroll {

    private ArrayList<EnrollSession> mEnrollSessions;
    private int mFingerId;

    public class EnrollSession {
        private EnrollStatus mEnrollStatus;
        private ArrayList<ImageData> mImageDataArrayList;

        public EnrollSession() {
            mEnrollStatus = EnrollStatus.CREATED;
            mImageDataArrayList = new ArrayList<>();
        }

        public boolean hasData() {
            return mImageDataArrayList != null && mImageDataArrayList.size() > 0;
        }

        public ArrayList<ImageData> getImageDataList() {
            return mImageDataArrayList;
        }

        public EnrollStatus getStatus() {
            return mEnrollStatus;
        }

        public void addImageData(ImageData imageData) {
            imageData.setSampleId(getCurrentSampleId());
            imageData.setEnrollSession(this);
            mImageDataArrayList.add(imageData);
            imageData.onCaptureComplete();
        }

        public void setEnrollStatus(final EnrollStatus enrollStatus) {
            mEnrollStatus = enrollStatus;
        }
    }

    public enum EnrollStatus {
        CREATED, COMPLETED, FAILED, CANCELLED;

        public boolean isComplete() {
            return this == COMPLETED;
        }

        public boolean isFailed() {
            return this == FAILED;
        }

        public boolean isCancelled() {
            return this == CANCELLED;
        }

    }

    public Enroll() {
        mEnrollSessions = new ArrayList<>();
    }

    private int getCurrentSampleId() {
        return getTotalNumberSamples();
    }

    public int getTotalNumberSamples() {
        int samples = 0;
        for (EnrollSession session : mEnrollSessions) {
            samples += session.getImageDataList().size();
        }
        return samples;
    }

    public EnrollSession getLastSession() {
        return mEnrollSessions.get(mEnrollSessions.size() - 1);
    }

    public EnrollSession createNewSession() {
        EnrollSession session = new EnrollSession();
        mEnrollSessions.add(session);
        return session;
    }

    public void setFingerId(final int fingerId) {
        mFingerId = fingerId;
    }

    public int getFingerId() {
        return mFingerId;
    }

    public ArrayList<EnrollSession> getEnrollSessions() {
        return mEnrollSessions;
    }
}
