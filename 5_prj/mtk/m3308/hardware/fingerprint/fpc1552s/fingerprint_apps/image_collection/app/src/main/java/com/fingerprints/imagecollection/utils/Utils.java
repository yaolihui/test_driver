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

package com.fingerprints.imagecollection.utils;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.Matrix;
import android.os.Environment;

import com.fingerprints.extension.engineering.SensorImage;
import com.fingerprints.imagecollection.values.Constants;
import com.fingerprints.imagecollection.values.ImageCollectionState;
import com.fingerprints.imagecollection.values.Preferences;

import java.io.File;
import java.io.FilenameFilter;
import java.util.Arrays;
import java.util.Collections;
import java.util.Locale;

public class Utils {
    private static Logger sLogger = new Logger("ImageUtils");

    private Utils() {
    }

    public static boolean makeDir(Context context) {
        File fpcDir = new File(
                context.getExternalFilesDir(Environment.DIRECTORY_PICTURES).getPath(),
                Constants.FPC_FOLDER);
        if (fpcDir.exists() || fpcDir.mkdir()) {
            int counter = 0;
            File[] files = fpcDir.listFiles();
            if (files != null) {
                Arrays.sort(files, Collections.reverseOrder());
                for (File file : files) {
                    if (file.isDirectory()) {
                        try {
                            counter = Integer.parseInt(file.getName());
                            break;
                        } catch (NumberFormatException e) {
                            sLogger.w("NumberFormatException: " + e);
                        }
                    }
                }
            }
            File dir = new File(fpcDir, String.format(Locale.US, "%04d", counter + 1));
            if (dir.mkdir()) {
                Preferences.setCurrentDir(context, dir.getAbsolutePath());
                return true;
            } else {
                sLogger.e("Could not create directory " + dir);
            }
        } else {
            sLogger.e("Could not create directory " + fpcDir);
        }
        return false;
    }

    public static void moveEnrollFailedImages(Context context, final int fingerIndex) {
        String dirPath = Preferences.getCurrentDir(context);
        if (dirPath.trim().length() != 0) {
            File dir = new File(dirPath + "/" + ImageCollectionState.ENROLL.getName());
            if (dir.exists()) {
                File[] files = dir.listFiles(new FilenameFilter() {
                    public boolean accept(File dir, String name) {
                        return name.toLowerCase().contains(String.format(Locale.US, "_%02d_",
                                fingerIndex));
                    }
                });

                if (files != null) {
                    File newDir = new File(dirPath + "/" + Constants.ENROLL_FAIL_DIR);
                    if (newDir.exists() || newDir.mkdir()) {
                        for (File file : files) {
                            File newFile = new File(file.getAbsolutePath().replaceFirst(
                                    ImageCollectionState.ENROLL.getName(),
                                    Constants.ENROLL_FAIL_DIR));
                            if (file.renameTo(newFile)) {
                                sLogger.i(file.getName() + " renamed to " + newFile.getName());
                            } else {
                                sLogger.i(file.getName() + " rename failed");
                            }
                        }
                    }
                }
            }
        }
    }

    public static Bitmap getBitmap(SensorImage image, boolean rotate) {
        byte[] imageData = image.getPixels();
        int width = image.getWidth();
        int height = image.getHeight();
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
