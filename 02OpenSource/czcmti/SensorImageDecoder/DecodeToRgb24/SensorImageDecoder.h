#ifndef SENSORIMAGEDECODER_H
#define SENSORIMAGEDECODER_H
#include "library_global.h"
#include "czcmtidefs.h"
#include <map>

class BayerRawDecoder;
class YuvDecoder;
class RGB16Decoder;
class LIBRARY_API SensorImageDecoder
{
public:
    SensorImageDecoder();
    virtual ~SensorImageDecoder();

    void Initialize(const std::map<std::string, std::string> &paramMap);
    void SetFrameFormat(const T_FrameParam &inFrameParam, T_FrameParam &outFrameParam, uint scaleFactor);
    int Decode(const uchar *inImage, uchar* &outImage);

private:
    T_FrameParam m_inFrameParam;
    BayerRawDecoder *m_bayerRawDecoder;
    YuvDecoder *m_yuvDecoder;
    RGB16Decoder *m_rgb16Decoder;
};

#endif // SENSORIMAGEDECODER_H
