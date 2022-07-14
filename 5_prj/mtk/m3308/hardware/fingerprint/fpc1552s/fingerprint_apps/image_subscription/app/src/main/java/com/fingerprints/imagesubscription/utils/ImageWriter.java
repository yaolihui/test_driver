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

package com.fingerprints.imagesubscription.utils;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.Matrix;
import android.os.Environment;

import com.fingerprints.extension.engineering.FingerprintEngineering;
import com.fingerprints.extension.engineering.SensorImage;
import com.fingerprints.fmi.FMI;
import com.fingerprints.fmi.FMIBlockType;
import com.fingerprints.fmi.FMIOpenMode;
import com.fingerprints.fmi.FMIStatus;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Collections;
import java.util.Date;
import java.util.Locale;

public class ImageWriter {
    private final Logger mLogger = new Logger(getClass().getSimpleName());
    private final File mDir;
    private final Context mContext;
    private FMI mFMIWriter;
    private TaskLog mTaskLog;
    private FingerprintEngineering mFingerprintEngineering;
    private String mBuildInfo;

    public ImageWriter(Context context) {
        mLogger.enter("ImageWriter");
        mContext = context;

        try {
            mFingerprintEngineering = new FingerprintEngineering();
        } catch (Exception e) {
            mLogger.e("Exception: " + e);
        }

        mBuildInfo = getBuildInfo();

        String storagePath = context.getExternalFilesDir(Environment.DIRECTORY_PICTURES).getPath();
        mDir = new File(storagePath);
        mLogger.i("Writing files to path: " + mDir.getPath());
        if (!mDir.exists() && !mDir.mkdirs()) {
            mLogger.w("Create subscription folder failed");
        }
        mTaskLog = new TaskLog();
        mLogger.exit("ImageWriter");
    }

    public void writeImage(final FingerprintEngineering.ImageData captureData) {
        mLogger.enter("writeImage");

        String fileName = getFileName(captureData.getRawImage());
        if (fileName != null) {
            mFMIWriter = new FMI();
            FMIStatus status = mFMIWriter.openFile(fileName + ".fmi", FMIOpenMode.WRITE);
            if (status == FMIStatus.OK) {
                mLogger.i("writeFMI: " + fileName + ".fmi");

                FMIBlockType versionType = new FMIBlockType();
                versionType.format = FMIBlockType.Format.JSON;
                versionType.id = FMIBlockType.Id.VERSION_INFO;
                status = mFMIWriter.addBlock(versionType, ("" + mBuildInfo).getBytes());
                if (status != FMIStatus.OK) {
                    mLogger.w("failed to add version info data block: " + status);
                }

                FMIBlockType rawType = new FMIBlockType();
                rawType.format = FMIBlockType.Format.FPC_IMAGE_DATA;
                rawType.id = FMIBlockType.Id.RAW_IMAGE;
                status = mFMIWriter.addBlock(rawType, captureData.getRawImage().getPixels());
                if (status != FMIStatus.OK) {
                    mLogger.w("failed to add fpc image data block: " + status);
                }

                FMIBlockType preprocessedType = new FMIBlockType();
                preprocessedType.format = FMIBlockType.Format.PNG;
                preprocessedType.id = FMIBlockType.Id.DISPLAY_IMAGE;
                ByteArrayOutputStream baos = new ByteArrayOutputStream();
                Bitmap bitmap = getBitmap(captureData.getEnhancedImage(), false);
                bitmap.compress(Bitmap.CompressFormat.PNG, 100, baos);
                status = mFMIWriter.addBlock(preprocessedType, baos.toByteArray());
                if (status != FMIStatus.OK) {
                    mLogger.w("failed to add preprocessed image block: " + status);
                }

                status = mFMIWriter.close();
                if (status != FMIStatus.OK) {
                    mLogger.e("failed to save file: " + status);
                } else {
                    writeDetails(fileName, captureData);
                }
            } else {
                mLogger.w("failed to open file: " + status);
            }
        }

        if (Preferences.getSavePng(mContext)) {
            writePng(fileName, captureData.getEnhancedImage());
        }

        mLogger.exit("writeImage");
    }

