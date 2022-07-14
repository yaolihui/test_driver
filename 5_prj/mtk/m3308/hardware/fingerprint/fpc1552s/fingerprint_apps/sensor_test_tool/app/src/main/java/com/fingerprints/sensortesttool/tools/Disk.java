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
package com.fingerprints.sensortesttool.tools;

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
    public static boolean externalFileExist(final String path, final Context context) {
        String fpath = context.getExternalCacheDir().getPath() + "/" + path;
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
    public static String readExternalTextFile(final String path, final Context context) throws FileNotFoundException, IOException {
        StringBuffer buffer = new StringBuffer();
        BufferedReader br = null;

        String fpath = context.getExternalCacheDir().getPath() + "/" + path;
        br = new BufferedReader(new FileReader(fpath));
        String line = "";
        while ((line = br.readLine()) != null) {
            buffer.append(line);
        }

        br.close();

        return buffer.toString();
    }

    /**
     * Will write a file to the application storage space.
     */
    public static boolean write(final String fileName, final String text, final Context context) {
        String fpath = context.getExternalCacheDir().getPath() + "/" + fileName;

        try {
            File file = new File(fpath);

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
