#ifndef __MACHINEHANDLER_H__
#define __MACHINEHANDLER_H__
#include "ICzPlugin.h"
#include "ConfWidget.h"
#include "Option.h"
#include "TcpServer.h"
#include "TcpClient.h"
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
    int ResponseToHandler(int command, int subStation, int chnIdx, int categoryCode);

private slots:
    void tcpServer_antiShakeTriggered(int id, int state);
    void tcpServer_commandReceived(uint command, uint subStation, uint chnIdx, const QString &parameter);
    void tcpClient_responseReceived(uint command, uint subStation, uint chnIdx, uint ec);

private:
    ConfWidget *m_wdtConf;
    Option *m_option;
    QString m_friendlyName;
    uint m_cameraCount;
    TcpServer *m_tcpServer;
    TcpClient *m_tcpClient;
    QMap<uint, T_ChannelContext*> m_mapChnIdx2Context;
    QVector<uint> m_vecAllChnIdx;
};

#endif // __MACHINEHANDLER_H__
