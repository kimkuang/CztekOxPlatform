
#ifndef __IMX476_H__
#define __IMX476_H__
#include "ImageSensor.h"

class IMX476 : public ImageSensor
{
public:
    IMX476(const T_SensorSetting &sensorSetting);

    int ReadReg(ushort startAddr, ushort endAddr, uchar *buf, ushort page);

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
