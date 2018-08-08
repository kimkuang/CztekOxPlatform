#ifndef DECODERSERVICE_H
#define DECODERSERVICE_H
#include <QMap>
#include <QString>
#include "czcmtidefs.h"
#include "ICzPlugin.h"
#include "CzUtilsDefs.h"

class DecoderService
{
public:
    static DecoderService *GetInstance(uint chnIdx);
    static void FreeInstance(uint chnIdx);
    static void FreeAllInstances();
private:
    static std::mutex m_mutex;
    static QMap<uint, DecoderService *> m_mapChnIdx2DecoderService;

    DecoderService(uint chnIdx);
    virtual ~DecoderService();
    DecoderService() = delete;
    DecoderService(const DecoderService &) = delete;
    DecoderService &operator=(const DecoderService &) = delete;

public:
    int GetPluginInfo(const QString &libFileName, T_PluginInfo &pluginInfo);
    ISensorImageDecoder *GetPluginInstance(const QString &libFileName);
    void FreePluginInstance(const QString &libFileName);
    void FreeAllPluginInstances();
    void SetCurrentDecoderName(const QString &libFileName);
    ISensorImageDecoder *GetCurrentDecoder();

    int RestoreDefaults(const QString &libFileName);
    int LoadSettings(const QString &libFileName);
    int SaveSettings(const QString &libFileName);

private:
    uint m_chnIdx;
    QObject *m_eventReceiver;
    QMap<QString, ISensorImageDecoder *> m_mapName2Instance; // <Library file name, Plugin Instance>
    QString m_currentDecoderName;
};

#endif // DECODERSERVICE_H
