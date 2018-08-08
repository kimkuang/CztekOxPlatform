#include "DecodeToRgb24.h"
#include "SensorImageDecoder.h"
#include "czcmtidefs.h"
#include <string.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <QDebug>
#include <QElapsedTimer>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

CZPLUGIN_API ICzPlugin *CreatePlugin(void *arg)
{
    (void)(arg);
    return new DecodeToRgb24();
}

CZPLUGIN_API void DestroyPlugin(ICzPlugin **plugin)
{
    DecodeToRgb24 *pTestItem = (DecodeToRgb24 *)(*plugin);
    if (pTestItem != nullptr) {
        delete pTestItem; pTestItem = nullptr;
        *plugin = nullptr;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

DecodeToRgb24::DecodeToRgb24()
{
    std::cout<<__FUNCTION__<<std::endl;
    m_sensorImageDecoder = new SensorImageDecoder();
    m_wdtConf = new ConfWidget();
    m_option = Option::GetInstance();
    m_context = nullptr;
    m_friendlyName = "DecodeToRgb24";
    m_resizedRgb24Mat = nullptr;
    m_scaleFactor = 1.0;
    m_binning = 1;
    m_inFrameParam.Reset();
}

DecodeToRgb24::~DecodeToRgb24()
{
    delete m_sensorImageDecoder;
    delete m_wdtConf;
    Option::FreeInstance();
    delete m_resizedRgb24Mat;
}

int DecodeToRgb24::GetPluginInfo(T_PluginInfo &pluginInfo)
{
    pluginInfo.Version = 0x09000004;
    strcpy(pluginInfo.Description, "Sensor Image Decoder.");
    strcpy(pluginInfo.FriendlyName, "Decode To RGB24");
    strcpy(pluginInfo.InstanceName, "Decode To RGB24");
    strcpy(pluginInfo.VendorName, "CZTEK");
    pluginInfo.OptionDlgHandle = 0;

    return 0;
}

int DecodeToRgb24::LoadOption()
{
    int ec = m_wdtConf->Cache2Ui();
    return ec;
}

int DecodeToRgb24::SaveOption()
{
    return ERR_NoError;
}

int DecodeToRgb24::RestoreDefaults()
{
    return m_wdtConf->RestoreDefaults();
}

int DecodeToRgb24::BindChannelContext(T_ChannelContext *context)
{
    m_context = context;
    if ((nullptr == m_context) || (nullptr == m_context->ModuleSettings)) {
        return ERR_Failed;
    }
    return ERR_NoError;
}

int DecodeToRgb24::Decode(uchar *inImage, const T_FrameParam &inFrameParam, const std::vector<T_RoiInfo> &roiInfos,
                          uchar *&outImage, T_FrameParam &outFrameParam)
{
    if ((nullptr == m_context) || (nullptr == inImage))
    {
        qCritical("m_context: %p, inImage: %p", m_context, inImage);
        return ERR_Failed;
    }

    bool frameParamChanged = ((m_inFrameParam.Width != inFrameParam.Width) ||
                              (m_inFrameParam.Height != inFrameParam.Height) ||
                              (m_inFrameParam.ImageFormat != inFrameParam.ImageFormat) ||
                              (m_inFrameParam.ImageMode != inFrameParam.ImageMode));
    if (frameParamChanged) {
        uint framSize = inFrameParam.Width * inFrameParam.Height;
        if (framSize >= 10 * 1000 * 1000)
            m_binning = 4;
        else if (framSize >= 4 * 1000 * 1000)
            m_binning = 2;
        else
            m_binning = 1;
        outFrameParam.ImageFormat = IMAGE_FMT_RGB24;
        m_sensorImageDecoder->SetFrameFormat(inFrameParam, outFrameParam, m_binning);
        m_inFrameParam = inFrameParam;
        m_outFrameParam = outFrameParam;
        m_scaleFactor = 1.0 * outFrameParam.Width / 640;
    }
    outFrameParam = m_outFrameParam;
    int ec = m_sensorImageDecoder->Decode(inImage, outImage);
    if (ec == ERR_NoError) {
        cv::Mat rgb24Mat(m_outFrameParam.Height, m_outFrameParam.Width, CV_8UC3, (void *)outImage);
        QByteArray btShowMode;
        int showMode = 0;
        if (ERR_NoError == m_context->ChannelController->GetCacheData(DECODESHOWMODE, btShowMode, false))
        {
            showMode = btShowMode.toInt();
        }

        int w = m_outFrameParam.Width;
        int h = m_outFrameParam.Height;
        switch (showMode)
        {
        case 1:  //九宫格
        {
            cv::rectangle(rgb24Mat, cv::Rect(0, 0, w, h), cv::Scalar(0xFF, 0xFF), m_scaleFactor + 0.5);
            cv::line(rgb24Mat, cv::Point(w / 3, 0), cv::Point(w / 3, h), cv::Scalar(0xFF, 0xFF), m_scaleFactor + 0.5);
            cv::line(rgb24Mat, cv::Point(2 * w / 3, 0), cv::Point(2 * w / 3, h), cv::Scalar(0xFF, 0xFF), m_scaleFactor + 0.5);
            cv::line(rgb24Mat, cv::Point(0, h / 3), cv::Point(w, h / 3), cv::Scalar(0xFF, 0xFF), m_scaleFactor + 0.5);
            cv::line(rgb24Mat, cv::Point(0, 2 * h / 3), cv::Point(w, 2 * h / 3), cv::Scalar(0xFF, 0xFF), m_scaleFactor + 0.5);
        }
        break;
        case 2:  //米字格
        {
            cv::line(rgb24Mat, cv::Point(0, 0), cv::Point(w, h), cv::Scalar(0xFF, 0x80), m_scaleFactor + 0.5, cv::LINE_AA);
            cv::line(rgb24Mat, cv::Point(0, h), cv::Point(w, 0), cv::Scalar(0xFF, 0x80), m_scaleFactor + 0.5, cv::LINE_AA);
            cv::rectangle(rgb24Mat, cv::Rect(0, 0, w, h), cv::Scalar(0xFF, 0xFF), m_scaleFactor + 0.5);
            cv::line(rgb24Mat, cv::Point(w / 2, 0), cv::Point(w / 2, h), cv::Scalar(0xFF, 0xFF), m_scaleFactor + 0.5);
            cv::line(rgb24Mat, cv::Point(0, h / 2), cv::Point(w, h / 2), cv::Scalar(0xFF, 0xFF), m_scaleFactor + 0.5);
        }
        break;
        case 3:  //田字格
        {
            cv::rectangle(rgb24Mat, cv::Rect(0, 0, w, h), cv::Scalar(0xFF, 0xFF), m_scaleFactor + 0.5);
            cv::line(rgb24Mat, cv::Point(w / 2, 0), cv::Point(w / 2, h), cv::Scalar(0xFF, 0xFF), m_scaleFactor + 0.5);
            cv::line(rgb24Mat, cv::Point(0, h / 2), cv::Point(w, h / 2), cv::Scalar(0xFF, 0xFF), m_scaleFactor + 0.5);
        }
        break;
        case 4: //交叉平行线
        {
            cv::rectangle(rgb24Mat, cv::Rect(0, 0, w, h), cv::Scalar(0xFF, 0xFF), m_scaleFactor + 0.5);
            cv::line(rgb24Mat, cv::Point(w / 16, 0), cv::Point(w, 15 * h / 16), cv::Scalar(0xFF, 0xFF), m_scaleFactor + 0.5);
            cv::line(rgb24Mat, cv::Point(0, h / 16), cv::Point(15 * w / 16, h), cv::Scalar(0xFF, 0xFF), m_scaleFactor + 0.5);
            cv::line(rgb24Mat, cv::Point(15 *w / 16, 0), cv::Point(0, 15 * h / 16), cv::Scalar(0xFF, 0xFF), m_scaleFactor + 0.5);
            cv::line(rgb24Mat, cv::Point(w, h / 16), cv::Point(w / 16, h), cv::Scalar(0xFF, 0xFF), m_scaleFactor + 0.5);
        }
        break;
        default: //无
            break;
        }
     
        for (int i = 0; i < (int)roiInfos.size(); i++) {
            // draw maker
            uint iTemp = roiInfos[i].MarkerColor;
            cv::Scalar makerColor = cv::Scalar((iTemp >> 16) & 0xff, (iTemp >> 8) & 0xff, iTemp & 0xff);
            int thickness = (int)(roiInfos[i].LineWidth * m_scaleFactor + 0.5);

            cv::rectangle(rgb24Mat, cv::Rect(roiInfos[i].X/m_binning , roiInfos[i].Y/m_binning,
                           roiInfos[i].Width/m_binning, roiInfos[i].Height/m_binning),
                          makerColor, thickness);
            // draw text
            if (!roiInfos[i].Label.empty()) {
                iTemp = roiInfos[i].LabelColor;
                cv::Scalar labelColor = cv::Scalar((iTemp >> 16) & 0xff, (iTemp >> 8) & 0xff, iTemp & 0xff);
                int fontFace = cv::FONT_HERSHEY_SIMPLEX;
                double fontScale = 0.5 * m_scaleFactor;
                thickness = (int)(m_scaleFactor + 0.5);
                int baseLine = 0;
                cv::Size textSize = cv::getTextSize(roiInfos[i].Label, fontFace, fontScale, thickness, &baseLine);
                cv::Point origin = cv::Point(roiInfos[i].X / m_binning, roiInfos[i].Y / m_binning);
                if (origin.x + textSize.width > rgb24Mat.cols)
                    origin.x = rgb24Mat.cols - textSize.width;
                if (origin.y < textSize.height) // top overflow
                    origin.y += (textSize.height + roiInfos[i].Height/m_binning); // display in below ROI
                cv::putText(rgb24Mat, roiInfos[i].Label, origin, fontFace, fontScale, labelColor, thickness);
            }
        }
        outImage = rgb24Mat.data;
    }
    return ec;
}
