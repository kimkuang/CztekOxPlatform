#ifndef __IMAGESENSOR_H__
#define __IMAGESENSOR_H__

#include "IImageSensor.h"
#include <QVector>

//I2C Mode:
//      0:Normal 8Addr,8Data;
//		1:Samsung 8 Addr,8Data;
//		2:Micron 8 Addr,16Data
//		3:Stmicro 16Addr,8Data;
//		4:Micron2 16 Addr,16Data
typedef enum
{
    I2C_MODE_ADDR8_DATA8_NORMAL = RB_NORMAL,
    I2C_MODE_ADDR8_DATA8_SAMSUNG = RB_ADDR8_DATA8,
    I2C_MODE_ADDR8_DATA16 = RB_ADDR8_DATA16,
    I2C_MODE_ADDR16_DATA8 = RB_ADDR16_DATA8,
    I2C_MODE_ADDR16_DATA16 = RB_ADDR16_DATA16,

    I2C_MODE_MAX
}_I2C_MODE_;

#define MSB(x)                                              (unsigned char)( ((x) >> 8) & 0xFF )
#define LSB(x)                                              (unsigned char)( (x) & 0xFF )
#ifdef MAKEWORD
#undef MAKEWORD
#endif
#define MAKEWORD(HBy, LBy)                                  (unsigned short)( ((unsigned short)((HBy) & 0xFF) << 8) | ((LBy) & 0xFF) )
#define GetBit(x, pos)                                      ( (x) & (0x01 << (pos)) )
#define ClrBit(x, pos)                                      ( (x) & (~(0x01 << (pos))) )

class ImageSensor : public IImageSensor
{
public:
    ImageSensor() = delete;
    ImageSensor(const T_SensorSetting &sensorSetting);
    virtual ~ImageSensor();

    int ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg);
    int ApplyLsc(const uchar* raw8,int width,int height,int nLSCTarget,int ob,uchar* nLenCReg,int nLenCRegCount,int LSCGroup);
    int ApplySpc(short *pSPC);

    int BindChannelContext(T_ChannelContext *context);
    int GetSensorFuseId(std::string &fuseId, bool bMultiple = false);
    int SetSensorExposure(uint value);
    int SetSensorGain(uint value, bool bMultiple = false);
    int GetSensorExposure(uint &value);
    int GetSensorGain(uint &value);
    int GetTemperature(int &temperature);

    int WriteRegisterList(const QList<T_RegConf> &regList);

protected:
    int I2cRead(ushort uRegAddr, ushort &uRegVal, ushort uMode);
    int I2cWrite(ushort uRegAddr, ushort uRegVal, ushort uMode);
    struct ST_RegData {
        ushort RegAddr;
        ushort Data;
        ST_RegData() {
            RegAddr = 0;
            Data = 0;
        }
        ST_RegData(ushort reg, ushort data) {
            RegAddr = reg;
            Data = data;
        }
    };

    int readRegisters(QVector<ST_RegData> &regData, ushort uMode);
    int writeRegisters(const QVector<ST_RegData> &regData, ushort uMode);
    int i2cReadContinuous(uint regAddr, uint regAddrSize, uchar *data,  uint dataSize);
    int i2cWriteContinuous(uint regAddr, uint regAddrSize, const uchar *data,  uint dataSize);

protected:
    T_SensorSetting m_sensorSetting;
    IChannelController *m_channelController;
    T_ChannelContext *m_channelContext;
};

typedef IImageSensor* (*PGET_SENSOR)(const T_SensorSetting &);
typedef struct
{
    const char *pchName;
    PGET_SENSOR pGetSensor;
}_IMAGE_SENSOR_TABLE_;

#endif /* __IMAGESENSOR_H__ */
