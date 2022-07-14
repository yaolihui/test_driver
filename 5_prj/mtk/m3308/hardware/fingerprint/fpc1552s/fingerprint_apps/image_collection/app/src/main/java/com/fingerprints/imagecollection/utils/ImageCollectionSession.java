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

import com.fingerprints.imagecollection.imageutils.VerifyStatistics;
import com.fingerprints.imagecollection.scenario.ImageCollectionConfig;
import com.fingerprints.imagecollection.scenario.VerifyConfig;
import com.fingerprints.imagecollection.values.Preferences;

public class ImageCollectionSession {
    private Context mContext;
    private TaskLog mTaskLog;
    private ImageCollectionConfig mConfig;

    public ImageCollectionSession(final ImageCollectionConfig config, final Context context) {
        mTaskLog = new TaskLog();
        mConfig = config;
        mContext = context;
    }

    public ImageCollectionConfig getConfig() {
        return mConfig;
    }

    public TaskLog getTaskLog() {
        return mTaskLog;
    }

    public void start() {
        getTaskLog().reset();
        getTaskLog().open("ImageCollection").logTime().log("device", android.os.Build.MODEL);
    }

    public void end() {
        String path = Preferences.getCurrentDir(mContext);
        Disk.writeExternalTextFile(path + "/log.xml", getTaskLog().toXml(), mContext);
    }

    public void startVerifySession(final VerifyConfig verifyConfig) {
        getTaskLog().open("Verify").
                log("position", verifyConfig.getPosition()).
                log("angle", verifyConfig.getAngle()).
                log("numberImages", verifyConfig.getNumberOfImages()).
                log("description", verifyConfig.getDescription());
    }

    public void endVerifySession(final VerifyStatistics verifyStatistics) {
        if (verifyStatistics.hasDecisionFeedback()) {
            getTaskLog().log("decisionFeedback", true).log("accepted", verifyStatistics.getAccepted()).
                    log("rejected", verifyStatistics.getRejected());
        }
    }
}
