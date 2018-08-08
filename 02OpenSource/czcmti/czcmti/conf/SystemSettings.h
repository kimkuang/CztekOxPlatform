#ifndef __SYSTEMSETTINGS_H__
#define __SYSTEMSETTINGS_H__

#include "ISettings.h"
#include <QString>
#include <QColor>
#include <QVector>
#include "czcmtidefs.h"
#include "UiDefs.h"

class SystemSettings : public ISettings<SystemSettings>
{
public:
    static SystemSettings *GetInstance();

    // Project
    QString ProjectName;
    QString MachineName;
    // system
    int WorkMode;
    int Operator;
    bool ShowLoginDialog;
    uint HardwareChannelCount;
    int CameraNumber;
    int DispCntPerRow;
    QList<int> DispChnIdxList;
    int AntiShakeDelay;
    int AlarmMsgThreshold;
    int StopTestingThreshold;
    uint SensorSelected; // bit ORed map
    bool ShowStatusBar;
    bool ShowToolBar;
    bool ShowTextPanel;
    bool ShowTestItemPanel;
    bool ShowTestResultPanel;
    
    //module.ini md5
    QVector<QString> vtMd5MoudleInI;
    QString md5SystemSection;
    // sensor
    struct T_SensorParam {
        E_InterfaceType InterfaceType;
        bool AutoSearch;
    };
    T_SensorParam SensorParam;

    // Middleware
    struct T_MiddlewareParam {
        // mes
        bool bMergeMesData;
        int iMergeMesChnIdx;
        // RPC
        QString RpcServerIp;
        int RpcServerPort;

        //Report Path
        bool bSaveData;
        QString ReportPath;
    } Middleware;

    struct T_IpcParam {
        bool Enabled;
        uint ListenPort;
        uint CurrentDeviceIdx; // 0,1
        QString Dev0Ip;
        uint Dev0ChnCnt;
        QString Dev1Ip;
        uint Dev1ChnCnt;
    } Ipc;

    struct T_ShareDirParam {
        QString ServerIP;
        QString UserName;
        QString Password;
        QString SourceDir;
        QString MountPath;
    };
    T_ShareDirParam ShareDir;

    // Misc
    uint CheckSum;
public:
    bool WriteSettings();
    bool ReadSettings();
    void InitSettings();

private:
    friend class ISettings<SystemSettings>;
    static QString SYS_PARAM_FILE_NAME;

    SystemSettings();
    SystemSettings(const std::string &fileName);
    SystemSettings(const SystemSettings &) = delete;
    SystemSettings &operator=(const SystemSettings &) = delete;
    ~SystemSettings();

    uint calcCheckSum();
    QString intList2String(const QList<int> &intList);
    QList<int> string2IntList(const QString &strTemp);
};

#endif // __SYSTEMSETTINGS_H__
