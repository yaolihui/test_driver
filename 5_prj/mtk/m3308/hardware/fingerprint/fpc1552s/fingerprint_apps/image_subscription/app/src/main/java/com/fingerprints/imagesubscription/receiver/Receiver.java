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

package com.fingerprints.imagesubscription.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Build;

import com.fingerprints.imagesubscription.services.ImageSubscriptionService;
import com.fingerprints.imagesubscription.utils.Constants;
import com.fingerprints.imagesubscription.utils.Logger;
import com.fingerprints.imagesubscription.utils.Preferences;

import java.lang.reflect.Method;

public class Receiver extends BroadcastReceiver {
    private Logger mLogger = new Logger(getClass().getSimpleName());

    @Override
    public void onReceive(Context context, Intent intent) {
        mLogger.enter("onReceive");
        if (Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction())) {
            Preferences.setEnabled(context, false);
            if (Preferences.isEnabledOnBoot(context)) {
                Intent serviceIntent = new Intent(Constants.ACTION_START);
                serviceIntent.setClass(context, ImageSubscriptionService.class);
                if (Build.VERSION.SDK_INT >= 26) {
                    try {
                        final Method method = Context.class.getMethod("startForegroundService",
                                Intent.class);
                        if (method != null) {
                            method.invoke(context, serviceIntent);
                        }
                    } catch (Exception e) {
                        mLogger.e("Exception: " + e);
                    }
                } else {
                    context.startService(serviceIntent);
                }
            }
        } else {
            mLogger.w("onReceive: could not handle: " + intent);
        }
        mLogger.exit("onReceive");
    }
}
