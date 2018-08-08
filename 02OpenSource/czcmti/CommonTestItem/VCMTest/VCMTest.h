#ifndef TI_VCMTest_H
#define TI_VCMTest_H

#include "ICzPlugin.h"
#include "ConfWidget.h"
#include "Option.h"
#include "IChannelController.h"

class VCMTest : public IImageEvaluation
{
public:
    VCMTest(const char* instanceName);
    virtual ~VCMTest();

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
    void GetAWBInfoRaw8(int block, const uchar *image);
    void CalcImageRect(void);

private:
    bool m_initialized;
    ConfWidget *m_wdtConf;
    Option *m_option;
    T_ChannelContext *m_context;
    QString m_instanceName;
    T_FrameParam m_FrameParam;

    uint64 m_u64VcmCodeTimeStamp;
    double m_fLastY[4];
    double m_fVcmYDiff[4];
    int m_i32Count;
    uint m_uLastCode;
    T_RoiInfo m_roiInfos[5];
};

#endif // TI_VCMTest_H
