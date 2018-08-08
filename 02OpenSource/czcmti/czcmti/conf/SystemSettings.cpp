#include "SystemSettings.h"
#include <QSettings>
#include "common/Global.h"
#include <QDebug>
#include <QCryptographicHash>
#include <QFileInfo>
#include "controller/ChannelController.h"

QString SystemSettings::SYS_PARAM_FILE_NAME = "czcmti.ini";

SystemSettings *SystemSettings::GetInstance()
{
    return ISettings<SystemSettings>::GetInstance((GlobalVars::APP_PATH + SYS_PARAM_FILE_NAME).toStdString());
}

SystemSettings::SystemSettings()
{
}

SystemSettings::SystemSettings(const std::string &fileName) :
    ISettings<SystemSettings>(fileName)
{
}

SystemSettings::~SystemSettings()
{
}

bool SystemSettings::WriteSettings()
{
    m_iniFile->WriteString("project", "projectName", ProjectName.toStdString());
    m_iniFile->WriteString("project", "machineName", MachineName.toStdString());

    m_iniFile->WriteInteger("system", "workMode", WorkMode);
    m_iniFile->WriteInteger("system", "operator", Operator);
    m_iniFile->WriteBool("system", "showLoginDialog", ShowLoginDialog);
    m_iniFile->WriteInteger("system", "cameraNumber", CameraNumber);
    m_iniFile->WriteInteger("system", "dispCntPerRow", DispCntPerRow);
    m_iniFile->WriteString("system", "dispChnIdxList", intList2String(DispChnIdxList).toStdString());
    m_iniFile->WriteInteger("system", "antiShakeDelay", AntiShakeDelay);
    m_iniFile->WriteInteger("system", "alarmMsgThreshold", AlarmMsgThreshold);
    m_iniFile->WriteInteger("system", "stopTestingThreshold", StopTestingThreshold);
    m_iniFile->WriteInteger("system", "sensorSelected", SensorSelected);
    m_iniFile->WriteBool("system", "showStatusBar", ShowStatusBar);
    m_iniFile->WriteBool("system", "showToolBar", ShowToolBar);
    m_iniFile->WriteBool("system", "showTextPanel", ShowTextPanel);
    m_iniFile->WriteBool("system", "showTestItemPanel", ShowTestItemPanel);
    m_iniFile->WriteBool("system", "showTestResultPanel", ShowTestResultPanel);

    m_iniFile->WriteBool("middleware", "bMergeMesData", Middleware.bMergeMesData);
    m_iniFile->WriteInteger("middleware", "iMergeMesChnIdx", Middleware.iMergeMesChnIdx);
    m_iniFile->WriteString("middleware", "rpcServerIp", Middleware.RpcServerIp.toStdString());
    m_iniFile->WriteInteger("middleware", "rpcServerPort", Middleware.RpcServerPort);
    m_iniFile->WriteString("middleware", "reportPath", Middleware.ReportPath.toStdString());
    m_iniFile->WriteInteger("middleware", "bSaveData", Middleware.bSaveData);

    m_iniFile->WriteBool("ipc", "enabled", Ipc.Enabled);
    m_iniFile->WriteInteger("ipc", "listenPort", Ipc.ListenPort);
    m_iniFile->WriteInteger("ipc", "currentDeviceIdx", Ipc.CurrentDeviceIdx);
    m_iniFile->WriteString("ipc", "dev0Ip", Ipc.Dev0Ip.toStdString());
    m_iniFile->WriteInteger("ipc", "dev0ChnCnt", Ipc.Dev0ChnCnt);
    m_iniFile->WriteString("ipc", "dev1Ip", Ipc.Dev1Ip.toStdString());
    m_iniFile->WriteInteger("ipc", "dev1ChnCnt", Ipc.Dev1ChnCnt);

    m_iniFile->WriteString("shareDir", "serverIP", ShareDir.ServerIP.toStdString());
    m_iniFile->WriteString("shareDir", "userName", ShareDir.UserName.toStdString());
    m_iniFile->WriteString("shareDir", "password", ShareDir.Password.toStdString());
    m_iniFile->WriteString("shareDir", "sourceDir", ShareDir.SourceDir.toStdString());
    m_iniFile->WriteString("shareDir", "mountPath", ShareDir.MountPath.toStdString());
    for (int i = 0; i < CameraNumber; ++i)
    {
        m_iniFile->WriteString("MD5", QString("Module%1InI").arg(i).toStdString(), vtMd5MoudleInI[i].toStdString());
    }

    m_iniFile->WriteString("MD5", "SystemSection", md5SystemSection.toStdString());
    m_iniFile->SaveFile();
	return true;
}

