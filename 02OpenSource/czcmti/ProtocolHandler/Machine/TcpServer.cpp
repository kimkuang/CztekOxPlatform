#include "TcpServer.h"
#include <QCoreApplication>
#include "czcmtidefs.h"
#include "Option.h"

TcpServer::TcpServer()
{
    m_tcpServer = new QTcpServer();
    connect(m_tcpServer, &QTcpServer::newConnection, this, &TcpServer::tcpServer_newConnection);
}

TcpServer::~TcpServer()
{
    m_mapDescriptor2PeerSocket.clear();
    m_mapDescriptor2CachePacket.clear();
    m_mapDescriptor2RequestPdu.clear();
    StopListen();
}

int TcpServer::StartListen(uint port, const QString &address)
{
//    qDebug()<<__FUNCTION__;
    QHostAddress hostAddress = QHostAddress(address);
    if (hostAddress.isNull())
        hostAddress = QHostAddress::LocalHost;
    if (!m_tcpServer->listen(QHostAddress::Any, port)/*m_tcpServer->listen(hostAddress, port)*/) {
        m_tcpServer->close();
        qCritical()<<QString("Listening on %1@%2 failed.").arg(hostAddress.toString()).arg(port);
        return ERR_Failed;
    }
    qDebug()<<QString("Listening on %1@%2 ...").arg(hostAddress.toString()).arg(port);
    return ERR_NoError;
}

void TcpServer::StopListen()
{
//    qDebug()<<__FUNCTION__;
    m_tcpServer->close();
}

int TcpServer::ResponseToHandler(int command, int subStation, int chnIdx, int ec)
{
//    qDebug()<<__FUNCTION__;
    QMutableMapIterator<qintptr, ControlPdu> it(m_mapDescriptor2RequestPdu);
    while (it.hasNext()) {
        it.next();
        ControlPdu reqCtrlPdu = it.value();
        if ((reqCtrlPdu.Command == command) && (reqCtrlPdu.CameraIndex == chnIdx) &&
            (reqCtrlPdu.SubStation == subStation))
        {
            ControlPdu respCtrlPdu;
            respCtrlPdu.Receiver = ControlPdu::Receiver_Handler;
            respCtrlPdu.Command = (E_MachineCommand)command;
            respCtrlPdu.CameraIndex = chnIdx;
            respCtrlPdu.SubStation = subStation;
            respCtrlPdu.ErrorCode = ec;

            std::string respPacket;
            respCtrlPdu.Pack(respPacket);
            qintptr descriptor = it.key();
//            qDebug()<<__FUNCTION__<<descriptor;
            if (m_mapDescriptor2PeerSocket.contains(descriptor))
                m_mapDescriptor2PeerSocket[descriptor]->write(respPacket.c_str(), respPacket.size());
            m_mapDescriptor2RequestPdu.remove(descriptor, reqCtrlPdu);
            return ERR_NoError;
        }
    }
    return ERR_Failed;
}

int TcpServer::ResponseToHandler_Handshake(qintptr descriptor)
{
//    qDebug()<<__FUNCTION__;
    if (m_mapDescriptor2PeerSocket.contains(descriptor)) {
//        qDebug()<<__FUNCTION__<<descriptor;
        std::string respPacket = Option::GetInstance()->HandshakeResponse.toStdString();
        m_mapDescriptor2PeerSocket[descriptor]->write(respPacket.c_str(), respPacket.size());
    }
    return ERR_NoError;
}

void TcpServer::tcpServer_newConnection()
{
    QTcpSocket *tcpPeerSocket = m_tcpServer->nextPendingConnection();
    connect(tcpPeerSocket, &QTcpSocket::readyRead, this, &TcpServer::tcpPeerSocket_readRead);
//    qDebug()<<__FUNCTION__<<tcpPeerSocket->socketDescriptor();
    m_mapDescriptor2PeerSocket.insert(tcpPeerSocket->socketDescriptor(), tcpPeerSocket);
    connect(tcpPeerSocket, &QTcpSocket::disconnected, this, &TcpServer::tcpPeerSocket_disconnected);
}

void TcpServer::tcpPeerSocket_readRead()
{
    QTcpSocket *tcpSocket = static_cast<QTcpSocket*>(sender());
    qintptr descriptor = tcpSocket->socketDescriptor();
//    qDebug()<<__FUNCTION__<<descriptor;
    QByteArray datagram;
    std::string strPduCache = "";
    if (m_mapDescriptor2CachePacket.contains(descriptor))
        strPduCache = m_mapDescriptor2CachePacket[descriptor];
    std::string handshakeRequest = Option::GetInstance()->HandshakeRequest.toStdString();
//    qDebug()<<Option::GetInstance()->HandshakeRequest<<Option::GetInstance()->HandshakeResponse;
    ControlPdu reqCtrlPdu;
    int ec = ERR_NoError;
//    QString strBind1, strBind2;
    while (((tcpSocket->bytesAvailable() > 0) || (strPduCache.size() > 0)) && (ec == ERR_NoError)) {
        qint64 toRead = tcpSocket->bytesAvailable();
        if (toRead > 0) {
            datagram = tcpSocket->read(toRead);
            strPduCache += datagram.toStdString();
        }
        qInfo("pdu cache: %s", strPduCache.c_str());
        if (!handshakeRequest.empty()) {
            int idx = strPduCache.find(handshakeRequest);
            if (idx != -1) {
                ec = ResponseToHandler_Handshake(descriptor);
                strPduCache = strPduCache.substr(idx + handshakeRequest.size(), strPduCache.size() - idx - handshakeRequest.size());
            }
        }
        ec = reqCtrlPdu.Unpack(strPduCache);
        if (ec != ERR_NoError)
            continue;

        if (ControlPdu::Receiver_TestItem == reqCtrlPdu.Receiver)
        {
            int id = std::stoi(reqCtrlPdu.Packet.substr(1, 2));
            int state = std::stoi(reqCtrlPdu.Packet.substr(3, 4));
            emit antiShakeTriggered(id, state);
        }
        else if (ControlPdu::Receiver_Tester == reqCtrlPdu.Receiver) {
            emit commandReceived(reqCtrlPdu.Command, reqCtrlPdu.SubStation, reqCtrlPdu.CameraIndex, QString::fromStdString(reqCtrlPdu.Parameter));
            m_mapDescriptor2RequestPdu.insert(descriptor, reqCtrlPdu);
        }
        qDebug("pdu cache2: %s", strPduCache.c_str());
    }
    m_mapDescriptor2CachePacket[descriptor] = strPduCache;
}

void TcpServer::tcpPeerSocket_disconnected()
{
//    qDebug()<<__FUNCTION__;
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());
    qintptr descriptor = socket->socketDescriptor();
    QMap<qintptr, QTcpSocket *>::iterator it;
    for (it = m_mapDescriptor2PeerSocket.begin(); it != m_mapDescriptor2PeerSocket.end(); ++it)
    {
        if (descriptor == it.key())
        {
//            qDebug()<<__FUNCTION__<<descriptor;
            m_mapDescriptor2CachePacket[descriptor] = "";
            m_mapDescriptor2PeerSocket.erase(it);
            break;
        }
    }
}
