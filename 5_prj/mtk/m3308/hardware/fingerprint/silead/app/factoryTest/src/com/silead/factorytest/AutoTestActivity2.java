
package com.silead.factorytest;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.ListView;
import android.widget.ProgressBar;

import java.util.ArrayList;
import java.util.Arrays;

public class AutoTestActivity2 extends Activity {
    private static final String FILE_TAG = "AutoTest2";

    private ProgressBar mTestProgressBar;
    private ListView mListView;
    private AutoTestAdapter mDataAdapter;
    private ArrayList<AutoTestAdapter.ItemData> mItemData;
    private int mRunIndex = 0;
    private boolean mFinish = false;
    boolean spi_result = false;
    boolean pin_result = false;
    boolean bad_point_result = false;
    private static final int MSG_AUTO_ITEM_RUN = 2001;
    private static final int ITEM_RUN_DELAY = 200; // ms

    private ArrayList<Integer> mAutoTestList = new ArrayList<>(Arrays.asList(
        R.string.main_test_item_spi,
        R.string.main_test_item_reset_pin,
        R.string.main_test_item_dead_pixel
    ));

    private ArrayList<?> mAutoTestListClass = new ArrayList<>(Arrays.asList(
        SpiTestActivity.class,
        ResetPinTestActivity.class,
        DeadPixelTestActivity.class
    ));

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.test_auto_main);

        mTestProgressBar = (ProgressBar) findViewById(R.id.test_item_progress_bar);
        mListView = (ListView) findViewById(R.id.test_item_list_view);
        mDataAdapter = new AutoTestAdapter(this);
        mItemData = new ArrayList<AutoTestAdapter.ItemData>();

        int index = 0;
        for (index = 0; index < mAutoTestList.size(); index++) {
            mItemData.add(new AutoTestAdapter.ItemData(getString(mAutoTestList.get(index)), -1));
        }

        mDataAdapter.setDataList(mItemData);
        mListView.setAdapter(mDataAdapter);

        mRunIndex = 0;
        mFinish = false;

        mHandler.sendEmptyMessageDelayed(MSG_AUTO_ITEM_RUN, ITEM_RUN_DELAY);
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mFinish = true;
    }

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_AUTO_ITEM_RUN: {
                    if (!mFinish && mRunIndex < mAutoTestList.size()) {
                        startItemRun(mRunIndex);
                    } else {
                        mTestProgressBar.setVisibility(View.INVISIBLE);
                        if(spi_result && pin_result && bad_point_result){
                            setResult(RESULT_OK);
                        }else{
                            setResult(RESULT_CANCELED);
                        }
                        finish();

                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }
    };

    private void startItemRun(int index) {
        if (index < mAutoTestListClass.size() && index < mAutoTestList.size()) {
            Intent intent = new Intent(this, (Class<?>) mAutoTestListClass.get(index));
            intent.putExtra(FactoryTestConst.LAUNCH_MODE_KEY, FactoryTestConst.LAUNCH_MODE_PCBA);
            startActivityForResult(intent, mAutoTestList.get(index));
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        int result_pass = 0;
        boolean is_result_test = false;

        if (mAutoTestList.get(mRunIndex) == requestCode) {
            if (0 == resultCode) {
                result_pass = 1;
            }
            is_result_test = true;
        }
        Log.e("AutoTestActivity2","is_result_test="+is_result_test+"requestCode="+requestCode+"mRunIndex="+mRunIndex);
        if (is_result_test) {
            if(mRunIndex == 0){
                spi_result = true;
            }else if(mRunIndex == 1){
                pin_result =true;
            }else if(mRunIndex == 2){
                bad_point_result = true;
            }
            Log.e("AutoTestActivity2","spi_result== "+spi_result+"pin_result== "+pin_result+"bad_point_result== "+bad_point_result);
            mDataAdapter.notifyDataSetChanged();
            Log.e("AutoTestActivity2","result_pass"+result_pass);
            mItemData.get(mRunIndex).setItemResult(result_pass);
            mRunIndex++;
            mHandler.sendEmptyMessageDelayed(MSG_AUTO_ITEM_RUN, ITEM_RUN_DELAY);
        }
    }
}