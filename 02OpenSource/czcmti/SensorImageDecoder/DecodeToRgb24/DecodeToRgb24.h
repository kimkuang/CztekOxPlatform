#ifndef IP_DECODETORGB24_H
#define IP_DECODETORGB24_H

#include "ICzPlugin.h"
#include "ConfWidget.h"
#include "Option.h"
#include "IChannelController.h"
#include "opencv2/opencv.hpp"

class SensorImageDecoder;
class DecodeToRgb24 : public ISensorImageDecoder
{
public:
    DecodeToRgb24();
    virtual ~DecodeToRgb24();

    // ICzPlugin
    int GetPluginInfo(T_PluginInfo &pluginInfo);
    int LoadOption();
    int SaveOption();
    int RestoreDefaults();
    // ISensorImageDecoder
    int BindChannelContext(T_ChannelContext *context);
    int Decode(uchar *inImage, const T_FrameParam &inFrameParam, const std::vector<T_RoiInfo> &roiInfos,
               uchar *&outImage, T_FrameParam &outFrameParam);

private:
    ConfWidget *m_wdtConf;
    Option *m_option;
    T_ChannelContext *m_context;
    std::string m_friendlyName;
    T_FrameParam m_inFrameParam;
    T_FrameParam m_outFrameParam;
    T_FrameParam m_resizedOutFrameParam;
    SensorImageDecoder *m_sensorImageDecoder;
    cv::Mat *m_resizedRgb24Mat;
    float m_scaleFactor;
    uint m_binning;
};

#endif // IP_DECODETORGB24_H
