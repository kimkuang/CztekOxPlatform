#ifndef SENSOR_H
#define SENSOR_H

#include <QString>
#include <QStringList>
#include "bo/sensorbo.h"

class Sensor
{
public:
    Sensor()
    {
        Id = 0;
        SchemeName = "";
        Description = "";
        VendorName = "";
        ChipName = "";
        InterfaceType = IT_MIPI;
        Lanes = 0;
        MipiFreqDef = 0;
        MipiFreq = 0;
        MclkDef = 0;
        Mclk = 0;
        DataWidth = 0;
        ImageFormat = IMAGE_FMT_INVALID;
        ImageModeDef = IMAGE_MODE_INVALID;
        ImageMode = IMAGE_MODE_INVALID;
        PixelWidth = 0;
        PixelHeight = 0;
        QuickWidth = 0;
        QuickHeight = 0;
        CropParam = "";
        RegBitsMode = RB_NORMAL;
        CommIntfType = T_CommIntfConf::IT_I2C;
        CommSpeedDef = 0;
        CommSpeed = 0;
        CommAddr = 0;
        CommExtraParam = "";
        PclkPolDef = 0;
        PclkPol = 0;
        DataPolDef = 0;
        DataPol = 0;
        HsyncPolDef = 0;
        HsyncPol = 0;
        VsyncPolDef = 0;
        VsyncPol = 0;
        PwdnDef = 0;
        Pwdn = 0;
        PwdnParam = "";
        ResetDef = 0;
        Reset = 0;
        ResetParam = "";
        FocusParam = "";
        AppType = 0;
        LvdsParam = "";
        FullModeParams = "";
        OtpInitParams = "";
        QuickModeParams = "";
        SleepParams = "";
        AfInitParams = "";
        AfAutoParams = "";
        AfFarParams = "";
        AfNearParams = "";
        ExposureParams = "";
        GainParams = "";
        OtpAlgParams = "";
        VoltageDef = "";
        Voltage = "";
        FlagRegister = "";
        SystemRecord = 0;
        LastUsedTime = 0;
    }

    Sensor(const Sensor& sensor)
    {
        Id = sensor.Id;
        SchemeName = sensor.SchemeName;
        Description = sensor.Description;
        VendorName = sensor.VendorName;
        ChipName = sensor.ChipName;
        InterfaceType = sensor.InterfaceType;
        Lanes = sensor.Lanes;
        MipiFreqDef = sensor.MipiFreqDef;
        MipiFreq = sensor.MipiFreq;
        MclkDef = sensor.MclkDef;
        Mclk = sensor.Mclk;
        DataWidth = sensor.DataWidth;
        ImageFormat = sensor.ImageFormat;
        ImageModeDef = sensor.ImageModeDef;
        ImageMode = sensor.ImageMode;
        PixelWidth = sensor.PixelWidth;
        PixelHeight = sensor.PixelHeight;
        QuickWidth = sensor.QuickWidth;
        QuickHeight = sensor.QuickHeight;
        CropParam = sensor.CropParam;
        RegBitsMode = sensor.RegBitsMode;
        CommIntfType = sensor.CommIntfType;
        CommSpeedDef = sensor.CommSpeedDef;
        CommSpeed = sensor.CommSpeed;
        CommAddr = sensor.CommAddr;
        CommExtraParam = sensor.CommExtraParam;
        PclkPolDef = sensor.PclkPolDef;
        PclkPol = sensor.PclkPol;
        DataPolDef = sensor.DataPolDef;
        DataPol = sensor.DataPol;
        HsyncPolDef = sensor.HsyncPolDef;
        HsyncPol = sensor.HsyncPol;
        VsyncPolDef = sensor.VsyncPolDef;
        VsyncPol = sensor.VsyncPol;
        PwdnDef = sensor.PwdnDef;
        Pwdn = sensor.Pwdn;
        PwdnParam = sensor.PwdnParam;
        ResetDef = sensor.ResetDef;
        Reset = sensor.Reset;
        ResetParam = sensor.ResetParam;
        FocusParam = sensor.FocusParam;
        AppType = sensor.AppType;
        LvdsParam = sensor.LvdsParam;
        FullModeParams = sensor.FullModeParams;
        OtpInitParams = sensor.OtpInitParams;
        QuickModeParams = sensor.QuickModeParams;
        SleepParams = sensor.SleepParams;
        AfInitParams = sensor.AfInitParams;
        AfAutoParams = sensor.AfAutoParams;
        AfFarParams = sensor.AfFarParams;
        AfNearParams = sensor.AfNearParams;
        ExposureParams = sensor.ExposureParams;
        GainParams = sensor.GainParams;
        OtpAlgParams = sensor.OtpAlgParams;
        VoltageDef = sensor.VoltageDef;
        Voltage = sensor.Voltage;
        FlagRegister = sensor.FlagRegister;
        SystemRecord = sensor.SystemRecord;
        LastUsedTime = sensor.LastUsedTime;
    }

public:
    int Id;
    QString SchemeName;
    QString Description;
    QString VendorName;
    QString ChipName;
    E_InterfaceType InterfaceType;
    int Lanes;
    int MipiFreqDef;
    int MipiFreq;
    float MclkDef; // MHz
    float Mclk;
    int DataWidth;
    E_ImageFormat ImageFormat;
    E_ImageMode ImageModeDef;
    E_ImageMode ImageMode;
    int PixelWidth;
    int PixelHeight;
    int QuickWidth;
    int QuickHeight;
    QString CropParam;
    E_RegBitsMode RegBitsMode;
    T_CommIntfConf::E_IntfType CommIntfType;
    int CommSpeedDef;
    int CommSpeed;
    int CommAddr;
    QString CommExtraParam;
    int PclkPolDef;
    int PclkPol;
    int DataPolDef;
    int DataPol;
    int HsyncPolDef;
    int HsyncPol;
    int VsyncPolDef;
    int VsyncPol;
    int PwdnDef;
    int Pwdn;
    QString PwdnParam;
    int ResetDef;
    int Reset;
    QString ResetParam;
    QString FocusParam;
    int AppType;
    QString LvdsParam;
    QString FullModeParams;
    QString OtpInitParams;
    QString QuickModeParams;
    QString SleepParams;
    QString AfInitParams;
    QString AfAutoParams;
    QString AfFarParams;
    QString AfNearParams;
    QString ExposureParams;
    QString GainParams;
    QString OtpAlgParams;
    QString VoltageDef;
    QString Voltage;
    QString FlagRegister;
    int SystemRecord;
    uint LastUsedTime;
};

#endif // SENSOR_H