bool SystemSettings::ReadSettings()
{
    ProjectName = QString::fromStdString(m_iniFile->ReadString("project", "projectName"));
    if (ProjectName.isEmpty())
        ProjectName = "default";
    MachineName = QString::fromStdString(m_iniFile->ReadString("project", "machineName", "libMachine.so"));

    WorkMode = m_iniFile->ReadInteger("system", "workMode", (int)UiDefs::WorkMode_Manual);
    Operator = m_iniFile->ReadInteger("system", "operator", (int)UiDefs::Operator_Engineer);
    ShowLoginDialog = m_iniFile->ReadBool("system", "showLoginDialog");
    CameraNumber = m_iniFile->ReadInteger("system", "cameraNumber", 1);
    DispCntPerRow = m_iniFile->ReadInteger("system", "dispCntPerRow", 2);
    if (DispCntPerRow > CameraNumber)
        DispCntPerRow = CameraNumber;
    QString strTemp = QString::fromStdString(m_iniFile->ReadString("system", "dispChnIdxList"));
    DispChnIdxList = string2IntList(strTemp);
    if (DispChnIdxList.size() < CameraNumber) {
        DispChnIdxList.clear();
        for (int i = 0; i < CameraNumber; i++)
            DispChnIdxList.push_back(i);
    }
    AntiShakeDelay = m_iniFile->ReadInteger("system", "antiShakeDelay", 3000);
    AlarmMsgThreshold = m_iniFile->ReadInteger("system", "alarmMsgThreshold", 2);
    StopTestingThreshold = m_iniFile->ReadInteger("system", "stopTestingThreshold", 1);
    SensorSelected = m_iniFile->ReadInteger("system", "sensorSelected", 1);
    ShowStatusBar = m_iniFile->ReadBool("system", "showStatusBar", true);
    ShowToolBar = m_iniFile->ReadBool("system", "showToolBar");
    ShowTextPanel = m_iniFile->ReadBool("system", "showTextPanel", true);
    ShowTestItemPanel = m_iniFile->ReadBool("system", "showTestItemPanel", true);
    ShowTestResultPanel = m_iniFile->ReadBool("system", "showTestResultPanel", true);

    Middleware.bMergeMesData = m_iniFile->ReadBool("middleware", "bMergeMesData");
    Middleware.iMergeMesChnIdx = m_iniFile->ReadInteger("middleware", "iMergeMesChnIdx", 0);
    Middleware.RpcServerIp = QString::fromStdString(m_iniFile->ReadString("middleware", "rpcServerIp"));
    Middleware.RpcServerPort = m_iniFile->ReadInteger("middleware", "rpcServerPort", 8080);
    Middleware.bSaveData = m_iniFile->ReadInteger("middleware", "bSaveData", 1);
    Middleware.ReportPath = QString::fromStdString(m_iniFile->ReadString("middleware", "reportPath"));
    if (Middleware.ReportPath.isEmpty())
        Middleware.ReportPath = "/mnt/windows";

    Ipc.Enabled = m_iniFile->ReadBool("ipc", "enabled");
    Ipc.ListenPort = m_iniFile->ReadInteger("ipc", "listenPort", 5100);
    Ipc.CurrentDeviceIdx = m_iniFile->ReadInteger("ipc", "currentDeviceIdx", 0);
    Ipc.Dev0Ip = QString::fromStdString(m_iniFile->ReadString("ipc", "dev0Ip"));
    Ipc.Dev0ChnCnt = m_iniFile->ReadInteger("ipc", "dev0ChnCnt", 2);
    Ipc.Dev1Ip = QString::fromStdString(m_iniFile->ReadString("ipc", "dev1Ip"));
    Ipc.Dev1ChnCnt = m_iniFile->ReadInteger("ipc", "dev1ChnCnt", 1);

    ShareDir.ServerIP = QString::fromStdString(m_iniFile->ReadString("shareDir", "serverIP"));
    ShareDir.UserName = QString::fromStdString(m_iniFile->ReadString("shareDir", "userName"));
    ShareDir.Password = QString::fromStdString(m_iniFile->ReadString("shareDir", "password"));
    ShareDir.SourceDir = QString::fromStdString(m_iniFile->ReadString("shareDir", "sourceDir"));
    ShareDir.MountPath = QString::fromStdString(m_iniFile->ReadString("shareDir", "mountPath"));

    for (int i = 0; i < CameraNumber; ++i)
    {
        QString md5ModuleInI = QString::fromStdString(m_iniFile->ReadString("MD5", QString("Module%1InI").arg(i).toStdString()));
        vtMd5MoudleInI.push_back(md5ModuleInI);
    }

    QString systemSection = "";
    QFileInfo sysSettingInfo(GlobalVars::APP_PATH + SYS_PARAM_FILE_NAME);
    if (false == sysSettingInfo.exists())
    {
        systemSection.sprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", WorkMode, Operator, ShowLoginDialog, CameraNumber, AntiShakeDelay,
                            AlarmMsgThreshold, StopTestingThreshold, SensorSelected, ShowStatusBar, ShowToolBar, ShowTextPanel,
                            ShowTestItemPanel, ShowTestResultPanel);
    }

    QByteArray btSysSection = QCryptographicHash::hash(systemSection.toLatin1(), QCryptographicHash::Md5).toHex();
    md5SystemSection = QString::fromStdString(m_iniFile->ReadString("MD5", "SystemSection", btSysSection.toStdString()));
    return true;
}

