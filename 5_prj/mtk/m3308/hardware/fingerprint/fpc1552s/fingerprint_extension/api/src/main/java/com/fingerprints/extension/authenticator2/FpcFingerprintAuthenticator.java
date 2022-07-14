/**
 * Copyright (c) 2021 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.authenticator2;

import android.os.RemoteException;

import java.util.ArrayList;

import com.fingerprints.extension.V1_0.IFpcFingerprintAuthenticator;
import com.fingerprints.extension.V1_0.IAuthenticatorCallback;
import com.fingerprints.extension.util.Logger;
import com.fingerprints.extension.util.ArrayUtils;

public class FpcFingerprintAuthenticator {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private final static String LOG_TAG = FpcFingerprintAuthenticator.class.getSimpleName();

    private IFpcFingerprintAuthenticator mService;
    private IAuthenticatorServiceReceiver mReceiver;

    public interface IAuthenticatorServiceReceiver {
        void onEnrollResult(int fingerId, int remaining);
        void onAuthenticated(int fingerId);
        void onEnumerate(int fingerId, int remaining);
        void onRemoved(int fingerId, int remaining);
        void onAcquired(int status);
        void onError(int errorCode);
    }

    private final IAuthenticatorCallback mCallback = new IAuthenticatorCallback.Stub() {
        @Override
        public void onEnrollResult(int fingerId, int remaining) {
            mLogger.d(LOG_TAG + "onEnrollResult");
            mReceiver.onEnrollResult(fingerId, remaining);
        }

        @Override
        public void onAuthenticated(int fingerId) {
            mLogger.d(LOG_TAG + "onAuthenticated");
            mReceiver.onAuthenticated(fingerId);
        }

        @Override
        public void onEnumerate(int fingerId, int remaining) {
            mLogger.d(LOG_TAG + "onEnumerate");
            mReceiver.onEnumerate(fingerId, remaining);
        }

        @Override
        public void onRemoved(int fingerId, int remaining) {
            mLogger.d(LOG_TAG + "onRemoved");
            mReceiver.onRemoved(fingerId, remaining);
        }

        @Override
        public void onAcquired(int status) {
            mLogger.d(LOG_TAG + "onAcquired");
            mReceiver.onAcquired(status);
        }

        @Override
        public void onError(int errorCode) {
            mLogger.d(LOG_TAG + "onError");
            mReceiver.onError(errorCode);
        }
    };

    public FpcFingerprintAuthenticator() throws RemoteException {
        mLogger.d(LOG_TAG + "FpcFingerprintAuthenticator");

        mService = IFpcFingerprintAuthenticator.getService();
        if (mService == null) {
            throw new RemoteException("Failed to get FpcFingerprintAuthenticator service");
        } else {
            try {
                mService.setNotify(mCallback);
            } catch (RemoteException e) {
                mLogger.e(LOG_TAG + "RemoteException: ", e);
            }
        }

        mLogger.d(LOG_TAG + "FpcFingerprintAuthenticator");
    }

    public void enroll(IAuthenticatorServiceReceiver receiver) {
        mLogger.d(LOG_TAG + "enroll");

        mReceiver = receiver;

        if (mService != null) {
            try {
                mService.enroll();
            } catch (RemoteException e) {
                mLogger.e(LOG_TAG + "RemoteException: ", e);
            }
        }

        mLogger.d(LOG_TAG + "enroll");
    }

    public void authenticate(IAuthenticatorServiceReceiver receiver) {
        mLogger.d(LOG_TAG + "authenticate");

        mReceiver = receiver;

        if (mService != null) {
            try {
                mService.authenticate();
            } catch (RemoteException e) {
                mLogger.e(LOG_TAG + "RemoteException: ", e);
            }
        }

        mLogger.d(LOG_TAG + "authenticate");
    }

    public void enumerate(IAuthenticatorServiceReceiver receiver) {
        mLogger.d(LOG_TAG + "enumerate");

        mReceiver = receiver;

        if (mService != null) {
            try {
                mService.enumerate();
            } catch (RemoteException e) {
                mLogger.e(LOG_TAG + "RemoteException: ", e);
            }
        }

        mLogger.d(LOG_TAG + "enumerate");
    }

    public void remove(int id, IAuthenticatorServiceReceiver receiver) {
        mLogger.d(LOG_TAG + "remove");

        mReceiver = receiver;

        if (mService != null) {
            try {
                mService.remove(id);
            } catch (RemoteException e) {
                mLogger.e(LOG_TAG + "RemoteException: ", e);
            }
        }

        mLogger.d(LOG_TAG + "remove");
    }

    public void cancel() {
        mLogger.d(LOG_TAG + "cancel");

        if (mService != null) {
            try {
                mService.cancel();
            } catch (RemoteException e) {
                mLogger.e(LOG_TAG + "RemoteException: ", e);
            }
        }

        mLogger.d(LOG_TAG + "cancel");
    }
}
