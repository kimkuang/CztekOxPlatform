#include "cmtidal.h"
#include <QDebug>
#include "conf/sensorini.h"
#include "bll/sensorbll.h"

LIBRARY_API bool LoadSensorSettingFromFile(T_SensorSetting *sensorSetting, const char *fileName)
{
    SensorIni sensorIni;
    Sensor sensor;
    if (!sensorIni.LoadFromFile(sensor, QString(fileName)))
        return false;
    T_SENSOR_CFG sensorConfig;
    if (!SensorBLL::Sensor2SensorConfig(sensorConfig, sensor))
        return false;

    if (sensorSetting->Id > 0)
        FreeSensorSetting(sensorSetting);

    sensorSetting->Id = 1; // greater than 0
    sensorSetting->SchemeName = new char[sensorConfig.SchemeName.size() + 1];
    strcpy(sensorSetting->SchemeName, sensorConfig.SchemeName.c_str());
    sensorSetting->ChipName = new char[sensorConfig.ChipName.size() + 1];
    strcpy(sensorSetting->ChipName, sensorConfig.ChipName.c_str());
    sensorSetting->Mclk_kHz = sensorConfig.Mclk_kHz;
    sensorSetting->InterfaceType = sensorConfig.InterfaceType;
    sensorSetting->MipiFreq = sensorConfig.MipiFreq;
    sensorSetting->Lanes = sensorConfig.Lanes;
    sensorSetting->DataWidth = sensorConfig.DataWidth;
    sensorSetting->PclkPol = sensorConfig.PclkPol;
    sensorSetting->DataPol = sensorConfig.DataPol;
    sensorSetting->HsyncPol = sensorConfig.HsyncPol;
    sensorSetting->VsyncPol = sensorConfig.VsyncPol;
    sensorSetting->Pwdn = sensorConfig.pwdn.val;
    sensorSetting->Reset = sensorConfig.reset.val;
    sensorSetting->ImageFormat = sensorConfig.ImageFormat;
    sensorSetting->ImageMode = sensorConfig.ImageMode;
    sensorSetting->PixelWidth = sensorConfig.PixelWidth;
    sensorSetting->PixelHeight = sensorConfig.PixelHeight;
    sensorSetting->QuickWidth = sensorConfig.QuickWidth;
    sensorSetting->QuickHeight = sensorConfig.QuickHeight;
    sensorSetting->CropParam = sensorConfig.CropParam;
    sensorSetting->I2cParam = sensorConfig.CommIntfConf.I2C;

    sensorSetting->FullModeParamCount = (int)sensorConfig.FullModeParams.size();
    sensorSetting->FullModeParams = new T_RegConf[sensorSetting->FullModeParamCount];
    for (int i = 0; i < sensorSetting->FullModeParamCount; i++) {
        sensorSetting->FullModeParams[i] = sensorConfig.FullModeParams[i];
    }
    sensorSetting->OtpInitParamCount = (int)sensorConfig.OtpInitParams.size();
    sensorSetting->OtpInitParams = new T_RegConf[sensorSetting->OtpInitParamCount];
    for (int i = 0; i < sensorSetting->OtpInitParamCount; i++) {
        sensorSetting->OtpInitParams[i] = sensorConfig.OtpInitParams[i];
    }
    sensorSetting->SleepParamCount = (int)sensorConfig.SleepParams.size();
    sensorSetting->SleepParams = new T_RegConf[sensorSetting->SleepParamCount];
    for (int i = 0; i < sensorSetting->SleepParamCount; i++) {
        sensorSetting->SleepParams[i] = sensorConfig.SleepParams[i];
    }
    sensorSetting->AfInitParamCount = (int)sensorConfig.AfInitParams.size();
    sensorSetting->AfInitParams = new T_RegConf[sensorSetting->AfInitParamCount];
    for (int i = 0; i < sensorSetting->AfInitParamCount; i++) {
        sensorSetting->AfInitParams[i] = sensorConfig.AfInitParams[i];
    }
    sensorSetting->AfAutoParamCount = (int)sensorConfig.AfAutoParams.size();
    sensorSetting->AfAutoParams = new T_RegConf[sensorSetting->AfAutoParamCount];
    for (int i = 0; i < sensorSetting->AfAutoParamCount; i++) {
        sensorSetting->AfAutoParams[i] = sensorConfig.AfAutoParams[i];
    }
    sensorSetting->AfFarParamCount = (int)sensorConfig.AfFarParams.size();
    sensorSetting->AfFarParams = new T_RegConf[sensorSetting->AfFarParamCount];
    for (int i = 0; i < sensorSetting->AfFarParamCount; i++) {
        sensorSetting->AfFarParams[i] = sensorConfig.AfFarParams[i];
    }
    sensorSetting->AfNearParamCount = (int)sensorConfig.AfNearParams.size();
    sensorSetting->AfNearParams = new T_RegConf[sensorSetting->AfNearParamCount];
    for (int i = 0; i < sensorSetting->AfNearParamCount; i++) {
        sensorSetting->AfNearParams[i] = sensorConfig.AfNearParams[i];
    }
#if 0
    sensorSetting->ExposureParamCount = (int)sensorConfig.ExposureParam.size();
    sensorSetting->ExposureParams = new T_RegConf[sensorSetting->ExposureParamCount];
    for (int i = 0; i < sensorSetting->ExposureParamCount; i++) {
        sensorSetting->ExposureParams[i] = sensorConfig.ExposureParam[i];
    }
    sensorSetting->GainParamCount = (int)sensorConfig.GainParam.size();
    sensorSetting->GainParams = new T_RegConf[sensorSetting->GainParamCount];
    for (int i = 0; i < sensorSetting->GainParamCount; i++) {
        sensorSetting->GainParams[i] = sensorConfig.GainParam[i];
    }
#else
    sensorSetting->ExposureParamCount = 0;
    sensorSetting->GainParamCount = 0;
#endif
    sensorSetting->FlagRegisterCount = (int)sensorConfig.FlagRegisters.size();
    sensorSetting->FlagRegisters = new T_RegConf[sensorSetting->FlagRegisterCount];
    for (int i = 0; i < sensorSetting->FlagRegisterCount; i++) {
        sensorSetting->FlagRegisters[i] = sensorConfig.FlagRegisters[i];
    }

    sensorSetting->PowerCount = (int)sensorConfig.Domains.size();
    qInfo()<<sensorSetting->PowerCount;
    sensorSetting->Powers = new T_Power[sensorSetting->PowerCount];
    for (int i = 0; i < sensorSetting->PowerCount; i++) {
        sensorSetting->Powers[i] = sensorConfig.Domains[i];
    }
    return true;
}

