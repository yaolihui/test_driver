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

import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Looper;
import android.app.AlertDialog;

import com.fingerprints.imagecollection.R;

public class AndroidUI {
    public static void showDialogAndWait(final int titleId, final int messageId, final Activity activity) {
        showDialogAndWait(activity.getString(titleId), activity.getString(messageId), activity.getString(R.string.ok), activity);
    }

    public static void showDialogAndWait(final int titleId, final String message, final Activity activity) {
        showDialogAndWait(activity.getString(titleId), message, activity.getString(R.string.ok), activity);
    }

    public static void showDialogAndWait(final String title, final String message, final String buttonText, final Activity activity) {

        boolean isOnUiThread = Thread.currentThread() == Looper.getMainLooper().getThread();

        if (isOnUiThread) {
            new Thread(new Runnable() {
                @Override
                public void run() {
                    showDialogAndWait(title, message, buttonText, activity);
                }
            }).start();
            return;
        }

        final Object waitSync = new Object();

        synchronized (waitSync) {
            activity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    AlertDialog.Builder builder = new AlertDialog.Builder(activity);
                    builder.setTitle(title);
                    builder.setMessage(message);
                    builder.setPositiveButton(buttonText,
                            new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    synchronized (waitSync) {
                                        waitSync.notifyAll();
                                    }
                                }
                            });
                    builder.setCancelable(false);
                    builder.show();
                }
            });

            try {
                //wait until user pressed ok.
                waitSync.wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    public static boolean showDialogAskAndWait(final String title, final String message, final String okButtonText, final String cancelButtonText, final Activity activity) {

        boolean isOnUiThread = Thread.currentThread() == Looper.getMainLooper().getThread();

        if (isOnUiThread) {
            new Thread(new Runnable() {
                @Override
                public void run() {
                    showDialogAskAndWait(title, message, okButtonText, cancelButtonText, activity);
                }
            }).start();
            return false;
        }

        final Object waitSync = new Object();
        final Holder<Boolean> userAnswer = new Holder<>();

        synchronized (waitSync) {
            activity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    AlertDialog.Builder builder = new AlertDialog.Builder(activity);
                    builder.setTitle(title);
                    builder.setMessage(message);
                    builder.setPositiveButton(okButtonText,
                            new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    synchronized (waitSync) {
                                        userAnswer.set(true);
                                        waitSync.notifyAll();
                                    }
                                }
                            });
                    builder.setNegativeButton(cancelButtonText, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(final DialogInterface dialog, final int which) {
                            synchronized (waitSync) {
                                userAnswer.set(false);
                                waitSync.notifyAll();
                            }
                        }
                    });
                    builder.setCancelable(false);
                    builder.show();
                }
            });

            try {
                //wait until user pressed ok.
                waitSync.wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        return userAnswer.get();
    }

    public static int pxToDp(final Context context, final int px) {
        return Math.round(px / (float) context.getResources().getDisplayMetrics().density);
    }

    public static int dpToPx(final Context context, final int dp) {
        return Math.round((float) dp * context.getResources().getDisplayMetrics().density);
    }
}
