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

package com.fingerprints.imagecollection.values;

public enum ImageCollectionState {

    STARTED("Started"),
    REQUEST_TOKEN("RequestToken"),
    ENROLL("Enroll"),
    VERIFY("Verify"),
    STOPPED("Stopped");

    private String mName;

    private ImageCollectionState(String name) {
        mName = name;
    }

    public String getName() {
        return mName;
    }

    public boolean inCollectImageState() {
        return isEnroll() || isVerify();
    }

    public boolean isEnroll() {
        return this == ENROLL;
    }

    public boolean isVerify() {
        return this == VERIFY;
    }

    public boolean isRunning() {
        return this != STOPPED;
    }
}
