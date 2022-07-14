package com.fingerprints.navigationtest.helpers;

import com.fingerprints.navigationtest.TestEvent;

import java.text.DecimalFormat;
import java.text.NumberFormat;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedList;

public class TestData {
    private HashMap<NavigationInput, HashMap<NavigationInput, Counter>> mResults;
    private HashMap<NavigationInput, Counter> mFailedToRegister;
    private int mTotalNumSuccessfulInputs;
    private int mTotalNumInputs;
    private HashMap<NavigationInput, Counter> mEventsUnderTestMap;
    private LinkedList<TestEvent> mTestSequence;
    private NumberFormat mFormatter;
    private int mSecondsElapsed;
    private long mStartTime;
    private long mTotalTime;

    public TestData() {
        mResults = new HashMap<>();
        for (NavigationInput expected : NavigationInput.values()) {
            HashMap<NavigationInput, Counter> actualResults = new HashMap<>();
            for (NavigationInput actual : NavigationInput.values()) {
                actualResults.put(actual, new Counter());
            }
            mResults.put(expected, actualResults);
        }

        mFailedToRegister = new HashMap<>();
        for (NavigationInput n : NavigationInput.values()) {
            mFailedToRegister.put(n, new Counter());
        }

        mEventsUnderTestMap = new HashMap<>();
        for (NavigationInput n : NavigationInput.values()) {
            mEventsUnderTestMap.put(n, new Counter());
        }

        mTestSequence = new LinkedList<>();

        mFormatter = new DecimalFormat("#0.00");
        mSecondsElapsed = 0;
        mStartTime = 0;
        mTotalTime = 0;

        mTotalNumSuccessfulInputs = 0;
        mTotalNumInputs = 0;
    }

    public void addFailedToRegisterResult(NavigationInput expectedResult) {
        mFailedToRegister.get(expectedResult).inc();
        mTotalNumInputs++;
    }

    public void addResult(NavigationInput expectedResult, NavigationInput actualResult) {
        mResults.get(expectedResult).get(actualResult).inc();
        mTotalNumInputs++;
        if (expectedResult == actualResult) {
            mTotalNumSuccessfulInputs++;
        }
    }

    public void resetTime() {
        mSecondsElapsed = 0;
        mStartTime = 0;
        mTotalTime = 0;
    }

    public void resetResults() {
        for (HashMap<NavigationInput, Counter> hm : mResults.values()) {
            for (Counter c : hm.values()) {
                c.reset();
            }
        }

        for (Counter c : mFailedToRegister.values()) {
            c.reset();
        }

        mTotalNumSuccessfulInputs = 0;
        mTotalNumInputs = 0;
    }

    public void reset() {
        resetTime();
        mTestSequence.clear();

        for (HashMap<NavigationInput, Counter> hm : mResults.values()) {
            for (Counter c : hm.values()) {
                c.reset();
            }
        }

        for (Counter c : mFailedToRegister.values()) {
            c.reset();
        }

        for (NavigationInput n : NavigationInput.values()) {
            mEventsUnderTestMap.get(n).reset();
        }

        mTotalNumSuccessfulInputs = 0;
        mTotalNumInputs = 0;
    }

    public void addNumEventsToTest(final NavigationInput n, final int eventCount) {
        mEventsUnderTestMap.get(n).inc(eventCount);
    }

    public void setNumEventsInTest(final NavigationInput n, final int eventCount) {
        mEventsUnderTestMap.get(n).set(eventCount);
    }

    public int getNumEventsInTest(final NavigationInput n) {
        return mEventsUnderTestMap.get(n).get();
    }

    public int getNumTestInputs(final NavigationInput n) {
        HashMap<NavigationInput, Counter> hm = mResults.get(n);
        int sum = 0;
        for (Counter c : hm.values()) {
            sum += c.get();
        }
        sum += mFailedToRegister.get(n).get();
        return sum;
    }

    public int getInputs(final NavigationInput expected, final NavigationInput n) {
        return mResults.get(expected).get(n).get();
    }

    private int getNumSuccessfulInputs(final NavigationInput n) {
        return mResults.get(n).get(n).get();
    }

    public int getNumSuccessfulInputsAsPercent(final NavigationInput n) {
        int numTestInputs = getNumTestInputs(n);
        if (numTestInputs > 0) {
            return Math.round((((float) getNumSuccessfulInputs(n) / numTestInputs) * 100f));
        } else {
            return 0;
        }
    }

    public String getNumSuccessfulInputsAsString(NavigationInput n) {
        return getNumSuccessfulInputs(n) + "/" + getNumTestInputs(n);
    }

    public void addAllEventsToTest(final int eventCount) {
        for (NavigationInput n : NavigationInput.values()) {
            mEventsUnderTestMap.get(n).inc(eventCount);
        }
    }

    public int getTotalNumEventsInTest() {
        int numEvents = 0;
        for (NavigationInput n : NavigationInput.values()) {
            numEvents += getNumEventsInTest(n);
        }
        return numEvents;
    }

    public int getTotalNumTestInputs() {
        return mTotalNumInputs;
    }

    public int getTotalNumSuccessfulInputs() {
        return mTotalNumSuccessfulInputs;
    }

    public String getTotalNumSuccessfulInputsAsString() {
        return mTotalNumSuccessfulInputs + "/" + mTotalNumInputs;
    }

    public int getTotalNumSuccessfulInputsAsPercent() {
        int numTestInputs = getTotalNumTestInputs();

        if (numTestInputs > 0) {
            float p = (float) getTotalNumSuccessfulInputs() / numTestInputs;
            return Math.round(p * 100);
        } else {
            return 0;
        }
    }

    public int getTotalNumFailedInputs() {
        return mTotalNumInputs - mTotalNumSuccessfulInputs;
    }

    public String getTotalNumFailedInputsAsString() {
        return getTotalNumFailedInputs() + "/" + mTotalNumInputs;
    }

    public int getNumFailedToRegister(NavigationInput n) {
        return mFailedToRegister.get(n).get();
    }

    public int getSecondsElapsed() {
        return mSecondsElapsed;
    }

    public void incSecondsElapsed() {
        mSecondsElapsed++;
    }

    public long getStartTime() {
        return mStartTime;
    }

    public long getTotalTime() {
        return mTotalTime;
    }

    public void testStart() {
        mStartTime = System.currentTimeMillis();
    }

    public void testEnd() {
        mTotalTime = System.currentTimeMillis() - mStartTime;
    }

    public void createTestSequence(final boolean random) {
        mTestSequence.clear();
        for (NavigationInput n : NavigationInput.values()) {
            for (int i = 0; i < getNumEventsInTest(n); i++) {
                mTestSequence.add(new TestEvent(n));
            }
        }
        if (random) {
            Collections.shuffle(mTestSequence);
        }
    }

    public LinkedList<TestEvent> getTestSequence() {
        return mTestSequence;
    }

    public boolean isComplete() {
        return mTestSequence.size() == 0;
    }

    public int getTestsRemaining() {
        return mTestSequence.size();
    }

    public Object getTotalTimeAsString() {
        return mFormatter.format(getTotalTime() / 1000f) + "s";
    }
}