void SystemSettings::InitSettings()
{
    // project
    ProjectName = "default";
    MachineName = "";

    // system
    WorkMode = (int)UiDefs::WorkMode_Manual;
    Operator = (int)UiDefs::Operator_Engineer;
    ShowLoginDialog = true;
    HardwareChannelCount = ChannelController::GetInstance(0)->GetBarcode().mid(1, 1).toUInt();
    CameraNumber = 1;
    DispCntPerRow = 2;
    DispChnIdxList.clear();
    DispChnIdxList << 0 << 1;
    AntiShakeDelay = 3000;
    AlarmMsgThreshold = 2;
    StopTestingThreshold = 1;
    SensorSelected = 1;
    ShowStatusBar = true;
    ShowToolBar = false;
    ShowTextPanel = true;
    ShowTestItemPanel = true;
    ShowTestResultPanel = true;

    // sensor
    SensorParam.InterfaceType = IT_MIPI;
    SensorParam.AutoSearch = true;

    // middleware
    Middleware.bMergeMesData = true;
    Middleware.iMergeMesChnIdx = 0;
    Middleware.RpcServerIp = "";
    Middleware.RpcServerPort = 8080;

    Ipc.Enabled = false;
    Ipc.ListenPort = 5001;
    Ipc.CurrentDeviceIdx = 0;
    Ipc.Dev0Ip = "";
    Ipc.Dev0ChnCnt = 2;
    Ipc.Dev1Ip = "";
    Ipc.Dev1ChnCnt = 1;

    ShareDir.ServerIP = "";
    ShareDir.UserName = "Administrator";
    ShareDir.Password = "";
    ShareDir.SourceDir = "";
    ShareDir.MountPath = "";

    vtMd5MoudleInI.clear();
    // misc
    CheckSum = calcCheckSum();
}

uint SystemSettings::calcCheckSum()
{
    uint cs = 0;
    // sensor
    cs += SensorParam.InterfaceType;

    return cs;
}

QString SystemSettings::intList2String(const QList<int> &intList)
{
    QStringList slstTemp;
    for (int i = 0; i < intList.size(); i++)
        slstTemp.push_back(QString::number(DispChnIdxList[i]));
    return slstTemp.join(QChar(','));
}

QList<int> SystemSettings::string2IntList(const QString &strTemp)
{
    QStringList slstTemp = strTemp.split(QChar(','), QString::SkipEmptyParts);
    QList<int> intList;
    foreach (const QString &str, slstTemp) {
        intList.push_back(str.toInt());
    }
    return intList;
}
