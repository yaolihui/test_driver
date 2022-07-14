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

package com.fingerprints.imagecollection.activities;

import android.Manifest;
import android.app.ActionBar;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.os.Bundle;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBarDrawerToggle;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.content.Intent;

import com.fingerprints.imagecollection.R;
import com.fingerprints.imagecollection.fragments.ConfigurationFragment;
import com.fingerprints.imagecollection.fragments.ImageCollectionFragment;
import com.fingerprints.imagecollection.interfaces.IConfigurationManager;
import com.fingerprints.imagecollection.scenario.ImageCollectionConfig;
import com.fingerprints.imagecollection.scenario.VerifyConfig;
import com.fingerprints.imagecollection.utils.AndroidUI;
import com.fingerprints.imagecollection.utils.Disk;
import com.fingerprints.imagecollection.utils.Logger;
import com.fingerprints.imagecollection.values.Constants;
import com.fingerprints.imagecollection.values.Preferences;

import tools.xml.GVisitor;
import tools.xml.IXMLNode;
import tools.xml.XMLParser;

import static com.fingerprints.imagecollection.scenario.ImageCollectionConfig.DECISION_FEEDBACK;
import static com.fingerprints.imagecollection.scenario.ImageCollectionConfig.IMAGE_DISPLAY;
import static com.fingerprints.imagecollection.scenario.ImageCollectionConfig.LEFT_INDEXES;
import static com.fingerprints.imagecollection.scenario.ImageCollectionConfig.NUMBER_OF_IMAGES;
import static com.fingerprints.imagecollection.scenario.ImageCollectionConfig.RIGHT_INDEXES;
import static com.fingerprints.imagecollection.scenario.VerifyConfig.ANGLE;
import static com.fingerprints.imagecollection.scenario.VerifyConfig.DESCRIPTION;
import static com.fingerprints.imagecollection.scenario.VerifyConfig.POSITION;
import com.fingerprints.extension.util.FpcConstants;

public class MainActivity extends DisabledNavigationActivity implements ImageCollectionFragment.ImageCollectionListener, IConfigurationManager {

    private Logger mLogger = new Logger(getClass().getSimpleName());
    private DrawerLayout mDrawerLayout;
    private ActionBarDrawerToggle mDrawerToggle;
    private ImageCollectionFragment mImageCollectionFragment;
    private ConfigurationFragment mConfigurationFragment;
    private boolean mIsRunning = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mLogger.enter("onCreate");
        setContentView(R.layout.activity_main);
        mDrawerLayout = (DrawerLayout) findViewById(R.id.drawer_layout);

