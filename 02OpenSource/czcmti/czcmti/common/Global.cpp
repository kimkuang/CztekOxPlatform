#include "Global.h"
#include <QDir>
#include <QLocale>

QString GlobalVars::UI_INI_FILE_NAME = "ui.ini";
QString GlobalVars::PATCH_INI_FILE_NAME = "patch.ini";
QString GlobalVars::EXEC_SH_FILE_NAME = "exec.sh";
QString GlobalVars::APP_PATH = "";
QString GlobalVars::LOG_PATH = "";
QString GlobalVars::PIC_PATH = "";
QString GlobalVars::MOV_PATH = "";
QString GlobalVars::DRV_PATH = "";
QString GlobalVars::PATCH_PATH = "";
QString GlobalVars::DAT_PATH = "";
QString GlobalVars::FIRMWARE_PATH = "";
QString GlobalVars::UPGRADE_PATH = "";
QString GlobalVars::PLUGIN_IMAGEDECODER_PATH = "";
QString GlobalVars::PLUGIN_TESTITEM_PATH = "";
QString GlobalVars::PLUGIN_MACHINE_PATH = "";
QString GlobalVars::PLUGIN_OTP_PATH = "";
QString GlobalVars::PROJECT_PATH = "";
ProductBarcode::T_Barcode GlobalVars::ThisProductBarcode;
ProductBarcode::ProductFeatures GlobalVars::ThisProductFeatures;
uint GlobalVars::MAX_SUPPORTED_CHANNEL_COUNT;
QMap<QString, QString> GlobalVars::Name2VersionMap;

void GlobalVars::InitVars()
{
    UI_INI_FILE_NAME = "ui.ini";
    PATCH_INI_FILE_NAME = "patch.ini";
    APP_PATH = qApp->applicationDirPath() + "/";
    EXEC_SH_FILE_NAME = APP_PATH + "exec.sh";
    LOG_PATH = APP_PATH + "log/";
    PIC_PATH = APP_PATH + "pic/";
    MOV_PATH = APP_PATH + "mov/";
    DRV_PATH = APP_PATH + "driver/";
    PATCH_PATH = APP_PATH + "patch/";
    DAT_PATH = APP_PATH + "dat/";
    FIRMWARE_PATH = APP_PATH + "firmware/";
    UPGRADE_PATH = APP_PATH + "upgrade/";
    PLUGIN_IMAGEDECODER_PATH = APP_PATH + "plugins/decoder/";
    PLUGIN_TESTITEM_PATH = APP_PATH + "plugins/testitem/";
    PLUGIN_MACHINE_PATH = APP_PATH + "plugins/machine/";
    PLUGIN_OTP_PATH = APP_PATH + "plugins/otp/";
    PROJECT_PATH = APP_PATH + "project/";
    MAX_SUPPORTED_CHANNEL_COUNT = 2;

    ThisProductFeatures = ProductBarcode::PF_None;    
}

void GlobalVars::InitApplicationDirectory()
{
    QDir::setCurrent(GlobalVars::APP_PATH);
    QString pathName = GlobalVars::LOG_PATH;
    QDir dir(pathName);
    if (!dir.exists()) {
        dir.mkdir(pathName);
    }
    pathName = GlobalVars::PIC_PATH;
    if (!dir.exists(pathName)) {
        dir.mkdir(pathName);
    }
    pathName = GlobalVars::MOV_PATH;
    if (!dir.exists(pathName)) {
        dir.mkdir(pathName);
    }
    pathName = GlobalVars::DAT_PATH;
    if (!dir.exists(pathName)) {
        dir.mkdir(pathName);
    }
    pathName = GlobalVars::UPGRADE_PATH;
    if (!dir.exists(pathName)) {
        dir.mkdir(pathName);
    }
    pathName = GlobalVars::PLUGIN_IMAGEDECODER_PATH;
    if (!dir.exists(pathName)) {
        dir.mkdir(pathName);
    }
    pathName = GlobalVars::PLUGIN_TESTITEM_PATH;
    if (!dir.exists(pathName)) {
        dir.mkdir(pathName);
    }
    pathName = GlobalVars::PLUGIN_MACHINE_PATH;
    if (!dir.exists(pathName)) {
        dir.mkdir(pathName);
    }
//    pathName = GlobalVars::PLUGIN_OTP_PATH;
//    if (!dir.exists(pathName)) {
//        dir.mkdir(pathName);
//    }
    pathName = GlobalVars::PROJECT_PATH;
    if (!dir.exists(pathName)) {
        dir.mkdir(pathName);
    }
    if (!dir.exists(pathName + "/default/")) {
        dir.mkdir(pathName + "/default/");
    }
}

void GlobalVars::GetProductVersion(QString &version, bool noBuildVer)
{
    QString strTime = QString("%1 %2").arg(__DATE__).arg(__TIME__).simplified(); // Here, LocalTime = UTC Time
    QLocale locale(QLocale::English, QLocale::UnitedStates);
    QDateTime buildTime = locale.toDateTime(strTime, "MMM d yyyy HH:mm:ss");
    buildTime = buildTime.toOffsetFromUtc(8 * 3600); // convert to +8 zone
    QString revision = buildTime.toString("Mdd");
    QString buildNumber = buildTime.toString("Hmmss");//yyMMddHHmm
    if (noBuildVer)
        version = QString("%1.%2.%3").arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(revision);
    else
        version = QString("%1.%2.%3.%4").arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(revision).arg(buildNumber);
}
