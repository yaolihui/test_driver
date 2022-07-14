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
package com.fingerprints.imagecollection.scenario;

import android.content.Context;
import android.text.TextUtils;

import com.fingerprints.imagecollection.utils.Logger;
import com.fingerprints.imagecollection.values.FingerType;
import com.google.gson.annotations.Expose;

public class VerifyConfig {
    public static final String ANGLE = "angle";
    public static final String POSITION = "position";
    public static final String DESCRIPTION = "description";

    private static final int UNSET = -1;
    private ImageCollectionConfig mConfig;
    @Expose
    private boolean mSkipable = true;
    @Expose
    private boolean mEnabled;
    @Expose
    private int mAngle = 0;
    @Expose
    private int mNumberOfImages = UNSET;
    @Expose
    private String mPosition;
    @Expose
    private String mDescription;
    @Expose
    private String mPathExtra = null;

    public VerifyConfig(final int angle, final String position, final String description) {
        mAngle = angle;
        mPosition = position;
        mDescription = description;
        mEnabled = true;
    }

    public void setEnabled(final boolean enabled) {
        mEnabled = enabled;
    }

    public boolean isEnabled() {
        return mEnabled;
    }

    public void setNumberOfImages(final int numberOfImages) {
        mNumberOfImages = numberOfImages;
    }

    public void setConfig(final ImageCollectionConfig config) {
        this.mConfig = config;
    }

    public ImageCollectionConfig getConfig() {
        return mConfig;
    }

    public int getAngle() {
        return mAngle;
    }

    public int getNumberOfImages() {
        if (mNumberOfImages == UNSET) {
            return mConfig.getNumberOfImages();
        } else {
            return mNumberOfImages;
        }
    }

    public String getDescription() {
        return mDescription;
    }

    public String getFormattedDescription(final FingerType finger, Context context) {

        String desc = mDescription;

        desc = desc.replaceAll("%f", context.getString(finger.getNameResourceId()));

        return desc;
    }

    public String getPosition() {
        return mPosition;
    }

    public boolean hasDescription() {
        return mDescription != null && !mDescription.isEmpty();
    }

    public String getInfo() {
        return "Verify " + getPositionPretty() + (getAngle() != 0 ? " (" + getAngle() + "°)" : "");
    }

    private String getPositionPretty() {
        String pos = getPosition();
        if (pos != null && !pos.equals("")) {
            pos = pos.substring(0, 1).toUpperCase() + pos.substring(1);
        }
        return pos;
    }

    public void setSkipable(final boolean skipable) {
        mSkipable = skipable;
    }

    public boolean isSkipable() {
        return mSkipable;
    }

    public void setPathExtra(final String pathExtra) {
        mPathExtra = pathExtra;
    }

    public String getPathExtra() {
        if (mPathExtra != null && !mPathExtra.equals("")) {
            return mPathExtra;
        }

        String generatedPath = "";
        if (!"".equals(getPosition()) || getAngle() != 0) {
            generatedPath += "_";
        }

        if (getAngle() != 0) {
            generatedPath += getAngle();
        }

        generatedPath += getPosition();

        return generatedPath;
    }
}
