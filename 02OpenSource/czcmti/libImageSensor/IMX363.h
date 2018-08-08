#ifndef __IMX363_H__
#define __IMX363_H__
#include "ImageSensor.h"

class IMX363 : public ImageSensor
{
public:
    IMX363(const T_SensorSetting &sensorSetting);

    int GetTemperature(int &temperature);
    int GetSensorFuseId(std::string &fuseId, bool bUseMasterI2C);
    int SetSensorExposure(uint value);
    int SetSensorGain(uint value, bool bMultiple=false);
    int GetSensorExposure(uint &value);
    int GetSensorGain(uint &value);
    int ApplySpc(short *pSPC);

private:
    int readOtpData(ushort startAddr, ushort endAddr, uchar *buf, ushort page);
};

#endif // __IMX363_H__