void FreeSensorSetting(T_SensorSetting *sensorSetting)
{
    if (sensorSetting->Id > 0) {
        delete[] sensorSetting->SchemeName;
        delete[] sensorSetting->ChipName;
        if (sensorSetting->FullModeParamCount > 0)
            delete[] sensorSetting->FullModeParams;
        if (sensorSetting->OtpInitParamCount > 0)
            delete[] sensorSetting->OtpInitParams;
        if (sensorSetting->SleepParamCount > 0)
            delete[] sensorSetting->SleepParams;
        if (sensorSetting->AfInitParamCount > 0)
            delete[] sensorSetting->AfInitParams;
        if (sensorSetting->AfAutoParamCount > 0)
            delete[] sensorSetting->AfAutoParams;
        if (sensorSetting->AfFarParamCount > 0)
            delete[] sensorSetting->AfFarParams;
        if (sensorSetting->AfNearParamCount > 0)
            delete[] sensorSetting->AfNearParams;
        if (sensorSetting->ExposureParamCount > 0)
            delete[] sensorSetting->ExposureParams;
        if (sensorSetting->GainParamCount > 0)
            delete[] sensorSetting->GainParams;
        if (sensorSetting->FlagRegisterCount > 0)
            delete[] sensorSetting->FlagRegisters;
        if (sensorSetting->PowerCount > 0)
            delete[] sensorSetting->Powers;
    }
}
