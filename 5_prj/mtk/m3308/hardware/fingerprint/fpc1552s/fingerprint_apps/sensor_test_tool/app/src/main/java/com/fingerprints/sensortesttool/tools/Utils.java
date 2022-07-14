/*
 *
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */

package com.fingerprints.sensortesttool.tools;

import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.Matrix;

public class Utils {
    public static Bitmap getBitmap(byte[] imageData, int width, int height, boolean rotate) {
        int frameSize = width * height;
        int[] colourArray = new int[imageData.length];
        int alpha;
        for (int i = 0; i < frameSize; i++) {
            // This promotes the pixel value to an int,
            // eliminating the MSB. This step may seem redundant
            // but it is not.
            alpha = imageData[i] & 0x00FF;
            colourArray[i] = Color.argb(0xFF, alpha, alpha, alpha);
        }
        Bitmap bmSource = Bitmap.createBitmap(colourArray, width, height, Bitmap.Config.ARGB_8888);
        Matrix matrix = new Matrix();
        if (rotate && height > width) {
            matrix.postRotate(-90);
            matrix.postScale(-1f, 1f);
        }
        return Bitmap.createBitmap(bmSource, 0, 0, width, height, matrix, true);
    }
}
