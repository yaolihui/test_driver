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

package com.fingerprints.imagesubscription.utils;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Build;

import com.fingerprints.imagesubscription.R;
import com.fingerprints.imagesubscription.activites.MainActivity;
import com.fingerprints.imagesubscription.services.ImageSubscriptionService;

import java.lang.reflect.Constructor;
import java.lang.reflect.Method;

public class NotificationUtils {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private static final int NOTIFICATION_ID = 101;
    private static final String CHANNEL_ID = "1";
    private static final String CHANNEL_NAME = "default";
    private final Context mContext;
    private final NotificationManager mNotificationManager;

    public NotificationUtils(Context context) {
        mContext = context;
        mNotificationManager = (NotificationManager) context.
                getSystemService(Context.NOTIFICATION_SERVICE);

        if (Build.VERSION.SDK_INT >= 26) {
            try {
                Class c = Class.forName("android.app.NotificationChannel");
                Constructor<?> ctor = c.getDeclaredConstructor(String.class, CharSequence.class,
                        int.class);
                Object notificationChannel = ctor.newInstance(CHANNEL_ID, CHANNEL_NAME, 3);
                Method method = mNotificationManager.getClass().getDeclaredMethod(
                        "createNotificationChannel", c);
                method.invoke(mNotificationManager, notificationChannel);
            } catch (Exception e) {
                mLogger.e("Exception: " + e);
            }
        }
    }

    public void updateNotification(Service service, boolean isSubscription) {
        mLogger.enter("updateNotification");
        if (Build.VERSION.SDK_INT >= 26) {
            try {
                if (isSubscription) {
                    Method method = Service.class.getMethod("startForeground",
                            new Class[]{int.class, Notification.class});
                    method.invoke(service, NOTIFICATION_ID, getNotification(isSubscription));
                } else {
                    Method method = Service.class.getMethod("stopForeground",
                            new Class[]{boolean.class});
                    method.invoke(service, true);
                }
            } catch (Exception e) {
                mLogger.e("Exception: " + e);
            }
        } else {
            mNotificationManager.notify(NOTIFICATION_ID, getNotification(isSubscription));
        }
        mLogger.exit("updateNotification");
    }

    public Notification getNotification(boolean isSubscription) {
        mLogger.enter("getNotification");
        mLogger.d("subscription: " + isSubscription);
        Notification.Builder b = new Notification.Builder(mContext);

        b.setContentTitle(mContext.getString(R.string.app_label))
                .setContentText(mContext.getString(R.string.app_description))
                .setSmallIcon(R.drawable.fpc_logo)
                .setContentIntent(PendingIntent.getActivity(mContext, 0,
                        new Intent(mContext, MainActivity.class), 0))
                .setOngoing(isSubscription);

        if (Build.VERSION.SDK_INT >= 26) {
            try {
                Method method = b.getClass().getDeclaredMethod("setChannelId", String.class);
                method.invoke(b, CHANNEL_ID);
            } catch (Exception e) {
                mLogger.e("Exception: " + e);
            }
        }

        if (isSubscription) {
            Intent intent = new Intent(mContext, ImageSubscriptionService.class);
            intent.setAction(Constants.ACTION_STOP);
            PendingIntent pi = PendingIntent.getService(mContext, 0, intent, 0);
            b.addAction(R.drawable.ic_disable_black,
                    mContext.getString(R.string.disable_notification_title), pi);
        } else {
            Intent intent = new Intent(mContext, ImageSubscriptionService.class);
            intent.setAction(Constants.ACTION_START);
            PendingIntent pi = PendingIntent.getService(mContext, 0, intent, 0);
            b.addAction(R.drawable.ic_enable_black,
                    mContext.getString(R.string.enable_notification_title), pi);
        }

        mLogger.exit("getNotification");
        return b.build();
    }
}
