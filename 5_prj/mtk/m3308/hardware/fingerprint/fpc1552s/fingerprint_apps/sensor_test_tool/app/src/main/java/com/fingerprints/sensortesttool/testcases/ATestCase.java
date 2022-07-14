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
package com.fingerprints.sensortesttool.testcases;

import android.content.Context;
import android.content.SharedPreferences;
import android.text.TextUtils;
import android.util.Base64;

import com.fingerprints.extension.sensortest.SensorTestResult;
import com.fingerprints.fmi.FMI;
import com.fingerprints.fmi.FMIBlockType;
import com.fingerprints.fmi.FMIOpenMode;
import com.fingerprints.fmi.FMIStatus;
import com.fingerprints.sensortesttool.ITestController;
import com.fingerprints.sensortesttool.R;
import com.fingerprints.sensortesttool.TestStatus;
import com.fingerprints.sensortesttool.logging.ITestReportInterface;
import com.fingerprints.sensortesttool.logging.MultiReportHandler;
import com.fingerprints.sensortesttool.logging.ReportHandler;
import com.fingerprints.sensortesttool.values.Constants;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.FloatBuffer;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

public abstract class ATestCase implements Runnable {
    private MultiReportHandler mReport;
    private ReportHandler mInternalReport;
    private boolean mIsManual = false;
    private String mName;
    private String mFileName;
    private boolean mSelected;
    private ITestController mController;
    private TestStatus mStatus;
    private Object mAwaitResultSync = new Object();
    private SensorTestResult mSensorTestResult;
    private String mId;
    private String mResultFolder;

    public ATestCase(final String name, final String id, final ITestController controller) {
        this.mStatus = TestStatus.STOPPED;
        this.mName = name;
        this.mId = id;
        this.mController = controller;
        mReport = new MultiReportHandler();
        mInternalReport = new ReportHandler();
        mReport.addHandler(mInternalReport);
        mReport.addHandler(controller.getReport());
    }

    @interface PixelFormat {
        int PIXEL_FORMAT_UNKNOWN = 0;
        int PIXEL_FORMAT_GRAY8 = 1;
        int PIXEL_FORMAT_GRAY16 = 2;
        int PIXEL_FORMAT_BGRA8 = 3;
        int PIXEL_FORMAT_GRAY32F = 4;
        int CTL_PIXEL_FORMAT_MASK = 5;
    }

    public boolean isManual() {
        return mIsManual;
    }

    public void setManual(final boolean manual) {
        mIsManual = manual;
    }

    public ITestController getController() {
        return mController;
    }

    public TestStatus getStatus() {
        return mStatus;
    }

    public String getName() {
        return mName;
    }

    public String getId() {
        return mId;
    }

    public void setSelected(final boolean selected) {
        mSelected = selected;
    }

    public boolean isSelected() {
        return mSelected;
    }

    public String getFileName() {
        return mFileName;
    }

    public ReportHandler getReport() {
        return mInternalReport;
    }

    protected ITestReportInterface getLog() {
        return mReport;
    }

    public void reset() {
        this.mSensorTestResult = null;
        setStatus(TestStatus.STOPPED);
    }

    public void cancel() {
        this.mSensorTestResult = null;
        setStatus(TestStatus.CANCELLED);
        onTestComplete(new SensorTestResult(SensorTestResult.ResultCode.CANCELLED, "", 0, ""));
    }

    public boolean hasResult() {
        return this.mSensorTestResult != null;
    }

    public SensorTestResult getSensorTestResult() {
        return mSensorTestResult;
    }

    private String createFileName() {
        SharedPreferences sharedPref = this.mController.getContext().getSharedPreferences(
                Constants.SENSOR_TEST_SHAREDPREFERENCES, Context.MODE_PRIVATE);
        this.mResultFolder = sharedPref.getString(Constants.SENSOR_TEST_RESULT_FOLDER_KEY,
                Constants.SENSOR_TEST_RESULT_DEFAULT_FOLDER);
        File mDir = new File(mResultFolder);

        mDir.mkdirs();
        StringBuilder sb = new StringBuilder();
        sb.append(mDir.getAbsolutePath());
        sb.append("/");
        sb.append(String.format(Locale.US, "%s_", mName.replace(' ', '_')));
        sb.append(new SimpleDateFormat("yyyyMMddHHmmss", Locale.US).format(new Date()));
        return sb.toString();
    }

