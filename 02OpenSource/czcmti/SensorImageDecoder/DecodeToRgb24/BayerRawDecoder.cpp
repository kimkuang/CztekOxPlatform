#include "BayerRawDecoder.h"
#include <QElapsedTimer>
#include <QDebug>

BayerRawDecoder::BayerRawDecoder()
{
    m_raw8Buffer = nullptr;
    m_rgb24Buffer = nullptr;
    m_scaleFactor = 1;
}

BayerRawDecoder::~BayerRawDecoder()
{
    if (!m_raw8Buffer)
        delete[] m_raw8Buffer;
    if (!m_rgb24Buffer)
        delete[] m_rgb24Buffer;
}

void BayerRawDecoder::SetFrameFormat(const T_FrameParam &inFrameParam, T_FrameParam &outFrameParam,
                                        uint scaleFactor)
{
    bool imageSizeChanged = (m_inFrameParam.Width != inFrameParam.Width) ||
            (m_inFrameParam.Height != inFrameParam.Height) ||
            (m_inFrameParam.ImageFormat != inFrameParam.ImageFormat);

    if ((m_raw8Buffer != nullptr) && imageSizeChanged) {
        delete[] m_raw8Buffer; m_raw8Buffer = nullptr;
    }
    if (m_raw8Buffer == nullptr)
        m_raw8Buffer = new uchar[inFrameParam.Height * inFrameParam.Width];

    if ((m_rgb24Buffer != nullptr) && imageSizeChanged) {
        delete[] m_rgb24Buffer; m_rgb24Buffer = nullptr;
    }
    if (m_rgb24Buffer == nullptr)
        m_rgb24Buffer = new uchar[inFrameParam.Height * inFrameParam.Width * 3];

//    std::cout << "in F: " << (int)inFrameParam.ImageFormat << " M: " << (int)inFrameParam.ImageMode
//              << " W: " << inFrameParam.Width << " H: " << inFrameParam.Height << " S: " << inFrameParam.Size << std::endl;
    m_inFrameParam = inFrameParam;
    m_scaleFactor = scaleFactor;
    if (m_scaleFactor < 1)
        m_scaleFactor = 1;
    outFrameParam.Width = m_inFrameParam.Width / m_scaleFactor; // FIXME: 8 bytes aligned?
    outFrameParam.Height = m_inFrameParam.Height / m_scaleFactor;
    outFrameParam.ImageMode = (E_ImageMode)0;

    m_outFrameParam = outFrameParam;
}

int BayerRawDecoder::Decode(const uchar *inImage, uchar *&outImage)
{
    switch (m_inFrameParam.ImageFormat)
    {
    case IMAGE_FMT_RAW8:
        raw8ToRaw8(inImage, m_raw8Buffer);
        return raw8ToRgb24(m_raw8Buffer, outImage,
                           m_outFrameParam.Width, m_outFrameParam.Height, m_inFrameParam.ImageMode);
    case IMAGE_FMT_RAW10:
        raw10ToRaw8((ushort *)inImage, m_raw8Buffer);
        return raw8ToRgb24(m_raw8Buffer, outImage,
                            m_outFrameParam.Width, m_outFrameParam.Height, m_inFrameParam.ImageMode);
    case IMAGE_FMT_RAW12:
    case IMAGE_FMT_RAW14:
    case IMAGE_FMT_RAW16:
    case IMAGE_FMT_PackedRaw10:
    case IMAGE_FMT_PackedRaw12:
    case IMAGE_FMT_PackedRaw14:
    default:
        std::cerr << "Cannot support this image format " << m_inFrameParam.ImageFormat << std::endl;
        return ERR_Failed;
    }
}

void BayerRawDecoder::raw10ToRaw8(const ushort *inImage, uchar *outImage)
{
    for (uint r = 0, outRowIdx = 0; r < m_inFrameParam.Height; r += 2 * m_scaleFactor, outRowIdx +=2) {
        const ushort *curInRow = inImage + r * m_inFrameParam.Width;
        const ushort *nextInRow = curInRow + m_inFrameParam.Width;
        uchar *curOutRow = outImage + m_outFrameParam.Width * outRowIdx;
        uchar *nextOutRow = curOutRow + m_outFrameParam.Width;
        for (uint c = 0, outColIdx = 0; c < m_inFrameParam.Width; c += 2 * m_scaleFactor, outColIdx +=2) {
            curOutRow[outColIdx] = curInRow[c] >> 2;
            curOutRow[outColIdx + 1] = curInRow[c + 1] >> 2;
            nextOutRow[outColIdx] = nextInRow[c] >> 2;
            nextOutRow[outColIdx + 1] = nextInRow[c + 1] >> 2;
        }
    }
}

int BayerRawDecoder::imageMode2opencvMode(E_ImageMode imgMode)
{
    int mode = cv::COLOR_BayerRG2BGR;
    switch (imgMode) {
    case IMAGE_MODE_YCbYCr_RG_GB: // YUYV
        mode = cv::COLOR_BayerRG2BGR;
        break;
    case IMAGE_MODE_YCrYCb_GR_BG: // YVYU
        mode = cv::COLOR_BayerGR2BGR;
        break;
    case IMAGE_MODE_CbYCrY_GB_RG: // UYVY
        mode = cv::COLOR_BayerGB2BGR;
        break;
    case IMAGE_MODE_CrYCbY_BG_GR: // VYUY
        mode = cv::COLOR_BayerBG2BGR;
        break;
    default:
        break;
    }
    return mode;
}

int BayerRawDecoder::raw8ToRgb24(const uchar *inImage, uchar *&outImage, uint width, uint height,
                                    E_ImageMode imgMode)
{
    cv::Mat raw8Mat(height, width, CV_8UC1, (void *)inImage);
    int mode = imageMode2opencvMode(imgMode);
    cv::Mat rgb24Mat(height, width, CV_8UC3, (void *)m_rgb24Buffer);
    cv::cvtColor(raw8Mat, rgb24Mat, mode);
    outImage = rgb24Mat.data;

    return ERR_NoError;
}

void BayerRawDecoder::raw8ToRaw8(const uchar *inImage, uchar *outImage)
{
    for (uint r = 0, outRowIdx = 0; r < m_inFrameParam.Height; r += 2 * m_scaleFactor, outRowIdx +=2) {
        const uchar *curInRow = inImage + r * m_inFrameParam.Width;
        const uchar *nextInRow = curInRow + m_inFrameParam.Width;
        uchar *curOutRow = outImage + m_outFrameParam.Width * outRowIdx;
        uchar *nextOutRow = curOutRow + m_outFrameParam.Width;
        for (uint c = 0, outColIdx = 0; c < m_inFrameParam.Width; c += 2 * m_scaleFactor, outColIdx +=2) {
            curOutRow[outColIdx] = curInRow[c] >> 2;
            curOutRow[outColIdx + 1] = curInRow[c + 1] >> 2;
            nextOutRow[outColIdx] = nextInRow[c] >> 2;
            nextOutRow[outColIdx + 1] = nextInRow[c + 1] >> 2;
        }
    }
}
