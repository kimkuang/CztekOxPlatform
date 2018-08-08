#ifndef __OTPSENSOR_H__
#define __OTPSENSOR_H__

#include "IOtpSensor.h"
#include <QVector>
#include <QThread>

class OtpSensor : public IOtpSensor
{
public:
    OtpSensor(uint i2cAddr);
    virtual ~OtpSensor();
    int BindChannelContext(T_ChannelContext *context);
    virtual int OtpRead(uint startAddr, uint endAddr, uchar *data, ushort page = 0);
    virtual int OtpWrite(uint startAddr, uint endAddr, const uchar *data, ushort page = 0);

protected:
    int I2cRead(ushort uRegAddr, ushort& uRegVal, ushort uMode);
    int I2cWrite(ushort uRegAddr, ushort uRegVal, ushort uMode);
    int I2cReadBlock(ushort uRegAddr, ushort uRegSize, uchar* pData, ushort uSize);
    int I2cWriteBlock(ushort uRegAddr, ushort uRegSize, const uchar* pData, ushort uSize);
    struct ST_RegData {
        ushort RegAddr;
        ushort Data;
    };

    int readRegisters(QVector<ST_RegData> &regData, ushort uMode);
    int writeRegisters(const QVector<ST_RegData> &regData, ushort uMode);

protected:
    enum {
        // 读写操作设置I2C的速率为400kHz时，长度可设置为1024字节, 为100kHz时设置为256字节。
        // 数据长度太长读写操作会失败。
        I2C_RW_MAX_BYTES  = 1024,
    };
    uint m_i2cAddr;
    T_ChannelContext *m_channelContext;
    IChannelController *m_channelController;
};

typedef OtpSensor* (*PGET_OTP)(uint &);
typedef struct
{
    const char *pchName;
    PGET_OTP pGetOtp;
}_OTP_SENSOR_TABLE_;

#endif /* __OTPSENSOR_H__ */
