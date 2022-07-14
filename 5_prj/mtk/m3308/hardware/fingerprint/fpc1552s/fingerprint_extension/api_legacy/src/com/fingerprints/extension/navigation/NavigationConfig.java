/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.navigation;

import android.os.Parcel;
import android.os.Parcelable;
import android.content.pm.PackageManager;

import java.lang.reflect.Field;

import com.fingerprints.extension.util.Logger;

public class NavigationConfig implements Parcelable {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    public int tapNoImageMaxThreshold;
    public int holdNoImageMinThreshold;
    public int doubleClickTimeInterval;
    public int tapImageTransMaxThreshold;
    public int swipeImageTransMinThreshold;
    public int backGroundAlgo;

    public NavigationConfig() {
        for (Field f : NavigationConfig.class.getFields()) {
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
                Field toField = this.getClass().getDeclaredField(fromField.getName());
                toField.set(this, fromField.get(objToCopy));
            }
        } catch (NoSuchFieldException | IllegalAccessException e) {
            e.printStackTrace();
        }
    }

    public void print() {
        StringBuilder stringBuilder = new StringBuilder();
        for (Field f : this.getClass().getFields()) {
            try {
                stringBuilder.append(f.getName() + ": " + f.get(this));
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            }
        }
        mLogger.d(stringBuilder.toString());
    }

    private NavigationConfig(Parcel source) throws IllegalAccessException {
        readFromParcel(source);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public void readFromParcel(Parcel source) throws IllegalAccessException {
        for (Field f : this.getClass().getFields()) {
            f.set(this, source.readInt());
        }
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        for (Field f : this.getClass().getFields()) {
            try {
                dest.writeInt(Integer.parseInt(f.get(this).toString()));
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            }
        }
    }

    public static final Parcelable.Creator<NavigationConfig> CREATOR =
    new Parcelable.Creator<NavigationConfig>() {
        @Override
        public NavigationConfig createFromParcel(Parcel source) {
            try {
                return new NavigationConfig(source);
            } catch (IllegalAccessException e) {
                return null;
            }
        }

        @Override
        public NavigationConfig[] newArray(int size) {
            return new NavigationConfig[size];
        }
    };
}
