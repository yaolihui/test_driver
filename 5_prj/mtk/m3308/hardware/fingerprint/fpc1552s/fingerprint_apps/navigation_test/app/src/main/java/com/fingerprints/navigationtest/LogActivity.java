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

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.app.Activity;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ListView;
import android.widget.TextView;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Collections;

public class LogActivity extends Activity {

    private Boolean deleteSingle;
    private String targetLog = "";
    private FrameLayout mOverviewFrame, mLogFrame;
    private ListView mLogList;
    private ArrayList<String> mStringList;
    private ArrayAdapter<String> mAdapter;
    private TextView mLogDisplayer, mNoLogsText;

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_log);
        getActionBar().setDisplayHomeAsUpEnabled(true);

        if (getResources().getBoolean(R.bool.orientation_locked_portrait)) {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        } else {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        }

        mOverviewFrame = (FrameLayout) findViewById(R.id.overviewFrame);
        mLogFrame = (FrameLayout) findViewById(R.id.logFrame);
        mLogDisplayer = (TextView) findViewById(R.id.logDisplayer);
        mLogList = (ListView) findViewById(R.id.allLogsViewer);
        mNoLogsText = (TextView) findViewById(R.id.noLogsText);
        mLogList.setClickable(true);
        mLogDisplayer.setMovementMethod(new ScrollingMovementMethod());


        mStringList = new ArrayList<>();
        File dir = new File(getFilesDir().toString());
        File[] filesList = dir.listFiles();
        for (File f : filesList) {
            if (f.isFile()) {
                mStringList.add(f.getName());
            }
        }
        if (mStringList.size() < 1) {
            mOverviewFrame.setVisibility(View.GONE);
            mNoLogsText.setVisibility(View.VISIBLE);
        }
        Collections.reverse(mStringList);

        mAdapter = new ArrayAdapter<>(this, android.R.layout.simple_list_item_1, mStringList);
        mLogList.setAdapter(mAdapter);
        mLogList.setOnItemClickListener(new AdapterView.OnItemClickListener() {

            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                mOverviewFrame.setVisibility(View.GONE);
                mLogFrame.setVisibility(View.VISIBLE);
                mLogDisplayer.setText(readInputFromFilename(mStringList.get(position)));
                targetLog = mStringList.get(position);
            }
        });

        Button returnButton = (Button) findViewById(R.id.returnButton);
        if (returnButton != null) {
            returnButton.setOnClickListener(new View.OnClickListener() {
                public void onClick(View v) {
                    targetLog = "";
                    mLogFrame.setVisibility(View.GONE);
                    mOverviewFrame.setVisibility(View.VISIBLE);
                }
            });
        }

        Button deleteCurrentLogButton = (Button) findViewById(R.id.deleteCurrentLogButton);
        if (deleteCurrentLogButton != null) {
            deleteCurrentLogButton.setOnClickListener(new View.OnClickListener() {
                public void onClick(View v) {
                    deleteSingle = true;
                    showDeleteLogDialog("Delete this log?");
                }
            });
        }
        Button deleteAllLogsButton = (Button) findViewById(R.id.clearFilesButton);
        deleteSingle = false;
        if (deleteAllLogsButton != null) {
            deleteAllLogsButton.setOnClickListener(new View.OnClickListener() {
                public void onClick(View v) {
                    int logCount = 0;
                    deleteSingle = false;
                    File dir = new File(getFilesDir().toString());
                    File[] filesList = dir.listFiles();
                    for (File f : filesList) {
                        if (f.isFile()) {
                            logCount++;
                        }
                    }
                    showDeleteLogDialog("Delete " + logCount + " logs?");
                }
            });
        }
    }

    private boolean showDeleteLogDialog(String message) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        String title = "Confirmation";
        builder.setTitle(title);
        builder.setMessage(message);
        builder.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int whichButton) {
                File dir = new File(getFilesDir().toString());
                File[] filesList = dir.listFiles();
                for (File f : filesList) {
                    if (f.isFile() && !deleteSingle) {
                        mAdapter.remove(f.getName());
                        f.delete();
                    } else {
                        if (f.isFile() && f.getName().equals(targetLog)) {
                            mAdapter.remove(f.getName());
                            f.delete();
                        }
                    }
                }
                mAdapter.notifyDataSetChanged();

                if (deleteSingle) {
                    mLogFrame.setVisibility(View.GONE);
                    mOverviewFrame.setVisibility(View.VISIBLE);
                } else {
                    mOverviewFrame.setVisibility(View.GONE);
                    mNoLogsText.setVisibility(View.VISIBLE);
                }
            }
        });
        builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int whichButton) {
            }
        });
        builder.show();
        return false;
    }

    private String readInputFromFilename(String filename) {
        String contents = "";
        try {
            InputStream inputStream = getApplicationContext().openFileInput(filename);
            if (inputStream != null) {
                InputStreamReader inputSteamReader = new InputStreamReader(inputStream);
                BufferedReader reader = new BufferedReader(inputSteamReader);
                StringBuilder sb = new StringBuilder();
                String receiveString;
                sb.append("Log file name: " + filename + " \n");
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
                Intent toTest = new Intent(LogActivity.this, TestModeActivity.class);
                startActivity(toTest);
                finish();
                return true;
            case R.id.log:
                Intent toLog = new Intent(LogActivity.this, LogActivity.class);
                startActivity(toLog);
                finish();
                return true;
            case R.id.settings:
                Intent toSettings = new Intent(LogActivity.this, Settings.class);
                startActivity(toSettings);
                finish();
                return true;
            case android.R.id.home:
                Intent toDemo = new Intent(LogActivity.this, DemoActivity.class);
                startActivity(toDemo);
                finish();
                return true;
            default:
                return super.onOptionsItemSelected(item);
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

}
