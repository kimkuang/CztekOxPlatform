#include "SensorImageDecoder.h"
#include "BayerRawDecoder.h"
#include "YuvDecoder.h"
#include "Rgb16Decoder.h"

SensorImageDecoder::SensorImageDecoder()
{
    m_bayerRawDecoder = new BayerRawDecoder();
    m_yuvDecoder = new YuvDecoder();
    m_rgb16Decoder = new RGB16Decoder();
}

SensorImageDecoder::~SensorImageDecoder()
{
    delete m_rgb16Decoder;
    delete m_yuvDecoder;
    delete m_bayerRawDecoder;
}

void SensorImageDecoder::Initialize(const std::map<std::string, std::string> &/*paramMap*/)
{
}

void SensorImageDecoder::SetFrameFormat(const T_FrameParam &inFrameParam, T_FrameParam &outFrameParam,
                                            uint scaleFactor)
{
    m_inFrameParam = inFrameParam;
    switch (inFrameParam.ImageFormat)
    {
    case IMAGE_FMT_RAW8:
    case IMAGE_FMT_RAW10:
    case IMAGE_FMT_RAW12:
    case IMAGE_FMT_RAW14:
    case IMAGE_FMT_RAW16:
    case IMAGE_FMT_PackedRaw10:
    case IMAGE_FMT_PackedRaw12:
    case IMAGE_FMT_PackedRaw14:
        m_bayerRawDecoder->SetFrameFormat(inFrameParam, outFrameParam, scaleFactor);
        break;
    case IMAGE_FMT_RGB16:
        m_rgb16Decoder->SetFrameFormat(inFrameParam, outFrameParam);
        break;
    case IMAGE_FMT_YUV422:
        m_yuvDecoder->SetFrameFormat(inFrameParam, outFrameParam);
        break;
    case IMAGE_FMT_YUV420:
    case IMAGE_FMT_LUMINANCE:
    default:
        break;
    }
}

int SensorImageDecoder::Decode(const uchar *inImage, uchar *&outImage)
{
    switch (m_inFrameParam.ImageFormat)
    {
    case IMAGE_FMT_RAW8:
    case IMAGE_FMT_RAW10:
    case IMAGE_FMT_RAW12:
    case IMAGE_FMT_RAW14:
    case IMAGE_FMT_RAW16:
    case IMAGE_FMT_PackedRaw10:
    case IMAGE_FMT_PackedRaw12:
    case IMAGE_FMT_PackedRaw14:
        m_bayerRawDecoder->Decode(inImage, outImage);
        break;
    case IMAGE_FMT_RGB16:
        m_rgb16Decoder->Decode(inImage, outImage);
        break;
    case IMAGE_FMT_YUV422:
        m_yuvDecoder->Decode(inImage, outImage);
        break;
    case IMAGE_FMT_YUV420:
    case IMAGE_FMT_LUMINANCE:
    default:
        break;
    }
    return ERR_NoError;
}
