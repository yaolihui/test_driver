
package com.silead.factorytest;

import android.app.Activity;
import android.content.Intent;
import android.os.SystemProperties;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.ListView;
import android.widget.ProgressBar;

import java.util.ArrayList;
import java.util.Arrays;

import com.silead.manager.FingerManager;
import com.silead.manager.FingerDeadPixelResult;
import com.silead.manager.FingerResult;
import com.silead.manager.FingerSpiResult;
import com.silead.manager.FingerSpeedResult;

public class AutoTestActivity extends Activity {
    private static final String FILE_TAG = "AutoTest";
    private FingerManager mFingerManager;

    private ProgressBar mTestProgressBar;
    private ListView mListView;
    private AutoTestAdapter mDataAdapter;
    private ArrayList<AutoTestAdapter.ItemData> mItemData;
    private int mRunIndex = 0;
    private int mRetryTime = 0;
    private boolean mFinish = false;

    private static final int MSG_AUTO_ITEM_RUN = 2001;
    private static final int ITEM_RUN_DELAY = 200; // ms
    private static final int ITEM_CAPTURE_RETRY_TIMES = 3;
    //shangfei add to exit current activity when test finished 2022/6/21 begin
    private static int last_result = 0;
    private static int RESULT_CODE_PASS = -1;
    private static int RESULT_CODE_FAIL = 0;
    //shangfei add to exit current activity when test finished 2022/6/21 end