    private void writeImage(String imageName, byte[] image) {
        if (mFileName != null) {
            FMI mFMIWriter = new FMI();
            FMIStatus status = mFMIWriter.openFile(mFileName + "_" + imageName + ".fmi", FMIOpenMode.WRITE);
            if (status == FMIStatus.OK) {
                FMIBlockType rawType = new FMIBlockType();
                rawType.format = FMIBlockType.Format.FPC_IMAGE_DATA;
                rawType.id = FMIBlockType.Id.RAW_IMAGE;
                status = mFMIWriter.addBlock(rawType, image);
                if (status != FMIStatus.OK) {
                    getLog().e("failed to add fpc image data block: " + status);
                }
            }
        }
    }

    private void writeBuffer(String fileName, byte[] buffer) {
        FileOutputStream fos = null;
        try {
            File file = new File(fileName);
            fos = new FileOutputStream(file);
            fos.write(buffer);
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (fos != null) {
                try {
                    fos.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private void writeRawImage(String name, int width, int height, byte[] buffer) {
        int byesPerPixel = buffer.length / (width * height);
        String fileName = mFileName + String.format("_%s!%d_%d_%d.raw", name, width, height, byesPerPixel);
        writeBuffer(fileName, buffer);
    }

    private void dumpAsCsvFloat(String name, int length, byte[] buffer) {
        String fileName = mFileName + String.format("_%s!%d.csv", name, length);
        StringBuilder output = new StringBuilder();
        ByteBuffer byteBuffer = ByteBuffer.allocate(length * Float.SIZE);
        FloatBuffer floatBuffer = byteBuffer.asFloatBuffer();
        for (int i = 0; i < length; i++) {
            float f = Float.intBitsToFloat(buffer[i * 4] ^ buffer[i * 4 + 1] << 8 ^ buffer[i * 4 + 2] << 16 ^ buffer[i * 4 + 3] << 24);
            floatBuffer.put(i, f);
            output.append(f).append(",");
        }

        // write to file
        try {
            BufferedWriter bwr = new BufferedWriter(new FileWriter(new File(fileName)));

            //write contents of StringBuffer to a file
            bwr.write(output.toString());

            //flush the stream
            bwr.flush();

            //close the stream
            bwr.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void writeImagesFromJson(JsonElement json, String parentName) {
        if (json.isJsonObject()) {

            JsonObject obj = json.getAsJsonObject();

            for (String t : obj.keySet()) {
                if (t.equals("fpc_image_data")) {
                    JsonObject image = obj.getAsJsonObject(t);
                    JsonElement rawBuffer = image.get("raw");
                    if (rawBuffer.isJsonPrimitive()) {
                        byte[] buffer = Base64.decode(rawBuffer.getAsString(), Base64.DEFAULT);
                        writeImage(parentName, buffer);
                    }
                } else if (t.equals("FpcImage")) {
                    JsonObject image = obj.getAsJsonObject(t);

                    JsonElement rawBuffer = image.get("buffer");
                    if (rawBuffer.isJsonPrimitive()) {
                        byte[] buffer = Base64.decode(image.get("buffer").getAsString(), Base64.DEFAULT);
                        int width = image.get("width").getAsInt();
                        int height = image.get("height").getAsInt();

                        writeRawImage(parentName, width, height, buffer);
                    }
                } else if (t.endsWith("CtlBitMap")) {
                    JsonObject image = obj.getAsJsonObject(t);
                    JsonElement rawBuffer = image.get("buffer");
                    if (rawBuffer.isJsonPrimitive()) {
                        byte[] buffer = Base64.decode(image.get("buffer").getAsString(), Base64.DEFAULT);
                        int width = image.get("width").getAsInt();
                        int height = image.get("height").getAsInt();
                        int pixel_format = image.get("pixel_format").getAsInt();

                        if (pixel_format == PixelFormat.PIXEL_FORMAT_GRAY8) {
                            writeRawImage(parentName, width, height, buffer);
                        } else if (pixel_format == PixelFormat.PIXEL_FORMAT_GRAY32F) {
                            dumpAsCsvFloat(parentName, width * height, buffer);
                        } else {
                            getLog().e("Not supported bitmap pixel format");
                        }
                    }
                } else {
                    writeImagesFromJson(obj.get(t), t);
                }
            }
        }
    }

    private void writeJson(String json) {
        writeBuffer(mFileName + ".json", json.getBytes());
    }

    public abstract String getDescription();

    @Override
    public final void run() {
        getLog().enter();
        mInternalReport.clear();

        try {
            onStartTest(getName());

            synchronized (mAwaitResultSync) {
                if (isManual()) {
                    setStatus(TestStatus.WAITING_FOR_USER);
                }
                this.runTest();
                awaitResult();
            }
        } catch (UnsatisfiedLinkError e) {
            setStatus(TestStatus.ERROR);
            getLog().e("UnsatisfiedLinkError: " + e, e);
            onError(getController().getContext().getString(R.string.error_unknown_error));
        } catch (Exception e) {
            setStatus(TestStatus.ERROR);
            getLog().e("Exception: " + e, e);
            onError(getController().getContext().getString(R.string.error_unknown_error));
        }
        getLog().exit();
    }

    private void awaitResult() {
        synchronized (mAwaitResultSync) {
            if (mSensorTestResult == null) {
                try {
                    mAwaitResultSync.wait();
                } catch (InterruptedException e) {
                    //ignore
                }
            }
        }
    }

    public void setStatus(final TestStatus status) {
        mStatus = status;
        getController().refresh(this);
    }

    protected abstract void runTest() throws Exception;

    protected void onError(final String error) {
        getLog().e(error);
    }

    protected void onStartTest(final String testCase) {
        mFileName = createFileName();
        setStatus(TestStatus.RUNNING);
        getLog().reportText("Starting \"" + getName() + "\"");
        getLog().reportText("-------------------------------------------------------------------------");
    }

    protected void onTestComplete(final SensorTestResult sensorTestResult) {
        if (mStatus.isRunning()) {
            mSensorTestResult = sensorTestResult;

            if (mSensorTestResult.resultCode == SensorTestResult.ResultCode.PASS) {
                StringBuilder sb = new StringBuilder();
                sb.append("ResultCode: ");
                sb.append(mSensorTestResult.resultCode.getString());
                if (mSensorTestResult.resultString.trim().length() != 0) {
                    sb.append("\n\nResultString:\n");
                    sb.append(mSensorTestResult.resultString);
                }

                getLog().reportOk(sb.toString());
            } else {
                StringBuilder sb = new StringBuilder();
                sb.append("ResultCode: ");
                sb.append(mSensorTestResult.resultCode.getString());
                if (!TextUtils.isEmpty(mSensorTestResult.resultString)) {
                    sb.append("\nResultString:\n");
                    sb.append(mSensorTestResult.resultString);
                }

                if (mSensorTestResult.hasErrorCode()) {
                    sb.append("\nErrorCode: ");
                    sb.append(mSensorTestResult.getErrorCode());
                    sb.append("\nModuleErrorCode: ");
                    sb.append(mSensorTestResult.getModuleErrorCode());

                    final String errorCodeString = mSensorTestResult.getErrorCodeString();
                    if (!TextUtils.isEmpty(errorCodeString)) {
                        sb.append("\n\nErrorString:\n").append(errorCodeString);
                    }
                }

                getLog().reportError(sb.toString());
                getLog().reportJson(null);
            }

            String jsonLog = sensorTestResult.log;
            if (!TextUtils.isEmpty(jsonLog)) {
                Gson gson = new GsonBuilder().setPrettyPrinting().create();
                JsonParser jp = new JsonParser();

                try {
                    JsonElement je = jp.parse(jsonLog);
                    getLog().reportJson(je);
                    String jsonString = gson.toJson(je);
                    writeJson(jsonString);
                    writeImagesFromJson(je, null);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }

            getLog().i("SensorTestResult: " + mSensorTestResult);

            setStatus(TestStatus.getStatusFromResult(mSensorTestResult));

            getLog().reportTestComplete(this, mSensorTestResult);

            synchronized (mAwaitResultSync) {
                mAwaitResultSync.notifyAll();
            }
        } else if (mStatus.isCancelled()) {
            synchronized (mAwaitResultSync) {
                mAwaitResultSync.notifyAll();
            }
        }
    }
}
