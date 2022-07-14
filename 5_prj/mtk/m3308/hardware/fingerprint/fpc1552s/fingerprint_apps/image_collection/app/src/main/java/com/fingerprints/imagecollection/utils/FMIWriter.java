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

package com.fingerprints.imagecollection.utils;

import android.content.Context;
import android.graphics.Bitmap;

import com.fingerprints.fmi.FMI;
import com.fingerprints.fmi.FMIBlockType;
import com.fingerprints.fmi.FMIOpenMode;
import com.fingerprints.fmi.FMIStatus;
import com.fingerprints.imagecollection.imageutils.ImageData;
import com.google.gson.JsonObject;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.TimeZone;

public class FMIWriter extends ImageWriter {
    private Logger mLogger = new Logger(getClass().getSimpleName());

    public FMIWriter(final Context context, final TaskLog taskLog) {
        super(context, "fmi", taskLog);
    }

    public FMIWriter(String info) {
        super();
        mLogger.i(info);
    }

    public void write(final ImageData imageData, String buildInfo) throws Exception {

        File directory = getDirectory(imageData);

        if (directory != null) {
            String fileName = getFileName(imageData);

            File file = new File(directory, fileName);

            FMI fmiWriter = new FMI();
            if (fmiWriter.openFile(file.getAbsolutePath(), FMIOpenMode.WRITE) == FMIStatus.OK) {

                mLogger.i("write: " + file.getAbsolutePath());

                TaskLog.LogPoint logPoint = getTaskLog().add("File").
                        log("name", file.getName()).
                        log("id", imageData.getSampleId()).
                        log("status", imageData.getCaptureStatus());

                if (imageData.hasErrorMessage()) {
                    logPoint.log("errMsgId", imageData.getErrMsgId());
                }

                JsonObject json = new JsonObject();
                FMIBlockType textType = new FMIBlockType();
                textType.format = FMIBlockType.Format.JSON;
                textType.id = FMIBlockType.Id.PUBLIC_METADATA;

                DateFormat dateFormat = new SimpleDateFormat("yyyyMMdd'T'HHmmss'Z'");
                dateFormat.setTimeZone(TimeZone.getTimeZone("UTC"));
                json.addProperty("datetime", dateFormat.format(imageData.getDate()));
                fmiWriter.addBlock(textType, json.toString().getBytes());

                FMIBlockType versionType = new FMIBlockType();
                versionType.format = FMIBlockType.Format.JSON;
                versionType.id = FMIBlockType.Id.VERSION_INFO;
                fmiWriter.addBlock(versionType, ("" + buildInfo).getBytes());

                FMIBlockType rawType = new FMIBlockType();
                rawType.format = FMIBlockType.Format.FPC_IMAGE_DATA;
                rawType.id = FMIBlockType.Id.RAW_IMAGE;
                fmiWriter.addBlock(rawType, imageData.getRawSensorImage().getPixels());

                FMIBlockType preprocessedType = new FMIBlockType();
                preprocessedType.format = FMIBlockType.Format.PNG;
                preprocessedType.id = FMIBlockType.Id.DISPLAY_IMAGE;
                ByteArrayOutputStream baos = new ByteArrayOutputStream();
                Bitmap bitmap = Utils.getBitmap(imageData.getPreprocessedSensorImage(), false);
                bitmap.compress(Bitmap.CompressFormat.PNG, 100, baos);
                fmiWriter.addBlock(preprocessedType, baos.toByteArray());

                FMIStatus status = fmiWriter.close();
                if (status != FMIStatus.OK) {
                    mLogger.e("Failed to save FMI file " + status);
                }
            } else {
                mLogger.e("Failed to open FMI file for writing " + file.getAbsolutePath());
            }
        }
    }
}
