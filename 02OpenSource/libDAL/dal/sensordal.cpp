#include "sensordal.h"
#include <QString>
#include <QSqlRecord>
#include <QDebug>

SensorDAL::SensorDAL()
{
    m_systemDb = SqlDatabaseUtil::GetDatabase("system");
    if (!m_systemDb.isOpen())
        qCritical("Database system has not opened.");
    m_sqlQuery = new QSqlQuery(m_systemDb);
}

SensorDAL::~SensorDAL()
{
    delete m_sqlQuery;
}

bool SensorDAL::Insert(const Sensor &sensor)
{
    m_time.restart();
    QString sql = "insert into t_sensor(id, scheme_name, description, vendor_name, chip_name, "
                  "interface_type, lanes, mipi_freq_def, mipi_freq, mclk_def, mclk, data_width, "
                  "image_fmt, image_mode_def, image_mode, pix_width, pix_height, "
                  "quick_width, quick_height, crop_param, lvds_param, reg_bits_mode, "
                  "comm_intf_type, comm_speed_def, comm_speed, comm_addr, comm_extra_param, "
                  "pclk_pol_def, pclk_pol, data_pol_def, data_pol, hsync_pol_def, hsync_pol, "
                  "vsync_pol_def, vsync_pol, pwdn_def, pwdn, pwdn_param, reset_def, reset, "
                  "reset_param, focus_param, app_type, full_mode_params, otp_init_params, "
                  "quick_mode_params, sleep_params, af_init_params, af_auto_params, "
                  "af_far_params, af_near_params, exposure_params, gain_params, otp_alg_params, "
                  "voltage_def, voltage, flag_register, system_record, last_used_time) values"
                  "(NULL, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, "
                  "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, "
                  "?, ?, ?, ?, ?, ?, ?, ?, ?)";
    m_sqlQuery->prepare(sql);
    int idx = 0;
    m_sqlQuery->bindValue(idx++, sensor.SchemeName);
    m_sqlQuery->bindValue(idx++, sensor.Description);
    m_sqlQuery->bindValue(idx++, sensor.VendorName);
    m_sqlQuery->bindValue(idx++, sensor.ChipName);
    m_sqlQuery->bindValue(idx++, sensor.InterfaceType);
    m_sqlQuery->bindValue(idx++, sensor.Lanes);
    m_sqlQuery->bindValue(idx++, sensor.MipiFreqDef);
    m_sqlQuery->bindValue(idx++, sensor.MipiFreq);
    m_sqlQuery->bindValue(idx++, sensor.MclkDef);
    m_sqlQuery->bindValue(idx++, sensor.Mclk);
    m_sqlQuery->bindValue(idx++, sensor.DataWidth);
    m_sqlQuery->bindValue(idx++, sensor.ImageFormat);
    m_sqlQuery->bindValue(idx++, sensor.ImageModeDef);
    m_sqlQuery->bindValue(idx++, sensor.ImageMode);
    m_sqlQuery->bindValue(idx++, sensor.PixelWidth);
    m_sqlQuery->bindValue(idx++, sensor.PixelHeight);
    m_sqlQuery->bindValue(idx++, sensor.QuickWidth);
    m_sqlQuery->bindValue(idx++, sensor.QuickHeight);
    m_sqlQuery->bindValue(idx++, sensor.CropParam);
    m_sqlQuery->bindValue(idx++, sensor.LvdsParam);
    m_sqlQuery->bindValue(idx++, sensor.RegBitsMode);
    m_sqlQuery->bindValue(idx++, sensor.CommIntfType);
    m_sqlQuery->bindValue(idx++, sensor.CommSpeedDef);
    m_sqlQuery->bindValue(idx++, sensor.CommSpeed);
    m_sqlQuery->bindValue(idx++, sensor.CommAddr);
    m_sqlQuery->bindValue(idx++, sensor.CommExtraParam);
    m_sqlQuery->bindValue(idx++, sensor.PclkPolDef);
    m_sqlQuery->bindValue(idx++, sensor.PclkPol);
    m_sqlQuery->bindValue(idx++, sensor.DataPolDef);
    m_sqlQuery->bindValue(idx++, sensor.DataPol);
    m_sqlQuery->bindValue(idx++, sensor.HsyncPolDef);
    m_sqlQuery->bindValue(idx++, sensor.HsyncPol);
    m_sqlQuery->bindValue(idx++, sensor.VsyncPolDef);
    m_sqlQuery->bindValue(idx++, sensor.VsyncPol);
    m_sqlQuery->bindValue(idx++, sensor.PwdnDef);
    m_sqlQuery->bindValue(idx++, sensor.Pwdn);
    m_sqlQuery->bindValue(idx++, sensor.PwdnParam);
    m_sqlQuery->bindValue(idx++, sensor.ResetDef);
    m_sqlQuery->bindValue(idx++, sensor.Reset);
    m_sqlQuery->bindValue(idx++, sensor.ResetParam);
    m_sqlQuery->bindValue(idx++, sensor.FocusParam);
    m_sqlQuery->bindValue(idx++, sensor.AppType);
    m_sqlQuery->bindValue(idx++, sensor.FullModeParams);
    m_sqlQuery->bindValue(idx++, sensor.OtpInitParams);
    m_sqlQuery->bindValue(idx++, sensor.QuickModeParams);
    m_sqlQuery->bindValue(idx++, sensor.SleepParams);
    m_sqlQuery->bindValue(idx++, sensor.AfInitParams);
    m_sqlQuery->bindValue(idx++, sensor.AfAutoParams);
    m_sqlQuery->bindValue(idx++, sensor.AfFarParams);
    m_sqlQuery->bindValue(idx++, sensor.AfNearParams);
    m_sqlQuery->bindValue(idx++, sensor.ExposureParams);
    m_sqlQuery->bindValue(idx++, sensor.GainParams);
    m_sqlQuery->bindValue(idx++, sensor.OtpAlgParams);
    m_sqlQuery->bindValue(idx++, sensor.VoltageDef);
    m_sqlQuery->bindValue(idx++, sensor.Voltage);
    m_sqlQuery->bindValue(idx++, sensor.FlagRegister);
    m_sqlQuery->bindValue(idx++, sensor.SystemRecord);
    m_sqlQuery->bindValue(idx++, sensor.LastUsedTime);
    //    qDebug()<<m_sqlQuery->executedQuery();
    //    qDebug()<<m_sqlQuery->lastQuery();
    if (!m_sqlQuery->exec()) {
        qDebug()<<sql;
        return false;
    }
    qDebug()<<QString("%1 execute time: %2ms").arg(__FUNCTION__).arg(m_time.elapsed());
    return m_sqlQuery->numRowsAffected() == 1;
}

