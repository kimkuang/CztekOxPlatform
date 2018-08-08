#ifndef TI_AEITEMTEST_H
#define TI_ITEMTEST_H

#include "ICzPlugin.h"
#include "ConfWidget.h"
#include "Option.h"
#include "IChannelController.h"

#define SLAVEID     0x20
class AFMoveToPos : public IImageEvaluation
{
public:
    AFMoveToPos(const char* instanceName);
    virtual ~AFMoveToPos();

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
    bool m_initialized;
    ConfWidget *m_wdtConf;
    Option *m_option;
    T_ChannelContext *m_context;
    QString m_instanceName;

    uint64 m_u64VcmCodeTimeStamp;
    ushort m_u16InfinityCode;
    ushort m_u16MacroCode;
    ushort m_u16MiddleCode;
    ushort m_u16MoveDistanceCode;
    ushort m_u16SaveCode;

    ushort calCodeByDistance(void);
};

#endif // TI_AFMoveToPos_H
