package com.silead.manager;

public class ArrayUtils {
    public static void arraycopy(byte[] src, int srcPos, byte[] dst, int dstPos, int length) {
        if (src == null) {
            throw new NullPointerException("src == null");
        }
        if (dst == null) {
            throw new NullPointerException("dst == null");
        }
        if (srcPos < 0 || dstPos < 0 || length < 0 || srcPos > src.length - length || dstPos > dst.length - length) {
            throw new ArrayIndexOutOfBoundsException("src.length=" + src.length + " srcPos=" + srcPos +
                " dst.length=" + dst.length + " dstPos=" + dstPos + " length=" + length);
        }

        if (src == dst && srcPos < dstPos && dstPos < srcPos + length) {
            for (int i = length - 1; i >= 0; --i) {
                dst[dstPos + i] = src[srcPos + i];
            }
        } else {
            for (int i = 0; i < length; ++i) {
                dst[dstPos + i] = src[srcPos + i];
            }
        }
    }
}