    private void writeDetails(final String fileName, final FingerprintEngineering.ImageData captureData) {
        mTaskLog.reset();
        mTaskLog.open("CaptureData").logTime();

        if (captureData instanceof FingerprintEngineering.CaptureData) {
            FingerprintEngineering.CaptureData data = (FingerprintEngineering.CaptureData) captureData;

        } else if (captureData instanceof FingerprintEngineering.VerifyData) {
            FingerprintEngineering.VerifyData data = (FingerprintEngineering.VerifyData) captureData;
            mTaskLog.open("VerifyData");
            if (data.getUserId() == 0) {
                mTaskLog.log("state", "Rejected");
            } else {
                mTaskLog.log("state", "Accepted").log("fid", data.getUserId());
            }
            mTaskLog.log("coverage", data.getCoverage());
            mTaskLog.log("quality", data.getQuality());
            mTaskLog.close();

        } else if (captureData instanceof FingerprintEngineering.EnrollData) {
            FingerprintEngineering.EnrollData data = (FingerprintEngineering.EnrollData) captureData;
            mTaskLog.open("EnrollData");
            mTaskLog.log("index", data.getRemaining());
            mTaskLog.log("status", data.getError().describe());
            if (data.getUserId() != 0) {
                mTaskLog.log("fid", data.getUserId());
            }

            mTaskLog.close();
        }

        mTaskLog.close();

        Disk.writeExternalTextFile(fileName + ".xml", mTaskLog.toXml(), mContext);
    }

    private String getBuildInfo() {
        if (mFingerprintEngineering != null) {
            try {
                return mFingerprintEngineering.getBuildInfo();
            } catch (Exception e) {
                mLogger.w("getBuildInfo: " + e);
            }
        }
        return null;
    }

    private void writePng(String fileName, SensorImage image) {
        mLogger.enter("writePng");
        if (fileName != null) {
            File file = new File(fileName + ".png");
            FileOutputStream fos = null;
            try {
                mLogger.i("savePngFile: " + file);
                fos = new FileOutputStream(file);
                Bitmap bitmap = getBitmap(image, false);
                bitmap.compress(Bitmap.CompressFormat.PNG, 100, fos);
                bitmap.recycle();
            } catch (IOException e) {
                mLogger.e("IOException: " + e);
            } finally {
                try {
                    if (fos != null) {
                        fos.flush();
                        fos.close();
                    }
                } catch (IOException ignore) {
                }
            }
        }
        mLogger.exit("writePng");
    }

    private Bitmap getBitmap(SensorImage image, boolean rotate) {
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

    private String getFileName(SensorImage image) {
        if (mDir.exists() && mDir.isDirectory()) {
            int imageCounter = 0;
            File[] files = mDir.listFiles();
            if (files != null) {
                Arrays.sort(files, Collections.reverseOrder());
                for (File file : files) {
                    if (file.isFile()) {
                        String s = file.getName();
                        try {
                            imageCounter = Integer.parseInt(s.substring(s.indexOf("_") +
                                    1, s.lastIndexOf("_")));
                            break;
                        } catch (NumberFormatException e) {
                            mLogger.w("NumberFormatException: " + e);
                        }
                    }
                }
            }
            StringBuilder sb = new StringBuilder();
            sb.append(mDir.getAbsolutePath());
            sb.append("/");
            sb.append(new SimpleDateFormat("yyyy-MM-dd-HH.mm.ss", Locale.US).format(new Date()));
            sb.append(String.format(Locale.US, "_%04d_%dx%d", imageCounter + 1, image.getHeight(),
                    image.getWidth()));
            return sb.toString();
        } else {
            mLogger.w("dir not found: " + mDir.getPath());
        }
        return null;
    }
}
