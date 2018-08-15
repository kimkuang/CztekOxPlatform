#include "sensorbll.h"
#include <QDebug>
#include <QCoreApplication>

bool SensorBLL::m_onlyMipi2L = false;
SensorBLL::SensorBLL()
{
    m_appType = 0;
    if (qApp->applicationName().contains("otp", Qt::CaseInsensitive))
        m_appType = 1;
}

SensorBLL::~SensorBLL()
{
}

void SensorBLL::SetOnlyMipi2L(bool val)
{
    m_onlyMipi2L = val;
}

bool SensorBLL::Insert(Sensor &sensor)
{
    int idx = sensor.ChipName.indexOf(QChar('&'));
    if (idx > 0) {
        QString chipNames[2];
        chipNames[0] = sensor.ChipName.mid(0, idx);
        chipNames[1] = sensor.ChipName.mid(idx + 1);
        QString schemeNames[2];
        schemeNames[0] = sensor.SchemeName; schemeNames[0].replace(sensor.ChipName, chipNames[0]);
        schemeNames[1] = sensor.SchemeName; schemeNames[1].replace(sensor.ChipName, chipNames[1]);
        QString vendorNames[2];
        vendorNames[0] = sensor.VendorName;
        vendorNames[1] = sensor.VendorName;
        int idx2 = sensor.VendorName.indexOf(QChar('&'));
        if (idx2 > 0) {
            vendorNames[0] = sensor.VendorName.mid(0, idx2);
            vendorNames[1] = sensor.VendorName.mid(idx2 + 1);
        }
        for (int i = 0; i < 2; i++) {
            sensor.SchemeName = schemeNames[i];
            sensor.ChipName = chipNames[i];
            sensor.VendorName = vendorNames[i];
            if (!insertOrUpdate(sensor))
                return false;
        }
        return true;
    }
    else {
        return insertOrUpdate(sensor);
    }
}

bool SensorBLL::Delete(int id)
{
    return m_sensorDAL.Delete(id);
}

bool SensorBLL::Update(const Sensor &sensor, bool updateDef)
{
    QString whereClause = QString(" where (id=%1)").arg(sensor.Id);

    return m_sensorDAL.Update(sensor, whereClause, updateDef);
}

bool SensorBLL::SelectById(Sensor &sensor)
{
    QString whereClause = QString(" where (app_type=%1)").arg(m_appType);
    if (m_onlyMipi2L)
        whereClause += " and (lanes<=2)";
    whereClause += QString(" and (id=%1)").arg(sensor.Id);

    return m_sensorDAL.Select(sensor, whereClause);
}

bool SensorBLL::SelectBySchemeName(Sensor &sensor)
{
    QString whereClause = QString(" where (app_type=%1)").arg(m_appType);
    if (m_onlyMipi2L)
        whereClause += " and (lanes<=2)";
    whereClause += QString(" and (scheme_name='%1')").arg(sensor.SchemeName);

    return m_sensorDAL.Select(sensor, whereClause);
}

bool SensorBLL::SelectList(QList<Sensor> &sensors)
{
    QString whereClause = QString(" where (app_type=%1)").arg(m_appType);
    if (m_onlyMipi2L)
        whereClause += " and (lanes<=2)";
    return m_sensorDAL.SelectList(sensors, whereClause);
}

/* Refer to SelectList, less db fields, more effective */
bool SensorBLL::SelectList_IdVendorSchemeNameByInterfaceType(QList<Sensor> &sensors, int interfaceType)
{
    QString whereClause = QString(" where (app_type=%1)").arg(m_appType);
    if (m_onlyMipi2L)
        whereClause += " and (lanes<=2)";
    if (interfaceType > IT_MAX_VAL) {
        int type1 = (interfaceType >> 8) & 0xff;
        int type2 = interfaceType & 0xff;
        whereClause += QString(" and (interface_type=%1 or interface_type=%2)").arg(type1).arg(type2);
    }
    else if (interfaceType == IT_DVP) {
        whereClause += QString(" and (interface_type=%1 or interface_type=%2)").arg(IT_DVP).arg(IT_MTK);
    }
    else if (interfaceType > -1) {
        whereClause += QString(" and (interface_type=%1)").arg(interfaceType);
    }

    return m_sensorDAL.SelectList_IdVendorSchemeName(sensors, whereClause);
}

