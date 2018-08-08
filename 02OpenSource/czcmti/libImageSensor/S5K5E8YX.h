#ifndef __S5K5E8YX_H__
#define __S5K5E8YX_H__
#include "ImageSensor.h"

class S5K5E8YX : public ImageSensor
{
public:
    S5K5E8YX(const T_SensorSetting &sensorSetting);

    virtual int ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg);
    virtual int GetTemperature(int &temperature);
    virtual int GetSensorFuseId(std::string &fuseId, bool bMultiple=false);
    virtual int SetSensorExposure(uint value);
    virtual int SetSensorGain(uint value, bool bMultiple=false);
    virtual int GetSensorExposure(uint &value);
    virtual int GetSensorGain(uint &value);

private:
    int readOtpData(ushort startAddr, ushort endAddr, uchar *buf, ushort page);
};

#endif  // __S5K5E8YX_H__
