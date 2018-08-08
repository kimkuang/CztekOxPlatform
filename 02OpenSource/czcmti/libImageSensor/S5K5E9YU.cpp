#include "S5K5E9YU.h"
#include <QThread>
#include <QVector>
#include "czcmtidefs.h"
#include "macrodefs.h"


#define REG_OP_MODE_ADDR            0x0100  //Operating Mode Registers---0:Software Standby, 1:Streaming(Active Video)--I2C Communiction with sensor is possible, Core is power on.
#define SENSOR_OP_MODE_MASK         0x01
#define SENSOR_SOFTWARE_STANDBY     (0x00 << 0)
#define SENSOR_STREAMING_ON         (0x01 << 0)

#define REG_CTL_RW_ADDR             0x0A00
#define INIT_CMD                    (0x00)
#define R_CMD                       (0x01 << 0)
#define W_CMD                       (0x03 << 0)
#define PENDING_ERROR               (0x01 << 2)

#define REG_ERR_FLGA_ADDR           0x0A01
#define R_RDY                       (0x01 << 0)
#define W_RDY                       (0x01 << 1)
#define DATA_CORRUPTED              (0x01 << 2)
#define IMPROPY_USAGE               (0x01 << 3)

#define REG_PAGE_SEL_ADDR           0x0A02

#define REG_DATA_START_ADDR         0x0A04
#define REG_DATA_END_ADDR           0x0A43
#define PAGE_LEN                    (REG_DATA_END_ADDR - REG_DATA_START_ADDR + 1)

/** Fuse ID Reg Addr Define **/
#define FUSE_ID_PAGE                0x00
#define FUSE_ID_ADDR_START          0x0A04
#define FUSE_ID_ADDR_END            0x0A0B
#define FUSE_ID_LEN                 (FUSE_ID_ADDR_END - FUSE_ID_ADDR_START + 1)

/** Exposure Reg Addr Define **/
#define EXPOSURE_REG_H_ADDR         0x0202
#define EXPOSURE_REG_L_ADDR         0x0203

/** Anglog Gain Reg Addr Define **/
#define SENSOR_GAIN_REG_H_ADDR      0x0204
#define SENSOR_GAIN_REG_L_ADDR      0x0205

S5K5E9YU::S5K5E9YU(const T_SensorSetting &sensorSetting) :
    ImageSensor(sensorSetting)
{
}

