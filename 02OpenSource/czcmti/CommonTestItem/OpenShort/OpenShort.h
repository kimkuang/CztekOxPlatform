#ifndef __TI_OPENSHORT_H__
#define __TI_OPENSHORT_H__

#include "ICzPlugin.h"
#include "ConfWidget.h"
#include "Option.h"
#include "IChannelController.h"

class OpenShort : public IHardwareTest
{
public:
    OpenShort(const char* instanceName);
    virtual ~OpenShort();

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
    bool allocaMemory(void);
    void freeMemory(void);
private:
    ConfWidget *m_wdtConf;
    Option *m_option;
    T_ChannelContext *m_context;
    QString m_instanceName;

    uint *m_readOpenVol_uV;
    uint *m_readShortVol_uV;
    uint *m_readResult;
    size_t m_size;
};

#endif // __TI_OPENSHORT_H__
