#ifndef __IMX398_H__
#define __IMX398_H__
#include "ImageSensor.h"

#ifdef IMG_DRV_IMX398
// 参考文档《IMX576-AAKH5_Software_Reference_Manual_0.0.1.pdf》
// 参考OTP读写操作的文档《IMX576_AAKH5_OTP_Manual_0.0.1.pdf》
/** Temperature Reg Addr Define **/
#define TEMPATURE_CTRL_ADDR         0x0138
#define TEMP_CTRL_DIS               (0x00 << 0)
#define TEMP_CTRL_EN                (0x01 << 0)
#define TEMPATURE_DATA_ADDR         0x013A

/** Exposure Reg Addr Define **/
#define EXPOSURE_REG_H_ADDR         0x0202
#define EXPOSURE_REG_L_ADDR         0x0203

/** Analog Gain Reg Addr Define **/
#define SENSOR_GAIN_REG_H_ADDR      0x0204
#define SENSOR_GAIN_REG_L_ADDR      0x0205

/** Channel GAIN Reg Addr Define **/
#define GAIN_DEFAULT                0x0100
#define GAIN_GREEN1_HIGH_ADDR       0x020E
#define GAIN_GREEN1_LOW_ADDR        0x020F
#define GAIN_BLUE_HIGH_ADDR         0x0212
#define GAIN_BLUE_LOW_ADDR          0x0213
#define GAIN_RED_HIGH_ADDR          0x0210
#define GAIN_RED_LOW_ADDR           0x0211
#define GAIN_GREEN2_HIGH_ADDR       0x0214
#define GAIN_GREEN2_LOW_ADDR        0x0215

#define DPGA_USE_GLOBAL_GAIN        0x3FF9
#define DIGITAL_GAIN_BY_COLOR       (0x00 << 0)
#define DIGITAL_GAIN_ALL_COLOR      (0x01 << 0)

/** OTP Reg Addr Define **/
#define REG_PAGE_SEL_ADDR           0x0A02
#define REG_DATA_START_ADDR         0x0A04
#define REG_DATA_END_ADDR           0x0A43
#define PAGE_LEN                    (REG_DATA_END_ADDR - REG_DATA_START_ADDR + 1)

/** Fuse ID Reg Addr Define **/
#define FUSE_ID_PAGE                0x1F
#define FUSE_ID_ADDR_START          0x0A27
#define FUSE_ID_ADDR_END            0x0A2E
#define FUSE_ID_LEN                 (FUSE_ID_ADDR_END - FUSE_ID_ADDR_START + 1)
#endif

class IMX398 : public ImageSensor
{
public:
    IMX398(const T_SensorSetting &sensorSetting);

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
