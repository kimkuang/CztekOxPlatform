#ifndef SENSORBLL_H
#define SENSORBLL_H

#include "bo/sensorbo.h"
#include "dal/sensordal.h"
#include "library_global.h"
#include <list>

class LIBRARY_API SensorBLL
{
public:
    SensorBLL();
    ~SensorBLL();

    static void SetOnlyMipi2L(bool val);
    bool Insert(Sensor &sensor);
    bool Delete(int id);
    bool Update(const Sensor &sensor, bool updateDef = false);
    bool SelectById(Sensor &sensor);
    bool SelectBySchemeName(Sensor &sensor);
    bool SelectList(QList<Sensor> &sensors);
    bool SelectList_IdVendorSchemeNameByInterfaceType(QList<Sensor> &sensors, int interfaceType = -1);
    bool SelectListOrderByLastUsedNr(QList<Sensor> &sensors, int lastNr, int interfaceType = -1);
    bool SelectListByInterfaceAddrList(QList<Sensor> &sensors, int interfaceType, const QList<uint> &addrList);
    bool SelectByChipNameI2cAddr(QList<Sensor> &sensors, const QString &chipName, uint i2cAddr);
    bool Exists(const QString &schemeName);

public:
    bool LoadSensorParam(T_SENSOR_CFG &sensorConfig, const QString &schemeName);
    static bool Sensor2SensorConfig(T_SENSOR_CFG &sensorConfig, const Sensor &sensor);
    static bool ParseFlagRegister(const QString &flagRegister, std::vector<T_RegConf> &regList);
    static int  GetHiLoRegisterValue(const T_HiLoRegCfg &hiLoRegCfg, E_RegBitsMode regBitsMode);
    static bool ParseVoltageList(const QString &voltageList, std::vector<T_Power> &powers);
    static bool ParsePwdnParam(T_PWDN_CFG &pwdnCfg, int val, const QString &paramString);
    static bool ParseResetParam(T_RESET_CFG &resetCfg, int val, const QString &paramString);
    static bool ParseCommIntfConfParam(T_CommIntfConf &commIntfConf, const Sensor &sensor);
    static bool parseLvdsParam(T_LvdsParam &lvdsParam, const QString &paramString);
    static uint GetHiLoRegisterMaxVal(int regBitCnt, int regCnt);
    static inline int GetBitsFromRegBitsMode(E_RegBitsMode mode, bool isDataBits)
    {
        if (isDataBits) {
            if ((mode == RB_ADDR8_DATA8) ||
                (mode == RB_ADDR16_DATA8) ||
                (mode == RB_NORMAL))
                return 8;
            else
                return 16;
        }
        else {
            if ((mode == RB_ADDR8_DATA8) ||
                (mode == RB_ADDR8_DATA16) ||
                (mode == RB_NORMAL))
                return 8;
            else
                return 16;
        }
    }

private:
    bool insertOrUpdate(Sensor &sensor);
    static bool parseRegisterList(const QString &paramString, std::vector<T_RegConf> &regList);
    static bool parseHiLoRegisterList(const QString &paramString, T_HiLoRegCfg &hiLoRegCfg);
    static bool parseFocusParam(T_FocusParam &focusParam, const QString &paramString);
    static bool parseCropParam(T_Rect &cropParam, const QString &paramString);
    static bool parseKeyValueParams(const QString &paramString, QMap<QString, QString> &baseInfoParams);
    static bool parseOtpAlgParams(const QString &paramString, T_OtpAlgParams &otpAlgParam);

private:
    SensorDAL m_sensorDAL;
    int m_appType;
    static bool m_onlyMipi2L; // GlobalVars::ThisProductFeatures.testFlag(ProductBarcode::PF_OnlyMipi2L)
};

#endif // SENSORBLL_H
