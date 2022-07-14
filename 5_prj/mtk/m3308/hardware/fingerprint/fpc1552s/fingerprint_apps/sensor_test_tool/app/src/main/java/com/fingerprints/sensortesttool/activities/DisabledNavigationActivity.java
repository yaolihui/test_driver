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
package com.fingerprints.sensortesttool.activities;

import android.app.Activity;
import android.os.Bundle;

import com.fingerprints.extension.navigation.FingerprintNavigation;
import com.fingerprints.sensortesttool.logging.Logger;

public class DisabledNavigationActivity extends Activity {
    private FingerprintNavigation mFingerprintNavigation;
    private boolean mNavigationState = false;
    private Logger mLog;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mLog = new Logger(getClass().getSimpleName());

        try {
            initNavigation();
            disableNavigation();
        } catch (Exception e) {
            mLog.e("Navigation library not found.");
        }
    }

    private void initNavigation() throws Exception {
        mFingerprintNavigation = new FingerprintNavigation();
        mNavigationState = mFingerprintNavigation.isEnabled();
        mLog.d("navigation state is: " + mNavigationState);
    }

    private void disableNavigation() {
        if (mFingerprintNavigation != null && mFingerprintNavigation.isEnabled()) {
            mFingerprintNavigation.setNavigation(false);
        }
    }

    private void resetNavigation() {
        if (mFingerprintNavigation != null) {
            mLog.d("setting navigation: " + mNavigationState);
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
