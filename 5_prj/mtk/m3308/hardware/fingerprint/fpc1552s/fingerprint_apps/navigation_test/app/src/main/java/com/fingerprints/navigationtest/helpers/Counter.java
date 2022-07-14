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
package com.fingerprints.navigationtest.helpers;

public class Counter {
    private int mValue = 0;

    public void inc() {
        if (mValue < Integer.MAX_VALUE) {
            mValue++;
        }
    }

    public void inc(final int value) {
        int newValue = mValue + value;
        if (newValue < Integer.MAX_VALUE) {
            mValue += value;
        } else {
            mValue = Integer.MAX_VALUE;
        }
    }

    public void set(final int value) {
        mValue = value;
    }

    public int get() {
        return mValue;
    }

    public void reset() {
        mValue = 0;
    }
}