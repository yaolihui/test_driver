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
package com.fingerprints.sensortesttool.views;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.Shader;
import android.util.AttributeSet;
import android.view.View;

import com.fingerprints.sensortesttool.tools.Android;

import static android.graphics.Color.argb;

public class ForceView extends View {
    private Paint mPaint;
    private float mForce;
    private float mThreshold;
    private float mGroundstate;
    private Shader mShader;
    private Object sync = new Object();

    public ForceView(final Context context) {
        super(context);
        init();
    }

    public ForceView(final Context context, final AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public ForceView(final Context context, final AttributeSet attrs, final int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    public ForceView(final Context context, final AttributeSet attrs, final int defStyleAttr, final int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        init();
    }

    private void init() {
        mPaint = new Paint();


    }

    @Override
    protected void onMeasure(final int widthMeasureSpec, final int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        mShader = new LinearGradient(0, 0, 0, MeasureSpec.getSize(heightMeasureSpec),
                Color.argb(255, 200, 250, 200), Color.argb(255, 0, 100, 0), Shader.TileMode.CLAMP);
    }

    /**
     * Set force
     *
     * @param force value between 0.0 and 1.0
     */
    public void setForce(final float force) {
        mForce = force;
    }

    public void setThreshold(final float threshold) {
        mThreshold = threshold;
    }

    public void setGroundstate(final float groundstate) {
        mGroundstate = groundstate;
    }

    @Override
    protected void onDraw(final Canvas canvas) {
        super.onDraw(canvas);

        int margin = Android.dpToPx(10, getContext());
        int forceY = Math.round(getHeight() - (getHeight() * mForce));
        int thresholdY = Math.round(getHeight() - (getHeight() * mThreshold));
        int groundstateY = Math.round(getHeight() - (getHeight() * mGroundstate));

        //draw background
        {
            mPaint.setColor(argb(255, 230, 230, 230));
            mPaint.setStyle(Paint.Style.FILL);
            mPaint.setStrokeWidth(Android.pxToDp(8, getContext()));
            canvas.drawRect(margin, 0, getWidth() - margin, getHeight(), mPaint);
        }

        // draw the force
        {
            mPaint.setShader(mShader);
            mPaint.setStyle(Paint.Style.FILL);
            mPaint.setColor(argb(255, 0, 100, 0));
            canvas.drawRect(margin, forceY, getWidth() - margin, getHeight(), mPaint);
            mPaint.setShader(null);

            mPaint.setColor(argb(255, 0, 100, 0));
            mPaint.setStyle(Paint.Style.STROKE);
            mPaint.setStrokeWidth(Android.pxToDp(12, getContext()));
            canvas.drawLine(margin, forceY, getWidth() - margin, forceY, mPaint);

            mPaint.setColor(argb(255, 0, 0, 0));
            mPaint.setTextSize(Android.dpToPx(12, getContext()));

            mPaint.setStrokeWidth(Android.pxToDp(2, getContext()));
            mPaint.setStyle(Paint.Style.FILL);
            String text = String.format("%.3f", mForce);

            canvas.drawText(text, margin + Android.dpToPx(17, getContext()), forceY - Android.dpToPx(2, getContext()), mPaint);
        }

        // draw groundstate
        {
            mPaint.setStyle(Paint.Style.STROKE);
            mPaint.setStrokeWidth(Android.pxToDp(14, getContext()));

            mPaint.setColor(argb(120, 230, 230, 255));
            canvas.drawLine(margin, groundstateY + mPaint.getStrokeWidth(), getWidth() - margin, groundstateY + mPaint.getStrokeWidth(), mPaint);
            canvas.drawLine(margin, groundstateY - mPaint.getStrokeWidth(), getWidth() - margin, groundstateY - mPaint.getStrokeWidth(), mPaint);

            mPaint.setColor(argb(255, 0, 0, 255));
            canvas.drawLine(margin, groundstateY, getWidth() - margin, groundstateY, mPaint);
            mPaint.setPathEffect(null);
        }

        // draw threshold
        if (mThreshold > 0) {
            mPaint.setStyle(Paint.Style.STROKE);
            mPaint.setStrokeWidth(Android.pxToDp(14, getContext()));

            mPaint.setColor(argb(120, 255, 230, 230));
            canvas.drawLine(margin, thresholdY + mPaint.getStrokeWidth(), getWidth() - margin, thresholdY + mPaint.getStrokeWidth(), mPaint);
            canvas.drawLine(margin, thresholdY - mPaint.getStrokeWidth(), getWidth() - margin, thresholdY - mPaint.getStrokeWidth(), mPaint);

            mPaint.setColor(argb(255, 255, 100, 0));
            canvas.drawLine(margin, thresholdY, getWidth() - margin, thresholdY, mPaint);
            mPaint.setPathEffect(null);
        }

        //draw outline
        {
            mPaint.setColor(argb(255, 0, 0, 0));
            mPaint.setStyle(Paint.Style.STROKE);
            mPaint.setStrokeWidth(Android.pxToDp(8, getContext()));
            canvas.drawRect(margin, 0, getWidth() - margin, getHeight(), mPaint);
        }

        //draw markers
        {
            mPaint.setColor(argb(255, 0, 0, 0));
            mPaint.setStyle(Paint.Style.FILL);
            drawMarker(canvas, mPaint, margin / 2, forceY, margin / 2, Color.argb(200, 0, 0, 0));
            drawMarker(canvas, mPaint, getWidth() - margin / 2, forceY, margin / 2, Color.argb(200, 0, 0, 0));

            drawMarker(canvas, mPaint, margin / 2, groundstateY, margin / 2, Color.argb(200, 0, 0, 200));
            drawMarker(canvas, mPaint, getWidth() - margin / 2, groundstateY, margin / 2, Color.argb(200, 0, 0, 200));

            drawMarker(canvas, mPaint, margin / 2, thresholdY, margin / 2, Color.argb(200, 200, 0, 0));
            drawMarker(canvas, mPaint, getWidth() - margin / 2, thresholdY, margin / 2, Color.argb(200, 200, 0, 0));
        }

        postInvalidate();

    }

    private void drawMarker(Canvas canvas, Paint paint, int x, int y, int width, int color) {
        int halfWidth = width / 2;

        mPaint.setColor(color);
        canvas.drawRect(x - halfWidth, y - halfWidth, x + halfWidth, y + halfWidth, mPaint);
    }
}
