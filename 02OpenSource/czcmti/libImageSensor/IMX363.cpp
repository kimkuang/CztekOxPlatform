#include "IMX363.h"
#include <QThread>
#include <QVector>
#include "czcmtidefs.h"
#include "macrodefs.h"

// 参考文档《IMX363_Software_Reference_Manual_1.0.1.pdf》
// 参考OTP读写操作的文档《IMX363_OTP_Manual_1.0.1》
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

/** OTP Reg Addr Define **/
#define REG_DATA_CTRL_ADDR          0x0A00
#define REG_OTP_STATUS_ADDR         0x0A01
#define DATA_READ_CTRL_DIS          (0x00 << 0)
#define DATA_READ_CTRL_EN           (0x01 << 0)
#define REG_PAGE_SEL_ADDR           0x0A02
#define REG_DATA_START_ADDR         0x0A04
#define REG_DATA_END_ADDR           0x0A43
#define PAGE_LEN                    (REG_DATA_END_ADDR - REG_DATA_START_ADDR + 1)

/** Fuse ID Reg Addr Define **/
#define FUSE_ID_PAGE                0x7F
#define FUSE_ID_ADDR_START          0x0A21
#define FUSE_ID_ADDR_END            0x0A29
#define FUSE_ID_LEN                 (FUSE_ID_ADDR_END - FUSE_ID_ADDR_START + 1)


IMX363::IMX363(const T_SensorSetting &sensorSetting) :
    ImageSensor(sensorSetting)
{
}