bool SensorDAL::Delete(int id)
{
    QString sql = QString("delete from t_sensor where id=%1").arg(id);
    if (!m_sqlQuery->exec(sql)) {
        qDebug()<<sql;
        return false;
    }
    return m_sqlQuery->numRowsAffected() == 1;
}

bool SensorDAL::Update(const Sensor &sensor, const QString &whereClause, bool updateDef)
{
    m_time.restart();
    QString sql = "update t_sensor set scheme_name=?, description=?, "
                  "vendor_name=?, chip_name=?, interface_type=?, lanes=?, "
                  "mipi_freq=?, mclk=?, data_width=?, image_fmt=?, image_mode=?, "
                  "pix_width=?, pix_height=?, quick_width=?, quick_height=?, "
                  "crop_param=?, lvds_param=?, reg_bits_mode=?, comm_intf_type=?, "
                  "comm_speed=?, comm_addr=?, comm_extra_param=?, pclk_pol=?, data_pol=?, "
                  "hsync_pol=?, vsync_pol=?, pwdn=?, pwdn_param=?, reset=?, reset_param=?, "
                  "focus_param=?, app_type=?, full_mode_params=?, otp_init_params=?, "
                  "quick_mode_params=?, sleep_params=?, af_init_params=?, af_auto_params=?, "
                  "af_far_params=?, af_near_params=?, exposure_params=?, gain_params=?, "
                  "otp_alg_params=?, voltage=?, flag_register=?, last_used_time=? ";
    if (updateDef) {
        sql += ", mipi_freq_def=?, mclk_def=?, image_mode_def=?, comm_speed_def=?, "
               "pclk_pol_def=?, data_pol_def=?, hsync_pol_def=?, vsync_pol_def=?, "
               "pwdn_def=?, reset_def=?, voltage_def=?";
    }
    if (!whereClause.isEmpty()) {
        sql += whereClause;
    }

    m_sqlQuery->prepare(sql);
    int idx = 0;
    m_sqlQuery->bindValue(idx++, sensor.SchemeName);
    m_sqlQuery->bindValue(idx++, sensor.Description);
    m_sqlQuery->bindValue(idx++, sensor.VendorName);
    m_sqlQuery->bindValue(idx++, sensor.ChipName);
    m_sqlQuery->bindValue(idx++, sensor.InterfaceType);
    m_sqlQuery->bindValue(idx++, sensor.Lanes);
    m_sqlQuery->bindValue(idx++, sensor.MipiFreq);
    m_sqlQuery->bindValue(idx++, sensor.Mclk);
    m_sqlQuery->bindValue(idx++, sensor.DataWidth);
    m_sqlQuery->bindValue(idx++, sensor.ImageFormat);
    m_sqlQuery->bindValue(idx++, sensor.ImageMode);
    m_sqlQuery->bindValue(idx++, sensor.PixelWidth);
    m_sqlQuery->bindValue(idx++, sensor.PixelHeight);
    m_sqlQuery->bindValue(idx++, sensor.QuickWidth);
    m_sqlQuery->bindValue(idx++, sensor.QuickHeight);
    m_sqlQuery->bindValue(idx++, sensor.CropParam);
    m_sqlQuery->bindValue(idx++, sensor.LvdsParam);
    m_sqlQuery->bindValue(idx++, sensor.RegBitsMode);
    m_sqlQuery->bindValue(idx++, sensor.CommIntfType);
    m_sqlQuery->bindValue(idx++, sensor.CommSpeed);
    m_sqlQuery->bindValue(idx++, sensor.CommAddr);
    m_sqlQuery->bindValue(idx++, sensor.CommExtraParam);
    m_sqlQuery->bindValue(idx++, sensor.PclkPol);
    m_sqlQuery->bindValue(idx++, sensor.DataPol);
    m_sqlQuery->bindValue(idx++, sensor.HsyncPol);
    m_sqlQuery->bindValue(idx++, sensor.VsyncPol);
    m_sqlQuery->bindValue(idx++, sensor.Pwdn);
    m_sqlQuery->bindValue(idx++, sensor.PwdnParam);
    m_sqlQuery->bindValue(idx++, sensor.Reset);
    m_sqlQuery->bindValue(idx++, sensor.ResetParam);
    m_sqlQuery->bindValue(idx++, sensor.FocusParam);
    m_sqlQuery->bindValue(idx++, sensor.AppType);
    m_sqlQuery->bindValue(idx++, sensor.FullModeParams);
    m_sqlQuery->bindValue(idx++, sensor.OtpInitParams);
    m_sqlQuery->bindValue(idx++, sensor.QuickModeParams);
    m_sqlQuery->bindValue(idx++, sensor.SleepParams);
    m_sqlQuery->bindValue(idx++, sensor.AfInitParams);
    m_sqlQuery->bindValue(idx++, sensor.AfAutoParams);
    m_sqlQuery->bindValue(idx++, sensor.AfFarParams);
    m_sqlQuery->bindValue(idx++, sensor.AfNearParams);
    m_sqlQuery->bindValue(idx++, sensor.ExposureParams);
    m_sqlQuery->bindValue(idx++, sensor.GainParams);
    m_sqlQuery->bindValue(idx++, sensor.OtpAlgParams);
    m_sqlQuery->bindValue(idx++, sensor.Voltage);
    m_sqlQuery->bindValue(idx++, sensor.FlagRegister);
    m_sqlQuery->bindValue(idx++, sensor.LastUsedTime);
    if (updateDef) {
        m_sqlQuery->bindValue(idx++, sensor.MipiFreqDef);
        m_sqlQuery->bindValue(idx++, sensor.MclkDef);
        m_sqlQuery->bindValue(idx++, sensor.ImageModeDef);
        m_sqlQuery->bindValue(idx++, sensor.CommSpeedDef);
        m_sqlQuery->bindValue(idx++, sensor.PclkPolDef);
        m_sqlQuery->bindValue(idx++, sensor.DataPolDef);
        m_sqlQuery->bindValue(idx++, sensor.HsyncPolDef);
        m_sqlQuery->bindValue(idx++, sensor.VsyncPolDef);
        m_sqlQuery->bindValue(idx++, sensor.PwdnDef);
        m_sqlQuery->bindValue(idx++, sensor.ResetDef);
        m_sqlQuery->bindValue(idx++, sensor.VoltageDef);
    }

//    qDebug()<<m_sqlQuery->executedQuery();
//    qDebug()<<m_sqlQuery->lastQuery();
    if (!m_sqlQuery->exec()) {
        qDebug()<<sql;
        return false;
    }
    qDebug()<<QString("%1 execute time: %2ms").arg(__FUNCTION__).arg(m_time.elapsed())<<sensor.Description;
    return m_sqlQuery->numRowsAffected() == 1;
}