bool SensorBLL::SelectListOrderByLastUsedNr(QList<Sensor> &sensors, int lastNr, int interfaceType)
{
    QString whereClause = QString(" where (app_type=%1)").arg(m_appType);
    if (m_onlyMipi2L)
        whereClause += " and (lanes<=2)";
    if (interfaceType > IT_MAX_VAL) {
        int type1 = (interfaceType >> 8) & 0xff;
        int type2 = interfaceType & 0xff;
        whereClause += QString(" and (interface_type=%1 or interface_type=%2)").arg(type1).arg(type2);
    }
    else if (interfaceType == IT_DVP) {
        whereClause += QString(" and (interface_type=%1 or interface_type=%2)").arg(IT_DVP).arg(IT_MTK);
    }
    else if (interfaceType > -1) {
        whereClause += QString(" and (interface_type=%1)").arg(interfaceType);
    }
    QString orderByClause = QString(" order by last_used_time desc LIMIT %1").arg(lastNr);

    return m_sensorDAL.SelectList(sensors, whereClause, orderByClause);
}

bool SensorBLL::SelectListByInterfaceAddrList(QList<Sensor> &sensors, int interfaceType, const QList<uint> &addrList)
{
    QString whereClause = QString(" where (app_type=%1)").arg(m_appType);
    if (m_onlyMipi2L)
        whereClause += " and (lanes<=2)";
    if (interfaceType > IT_MAX_VAL) {
        int type1 = (interfaceType >> 8) & 0xff;
        int type2 = interfaceType & 0xff;
        whereClause += QString(" and (interface_type=%1 or interface_type=%2)").arg(type1).arg(type2);
    }
    else if (interfaceType == IT_DVP) {
        whereClause += QString(" and (interface_type=%1 or interface_type=%2)").arg(IT_DVP).arg(IT_MTK);
    }
    else if (interfaceType > -1) {
        whereClause += QString(" and (interface_type=%1)").arg(interfaceType);
    }

    QString addrListStr = "";
    if (addrList.count() > 0) {
        addrListStr = " and (";
        foreach (uint addr, addrList) {
            addrListStr += QString("comm_addr=%1 or ").arg(addr);
        }
        addrListStr.chop(4);
        addrListStr += ")";
    }
    if (!addrListStr.isEmpty())
        whereClause += addrListStr;

    return m_sensorDAL.SelectList(sensors, whereClause);
}

bool SensorBLL::SelectByChipNameI2cAddr(QList<Sensor> &sensors, const QString &chipName, uint i2cAddr)
{
    QString whereClause = QString(" where (app_type=%1)").arg(m_appType);
    whereClause += QString(" and (chip_name='%1') and (comm_addr=%2)").arg(chipName).arg(i2cAddr);

    return m_sensorDAL.SelectList(sensors, whereClause);
}

bool SensorBLL::Exists(const QString &schemeName)
{
    QString whereClause = QString(" where (app_type=%1)").arg(m_appType);
    if (m_onlyMipi2L)
        whereClause += " and (lanes<=2)";
    whereClause += QString(" and (scheme_name='%1')").arg(schemeName);
    int cnt = m_sensorDAL.SelectCount(whereClause);

    return cnt > 0;
}

bool SensorBLL::LoadSensorParam(T_SENSOR_CFG &sensorConfig, const QString &schemeName)
{
    Sensor sensor;
    sensor.SchemeName = schemeName;
    if (!SelectBySchemeName(sensor) || !Sensor2SensorConfig(sensorConfig, sensor)) {
        qCritical()<<"Loading sensor parameter failed."<<schemeName;
        return false;
    }
    // update LastUsedTime
    uint currTime_t = QDateTime::currentDateTime().toTime_t();
    qDebug()<<schemeName<<currTime_t<<sensor.LastUsedTime;
    if (qAbs((int)currTime_t - (int)sensor.LastUsedTime) > 24*3600) {
        qDebug()<<"Update last used time.";
        sensor.LastUsedTime = currTime_t;
        Update(sensor);
    }
    return true;
}

