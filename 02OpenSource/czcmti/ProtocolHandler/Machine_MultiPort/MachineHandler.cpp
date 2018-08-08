#include "MachineHandler.h"
#include <QDebug>
#include <QCoreApplication>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

CZPLUGIN_API ICzPlugin *CreatePlugin(void *arg)
{
    (void)(arg);
    return new MachineHandler();
}

CZPLUGIN_API void DestroyPlugin(ICzPlugin **plugin)
{
    MachineHandler *pProtocolHandler = (MachineHandler *)(*plugin);
    if (pProtocolHandler != nullptr) {
        delete pProtocolHandler; pProtocolHandler = nullptr;
        *plugin = nullptr;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

MachineHandler::MachineHandler()
{
    m_option = Option::GetInstance();
    m_wdtConf = new ConfWidget();
    m_friendlyName = "Multi-Port Machine Handler";
    m_mapWndHandle2TcpServerInfo.clear();
}

MachineHandler::~MachineHandler()
{
    Stop();
    delete m_wdtConf;
    Option::FreeInstance();
    QMapIterator<qintptr, T_TcpServerInfo> it(m_mapWndHandle2TcpServerInfo);
    while (it.hasNext()) {
        it.next();
        delete it.value().pTcpServer;
    }
    m_mapWndHandle2TcpServerInfo.clear();
}

int MachineHandler::GetPluginInfo(T_PluginInfo &pluginInfo)
{
    pluginInfo.Version = 0x09000001;
    strcpy(pluginInfo.Description, "Multi-port Machine handler(2018-7-7).");
    strcpy(pluginInfo.FriendlyName, m_friendlyName.toLatin1().constData());
    strcpy(pluginInfo.InstanceName, m_friendlyName.toLatin1().constData());
    strcpy(pluginInfo.VendorName, "CZTEK");
    pluginInfo.OptionDlgHandle = (uint64)m_wdtConf;

    return ERR_NoError;
}

int MachineHandler::LoadOption()
{
    if (m_mapWndHandle2TcpServerInfo.size() < 1)
        return ERR_Failed;
    T_ChannelContext *pChannelContext = m_mapWndHandle2TcpServerInfo.first().pChannelContext;
    if ((nullptr == pChannelContext) || (nullptr == pChannelContext->MachineSettings))
        return ERR_Failed;
    QMap<QString, QString> configurations;
    pChannelContext->MachineSettings->ReadSection(m_friendlyName, configurations);
    if (configurations.contains("listenPort1"))
        m_option->ListenPort1 = configurations["listenPort1"].toInt();
    if (configurations.contains("listenPort2"))
        m_option->ListenPort2 = configurations["listenPort2"].toInt();
    if (configurations.contains("listenPort3"))
        m_option->ListenPort3 = configurations["listenPort3"].toInt();
    if (configurations.contains("listenPort4"))
        m_option->ListenPort4 = configurations["listenPort4"].toInt();
    if (configurations.contains("listenIp"))
        m_option->ListenIp = configurations["listenIp"];
    if (configurations.contains("handshakeRequest"))
        m_option->HandshakeRequest = configurations["handshakeRequest"];
    if (configurations.contains("handshakeResponse"))
        m_option->HandshakeResponse = configurations["handshakeResponse"];

    int ec = m_wdtConf->Cache2Ui();
    return ec;
}

int MachineHandler::SaveOption()
{
    if (m_mapWndHandle2TcpServerInfo.size() < 1)
        return ERR_Failed;
    T_ChannelContext *pChannelContext = m_mapWndHandle2TcpServerInfo.first().pChannelContext;
    if ((nullptr == pChannelContext) || (nullptr == pChannelContext->MachineSettings))
        return ERR_Failed;
    int ec = m_wdtConf->Ui2Cache();
    if (ec == 0) {
        QMap<QString, QString> configurations;
        configurations["listenPort1"] = QString::number(m_option->ListenPort1);
        configurations["listenPort2"] = QString::number(m_option->ListenPort2);
        configurations["listenPort3"] = QString::number(m_option->ListenPort3);
        configurations["listenPort4"] = QString::number(m_option->ListenPort4);
        configurations["listenIp"] = m_option->ListenIp.trimmed();
        configurations["handshakeRequest"] = m_option->HandshakeRequest.trimmed();
        configurations["handshakeResponse"] = m_option->HandshakeResponse.trimmed();

        // save to file
        if (!pChannelContext->MachineSettings->WriteSection(m_friendlyName, configurations))
            ec = ERR_Failed;
    }
    return ec;
}

int MachineHandler::RestoreDefaults()
{
    return m_wdtConf->RestoreDefaults();
}

// each child window will call this function one time
int MachineHandler::BindChannelContext(T_ChannelContext *context)
{
    if (context == nullptr)
        return ERR_Failed;
    qintptr wndHandle = (qintptr)(context->EventReceiver);
    if (!m_mapWndHandle2TcpServerInfo.contains(wndHandle)) {
        m_mapWndHandle2TcpServerInfo.insert(wndHandle, T_TcpServerInfo());
    }
    T_TcpServerInfo &tcpServerInfo = m_mapWndHandle2TcpServerInfo[wndHandle];
    tcpServerInfo.pChannelContext = context;
    tcpServerInfo.nChnIdx = context->ChnIdx;
    if (nullptr != tcpServerInfo.pTcpServer) {
        delete tcpServerInfo.pTcpServer;
        tcpServerInfo.pTcpServer = nullptr;
    }
    int ec = LoadOption();
    if (ec != ERR_NoError) {
        qCritical() << QString("Loading plugin option[%1] failed.").arg(m_friendlyName);
        return ec;
    }
    if (nullptr == tcpServerInfo.pTcpServer) {
        uint port = 0;
        switch (context->ChnIdx) {
        case 0:
            port = m_option->ListenPort1;
            break;
        case 1:
            port = m_option->ListenPort2;
            break;
        case 2:
            port = m_option->ListenPort3;
            break;
        case 3:
            port = m_option->ListenPort4;
            break;
        default:
            qCritical("Max support 4 channels!");
            return ERR_NoError;
        }
        tcpServerInfo.pTcpServer = new TcpServer(port, context->EventReceiver);
    }
    return ERR_NoError;
}

int MachineHandler::Start()
{
    int ec = 0;
    QMapIterator<qintptr, T_TcpServerInfo> it(m_mapWndHandle2TcpServerInfo);
    while (it.hasNext()) {
        it.next();
        ec = it.value().pTcpServer->StartListen(m_option->ListenIp);
        if (ec != ERR_NoError)
            return ec;
    }
    return ec;
}

int MachineHandler::Stop()
{
    qDebug("<<===Stopping listen.");
    QMapIterator<qintptr, T_TcpServerInfo> it(m_mapWndHandle2TcpServerInfo);
    while (it.hasNext()) {
        it.next();
        it.value().pTcpServer->StopListen();
    }
    return ERR_NoError;
}

int MachineHandler::ResponseToHandler(int command, int subStation, int chnIdx, int ec)
{
    TcpServer *tcpServer = nullptr;
    QMapIterator<qintptr, T_TcpServerInfo> it(m_mapWndHandle2TcpServerInfo);
    while (it.hasNext()) {
        it.next();
        if ((int)it.value().nChnIdx == chnIdx) {
            tcpServer = it.value().pTcpServer;
            break;
        }
    }
    if (tcpServer != nullptr) {
        return tcpServer->ResponseToHandler(command, subStation, 0, ec);
    }
    return ERR_Failed;
}
