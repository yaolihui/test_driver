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

import android.content.Context;
import android.util.Log;
import android.view.KeyEvent;

import com.fingerprints.navigationtest.helpers.NavigationTestResultTable;
import com.fingerprints.navigationtest.helpers.TestData;

import java.io.BufferedReader;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.LinkedList;
import java.util.Locale;

import static com.fingerprints.navigationtest.helpers.NavigationInput.KEYCODE_FAIL_TO_REGISTER;

class Statistics {
    private final LinkedList<TestEvent> mList;
    private final Context mContext;
    private final DateFormat mDateFormat;
    private final DateFormat mTimeFormat;
    private String currentFileName;

    public Statistics(LinkedList<TestEvent> mList, Context context) {
        this.mList = mList;
        this.mContext = context;
        mDateFormat = new SimpleDateFormat("yyyyMMdd-HHmmss", Locale.ENGLISH);
        mTimeFormat = new SimpleDateFormat("HHmmssSSS", Locale.ENGLISH);
        newLog();
        nextLevel();
    }

    public void addLevelEntry(int keyCode) {
        try {
            FileOutputStream fOut = mContext.openFileOutput(currentFileName, Context.MODE_APPEND);
            OutputStreamWriter out = new OutputStreamWriter(fOut);
            out.write("[" + mTimeFormat.format(new Date(System.currentTimeMillis())) +
                    "]           Input: " + translate(keyCode) + "\n");
            out.flush();
            out.close();
        } catch (IOException e) {
            Log.e("IO error", e.toString());
        }
    }

    public void nextLevel() {
        if (mList.peek() != null) {
            try {
                FileOutputStream fOut = mContext.openFileOutput(currentFileName,
                        Context.MODE_APPEND);
                OutputStreamWriter out = new OutputStreamWriter(fOut);
                out.write("[" + mTimeFormat.format(new Date(System.currentTimeMillis())) +
                        "] Active test: " + translate(mList.pop().getKeyIdentifier()) + "\n");
                out.flush();
                out.close();
            } catch (IOException e) {
                Log.e("IO error", e.toString());
            }
        }
    }

    public String getLog() {
        String contents = "";
        try {
            InputStream inputStream = mContext.openFileInput(currentFileName);
            if (inputStream != null) {
                InputStreamReader inputSteamReader = new InputStreamReader(inputStream);
                BufferedReader reader = new BufferedReader(inputSteamReader);
                StringBuilder sb = new StringBuilder();
                String receiveString;
                sb.append("Log file name: " + currentFileName + " \n");
                while ((receiveString = reader.readLine()) != null) {
                    sb.append(receiveString + " \n");
                }
                inputStream.close();
                contents = sb.toString();
            }
        } catch (Exception e) {
            Log.e("Read error", e.toString());
        }
        return contents;
    }

    public void newLog() {
        long now = System.currentTimeMillis();
        currentFileName = mDateFormat.format(new Date(now));
    }

    private String translate(int keyCode) {
        switch (keyCode) {
            case KEYCODE_FAIL_TO_REGISTER:
                return "NO EVENT";
            case KeyEvent.KEYCODE_DPAD_UP:
                return "UP";
            case KeyEvent.KEYCODE_DPAD_RIGHT:
                return "RIGHT";
            case KeyEvent.KEYCODE_DPAD_DOWN:
                return "DOWN";
            case KeyEvent.KEYCODE_DPAD_LEFT:
                return "LEFT";
            case KeyEvent.KEYCODE_BUTTON_A:
                return "CLICK";
            case KeyEvent.KEYCODE_BUTTON_B:
                return "HOLD_CLICK";
            case KeyEvent.KEYCODE_BUTTON_C:
                return "DOUBLE_CLICK";
            case KeyEvent.KEYCODE_BUTTON_X:
                return "FORCE PRESS";
            //case KEYCODE_SOFT_PRESS:
            //    return "SOFT_PRESS";
            default:
                return "UNKNOWN KEY PRESSED";
        }
    }

    public void addTestResults(TestData data) {
        NavigationTestResultTable.writeToFile(mContext, currentFileName, data);
    }

    public void abort() {
        try {
            FileOutputStream fOut = mContext.openFileOutput(currentFileName, Context.MODE_APPEND);
            OutputStreamWriter out = new OutputStreamWriter(fOut);
            out.write("----TEST ABORTED----");
            out.flush();
            out.close();
        } catch (IOException e) {
            Log.e("IO error", e.toString());
        }
    }
}
