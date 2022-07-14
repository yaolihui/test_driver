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
package com.fingerprints.imagesubscription.utils;

import android.content.Context;
import android.os.Environment;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;

public class Disk {

    /**
     * Checks is a file exist on external storage.
     * Requires permission android.permission.WRITE_EXTERNAL_STORAGE
     *
     * @param path
     * @return true if the file exist, false otherwise
     */
    public static boolean externalFileExist(final String path) {
        String fpath = Environment.getExternalStorageDirectory() + "/" + path;
        return new File(fpath).exists();
    }

    /**
     * Reads a file from external storage.
     * Requires permission android.permission.WRITE_EXTERNAL_STORAGE
     *
     * @param path
     * @return the file as a String
     * @throws FileNotFoundException
     * @throws IOException
     */
    public static String readExternalTextFile(final String path) throws FileNotFoundException, IOException {
        StringBuffer buffer = new StringBuffer();
        BufferedReader br = null;

        String fpath = Environment.getExternalStorageDirectory() + "/" + path;
        br = new BufferedReader(new FileReader(fpath));
        String line = "";
        while ((line = br.readLine()) != null) {
            buffer.append(line);
        }

        return buffer.toString();
    }

    /**
     * Will write a file to an external path.
     */
    public static boolean writeExternalTextFile(final String path, final String text, final Context context) {
        File file = new File(path);
        FileOutputStream fos = null;
        try {
            fos = new FileOutputStream(file);
            fos.write(text.getBytes());
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        } finally {
            try {
                if (fos != null) {
                    fos.flush();
                    fos.close();
                }
            } catch (IOException ignore) {
                return false;
            }
        }
        return true;
    }

    /**
     * Will write a file to the application storage space.
     */
    public static boolean write(final String fileName, final String text, final Context context) {
        try {
            File file = new File((context.getFileStreamPath(fileName)
                    .getPath()));

            FileOutputStream fos;
            byte[] data = text.getBytes();

            fos = new FileOutputStream(file);
            fos.write(data);
            fos.flush();
            fos.close();
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }

        return true;
    }
}