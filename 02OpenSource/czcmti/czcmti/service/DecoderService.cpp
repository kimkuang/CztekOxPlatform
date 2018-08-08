#include "DecoderService.h"
#include <QDebug>
#include "common/Global.h"
#include "PluginLibPool.h"

std::mutex DecoderService::m_mutex;
QMap<uint, DecoderService *> DecoderService::m_mapChnIdx2DecoderService;
DecoderService *DecoderService::GetInstance(uint chnIdx)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    (void)(locker);
    DecoderService *instance = nullptr;
    if (!m_mapChnIdx2DecoderService.contains(chnIdx)) {
        qDebug("New decoder service...[%d]", chnIdx);
        instance = new DecoderService(chnIdx);
        m_mapChnIdx2DecoderService.insert(chnIdx, instance);
    }
    else {
        instance = m_mapChnIdx2DecoderService[chnIdx];
    }
    return instance;
}

void DecoderService::FreeInstance(uint chnIdx)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    (void)(locker);
    if (m_mapChnIdx2DecoderService.contains(chnIdx)) {
        delete m_mapChnIdx2DecoderService[chnIdx];
        m_mapChnIdx2DecoderService.remove(chnIdx);
    }
}

void DecoderService::FreeAllInstances()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    (void)(locker);
    QMutableMapIterator<uint, DecoderService*> it(m_mapChnIdx2DecoderService);
    while (it.hasNext()) {
        it.next();
        delete it.value();
        it.remove();
    }
}

DecoderService::DecoderService(uint chnIdx)
{
    m_chnIdx = chnIdx;
}

DecoderService::~DecoderService()
{
    FreeAllPluginInstances();
}

int DecoderService::GetPluginInfo(const QString &libFileName, T_PluginInfo &pluginInfo)
{
    ISensorImageDecoder *instance = GetPluginInstance(libFileName);
    if (instance != nullptr) {
        return instance->GetPluginInfo(pluginInfo);
    }
    return ERR_Failed;
}

ISensorImageDecoder *DecoderService::GetPluginInstance(const QString &libFileName)
{
    ISensorImageDecoder *instance = nullptr;
    if (!m_mapName2Instance.contains(libFileName)) {
        qDebug("Create decoder plugin...[chnIdx: %d]", m_chnIdx);
        QString libFullPathName = GlobalVars::PLUGIN_IMAGEDECODER_PATH + libFileName;
        instance = static_cast<ISensorImageDecoder *>(PluginLibPool::GetInstance().NewPlugin(libFullPathName, m_eventReceiver));
        if (instance == nullptr) {
            qCritical().noquote() << QString("Create decoder instance failed. [pluginName: %1]").arg(libFileName);
            return instance;
        }
        m_mapName2Instance.insert(libFileName, instance);
    }
    instance = m_mapName2Instance[libFileName];
    return instance;
}

void DecoderService::FreePluginInstance(const QString &libFileName)
{
    if (m_mapName2Instance.contains(libFileName)) {
        qDebug().noquote() << QString("Delete decoder instance...[%1]").arg(libFileName);
        QString libFullPathName = GlobalVars::PLUGIN_IMAGEDECODER_PATH + libFileName;
        ISensorImageDecoder* &instance = m_mapName2Instance[libFileName];
        PluginLibPool::GetInstance().DeletePlugin(libFullPathName, (ICzPlugin **)(&instance));
        m_mapName2Instance.remove(libFileName);
    }
}

void DecoderService::FreeAllPluginInstances()
{
    QMutableMapIterator<QString, ISensorImageDecoder *> it(m_mapName2Instance);
    while (it.hasNext()) {
        it.next();
        qDebug().noquote() << QString("Delete decoder instance...[%1]").arg(it.key());
        QString libFullPathName = GlobalVars::PLUGIN_IMAGEDECODER_PATH + it.key();
        ISensorImageDecoder* &instance = it.value();
        PluginLibPool::GetInstance().DeletePlugin(libFullPathName, (ICzPlugin **)(&instance));
        it.remove();
    }
}

void DecoderService::SetCurrentDecoderName(const QString &libFileName)
{
    m_currentDecoderName = libFileName;
}

ISensorImageDecoder *DecoderService::GetCurrentDecoder()
{
    return GetPluginInstance(m_currentDecoderName);
}

int DecoderService::RestoreDefaults(const QString &libFileName)
{
    ISensorImageDecoder *instance = GetPluginInstance(libFileName);
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

int DecoderService::LoadSettings(const QString &libFileName)
{
    ISensorImageDecoder *instance = GetPluginInstance(libFileName);
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

int DecoderService::SaveSettings(const QString &libFileName)
{
    ISensorImageDecoder *instance = GetPluginInstance(libFileName);
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
