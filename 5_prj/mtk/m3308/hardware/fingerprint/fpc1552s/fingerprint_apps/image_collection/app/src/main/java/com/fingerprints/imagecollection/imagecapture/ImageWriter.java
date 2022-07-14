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
package com.fingerprints.imagecollection.imagecapture;

import android.content.Context;

import com.fingerprints.imagecollection.imageutils.Enroll;
import com.fingerprints.imagecollection.imageutils.ImageData;
import com.fingerprints.imagecollection.utils.FMIWriter;
import com.fingerprints.imagecollection.utils.Logger;
import com.fingerprints.imagecollection.utils.TaskLog;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class ImageWriter {
    private final ExecutorService mImageSaverExecutor = Executors.newFixedThreadPool(1);
    private Logger mLogger;
    private static ImageWriter mInstance;

    public interface IImageWriterProgressCallback {
        public void beganWriting(int max);

        public void writingImage(int current, int max);

        public void doneWriting();
    }

    public static ImageWriter getInstance() {
        if (mInstance == null) {
            mInstance = new ImageWriter();
        }
        return mInstance;
    }

    private ImageWriter() {
        mLogger = new Logger(getClass().getSimpleName());
    }

    public synchronized void write(final Enroll enroll,
            final Context context,
            final IImageWriterProgressCallback progressCallback,
            final TaskLog taskLog, final String buildInfo) throws Exception {

        int current = 0;

        progressCallback.beganWriting(enroll.getTotalNumberSamples());
        for (Enroll.EnrollSession enrollSession : enroll.getEnrollSessions()) {
            if (enrollSession.hasData()) {
                for (int i = 0; i < enrollSession.getImageDataList().size(); i++) {
                    progressCallback.writingImage(current + 1, enroll.getTotalNumberSamples());
                    write(enrollSession.getImageDataList().get(i), context, taskLog, buildInfo);
                    current++;
                }
            } else {
                mLogger.e("enroll has no data.");
            }
        }
        progressCallback.doneWriting();

    }

    public synchronized void write(final ImageData data, final Context context,
            final IImageWriterProgressCallback progressCallback,
            final TaskLog taskLog, final String buildInfo) throws Exception {

        progressCallback.beganWriting(1);
        progressCallback.writingImage(0, 1);
        write(data, context, taskLog, buildInfo);
        progressCallback.doneWriting();

    }

    private synchronized void write(final ImageData data, final Context context,
            final TaskLog taskLog, final String buildInfo) throws Exception {

        final FMIWriter fmiWriter = new FMIWriter(context, taskLog);
        fmiWriter.write(data, buildInfo);

    }
}
