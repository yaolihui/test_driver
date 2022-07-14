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

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

import com.fingerprints.imagesubscription.R;

public class Preferences {

    public static boolean isEnabled(Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        return prefs.getBoolean(context.getString(R.string.enable_key), false);
    }

    public static void setEnabled(Context context, boolean value) {
        SharedPreferences.Editor editor;
        editor = PreferenceManager.getDefaultSharedPreferences(context).edit();
        editor.putBoolean(context.getString(R.string.enable_key), value);
        editor.commit();
    }

    public static boolean isEnabledOnBoot(Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        return prefs.getBoolean(context.getString(R.string.enable_on_boot_key), false);
    }

    public static void setEnabledOnBoot(Context context, boolean value) {
        SharedPreferences.Editor editor;
        editor = PreferenceManager.getDefaultSharedPreferences(context).edit();
        editor.putBoolean(context.getString(R.string.enable_on_boot_key), value);
        editor.commit();
    }

    public static boolean getSavePng(Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        return prefs.getBoolean(context.getString(R.string.save_png_key), false);
    }

    public static void setSavePng(Context context, boolean value) {
        SharedPreferences.Editor editor;
        editor = PreferenceManager.getDefaultSharedPreferences(context).edit();
        editor.putBoolean(context.getString(R.string.save_png_key), value);
        editor.commit();
    }

    public static String getFolder(Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        String s = context.getString(R.string.default_folder);
        return prefs.getString(context.getString(R.string.folder_key), s).trim();
    }

    public static void setFolder(Context context, String value) {
        SharedPreferences.Editor editor;
        editor = PreferenceManager.getDefaultSharedPreferences(context).edit();
        editor.putString(context.getString(R.string.folder_key), value);
        editor.commit();
    }
}