bool SensorDAL::Select(Sensor &sensor, const QString &whereClause)
{
    m_time.restart();
    QString sql = "select id, scheme_name, description, vendor_name, chip_name, interface_type, "
                  "lanes, mipi_freq_def, mipi_freq, mclk_def, mclk, data_width, image_fmt, "
                  "image_mode_def, image_mode, pix_width, pix_height, quick_width, quick_height, "
                  "crop_param, lvds_param, reg_bits_mode, comm_intf_type, comm_speed_def, comm_speed, "
                  "comm_addr, comm_extra_param, pclk_pol_def, pclk_pol, data_pol_def, data_pol, "
                  "hsync_pol_def, hsync_pol, vsync_pol_def, vsync_pol, pwdn_def, pwdn, pwdn_param, "
                  "reset_def, reset, reset_param, focus_param, app_type, full_mode_params, "
                  "otp_init_params, quick_mode_params, sleep_params, af_init_params, af_auto_params, "
                  "af_far_params, af_near_params, exposure_params, gain_params, otp_alg_params, "
                  "voltage_def, voltage, flag_register, system_record, last_used_time from t_sensor";
    if (!whereClause.isEmpty())
        sql += whereClause;
    if (!m_sqlQuery->exec(sql)) {
        qDebug()<<sql;
        return false;
    }
    if (m_sqlQuery->next()) {
        int idx = 0;
        sensor.Id = m_sqlQuery->value(idx++).toInt();
        sensor.SchemeName = m_sqlQuery->value(idx++).toString();
        sensor.Description = m_sqlQuery->value(idx++).toString();
        sensor.VendorName = m_sqlQuery->value(idx++).toString();
        sensor.ChipName = m_sqlQuery->value(idx++).toString();
        sensor.InterfaceType = (E_InterfaceType)m_sqlQuery->value(idx++).toInt();
        sensor.Lanes = m_sqlQuery->value(idx++).toInt();
        sensor.MipiFreqDef = m_sqlQuery->value(idx++).toInt();
        sensor.MipiFreq = m_sqlQuery->value(idx++).toInt();
        sensor.MclkDef = m_sqlQuery->value(idx++).toFloat();
        sensor.Mclk = m_sqlQuery->value(idx++).toFloat();
        sensor.DataWidth = m_sqlQuery->value(idx++).toInt();
        sensor.ImageFormat = (E_ImageFormat)m_sqlQuery->value(idx++).toInt();
        sensor.ImageModeDef = (E_ImageMode)m_sqlQuery->value(idx++).toInt();
        sensor.ImageMode = (E_ImageMode)m_sqlQuery->value(idx++).toInt();
        sensor.PixelWidth = m_sqlQuery->value(idx++).toInt();
        sensor.PixelHeight = m_sqlQuery->value(idx++).toInt();
        sensor.QuickWidth = m_sqlQuery->value(idx++).toInt();
        sensor.QuickHeight = m_sqlQuery->value(idx++).toInt();
        sensor.CropParam = m_sqlQuery->value(idx++).toString();
        sensor.LvdsParam = m_sqlQuery->value(idx++).toString();
        sensor.RegBitsMode = (E_RegBitsMode)m_sqlQuery->value(idx++).toInt();
        sensor.CommIntfType = (T_CommIntfConf::E_IntfType)m_sqlQuery->value(idx++).toInt();
        sensor.CommSpeedDef = m_sqlQuery->value(idx++).toInt();
        sensor.CommSpeed = m_sqlQuery->value(idx++).toInt();
        sensor.CommAddr = m_sqlQuery->value(idx++).toInt();
        sensor.CommExtraParam = m_sqlQuery->value(idx++).toString();
        sensor.PclkPolDef = m_sqlQuery->value(idx++).toInt();
        sensor.PclkPol = m_sqlQuery->value(idx++).toInt();
        sensor.DataPolDef = m_sqlQuery->value(idx++).toInt();
        sensor.DataPol = m_sqlQuery->value(idx++).toInt();
        sensor.HsyncPolDef = m_sqlQuery->value(idx++).toInt();
        sensor.HsyncPol = m_sqlQuery->value(idx++).toInt();
        sensor.VsyncPolDef = m_sqlQuery->value(idx++).toInt();
        sensor.VsyncPol = m_sqlQuery->value(idx++).toInt();
        sensor.PwdnDef = m_sqlQuery->value(idx++).toInt();
        sensor.Pwdn = m_sqlQuery->value(idx++).toInt();
        sensor.PwdnParam = m_sqlQuery->value(idx++).toString();
        sensor.ResetDef = m_sqlQuery->value(idx++).toInt();
        sensor.Reset = m_sqlQuery->value(idx++).toInt();
        sensor.ResetParam = m_sqlQuery->value(idx++).toString();
        sensor.FocusParam = m_sqlQuery->value(idx++).toString();
        sensor.AppType = m_sqlQuery->value(idx++).toInt();
        sensor.FullModeParams = m_sqlQuery->value(idx++).toString();
        sensor.OtpInitParams = m_sqlQuery->value(idx++).toString();
        sensor.QuickModeParams = m_sqlQuery->value(idx++).toString();
        sensor.SleepParams = m_sqlQuery->value(idx++).toString();
        sensor.AfInitParams = m_sqlQuery->value(idx++).toString();
        sensor.AfAutoParams = m_sqlQuery->value(idx++).toString();
        sensor.AfFarParams = m_sqlQuery->value(idx++).toString();
        sensor.AfNearParams = m_sqlQuery->value(idx++).toString();
        sensor.ExposureParams = m_sqlQuery->value(idx++).toString();
        sensor.GainParams = m_sqlQuery->value(idx++).toString();
        sensor.OtpAlgParams = m_sqlQuery->value(idx++).toString();
        sensor.VoltageDef = m_sqlQuery->value(idx++).toString();
        sensor.Voltage = m_sqlQuery->value(idx++).toString();
        sensor.FlagRegister = m_sqlQuery->value(idx++).toString();
        sensor.SystemRecord = m_sqlQuery->value(idx++).toInt();
        sensor.LastUsedTime = m_sqlQuery->value(idx++).toUInt();
        qDebug()<<QString("%1 execute time: %2ms").arg(__FUNCTION__).arg(m_time.elapsed());
        return true;
    }
    return false;
}