int IMX363::GetTemperature(int &temperature)
{
    ushort tempCtlData = 0;
    int ec = I2cRead(TEMPATURE_CTRL_ADDR, tempCtlData, I2C_MODE_ADDR16_DATA8);
    if (ec < 0) {
        QString strLog = "Read IMX363 TEMPERATURE CTRL Register failed";
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }
    if ((tempCtlData & 0x01) != 0x01) {
        ec = I2cWrite(TEMPATURE_CTRL_ADDR, 0x01, I2C_MODE_ADDR16_DATA8);
        if (ec < 0) {
            QString strLog = "Write IMX363 temperature control Register failed";
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            qCritical() << strLog;
            return ec;
        }
        QThread::msleep(200); // delay 2 frames
    }

    ushort ubTemp = 0x00;
    ec = I2cRead(TEMPATURE_DATA_ADDR, ubTemp, I2C_MODE_ADDR16_DATA8);
    if (ec < 0) {
        QString strLog = "Read IMX363 temperature data Register failed";
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    /**
      * 温度范围：-20°~80°，负温度采用补码表示，正温度采用原码表示。
      *     1、0x81~0xEC：都是-20°
      *     2、0xED~0xFF：-19°~-1°
      *     3、0x00：0°
      *     4、0x01~0x4F：1°~79°
      *     5、0x50~0x7F：都是80°
      * ----20180411
    **/
    if (ubTemp >= 0xED/* && ubTemp <= 0xFF*/)
    {
        temperature = (int)(ubTemp - 256);
    }
    else if (ubTemp >= 0x81 && ubTemp <= 0xEC)
    {
        temperature = -20;
    }
    else if (ubTemp >= 0x50 && ubTemp <= 0x7F)
    {
        temperature = 80;
    }
    else if (/*ubTemp >= 0x00 && */ubTemp < 0x50)
    {
        temperature = ubTemp;
    }
    else    //处理0x80的值，手册没有说明，按手册的写法，应该是【-0°】的补码
    {
        temperature = 0;
    }

    return ERR_NoError;
}

int IMX363::GetSensorFuseId(std::string &fuseId, bool bUseMasterI2C)
{
    (void)bUseMasterI2C;
    uchar tmpBuff[FUSE_ID_LEN] = {0};
    int ec = readOtpData(FUSE_ID_ADDR_START, FUSE_ID_ADDR_END, tmpBuff, FUSE_ID_PAGE);
    if (ec < 0) {
        QString strLog = QString("Read IMX363 FuseId failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }
    fuseId = "";
    for (uint i = 0; i < ARRAY_SIZE(tmpBuff); i++) {
        QString strFuseId = "";
        strFuseId = QString::asprintf("%02X", tmpBuff[i]);
        fuseId += strFuseId.toStdString();
    }
    return ERR_NoError;
}

int IMX363::SetSensorExposure(uint value)
{
    uchar buf[] = {MSB(value), LSB(value)};
    int ec = i2cWriteContinuous(EXPOSURE_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("IMX363::Call WriteContinuousI2c() failed[%2]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ERR_NoError;
}

int IMX363::SetSensorGain(uint value, bool bMultiple)
{
    if (true == bMultiple)      //1x,2x,4x,8x,16x
    {
        value = 512 - (512 / value);
        //return SetSensorGain(value, false);
    }

    if (value > 448)    //Gain: 0 ~ 448
    {
        value = 448;
    }

    uchar buf[] = {MSB(value), LSB(value)};
    int ec = i2cWriteContinuous(SENSOR_GAIN_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("IMX363::Call WriteContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ERR_NoError;
}

int IMX363::GetSensorExposure(uint &value)
{
    uchar buf[] = {0, 0};
    int ec = i2cReadContinuous(EXPOSURE_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("IMX363::Call ReadContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    value = MAKEWORD(buf[0], buf[1]);
    return ERR_NoError;
}

int IMX363::GetSensorGain(uint &value)
{
    uchar buf[] = {0, 0};
    int ec = i2cReadContinuous(SENSOR_GAIN_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("IMX363::Call ReadContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    value = MAKEWORD(buf[0], buf[1]);
    return ERR_NoError;
}

int IMX363::readOtpData(ushort startAddr, ushort endAddr, uchar *buf, ushort page)
{
    if (nullptr == buf) {
        qCritical("IMX363::%s Error---buf is NULL", __FUNCTION__);
        return ERR_InvalidParameter;
    }

    uint bufferLen = endAddr - startAddr + 1;
    if (bufferLen > PAGE_LEN) {
        qCritical("IMX363::%s Error---nLen:%d is outside PAGE_LEN:%d", __FUNCTION__, bufferLen, PAGE_LEN);
        return ERR_InvalidParameter;
    }

    int ec = ERR_NoError;
    ec = I2cWrite(REG_PAGE_SEL_ADDR, page, I2C_MODE_ADDR16_DATA8);     // select otp page
    ec |= I2cWrite(REG_DATA_CTRL_ADDR, DATA_READ_CTRL_EN, I2C_MODE_ADDR16_DATA8);    // turn on otp read mode
    if (ec < 0) {
        qCritical("Turn on IMX363 otp read mode failed[%d]", ec);
        return ec;
    }

    bool checkFailed = true;
    ushort flag = 0;
    for (uint i = 0; i < 20; i++) { // check status repeat 20 times
        if (ERR_NoError == I2cRead(REG_OTP_STATUS_ADDR, flag, I2C_MODE_ADDR16_DATA8)) {
            if ((flag & 0x01) == 0x01) {
                checkFailed = false;
                break;
            }
        }
        QThread::msleep(10);
    }
    if (checkFailed) {
        qCritical("Check IMX363 OTP status[0x%x] failed", flag);
        return ERR_Failed;
    }

    //read otp data to buff
    ec = i2cReadContinuous(startAddr, 2, buf, bufferLen);
    if (ec < 0) {
        qCritical("IMX363 read otp addr[0x%x] failed[%d]", startAddr, ec);
        return ec;
    }

    return ec;
}

int IMX363::ApplySpc(short *pSPC)
{
    if (nullptr == pSPC) {
        qCritical("IMX363::%s Error---pSPC is NULL", __FUNCTION__);
        return ERR_InvalidParameter;
    }

    if(I2cWrite(0x0100, 0x00, I2C_MODE_ADDR16_DATA8) < 0) return ERR_I2cWrite;
    if(I2cWrite(0x0101, 0x00, I2C_MODE_ADDR16_DATA8) < 0) return ERR_I2cWrite; //V and H are both set by this.
    for (int i=0; i<48; i++)
    {
        if(I2cWrite(0x7d4c+i, pSPC[i], I2C_MODE_ADDR16_DATA8) < 0) return ERR_I2cWrite;
        if(I2cWrite(0x7d80+i, pSPC[i+48], I2C_MODE_ADDR16_DATA8) < 0) return ERR_I2cWrite;
    }

    if(I2cWrite(0x0101, 0x00, I2C_MODE_ADDR16_DATA8) < 0) return ERR_I2cWrite;  //setting 2
    if(I2cWrite(0x0b00, 0x00, I2C_MODE_ADDR16_DATA8) < 0) return ERR_I2cWrite;
    if(I2cWrite(0x3051, 0x00, I2C_MODE_ADDR16_DATA8) < 0) return ERR_I2cWrite;
    if(I2cWrite(0x3052, 0x00, I2C_MODE_ADDR16_DATA8) < 0) return ERR_I2cWrite;
    if(I2cWrite(0x3055, 0x00, I2C_MODE_ADDR16_DATA8) < 0) return ERR_I2cWrite;
    if(I2cWrite(0x3036, 0x01, I2C_MODE_ADDR16_DATA8) < 0) return ERR_I2cWrite;
    if(I2cWrite(0x3047, 0x01, I2C_MODE_ADDR16_DATA8) < 0) return ERR_I2cWrite;
    if(I2cWrite(0x3049, 0x01, I2C_MODE_ADDR16_DATA8) < 0) return ERR_I2cWrite;

    if(I2cWrite(0x0100, 0x01, I2C_MODE_ADDR16_DATA8) < 0) return ERR_I2cWrite;

    QThread::msleep(500);

    return ERR_NoError;
}
