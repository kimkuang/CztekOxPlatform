#include "ModuleSettings.h"

QString ModuleSettings::MODULE_FILE_NAME_TEMPLATE = "module%d.ini";
ModuleSettings *ModuleSettings::GetInstance(const QString &projectName, uint windowIdx)
{
    QString fullPathName = GlobalVars::PROJECT_PATH + projectName + "/" +
            QString::asprintf(ModuleSettings::MODULE_FILE_NAME_TEMPLATE.toLatin1().data(), windowIdx);
    return ISettings<ModuleSettings>::GetInstance(fullPathName.toStdString());
}

ModuleSettings::ModuleSettings()
{
}

ModuleSettings::ModuleSettings(const std::string &fileName) :
    ISettings<ModuleSettings>(fileName)
{}

ModuleSettings::~ModuleSettings()
{}

bool ModuleSettings::WriteSettings()
{
    m_iniFile->WriteString("sensorDb", "schemeName", SensorDb.SchemeName.toStdString());
    m_iniFile->WriteBool("sensorDb", "useMasterI2c", SensorDb.UseMasterI2c);
    QMapIterator<QString, QString> itDbParam(SensorDb.DbParamMap);
    while (itDbParam.hasNext()) {
        itDbParam.next();
        m_iniFile->WriteString("sensorDb", itDbParam.key().toStdString(), itDbParam.value().toStdString());
    }

    m_iniFile->WriteBool("vcmDriverDb", "enabled", VcmDriverDb.Enabled);
    m_iniFile->WriteString("vcmDriverDb", "driverIcName", VcmDriverDb.DriverIcName.toStdString());
    m_iniFile->WriteInteger("vcmDriverDb", "i2cAddr", VcmDriverDb.I2cAddr);

    m_iniFile->WriteBool("otpSensorDb", "enabled", OtpSensorDb.Enabled);
    m_iniFile->WriteString("otpSensorDb", "otpSensorName", OtpSensorDb.OtpSensorName.toStdString());
    m_iniFile->WriteInteger("otpSensorDb", "i2cAddr", OtpSensorDb.I2cAddr);

    m_iniFile->ClearSection("imageTestItem");
    QMapIterator<QString, QString> itTestItem(ImageTestItemMap);
    while (itTestItem.hasNext()) {
        itTestItem.next();
        m_iniFile->WriteString("imageTestItem", itTestItem.key().toStdString(), itTestItem.value().toStdString());
    }

    m_iniFile->ClearSection("hardwareTestItem");
    QMapIterator<QString, QString> itTestItem2(HardwareTestItemMap);
    while (itTestItem2.hasNext()) {
        itTestItem2.next();
        m_iniFile->WriteString("hardwareTestItem", itTestItem2.key().toStdString(), itTestItem2.value().toStdString());
    }

    m_iniFile->WriteString("imageDecoder", "decoder", ImageDecoder.toStdString());

    m_iniFile->ClearSection("subStation");
    QMapIterator<QString, QString> itSubStation(SubStationMap);
    while (itSubStation.hasNext()) {
        itSubStation.next();
        m_iniFile->WriteString("subStation", itSubStation.key().toStdString(), itSubStation.value().toStdString());
    }

    m_iniFile->SaveFile();
    return true;
}

bool ModuleSettings::ReadSettings()
{
    std::vector<IniFile::T_LineConf> vecSection;
    m_iniFile->ReadSection("sensorDb", vecSection);
    for (auto it = vecSection.begin(); it != vecSection.end(); ++it) {
        QString key = QString::fromStdString(it->Key);
        QString val = QString::fromStdString(it->Value);
        if (key == "schemeName")
            SensorDb.SchemeName = val;
        else if (key == "useMasterI2c")
            SensorDb.UseMasterI2c = (val == "true");
        else
            SensorDb.DbParamMap[key] = val;
    }    

    VcmDriverDb.Enabled = m_iniFile->ReadBool("vcmDriverDb", "enabled");
    VcmDriverDb.DriverIcName = QString::fromStdString(m_iniFile->ReadString("vcmDriverDb", "driverIcName"));
    VcmDriverDb.I2cAddr = m_iniFile->ReadInteger("vcmDriverDb", "i2cAddr");

    OtpSensorDb.Enabled = m_iniFile->ReadBool("otpSensorDb", "enabled");
    OtpSensorDb.OtpSensorName = QString::fromStdString(m_iniFile->ReadString("otpSensorDb", "otpSensorName"));
    OtpSensorDb.I2cAddr = m_iniFile->ReadInteger("otpSensorDb", "i2cAddr");

    m_iniFile->ReadSection("imageTestItem", vecSection);
    for (auto it = vecSection.begin(); it != vecSection.end(); ++it) {
        QString key = QString::fromStdString(it->Key);
        QString val = QString::fromStdString(it->Value);
        if (key.startsWith("testItem"))
            ImageTestItemMap[key] = val;
    }

    m_iniFile->ReadSection("hardwareTestItem", vecSection);
    for (auto it = vecSection.begin(); it != vecSection.end(); ++it) {
        QString key = QString::fromStdString(it->Key);
        QString val = QString::fromStdString(it->Value);
        if (key.startsWith("testItem"))
            HardwareTestItemMap[key] = val;
    }

    ImageDecoder = QString::fromStdString(m_iniFile->ReadString("imageDecoder", "decoder", "libDecodeToRgb24.so"));

    m_iniFile->ReadSection("subStation", vecSection);
    for (auto it = vecSection.begin(); it != vecSection.end(); ++it) {
        QString key = QString::fromStdString(it->Key);
        QString val = QString::fromStdString(it->Value);
        if (key.startsWith("subStation"))
            SubStationMap[key] = val;
    }

    return true;
}

void ModuleSettings::InitSettings()
{
    SensorDb.UseMasterI2c = false;
    SensorDb.SchemeName = "";
    SensorDb.DbParamMap.clear();

    VcmDriverDb.Enabled = false;
    VcmDriverDb.DriverIcName = "";
    VcmDriverDb.I2cAddr = 0;

    OtpSensorDb.Enabled = false;
    OtpSensorDb.OtpSensorName = "";
    OtpSensorDb.I2cAddr = 0;

    ImageTestItemMap.clear();
    HardwareTestItemMap.clear();
    ImageDecoder = "";
    SubStationMap.clear();
}
