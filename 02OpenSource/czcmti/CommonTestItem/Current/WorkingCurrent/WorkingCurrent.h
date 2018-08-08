#ifndef WORKINGCURRENT_H
#define WORKINGCURRENT_H

#include "ICzPlugin.h"
#include "ConfWidget.h"
#include "Option.h"
#include "IChannelController.h"

class WorkingCurrent : public IHardwareTest
{
public:
    WorkingCurrent(const char* instanceName);
    virtual ~WorkingCurrent();

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
    // IHardwareTest
    E_HardwareTestType GetHardwareType() const;
    bool GetContinueWhenFailed() const;
    int Initialize();
    int RunTest(std::vector<std::string> &resultTable);

private:
    ConfWidget *m_wdtConf;
    Option *m_option;
    T_ChannelContext *m_context;
    QString m_instanceName;

    float *m_current_nA;
    size_t m_size;
};

#endif // WORKINGCURRENT_H
