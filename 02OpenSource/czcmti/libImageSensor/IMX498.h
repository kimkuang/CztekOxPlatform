
#ifndef __IMX498_H__
#define __IMX498_H__
#include "ImageSensor.h"

class IMX498 : public ImageSensor
{
public:
    IMX498(const T_SensorSetting &sensorSetting);

    int ReadReg(ushort startAddr, ushort endAddr, uchar *buf, ushort page);

    int ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg);
    int GetSensorFuseId(std::string &fuseId, bool bMultiple=false);
    int SetSensorExposure(uint value);
    int SetSensorGain(uint value, bool bMultiple=false);
    int GetSensorExposure(uint &value);
    int GetSensorGain(uint &value);
    int ApplySpc(short *pSPC);

private:
    int readOtpData(ushort startAddr, ushort endAddr, uchar *buf, ushort page);
};

#endif
