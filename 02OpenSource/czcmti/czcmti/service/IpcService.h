#ifndef IPCSERVICE_H
#define IPCSERVICE_H
#include <QTcpServer>
#include <QTcpSocket>
#include "conf/SystemSettings.h"

class IpcService : public QObject
{
    Q_OBJECT
public:
    IpcService(SystemSettings *sysSettings);
    ~IpcService();

public:
    int StartListen(uint port, const QString &address = "127.0.0.1");
    void StopListen();
    QString GetFuseId(uint chnIdx);

private slots:
    void tcpServer_newConnection();
    void tcpPeerSocket_readRead();
    void tcpPeerSocket_disconnected();

private:
    SystemSettings *m_sysSettings;
    QTcpServer *m_tcpServer;

    QByteArray processRequest(const QByteArray &request);
    void getPeerIpcParam(QString &ip, uint &port);
    QString sendReceive(const QString request);
};

#endif // IPCSERVICE_H
