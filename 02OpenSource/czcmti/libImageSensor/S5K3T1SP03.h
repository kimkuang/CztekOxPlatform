
#ifndef __S5K3T1SP03_H__
#define __S5K3T1SP03_H__

#include "ImageSensor.h"

#ifdef IMG_DRV_S5K3T1SP03
#define GAIN_DEFAULT                                                    0x0100

/** Exposure Reg Addr Define **/
#define EXPOSURE_REG_H_ADDR                                             0x0202
#define EXPOSURE_REG_L_ADDR                                             0x0203

/** Anglog Gain Reg Addr Define **/
#define SENSOR_GAIN_REG_H_ADDR                                          0x0204
#define SENSOR_GAIN_REG_L_ADDR                                          0x0205

#endif

class S5K3T1SP03 : public ImageSensor
{
public:
    S5K3T1SP03(const T_SensorSetting &sensorSetting);

    int ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg);
    int GetTemperature(int &temperature);
    int GetSensorFuseId(std::string &fuseId, bool bUseMasterI2C=false);
    int SetSensorExposure(uint value);
    int SetSensorGain(uint value, bool bMultiple=false);
    int GetSensorExposure(uint &value);
    int GetSensorGain(uint &value);
};

#endif