bool SensorBLL::Sensor2SensorConfig(T_SENSOR_CFG &sensorConfig, const Sensor &sensor)
{
    sensorConfig.id             = sensor.Id;
    sensorConfig.SchemeName     = sensor.SchemeName.toStdString();
    sensorConfig.ChipName       = sensor.ChipName.toStdString();
    sensorConfig.Mclk_kHz       = (int)(sensor.Mclk * 1000); // MHz => kHz
    qDebug()<<"sensor.SchemeName:"<<sensor.SchemeName<<"Mclk_kHz:"<<sensorConfig.Mclk_kHz;
    sensorConfig.InterfaceType = sensor.InterfaceType;
    sensorConfig.Lanes          = sensor.Lanes;
    sensorConfig.MipiFreq      = sensor.MipiFreq;
    sensorConfig.DataWidth     = sensor.DataWidth;
    sensorConfig.PclkPol       = sensor.PclkPol;
    sensorConfig.DataPol       = sensor.DataPol;
    sensorConfig.HsyncPol      = sensor.HsyncPol;
    sensorConfig.VsyncPol      = sensor.VsyncPol;
    if (!ParsePwdnParam(sensorConfig.pwdn, sensor.Pwdn, sensor.PwdnParam))
        return false;
    if (!ParseResetParam(sensorConfig.reset, sensor.Reset, sensor.ResetParam))
        return false;
    if (!parseFocusParam(sensorConfig.FocusParam, sensor.FocusParam))
        return false;
    sensorConfig.ImageFormat   = sensor.ImageFormat;
    sensorConfig.ImageMode     = sensor.ImageMode;
    sensorConfig.PixelWidth    = sensor.PixelWidth;
    sensorConfig.PixelHeight   = sensor.PixelHeight;
    sensorConfig.QuickWidth    = sensor.QuickWidth;
    sensorConfig.QuickHeight   = sensor.QuickHeight;
    parseCropParam(sensorConfig.CropParam, sensor.CropParam); // crop param
    if (!ParseCommIntfConfParam(sensorConfig.CommIntfConf, sensor)) {
        return false;
    }
    parseLvdsParam(sensorConfig.LvdsParam, sensor.LvdsParam);

    if (!ParseVoltageList(sensor.Voltage, sensorConfig.Domains)) {
        qCritical()<<"Voltage parameters is invalid.";
        return false;
    }
    if (!parseRegisterList(sensor.FullModeParams, sensorConfig.FullModeParams)) {
        qCritical()<<"FullMode parameters is invalid.";
        return false;
    }
    if (!parseRegisterList(sensor.OtpInitParams, sensorConfig.OtpInitParams)) {
        qCritical()<<"OtpInit parameters is invalid.";
        return false;
    }
    if (!parseRegisterList(sensor.QuickModeParams, sensorConfig.QuickModeParams)) {
        qCritical()<<"QuickMode parameters is invalid.";
        return false;
    }
    if (!parseRegisterList(sensor.SleepParams, sensorConfig.SleepParams)) {
        qCritical()<<"Sleep parameters is invalid.";
        return false;
    }
    if (!parseRegisterList(sensor.AfInitParams, sensorConfig.AfInitParams)) {
        qCritical()<<"AfInit parameters is invalid.";
        return false;
    }
    if (!parseRegisterList(sensor.AfAutoParams, sensorConfig.AfAutoParams)) {
        qCritical()<<"AfAuto parameters is invalid.";
        return false;
    }
    if (!parseRegisterList(sensor.AfFarParams, sensorConfig.AfFarParams)) {
        qCritical()<<"AfFar parameters is invalid.";
        return false;
    }
    if (!parseRegisterList(sensor.AfNearParams, sensorConfig.AfNearParams)) {
        qCritical()<<"AfNear parameters is invalid.";
        return false;
    }
    if (sensorConfig.FocusParam.Type == AF_MODE_INVALID) {
        if ((sensorConfig.AfAutoParams.size() > 0) || (sensorConfig.AfFarParams.size() > 0) ||
            (sensorConfig.AfNearParams.size() > 0)) {
            sensorConfig.FocusParam.Type = AF_MODE_SENSOR;
        }
    }

    if (!parseHiLoRegisterList(sensor.ExposureParams, sensorConfig.ExposureParam)) {
        qDebug()<<"No exposure parameters.";
    }
    if (!parseHiLoRegisterList(sensor.GainParams, sensorConfig.GainParam)) {
        qDebug()<<"No gain parameters.";
    }
    if (!parseOtpAlgParams(sensor.OtpAlgParams, sensorConfig.OtpAlgDefParams)) {
        qCritical()<<"OtpAlgParams is invalid.";
        return false;
    }
    if (!ParseFlagRegister(sensor.FlagRegister, sensorConfig.FlagRegisters)/* || sensorConfig.FlagRegisters.count() != 2*/) {
        qCritical()<<"Flag registers is invalid."<<sensor.FlagRegister;
        return false;
    }
    return true;
}

