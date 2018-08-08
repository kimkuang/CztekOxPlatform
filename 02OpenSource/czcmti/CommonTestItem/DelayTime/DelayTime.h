#ifndef TI_DelayTime_H
#define TI_DelayTime_H

#include "ICzPlugin.h"
#include "ConfWidget.h"
#include "Option.h"
#include "IChannelController.h"

class DelayTime : public IImageEvaluation
{
public:
    DelayTime(const char *instanceName);
    DelayTime() = delete;
    virtual ~DelayTime();

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
    ConfWidget *m_wdtConf;
    Option *m_option;
    T_ChannelContext *m_context;
    QString m_instanceName;

};

#endif // TI_DelayTime_H
