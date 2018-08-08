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
    m_friendlyName = "Machine Handler";
    m_cameraCount = 0;
    m_tcpServer = new TcpServer();
    connect(m_tcpServer, &TcpServer::antiShakeTriggered, this, &MachineHandler::tcpServer_antiShakeTriggered);
    connect(m_tcpServer, &TcpServer::commandReceived, this, &MachineHandler::tcpServer_commandReceived);
    m_tcpClient = new TcpClient();
    connect(m_tcpClient, &TcpClient::responseReceived, this, &MachineHandler::tcpClient_responseReceived);
}

MachineHandler::~MachineHandler()
{
    Stop();
    delete m_wdtConf;
    Option::FreeInstance();
    delete m_tcpClient;
    delete m_tcpServer;
}

int MachineHandler::GetPluginInfo(T_PluginInfo &pluginInfo)
{
    pluginInfo.Version = 0x09000008;
    strcpy(pluginInfo.Description, "Machine handler(2018-7-9).");
    strcpy(pluginInfo.FriendlyName, m_friendlyName.toLatin1().constData());
    strcpy(pluginInfo.InstanceName, m_friendlyName.toLatin1().constData());
    strcpy(pluginInfo.VendorName, "CZTEK");
    pluginInfo.OptionDlgHandle = (uint64)m_wdtConf;

    return ERR_NoError;
}

int MachineHandler::LoadOption()
{
    if (m_mapChnIdx2Context.size() < 1)
        return ERR_Failed;
    T_ChannelContext *pChannelContext = m_mapChnIdx2Context.first();
    if ((nullptr == pChannelContext) || (nullptr == pChannelContext->MachineSettings) || (nullptr == pChannelContext->SystemSettings))
        return ERR_Failed;
    QMap<QString, QString> configurations;
    pChannelContext->MachineSettings->ReadSection(m_friendlyName, configurations);
    if (configurations.contains("listenPort"))
        m_option->ListenPort = configurations["listenPort"].toInt();
    if (configurations.contains("listenIp"))
        m_option->ListenIp = configurations["listenIp"];
    if (configurations.contains("forwardPort"))
        m_option->ForwardPort = configurations["forwardPort"].toInt();
    if (configurations.contains("forwardIp"))
        m_option->ForwardIp = configurations["forwardIp"];
    if (configurations.contains("handshakeRequest"))
        m_option->HandshakeRequest = configurations["handshakeRequest"];
    if (configurations.contains("handshakeResponse"))
        m_option->HandshakeResponse = configurations["handshakeResponse"];

    m_tcpClient->SetParameter(m_option->ForwardIp, m_option->ForwardPort);

    m_cameraCount = pChannelContext->SystemSettings->ReadInteger("system", "cameraNumber", 1);

    int ec = m_wdtConf->Cache2Ui();
    return ec;
}

int MachineHandler::SaveOption()
{
    if (m_mapChnIdx2Context.size() < 1)
        return ERR_Failed;
    T_ChannelContext *pChannelContext = m_mapChnIdx2Context.first();
    if ((nullptr == pChannelContext) || (nullptr == pChannelContext->MachineSettings))
        return ERR_Failed;
    int ec = m_wdtConf->Ui2Cache();
    if (ec == 0) {
        QMap<QString, QString> configurations;
        configurations["listenPort"] = QString::number(m_option->ListenPort);
        configurations["listenIp"] = m_option->ListenIp.trimmed();
        configurations["forwardPort"] = QString::number(m_option->ForwardPort);
        configurations["forwardIp"] = m_option->ForwardIp.trimmed();
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
    if (!m_mapChnIdx2Context.contains(context->ChnIdx)) {
        m_mapChnIdx2Context.insert(context->ChnIdx, context);
    }
    return ERR_NoError;
}

int MachineHandler::Start()
{
    m_tcpClient->Connect();
    int ec = m_tcpServer->StartListen(m_option->ListenPort, m_option->ListenIp);
    return ec;
}

int MachineHandler::Stop()
{
    qDebug("<<===Closing client...");
    m_tcpClient->Close();
    qDebug("<<===Stopping listen...");
    m_tcpServer->StopListen();
    return ERR_NoError;
}

int MachineHandler::ResponseToHandler(int command, int subStation, int chnIdx, int categoryCode)
{
    int ec = m_tcpServer->ResponseToHandler(command, subStation, chnIdx, categoryCode);
#if 0 // all
    if (categoryCode != 0) {
        // notify all other channels
        foreach (uint _chnIdx, m_vecAllChnIdx) {
            if (_chnIdx == chnIdx)
                continue;
            if (m_mapChnIdx2Context.contains(_chnIdx)) {
                QObject *eventReceiver = m_mapChnIdx2Context[_chnIdx]->EventReceiver;
                QCoreApplication::postEvent(eventReceiver, new SiblingChannelErrorEvent(categoryCode));
            }
            else {
                if (_chnIdx > m_cameraCount - 1) { // forward
                    uint newChnIdx = _chnIdx - m_cameraCount;
                    m_tcpClient->SendCommand(command, subStation, newChnIdx, parameter);
                }
            }
        }
    }
#endif
    return ec;
}

void MachineHandler::tcpServer_antiShakeTriggered(int id, int state)
{
    for (auto it = m_mapChnIdx2Context.begin(); it != m_mapChnIdx2Context.end(); ++it)
    {
        QCoreApplication::postEvent(it.value()->EventReceiver, new MachineAntiShakeEvent(id, state));
    }
}

void MachineHandler::tcpServer_commandReceived(uint command, uint subStation, uint chnIdx, const QString &parameter)
{
    if (!m_vecAllChnIdx.contains(chnIdx)) // record all channels
        m_vecAllChnIdx.append(chnIdx);
    if (m_mapChnIdx2Context.contains(chnIdx)) {
        QObject *eventReceiver = m_mapChnIdx2Context[chnIdx]->EventReceiver;
        QCoreApplication::postEvent(eventReceiver, new MachineRequestEvent(command, subStation, chnIdx, parameter));
    }
    else {
        if (chnIdx > m_cameraCount - 1) { // forward
            uint newChnIdx = chnIdx - m_cameraCount;
            m_tcpClient->SendCommand(command, subStation, newChnIdx, parameter);
        }
    }
}

void MachineHandler::tcpClient_responseReceived(uint command, uint subStation, uint chnIdx, uint ec)
{
    uint realChnIdx = chnIdx + m_cameraCount;
    m_tcpServer->ResponseToHandler(command, subStation, realChnIdx, ec);
}