bool SensorBLL::ParseFlagRegister(const QString &flagRegister, std::vector<T_RegConf> &regList)
{
    regList.clear();
    QStringList lines = flagRegister.trimmed().split(QChar('\n'), QString::SkipEmptyParts);
    QString line;
    for (int i = 0; i < lines.count(); i++) {
         line = lines[i].trimmed().remove(QChar(';'));
//         qDebug()<<"line:"<<line;
         QStringList paramList = line.split(QChar(','), QString::SkipEmptyParts);
         if (paramList.count() < 3)
             continue;
         bool ok;
         T_RegConf reg_cfg;
         reg_cfg.Delay_ms = 0;
         reg_cfg.Addr = paramList[0].toInt(&ok, 16);
         if (!ok)
             continue;
         reg_cfg.Value = paramList[1].toInt(&ok, 16);
         if (!ok)
             continue;
         reg_cfg.Mask = paramList[2].toInt(&ok, 16);
         if (!ok)
             continue;
//         qDebug()<<"addr:"<<reg_cfg.addr<<"value:"<<reg_cfg.value<<"mask:"<<reg_cfg.mask;
         regList.push_back(reg_cfg);
    }
    return true; //regList.size() > 0
}

int SensorBLL::GetHiLoRegisterValue(const T_HiLoRegCfg &hiLoRegCfg, E_RegBitsMode regBitsMode)
{
    // Find value register
    QList<T_RegConf> valRegList;
    std::vector<T_RegConf>::const_iterator it;
    for (it = hiLoRegCfg.Registers.begin(); it != hiLoRegCfg.Registers.end(); ++it) {
        if ((it->Addr != ESC_MODE) &&
            (it->Addr != ESC_ADDR)) {
            valRegList.append(*it);
//            qDebug("addr: %04x, value: %04x", it->addr, it->value);
            if (valRegList.count() >= hiLoRegCfg.ValRegCnt)
                break;
        }
    }

    int regDataBits = GetBitsFromRegBitsMode(regBitsMode, true);
    uint mask = (1 << regDataBits) - 1;
    int value = 0;
    if (valRegList.count() > 1) { // hi-register & lo-register
        value = ((valRegList[0].Value & mask) << regDataBits) | (valRegList[1].Value & mask);
    }
    else if (valRegList.count() > 0) {
        value = valRegList[0].Value;
    }
    return value;
}

