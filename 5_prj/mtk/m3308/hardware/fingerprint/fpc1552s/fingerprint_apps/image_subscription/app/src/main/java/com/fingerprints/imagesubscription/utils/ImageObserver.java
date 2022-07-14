/*
 *
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */

package com.fingerprints.imagesubscription.utils;

import android.os.FileObserver;

public class ImageObserver extends FileObserver {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private static final int NOTIFY_EVENTS = /*CLOSE_WRITE | MOVED_TO | */MOVED_FROM | DELETE |
            DELETE_SELF;

    public ImageObserver(String path) {
        super(path, NOTIFY_EVENTS);
        mLogger.enter("ImageObserver");
        mLogger.exit("ImageObserver");
    }

    @Override
    public void onEvent(int event, String path) {
        mLogger.enter("onEvent ");
        // TODO: Android M is not supporting below events from sdcard, add this later.
        /*if (((CLOSE_WRITE & event) != 0) || ((MOVED_TO & event) != 0)) {
            synchronized (this) {
                mLogger.d("added file: " + path);
                notify();
            }
        } else*/
        if (((DELETE & event) != 0) || ((DELETE_SELF & event) != 0) ||
                ((MOVED_FROM & event) != 0)) {
            mLogger.i("deleted file: " + path);
        }
        mLogger.exit("onEvent");
    }
}
