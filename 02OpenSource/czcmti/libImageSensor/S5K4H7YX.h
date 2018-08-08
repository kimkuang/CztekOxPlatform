#ifndef __S5K4H7YX_H__
#define __S5K4H7YX_H__
#include "ImageSensor.h"


class S5K4H7YX : public ImageSensor
{
public:
    S5K4H7YX(const T_SensorSetting &sensorSetting);

    int S5K4H7_ReadOTPPage(int page, uchar *Buff, int nLen);
    int ApplyLsc(BYTE* raw8, int width, int height, int nLSCTarget, int ob, uchar* nLenCReg, int nLenCRegCount, int LSCGroup);
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
#endif  // __S5K4H7YX_H__
