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

import android.animation.ArgbEvaluator;
import android.animation.ValueAnimator;
import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.graphics.Typeface;
import android.os.Bundle;
import android.os.Vibrator;
import android.app.Activity;
import android.text.Editable;
import android.text.TextWatcher;
import android.text.method.ScrollingMovementMethod;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ExpandableListView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

import com.fingerprints.navigationtest.helpers.Android;
import com.fingerprints.navigationtest.helpers.LogItem;
import com.fingerprints.navigationtest.helpers.NavigationInput;
import com.fingerprints.navigationtest.helpers.TestData;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Timer;
import java.util.TimerTask;

import static com.fingerprints.navigationtest.helpers.NavigationInput.CLICK;
import static com.fingerprints.navigationtest.helpers.NavigationInput.DOUBLE_CLICK;
import static com.fingerprints.navigationtest.helpers.NavigationInput.DOWN;
import static com.fingerprints.navigationtest.helpers.NavigationInput.HARD_PRESS;
import static com.fingerprints.navigationtest.helpers.NavigationInput.HOLD_CLICK;
import static com.fingerprints.navigationtest.helpers.NavigationInput.KEYCODE_FAIL_TO_REGISTER;
import static com.fingerprints.navigationtest.helpers.NavigationInput.LEFT;
import static com.fingerprints.navigationtest.helpers.NavigationInput.RIGHT;
import static com.fingerprints.navigationtest.helpers.NavigationInput.UP;

public class TestModeActivity extends Activity {
    private static final int COLOR_FLASH_GREEN = Color.argb(150, 0, 255, 0);
    private static final int COLOR_FLASH_RED = Color.argb(150, 255, 0, 0);
    private static final long VIBRATE_TIME = 100;
    private static final int SENSOR_EVENTS_CAPTURE_SLEEP_TIME = 500;

    private State mState = State.SETUP;
    private Vibrator mVibrator;
    private Animation mAnimation;
    private Timer mTimer;
    private int mSoughtKey;
    private long mLastUsedKeyEventTime = 0;
    private LogItemAdapter mAdapter;
    private Button mEndTestButton, mFailedToRegisterInputButton, mStartButton, mExitTestSummaryButton,
            mLogButton, mReturnFromLogViewButton, mQuickInputButton, mClearButton;
    private TextView mtestView;
    private RelativeLayout mLogLayout;
    private LinearLayout mTestLayout;
    private ScrollView mSetupScrollView;
    private LinearLayout mTestSelectionLayout;
    private RelativeLayout mScoreScreen;
    private ImageView mTestImageDisplayer;
    private ExpandableListView mListView;
    private HashMap<NavigationInput, EditText> mTestTexts = new HashMap<>();
    private CheckBox mRandomCheckbox;
    private TextView mStatusTotalTestsRemaining, mStatusRate, mStatusTotalSuccessfulInputs,
            mStatusTotalFailedInputs, mStatusTotalInputsText, mTimeDisplayer, mLogView;
    private Statistics mStats;

    private TestData mTestData = new TestData();

    public enum State {
        SETUP, RUN, SHOW_STATS, SHOW_LOG;
    }

