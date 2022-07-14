/*
 *
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */

package com.fingerprints.imagesubscription.services;

import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;

import com.fingerprints.extension.engineering.FingerprintEngineering;
import com.fingerprints.extension.engineering.FingerprintEngineering.ImageSubscriptionCallback;
import com.fingerprints.imagesubscription.utils.Constants;
import com.fingerprints.imagesubscription.utils.ImageWriter;
import com.fingerprints.imagesubscription.utils.Logger;
import com.fingerprints.imagesubscription.utils.NotificationUtils;
import com.fingerprints.imagesubscription.utils.Preferences;

public class ImageSubscriptionService extends Service implements ImageSubscriptionCallback {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private volatile Looper mServiceLooper;
    private volatile ServiceHandler mServiceHandler;
    private Handler mHandler;
    private FingerprintEngineering mFingerprintEngineering = null;
    private NotificationUtils mNotificationUtils = null;
    private ImageWriter mImageWriter = null;

    private final class ServiceHandler extends Handler {
        public ServiceHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            onHandleIntent((Intent) msg.obj, msg.arg1);
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mLogger.enter("onCreate");
        mHandler = new Handler();
        HandlerThread thread = new HandlerThread(getClass().getSimpleName());
        thread.start();

        mServiceLooper = thread.getLooper();
        mServiceHandler = new ServiceHandler(mServiceLooper);
        try {
            mFingerprintEngineering = new FingerprintEngineering();
        } catch (Exception e) {
            mLogger.e("Exception: " + e);
        }
        mNotificationUtils = new NotificationUtils(this);
        mLogger.exit("onCreate");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        mLogger.enter("onStartCommand");
        Message msg = mServiceHandler.obtainMessage();
        msg.arg1 = startId;
        msg.obj = intent;
        mServiceHandler.sendMessage(msg);
        mLogger.exit("onStartCommand");
        return START_STICKY_COMPATIBILITY;
    }

    protected void onHandleIntent(Intent intent, final int startId) {
        mLogger.enter("onHandleIntent");
        if (intent != null) {
            String action = intent.getAction();
            if (action != null) {
                mLogger.i("action: " + action);
                if (Constants.ACTION_START.equals(action)) {
                    if (mFingerprintEngineering != null) {
                        mLogger.i("Image Subscription started...");
                        String folder = intent.getStringExtra("setFolder");
                        if (folder != null) {
                            Preferences.setFolder(this, folder);
                        }
                        Preferences.setEnabled(this, true);
                        mFingerprintEngineering.startImageSubscription(this);
                        mImageWriter = new ImageWriter(this);
                        mNotificationUtils.updateNotification(this, Preferences.isEnabled(this));
                    }
                } else if (Constants.ACTION_STOP.equals(action)) {
                    if (mFingerprintEngineering != null) {
                        mImageWriter = null;
                        mFingerprintEngineering.stopImageSubscription();
                        Preferences.setEnabled(this, false);
                        mLogger.i("Image Subscription stopped...");
                        mNotificationUtils.updateNotification(this, Preferences.isEnabled(this));
                    }
                } else if (Constants.ACTION_START_ON_BOOT.equals(action)) {
                    Preferences.setEnabledOnBoot(this,
                            intent.getBooleanExtra("startOnBoot", false));
                } else if (Constants.ACTION_SAVE_PNG.equals(action)) {
                    Preferences.setSavePng(this,
                            intent.getBooleanExtra("savePng", false));
                } else {
                    mLogger.w("not supported action...");
                }

                sendBroadcast(new Intent(Constants.ACTION_ACTIVITY_REFRESH));

                if (!(Preferences.isEnabled(this))) {
                    mNotificationUtils.updateNotification(this, true);
                    mNotificationUtils.updateNotification(this, false);

                    mHandler.post(new Runnable() {
                        public void run() {
                            stopSelf(startId);
                        }
                    });
                }
            } else {
                mLogger.i("Image Subscription started services...");
                Preferences.setEnabled(this, true);
                mNotificationUtils.updateNotification(this, Preferences.isEnabled(this));
            }
        }
        mLogger.exit("onHandleIntent");
    }

    @Override
    public void onImage(final FingerprintEngineering.ImageData captureData) {
        mLogger.enter("onImage");
        mImageWriter.writeImage(captureData);
        mLogger.exit("onImage");
    }

    @Override
    public void onDestroy() {
        mLogger.enter("onDestroy");
        mServiceLooper.quit();
        mLogger.exit("onDestroy");
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}
