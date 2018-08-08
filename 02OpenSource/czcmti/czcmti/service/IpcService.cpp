#include "IpcService.h"
#include "czcmtidefs.h"
#include "controller/ChannelController.h"

IpcService::IpcService(SystemSettings *sysSettings)
{
    m_sysSettings = sysSettings;
    m_tcpServer = new QTcpServer();
    connect(m_tcpServer, &QTcpServer::newConnection, this, &IpcService::tcpServer_newConnection);
}

IpcService::~IpcService()
{
    StopListen();
}

int IpcService::StartListen(uint port, const QString &address)
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

void IpcService::StopListen()
{
//    qDebug()<<__FUNCTION__;
    if (m_tcpServer->isListening()) {
        m_tcpServer->close();
    }
}

QString IpcService::GetFuseId(uint chnIdx)
{
    QString request = QString("GetFuseId:%1").arg(chnIdx);
    return sendReceive(request);
}

void IpcService::tcpServer_newConnection()
{
    QTcpSocket *tcpPeerSocket = m_tcpServer->nextPendingConnection();
    connect(tcpPeerSocket, &QTcpSocket::readyRead, this, &IpcService::tcpPeerSocket_readRead);
//    qDebug()<<__FUNCTION__<<tcpPeerSocket->socketDescriptor();
    connect(tcpPeerSocket, &QTcpSocket::disconnected, this, &IpcService::tcpPeerSocket_disconnected);
}

void IpcService::tcpPeerSocket_readRead()
{
    QTcpSocket *tcpSocket = static_cast<QTcpSocket*>(sender());
    QByteArray request = tcpSocket->readAll();
    QByteArray response = processRequest(request);
    tcpSocket->write(response);
}

void IpcService::tcpPeerSocket_disconnected()
{
//    qDebug()<<__FUNCTION__;
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());
    socket->deleteLater();
}

QByteArray IpcService::processRequest(const QByteArray &request)
{
    QString strRequest = QString::fromUtf8(request);
    QString command = strRequest.section(QChar(':'), 0, 0).trimmed();
    QString parameter = strRequest.section(QChar(':'), 1, 1).trimmed();
    qDebug() << command << parameter;
    QStringList slstAllChnParameter = parameter.split(QChar(';'), QString::SkipEmptyParts);
    int ec = 0;
    if (command == "SetUseMesFlag") {
        for (int i = 0; i < slstAllChnParameter.size(); i++) {
            QStringList slstTheChnParameter = slstAllChnParameter[i].split(QChar(','), QString::SkipEmptyParts);
            if (slstTheChnParameter.size() != 2) {
                return QByteArray();
            }
            uint chnIdx = slstTheChnParameter[0].toUInt();
            bool flag = (slstTheChnParameter[1] == "1");
            ec = ChannelController::GetInstance(chnIdx)->SetUseMesFlag(flag);
            if (ec != ERR_NoError)
                break;
        }
        return QString((ec == ERR_NoError) ? "OK" : "NG").toUtf8();
    }
    else if (command == "MesCheck") {
        for (int i = 0; i < slstAllChnParameter.size(); i++) {
            uint chnIdx = slstAllChnParameter[i].toUInt();
            QStringList mesData;
            ec = ChannelController::GetInstance(chnIdx)->GetMesData(Mes_Check, mesData);
            if (ec != ERR_NoError)
                break;
        }
        return QString((ec == ERR_NoError) ? "OK" : "NG").toUtf8();
    }
    else if (command == "MesDataDetail") {
        QString mesDetail = "";
        for (int i = 0; i < slstAllChnParameter.size(); i++) {
            uint chnIdx = slstAllChnParameter[i].toUInt();
            QStringList mesData;
            ec = ChannelController::GetInstance(chnIdx)->GetMesData(Mes_Update, mesData);
            if (ec != ERR_NoError) {
                mesDetail.clear();
                break;
            }
            if (!mesData.isEmpty())
                mesDetail += mesData.last();
        }
        return mesDetail.toUtf8();
    }
    else if (command == "GetSensorFuseId") {
        QStringList fuseIdList;
        for (int i = 0; i < slstAllChnParameter.size(); i++) {
            uint chnIdx = slstAllChnParameter[i].toUInt();
            fuseIdList.append(ChannelController::GetInstance(chnIdx)->GetSensorFuseId());
        }
        return fuseIdList.join(QChar(';')).toUtf8();
    }
    return QByteArray();
}

void IpcService::getPeerIpcParam(QString &ip, uint &port)
{
    port = m_sysSettings->Ipc.ListenPort;
    if (m_sysSettings->Ipc.CurrentDeviceIdx == 0) {
        ip = m_sysSettings->Ipc.Dev1Ip;
    }
    else {
        ip = m_sysSettings->Ipc.Dev0Ip;
    }
}

QString IpcService::sendReceive(const QString request)
{
    QString ip;
    uint port;
    getPeerIpcParam(ip, port);
    QTcpSocket tcpSocket;
    tcpSocket.connectToHost(QHostAddress(ip), port);
    if (!tcpSocket.waitForConnected(2000)) {
        qCritical().noquote() << QString("Connect %1@%2 failed.").arg(ip).arg(port);
        return "";
    }
    tcpSocket.write(request.toUtf8());
    if (!tcpSocket.waitForBytesWritten(1000) || !tcpSocket.waitForReadyRead(1000)) {
        qCritical().noquote() << QString("Write and read data %1@%2 failed.").arg(ip).arg(port);
        return "";
    }
    QString response = QString::fromUtf8(tcpSocket.readAll());
    return response;
}
