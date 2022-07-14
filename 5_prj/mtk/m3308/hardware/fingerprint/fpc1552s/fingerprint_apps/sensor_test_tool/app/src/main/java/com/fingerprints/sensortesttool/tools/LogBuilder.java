/*
 *
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */
package com.fingerprints.sensortesttool.tools;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Typeface;
import android.util.Base64;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.fingerprints.sensortesttool.R;
import com.google.gson.Gson;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonPrimitive;

public class LogBuilder {
    private static final int LAYOUT_MARGIN = 10;
    private static void addImage(final LinearLayout logView, int ImageView_width, JsonElement json, Context context) {
        JsonObject image = json.getAsJsonObject();
        byte[] buffer = Base64.decode(image.get("buffer").getAsString(), Base64.DEFAULT);
        int width = image.get("width").getAsInt();
        int height = image.get("height").getAsInt();
        int sensorControlSize = width * height;

        if (buffer.length != sensorControlSize) {
            TextView textView = new TextView(context);
            textView.setText("(invalid size)");
            textView.setTextSize(Android.dpToPx(5, context));
            logView.addView(textView);
        } else {
            RelativeLayout layout = new RelativeLayout(context);
            RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.WRAP_CONTENT);
            params.bottomMargin = Android.dpToPx(10, context);
            params.topMargin = Android.dpToPx(10, context);
            layout.setLayoutParams(params);

            Bitmap bitmap = getBitmap(buffer, width, height, true);
            ImageView imageView = new ImageView(context);
            int w, h, temp;
            if (height > width) {
                //Do rotation for the biggest display image
                temp = width;
                width = height;
                height = temp;
            }
            //Resize to fit the view layout
            w = ImageView_width;
            h = w * height / width;

            RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(w/2, h/2);
            layoutParams.addRule(RelativeLayout.CENTER_IN_PARENT, RelativeLayout.TRUE);
            layoutParams.setMargins(LAYOUT_MARGIN, LAYOUT_MARGIN, LAYOUT_MARGIN, LAYOUT_MARGIN);
            imageView.setLayoutParams(layoutParams);
            imageView.requestLayout();
            imageView.setImageBitmap(bitmap);
            imageView.setScaleType(ImageView.ScaleType.FIT_XY);
            layout.addView(imageView);

            logView.addView(layout);
        }
    }

    private static Bitmap getBitmap(byte[] image, int width, int height, boolean rotate) {
        byte[] imageData = image;
        int frameSize = width * height;
        int[] colourArray = new int[imageData.length];
        int alpha;
        for (int i = 0; i < frameSize; i++) {
            // This promotes the pixel value to an int,
            // eliminating the MSB. This step may seem redundant
            // but it is not.
            alpha = imageData[i] & 0x00FF;
            colourArray[i] = Color.argb(0xFF, alpha, alpha, alpha);
        }
        Bitmap bmSource = Bitmap.createBitmap(colourArray, width, height, Bitmap.Config.ARGB_8888);
        Matrix matrix = new Matrix();
        if (rotate && height > width) {
            matrix.postRotate(-90);
            matrix.postScale(-1f, 1f);
        }
        return Bitmap.createBitmap(bmSource, 0, 0, width, height, matrix, true);
    }

    private static String toCamelCase(final String init) {
        if (init == null) return null;

        final StringBuilder ret = new StringBuilder(init.length());

        for (final String word : init.split("_")) {
            if (!word.isEmpty()) {
                ret.append(word.substring(0, 1).toUpperCase());
                ret.append(word.substring(1).toLowerCase());
            }
            if (!(ret.length() == init.length())) ret.append(" ");
        }

        return ret.toString();
    }

    private static LinearLayout create(final LinearLayout parent, String title, Context context) {
        LinearLayout layout = new LinearLayout(context);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        params.setMargins(0, 0, 0, 0);
        layout.setLayoutParams(params);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setElevation(Android.dpToPx(5, context));

        {
            TextView textView = new TextView(context);
            textView.setText(title);
            textView.setTextSize(Android.dpToPx(7, context));
            textView.setTypeface(Typeface.DEFAULT_BOLD);
            layout.addView(textView);

        }

        LinearLayout innerlayout = new LinearLayout(context);
        LinearLayout.LayoutParams params2 = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        params2.setMargins(0, 10, 0, 20);
        innerlayout.setPadding(10, 10, 10, 10);
        innerlayout.setLayoutParams(params2);
        innerlayout.setBackground(context.getDrawable(R.drawable.back));
        innerlayout.setOrientation(LinearLayout.VERTICAL);
        innerlayout.setElevation(Android.dpToPx(5, context));
        layout.addView(innerlayout);

        parent.addView(layout);

        return innerlayout;
    }

    public static void buildLog(final LinearLayout logView, int ImageView_width, String title, JsonElement json, int level, Context context) {

        LinearLayout layout = logView;

        int size = 15;

        if (json.isJsonObject()) {
            title = toCamelCase(title);
            String text = title;

            if (!text.equals("")) {
                if (level == 1) {
                    layout = create(logView, text, context);
                } else {
                    TextView textView = new TextView(context);
                    textView.setText(title);
                    textView.setTextSize(Android.dpToPx(10 - level, context));
                    textView.setTypeface(Typeface.DEFAULT_BOLD);
                    layout.addView(textView);
                }
            }

            JsonObject obj = json.getAsJsonObject();

            for (String t : obj.keySet()) {
                if (t.equals("fpc_image_data")) {
                    JsonObject image = obj.getAsJsonObject(t);
                    for (String buf_name : image.keySet()) {
                        if (image.get(buf_name).isJsonObject()) {
                            JsonObject buf = image.getAsJsonObject(buf_name);
                            addImage(layout, ImageView_width, buf, context);
                        }
                    }
                } else if (t.equals("FpcImage")) {
                    addImage(layout, ImageView_width, obj.get(t), context);
                } else if (t.equals("CtlBitMap")) {
                    buildLog(layout, ImageView_width, t, new Gson().fromJson(new JsonPrimitive("[bitmap]"), JsonElement.class), level + 1, context);
                } else {
                    buildLog(layout, ImageView_width, t, obj.get(t), level + 1, context);
                }
            }
        } else if (json.isJsonPrimitive()) {
            JsonPrimitive obj = json.getAsJsonPrimitive();
            TextView textView = new TextView(context);
            textView.setText(Character.toUpperCase(title.charAt(0)) + title.substring(1).replaceAll("_", " ") + ": " + obj.toString().replaceAll("\"", ""));
            textView.setTextSize(Android.dpToPx(6, context));
            layout.addView(textView);
        }
    }
}
