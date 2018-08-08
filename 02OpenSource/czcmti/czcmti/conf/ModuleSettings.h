#ifndef __MODULESETTINGS_H__
#define __MODULESETTINGS_H__
#include "ISettings.h"
#include "UiDefs.h"

class ModuleSettings : public ISettings<ModuleSettings>
{
public:
    static QString MODULE_FILE_NAME_TEMPLATE;
    static ModuleSettings *GetInstance(const QString &projectName, uint windowIdx); // Notice: window index

    struct T_SensorDb {
        bool UseMasterI2c;
        QString SchemeName;
        QMap<QString, QString> DbParamMap;        
    } SensorDb;
    struct T_VcmDriverDb {
        bool Enabled;
        QString DriverIcName;
        uint I2cAddr;
    } VcmDriverDb;
    struct T_OtpSensorDb {
        bool Enabled;
        QString OtpSensorName;
        uint I2cAddr;
    } OtpSensorDb;
    QMap<QString, QString> ImageTestItemMap;    // testItem01=GroupIdx,InstanceName@FileName,ErrorCode
    QMap<QString, QString> HardwareTestItemMap; // testItem01=InstanceName@FileName
    QString ImageDecoder;                       // decoder=libFileName
    QMap<QString, QString> SubStationMap;       // subStation01=InstanceName@FileName,InstanceName@FileName
public:
    bool WriteSettings();
    bool ReadSettings();
    void InitSettings();

private:
    friend class ISettings<ModuleSettings>;
    ModuleSettings();
    ModuleSettings(const std::string &fileName);
    ModuleSettings(const ModuleSettings&) = delete;
    ModuleSettings &operator=(const ModuleSettings&) = delete;
    ~ModuleSettings();
};

#endif // __MODULESETTINGS_H__