bool SensorDAL::SelectList(QList<Sensor> &sensors, const QString &whereClause, const QString &orderByClause)
{
    m_time.restart();
    QString sql = "select id, scheme_name, description, vendor_name, chip_name, interface_type, "
                  "lanes, mipi_freq_def, mipi_freq, mclk_def, mclk, data_width, image_fmt, "
                  "image_mode_def, image_mode, pix_width, pix_height, quick_width, quick_height, "
                  "crop_param, lvds_param, reg_bits_mode, comm_intf_type, comm_speed_def, comm_speed, "
                  "comm_addr, comm_extra_param, pclk_pol_def, pclk_pol, data_pol_def, data_pol, "
                  "hsync_pol_def, hsync_pol, vsync_pol_def, vsync_pol, pwdn_def, pwdn, pwdn_param, "
                  "reset_def, reset, reset_param, focus_param, app_type, full_mode_params, "
                  "otp_init_params, quick_mode_params, sleep_params, af_init_params, af_auto_params, "
                  "af_far_params, af_near_params, exposure_params, gain_params, otp_alg_params, "
                  "voltage_def, voltage, flag_register, system_record, last_used_time from t_sensor";
    if (!whereClause.isEmpty())
        sql += whereClause;
    if (!orderByClause.isEmpty())
        sql += orderByClause;
    if (!m_sqlQuery->exec(sql)) {
        qDebug()<<sql;
        return false;
    }
    sensors.clear();
    while (m_sqlQuery->next()) {
        Sensor sensor;
        int idx = 0;
        sensor.Id = m_sqlQuery->value(idx++).toInt();
        sensor.SchemeName = m_sqlQuery->value(idx++).toString();
        sensor.Description = m_sqlQuery->value(idx++).toString();
        sensor.VendorName = m_sqlQuery->value(idx++).toString();
        sensor.ChipName = m_sqlQuery->value(idx++).toString();
        sensor.InterfaceType = (E_InterfaceType)m_sqlQuery->value(idx++).toInt();
        sensor.Lanes = m_sqlQuery->value(idx++).toInt();
        sensor.MipiFreqDef = m_sqlQuery->value(idx++).toInt();
        sensor.MipiFreq = m_sqlQuery->value(idx++).toInt();
        sensor.MclkDef = m_sqlQuery->value(idx++).toFloat();
        sensor.Mclk = m_sqlQuery->value(idx++).toFloat();
        sensor.DataWidth = m_sqlQuery->value(idx++).toInt();
        sensor.ImageFormat = (E_ImageFormat)m_sqlQuery->value(idx++).toInt();
        sensor.ImageModeDef = (E_ImageMode)m_sqlQuery->value(idx++).toInt();
        sensor.ImageMode = (E_ImageMode)m_sqlQuery->value(idx++).toInt();
        sensor.PixelWidth = m_sqlQuery->value(idx++).toInt();
        sensor.PixelHeight = m_sqlQuery->value(idx++).toInt();
        sensor.QuickWidth = m_sqlQuery->value(idx++).toInt();
        sensor.QuickHeight = m_sqlQuery->value(idx++).toInt();
        sensor.CropParam = m_sqlQuery->value(idx++).toString();
        sensor.LvdsParam = m_sqlQuery->value(idx++).toString();
        sensor.RegBitsMode = (E_RegBitsMode)m_sqlQuery->value(idx++).toInt();
        sensor.CommIntfType = (T_CommIntfConf::E_IntfType)m_sqlQuery->value(idx++).toInt();
        sensor.CommSpeedDef = m_sqlQuery->value(idx++).toInt();
        sensor.CommSpeed = m_sqlQuery->value(idx++).toInt();
        sensor.CommAddr = m_sqlQuery->value(idx++).toInt();
        sensor.CommExtraParam = m_sqlQuery->value(idx++).toString();
        sensor.PclkPolDef = m_sqlQuery->value(idx++).toInt();
        sensor.PclkPol = m_sqlQuery->value(idx++).toInt();
        sensor.DataPolDef = m_sqlQuery->value(idx++).toInt();
        sensor.DataPol = m_sqlQuery->value(idx++).toInt();
        sensor.HsyncPolDef = m_sqlQuery->value(idx++).toInt();
        sensor.HsyncPol = m_sqlQuery->value(idx++).toInt();
        sensor.VsyncPolDef = m_sqlQuery->value(idx++).toInt();
        sensor.VsyncPol = m_sqlQuery->value(idx++).toInt();
        sensor.PwdnDef = m_sqlQuery->value(idx++).toInt();
        sensor.Pwdn = m_sqlQuery->value(idx++).toInt();
        sensor.PwdnParam = m_sqlQuery->value(idx++).toString();
        sensor.ResetDef = m_sqlQuery->value(idx++).toInt();
        sensor.Reset = m_sqlQuery->value(idx++).toInt();
        sensor.ResetParam = m_sqlQuery->value(idx++).toString();
        sensor.FocusParam = m_sqlQuery->value(idx++).toString();
        sensor.AppType = m_sqlQuery->value(idx++).toInt();
        sensor.FullModeParams = m_sqlQuery->value(idx++).toString();
        sensor.OtpInitParams = m_sqlQuery->value(idx++).toString();
        sensor.QuickModeParams = m_sqlQuery->value(idx++).toString();
        sensor.SleepParams = m_sqlQuery->value(idx++).toString();
        sensor.AfInitParams = m_sqlQuery->value(idx++).toString();
        sensor.AfAutoParams = m_sqlQuery->value(idx++).toString();
        sensor.AfFarParams = m_sqlQuery->value(idx++).toString();
        sensor.AfNearParams = m_sqlQuery->value(idx++).toString();
        sensor.ExposureParams = m_sqlQuery->value(idx++).toString();
        sensor.GainParams = m_sqlQuery->value(idx++).toString();
        sensor.OtpAlgParams = m_sqlQuery->value(idx++).toString();
        sensor.VoltageDef = m_sqlQuery->value(idx++).toString();
        sensor.Voltage = m_sqlQuery->value(idx++).toString();
        sensor.FlagRegister = m_sqlQuery->value(idx++).toString();
        sensor.SystemRecord = m_sqlQuery->value(idx++).toInt();
        sensor.LastUsedTime = m_sqlQuery->value(idx++).toUInt();
        sensors.append(sensor);
    }
    qDebug()<<QString("%1 execute time: %2ms").arg(__FUNCTION__).arg(m_time.elapsed());
    return true;
}

