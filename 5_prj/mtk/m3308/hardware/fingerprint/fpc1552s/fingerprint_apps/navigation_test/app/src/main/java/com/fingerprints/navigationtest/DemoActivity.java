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


import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.app.Activity;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
//import android.view.MotionEvent;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.Switch;
//import android.widget.TextView;

import com.fingerprints.navigationtest.view.ForceBarView;
//import static com.fingerprints.navigationtest.Settings.MAX_FORCE;

public class DemoActivity extends Activity {
    private Button mStartDemoButton;
    private ImageView mImageViewer;
    private Animation mAnimation;
    private RelativeLayout mDemoStartLayout, mDemoRunningLayout, mForceBarLayout;
//    private TextView mForceBarValueText;
//    private ForceBarView mForceBarView;
    private Switch mSenseTouchSwitch;
    private boolean mDemoSenseTouch = true;

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_demo);
        getActionBar().setDisplayHomeAsUpEnabled(true);
        getActionBar().setTitle(R.string.demo);

        if (getResources().getBoolean(R.bool.orientation_locked_portrait)) {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        } else {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        }


        mDemoStartLayout = (RelativeLayout) findViewById(R.id.demoStartLayout);
        mDemoRunningLayout = (RelativeLayout) findViewById(R.id.demoRunningLayout);
        mDemoRunningLayout.setVisibility(View.GONE);
        mForceBarLayout = (RelativeLayout) findViewById(R.id.forceBarLayout);
        mForceBarLayout.setVisibility(View.GONE);
        mAnimation = AnimationUtils.loadAnimation(getApplicationContext(), R.anim.fade_in);
        mImageViewer = (ImageView) findViewById(R.id.imageViewDemo);
//        mForceBarValueText = (TextView) findViewById(R.id.forceBarValueText);
//        mForceBarView = (ForceBarView) findViewById(R.id.forceBarView);
        mSenseTouchSwitch = (Switch) findViewById(R.id.senseTouchSwitch);

        mStartDemoButton = (Button) findViewById(R.id.startDemoButton);
        mStartDemoButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mDemoSenseTouch = mSenseTouchSwitch.isChecked();
                mDemoStartLayout.setVisibility(View.GONE);
                mDemoRunningLayout.setVisibility(View.VISIBLE);
                mImageViewer.setImageResource(0);

                if (mDemoSenseTouch) {
                    mForceBarLayout.setVisibility(View.VISIBLE);
                } else {
                    mForceBarLayout.setVisibility(View.GONE);
                }
            }
        });
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
                Intent toTest = new Intent(DemoActivity.this, TestModeActivity.class);
                startActivity(toTest);
                finish();
                return true;
            case R.id.log:
                Intent toLog = new Intent(DemoActivity.this, LogActivity.class);
                startActivity(toLog);
                finish();
                return true;
            case R.id.settings:
                Intent toSettings = new Intent(DemoActivity.this, Settings.class);
                startActivity(toSettings);
                finish();
                return true;
            case android.R.id.home:
                finish();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    private void close() {
        mDemoRunningLayout.setVisibility(View.GONE);
        mDemoStartLayout.setVisibility(View.VISIBLE);
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            if (mDemoStartLayout.getVisibility() == View.VISIBLE) {
                finish();
            } else {
                close();
            }
            return true;
        }
        return false;

    }

    /*
    @Override
    public boolean dispatchGenericMotionEvent(MotionEvent motionEvent) {
        int axisValue = (int) (motionEvent.getAxisValue(MotionEvent.AXIS_LTRIGGER) * MAX_FORCE);
        Log.d("fpc", "Received force value: " + axisValue);

        if (mDemoSenseTouch) {
            mForceBarValueText.setText(Integer.toString(axisValue));
            mForceBarView.setForce(axisValue);
        }

        return super.dispatchGenericMotionEvent(motionEvent);
    }
    */

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        int resource = -1;
        int keyCode = event.getKeyCode();
        int keyStatus = event.getAction();

        Log.d("fpc", "Received key event: " + keyCode + " keyStatus: " + keyStatus);

        if (keyStatus == KeyEvent.ACTION_DOWN && event.getRepeatCount() == 0) {
            switch (keyCode) {
                case KeyEvent.KEYCODE_DPAD_UP:
                    resource = R.drawable.up;
                    break;
                case KeyEvent.KEYCODE_DPAD_RIGHT:
                    resource = R.drawable.right;
                    break;
                case KeyEvent.KEYCODE_DPAD_DOWN:
                    resource = R.drawable.down;
                    break;
                case KeyEvent.KEYCODE_DPAD_LEFT:
                    resource = R.drawable.left;
                    break;
                case KeyEvent.KEYCODE_BUTTON_A:
                    resource = R.drawable.click;
                    break;
                case KeyEvent.KEYCODE_BUTTON_B:
                    resource = R.drawable.hold_click;
                    break;
                case KeyEvent.KEYCODE_BUTTON_C:
                    resource = R.drawable.double_click;
                    break;
                case KeyEvent.KEYCODE_BUTTON_X:
                    if (mDemoSenseTouch) {
                        resource = R.drawable.hard_press;
                    }
                    break;
                default:
                    return super.dispatchKeyEvent(event);
            }

            if (resource != -1) {
                mImageViewer.setImageResource(resource);
                mImageViewer.startAnimation(mAnimation);
            }
        }
        return true;
    }
}
