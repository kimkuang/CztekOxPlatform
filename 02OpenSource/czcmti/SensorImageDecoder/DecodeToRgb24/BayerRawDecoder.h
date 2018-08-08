#ifndef BAYERRAWDECODER_H
#define BAYERRAWDECODER_H
#include "czcmtidefs.h"
#include <opencv2/opencv.hpp>

class BayerRawDecoder
{
public:
    BayerRawDecoder();
    virtual ~BayerRawDecoder();

    void SetFrameFormat(const T_FrameParam &inFrameParam, T_FrameParam &outFrameParam, uint scaleFactor);
    int Decode(const uchar *inImage, uchar* &outImage);

private:
    T_FrameParam m_inFrameParam;
    T_FrameParam m_outFrameParam;
    cv::Mat *m_raw8MatTemp;
    uchar *m_raw8Buffer;
    uchar *m_rgb24Buffer;
    uint m_scaleFactor;

    void raw10ToRaw8(const ushort *inImage, uchar *outImage);
    int imageMode2opencvMode(E_ImageMode imgMode);
    int raw8ToRgb24(const uchar *inImage, uchar *&outImage, uint width, uint height, E_ImageMode imgMode);
    void raw8ToRaw8(const uchar *inImage, uchar *outImage);

};

#endif // BAYERRAWDECODER_H
