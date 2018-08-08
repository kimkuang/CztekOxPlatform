#include "YuvDecoder.h"

YuvDecoder::YuvDecoder()
{
    m_rgb24Mat = nullptr;
}

YuvDecoder::~YuvDecoder()
{
    if (m_rgb24Mat)
        delete m_rgb24Mat;
}

void YuvDecoder::SetFrameFormat(const T_FrameParam &inFrameParam, T_FrameParam &outFrameParam)
{
    if ((m_rgb24Mat != nullptr) && ((m_inFrameParam.Width != inFrameParam.Width) ||
                                    (m_inFrameParam.Height != inFrameParam.Height))) {
        delete m_rgb24Mat;
        m_rgb24Mat = nullptr;
    }
    if (m_rgb24Mat == nullptr)
        m_rgb24Mat = new cv::Mat(inFrameParam.Height, inFrameParam.Width, CV_8UC3);
//    std::cout << "in F: " << (int)inFrameParam.ImageFormat << " M: " << (int)inFrameParam.ImageMode
//              << " W: " << inFrameParam.Width << " H: " << inFrameParam.Height << " S: " << inFrameParam.Size << std::endl;
    m_inFrameParam = inFrameParam;
    outFrameParam.Width = m_inFrameParam.Width; // FIXME: 8 bytes aligned?
    outFrameParam.Height = m_inFrameParam.Height;
    outFrameParam.ImageMode = (E_ImageMode)0;
//    std::cout << "out F: " << (int)outFrameParam.ImageFormat << " M: "  << (int)outFrameParam.ImageMode
//              << " W: " << outFrameParam.Width << " H: "  << outFrameParam.Height << std::endl;
    m_outFrameParam = outFrameParam;
}

int YuvDecoder::Decode(const uchar *inImage, uchar *&outImage)
{
    cv::Mat yuvMat(m_inFrameParam.Height, m_inFrameParam.Width, CV_8UC1, (void *)inImage);
    int mode = imageMode2opencvMode(m_inFrameParam.ImageMode);
    cv::cvtColor(yuvMat, *m_rgb24Mat, mode);
    outImage = m_rgb24Mat->data;

    return ERR_NoError;
}

int YuvDecoder::imageMode2opencvMode(E_ImageMode imgMode)
{
    int mode = cv::COLOR_YUV2RGB_YUYV;
    switch (imgMode) {
    case IMAGE_MODE_YCbYCr_RG_GB: // YUYV
        mode = cv::COLOR_YUV2RGB_YUYV;
        break;
    case IMAGE_MODE_YCrYCb_GR_BG: // YVYU
        mode = cv::COLOR_YUV2RGB_YVYU;
        break;
    case IMAGE_MODE_CbYCrY_GB_RG: // UYVY
        mode = cv::COLOR_YUV2RGB_UYVY;
        break;
    case IMAGE_MODE_CrYCbY_BG_GR: // VYUY
        mode = cv::COLOR_YUV2RGB_UYVY;
        break;
    default:
        break;
    }
    return mode;
}

