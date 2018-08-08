#ifndef __OV02A10_H__
#define __OV02A10_H__
#include "ImageSensor.h"

#ifdef IMG_DRV_OV02A10
#define EXPOSURE_REG_H_ADDR                                             0x03
#define EXPOSURE_REG_L_ADDR                                             0x04

#define SENSOR_GAIN_REG_ADDR                                            0x24

#endif

class OV02A10 : public ImageSensor
{
public:
    OV02A10(const T_SensorSetting &sensorSetting);

    virtual int ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg);
    virtual int GetTemperature(int &temperature);
    virtual int GetSensorFuseId(std::string &fuseId, bool bMultiple=false);
    virtual int SetSensorExposure(uint value);
    virtual int SetSensorGain(uint value, bool bMultiple=false);
    virtual int GetSensorExposure(uint &value);
    virtual int GetSensorGain(uint &value);
};

#endif // __OV02A10_H__
