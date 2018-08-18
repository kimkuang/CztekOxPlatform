#ifndef BAYERRAWDECODER_H
#define BAYERRAWDECODER_H
#include "czcmtidefs.h"
#include "library_global.h"

class BayerDecoderPrivate;
class LIBRARY_API BayerDecoder
{
public:
    BayerDecoder();
    ~BayerDecoder();

    void SetFrameFormat(const T_FrameParam &inFrameParam, T_FrameParam &outFrameParam);
    int Decode(const uchar *inImage, uchar* &outImage);

private:
    BayerDecoderPrivate * const d_ptr;
};

#endif // BAYERRAWDECODER_H
