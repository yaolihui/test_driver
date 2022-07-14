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

import java.util.ArrayList;
import java.util.Stack;

public class TaskLog {
    private LogPoint mRoot;
    private Stack<LogPoint> mCurrent;

    private class Tuple {
        private String mName;
        private Object mValue;

        public Tuple(final String name, final Object value) {
            mName = name;
            mValue = value;
        }

        public String getName() {
            return mName;
        }

        public Object getValue() {
            return mValue;
        }

        public String toString() {
            return mName + "=" + mValue;
        }
    }

    public abstract class TaskLogRunnable<T> {
        public T run(LogPoint logPoint) throws Exception {
            return performTask(logPoint);
        }

        public abstract T performTask(final LogPoint logPoint) throws Exception;
    }

    public class LogPoint {
        private String mName;
        private ArrayList<Tuple> mInfo;
        private ArrayList<LogPoint> mLogPoints;

        public LogPoint(final String name) {
            mName = name;
            mInfo = new ArrayList<>();
            mLogPoints = new ArrayList<>();
        }

        public String getName() {
            return mName;
        }

        public ArrayList<Tuple> getInfo() {
            return mInfo;
        }

        public LogPoint log(final String name, final Object value) {
            mInfo.add(new Tuple(name, value));
            return this;
        }

        public boolean hasInfo() {
            return mInfo != null && mInfo.size() > 0;
        }

        public ArrayList<LogPoint> getLogPoints() {
            if (mLogPoints == null) {
                mLogPoints = new ArrayList<>();
            }
            return mLogPoints;
        }

        public LogPoint addLogPoint(final LogPoint logPoint) {
            getLogPoints().add(logPoint);
            return logPoint;
        }

        public String toString() {
            return toStringRec("");
        }

        protected String toStringRec(final String add) {
            String s = "(" + mName + ")";
            if (hasInfo()) {
                for (Tuple t : mInfo) {
                    s += " " + t.toString();
                }
            }
            s += "\n";

            for (LogPoint logPoint : mLogPoints) {
                s += logPoint.toStringRec(add + "  ");
            }

            return s;
        }

        public boolean hasLogPoints() {
            return mLogPoints != null && mLogPoints.size() > 0;
        }

        public LogPoint logTime() {
            log("time", System.currentTimeMillis());
            return this;
        }
    }

    public TaskLog() {
        reset();
    }

    public void reset() {
        mRoot = new LogPoint("root");
        mCurrent = new Stack<>();
        mCurrent.push(mRoot);
    }

    public LogPoint open(final String name) {
        LogPoint log = mCurrent.peek().addLogPoint(new LogPoint(name));
        mCurrent.push(log);
        return log;
    }

    public <T> T openAndClose(final Class<T> returnType, final String name, final TaskLogRunnable<T> runnable) throws Exception {

        LogPoint log = mCurrent.peek().addLogPoint(new LogPoint(name));
        mCurrent.push(log);

        try {
            return runnable.run(log);
        } catch (Exception e) {
            throw e;
        } finally {
            close();
        }
    }

    public LogPoint add(final String name) {
        LogPoint log = mCurrent.peek().addLogPoint(new LogPoint(name));
        return log;
    }

    public LogPoint log(final String name, final Object value) {
        LogPoint current = mCurrent.peek();
        current.log(name, value);
        return current;
    }

    public void close() {
        mCurrent.pop();
    }

    public void close(final String name) {
        while (!mCurrent.peek().getName().equals(name) && mCurrent.size() > 0) {
            mCurrent.pop();
        }
        if (mCurrent.peek().getName().equals(name)) {
            mCurrent.pop();
        }
    }

    @Override
    public String toString() {
        return mRoot.toString();
    }

    public String toXml() {
        StringBuffer buffer = new StringBuffer();

        for (LogPoint p : mRoot.getLogPoints()) {
            buffer.append(toXml(p, ""));
        }

        return buffer.toString();
    }

    private String toXml(final LogPoint point, final String indent) {
        String s = indent + "<" + point.getName() + "";
        for (Tuple t : point.getInfo()) {
            if (t.getValue() != null) {
                s += " " + t.getName() + "=\"" + t.getValue().toString() + "\"";
            }
        }
        if (point.hasLogPoints()) {
            s += ">\n";
            for (LogPoint p : point.getLogPoints()) {
                s += toXml(p, indent + "  ");
            }

            s += indent + "</" + point.getName() + ">\n";
        } else {
            s += "/>\n";
        }

        return s;
    }
}