bool SensorBLL::ParseVoltageList(const QString &voltageList, std::vector<T_Power> &powers)
{
//    qDebug()<<"voltageList:"<<voltageList;
    powers.clear();
//    PI_DVDD .. PI_VPP
    QStringList VoltageIdList;
    VoltageIdList<<"DVDD"<<"AVDD"<<"DOVDD"<<"AFVCC"<<"VPP";
    QStringList lines = voltageList.trimmed().split(QChar('\n'), QString::SkipEmptyParts);
    QString line;
    for (int i = 0; i < lines.count(); i++) {
        line = lines[i].trimmed().remove(QChar(';'));
//        qDebug()<<"line:"<<line;
        QStringList paramList = line.split(QChar(','), QString::SkipEmptyParts);
        if (paramList.count() < 3) // should be 3
            continue;
        T_Power power;
        power.Id = VoltageIdList.indexOf(paramList[0].toUpper());
        if (power.Id == -1)
            return false;
        bool ok;
        power.Value = paramList[1].toInt(&ok); // mV
        if (!ok)
            return false;
        power.Delay_ms = paramList[2].toInt(&ok);
        if (!ok)
            return false;
        powers.push_back(power);
    }
    return true; // domains.size() > 0
}

bool SensorBLL::insertOrUpdate(Sensor &sensor)
{
    bool flag = false;
    Sensor sensorDb;
    sensorDb.SchemeName = sensor.SchemeName;
    QString whereClause = QString(" where (scheme_name='%1')").arg(sensorDb.SchemeName);
    if (m_sensorDAL.Select(sensorDb, whereClause)) {
        sensor.Id = sensorDb.Id;
        flag = Update(sensor, true); // Special(update all fields)
    }
    else {
        flag = m_sensorDAL.Insert(sensor);
    }
    return flag;
}

bool SensorBLL::parseRegisterList(const QString &paramString, std::vector<T_RegConf> &regList)
{
//    qDebug()<<"paramString:"<<paramString;
    regList.clear();
    QStringList lines = paramString.trimmed().split(QChar('\n'), QString::SkipEmptyParts);
    QString line;
    for (int i = 0; i < lines.count(); i++) {
         line = lines[i].trimmed().remove(QChar(';'));
//         qDebug()<<"line:"<<line;
         QStringList regParamList = line.split(QChar(','), QString::SkipEmptyParts);
         if (regParamList.count() < 2)
             continue;
         bool ok;
         T_RegConf reg_cfg;
         reg_cfg.Addr = regParamList[0].toUInt(&ok, 16);
         if (!ok)
             continue;
         reg_cfg.Value = regParamList[1].toUInt(&ok, 16);
         if (!ok)
             continue;
         reg_cfg.Delay_ms = 0;
         if (regParamList.count() > 2) {
             reg_cfg.Delay_ms = regParamList[2].toUInt(&ok);
             if (!ok)
                 continue;
         }
         reg_cfg.Mask = 0xff;
         if (regParamList.count() > 3) {
             reg_cfg.Mask = regParamList[3].toUInt(&ok, 16);
             if (!ok)
                 continue;
         }
//         qDebug("addr: 0x%08x, value: 0x%08x, delay: %d, mask: 0x%08x", reg_cfg.Addr, reg_cfg.Value,
//                reg_cfg.Delay_ms, reg_cfg.Mask);
         regList.push_back(reg_cfg);
    }
    return true;
}

