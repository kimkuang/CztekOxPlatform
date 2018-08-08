#ifndef GLOBAL_H
#define GLOBAL_H
#include <QString>
#include <QCoreApplication>
#include "ProductBarcode.h"

/**
 * Version Convention
 * Major_Version_Number.Minor_Version_Number[.Revision_Number[.Build_Number]]
 * Ex: V1.0.001.180728145055
*/
#define MAJOR_VERSION       1           //1~9
#define MINOR_VERSION       0           //0~999
#define REVISION_VERSION    003         //001 ~ 999, clear zero by major or minor

class GlobalVars
{
public:
    static QString UI_INI_FILE_NAME;
    static QString PATCH_INI_FILE_NAME;
    static QString EXEC_SH_FILE_NAME;
    static QString APP_PATH;
    static QString LOG_PATH;
    static QString PIC_PATH;
    static QString MOV_PATH;
    static QString DRV_PATH;
    static QString PATCH_PATH;
    static QString DAT_PATH;
    static QString FIRMWARE_PATH;
    static QString UPGRADE_PATH;
    static QString PLUGIN_IMAGEDECODER_PATH;
    static QString PLUGIN_TESTITEM_PATH;
    static QString PLUGIN_MACHINE_PATH;
    static QString PLUGIN_OTP_PATH;
    static QString PROJECT_PATH;
    static ProductBarcode::T_Barcode ThisProductBarcode;
    static ProductBarcode::ProductFeatures ThisProductFeatures;
    static uint MAX_SUPPORTED_CHANNEL_COUNT;
    static QMap<QString, QString> Name2VersionMap;

    static void InitVars();
    static void InitApplicationDirectory();
    static void GetProductVersion(QString &version, bool noBuildVer = false);
};

#endif // GLOBAL_H