int S5K5E9YU::GetSensorFuseId(std::string &fuseId, bool bUseMasterI2C)
{
    (void)bUseMasterI2C;
    uchar buf[FUSE_ID_LEN] = {0};
    int ec = readOtpData(FUSE_ID_ADDR_START, FUSE_ID_ADDR_END, buf, FUSE_ID_PAGE);
    if (ec < 0) {
        QString strLog = QString("S5K5E9YU: Read FuseId failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    fuseId = "";
    for (int i = 0; i < FUSE_ID_LEN; i++) {
        QString strFuseId = "";
        strFuseId = QString::asprintf("%02X", buf[i]);
        fuseId += strFuseId.toStdString();
    }

    return ERR_NoError;
}

int S5K5E9YU::SetSensorExposure(uint value)
{
    uchar buf[] = {MSB(value), LSB(value)};
    int ec = i2cWriteContinuous(EXPOSURE_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("S5K5E9YU::%1---WriteContinuousI2c() failed[%2]")
                                .arg(__FUNCTION__).arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ERR_NoError;
}

int S5K5E9YU::SetSensorGain(uint value, bool bMultiple)
{
    if (true == bMultiple)
    {
        value = value * 0x20;
        //return SetSensorGain(value, false);
    }
    if (value > 512)
    {
        value = 512;  // 0 ~ 512
    }

    uchar buf[] = {MSB(value), LSB(value)};
    int ec = i2cWriteContinuous(SENSOR_GAIN_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("S5K5E9YU::Call WriteContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ERR_NoError;
}

int S5K5E9YU::GetSensorExposure(uint &value)
{
    uchar buf[] = {0, 0};
    int ec = i2cReadContinuous(EXPOSURE_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("S5K5E9YU::%1---ReadContinuousI2c() failed[%2]")
                                .arg(__FUNCTION__).arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;

    }

    value = MAKEWORD(buf[0], buf[1]);
    return ERR_NoError;
}

int S5K5E9YU::GetSensorGain(uint &value)
{
    uchar buf[] = {0, 0};
    int ec = i2cReadContinuous(SENSOR_GAIN_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("S5K5E9YU::%1---ReadContinuousI2c() failed[%2]")
                                .arg(__FUNCTION__).arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;

    }

    value = MAKEWORD(buf[0], buf[1]);
    return ERR_NoError;
}

int S5K5E9YU::readOtpData(ushort startAddr, ushort endAddr, uchar *buf, ushort page)
{
    if (nullptr == buf) {
        qCritical("S5K5E9YU::%s Error---buf is NULL", __FUNCTION__);
        return ERR_InvalidParameter;
    }

    int nLen = endAddr - startAddr + 1;
    if (nLen > PAGE_LEN) {
        qCritical("S5K5E9YU::%s Error---nLen:%d is outside PAGE_LEN:%d", __FUNCTION__, nLen, PAGE_LEN);
        return ERR_InvalidParameter;
    }

    ushort stram_flag = 0;
    if (ERR_NoError != I2cRead(REG_OP_MODE_ADDR, stram_flag, I2C_MODE_ADDR16_DATA8)) {
        qCritical("S5K5E9YU::%s---I2cRead Error", __FUNCTION__);
        return ERR_Failed;
    }

    if (SENSOR_SOFTWARE_STANDBY == (stram_flag & SENSOR_OP_MODE_MASK)) {
        if (ERR_NoError != I2cWrite(REG_OP_MODE_ADDR, SENSOR_STREAMING_ON, I2C_MODE_ADDR16_DATA8)) {
            qCritical("S5K5E9YU::%s---I2cWrite REG_OP_MODE_ADDR Error", __FUNCTION__);
            return ERR_Failed;
        }
        /**
          * 根据文档<(5E9YX)OTP_Read_Write_Guide_REV0.0_20171030.pdf>中的Read部分介绍，在对Reg[0x0100]写入0x01后，需要延时
          * 50ms，然后再对寄存器Reg[0x0A02]写入Page号。而Reg[0x0100]写入0x01是[Streaming on]的意思，0x00是Software Standby
          * 的意思。所以，根据个人理解，只有从[Software Standby]变到[Streaming on]时才要延时这50ms，如果Sensor一直处于[Streaming on]
          * 状态下，不需要延时50ms。所以，把这个延时50ms的操作从下面挪到此处。
          * ----20180423
        **/
        QThread::msleep(50);
    }

    int ec = ERR_NoError;
    ec = I2cWrite(REG_PAGE_SEL_ADDR, page, I2C_MODE_ADDR16_DATA8);  // select otp page
    ec |= I2cWrite(REG_CTL_RW_ADDR, R_CMD, I2C_MODE_ADDR16_DATA8);  // turn on otp read mode
    if (ec < 0) {
        qCritical("Turn on S5K5E9YU otp read mode failed[%d]", ec);
        return ec;
    }

    bool checkFailed = true;
    ushort flag = 0;
    for (uint i = 0; i < 10; i++) { // check status repeat 10 times
        if (ERR_NoError == I2cRead(REG_ERR_FLGA_ADDR, flag, I2C_MODE_ADDR16_DATA8)) {
            if ((flag & 0x01) == 0x01) {
                checkFailed = false;
                break;
            }
        }
        QThread::msleep(50);
    }
    if (checkFailed) {
        qCritical("Check S5K5E9YU OTP status[0x%x] failed", flag);
        return ERR_Failed;
    }

    //read otp data to buff
    ec = i2cReadContinuous(startAddr, 2, buf, endAddr - startAddr + 1);
    if (ec < 0) {
        qCritical("S5K5E9YU read otp addr[0x%x] failed[%d]", startAddr, ec);
        return ec;
    }

    // Inital command
    ec = I2cWrite(REG_CTL_RW_ADDR, INIT_CMD, I2C_MODE_ADDR16_DATA8);
    if (ec < 0) {
        qCritical("S5K5E9YU::%s---I2cWrite REG_CTL_RW_ADDR Error\r\n", __FUNCTION__);
        return ec;
    }

    return ec;
}