bool SensorBLL::parseHiLoRegisterList(const QString &paramString, T_HiLoRegCfg &hiLoRegCfg)
{
//    qDebug()<<"paramString:"<<paramString;
    hiLoRegCfg.Registers.clear(); hiLoRegCfg.MinVal = 0; hiLoRegCfg.MaxVal = 0;  hiLoRegCfg.ValRegCnt = 0;
    QStringList lines = paramString.trimmed().split(QChar('\n'), QString::SkipEmptyParts);
    if (lines.size() < 2)
        return false;
    // Parse first line
    QString line = lines[0].trimmed().remove(QChar(';'));
    QStringList regParamList = line.split(QChar(','), QString::SkipEmptyParts);
    if (regParamList.count() < 2)
        return false;
    bool ok;
    hiLoRegCfg.MinVal = regParamList[0].toInt(&ok, 16);
    if (!ok)
        return false;
    hiLoRegCfg.MaxVal = regParamList[1].toInt(&ok, 16);
    if (!ok)
        return false;
    if (regParamList.count() > 2) { // Downward compatibility
        hiLoRegCfg.ValRegCnt = regParamList[2].toInt(&ok);
        if (!ok)
            return false;
    }
    else {
        hiLoRegCfg.ValRegCnt = lines.count() - 1;
    }
    // Parse register list
    for (int i = 1; i < lines.count(); i++) {
         line = lines[i].trimmed().remove(QChar(';'));
//         qDebug()<<"line:"<<line;
         regParamList = line.split(",", QString::SkipEmptyParts);
         if (regParamList.count() < 2)
             continue;
         T_RegConf reg_cfg;
         reg_cfg.Addr = regParamList[0].toUInt(&ok, 16);
         if (!ok)
             continue;
         reg_cfg.Value = regParamList[1].toUInt(&ok, 16);
         if (!ok)
             continue;
#if 0
         // fault-tolerant
         if (hiLoRegCfg.MinVal > (int)reg_cfg.value)
             hiLoRegCfg.MinVal = reg_cfg.value;
         if (hiLoRegCfg.MaxVal < (int)reg_cfg.value)
             hiLoRegCfg.MaxVal = reg_cfg.value;
#endif
         reg_cfg.Delay_ms = 0;
         if (regParamList.count() > 2) {
             reg_cfg.Delay_ms = regParamList[2].toUInt(&ok);
             if (!ok)
                 continue;
         }
//         qDebug()<<"addr:"<<reg_cfg.Addr<<"value:"<<reg_cfg.Value<<"delay:"<<reg_cfg.Delay_ms;
         hiLoRegCfg.Registers.push_back(reg_cfg);
    }
    return hiLoRegCfg.Registers.size() > 0;
}

bool SensorBLL::ParsePwdnParam(T_PWDN_CFG &pwdnCfg, int val, const QString &paramString)
{
    pwdnCfg.val = val;

//    qDebug()<<"Pwdn param:"<<paramString;
    QStringList strList = paramString.trimmed().split(QChar(','), QString::SkipEmptyParts);
    if (strList.size() < 3)
        return false;
    bool ok;
    pwdnCfg.initIsValid = strList[0].toInt(&ok);
    if (!ok)
        return false;
    pwdnCfg.keepDelayMs = strList[1].toInt(&ok);
    if (!ok)
        return false;
    pwdnCfg.releaseDelayMs = strList[2].toInt(&ok);
    if (!ok)
        return false;
    return true;
}

bool SensorBLL::ParseResetParam(T_RESET_CFG &resetCfg, int val, const QString &paramString)
{
    resetCfg.val = val;

//    qDebug()<<"Reset param:"<<paramString;
    QStringList strList = paramString.trimmed().split(QChar(','), QString::SkipEmptyParts);
    if (strList.size() < 2)
        return false;
    bool ok;
    resetCfg.keepDelayMs = strList[0].toInt(&ok);
    if (!ok)
        return false;
    resetCfg.releaseDelayMs = strList[1].toInt(&ok);
    if (!ok)
        return false;
    return true;
}

bool SensorBLL::ParseCommIntfConfParam(T_CommIntfConf &commIntfConf, const Sensor &sensor)
{
    commIntfConf.IntfType    = sensor.CommIntfType;
    if (sensor.CommIntfType == T_CommIntfConf::IT_I2C) {
        commIntfConf.I2C.RegBitsMode = sensor.RegBitsMode;
        commIntfConf.I2C.Addr = sensor.CommAddr;
        commIntfConf.I2C.Speed = sensor.CommSpeed;
    }
    else {
        commIntfConf.Spi.RegBitsMode = sensor.RegBitsMode;
        commIntfConf.Spi.ChipId = sensor.CommAddr;
        commIntfConf.Spi.Speed = sensor.CommSpeed;
    }
    bool ok = true;
    if (!sensor.CommExtraParam.isEmpty()) {
        QStringList strList = sensor.CommExtraParam.trimmed().split(QChar(','), QString::SkipEmptyParts);
        if (commIntfConf.IntfType == T_CommIntfConf::IT_I2C) {
            // TBD
        }
        else {
            if (strList.size() < 3)
                return false;

            commIntfConf.Spi.Mode = strList[0].toUInt(&ok, 16);
            commIntfConf.Spi.DataBits = strList[1].toUInt(&ok);
            commIntfConf.Spi.IsBigEndian = (strList[2] == "1");
        }
    }

    return ok;
}

