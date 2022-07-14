
package com.silead.manager;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * @hide
 */
public final class FingerFrrFarImageResult implements Parcelable {
    private static final int IMG_TYPE_AUTH = 0;
    private static final int IMG_TYPE_TPL = 1;
    private static final int IMG_TYPE_CAL = 2;

    private int mErrCode;
    private int mType;
    private int mResult;
    private int mIndex;
    private int mTime;
    private int mTime2;
    private int mFingerId;
    private int mTplCount;

    public FingerFrrFarImageResult(int err, int type, int result, int index, int time1, int time2, int fid, int tpl_num) {
        mErrCode = err;
        mType = type;
        mResult = result;
        mIndex = index;
        mTime = time1;
        mTime2 = time2;
        mFingerId = fid;
        mTplCount = tpl_num;
    }

    private FingerFrrFarImageResult(Parcel in) {
        mErrCode = in.readInt();
        mType = in.readInt();
        mResult = in.readInt();
        mIndex = in.readInt();
        mTime = in.readInt();
        mTime2 = in.readInt();
        mFingerId = in.readInt();
        mTplCount = in.readInt();
    }

    public int getErrorCode() {
        return mErrCode;
    }

    public boolean isTpl() {
        return (mType == IMG_TYPE_TPL);
    }

    public boolean isCalibrate() {
        return (mType == IMG_TYPE_CAL);
    }

    public boolean isAuth() {
        return (mType == IMG_TYPE_AUTH);
    }

    public int getResult() {
        return mResult;
    }

    public int getIndex() {
        return mIndex;
    }

    public int getTime() {
        return mTime;
    }

    public int getTime2() {
        return mTime2;
    }

    public int getFingerId() {
        return mFingerId;
    }

    public int getTplCount() {
        return mTplCount;
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(mErrCode);
        out.writeInt(mType);
        out.writeInt(mResult);
        out.writeInt(mIndex);
        out.writeInt(mTime);
        out.writeInt(mTime2);
        out.writeInt(mFingerId);
        out.writeInt(mTplCount);
    }

    public static final Parcelable.Creator<FingerFrrFarImageResult> CREATOR = new Parcelable.Creator<FingerFrrFarImageResult>() {
        public FingerFrrFarImageResult createFromParcel(Parcel in) {
            return new FingerFrrFarImageResult(in);
        }

        public FingerFrrFarImageResult[] newArray(int size) {
            return new FingerFrrFarImageResult[size];
        }
    };

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("error:").append(mErrCode).append(" ");
        sb.append("type:").append(mType).append(" ");
        sb.append("result:").append(mResult).append(" ");
        sb.append("index:").append(mIndex).append(" ");
        sb.append("time:").append(mTime).append(" ");
        sb.append("time2:").append(mTime2);
        sb.append("fid:").append(Integer.toHexString(mFingerId));
        sb.append("tpl_count:").append(mTplCount);
        return sb.toString();
    }

    public static FingerFrrFarImageResult parse(byte[] result) {
        int err = FingerManager.TEST_RESULT_DATA_IMCOMPLITE;
        int rlt = 0;
        int offset = 0;
        int type = 0;
        int index = 0;
        int time1 = 0;
        int time2 = 0;
        int fid = -1;
        int tpl_count = -1;

        if (result != null && result.length >= 4) {
            err = (0xFF & result[offset++]) << 24;
            err |= (0xFF & result[offset++]) << 16;
            err |= (0xFF & result[offset++]) << 8;
            err |= (0xFF & result[offset++]);

            if (err == FingerManager.TEST_RESULT_OK) {
                if (result.length >= offset + 6) {
                    index = (0xFF & result[offset++]) << 24;
                    index |= (0xFF & result[offset++]) << 16;
                    index |= (0xFF & result[offset++]) << 8;
                    index |= (0xFF & result[offset++]);
                    type = (0xFF & result[offset++]);
                    rlt = (0xFF & result[offset++]);
                    if (result.length >= offset + 4) {
                        time1 = (0xFF & result[offset++]) << 24;
                        time1 |= (0xFF & result[offset++]) << 16;
                        time1 |= (0xFF & result[offset++]) << 8;
                        time1 |= (0xFF & result[offset++]);
                    }
                    if (result.length >= offset + 4) {
                        time2 = (0xFF & result[offset++]) << 24;
                        time2 |= (0xFF & result[offset++]) << 16;
                        time2 |= (0xFF & result[offset++]) << 8;
                        time2 |= (0xFF & result[offset++]);
                    }
                    if (result.length >= offset + 4) {
                        fid = (0xFF & result[offset++]) << 24;
                        fid |= (0xFF & result[offset++]) << 16;
                        fid |= (0xFF & result[offset++]) << 8;
                        fid |= (0xFF & result[offset++]);
                    }
                    if (result.length >= offset + 4) {
                        tpl_count = (0xFF & result[offset++]) << 24;
                        tpl_count |= (0xFF & result[offset++]) << 16;
                        tpl_count |= (0xFF & result[offset++]) << 8;
                        tpl_count |= (0xFF & result[offset++]);
                    }
                } else {
                    err = FingerManager.TEST_RESULT_DATA_IMCOMPLITE;
                }
            }
        }

        return new FingerFrrFarImageResult(err, type, rlt, index, time1, time2, fid, tpl_count);
    }
};
