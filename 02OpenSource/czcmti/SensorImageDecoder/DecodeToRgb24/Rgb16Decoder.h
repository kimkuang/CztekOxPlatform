#ifndef RGB16DECODER_H
#define RGB16DECODER_H
#include "czcmtidefs.h"

class RGB16Decoder
{
public:
    RGB16Decoder();

    void SetFrameFormat(const T_FrameParam &inFrameParam, T_FrameParam &outFrameParam);
    int Decode(const uchar *inImage, uchar* &outImage);
};

#endif // RGB16DECODER_H
