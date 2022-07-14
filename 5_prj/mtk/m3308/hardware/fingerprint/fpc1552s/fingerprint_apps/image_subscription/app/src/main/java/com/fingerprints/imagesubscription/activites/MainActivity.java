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

package com.fingerprints.imagesubscription.activites;

import android.Manifest;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.preference.EditTextPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.SwitchPreference;

import com.fingerprints.imagesubscription.R;
import com.fingerprints.imagesubscription.services.ImageSubscriptionService;
import com.fingerprints.imagesubscription.utils.Constants;
import com.fingerprints.imagesubscription.utils.Logger;
import com.fingerprints.imagesubscription.utils.Preferences;

public class MainActivity extends Activity {
    private static Logger mLogger = new Logger("MainActivity");
    private ImageSubscriptionFragment mFragment;
    private boolean isRunning = false;
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            mLogger.enter("onReceive");
            String action = intent.getAction();
            if (action.equals(Constants.ACTION_ACTIVITY_REFRESH)) {
                if (isRunning) {
                    getFragmentManager().beginTransaction().detach(mFragment).attach(mFragment).
                            commit();
                }
            }
            mLogger.exit("onReceive");
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mLogger.enter("onCreate");
        mFragment = new ImageSubscriptionFragment();
        getFragmentManager().beginTransaction().replace(android.R.id.content, mFragment).commit();
        if (checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {
            mLogger.i("Requesting WRITE_EXTERNAL_STORAGE permission");
            requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1234);
        }
        registerReceiver(mReceiver, new IntentFilter(Constants.ACTION_ACTIVITY_REFRESH));
        mLogger.exit("onCreate");
    }

    @Override
    public void onResume() {
        super.onResume();
        mLogger.enter("onResume");
        isRunning = true;
        mLogger.exit("onResume");
    }

    @Override
    public void onPause() {
        super.onPause();
        mLogger.enter("onPause");
        isRunning = false;
        mLogger.exit("onPause");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mReceiver);
    }

    public static class ImageSubscriptionFragment extends PreferenceFragment implements
            Preference.OnPreferenceChangeListener {
        private SwitchPreference mEnableSwitch;
        private SwitchPreference mSavePngSwitch;
        private EditTextPreference mFolderEditText;

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            mLogger.enter("onCreate");
            mLogger.exit("onCreate");
        }

        @Override
        public void onResume() {
            super.onResume();
            mLogger.enter("onResume");
            if (getPreferenceScreen() != null) {
                getPreferenceScreen().removeAll();
            }
            addPreferencesFromResource(R.xml.preferences);

            mEnableSwitch = (SwitchPreference) findPreference(getContext()
                    .getString(R.string.enable_key));
            mSavePngSwitch = (SwitchPreference) findPreference(getContext()
                    .getString(R.string.save_png_key));
            mFolderEditText = (EditTextPreference) findPreference(getContext()
                    .getString(R.string.folder_key));

            mEnableSwitch.setOnPreferenceChangeListener(this);
            mSavePngSwitch.setOnPreferenceChangeListener(this);
            mFolderEditText.setOnPreferenceChangeListener(this);

            mFolderEditText.setSummary(Preferences.getFolder(getActivity()));
            mFolderEditText.setEnabled(false);
            mLogger.exit("onResume");
        }

        @Override
        public boolean onPreferenceChange(Preference pref, Object objValue) {
            mLogger.enter("onPreferenceChange");
            String key = pref.getKey();
            if (key.equals(getContext().getString(R.string.enable_key))) {
                Context context = pref.getContext().getApplicationContext();
                Intent intent;
                if ((Boolean) objValue) {
                    intent = new Intent(Constants.ACTION_START);
                } else {
                    intent = new Intent(Constants.ACTION_STOP);
                }
                intent.setClass(context, ImageSubscriptionService.class);
                context.startService(intent);

                mFolderEditText.setEnabled(false);
            } else if (key.equals(getContext().getString(R.string.folder_key))) {
                pref.setSummary(objValue.toString());
            }
            mLogger.exit("onPreferenceChange");
            return true;
        }
    }
}
