#ifndef __MACHINEHANDLER_H__
#define __MACHINEHANDLER_H__
#include "ICzPlugin.h"
#include "ConfWidget.h"
#include "Option.h"
#include "TcpServer.h"
#include <QObject>

class MachineHandler : public QObject, public IProtocolHandler
{
    Q_OBJECT
public:
    MachineHandler();
    virtual ~MachineHandler();

    // ICzPlugin
    int GetPluginInfo(T_PluginInfo &pluginInfo);
    int LoadOption();
    int SaveOption();
    int RestoreDefaults();
    // IProtocolHandler
    int BindChannelContext(T_ChannelContext *context);
    int Start();
    int Stop();
    int ResponseToHandler(int command, int subStation, int chnIdx, int ec);

private:
    struct T_TcpServerInfo {
        T_ChannelContext *pChannelContext;
        TcpServer *pTcpServer;
        uint nChnIdx;
        T_TcpServerInfo() {
            pChannelContext = nullptr;
            pTcpServer = nullptr;
            nChnIdx = 0;
        }
    };
    ConfWidget *m_wdtConf;
    Option *m_option;
    QString m_friendlyName;
    QMap<qintptr, T_TcpServerInfo> m_mapWndHandle2TcpServerInfo;
};

#endif // __MACHINEHANDLER_H__
