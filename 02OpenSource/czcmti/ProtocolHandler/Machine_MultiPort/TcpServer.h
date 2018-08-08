#ifndef TCPSERVER_H
#define TCPSERVER_H
#include <QTcpServer>
#include <QTcpSocket>
#include "CustomEvent.h"
#include "ControlPdu.h"
#include "ICzPlugin.h"

class TcpServer : public QObject
{
    Q_OBJECT
public:
    TcpServer(uint port, QObject *eventReceiver);
    ~TcpServer();

public:
    int StartListen(const QString &address = "127.0.0.1");
    void StopListen();
    int ResponseToHandler(int command, int subStation, int chnIdx, int ec);
    int ResponseToHandler_Handshake(qintptr descriptor);
    inline int GetPort() const {
        return m_tcpServer->serverPort();
    }

private slots:
    void tcpServer_newConnection();
    void tcpPeerSocket_readRead();
    void tcpPeerSocket_disconnected();

private:
    uint m_port;
    QObject *m_eventReceiver;
    QTcpServer *m_tcpServer;
    QMap<qintptr, QTcpSocket *> m_mapDescriptor2PeerSocket;
    QMap<qintptr, std::string> m_mapDescriptor2CachePacket;
    QMultiMap<qintptr, ControlPdu> m_mapDescriptor2RequestPdu;
};

#endif // TCPSERVER_H
