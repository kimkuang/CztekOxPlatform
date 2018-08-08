#include "PluginLibPool.h"
#include <QFileInfo>
#include <QDebug>

std::mutex PluginLibPool::m_mutex;
PluginLibPool PluginLibPool::m_pluginLibPool;
PluginLibPool::PluginLibPool()
{
}

PluginLibPool::~PluginLibPool()
{
    QList<QString> allLibNames = m_mapName2Plugin.keys();
    foreach (QString libName, allLibNames)
        UnloadPluginLibrary(libName);
    m_mapName2Plugin.clear();
}

PluginLibPool &PluginLibPool::GetInstance()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    (void)(locker);
    return m_pluginLibPool;
}

int PluginLibPool::LoadPluginLibrary(const QString &libFullPathName)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    (void)(locker);

    if (!m_mapName2Plugin.contains(libFullPathName)) {
        QFileInfo fileInfo(libFullPathName);
        qDebug().noquote() << fileInfo.absolutePath() << fileInfo.baseName();
        T_PluginParam pluginParam;
        pluginParam.LibFullPathName = libFullPathName;
        pluginParam.PLibraryLoader = new LibraryLoader(fileInfo.absolutePath().toStdString());
        if (!pluginParam.PLibraryLoader->Load(fileInfo.baseName().toStdString())) {
            qCritical().noquote() << QString("Loading library[%1] failed.").arg(libFullPathName);
            delete pluginParam.PLibraryLoader; pluginParam.PLibraryLoader = nullptr;
            return ERR_Failed;
        }
        pluginParam.PFunCreate = (PFuncCreatePlugin)pluginParam.PLibraryLoader->GetSymbolAddress("CreatePlugin");
        if (pluginParam.PFunCreate == nullptr) {
            qCritical("Calling GetSymbolAddress(\"CreatePlugin\") failed.");
            return ERR_Failed;
        }
        pluginParam.PFunDestroy = (PFuncDestroyPlugin)pluginParam.PLibraryLoader->GetSymbolAddress("DestroyPlugin");
        if (pluginParam.PFunDestroy == nullptr) {
            qCritical("Calling GetSymbolAddress(\"DestroyPlugin\") failed.");
            return ERR_Failed;
        }
        m_mapName2Plugin.insert(libFullPathName, pluginParam);
    }
    return ERR_NoError;
}

int PluginLibPool::UnloadPluginLibrary(const QString &libFullPathName)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    (void)(locker);
    if (m_mapName2Plugin.contains(libFullPathName)) {
        T_PluginParam &pluginParam = m_mapName2Plugin[libFullPathName];
        if (pluginParam.PLibraryLoader != nullptr) {
            pluginParam.PLibraryLoader->Unload();
            delete pluginParam.PLibraryLoader;
            pluginParam.PLibraryLoader = nullptr;
        }
    }
    return ERR_NoError;
}

ICzPlugin *PluginLibPool::NewPlugin(const QString &libFullPathName, void *arg)
{
    if (LoadPluginLibrary(libFullPathName) != ERR_NoError)
        return nullptr;
    return m_mapName2Plugin[libFullPathName].PFunCreate(arg);
}

void PluginLibPool::DeletePlugin(const QString &libFullPathName, ICzPlugin **plugin)
{
    m_mapName2Plugin[libFullPathName].PFunDestroy(plugin);
}
