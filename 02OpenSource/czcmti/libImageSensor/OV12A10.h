#ifndef __OV12A10_H__
#define __OV12A10_H__
#include "ImageSensor.h"

#ifdef IMG_DRV_OV12A10
#define EXPOSURE_REG_H_ADDR                                             0x3500
#define EXPOSURE_REG_M_ADDR                                             0x3501
#define EXPOSURE_REG_L_ADDR                                             0x3502

#define SENSOR_GAIN_REG_H_ADDR                                          0x3508
#define SENSOR_GAIN_REG_L_ADDR                                          0x3509

#define TEMPERATURE_CTL_REG_ADDR                                        0x4D12
#define TEMPERATURE_VAL_REG_ADDR                                        0x4D13

/** Fuse ID Reg Addr Define **/
#define FUSE_ID_ADDR_START                                              0x7000
#define FUSE_ID_ADDR_END                                                0x700F
    #define FUSE_ID_LEN                                                 (FUSE_ID_ADDR_END - FUSE_ID_ADDR_START + 1)
#endif

class OV12A10 : public ImageSensor
{
public:
    OV12A10(const T_SensorSetting &sensorSetting);

    virtual int ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg);
    virtual int GetTemperature(int &temperature);
    virtual int GetSensorFuseId(std::string &fuseId, bool bMultiple=false);
    virtual int SetSensorExposure(uint value);
    virtual int SetSensorGain(uint value, bool bMultiple=false);
    virtual int GetSensorExposure(uint &value);
    virtual int GetSensorGain(uint &value);

private:
    int ReadReg(ushort startAddr, ushort endAddr, ushort *buf, ushort page = 0);
};

#endif // __OV12A10_H__
