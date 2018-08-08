#ifndef TI_SHADING5_H
#define TI_SHADING5_H

#include "ICzPlugin.h"
#include "ConfWidget.h"
#include "Option.h"
#include "IChannelController.h"

class Shading5 : public IImageEvaluation
{
public:
    Shading5(const char* instanceName);
    virtual ~Shading5();

    // ICzPlugin
    int GetPluginInfo(T_PluginInfo &pluginInfo);
    int LoadOption();
    int SaveOption();
    int RestoreDefaults();
    // ITestItem
    int BindChannelContext(T_ChannelContext *context);
    E_ItemType GetItemType() const;
    bool GetIsSynchronous() const;
    QString GetReportHeader() const;
    QString GetReportContent() const;
    bool GetIsEngineeringMode() const;
    // IImageEvaluation
    int Initialize(const T_FrameParam &frameParam);
    int Evaluate(const uchar *image, uint64 &timestamp, std::vector<T_RoiInfo> &roiInfos);

private:
    struct T_ShadingInfo
    {
        bool Enable;
        T_RoiInfo ROI;
        int cx;
        int cy;
        int aveY;
        int aveR;
        int aveG;
        int aveB;
        int aveGr;
        int aveGb;

        double deltaR;
        double deltaG;
        double deltaB;
        double deltaY;

        int RGain;
        int BGain;
        int YRatio;

        double RGdif;
        double BGdif;
        int RG_result;
        int BG_result;
        int deltaY_result;
        int result;
    };

    struct ShadingStd
    {
        T_ShadingInfo ShadingInfo[5];
        int ROI_height;
        int ROI_width;
        double deltaY_limit;
        double Ydif_limit;
        double RGdif_limit;
        double BGdif_limit;
        double Ydif_max;
        int Ydif_result;
        double RGdif_max;
        double BGdif_max;
        double PPdif_max;
        int result;
    };

    bool m_initialized;
    ConfWidget *m_wdtConf;
    Option *m_option;
    T_ChannelContext *m_context;
    ShadingStd m_shading;
    QString m_instanceName;
    T_FrameParam m_frameParam;

    float Rgain[5];
    float Bgain[5];
    float ColorRGain[5];
    float ColorBGain[5];
    float Y[5];

    void calcPosition(int idx, const T_FrameParam &frameParam, T_RoiInfo &roiInfo);
    void raw10ToRaw8(const uchar *raw10, uint width, uint height, uchar *raw8,
                     uint startX, uint startY, uint roiWidth, uint roiHeight);
    int getAWBInfoResult8bit(const uchar *image, int blockIdx);
};

#endif // TI_SHADING5_H
