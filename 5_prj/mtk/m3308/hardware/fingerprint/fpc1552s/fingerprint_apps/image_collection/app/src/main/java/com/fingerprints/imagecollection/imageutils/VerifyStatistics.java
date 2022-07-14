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

public class VerifyStatistics {
    private int mAccepted = 0;
    private int mRejected = 0;
    private int mImages = 0;
    private boolean mDecisionFeedback;

    public VerifyStatistics(final boolean decisionFeedback) {
        mAccepted = 0;
        mRejected = 0;
        mDecisionFeedback = decisionFeedback;
    }

    public void addData(final ImageData data) {
        mImages++;
        if (data.getVerifyConfig().getConfig().getDecisionFeedback()) {
            if (data.getCaptureStatus().isAccepted()) {
                mAccepted++;
            } else {
                mRejected++;
            }
        }
    }

    public int getAccepted() {
        return mAccepted;
    }

    public int getRejected() {
        return mRejected;
    }

    public boolean hasDecisionFeedback() {
        return mDecisionFeedback;
    }
}
