#ifndef YUVDECODER_H
#define YUVDECODER_H
#include "czcmtidefs.h"
#include <opencv2/opencv.hpp>

class YuvDecoder
{
public:
    YuvDecoder();
    ~YuvDecoder();

    void SetFrameFormat(const T_FrameParam &inFrameParam, T_FrameParam &outFrameParam);
    int Decode(const uchar *inImage, uchar* &outImage);

private:
    T_FrameParam m_inFrameParam;
    T_FrameParam m_outFrameParam;
    cv::Mat m_yuvMat;
    cv::Mat *m_rgb24Mat;

    int imageMode2opencvMode(E_ImageMode imgMode);
};

#endif // YUVDECODER_H
