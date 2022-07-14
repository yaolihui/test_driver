/*
 *
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.navigationtest;

import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.app.Activity;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.ScrollView;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;

import com.fingerprints.extension.navigation.FingerprintNavigation;
import com.fingerprints.extension.navigation.NavigationConfig;
import com.fingerprints.extension.sensetouch.FingerprintSenseTouch;
import com.fingerprints.extension.sensetouch.SenseTouchConfig;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;

/* TODO: Investigate warning: MainActivity.java uses unchecked or unsafe operations
         caused by system properties operations. */
public class Settings extends Activity implements CompoundButton.OnCheckedChangeListener,
        Button.OnClickListener {

    /* Object used to communicate with lower layers through Fingerprint Extension Service */
    private FingerprintNavigation mNavigation;
    /* Active configuration objects */
    private NavigationConfig mNavConfig;
    private NavigationConfig mDefaultNavConfig;
    /* Used to read and set sense touch configuration */
    private FingerprintSenseTouch mFingerprintSenseTouch;
    private SenseTouchConfig mSenseTouchConfig;
    /* System properties used to store default nav config */
    private final int FPC_SYS_PROP_NOT_SET = 0xDEAD;
    private final String FPC_SYS_PROP_PREFIX = "vendor.fpc.nav.";

    /* Graphical components */
    private ScrollView mSettingScrollView;
    private Switch mEnableNavigationSwitch;
    private FrameLayout mConfigButtonLayout;
    private Button mLoadActiveConfigButton;
    private Button mLoadDefaultConfigButton;

    private FrameLayout mNavConfigLayout;
    private ListView mNavConfigParamList;

    /* Adapter */
    private CustomAdapter mAdapter;

    /* Read navigation default values from system properties. */
    private NavigationConfig readConfigDefaults() {
        Log.d("FPC", "readConfigDefaults Enter");
        NavigationConfig navConf = new NavigationConfig();
        try {
            final Class SystemProperties = Class.forName("android.os.SystemProperties");
            if (SystemProperties != null) {
                final Method getInt = SystemProperties.getMethod("getInt", String.class, int.class);
                if (getInt != null) {
                    /* null is used instead of class obj since invoking method is static */
                    for (Field f : navConf.getClass().getFields()) {
                        int value = (int) getInt.invoke(null, FPC_SYS_PROP_PREFIX + f.getName(), FPC_SYS_PROP_NOT_SET);
                        if (value == FPC_SYS_PROP_NOT_SET) {


                        Log.e("FPC", "Unable to read default config from sys properties.");
                        navConf = null;break;
                        }

                        f.set(navConf, value);
                    }
                }
            }

        } catch (Exception e) {
            /* getInt @throws IllegalArgumentException if the key exceeds 32 characters */
            Log.d("FPC", "exception was cast: " + e.toString() + " cause " + e.getCause());
            navConf = null;
        }
        Log.d("FPC", "readConfigDefaults Exit");
        return navConf;
    }

    /* Save navigation default values in system properties. */
    private void storeConfigDefaults(NavigationConfig navConf) {
        Log.d("FPC", "storeConfigDefaults Enter");
        try {
            final Class SystemProperties = Class.forName("android.os.SystemProperties");
            if (SystemProperties != null) {
                final Method set = SystemProperties.getMethod("set", String.class, String.class);
                if (set != null) {
                    /* null is used instead of class obj since invoking method is static */
                    for (Field f: navConf.getClass().getFields()) {
                        set.invoke(null, FPC_SYS_PROP_PREFIX + f.getName(),
                                f.get(navConf).toString());

                    }
                    Log.d("FPC", "Navigation default values stored as system properties.");
                }
            }
        } catch (Exception e) {
            /* set @throws IllegalArgumentException if the key exceeds 32 characters
                   @throws IllegalArgumentException if the value exceeds 92 characters */
        }
        Log.d("FPC", "storeConfigDefaults Exit");
    }

    /* Initiates the application. */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d("FPC", "onCreate Enter");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);
        getActionBar().setDisplayHomeAsUpEnabled(true);

        if (getResources().getBoolean(R.bool.orientation_locked_portrait)) {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        } else {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        }

        mSettingScrollView = (ScrollView) findViewById(R.id.settings_scroll_view);
        mEnableNavigationSwitch = (Switch) findViewById(R.id.enable_nav_switch);
        mEnableNavigationSwitch.setOnCheckedChangeListener(this);
        mEnableNavigationSwitch.setFocusable(false);
        mConfigButtonLayout = (FrameLayout) findViewById(R.id.config_buttons_layout);
        mLoadActiveConfigButton = (Button) findViewById(R.id.load_active_config_button);
        mLoadActiveConfigButton.setFocusable(false);
        mLoadActiveConfigButton.setOnClickListener(this);
        mLoadDefaultConfigButton = (Button) findViewById(R.id.load_default_config_button);
        mLoadDefaultConfigButton.setOnClickListener(this);
        mLoadDefaultConfigButton.setFocusable(false);

        mNavConfigLayout = (FrameLayout) findViewById(R.id.nav_config_layout);
        mNavConfigParamList = (ListView) findViewById(R.id.nav_config_param_list);

        //Fill list with parameters and its corresponding value
        mAdapter = new CustomAdapter(this, android.R.id.text1, getTempArrayList());
        mNavConfigParamList.setAdapter(mAdapter);

        boolean wasNavEnabled = true;
        try {
            if (mNavigation == null) {
                mNavigation = new FingerprintNavigation();
                mNavConfig = new NavigationConfig();
            }
            wasNavEnabled = mNavigation.isEnabled();

            mNavigation.setNavigation(false);
            /* Get the predefined default config from HAL the first time and display in the UI. */
            mNavConfig = getConfig();
            updateConfigInUi(mNavConfig);

            NavigationConfig navConf = readConfigDefaults();

            if (navConf != null) {
                Log.d("FPC", "Navigation default config read from system properties.");
                mDefaultNavConfig = new NavigationConfig(navConf);
            } else {
                Log.d("FPC", "Navigation default config read from HAL.");
                mDefaultNavConfig = new NavigationConfig(mNavConfig);
                /* Since HAL currently doesn't remember the default values we save the defaults
                   in system properties that can be used if the app is restarted. */
                storeConfigDefaults(mDefaultNavConfig);
            }
        } catch (Throwable e) {
            Log.e("FPC", "Failed to initialize navigation extension module.");
            disableView(mConfigButtonLayout, -1);
            disableView(mNavConfigLayout, R.color.lightGrey);
            showDialog(getString(R.string.dialogWarning), getString(R.string.dialogNavExtCreationFailure) + "\n" + e.getMessage());
        }

        if (wasNavEnabled) {
            mEnableNavigationSwitch.setChecked(true);
        } else {
            mEnableNavigationSwitch.setChecked(false);
        }

        Log.d("FPC", "onCreate Exit");
    }

    private ArrayList<Param> getTempArrayList() {
        ArrayList<Param> tempList = new ArrayList<>();
        for (Field f: NavigationConfig.class.getFields()) {
            tempList.add(new Param(f.getName(), 0));
        }
        return tempList;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.menu1, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle item selection
        switch (item.getItemId()) {
            case R.id.input:
                Intent toTest = new Intent(Settings.this, TestModeActivity.class);
                startActivity(toTest);
                finish();
                return true;
            case R.id.log:
                Intent toLog = new Intent(Settings.this, LogActivity.class);
                startActivity(toLog);
                finish();
                return true;
            case R.id.settings:
                Intent toSettings = new Intent(Settings.this, Settings.class);
                startActivity(toSettings);
                finish();
                return true;
            case android.R.id.home:
                Intent toDemo = new Intent(Settings.this, DemoActivity.class);
                startActivity(toDemo);
                finish();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    /* Listener function for when navigation switch is changed. */
    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        Log.d("FPC", "onCheckedChanged Enter, data isChecked=" + isChecked);
        if (isChecked) {
            disableView(mConfigButtonLayout, -1);
            disableView(mNavConfigLayout, R.color.lightGrey);
            setConfig();

            if (mNavConfig != null) {
                mNavConfig.print();
            }

            if (mNavigation != null) {
                mNavigation.setNavigation(true);
            }
        } else {
            if (mNavigation != null) {
                mNavigation.setNavigation(false);
                enableView(mConfigButtonLayout, R.color.white);
                enableView(mNavConfigLayout, R.color.white);
            }
        }
        mAdapter.notifyDataSetChanged();
        Log.d("FPC", "onCheckedChanged Exit");
    }

    private void disableView(ViewGroup view, int newBackgroundColor) {
        Log.d("FPC", "disableView Enter");
        if (newBackgroundColor != -1) {
            view.setBackgroundResource(newBackgroundColor);
        }
        for (int i = 0; i < view.getChildCount(); i++) {
            if (view.getChildAt(i) instanceof SeekBar) {
                ((SeekBar) view.getChildAt(i)).setThumb(getDrawable(R.drawable.thumb_slider_disabled));
            }
            view.getChildAt(i).setEnabled(false);
        }
        view.setEnabled(false);
        Log.d("FPC", "disableView Exit");
    }

    private void enableView(ViewGroup view, int newBackgroundColor) {
        Log.d("FPC", "enableView Enter");
        view.setEnabled(true);
        if (newBackgroundColor != -1) {
            view.setBackgroundResource(newBackgroundColor);
        }
        for (int i = 0; i < view.getChildCount(); i++) {
            if (view.getChildAt(i) instanceof SeekBar) {
                ((SeekBar) view.getChildAt(i)).setThumb(getDrawable(R.drawable.thumb_slider));
            }
            view.getChildAt(i).setEnabled(true);
        }
        Log.d("FPC", "enableView Exit");
    }

    /* Listener function for when a component is clicked in the user interface. */
    public void onClick(View v) {
        Log.d("FPC", "onClick Enter");
        switch (v.getId()) {
            case R.id.load_active_config_button:
                Log.d("FPC", "Get Active Config button pressed.");
                boolean readSuccess = false;
                mNavConfig = getConfig();
                updateConfigInUi(mNavConfig);
                break;
            case R.id.load_default_config_button:
                Log.d("FPC", "Load Defaults button pressed.");
                setDefaultConfig();
                break;
            default:
                Log.d("FPC", "No click behavior is defined for this component: " + v.getId());
                break;
        }
        Log.d("FPC", "onClick Exit");
    }

    /* Read and store the current config specified in the user interface. */
    private NavigationConfig readConfigFromUi() {
        Log.d("FPC", "readConfigFromUi Enter");

        ArrayList<Param> mTempList = new ArrayList<>();
        for (int i = 0; i < mAdapter.getCount(); i++) {
            Param item = mAdapter.getItem(i);
            mTempList.add(item);
        }
        if (mNavConfig != null) {
            int i = 0;
            for (Field f: mNavConfig.getClass().getFields()) {
                try {
                    f.set(mNavConfig, mTempList.get(i).getParamValue());
                } catch (IllegalAccessException e) {
                    e.printStackTrace();
                }
                i ++;
            }
        }
        Log.d("FPC", "readConfigFromUi Exit");
        return mNavConfig;
    }

    /* Update the user interface with new config values. */
    private void updateConfigInUi(NavigationConfig navConfig) {
        Log.d("FPC", "updateConfigInUi Enter, data: navConfig");
        if (navConfig != null) {
            navConfig.print();
            ArrayList<Param> mTempList = new ArrayList<>();
            for (int i = 0; i < mAdapter.getCount(); i++) {
                Param item = mAdapter.getItem(i);
                mTempList.add(item);
            }
            int i = 0;
            for (Field f: navConfig.getClass().getFields()) {
                try {
                    mTempList.get(i).setParamValue((int)f.get(navConfig));
                } catch (IllegalAccessException e) {
                    e.printStackTrace();
                }
                i++;
            }
            mAdapter.clear();
            mAdapter.addAll(mTempList);
            mAdapter.notifyDataSetChanged();
        } else {
            Log.d("FPC", "Unable to update config in UI input navConfig was null.");
        }
        Log.d("FPC", "updateConfigInUi Exit");
    }

    /* Set a new active navigation config to be used. */
    private void setConfig() {
        Log.d("FPC", "setConfig Enter");
        readConfigFromUi();
        if (mNavigation != null && mNavConfig != null) {
            mNavConfig.print();
            mNavigation.setNavigationConfig(mNavConfig);
        }
        Log.d("FPC", "setConfig Exit");
    }

    /* Get the navigation config currently used. */
    private NavigationConfig getConfig() {
        Log.d("FPC", "getConfig Enter");
        NavigationConfig halNavConfig = null;
        if (mNavigation != null) {
            halNavConfig = mNavigation.getNavigationConfig();
        }
        if (halNavConfig == null) {
            Log.e("FPC", "Failed to fetch navigation config from HAL.");
            showDialog(getString(R.string.dialogError), getString(R.string.dialogUnableToFetchNavConfig));
            halNavConfig = mNavConfig;
        }
        Log.d("FPC", "getConfig Exit");
        return halNavConfig;
    }

    /* This method is used to reset navigation UI to use the pre-defined default values. */
    private void setDefaultConfig() {
        Log.d("FPC", "setDefaultConfig Enter");
        updateConfigInUi(mDefaultNavConfig);
        Log.d("FPC", "setDefaultConfig Exit");
    }

    /* Send a pop-up message to the user. */
    private void showDialog(String title, String text) {
        Log.d("FPC", "showDialog Enter");
        AlertDialog alertDialog = new AlertDialog.Builder(this).create();
        alertDialog.setTitle(title);
        alertDialog.setMessage(text);
        alertDialog.setButton(AlertDialog.BUTTON_NEUTRAL, "OK",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                    }
                });
        alertDialog.show();
        Log.d("FPC", "showDialog Exit");
    }

    //Custom adapter class which fills the listview with Params.
    private class CustomAdapter extends ArrayAdapter<Param> {
        ArrayList<Param> items;

        public CustomAdapter(Context context, int textViewResourceId, ArrayList<Param> items) {
            super(context, textViewResourceId, items);
            this.items = items;
        }

        @SuppressLint("ViewHolder")
        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            LayoutInflater vi = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = vi.inflate(R.layout.list_item, null);

            final TextView mParamName = (TextView) convertView.findViewById(R.id.paramName);
            final EditText mParamValue = (EditText) convertView.findViewById(R.id.paramValue);
            mParamValue.setOnEditorActionListener(new DoneOnEditorActionListener());
            mParamValue.setFocusable(!mEnableNavigationSwitch.isChecked());

            mParamName.setText(items.get(position).getParamName());
            mParamValue.setText("" + items.get(position).getParamValue());

            final int index = position;
            mParamValue.addTextChangedListener(new TextWatcher() {

                @Override
                public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                }

                @Override
                public void onTextChanged(CharSequence s, int start, int before, int count) {
                }

                @Override
                public void afterTextChanged(Editable arg0) {
                    if (arg0.length() != 0) {
                        items.get(index).setParamValue(Integer.valueOf(arg0.toString()));
                    }
                }
            });
            return convertView;
        }
    }

    class DoneOnEditorActionListener implements TextView.OnEditorActionListener {
        @Override
        public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
            if (actionId == EditorInfo.IME_ACTION_DONE) {
                InputMethodManager imm = (InputMethodManager) v.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(v.getWindowToken(), 0);
                return true;
            }
            return false;
        }
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        if (event.getKeyCode() == KeyEvent.KEYCODE_BUTTON_B) {
            return true;
        } else {
            return super.dispatchKeyEvent(event);
        }
    }

    private void hideKeyboard() {
        View view = this.getCurrentFocus();
        if (view != null) {
            InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
            imm.hideSoftInputFromWindow(view.getWindowToken(), 0);
        }
    }
}


