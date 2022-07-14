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

import android.content.Context;
import android.content.SharedPreferences;

import com.fingerprints.imagecollection.imageutils.ImageData;
import com.fingerprints.imagecollection.scenario.ImageCollectionConfig;
import com.fingerprints.imagecollection.scenario.VerifyConfig;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;

import java.util.ArrayList;

public class Preferences {
    public static final String SCENARIO_CONFIG_NAME = "fpc_scenario_config.xml";

    private static final String SHARED_PREFERENCES_NAME = "configuration";
    private static final String KEY_CONFIGURATION = "finger_select_";
    private static final String KEY_ENROLLED_FINGERPRINT_IDS = "enrolled_fingerprint_ids";
    private static final String KEY_CURRENT_DIR = "current_dir";

    private Preferences() {
    }

    private static SharedPreferences getPref(Context context) {
        return context.getSharedPreferences(SHARED_PREFERENCES_NAME, Context.MODE_PRIVATE);
    }

    /**
     * Makes sure we keep track of the fingerprints the app has enrolled so that we can remove them.
     */
    public static void addAppErolledFingerprint(int id, final Context context) {
        ArrayList<String> list = getAppEnrolledFingerprintIds(context);
        list.add("" + id);

        GsonBuilder gsonBuilder = new GsonBuilder();
        Gson gson = gsonBuilder.excludeFieldsWithoutExposeAnnotation().create();
        String json = gson.toJson(list);
        getPref(context).edit().putString(KEY_ENROLLED_FINGERPRINT_IDS, json).apply();
    }

    /**
     * Notifies that we have removed an enrolled fingerprint.
     */
    public static void removeAppErolledFingerprint(int id, final Context context) {
        ArrayList<String> list = getAppEnrolledFingerprintIds(context);

        for (int i = list.size() - 1; i >= 0; i--) {
            if (list.get(i).equals("" + id)) {
                list.remove(i);
            }
        }

        GsonBuilder gsonBuilder = new GsonBuilder();
        Gson gson = gsonBuilder.excludeFieldsWithoutExposeAnnotation().create();
        String json = gson.toJson(list);
        getPref(context).edit().putString(KEY_ENROLLED_FINGERPRINT_IDS, json).apply();
    }


    public static ArrayList<String> getAppEnrolledFingerprintIds(final Context context) {
        Gson gson = new Gson();
        String json = getPref(context).getString(KEY_ENROLLED_FINGERPRINT_IDS, "");

        if (json != null && !json.equals("")) {
            return gson.fromJson(json, ArrayList.class);
        } else {
            return new ArrayList<String>();
        }
    }

    public static void saveConfig(final Context context, final ImageCollectionConfig config) {
        GsonBuilder gsonBuilder = new GsonBuilder();
        Gson gson = gsonBuilder.excludeFieldsWithoutExposeAnnotation().create();
        String json = gson.toJson(config);
        getPref(context).edit().putString(KEY_CONFIGURATION, json).apply();
    }

    public static ImageCollectionConfig getConfig(final Context context) {
        Gson gson = new Gson();
        String json = getPref(context).getString(KEY_CONFIGURATION, "");
        if (json != null && !json.equals("")) {
            ImageCollectionConfig obj = gson.fromJson(json, ImageCollectionConfig.class);

            //cant save this property, so need to fix it when loading.
            for (VerifyConfig v : obj.getVerifyConfigList()) {
                v.setConfig(obj);
            }

            return obj;
        }

        return null;
    }

    public static String getCurrentDir(Context context) {
        return getPref(context).getString(KEY_CURRENT_DIR, "");
    }

    public static void setCurrentDir(Context context, String value) {
        getPref(context).edit().putString(KEY_CURRENT_DIR, value).apply();
    }

    public static String folderNameForImageData(ImageData imageData) {
        if (imageData.hasEnrollSession()) {
            if (imageData.getEnrollSession().getStatus().isFailed()) {
                return Constants.ENROLL_FAIL_DIR;
            }
        }

        switch (imageData.getImageCollectionState()) {
            case ENROLL: {
                return Constants.ENROLL_DIR;
            }
            case VERIFY: {
                return Constants.VERIFY_DIR;
            }
            default: {
                return "error_" + imageData.getImageCollectionState().getName();
            }
        }
    }
}