bool SensorBLL::parseLvdsParam(T_LvdsParam &lvdsParam, const QString &paramString)
{
    lvdsParam.Reset();
    QStringList paramList = paramString.trimmed().split(QChar(','), QString::SkipEmptyParts);
    if (paramList.size() > 6) {
        bool ok = false;
        lvdsParam.SeaFileName = paramList[0].trimmed().toStdString();
        lvdsParam.SedFileName = paramList[1].trimmed().toStdString();
        lvdsParam.Version = paramList[2].trimmed().toUInt(&ok, 16);
        lvdsParam.Mode = paramList[3].trimmed().toUInt(&ok, 16);
        lvdsParam.V_Total = paramList[4].trimmed().toUInt(&ok);
        lvdsParam.H_Total = paramList[5].trimmed().toUInt(&ok);
        lvdsParam.V_H_Blank = paramList[6].trimmed().toUInt(&ok);
        if (paramList.size() > 7)
            lvdsParam.Crop_Top = paramList[7].trimmed().toUInt(&ok);

//        qDebug("lvdsParam: %s %s Version: 0x%02x Mode: 0x%02x V_Total: %d H_Total: %d V_H_Blank: %d",
//               lvdsParam.SeaFileName.c_str(), lvdsParam.SedFileName.c_str(),
//               lvdsParam.Version, lvdsParam.Mode, lvdsParam.V_Total, lvdsParam.H_Total, lvdsParam.V_H_Blank);
    }

    return true;
}

uint SensorBLL::GetHiLoRegisterMaxVal(int regBitCnt, int regCnt)
{
    uint max = 0;
    if (regBitCnt == 8) {
        max = (regCnt == 1) ? 0xff : 0xffff;
    }
    else {
        max = (regCnt == 1) ? 0xffff : 0xffffffff;
    }
    return max;
}

bool SensorBLL::parseFocusParam(T_FocusParam &focusParam, const QString &paramString)
{
    focusParam.Type = (int)AF_MODE_INVALID;
    focusParam.SubType = 0;
    bool ok;
    QStringList strList = paramString.trimmed().split(QChar(','), QString::SkipEmptyParts);
    if (strList.size() > 1) {
        focusParam.Type = strList[0].toInt(&ok);
        if (!ok)
            return false;
        focusParam.SubType = strList[1].toInt(&ok);
        if (!ok)
            return false;
//        focusParam.FarPosition = strList[2].toInt(&ok);
//        if (!ok)
//            return false;
//        focusParam.NearPosition = strList[3].toInt(&ok);
//        if (!ok)
//            return false;
    }
    return true;
}

bool SensorBLL::parseCropParam(T_Rect &cropParam, const QString &paramString)
{
    cropParam.X = cropParam.Y = cropParam.Width = cropParam.Height = 0;
    QStringList strList = paramString.trimmed().split(QChar(','), QString::SkipEmptyParts);
    if (strList.size() > 3) {
        bool ok;
        cropParam.X = strList[0].toUInt(&ok);
        if (!ok)
            return false;
        cropParam.Y = strList[1].toUInt(&ok);
        if (!ok)
            return false;
        cropParam.Width = strList[2].toUInt(&ok);
        if (!ok)
            return false;
        cropParam.Height = strList[3].toUInt(&ok);
        if (!ok)
            return false;
    }
    return true;
}

bool SensorBLL::parseKeyValueParams(const QString &paramString, QMap<QString, QString> &baseInfoParams)
{
//    qDebug()<<"paramString:"<<paramString;
    baseInfoParams.clear();
    QStringList lines = paramString.trimmed().split(QChar('\n'), QString::SkipEmptyParts);
    QString line;
    for (int i = 0; i < lines.count(); i++) {
        line = lines[i].trimmed();
//        qDebug()<<"line:"<<line;
        QStringList regParamList = line.split(QChar(','), QString::SkipEmptyParts);
        if (regParamList.count() < 2)
            continue;
//        qDebug()<<"key:"<<regParamList[0].trimmed()<<"value:"<<regParamList[1].trimmed();
        baseInfoParams.insert(regParamList[0].trimmed().toLower(), regParamList[1].trimmed()); // 键值以小写保存
    }
    return true;
}

