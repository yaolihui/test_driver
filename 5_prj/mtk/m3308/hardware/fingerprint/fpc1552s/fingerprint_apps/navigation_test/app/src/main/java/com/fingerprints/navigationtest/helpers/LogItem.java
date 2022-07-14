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

import java.util.ArrayList;
import java.util.List;

public class LogItem {
    private String mHeader;
    private String mValue;
    private List<LogItem> mItems = new ArrayList<>();

    public LogItem(final String header, final Object value) {
        mHeader = header;
        mValue = value != null ? value.toString() : "null";
    }

    public String getHeader() {
        return mHeader;
    }

    public String getValue() {
        return mValue;
    }

    public List<LogItem> getItems() {
        return mItems;
    }

    public void add(LogItem item) {
        mItems.add(item);
    }

    public void add(String header, Object item) {
        mItems.add(new LogItem(header, item));
    }
}
