#include "MachineService.h"
#include <QDebug>
#include "common/Global.h"
#include "IChannelController.h"
#include "PluginLibPool.h"

std::mutex MachineService::m_mutex;
MachineService *MachineService::m_machineService = nullptr;
MachineService *MachineService::GetInstance()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    (void)(locker);
    if (m_machineService == nullptr)
        m_machineService = new MachineService();
    return m_machineService;
}

void MachineService::FreeInstance()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    (void)(locker);
    if (m_machineService != nullptr) {
        delete m_machineService;
        m_machineService = nullptr;
    }
}

MachineService::MachineService()
{
}

MachineService::~MachineService()
{
    FreeAllPluginInstances();
}

//void MachineService::BindChannelContext(T_ChannelContext *context)
//{
//    IProtocolHandler *instance = GetPluginInstance(libFileName);
//    if (instance != nullptr) {
//        qDebug()<<context->ChnIdx;
//        instance->BindChannelContext(context); // !!!IMPORTANT!!!
//    }
//}

int MachineService::GetPluginInfo(const QString &libFileName, T_PluginInfo &pluginInfo)
{
    IProtocolHandler *instance = GetPluginInstance(libFileName);
    if (instance != nullptr) {
        return instance->GetPluginInfo(pluginInfo);
    }
    return ERR_Failed;
}

IProtocolHandler *MachineService::GetPluginInstance(const QString &libFileName)
{
    IProtocolHandler *instance = nullptr;
    if (!m_mapName2Instance.contains(libFileName)) {
        qDebug("Create protocol handler plugin...");
        QString libFullPathName = GlobalVars::PLUGIN_MACHINE_PATH + libFileName;
        instance = static_cast<IProtocolHandler *>(PluginLibPool::GetInstance().NewPlugin(libFullPathName, nullptr));
        if (instance == nullptr) {
            qCritical("Loading machine plugin failed. [pluginName: %s]", libFileName.toStdString().c_str());
            return instance;
        }
        m_mapName2Instance.insert(libFileName, instance);
    }
    instance = m_mapName2Instance[libFileName];
    return instance;
}

void MachineService::FreePluginInstance(const QString &libFileName)
{
    if (m_mapName2Instance.contains(libFileName)) {
        qDebug().noquote() << QString("Delete protocol handler instance...[%1]").arg(libFileName);
        QString libFullPathName = GlobalVars::PLUGIN_MACHINE_PATH + libFileName;
        IProtocolHandler* &instance = m_mapName2Instance[libFileName];
        PluginLibPool::GetInstance().DeletePlugin(libFullPathName, (ICzPlugin **)(&instance));
        m_mapName2Instance.remove(libFileName);
    }
}

void MachineService::FreeAllPluginInstances()
{
    QMutableMapIterator<QString, IProtocolHandler *> it(m_mapName2Instance);
    while (it.hasNext()) {
        it.next();
        qDebug().noquote() << QString("Delete protocol handler plugin...[%1]").arg(it.key());
        QString libFullPathName = GlobalVars::PLUGIN_MACHINE_PATH + it.key();
        IProtocolHandler* &instance = it.value();
        PluginLibPool::GetInstance().DeletePlugin(libFullPathName, (ICzPlugin **)(&instance));
        it.remove();
    }
}

int MachineService::RestoreDefaults(const QString &libFileName)
{
    IProtocolHandler *instance = GetPluginInstance(libFileName);
    if (instance != nullptr) {
        try {
            return instance->RestoreDefaults();
        }
        catch (...) {
            qCritical()<<"Exception in"<<__FUNCTION__;
            return ERR_Failed;
        }
    }
    return ERR_Failed;
}

int MachineService::LoadSettings(const QString &libFileName)
{
    IProtocolHandler *instance = GetPluginInstance(libFileName);
    if (instance != nullptr) {
        try {
            return instance->LoadOption();
        }
        catch (...) {
            return ERR_Failed;
        }
    }
    return ERR_Failed;
}

int MachineService::SaveSettings(const QString &libFileName)
{
    IProtocolHandler *instance = GetPluginInstance(libFileName);
    if (instance != nullptr) {
        try {
            return instance->SaveOption();
        }
        catch (...) {
            qCritical()<<"Exception in"<<__FUNCTION__;
            return ERR_Failed;
        }
    }
    return ERR_Failed;
}

int MachineService::Stop(const QString &libFileName)
{
    IProtocolHandler *instance = GetPluginInstance(libFileName);
    if (instance != nullptr) {
        return instance->Stop();
    }
    return ERR_Failed;
}

int MachineService::Start(const QString &libFileName)
{
    IProtocolHandler *instance = GetPluginInstance(libFileName);
    if (instance != nullptr) {
        return instance->Start();
    }
    return ERR_Failed;
}

int MachineService::ResponseToHandler(const QString &libFileName, int command, int substation, int chnIdx, int ec)
{
    IProtocolHandler *instance = GetPluginInstance(libFileName);
    if (instance != nullptr) {
        try {
            return instance->ResponseToHandler(command, substation, chnIdx, ec);
        }
        catch (...) {
            qCritical()<<"Exception in"<<__FUNCTION__;
            return ERR_Failed;
        }
    }
    return ERR_Failed;
}