/* 相对于SelectList，检索的字段减少，提高了效率 */
bool SensorDAL::SelectList_IdVendorSchemeName(QList<Sensor> &sensors, const QString &whereClause)
{
    m_time.restart();
    QString sql = "select id, scheme_name, vendor_name from t_sensor";
    if (!whereClause.isEmpty())
        sql += whereClause;
    if (!m_sqlQuery->exec(sql)) {
        qDebug()<<sql;
        return false;
    }
    sensors.clear();
    while (m_sqlQuery->next()) {
        Sensor sensor;
        int idx = 0;
        sensor.Id = m_sqlQuery->value(idx++).toInt();
        sensor.SchemeName = m_sqlQuery->value(idx++).toString();
        sensor.VendorName = m_sqlQuery->value(idx++).toString();
        sensors.append(sensor);
    }
    qDebug()<<QString("%1 execute time: %2ms").arg(__FUNCTION__).arg(m_time.elapsed());
    return true;
}

int SensorDAL::SelectCount(const QString &whereClause)
{
    QString sql = "select count(id) from t_sensor";
    if (!whereClause.isEmpty())
        sql += whereClause;
    if (!m_sqlQuery->exec(sql)) {
        qDebug()<<sql;
        return 0;
    }
    int cnt = 0;
    if (m_sqlQuery->next()) {
        cnt = m_sqlQuery->value(0).toInt();
    }
    return cnt;
}