    @SuppressLint("SetTextI18n")
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_testmode);
        getActionBar().setDisplayHomeAsUpEnabled(true);

        if (getResources().getBoolean(R.bool.orientation_locked_portrait)) {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        } else {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        }

        mAnimation = AnimationUtils.loadAnimation(getApplicationContext(), R.anim.fade_in);
        mEndTestButton = (Button) findViewById(R.id.endTestButton);
        mStartButton = (Button) findViewById(R.id.startButton);
        mExitTestSummaryButton = (Button) findViewById(R.id.exitTestSummaryButton);
        mLogButton = (Button) findViewById(R.id.logButton);
        mClearButton = (Button) findViewById(R.id.clearButton);
        mQuickInputButton = (Button) findViewById(R.id.quickInputButton);
        mFailedToRegisterInputButton = (Button) findViewById(R.id.failedToRegisterInputButton);
        mReturnFromLogViewButton = (Button) findViewById(R.id.returnFromLogView);
        mEndTestButton.setFocusable(false);
        mVibrator = (Vibrator) getApplicationContext().getSystemService(Context.VIBRATOR_SERVICE);
        mTestSelectionLayout = (LinearLayout) findViewById(R.id.outerFrame);
        mSetupScrollView = (ScrollView) findViewById(R.id.setupScrollView);

        mtestView = (TextView) findViewById(R.id.eventContainer);
        mtestView.setFocusable(true);
        mTestTexts.put(UP, (EditText) findViewById(R.id.numUpTestsInputField));
        mTestTexts.put(RIGHT, (EditText) findViewById(R.id.numRightTestsInputField));
        mTestTexts.put(DOWN, (EditText) findViewById(R.id.numDownTestsInputField));
        mTestTexts.put(LEFT, (EditText) findViewById(R.id.numLeftTestsInputField));
        mTestTexts.put(CLICK, (EditText) findViewById(R.id.numClickTestsInputField));
        mTestTexts.put(DOUBLE_CLICK, (EditText) findViewById(R.id.numDoubleClickTestsInputField));
        mTestTexts.put(HOLD_CLICK, (EditText) findViewById(R.id.numHoldClickTestsInputField));
        mTestTexts.put(HARD_PRESS, (EditText) findViewById(R.id.numHardPressTestsInputField));

        mRandomCheckbox = (CheckBox) findViewById(R.id.randomCheckbox);
        mStatusTotalTestsRemaining = (TextView) findViewById(R.id.statusTotalTestsRemaining);
        mStatusRate = (TextView) findViewById(R.id.statusRate);
        mLogView = (TextView) findViewById(R.id.logView);
        mLogView.setMovementMethod(new ScrollingMovementMethod());
        mStatusTotalSuccessfulInputs = (TextView) findViewById(R.id.statusTotalSuccessfulInputs);
        mStatusTotalFailedInputs = (TextView) findViewById(R.id.statusTotalFailedInputs);
        mStatusTotalInputsText = (TextView) findViewById(R.id.statusTotalInputs);
        mTimeDisplayer = (TextView) findViewById(R.id.timeDisplayer);
        mLogLayout = (RelativeLayout) findViewById(R.id.logLayout);
        mTestLayout = (LinearLayout) findViewById(R.id.testFrame);
        mTestImageDisplayer = (ImageView) findViewById(R.id.testImageDisplayer);
        mTestImageDisplayer.setFocusable(false);
        mScoreScreen = (RelativeLayout) findViewById(R.id.scoreScreen);
        mListView = (ExpandableListView) findViewById(R.id.mListView);

        DisplayMetrics metrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(metrics);
        int width = metrics.widthPixels;

        mListView.setIndicatorBounds(width - Android.dpToPx(50, this), width - Android.dpToPx(10, this));
        mTimer = new Timer();

        mLastUsedKeyEventTime = 0;

        mQuickInputButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final int numEventsToAdd = 1;
                incrementAllTestEvents(numEventsToAdd);
            }
        });

        mClearButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                clearTestInput();
            }
        });

        mStartButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setState(State.RUN);
            }
        });

        mFailedToRegisterInputButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                evaluateTestInput(KEYCODE_FAIL_TO_REGISTER);
            }
        });

        mEndTestButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mStats.abort();
                mTestData.testEnd();
                try {
                    mStats.addTestResults(mTestData);
                } catch (Exception e) {
                    Log.e("", e.toString());
                }
                setState(State.SHOW_STATS);
            }
        });

        mLogButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setState(State.SHOW_LOG);
            }
        });

        mExitTestSummaryButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                clearTestInput();
                setState(State.SETUP);
            }
        });

        mReturnFromLogViewButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setState(State.SHOW_STATS);
            }
        });

        for (final NavigationInput n : NavigationInput.values()) {
            final EditText editText = mTestTexts.get(n);
            editText.addTextChangedListener(new TextWatcher() {
                @Override
                public void afterTextChanged(final Editable s) {

                }

                @Override
                public void beforeTextChanged(final CharSequence s, final int start, final int count, final int after) {

                }

                @Override
                public void onTextChanged(final CharSequence s, final int start, final int before, final int count) {
                    try {
                        if (s.length() > 0) {
                            mTestData.setNumEventsInTest(n, Integer.parseInt(s.toString()));
                        } else {
                            mTestData.setNumEventsInTest(n, 0);
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            });

        }

        setState(State.SETUP);


        mLogView.setTypeface(Typeface.MONOSPACE);
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        return super.dispatchKeyEvent(event);
    }

    private void evaluateTestInput(final int keyCode) {
        mStats.addLevelEntry(keyCode);
        mStats.nextLevel();
        if (keyCode == KEYCODE_FAIL_TO_REGISTER) {
            mFailedToRegisterInputButton.clearFocus();
            mTestData.addFailedToRegisterResult(NavigationInput.fromKeyCode(mSoughtKey));
        } else {
            mTestData.addResult(NavigationInput.fromKeyCode(mSoughtKey),
                    NavigationInput.fromKeyCode(keyCode));
        }

        if (keyCode == mSoughtKey) {
            flash(COLOR_FLASH_GREEN);
        } else {
            flash(COLOR_FLASH_RED);
            mVibrator.vibrate(VIBRATE_TIME);
        }


        iterateAppState();
        updateRunUI();
    }

    private void updateRunUI() {
        mStatusTotalSuccessfulInputs.setText(String.valueOf(mTestData.getTotalNumSuccessfulInputs()));
        mStatusTotalFailedInputs.setText(String.valueOf(mTestData.getTotalNumFailedInputs()));
        mStatusRate.setText(mTestData.getTotalNumSuccessfulInputsAsString());
        mStatusTotalInputsText.setText(String.valueOf(mTestData.getTotalNumTestInputs()));
    }

    private void updateTime() {
        runOnUiThread(new TimerTask() {
            @Override
            public void run() {
                mTimeDisplayer.setText(String.valueOf(mTestData.getSecondsElapsed()));
            }
        });
    }

    private void setState(final State state) {
        State prevState = mState;
        this.mState = state;
        if (state == State.SETUP) {
            mTimer.cancel();
            mLogLayout.setVisibility(View.GONE);
            mEndTestButton.setVisibility(View.GONE);
            mSetupScrollView.setVisibility(View.VISIBLE);
            mTestSelectionLayout.setVisibility(View.VISIBLE);
            mTestLayout.setVisibility(View.GONE);
        } else if (state == State.RUN) {
            if (mTestData.getTotalNumEventsInTest() > 0) {
                mTestData.testStart();
                mTimer = new Timer();
                updateTime();

                mTimer.scheduleAtFixedRate(new TimerTask() {
                    @Override
                    public void run() {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                mTestData.incSecondsElapsed();
                                updateTime();
                            }
                        });
                    }
                }, 1000, 1000);
                mEndTestButton.setVisibility(View.VISIBLE);
                mTestSelectionLayout.setVisibility(View.GONE);
                mSetupScrollView.setVisibility(View.GONE);
                mTestLayout.setVisibility(View.VISIBLE);

                mtestView.requestFocus();

                mTestData.createTestSequence(mRandomCheckbox.isChecked());
                mStats = new Statistics((LinkedList<TestEvent>) mTestData.getTestSequence().clone(), this);
                mStats.newLog();

                final View.OnKeyListener keyListener = new View.OnKeyListener() {
                    @Override
                    public boolean onKey(View v, int keyCode, KeyEvent event) {

                        if (keyCode == KeyEvent.KEYCODE_BACK) {
                            return false;
                        }

                        if (event.getEventTime() < mLastUsedKeyEventTime + SENSOR_EVENTS_CAPTURE_SLEEP_TIME) {
                            return false;
                        } else {
                            mLastUsedKeyEventTime = event.getEventTime();
                        }

                        if (mState == State.RUN && event.getAction() == KeyEvent.ACTION_DOWN &&
                                event.getRepeatCount() == 0) {
                            evaluateTestInput(keyCode);
                        }

                        return true;
                    }
                };
                mtestView.setOnKeyListener(keyListener);


                updateRunUI();

                iterateAppState();
            } else {
                Toast.makeText(TestModeActivity.this, "Faulty input detected. Please enter " +
                        "valid input.", Toast.LENGTH_SHORT).show();
                mState = prevState;
            }
        } else if (state == State.SHOW_STATS) {
            mStatusTotalInputsText.setText(String.valueOf(mTestData.getTotalNumTestInputs()));
            mTimer.cancel();
            presentData();
            mTestLayout.setVisibility(View.GONE);
            mScoreScreen.setVisibility(View.VISIBLE);
            mLogLayout.setVisibility(View.GONE);
        } else if (state == State.SHOW_LOG) {
            mScoreScreen.setVisibility(View.GONE);
            mLogView.setScrollY(0);
            mLogView.setText(mStats.getLog());
            mLogLayout.setVisibility(View.VISIBLE);
        } else {
            mState = prevState;
        }
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.menu1, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.input:
                Intent toTest = new Intent(TestModeActivity.this, TestModeActivity.class);
                startActivity(toTest);
                finish();
                return true;
            case R.id.log:
                Intent toLog = new Intent(TestModeActivity.this, LogActivity.class);
                startActivity(toLog);
                finish();
                return true;
            case R.id.settings:
                Intent toSettings = new Intent(TestModeActivity.this, Settings.class);
                startActivity(toSettings);
                finish();
                return true;
            case android.R.id.home:
                Intent toDemo = new Intent(TestModeActivity.this, DemoActivity.class);
                startActivity(toDemo);
                finish();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    @SuppressLint("SetTextI18n")
    private void iterateAppState() {
        if (mTestData.isComplete()) {
            mTestData.testEnd();
            try {
                mStats.addTestResults(mTestData);
            } catch (Exception e) {
                Log.e("", e.toString());
            }
            setState(State.SHOW_STATS);
        } else {
            mSoughtKey = mTestData.getTestSequence().peek().getKeyIdentifier();
            TestEvent t = mTestData.getTestSequence().pop();
            mTestImageDisplayer.setImageResource(t.getRID());
            mTestImageDisplayer.startAnimation(mAnimation);
            mStatusTotalTestsRemaining.setText(String.valueOf((mTestData.getTestsRemaining() + 1)));

            mStatusRate.setText(mTestData.getTotalNumSuccessfulInputsAsString());
        }
        mtestView.requestFocus();
    }


    private void presentData() {
        ArrayList<LogItem> list = new ArrayList<>();
        try {
            list.add(new LogItem("Total inputs: ", mTestData.getTotalNumTestInputs()));

            LogItem success = new LogItem("Successful inputs: ",
                    mTestData.getTotalNumSuccessfulInputsAsString() + " (" +
                            mTestData.getTotalNumSuccessfulInputsAsPercent() + "%)");
            for (NavigationInput n : NavigationInput.values()) {
                if (mTestData.getNumEventsInTest(n) > 0) {
                    success.add(n.toString() + ": ", mTestData.getNumSuccessfulInputsAsString(n) +
                            " (" + mTestData.getNumSuccessfulInputsAsPercent(n) + "%)");
                }
            }

            list.add(success);

            list.add(new LogItem("Random order: ", mRandomCheckbox.isChecked()));
            list.add(new LogItem("Total time: ", mTestData.getTotalTimeAsString()));
            list.add(new LogItem("Average time per test: ", mTestData.getTotalNumTestInputs() > 0 ?
                    (((double) (mTestData.getTotalTime() / mTestData.getTotalNumTestInputs()) / 1000) + "s") : "N/A"));

        } catch (Exception e) {
            Log.e(e.toString(), "Error in presentData()");
            e.printStackTrace();
        }
        mAdapter = new LogItemAdapter(this, list);
        mListView.setAdapter(mAdapter);
    }

    private void flash(final int color) {
        runOnUiThread(new TimerTask() {
            @Override
            public void run() {
                mTestLayout.setBackgroundColor(color);

                ValueAnimator colorAnimation = ValueAnimator.ofObject(new ArgbEvaluator(), color, Color.WHITE);
                colorAnimation.setDuration(200); // milliseconds
                colorAnimation.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
                    @Override
                    public void onAnimationUpdate(ValueAnimator animator) {
                        mTestLayout.setBackgroundColor((int) animator.getAnimatedValue());
                    }
                });
                colorAnimation.start();
            }
        });
    }

    private void clearTestInput() {
        mTestData.reset();
        for (NavigationInput n : NavigationInput.values()) {
            mTestTexts.get(n).setText("");
        }
        mRandomCheckbox.setChecked(false);
    }

    private void clearTestData() {
        mTestData.resetTime();
        mTestData.resetResults();
    }

    @SuppressLint("SetTextI18n")
    private void incrementAllTestEvents(final int eventCount) {
        mTestData.addAllEventsToTest(eventCount);

        for (NavigationInput n : NavigationInput.values()) {
            mTestTexts.get(n).setText(String.valueOf(mTestData.getNumEventsInTest(n)));
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            if (mState == State.SHOW_LOG) {
                setState(State.SHOW_STATS);
            } else if (mState == State.SETUP) {
                finish();
            } else if (mState == State.RUN) {
                if (mStats != null) {
                    mStats.abort();
                }

                clearTestData();
                setState(State.SETUP);
            } else {
                clearTestInput();
                setState(State.SETUP);
            }
            return true;
        }
        return false;
    }
}
