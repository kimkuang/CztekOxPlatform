#ifndef __IMX600_H__
#define __IMX600_H__
#include "ImageSensor.h"

class IMX600 : public ImageSensor
{
public:
    IMX600(const T_SensorSetting &sensorSetting);

    int ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg);
    int GetTemperature(int &temperature);
    int GetSensorFuseId(std::string &fuseId, bool bMultiple=false);
    int SetSensorExposure(uint value);
    int SetSensorGain(uint value, bool bMultiple=false);
    int GetSensorExposure(uint &value);
    int GetSensorGain(uint &value);

private:
    int readOtpData(ushort startAddr, ushort endAddr, uchar *buf, ushort page);
};

#endif
