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
package com.fingerprints.sensortesttool.activities;

import android.Manifest;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.RemoteException;
import android.os.Vibrator;
import android.text.Html;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toolbar;

import com.fingerprints.extension.sensetouch.FingerprintSenseTouch;
import com.fingerprints.extension.sensortest.FingerprintSensorTest;
import com.fingerprints.extension.sensortest.SensorInfo;
import com.fingerprints.extension.sensortest.SensorTest;
import com.fingerprints.sensortesttool.ITestController;
import com.fingerprints.sensortesttool.R;
import com.fingerprints.sensortesttool.TestCaseAdapter;
import com.fingerprints.sensortesttool.logging.ReportHandler;
import com.fingerprints.sensortesttool.testcases.ATestCase;
import com.fingerprints.sensortesttool.testcases.AUITestCase;
import com.fingerprints.sensortesttool.testcases.SensorTestCase;
import com.fingerprints.sensortesttool.testcases.manual.SenseTouchCalibration;
import com.fingerprints.sensortesttool.tools.Android;
import com.fingerprints.sensortesttool.tools.Disk;
import com.fingerprints.sensortesttool.tools.LogBuilder;
import com.fingerprints.sensortesttool.tools.ThreadTools;
import com.fingerprints.sensortesttool.values.Constants;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import tools.interpolation.Function;
import tools.interpolation.Interpolation;
import tools.interpolation.InterpolationManager;
import tools.interpolation.InterpolationVisitor;
import tools.interpolation.SinInterpolation;

public class MainActivityFTM extends DisabledNavigationActivity implements ITestController {
    private Toolbar mToolbar;
    private FingerprintSensorTest mFingerprintSensorTest;
    private FingerprintSenseTouch mFingerprintSenseTouch;
    private Handler mHandler;
    private ReportHandler mReport;
    private ListView mListView;
    private TestCaseAdapter mAdapter;
    private boolean mAutoRun = false;
    private TextView mStatusText;
    private Vibrator mVibrator;
    private Boolean mIsRunning = false;
    private Button mRunButton;
    private TextView mTitle;
    private RelativeLayout mSectionBottom;
    private RelativeLayout mTestCaseOverview;
    private RelativeLayout mTestCaseDetails;
    private RelativeLayout mViewContainer;
    private ScrollView mDetailsScrollView;

    public Button mBtnPass, mBtnFail;
    public boolean onResumeToRun = false;

    @Override
    public void onBackPressed() {
        if (mTestCaseDetails.getVisibility() == View.VISIBLE) {
            viewOverview(mTestCaseDetails);
        } else if (mIsRunning) {
            askCancel();
        } else {
            new Thread(new Runnable() {
                @Override
                public void run() {
                    if (Android.showDialogAskAndWait(getString(R.string.exit), getString(R.string.exit_message),
                            getString(R.string.ok), getString(R.string.cancel), MainActivityFTM.this)) {
                        finish();
                    }
                }
            }).start();
        }
    }

