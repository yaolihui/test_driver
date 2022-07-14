/*
 *
 * Copyright (c) 2018 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */

package com.fingerprints.navigationtest.helpers;

import android.content.Context;
import android.util.Log;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;

public class NavigationTestResultTable {

    private static String getShortName(NavigationInput n) {
        switch (n) {
            case UP:
                return "U";
            case RIGHT:
                return "R";
            case DOWN:
                return "D";
            case LEFT:
                return "L";
            case CLICK:
                return "C";
            case HOLD_CLICK:
                return "H";
            case DOUBLE_CLICK:
                return "DC";
            //case SOFT_PRESS:
            //    return "S";
            case HARD_PRESS:
                return "F";
        }

        throw new RuntimeException("Missing case for NavigationInput " + n.toString());
    }

    private static String getFieldSpace(int i) {
        String s;

        if (i < 10) {
            s = " " + i + " ";
        } else if (i < 100) {
            s = i + " ";
        } else {
            s = i + "";
        }

        return s;
    }

    private static String getResultRow(NavigationInput n, TestData data) {
        String row = getShortName(n);
        if (row.length() == 1) {
            row += " ";
        }

        for (NavigationInput v : NavigationInput.values()) {
            row += "|" + getFieldSpace(data.getInputs(n, v));

            if (getShortName(v).length() == 2) {
                row += " ";
            }
        }

        row += "|" + getFieldSpace(data.getNumFailedToRegister(n));

        return row + "|";
    }

    private static String getTableHeader() {
        String header = "  |";
        for (NavigationInput n : NavigationInput.values()) {
            header += " " + getShortName(n) + " |";
        }
        return header + " N |\n";
    }

    public static void writeToFile(Context context, String filename, TestData data) {
        long timeElapsed = data.getTotalTime();

        try {
            FileOutputStream fOut = context.openFileOutput(filename, Context.MODE_APPEND);
            OutputStreamWriter out = new OutputStreamWriter(fOut);

            out.write("---------------------------------------------" + "\n");
            out.write("[Number of successful test inputs: " + data.getTotalNumSuccessfulInputsAsString() + " (" + data.getTotalNumSuccessfulInputsAsPercent() + "%)]\n");
            out.write("[Number of failed test inputs: " + data.getTotalNumFailedInputsAsString() + "]\n");
            out.write("[Total time elapsed: " + (double) timeElapsed / 1000 + "s]\n");
            out.write("[Average time per test: " + (data.getTotalNumTestInputs() > 0 ? (double) (timeElapsed / data.getTotalNumTestInputs()) / 1000 + "s" : "N/A") + "]\n");
            out.write("Detailed test results:\n");

            out.write(getTableHeader());
            for (NavigationInput n : NavigationInput.values()) {
                out.write(getResultRow(n, data) + "\n");
            }

            out.flush();
            out.close();
        } catch (IOException e) {
            Log.e("IO error", e.toString());
        }
    }

}
