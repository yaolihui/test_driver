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

import android.app.Activity;
import android.os.Bundle;
import android.text.Html;
import android.text.method.ScrollingMovementMethod;
import android.view.MenuItem;
import android.view.View;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toolbar;

import com.fingerprints.sensortesttool.R;

public class LogActivity extends Activity {

    public static final String LOG_DATA = "log_data";

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_log);

        // Set a toolbar to replace the action bar.
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        toolbar.setTitle("Result");

        final ScrollView scrollView = (ScrollView) findViewById(R.id.log_scrollview);
        final TextView resultView = (TextView) findViewById(R.id.log_result_textview);

        String logData = (String) getIntent().getStringExtra(LOG_DATA);

        resultView.setText(Html.fromHtml(logData));
        resultView.setVisibility(View.INVISIBLE);
        resultView.setMovementMethod(new ScrollingMovementMethod());

        //make it scroll without ui-glitch
        scrollView.post(new Runnable() {
            @Override
            public void run() {
                scrollView.scrollTo(0, resultView.getBottom());
                resultView.setVisibility(View.VISIBLE);

            }
        });
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                onBackPressed();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }
}