    private void askCancel() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                if (Android.showDialogAskAndWait(getString(R.string.abort), getString(R.string.abort_message),
                        getString(R.string.ok), getString(R.string.cancel), MainActivityFTM.this)) {
                    cancel();
                }
            }
        }).start();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        getWindow().requestFeature(Window.FEATURE_CONTENT_TRANSITIONS);
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_stt_ftm);
        getContext().getExternalFilesDir(Environment.DIRECTORY_PICTURES);

        mVibrator = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
        mReport = new ReportHandler();
        mTestCaseOverview = (RelativeLayout) findViewById(R.id.stt_testcase_overview);
        mTestCaseDetails = (RelativeLayout) findViewById(R.id.stt_testcase_details);
        mViewContainer = (RelativeLayout) findViewById(R.id.stt_view_container);
        mTitle = (TextView) findViewById(R.id.stt_title_text);
        mDetailsScrollView = (ScrollView) findViewById(R.id.stt_details_scrollview);
        InterpolationManager.getInstance().startThread();

        initToolbar();

        mSectionBottom = (RelativeLayout) findViewById(R.id.stt_section_bottom);
        mStatusText = (TextView) findViewById(R.id.stt_status_text_result);

        mRunButton = (Button) findViewById(R.id.stt_run_button);
        mRunButton.setText(getResources().getString(R.string.run));

        initPassFailView();

        try {
            getSenseTouch();
        } catch (Throwable e) {
            mReport.e("SenseTouch library not found, skipping...");
        }

        try {
            //test that we can load the extension before continuing.
            getSensorTest();

            mHandler = new Handler(Looper.getMainLooper());

            mRunButton.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(final View v) {
                    updateCurrentResultFolder();
                    if (!mIsRunning) {
                        runTestCases();
                    } else {
                        askCancel();
                    }
                }
            });

            mListView = (ListView) findViewById(R.id.stt_test_cases_listview);
            mAdapter = new TestCaseAdapter(this, R.layout.testcase_item, getTestCases());
            mListView.setAdapter(mAdapter);

            mListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                @Override
                public void onItemClick(final AdapterView<?> parent, final View view, final int position, final long id) {
                    final ATestCase testCase = mAdapter.getItem(position);

                    if (testCase.hasResult()) {
                        new Thread(new Runnable() {
                            @Override
                            public void run() {
                                viewDetails(testCase);
                            }
                        }).start();
                    } else {
                        testCase.setSelected(!testCase.isSelected());
                        mAdapter.notifyDataSetChanged();
                    }
                }
            });

        } catch (Exception e) {
            mRunButton.setEnabled(false);
            mReport.e(e.getMessage());
            mStatusText.setTextColor(getColor(R.color.failed_color));
            mStatusText.setText("Error -" + e.getMessage());
            showErrorDialog(getString(R.string.error), getString(R.string.error_initializing_sensor, e.getMessage()));
        }

        //TODO, when we load tests dynamically make sure this is executed after those has been fully loaded.
        checkAndAutoRun();