bool SensorBLL::parseOtpAlgParams(const QString &paramString, T_OtpAlgParams &otpAlgParam)
{
//    qDebug()<<"paramString:"<<paramString;
    QStringList lines = paramString.trimmed().split(QChar('\n'), QString::SkipEmptyParts);
    QString line, key, value;
    bool ok;
    for (int i = 0; i < lines.count(); i++) {
        line = lines[i].trimmed();
//        qDebug()<<"line:"<<line;
        int pos = line.indexOf(QChar('='));
        if (pos < 0)
            continue;
        key = line.mid(0, pos).trimmed();
        value = line.mid(pos + 1).trimmed();
//        qDebug()<<key<<value;
        QStringList paramList = value.split(QChar(','), QString::SkipEmptyParts);
        if (key == "Exposure") {
            if (paramList.count() < 3)
                continue;
            otpAlgParam.ExposureParam.StartStep = paramList[0].trimmed().toUInt(&ok);
            otpAlgParam.ExposureParam.EndStep = paramList[1].trimmed().toUInt(&ok);
            otpAlgParam.ExposureParam.StepValue = paramList[2].trimmed().toUInt(&ok);
        }
        else if ((key == "InitRoi") || (key == "CenterRoi")) {
            if (paramList.count() < 2)
                continue;
            otpAlgParam.CenterRoi.Width = paramList[0].trimmed().toUInt(&ok);
            otpAlgParam.CenterRoi.Height = paramList[1].trimmed().toUInt(&ok);
        }
        else if (key == "WbGCode") {
            if (paramList.count() < 2)
                continue;
            otpAlgParam.WbParam.GCodeRef = paramList[0].trimmed().toUInt(&ok);
            otpAlgParam.WbParam.GCodeOffset = paramList[1].trimmed().toUInt(&ok);
        }
        else if (key == "WbRoi") {
            if (paramList.count() < 2)
                continue;
            otpAlgParam.WbParam.WbRoi.Width = paramList[0].trimmed().toUInt(&ok);
            otpAlgParam.WbParam.WbRoi.Height = paramList[1].trimmed().toUInt(&ok);
            qDebug("WbRoi(%d, %d)", otpAlgParam.WbParam.WbRoi.Width, otpAlgParam.WbParam.WbRoi.Height);
        }
        else if (key == "LscRoi") {
            if (paramList.count() < 2)
                continue;
            otpAlgParam.LscParam.LscRoi.Width = paramList[0].trimmed().toUInt(&ok);
            otpAlgParam.LscParam.LscRoi.Height = paramList[1].trimmed().toUInt(&ok);
        }
        else if (key == "LscRatio") {
            if (paramList.count() > 3) {
                otpAlgParam.LscParam.RatioR = paramList[0].trimmed().toUInt(&ok);
                otpAlgParam.LscParam.RatioGr = paramList[1].trimmed().toUInt(&ok);
                otpAlgParam.LscParam.RatioGb = paramList[2].trimmed().toUInt(&ok);
                otpAlgParam.LscParam.RatioB = paramList[3].trimmed().toUInt(&ok);
            }
            else if (paramList.count() > 0)
                otpAlgParam.LscParam.RatioR = otpAlgParam.LscParam.RatioGr = otpAlgParam.LscParam.RatioGb
                        = otpAlgParam.LscParam.RatioB = paramList[0].trimmed().toUInt(&ok);
        }
        else if (key == "LscDestRatio") {
            if (paramList.count() > 1) {
                otpAlgParam.LscParam.DestRatioLower = paramList[0].trimmed().toUInt(&ok);
                otpAlgParam.LscParam.DestRatioUpper = paramList[1].trimmed().toUInt(&ok);
            }
        }
    }

    return true;
}