        final ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            actionBar.setDisplayHomeAsUpEnabled(true);
            actionBar.setHomeButtonEnabled(true);
        }

        mDrawerToggle = new ActionBarDrawerToggle(this, mDrawerLayout, R.string.drawer_open,
                R.string.drawer_close) {

            public void onDrawerClosed(View view) {
                getActionBar().setTitle(R.string.image_collection_title);
                mConfigurationFragment.onClosed();
                invalidateOptionsMenu();
            }

            public void onDrawerOpened(View drawerView) {
                getActionBar().setTitle(R.string.configuration_title);
                invalidateOptionsMenu();
            }
        };
        mDrawerLayout.setDrawerListener(mDrawerToggle);

        getFragmentManager().beginTransaction().add(R.id.left_drawer_frame,
                getConfigurationFragment()).commit();

        getFragmentManager().beginTransaction().add(R.id.content_frame,
                getImageCollectionFragment()).commit();

        mLogger.exit("onCreate");
    }

    @Override
    protected void onStart() {
        super.onStart();
        setDrawerState(!hasExternalConfiguration() && !mIsRunning);
    }

    public ConfigurationFragment getConfigurationFragment() {
        if (mConfigurationFragment == null) {
            mConfigurationFragment = new ConfigurationFragment();
        }
        return mConfigurationFragment;
    }

    public ImageCollectionFragment getImageCollectionFragment() {
        if (mImageCollectionFragment == null) {
            mImageCollectionFragment = new ImageCollectionFragment();
        }
        return mImageCollectionFragment;
    }

    @Override
    public void onBackPressed() {
        mLogger.enter("onBackPressed");
        mConfigurationFragment.onClosed();
        getImageCollectionFragment().onBackPressed();
        mLogger.exit("onBackPressed");
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (mDrawerToggle.onOptionsItemSelected(item)) {
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onPostCreate(Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);
        mDrawerToggle.syncState();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        mDrawerToggle.onConfigurationChanged(newConfig);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        mLogger.enter("onActivityResult");
        if (requestCode == Constants.ENROLL_GET_TOKEN) {
            if (resultCode != RESULT_OK) {
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        boolean abort = AndroidUI.showDialogAskAndWait(getString(R.string.abort),
                                getString(R.string.back_pressed_alert_message),
                                getString(R.string.back_pressed_alert_positive),
                                getString(R.string.back_pressed_alert_negative),
                                MainActivity.this);

                        if (abort) {
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    onStopImageCollection();
                                }
                            });
                        }
                    }
                }).start();
            }
        }
        mLogger.exit("onActivityResult");
    }

    public void onStartImageCollection() {
        mLogger.enter("onStartImageCollection");
        mIsRunning = true;
        invalidateOptionsMenu();
        setDrawerState(false);
        mLogger.exit("onStartImageCollection");
    }

    @Override
    public void onStopImageCollection() {
        mLogger.enter("onStopImageCollection");
        mIsRunning = false;
        invalidateOptionsMenu();
        setDrawerState(!hasExternalConfiguration());
        mLogger.exit("onStopImageCollection");
    }

    public void setDrawerState(boolean isEnabled) {
        getActionBar().setHomeButtonEnabled(isEnabled);
        getConfigurationFragment().setInteractive(isEnabled);

        if (isEnabled) {
            mDrawerLayout.setDrawerLockMode(DrawerLayout.LOCK_MODE_UNLOCKED);
            mDrawerToggle.syncState();
            mDrawerToggle.setDrawerIndicatorEnabled(true);

        } else {
            mDrawerLayout.setDrawerLockMode(DrawerLayout.LOCK_MODE_LOCKED_CLOSED);
            mDrawerToggle.syncState();
            mDrawerToggle.setDrawerIndicatorEnabled(false);
            mDrawerToggle.setHomeAsUpIndicator(R.drawable.fpc_logo);
        }
    }

    @Override
    public boolean onCreateOptionsMenu(final Menu menu) {
        MenuInflater inflater = getMenuInflater();
        if (mIsRunning) {
            inflater.inflate(R.menu.menu, menu);
            ((MenuItem) menu.findItem(R.id.menu_abort)).setOnMenuItemClickListener(new MenuItem.OnMenuItemClickListener() {
                @Override
                public boolean onMenuItemClick(final MenuItem item) {
                    mImageCollectionFragment.abort();
                    return true;
                }
            });
        }
        return true;
    }

    public ImageCollectionConfig getConfiguration() throws Exception {
        if (hasExternalConfiguration()) {
            return readImageCollectionConfig();
        } else {
            return mConfigurationFragment.getConfiguration();
        }
    }

    public boolean hasExternalConfiguration() {
        return Disk.externalFileExist(Preferences.SCENARIO_CONFIG_NAME, getApplicationContext());
    }

    private ImageCollectionConfig readImageCollectionConfig() throws Exception {
        String xmlFile = Disk.readExternalTextFile(Preferences.SCENARIO_CONFIG_NAME, getApplicationContext());

        IXMLNode root = XMLParser.parseString(xmlFile);

        final ImageCollectionConfig config = new ImageCollectionConfig(root.getValue(LEFT_INDEXES),
                root.getValue(RIGHT_INDEXES),
                root.getValueI(NUMBER_OF_IMAGES),
                root.getValueB(IMAGE_DISPLAY, false),
                root.getValueB(DECISION_FEEDBACK, false));

        root.get("Verify", new GVisitor<IXMLNode>() {
            @Override
            public void visit(final IXMLNode verifyNode) {
                VerifyConfig verifyConfig = new VerifyConfig(verifyNode.getValueI(ANGLE),
                        verifyNode.getValue(POSITION),
                        verifyNode.getValue(DESCRIPTION));
                if (verifyNode.hasValue(NUMBER_OF_IMAGES)) {
                    verifyConfig.setNumberOfImages(verifyNode.getValueI(NUMBER_OF_IMAGES));
                }

                config.add(verifyConfig);
            }
        });

        return config;
    }
}
