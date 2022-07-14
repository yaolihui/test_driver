/*
 *
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */
package com.fingerprints.sensortesttool.testcases;

import android.view.View;

import com.fingerprints.sensortesttool.ITestController;

public abstract class AUITestCase extends ATestCase {

    public AUITestCase(final String name, final String id, final ITestController controller) {
        super(name, id, controller);
    }

    public void onTestWillDisplay() {

    }

    public abstract View getView();
}
