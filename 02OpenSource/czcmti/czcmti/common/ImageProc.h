#ifndef IMAGEPROC_H
#define IMAGEPROC_H
#include "czcmtidefs.h"

class ImageProc
{
public:
    ImageProc();

    static void verticalFlip(uchar *in, int w, int h);
    static void rawToRaw8(const ushort *src, uchar *dst, int size, uchar bit);
    static void rawToRaw8(const ushort *src, uchar *dst, int w, int h, uchar bit);
    static void raw8ToRaw(const uchar *src, ushort *dst, int w, int h, uchar bit);

    static void scaleOutRGBImage(const uchar *src, int sw, int sh, uchar *dst, int dw, int dh);
    static void cutOutRGBImage(const uchar *src, int sw, int sh, uchar *dst, int dw, int dh);

    static void rawToRgb24Demosaic(const uchar *src, uchar *dst, int w, int h, uchar fmt);
    static void rawToRgb24DemosaicForRGGB(const uchar *src, uchar *dst, int w, int h);
    static void rawToRgb24DemosaicForGRBG(const uchar *src, uchar *dst, int w, int h);
    static void rawToRgb24DemosaicForGBRG(const uchar *src, uchar *dst, int w, int h);
    static void rawToRgb24DemosaicForBGGR(const uchar *src, uchar *dst, int w, int h);

    static void rawToRgb24(const uchar *src, uchar *dst, int w, int h, uchar fmt);
    static void rawToRgb24ForRGGB(const uchar *src, uchar *dst, int w, int h);
    static void rawToRgb24ForGRBG(const uchar *src, uchar *dst, int w, int h);
    static void rawToRgb24ForGBRG(const uchar *src, uchar *dst, int w, int h);
    static void rawToRgb24ForBGGR(const uchar *src, uchar *dst, int w, int h);

    static void rgb24ToRaw(const uchar *src, uchar *dst, int w, int h, uchar fmt);
    static void rgb24ToRawForGBRG(const uchar *src, uchar *dst, int w, int h);
    static void rgb24ToRawForRGGB(const uchar *src, uchar *dst, int w, int h);
    static void rgb24ToRawForGRBG(const uchar *src, uchar *dst, int w, int h);
    static void rgb24ToRawForBGGR(const uchar *src, uchar *dst, int w, int h);
};

#endif // IMAGEPROC_H
