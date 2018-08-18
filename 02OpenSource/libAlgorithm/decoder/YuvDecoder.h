#ifndef YUVDECODER_H
#define YUVDECODER_H
#include "czcmtidefs.h"
#include "library_global.h"

class YuvDecoderPrivate;
class LIBRARY_API YuvDecoder
{
public:
    YuvDecoder();
    ~YuvDecoder();

    void SetFrameFormat(const T_FrameParam &inFrameParam, T_FrameParam &outFrameParam);
    int Decode(const uchar *inImage, uchar* &outImage);

private:
    YuvDecoderPrivate * const d_ptr;
};

#endif // YUVDECODER_H