//        if (checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)
//                != PackageManager.PERMISSION_GRANTED) {
//            requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1234);
//        }
    }

    private void initPassFailView(){
        mBtnPass = findViewById(R.id.btn_pass);
        mBtnPass.setEnabled(true);
        mBtnPass.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //设置点击事件
                setResult(2);
                finish();
            }
        });
        mBtnPass.setEnabled(false);
        mBtnFail = findViewById(R.id.btn_fail);
        mBtnFail.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //设置点击事件
                setResult(0);
                finish();
            }
        });

    }

    @Override
    protected void onResume() {
        super.onResume();
        if(!onResumeToRun){
            onResumeToRun = true;
            if(null != mRunButton && !mIsRunning){
                mRunButton.performClick();
            }
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        cancel();
    }

    private void cancel() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mIsRunning) {
                    mFingerprintSensorTest.cancelSensorTest();
                    mRunButton.setText(getResources().getString(R.string.run));
                    mIsRunning = false;

                    for (int i = 0; i < mAdapter.getCount(); i++) {
                        ATestCase testCase = mAdapter.getItem(i);
                        if (testCase.getStatus().isRunning()) {
                            testCase.cancel();
                        }
                    }
                    mAdapter.setEnabled(true);
                }
            }
        });
    }

    @Override
    public FingerprintSensorTest getSensorTest() throws RemoteException {
        if (mFingerprintSensorTest == null) {
            mFingerprintSensorTest = new FingerprintSensorTest();
        }

        return mFingerprintSensorTest;
    }

    @Override
    public FingerprintSenseTouch getSenseTouch() throws RemoteException {
        if (mFingerprintSenseTouch == null) {
            mFingerprintSenseTouch = new FingerprintSenseTouch();
        }

        return mFingerprintSenseTouch;
    }

    private static int getFolderNumber(File folder) {
        return Integer.parseInt(folder.getName().replaceAll(".*_", ""));
    }

    private static class FileComparator implements Comparator<File> {
        @Override
        public int compare(File file1, File file2) {
            return getFolderNumber(file1) - getFolderNumber(file2);
        }
    }

    private void updateCurrentResultFolder() {
        File[] files = new File(Constants.SENSOR_TEST_RESULT_FOLDER).listFiles();
        String name = Constants.SENSOR_TEST_RESULT_DEFAULT_FOLDER;
        SharedPreferences.Editor preferencesEditor = getContext().getSharedPreferences(
                Constants.SENSOR_TEST_SHAREDPREFERENCES,
                Context.MODE_PRIVATE).edit();


        if (files != null && files.length > 0) {
            List<File> fileList = Arrays.asList(files);
            Collections.sort(fileList, new FileComparator());

            // Remove folder prefix and filter out latest test run number.
            name = String.format(Constants.SENSOR_TEST_CASE_RESULT_FOLDER,
                    getFolderNumber(fileList.get(fileList.size() - 1)) + 1);
        }
        preferencesEditor.putString(Constants.SENSOR_TEST_RESULT_FOLDER_KEY, name);
        preferencesEditor.commit();
    }

    public void runTestCases() {
        mReport.enter();

        new Thread(new Runnable() {
            @Override
            public void run() {

                if(mAutoRun) {
                    ThreadTools.sleep(100);
                }

                mIsRunning = true;

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        mRunButton.setText(getResources().getString(R.string.cancel));
                        mAdapter.setEnabled(false);
                        mStatusText.setTextColor(getColor(R.color.black));
                    }
                });

                int countTestCases = 0;

                //reset all
                for (int i = 0; i < mListView.getCount(); i++) {
                    ATestCase testCase = (ATestCase) mListView.getItemAtPosition(i);
                    testCase.reset();
                    if (testCase.isSelected()) {
                        countTestCases++;
                    }
                }

                if (countTestCases > 0) {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            mStatusText.setText(getString(R.string.status_running));
                        }
                    });

                    mReport.testRunStarted(mFingerprintSensorTest.getSensorInfo());
                    mReport.reportText("");
                    mReport.reportText("Running " + countTestCases + " test cases.");
                    mReport.reportText("=====================================");
                    //run checked test cases
                    for (int i = 0; i < mListView.getCount(); i++) {
                        ATestCase testCase = (ATestCase) mListView.getItemAtPosition(i);

                        if (testCase.isSelected() && mIsRunning) {
                            final int position = i;

                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    mListView.smoothScrollToPosition(position);
                                }
                            });

                            mReport.reportText("");

                            if (testCase instanceof AUITestCase) {
                                Interpolation interpolation = viewTestUI((AUITestCase) testCase);
                                interpolation.waitOnFinish();
                            }

                            testCase.run();

                            if (testCase.isManual() && testCase.hasResult()) {
                                vibrate();
                                ThreadTools.sleep(getResources().getInteger(R.integer.testcase_manual_sleeptime_ms));
                            }

                            if (testCase instanceof AUITestCase) {
                                Interpolation interpolation = viewOverview(((AUITestCase) testCase).getView());
                                interpolation.waitOnFinish();
                            }

                            mReport.i("Test completed: " + testCase.getName() + "\n");
                        }
                    }

                    mReport.reportText("");
                    mReport.reportText("Run complete");
                    mReport.testRunComplete();

                    boolean runOk = true;
                    for (int i = 0; i < mListView.getCount(); i++) {
                        ATestCase testCase = (ATestCase) mListView.getItemAtPosition(i);
                        if (testCase.getStatus().isFailed() || testCase.getStatus().isCancelled()) {
                            runOk = false;
                            break;
                        }

                    }

                    final boolean runOkFinal = runOk;

                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            if (runOkFinal) {
                                mStatusText.setTextColor(getColor(R.color.passed_color));
                                mStatusText.setText(getString(R.string.status_passed));
                            } else {
                                mStatusText.setTextColor(getColor(R.color.failed_color));
                                mStatusText.setText(getString(R.string.status_failed));
                            }
                        }

                    });
                }

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        mRunButton.setText(getResources().getString(R.string.run));
                        mIsRunning = false;
                        mAdapter.setEnabled(true);
                        updatePassAndFail();
                    }
                });

                if (mAutoRun) {
                    Disk.write(getString(R.string.autorun_output_filename), mReport.getXmlResultLog().toString(),
                            getApplicationContext());
                }
            }
        }).start();
    }

    private void updatePassAndFail(){
        boolean isPassOK = false;
        if(null != mListView){
            isPassOK = true;
            for (int i = 0; i < mListView.getCount(); i++) {
                ATestCase testCase = (ATestCase) mListView.getItemAtPosition(i);
                if(!testCase.getStatus().isPassed()){
                    isPassOK = false;
                    break;
                }
            }
        }

        if(isPassOK){
            mBtnPass.setEnabled(true);
            mBtnPass.performClick();
        }

    }

    private ArrayList<ATestCase> getTestCases() {
        ArrayList<ATestCase> list = new ArrayList<>();

        if (mFingerprintSenseTouch != null && mFingerprintSenseTouch.isSupported()) {
            SenseTouchCalibration senseTouchCalibration = new SenseTouchCalibration(this);
            senseTouchCalibration.setSelected(true);
            list.add(senseTouchCalibration);
        }

        for (SensorTest sensorTest : mFingerprintSensorTest.getSensorTests()) {
            SensorTestCase sensorTestCase = new SensorTestCase(sensorTest, this);
            sensorTestCase.setSelected(true);
            list.add(sensorTestCase);
        }

        return list;
    }

    public void showSensorInfo() {

        final SensorInfo sensorInfo = mFingerprintSensorTest.getSensorInfo();

        if (sensorInfo != null) {
            final AlertDialog.Builder dialog = new AlertDialog.Builder(this);
            dialog.setTitle(getString(R.string.sensor_information));
            final TextView input = new TextView(this);
            input.setPadding(Android.dpToPx(25, this), 0, Android.pxToDp(20, this), 0);
            dialog.setView(input);

            input.setText(sensorInfo.toString());

            dialog.setPositiveButton(getString(R.string.ok), new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int whichButton) {
                    dialog.dismiss();
                }
            });

            dialog.show();
        } else {
            showErrorDialog(getString(R.string.error), getString(R.string.error_getting_sensor_info));
        }
    }

    private void showErrorDialog(final String title, final String text) {
        new AlertDialog.Builder(this)
                .setTitle(title)
                .setMessage(text)
                .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                    }
                }).setIcon(android.R.drawable.ic_dialog_alert)
                .show();
    }

    private void openLogView() {
        Intent intent = new Intent(this, LogActivity.class);
        intent.putExtra(LogActivity.LOG_DATA, mReport.getHtmlLog());
        startActivity(intent);
    }

    public void initToolbar() {
        // Set a toolbar to replace the action bar.
        mToolbar = (Toolbar) findViewById(R.id.toolbar);
        mToolbar.setTitle(R.string.app_name);
        mToolbar.inflateMenu(R.menu.menu);

        mToolbar.setOnMenuItemClickListener(new Toolbar.OnMenuItemClickListener() {
            @Override
            public boolean onMenuItemClick(final MenuItem item) {
                switch (item.getItemId()) {
                    case R.id.menu_select_all: {
                        setSelectedTests(true, true);
                        break;
                    }
                    case R.id.menu_select_automatic: {
                        setSelectedTests(true, false);
                        break;
                    }
                    case R.id.menu_select_manual: {
                        setSelectedTests(false, true);
                        break;
                    }
                    case R.id.menu_select_none: {
                        setSelectedTests(false, false);
                        break;
                    }
                    case R.id.menu_show_log: {
                        openLogView();
                        break;
                    }
                    case R.id.menu_options: {
                        showSensorInfo();
                        break;
                    }
                    default: {
                        mReport.d("Click on unhandled item " + item.getTitle());
                    }
                }
                return false;
            }
        });
    }

    private void setEnabledTestsByIds(final String... ids) {
        for (int i = 0; i < mAdapter.getCount(); i++) {
            ATestCase testCase = mAdapter.getItem(i);
            for (String id : ids) {
                if (testCase.getId().equals(id)) {
                    testCase.reset();
                    testCase.setSelected(true);
                }
            }
        }

        mAdapter.notifyDataSetChanged();
    }

    private void setSelectedTests(final boolean automatic, final boolean manual) {
        if (mAdapter.isEnabled()) {
            for (int i = 0; i < mAdapter.getCount(); i++) {
                ATestCase testCase = mAdapter.getItem(i);
                testCase.reset();
                if (testCase.isManual()) {
                    testCase.setSelected(manual);
                } else {
                    testCase.setSelected(automatic);
                }
            }
        }
        mAdapter.notifyDataSetChanged();
    }

    public void checkAndAutoRun() {
        if (getIntent().hasExtra("autorun") && getIntent().getStringExtra("autorun").equals("true")) {
            this.mAutoRun = true;

            ArrayList<String> enableSpecificTestCasesList = new ArrayList<>();

            for (String key : getIntent().getExtras().keySet()) {
                String value = getIntent().getStringExtra(key);
                if (value.equals("true")) {
                    switch (key) {
                        case "autorun": {
                            //ignore
                            break;
                        }
                        case "automatic": {
                            setSelectedTests(true, false);
                            break;
                        }
                        default: {
                            enableSpecificTestCasesList.add(key);
                            break;
                        }
                    }
                }
            }

            for (String key : enableSpecificTestCasesList) {
                setEnabledTestsByIds(key);
            }

            runTestCases();
        }
    }


    /**
     * Controller methods, used for the testcases to communicate
     * with the UI when it needs to refresh etc.
     */

    @Override
    public void refresh(final ATestCase testCase) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                mAdapter.notifyDataSetChanged();
            }
        });
    }

    @Override
    public void vibrate() {
        mVibrator.vibrate(getResources().getInteger(R.integer.testcase_manual_vibrate_ms));
    }

    @Override
    public ReportHandler getReport() {
        return mReport;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu, menu);
        return true;
    }

    @Override
    public Context getContext() {
        return this;
    }

    public Interpolation viewOverview(final View previousView) {
        final int transitionTimeMS = 150;

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                previousView.setVisibility(View.VISIBLE);
                mTestCaseOverview.setVisibility(View.VISIBLE);
                mTitle.setText(getString(R.string.test_cases));
                mTestCaseOverview.setAlpha(1.0f);
            }
        });

        Interpolation move = new SinInterpolation(mTestCaseOverview.getX(), 0,
                InterpolationManager.timeToFrames(transitionTimeMS), new InterpolationVisitor() {
            @Override
            public void update(final float value, final float percent) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        mTestCaseOverview.setX(value);
                        previousView.setX(value + Android.getScreenSize(MainActivityFTM.this).x);
                        mSectionBottom.setAlpha(percent);
                        previousView.setAlpha(1.0f - percent);
                    }
                });
            }
        }).next(new Function() {
            @Override
            public void call(final Interpolation interpolation) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        previousView.setVisibility(View.INVISIBLE);
                        if (previousView != mTestCaseDetails) {
                            ((ViewGroup) previousView.getParent()).removeView(previousView);
                        }
                    }
                });
            }
        }).first();

        InterpolationManager.getInstance().addInterpolation(move);
        return move;
    }

    public Interpolation getTransitionInterpolation(final View targetView, final boolean hideRunControls) {
        final int transitionTimeMS = 150;

        Interpolation move = new SinInterpolation(mTestCaseOverview.getX(), -Android.getScreenSize(this).x,
                InterpolationManager.timeToFrames(transitionTimeMS), new InterpolationVisitor() {
            @Override
            public void update(final float value, final float percent) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        mTestCaseOverview.setX(value);
                        targetView.setX(value + Android.getScreenSize(MainActivityFTM.this).x);
                        mTestCaseOverview.setAlpha(1.0f - percent);
                        if (hideRunControls) {
                            mSectionBottom.setAlpha(1.0f - percent);
                        }
                    }
                });
            }
        }).next(new Function() {
            @Override
            public void call(final Interpolation interpolation) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        mTestCaseOverview.setVisibility(View.INVISIBLE);
                    }
                });
            }
        }).first();

        return move;
    }

    public Interpolation viewTestUI(final AUITestCase testCase) {

        final View view = testCase.getView();

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                view.setVisibility(View.VISIBLE);
                view.setX(Android.getScreenSize(MainActivityFTM.this).x);
                mViewContainer.addView(view);
                mTitle.setText(getString(R.string.test_cases) + " > " + testCase.getName());
                view.setAlpha(1.0f);

            }
        });

        Interpolation move = getTransitionInterpolation(view, false);
        testCase.onTestWillDisplay();
        InterpolationManager.getInstance().addInterpolation(move);
        return move;
    }

    public Interpolation viewDetails(final ATestCase testCase) {

        final TextView titleView = (TextView) findViewById(R.id.stt_details_title);
        final LinearLayout logView = (LinearLayout) findViewById(R.id.stt_details_log);

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                logView.removeAllViews();
                mDetailsScrollView.scrollTo(0, 0);
                mTestCaseDetails.setVisibility(View.VISIBLE);
                mTestCaseDetails.setX(Android.getScreenSize(MainActivityFTM.this).x);
                mTestCaseDetails.setAlpha(1.0f);
                titleView.setText(testCase.getName());

                if (testCase.getReport().hasJson()) {
                    LogBuilder.buildLog(logView, Android.getScreenSize(MainActivityFTM.this).x, "", testCase.getReport().getJson(), 0, getContext());
                    logView.requestLayout();
                } else {
                    TextView text = new TextView(getContext());
                    text.setBackgroundColor(Color.BLACK);
                    LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
                    text.setLayoutParams(params);
                    logView.addView(text);
                    text.setText(Html.fromHtml(testCase.getReport().getHtmlLog()));
                }

                mTitle.setText(getString(R.string.test_cases) + " > " + "Test Report");
            }
        });

        Interpolation move = getTransitionInterpolation(mTestCaseDetails, true);
        InterpolationManager.getInstance().addInterpolation(move);
        return move;
    }
}
