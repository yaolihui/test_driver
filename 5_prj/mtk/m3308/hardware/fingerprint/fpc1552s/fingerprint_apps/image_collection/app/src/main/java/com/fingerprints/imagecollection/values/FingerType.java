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

package com.fingerprints.imagecollection.values;

import com.fingerprints.imagecollection.R;

public enum FingerType {

    L4("L4", 4, R.string.left_little_finger, false),
    L3("L3", 3, R.string.left_ring_finger, true),
    L2("L2", 2, R.string.left_middle_finger, true),
    L1("L1", 1, R.string.left_index_finger, true),
    L0("L0", 0, R.string.left_thumb, true),
    R0("R0", 5, R.string.right_thumb, true),
    R1("R1", 6, R.string.right_index_finger, true),
    R2("R2", 7, R.string.right_middle_finger, true),
    R3("R3", 8, R.string.right_ring_finger, true),
    R4("R4", 9, R.string.right_little_finger, false);

    //values from https://fpc-jira.fingerprint.local/jira/browse/ALGO-3541
    private final int mFingerId;
    private final String mId;
    private final int mNameResourceId;
    private final boolean mDefaultFingerSelect;

    private FingerType(String id, int fingerId, int nameResourceId, boolean defaultFingerSelect) {
        mId = id;
        mFingerId = fingerId;
        mNameResourceId = nameResourceId;
        mDefaultFingerSelect = defaultFingerSelect;
    }


    public int getFingerId() {
        return mFingerId;
    }

    public String getId() {
        return mId;
    }

    public int getNameResourceId() {
        return mNameResourceId;
    }


    public boolean getDefaultFingerSelect() {
        return mDefaultFingerSelect;
    }

    public static FingerType getFingerType(int value) {
        return FingerType.values()[value];
    }

    public static FingerType getFingerTypeFromId(int id) {
        for (FingerType f : FingerType.values()) {
            if (f.getFingerId() == id) {
                return f;
            }
        }
        throw new RuntimeException("Invalid finger id: " + id);
    }
}
