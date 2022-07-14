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

import com.fingerprints.imagecollection.imageutils.ImageData;
import com.fingerprints.imagecollection.values.Preferences;
import com.fingerprints.extension.util.FpcError;

import java.io.File;
import java.util.Locale;

public abstract class ImageWriter {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private Context mContext;
    protected String mExtension;
    private TaskLog mTaskLog;


    public ImageWriter(final Context context, final String extension, final TaskLog taskLog) {
        mContext = context;
        mExtension = extension;
        mTaskLog = taskLog;
    }

    public ImageWriter() {
    }

    public Context getContext() {
        return mContext;
    }

    public TaskLog getTaskLog() {
        return mTaskLog;
    }

    public File getDirectory(final ImageData imageData) {
        String dirPath = Preferences.getCurrentDir(mContext);

        if (dirPath.trim().length() != 0) {
            File dir = new File(dirPath);
            if (dir.exists()) {
                String path = Preferences.folderNameForImageData(imageData);
                if (imageData.hasVerifyConfig()) {
                    path += imageData.getVerifyConfig().getPathExtra();
                }

                File mSubDir = new File(dir, path);
                if (mSubDir.exists() || mSubDir.mkdir()) {
                    return mSubDir;
                }
            }
        }
        return null;
    }

    public String getFileName(ImageData imageData) {
        StringBuilder sb = new StringBuilder();
        sb.append(imageData.getTimeString());
        if (imageData.getCaptureStatus() == ImageData.CaptureStatus.REJECTED ||
            imageData.getCaptureStatus() == ImageData.CaptureStatus.ERROR) {
            if (imageData.hasEnrollSession() &&
                imageData.getEnrollResult().getExternalEnum() == FpcError.Error.FPC_STATUS_ENROLL_LOW_MOBILITY) {
                sb.append(String.format(Locale.US, "m" + "_%02d_%03d",
                        imageData.getFingerType().getFingerId(), imageData.getSampleId()));
            } else {
                sb.append(String.format(Locale.US, "r" + "_%02d_%03d",
                    imageData.getFingerType().getFingerId(), imageData.getSampleId()));
            }
        } else {
            sb.append(String.format(Locale.US, "_%02d_%03d",
                imageData.getFingerType().getFingerId(), imageData.getSampleId()));
        }
        sb.append(".");
        sb.append(mExtension);

        return sb.toString();
    }

    public abstract void write(final ImageData imageData, String buildInfo) throws Exception;
}
