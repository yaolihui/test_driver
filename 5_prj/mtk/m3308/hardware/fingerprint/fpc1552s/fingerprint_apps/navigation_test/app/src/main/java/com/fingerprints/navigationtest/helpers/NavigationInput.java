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

import android.view.KeyEvent;

public enum NavigationInput {
    UP, RIGHT, DOWN, LEFT, CLICK, HOLD_CLICK, DOUBLE_CLICK, /*SOFT_PRESS,*/ HARD_PRESS;

    //public static final int KEYCODE_SOFT_PRESS = 9000;
    public static final int KEYCODE_FAIL_TO_REGISTER = 9002;

    public static NavigationInput fromKeyCode(final int keyCode) {
        switch (keyCode) {
            case KeyEvent.KEYCODE_DPAD_UP:
                return UP;

            case KeyEvent.KEYCODE_DPAD_RIGHT:
                return RIGHT;

            case KeyEvent.KEYCODE_DPAD_DOWN:
                return DOWN;

            case KeyEvent.KEYCODE_DPAD_LEFT:
                return LEFT;

            case KeyEvent.KEYCODE_BUTTON_A:
                return CLICK;

            case KeyEvent.KEYCODE_BUTTON_B:
                return HOLD_CLICK;

            case KeyEvent.KEYCODE_BUTTON_C:
                return DOUBLE_CLICK;

            //case KEYCODE_SOFT_PRESS:
            //    return SOFT_PRESS;

            case KeyEvent.KEYCODE_BUTTON_X:
                return HARD_PRESS;
        }

        throw new RuntimeException("invalid keycode " + keyCode);
    }

    public String toString() {
        switch (this) {
            case UP:
                return "Up";
            case RIGHT:
                return "Right";
            case DOWN:
                return "Down";
            case LEFT:
                return "Left";
            case CLICK:
                return "Click";
            case HOLD_CLICK:
                return "Hold Click";
            case DOUBLE_CLICK:
                return "Double Click";
            //case SOFT_PRESS:
            //    return "Soft Press";
            case HARD_PRESS:
                return "Force Press";
        }

        return "Unknown";
    }
}