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

public class Holder<T> {
    private T mObject;

    public Holder(T object) {
        mObject = object;
    }

    public Holder() {
        mObject = null;
    }

    public T get() {
        return mObject;
    }

    public void set(final T object) {
        mObject = object;
    }
}
