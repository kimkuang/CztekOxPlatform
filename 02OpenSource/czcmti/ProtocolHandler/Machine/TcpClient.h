#ifndef TCPCLIENT_H
#define TCPCLIENT_H
#include <QObject>
#include <QTcpSocket>
#include "ControlPdu.h"

class TcpClient : public QObject
{
    Q_OBJECT
public:
    TcpClient();

    void SetParameter(const QString &ip, uint port);
    void Connect();
    void Close();
    void SendCommand(uint command, uint subStation, uint chnIdx, const QString &parameter);

signals:
    void responseReceived(uint command, uint subStation, uint chnIdx, uint categoryCode);

private slots:
    void tcpSocket_readyRead();

private:
    QTcpSocket m_tcpSocket;
    QString m_ip;
    uint m_port;
};

#endif // TCPCLIENT_H
