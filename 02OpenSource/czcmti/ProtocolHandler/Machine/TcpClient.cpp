#include "TcpClient.h"
#include <QDebug>

TcpClient::TcpClient()
{
    m_ip = "";
    m_port = 0;
    connect(&m_tcpSocket, &QTcpSocket::readyRead, this, &TcpClient::tcpSocket_readyRead);
}

void TcpClient::SetParameter(const QString &ip, uint port)
{
    m_ip = ip;
    m_port = port;
}

void TcpClient::Connect()
{
    if (m_ip.isEmpty())
        return;
    QAbstractSocket::SocketState state = m_tcpSocket.state();
    if ((state != QAbstractSocket::ConnectingState) && (state != QAbstractSocket::ConnectedState)) {
        qDebug("Connecting forward ip: %s, port: %u...", m_ip.toLatin1().data(), m_port);
        m_tcpSocket.connectToHost(m_ip, m_port);
    }
}

void TcpClient::Close()
{
    if (m_ip.isEmpty())
        return;
    m_tcpSocket.disconnectFromHost();
    m_tcpSocket.close();
    m_tcpSocket.abort();
}

void TcpClient::SendCommand(uint command, uint subStation, uint chnIdx, const QString &parameter)
{
    if (m_ip.isEmpty())
        return;
    Connect();

    ControlPdu reqCtrlPdu;
    reqCtrlPdu.Receiver = ControlPdu::Receiver_Tester;
    reqCtrlPdu.Command = (E_MachineCommand)command;
    reqCtrlPdu.SubStation = subStation;
    reqCtrlPdu.CameraIndex = chnIdx;
    reqCtrlPdu.Parameter = parameter.toStdString();
    std::string strPdu;
    if (ERR_NoError == reqCtrlPdu.Pack(strPdu)) {
        qInfo("client send command: %s", strPdu.c_str());
        m_tcpSocket.write(strPdu.c_str(), strPdu.size());
    }
}

void TcpClient::tcpSocket_readyRead()
{
    QByteArray datagram;
    std::string strPduCache = "";
    ControlPdu respCtrlPdu;
    int ec = ERR_NoError;
    while (((m_tcpSocket.bytesAvailable() > 0) || (strPduCache.size() > 0)) && (ec == ERR_NoError)) {
        qint64 toRead = m_tcpSocket.bytesAvailable();
        if (toRead > 0) {
            datagram = m_tcpSocket.read(toRead);
            strPduCache += datagram.toStdString();
        }
        qInfo("client pdu cache: %s", strPduCache.c_str());
        ec = respCtrlPdu.Unpack(strPduCache);
        if (ec != ERR_NoError)
            continue;

        if (ControlPdu::Receiver_Handler == respCtrlPdu.Receiver) {
            emit responseReceived(respCtrlPdu.Command, respCtrlPdu.SubStation, respCtrlPdu.CameraIndex, respCtrlPdu.ErrorCode);
        }
        qDebug("client pdu cache2: %s", strPduCache.c_str());
    }
}
