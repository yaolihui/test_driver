/*
 *
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */

package com.fingerprints.sensortesttool.tools;

import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.Color;
import android.graphics.Matrix;

import com.fingerprints.extension.engineering.SensorImage;

public class SensorImageBitmap {
    private Bitmap mBitmap;
    private SensorImage mSensorImage;

    public SensorImageBitmap(SensorImage sensorImage) {
        mSensorImage = sensorImage;
    }

    public Bitmap getBitmap() {
        if (mBitmap == null) {
            mBitmap = createBitmap();
        }
        return mBitmap;
    }

    /*
     * Creates a bitmap from a SensorImage object.
     * @param image the image source
     */
    private Bitmap createBitmap() {
        byte[] imageData = mSensorImage.getPixels();
        int width = mSensorImage.getWidth();
        int height = mSensorImage.getHeight();
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
        Bitmap bitmap = Bitmap.createBitmap(colourArray, width, height, Config.ARGB_8888);
        Matrix matrix = new Matrix();
        return Bitmap.createBitmap(bitmap, 0, 0, width, height, matrix, true);
    }

    public int getWidth() {
        return mSensorImage.getWidth();
    }

    public int getHeight() {
        return mSensorImage.getHeight();
    }
}
