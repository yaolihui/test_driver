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
package com.fingerprints.imagecollection.interfaces;

import android.graphics.Bitmap;

import com.fingerprints.imagecollection.scenario.VerifyConfig;
import com.fingerprints.imagecollection.values.FingerType;

public interface IImageCollectionTask {
    public void clearProgress();

    public void updateProgress(int max, int progress, int rejected);

    public void setTitle(final String title, final FingerType finger);

    public void clearTitle();

    public void updateFingerprint(Bitmap bitmap);

    public void clearFingerprint();

    public void showNotification(String message);

    public void showHighlightNotification(String message);

    public void onError(final String message);

    public void setMessage(String message);

    public void clearStatus();

    public void clearMessage();

    public void updateFinger(FingerType fingerType);

    public void onFinishedCollection(boolean isCancel);

    public void onStartedEnroll();

    public void onStartedVerify(final VerifyConfig verifyConfig, final FingerType finger);

    public void resetUI();
}
