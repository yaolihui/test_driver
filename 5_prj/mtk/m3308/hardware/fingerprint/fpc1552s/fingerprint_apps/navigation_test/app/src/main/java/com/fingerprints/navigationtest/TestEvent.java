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

import android.view.KeyEvent;

import com.fingerprints.navigationtest.helpers.NavigationInput;

import java.util.HashMap;

import static com.fingerprints.navigationtest.helpers.NavigationInput.CLICK;
import static com.fingerprints.navigationtest.helpers.NavigationInput.DOUBLE_CLICK;
import static com.fingerprints.navigationtest.helpers.NavigationInput.DOWN;
import static com.fingerprints.navigationtest.helpers.NavigationInput.HARD_PRESS;
import static com.fingerprints.navigationtest.helpers.NavigationInput.HOLD_CLICK;
import static com.fingerprints.navigationtest.helpers.NavigationInput.LEFT;
import static com.fingerprints.navigationtest.helpers.NavigationInput.RIGHT;
import static com.fingerprints.navigationtest.helpers.NavigationInput.UP;

//import static com.fingerprints.navigationtest.helpers.NavigationInput.KEYCODE_SOFT_PRESS;
//import static com.fingerprints.navigationtest.helpers.NavigationInput.SOFT_PRESS;

public class TestEvent {
    private final NavigationInput type;
    private static HashMap<NavigationInput, Integer> sRID;
    private static HashMap<NavigationInput, Integer> sIdentifiers; // from KeyEvent, e.g. 20

    static {
        sRID = new HashMap<>();
        sRID.put(UP, R.drawable.up);
        sRID.put(RIGHT, R.drawable.right);
        sRID.put(DOWN, R.drawable.down);
        sRID.put(LEFT, R.drawable.left);
        sRID.put(CLICK, R.drawable.click);
        sRID.put(HOLD_CLICK, R.drawable.hold_click);
        sRID.put(DOUBLE_CLICK, R.drawable.double_click);
        //sRID.put(SOFT_PRESS, R.drawable.soft_press);
        sRID.put(HARD_PRESS, R.drawable.hard_press);

        sIdentifiers = new HashMap<>();
        sIdentifiers.put(UP, KeyEvent.KEYCODE_DPAD_UP);
        sIdentifiers.put(RIGHT, KeyEvent.KEYCODE_DPAD_RIGHT);
        sIdentifiers.put(DOWN, KeyEvent.KEYCODE_DPAD_DOWN);
        sIdentifiers.put(LEFT, KeyEvent.KEYCODE_DPAD_LEFT);
        sIdentifiers.put(CLICK, KeyEvent.KEYCODE_BUTTON_A);
        sIdentifiers.put(HOLD_CLICK, KeyEvent.KEYCODE_BUTTON_B);
        sIdentifiers.put(DOUBLE_CLICK, KeyEvent.KEYCODE_BUTTON_C);
        sIdentifiers.put(HARD_PRESS, KeyEvent.KEYCODE_BUTTON_X);
        //sIdentifiers.put(SOFT_PRESS, KEYCODE_SOFT_PRESS);
    }

    public TestEvent(NavigationInput type) {
        this.type = type;
    }

    public int getKeyIdentifier() {
        return sIdentifiers.get(type);
    }

    public int getRID() {
        return sRID.get(type);
    }
}