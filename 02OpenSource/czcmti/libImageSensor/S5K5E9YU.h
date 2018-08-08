#ifndef __S5K5E9YU_H__
#define __S5K5E9YU_H__
#include "ImageSensor.h"

class S5K5E9YU : public ImageSensor
{
public:
    S5K5E9YU(const T_SensorSetting &sensorSetting);
    int GetSensorFuseId(std::string &fuseId, bool bMultiple=false);
    int SetSensorExposure(uint value);
    int SetSensorGain(uint value, bool bMultiple=false);
    int GetSensorExposure(uint &value);
    int GetSensorGain(uint &value);

private:
    int ReadReg(ushort startAddr, ushort endAddr, uchar *buf, ushort page = 0);
    int readOtpData(ushort startAddr, ushort endAddr, uchar *buf, ushort page);
};

#endif // __S5K5E9YU_H__
