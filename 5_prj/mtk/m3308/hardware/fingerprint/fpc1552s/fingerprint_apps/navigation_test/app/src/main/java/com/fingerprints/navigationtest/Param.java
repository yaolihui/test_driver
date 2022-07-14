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

package com.fingerprints.navigationtest;

class Param {

    private final String name;
    private int value;

    public Param(String name, int value) {
        this.name = name;
        this.value = value;
    }

    public String getParamName() {
        return name;
    }

    public int getParamValue() {
        return value;
    }

    public void setParamValue(int newValue) {
        this.value = newValue;
    }

    public String toString() {
        return name + " " + value;
    }
}