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

package com.fingerprints.imagecollection.imageutils;

import android.graphics.Bitmap;

public interface IImageToolCallback {
    public void onError(String message);

    public void onReset();

    public void onMessage(String s);

    public void onNotify(String s);

    public void onNotifyAndHighlight(String s);

    public void clearMessage();

    public void updateEnrollProgress(int max, int progress, final int rejected);

    public void clearProgress();

    public void onImage(Bitmap bitmap);
}
