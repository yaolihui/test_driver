/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.navigation;

import java.lang.reflect.Field;
import com.fingerprints.extension.util.Logger;

public class NavigationConfig {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    public int tapNoImageMaxThreshold;
    public int holdNoImageMinThreshold;
    public int doubleClickTimeInterval;
    public int tapImageTransMaxThreshold;
    public int swipeImageTransMinThreshold;
    public int backGroundAlgo;

    public NavigationConfig() {
        for (Field f: NavigationConfig.class.getFields()) {
            try {
                f.set(this, 0);
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            }
        }
    }

    public NavigationConfig(NavigationConfig objToCopy) throws IllegalAccessException {
        try {
            for (Field fromField : objToCopy.getClass().getFields()) {
                Field toField =this.getClass().getDeclaredField(fromField.getName());
                toField.set(this, fromField.get( objToCopy));
            }
        } catch (NoSuchFieldException | IllegalAccessException e) {
            e.printStackTrace();
            throw(new IllegalAccessException(e.getMessage()));
        }
    }

    NavigationConfig(com.fingerprints.extension.V1_0.NavigationConfig config) {
        this.tapNoImageMaxThreshold = config.tapNoImageMaxThreshold;
        this.holdNoImageMinThreshold      = config.holdNoImageMinThreshold;
        this.doubleClickTimeInterval     = config.doubleClickTimeInterval;
        this.tapImageTransMaxThreshold           = config.tapImageTransMaxThreshold;
        this.swipeImageTransMinThreshold        = config.swipeImageTransMinThreshold;
        this.backGroundAlgo      = config.backGroundAlgo;
    }

    com.fingerprints.extension.V1_0.NavigationConfig getHidlConfig() {
        com.fingerprints.extension.V1_0.NavigationConfig config =
            new com.fingerprints.extension.V1_0.NavigationConfig();

        config.tapNoImageMaxThreshold = this.tapNoImageMaxThreshold;
        config.holdNoImageMinThreshold      = this.holdNoImageMinThreshold;
        config.doubleClickTimeInterval     = this.doubleClickTimeInterval;
        config.tapImageTransMaxThreshold           = this.tapImageTransMaxThreshold;
        config.swipeImageTransMinThreshold        = this.swipeImageTransMinThreshold;
        config.backGroundAlgo      = this.backGroundAlgo;
        return config;
    }

    public void print() {
        StringBuilder stringBuilder = new StringBuilder();
        for (Field f: this.getClass().getFields()) {
            try {
                stringBuilder.append(f.getName() + ": " + f.get(this));
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            }
        }
        mLogger.d(stringBuilder.toString());
    }
}