    private ArrayList<Integer> mAutoTestList = new ArrayList<>(Arrays.asList(
        R.string.main_test_item_spi,
        R.string.main_test_item_reset_pin,
        R.string.main_test_item_dead_pixel,
        R.string.main_test_item_capture
    ));

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.test_auto_main);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mTestProgressBar = (ProgressBar) findViewById(R.id.test_item_progress_bar);
        mListView = (ListView) findViewById(R.id.test_item_list_view);
        mDataAdapter = new AutoTestAdapter(this);
        mItemData = new ArrayList<AutoTestAdapter.ItemData>();

        int index = 0;
        for (index = 0; index < mAutoTestList.size(); index++) {
            int id = mAutoTestList.get(index);
            AutoTestAdapter.ItemData item = new AutoTestAdapter.ItemData(getString(id));
            mItemData.add(item);
            if (id == R.string.main_test_item_capture) {
                item.setItemDesc(getString(R.string.auto_test_capture_desc));
            }
        }

        mDataAdapter.setDataList(mItemData);
        mListView.setAdapter(mDataAdapter);

        mRunIndex = 0;
        mRetryTime = 0;
        mFinish = false;
        mFingerManager = FingerManager.getDefault(this);
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
        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    private FingerManager.TestCmdCallback mTestCmdCallback = new FingerManager.TestCmdCallback() {
        @Override
        public void onTestResult(int cmdId, Object result) {
            int result_pass = 0;
            if (mRunIndex >= mAutoTestList.size()) {
                return;
            }

            switch (mAutoTestList.get(mRunIndex)) {
                case R.string.main_test_item_spi: {
                    if (cmdId == FingerManager.TEST_CMD_SPI && result instanceof FingerSpiResult) {
                        FingerSpiResult rsp = (FingerSpiResult)result;
                        String strChipId = rsp.getChipId().toString();
                        if (strChipId != null && !strChipId.equals("unknow")) {
                            result_pass = 1;
                        }
                    }
                    mItemData.get(mRunIndex).setItemResult(result_pass);
                    mDataAdapter.notifyDataSetChanged();
                    mRunIndex++;
                    mHandler.sendEmptyMessageDelayed(MSG_AUTO_ITEM_RUN, ITEM_RUN_DELAY);
                    break;
                }
                case R.string.main_test_item_reset_pin: {
                    if (cmdId == FingerManager.TEST_CMD_RESET_PIN && result instanceof FingerResult) {
                        FingerResult rsp = (FingerResult)result;
                        if (rsp.getErrorCode() == FingerManager.TEST_RESULT_OK) {
                            result_pass = 1;
                        }
                    }
                    mItemData.get(mRunIndex).setItemResult(result_pass);
                    mDataAdapter.notifyDataSetChanged();
                    mRunIndex++;
                    mHandler.sendEmptyMessageDelayed(MSG_AUTO_ITEM_RUN, ITEM_RUN_DELAY);
                    break;
                }
                case R.string.main_test_item_dead_pixel: {
                    if (cmdId == FingerManager.TEST_CMD_DEAD_PIXEL && result instanceof FingerDeadPixelResult) {
                        FingerDeadPixelResult rsp = (FingerDeadPixelResult)result;
                        if (rsp.getErrorCode() == FingerManager.TEST_RESULT_OK) {
                            if (rsp.getResult() == 0) {
                                result_pass = 1;
                            }
                        }
                    }
                    mItemData.get(mRunIndex).setItemResult(result_pass);
                    mDataAdapter.notifyDataSetChanged();
                    mRunIndex++;
                    mHandler.sendEmptyMessageDelayed(MSG_AUTO_ITEM_RUN, ITEM_RUN_DELAY);
                    break;
                }
                case R.string.main_test_item_capture: {
                    if (cmdId == FingerManager.TEST_CMD_SPEED_TEST && result instanceof FingerSpeedResult) {
                        FingerSpeedResult rsp = (FingerSpeedResult)result;
                        if (rsp.getErrorCode() != FingerManager.TEST_RESULT_OK) {
                            mRetryTime++;
                            if (mRetryTime < ITEM_CAPTURE_RETRY_TIMES) {
                                mItemData.get(mRunIndex).setItemDesc(getString(R.string.auto_test_capture_retry_desc) + "(" + mRetryTime + "/" + ITEM_CAPTURE_RETRY_TIMES + ")");
                                mDataAdapter.notifyDataSetChanged();
                            } else {
                                mRetryTime = 0;
                                mItemData.get(mRunIndex).setItemResult(0);
                                mDataAdapter.notifyDataSetChanged();
                                mRunIndex++;
                                mHandler.sendEmptyMessageDelayed(MSG_AUTO_ITEM_RUN, ITEM_RUN_DELAY);
                            }
                        } else {
                            mRetryTime = 0;
                            mItemData.get(mRunIndex).setItemResult(1);
                            //shangfei add to exit current activity when test finished 2022/6/21 begin
                            if (SystemProperties.get("ro.product.model", "").equals("PROM")) {
                                result_pass = 1;
                            }
                            //shangfei add to exit current activity when test finished 2022/6/21 end
                            mDataAdapter.notifyDataSetChanged();
                            mRunIndex++;
                            mHandler.sendEmptyMessageDelayed(MSG_AUTO_ITEM_RUN, ITEM_RUN_DELAY);
                        }
                    }
                    break;
                }
            }
            //shangfei add to exit current activity when test finished 2022/6/21 begin
            if (SystemProperties.get("ro.product.model", "").equals("PROM")) {
                if (1 != result_pass) {
                    last_result |= (1 << cmdId);
                }
                Log.d(FactoryTestConst.LOG_TAG, "cmdId = " + cmdId + ", result_pass = " + result_pass + ", last_result" + last_result);
            }
            //shangfei add to exit current activity when test finished 2022/6/21 end
        }
    };

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_AUTO_ITEM_RUN: {
                    if (!mFinish && mRunIndex < mAutoTestList.size()) {
                        switch (mAutoTestList.get(mRunIndex)) {
                            case R.string.main_test_item_spi: {
                                mFingerManager.testSpi(mTestCmdCallback);
                                break;
                            }
                            case R.string.main_test_item_reset_pin: {
                                mFingerManager.testResetPin(mTestCmdCallback);
                                break;
                            }
                            case R.string.main_test_item_dead_pixel: {
                                mFingerManager.testDeadPixel(mTestCmdCallback);
                                break;
                            }
                            case R.string.main_test_item_capture: {
                                mFingerManager.testSpeed(mTestCmdCallback);
                                break;
                            }
                        }
                    } else {
                        mFingerManager.testFinish(mTestCmdCallback);
                        mTestProgressBar.setVisibility(View.INVISIBLE);
                        //shangfei add to exit current activity when test finished 2022/6/21 begin
                        if (SystemProperties.get("ro.product.model", "").equals("PROM")){
                            Log.d(FactoryTestConst.LOG_TAG, "3test finish, exit!");
                            Intent intent = new Intent();
                            Log.d(FactoryTestConst.LOG_TAG, "finish() ErrorCode = 0x" + Integer.toHexString(last_result) + "(" + Integer.toBinaryString(last_result) + ")");
                            intent.putExtra("ErrorCode", last_result);
                            if (last_result == 0){
                                setResult(RESULT_CODE_PASS, intent);
                            } else{
                                setResult(RESULT_CODE_FAIL, intent);
                            }
                            last_result = 0;
                            finish();
                        }
                       //shangfei add to exit current activity when test finished 2022/6/21 end
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }
    };
}
