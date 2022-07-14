/*
 *
 * Copyright (c) 2016-2021 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */

package com.fingerprints.imagecollection.values;

public class Constants {
    public static final int DEFAULT_VERIFY_COUNT_FOR_OLD_SENSORS = 80;
    public static final int DEFAULT_VERIFY_COUNT_FOR_1290 = 120;
    public static final int DEFAULT_VERIFY_COUNT = 140;
    public static final int HWID_FPC1290_DEVICE_ID = 0x0E00;
    public static final int HWID_FPC1510_DEVICE_ID = 0x1000;
    public static final int HWID_FPC1540_DEVICE_ID = 0x1800;
    public static final int HWID_FPC1542_DEVICE_ID = 0x1C00;
    public static final int HWID_FPC1542SA_DEVICE_ID = 0x1F00;
    public static final int HWID_FPC1552_DEVICE_ID = 0x1E00;
    public static final int HWID_FPC1553_DEVICE_ID = 0x2000;
    public static final int MAX_VERIFY_COUNT = 200;
    public static final int MAX_ENROLL_RETRY = 3;
    public static final int FINGER_COUNT = 10;
    public static final int ENROLL_GET_TOKEN = 1;
    public static final String FPC_FOLDER = "Fingerprints";
    // To comply with BET use "Enrol" instead of "Enroll" in SW21.
    public static final String ENROLL_FAIL_DIR = "Enrol_FTE";
    public static final String ENROLL_DIR = "Enrol";
    public static final String VERIFY_DIR = "Verify";
    public static final String VERIFY_ORTHOGONAL_DIR = "VerifyOrthogonal";
    public static final String PROPERTY_FINGERPRINTS_CAPTURE_ENABLED = "persist.vendor.fpc.ict.capture";
}
