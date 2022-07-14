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
package com.fingerprints.imagetool.activities;

import android.app.Activity;
import android.os.Bundle;

import com.fingerprints.extension.navigation.FingerprintNavigation;
import com.fingerprints.imagetool.utils.Logger;

public class DisabledNavigationActivity extends Activity {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private FingerprintNavigation mFingerprintNavigation;
    private boolean mNavigationState = false;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        try {
            initNavigation();
            disableNavigation();
        } catch (Exception e) {
            mLogger.w("Exception: " + e);
        }
    }

    private void initNavigation() throws Exception {
        mFingerprintNavigation = new FingerprintNavigation();
        mNavigationState = mFingerprintNavigation.isEnabled();
        mLogger.d("navigation state is: " + mNavigationState);
    }

    private void disableNavigation() {
        if (mFingerprintNavigation != null && mFingerprintNavigation.isEnabled()) {
            mFingerprintNavigation.setNavigation(false);
        }
    }

    private void resetNavigation() {
        if (mFingerprintNavigation != null) {
            mLogger.d("setting navigation: " + mNavigationState);
            mFingerprintNavigation.setNavigation(mNavigationState);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        resetNavigation();
    }

    @Override
    protected void onResume() {
        super.onResume();
        disableNavigation();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        resetNavigation();
    }
}
