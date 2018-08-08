#ifndef PLUGINLIBPOOL_H
#define PLUGINLIBPOOL_H
#include <QString>
#include <QMap>
#include <mutex>
#include "utils/LibraryLoader.h"
#include "ICzPlugin.h"

// all the plugin's pool
class PluginLibPool
{
public:
    static PluginLibPool &GetInstance();

    int LoadPluginLibrary(const QString &libFullPathName);
    int UnloadPluginLibrary(const QString &libFullPathName);
    ICzPlugin *NewPlugin(const QString &libFullPathName, void *arg);
    void DeletePlugin(const QString &libFullPathName, ICzPlugin** plugin);

private:
    static std::mutex m_mutex;
    static PluginLibPool m_pluginLibPool;
    PluginLibPool();
    ~PluginLibPool();
    PluginLibPool(const PluginLibPool &) = delete;
    PluginLibPool &operator=(const PluginLibPool &) = delete;

private:
    struct T_PluginParam {
        QString LibFullPathName;
        LibraryLoader *PLibraryLoader;
        PFuncCreatePlugin PFunCreate;
        PFuncDestroyPlugin PFunDestroy;
        T_PluginParam() {
            LibFullPathName = "";
            PLibraryLoader = nullptr;
            PFunCreate = nullptr;
            PFunDestroy = nullptr;
        }
    };
    QMap<QString, T_PluginParam> m_mapName2Plugin; // <Library full path name, Plugin Parameter>
};

#endif // PLUGINLIBPOOL_H
