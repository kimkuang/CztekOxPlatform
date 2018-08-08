#ifndef TI_AEITEMTEST_H
#define TI_ITEMTEST_H

#include "ICzPlugin.h"
#include "ConfWidget.h"
#include "Option.h"
#include "IChannelController.h"

#define SLAVEID     0x20
class AETestItem : public IImageEvaluation
{
public:
    AETestItem(const char* instanceName);
    virtual ~AETestItem();

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
    void CalROIChlSum(const ushort* pRawBuffer, uint *pSumR, uint *pSumGr, uint *pSumGb, uint *pSumB, int iCol, int iRow);
    void CalROIPos(int& iStartX, int& iStartY, int& iEndX, int& iEndY);
    void CenterBlockRawInfo(const uchar* pImg, float& fAvgVal);

private:
    bool m_initialized;
    ConfWidget *m_wdtConf;
    Option *m_option;
    T_ChannelContext *m_context;
    T_FrameParam m_FrameParam;
    QString m_instanceName;

    uint64 m_u64ExpGainTimeStamp;
    uint m_u32Count;
    T_RoiInfo m_roiInfo;
    float m_fBrightness;
};

#endif // TI_AETestItem_H
