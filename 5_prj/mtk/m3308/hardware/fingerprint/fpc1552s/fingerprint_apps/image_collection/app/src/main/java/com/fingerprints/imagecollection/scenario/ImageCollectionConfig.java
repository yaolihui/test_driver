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

import com.fingerprints.imagecollection.values.FingerType;
import com.google.gson.annotations.Expose;

import java.util.ArrayList;

public class ImageCollectionConfig {
    public static final String LEFT_INDEXES = "leftIndexes";
    public static final String RIGHT_INDEXES = "rightIndexes";
    public static final String NUMBER_OF_IMAGES = "numberOfImages";
    public static final String IMAGE_DISPLAY = "imageDisplay";
    public static final String DECISION_FEEDBACK = "decisionFeedback";

    @Expose
    private ArrayList<FingerType> mFingerIndexes;
    @Expose
    private boolean mImageDisplay = false;
    @Expose
    private boolean mDecisionFeedback = false;
    @Expose
    private int mNumberOfImages = 0;
    @Expose
    private ArrayList<VerifyConfig> mVerifyConfigList;

    public ImageCollectionConfig() {
        mFingerIndexes = new ArrayList<>();
        mVerifyConfigList = new ArrayList<>();
    }

    public ImageCollectionConfig(final String leftIndexes, final String rightIndexes, final int numberOfImages, final boolean imageDisplay, final boolean decisionFeedback) {
        this();
        ArrayList<FingerType> mLeftIndexes = parseIndexes(leftIndexes, false);
        ArrayList<FingerType> mRightIndexes = parseIndexes(rightIndexes, true);

        mFingerIndexes.addAll(mLeftIndexes);
        mFingerIndexes.addAll(mRightIndexes);

        mImageDisplay = imageDisplay;
        mDecisionFeedback = decisionFeedback;
        mNumberOfImages = numberOfImages;
    }

    private ArrayList<FingerType> parseIndexes(final String indexes, boolean right) {
        if (indexes.equals("")) {
            return new ArrayList<>();
        }

        String[] values = indexes.split(",");
        ArrayList<FingerType> list = new ArrayList<>();
        for (String value : values) {
            list.add(FingerType.getFingerTypeFromId(Integer.valueOf(value) + (right ? 5 : 0)));
        }
        return list;
    }

    public void add(final VerifyConfig verifyConfig) {
        verifyConfig.setConfig(this);
        mVerifyConfigList.add(verifyConfig);
    }

    public ArrayList<VerifyConfig> getVerifyConfigList() {
        return mVerifyConfigList;
    }

    public int getNumberOfImages() {
        return mNumberOfImages;
    }

    public void setNumberOfImages(final int numberOfImages) {
        mNumberOfImages = numberOfImages;
    }

    public int getNumberOfFingers() {
        return mFingerIndexes.size();
    }

    public boolean hasFingers() {
        return getNumberOfFingers() > 0;
    }

    public boolean isValid() {
        return getNumberOfFingers() > 0 && getNumberOfImages() > 0;
    }

    public boolean hasFinger(final FingerType finger) {
        return mFingerIndexes.contains(finger);
    }

    public void addFinger(final FingerType... fingers) {
        for (FingerType f : fingers) {
            if (!mFingerIndexes.contains(f)) {
                mFingerIndexes.add(f);
            }
        }
    }

    public void removeFinger(final FingerType finger) {
        if (mFingerIndexes.contains(finger)) {
            mFingerIndexes.remove(finger);
        }
    }

    public VerifyConfig getVerifyConfigByDescription(final String desc) {
        for (VerifyConfig v : getVerifyConfigList()) {
            if (desc.equals(v.getDescription())) {
                return v;
            }
        }
        return null;
    }

    public boolean getImageDisplay() {
        return mImageDisplay;
    }

    public boolean getDecisionFeedback() {
        return mDecisionFeedback;
    }

    public void setDecisionFeedback(final boolean decisionFeedback) {
        mDecisionFeedback = decisionFeedback;
    }
}
