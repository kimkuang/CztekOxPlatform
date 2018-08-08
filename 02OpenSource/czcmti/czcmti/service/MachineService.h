#ifndef MACHINESERVICE_H
#define MACHINESERVICE_H
#include <QMap>
#include <QString>
#include "czcmtidefs.h"
#include "ICzPlugin.h"
#include "CzUtilsDefs.h"

class MachineService
{
public:
    static MachineService *GetInstance();
    static void FreeInstance();
private:
    static std::mutex m_mutex;
    static MachineService *m_machineService;

    MachineService();
    virtual ~MachineService();
    MachineService(const MachineService &) = delete;
    MachineService &operator=(const MachineService &) = delete;

public:
//    void BindChannelContext(T_ChannelContext *context);
    int GetPluginInfo(const QString &libFileName, T_PluginInfo &pluginInfo);
    IProtocolHandler *GetPluginInstance(const QString &libFileName);
    void FreePluginInstance(const QString &libFileName);
    void FreeAllPluginInstances();

    int RestoreDefaults(const QString &libFileName);
    int LoadSettings(const QString &libFileName);
    int SaveSettings(const QString &libFileName);
    int Stop(const QString &libFileName);
    int Start(const QString &libFileName);
    int ResponseToHandler(const QString &libFileName, int command, int substation, int chnIdx, int ec);

private:
    QMap<QString, IProtocolHandler *> m_mapName2Instance; // <Library file name, Instance>
};

#endif // MACHINESERVICE_H
