
#define IMG_DRV_IMX398

#include "IMX398.h"
#include <QThread>
#include <QVector>
#include "czcmtidefs.h"
#include "macrodefs.h"

IMX398::IMX398(const T_SensorSetting &sensorSetting) :
    ImageSensor(sensorSetting)
{
}

int IMX398::GetTemperature(int &temperature)
{
    (void)temperature;

    return ERR_NoError;
}

int IMX398::GetSensorFuseId(std::string &fuseId, bool bUseMasterI2C)
{
    (void)bUseMasterI2C;
    uchar tmpBuff[FUSE_ID_LEN] = {0};
    int ec = readOtpData(FUSE_ID_ADDR_START, FUSE_ID_ADDR_END, tmpBuff, FUSE_ID_PAGE);
    if (ec < 0) {
        QString strLog = QString("Read IMX398 FuseId failed[%1]").arg(ec);
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

int IMX398::ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg)
{
    ushort r_ratio, b_ratio;
    r_ratio = 512 * (Typical_rg) /(rg);
    b_ratio = 512 * (Typical_bg) /(bg);

    ushort R_GAIN;
    ushort B_GAIN;
    ushort Gr_GAIN;
    ushort Gb_GAIN;
    ushort G_GAIN;

    if(r_ratio >= 512 )
    {
        if(b_ratio>=512)
        {
            R_GAIN = (ushort)(GAIN_DEFAULT * r_ratio / 512);
            G_GAIN = GAIN_DEFAULT;
            B_GAIN = (ushort)(GAIN_DEFAULT * b_ratio / 512);
        }
        else
        {
            R_GAIN =  (ushort)(GAIN_DEFAULT * r_ratio / b_ratio);
            G_GAIN = (ushort)(GAIN_DEFAULT * 512 / b_ratio);
            B_GAIN = GAIN_DEFAULT;
        }
    }
    else
    {
        if(b_ratio >= 512)
        {
            R_GAIN = GAIN_DEFAULT;
            G_GAIN =(ushort)(GAIN_DEFAULT * 512 / r_ratio);
            B_GAIN =(ushort)(GAIN_DEFAULT *  b_ratio / r_ratio);

        }
        else
        {
            Gr_GAIN = (ushort)(GAIN_DEFAULT * 512 / r_ratio );
            Gb_GAIN = (ushort)(GAIN_DEFAULT * 512 / b_ratio );

            if(Gr_GAIN >= Gb_GAIN)
            {
                R_GAIN = GAIN_DEFAULT;
                G_GAIN = (ushort)(GAIN_DEFAULT * 512 / r_ratio );
                B_GAIN = (ushort)(GAIN_DEFAULT * b_ratio / r_ratio);
            }
            else
            {
                R_GAIN =  (ushort)(GAIN_DEFAULT * r_ratio / b_ratio );
                G_GAIN = (ushort)(GAIN_DEFAULT * 512 / b_ratio );
                B_GAIN = GAIN_DEFAULT;
            }
        }
    }

    QVector<ST_RegData> vtRegData;
    vtRegData << ST_RegData(GAIN_RED_HIGH_ADDR,        MSB(R_GAIN))
              << ST_RegData(GAIN_RED_LOW_ADDR,         LSB(R_GAIN))
              << ST_RegData(GAIN_GREEN1_HIGH_ADDR,     MSB(G_GAIN))
              << ST_RegData(GAIN_GREEN1_LOW_ADDR,      LSB(G_GAIN))
              << ST_RegData(GAIN_GREEN2_HIGH_ADDR,     MSB(G_GAIN))
              << ST_RegData(GAIN_GREEN2_LOW_ADDR,      LSB(G_GAIN))
              << ST_RegData(GAIN_BLUE_HIGH_ADDR,       MSB(B_GAIN))
              << ST_RegData(GAIN_BLUE_LOW_ADDR,        LSB(B_GAIN));
    return writeRegisters(vtRegData, I2C_MODE_ADDR16_DATA8);
}

int IMX398::SetSensorExposure(uint value)
{
    uchar buf[] = {MSB(value), LSB(value)};
    int ec = i2cWriteContinuous(EXPOSURE_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("IMX398::Call WriteContinuousI2c() failed[%2]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ERR_NoError;
}

int IMX398::SetSensorGain(uint value, bool bMultiple)
{
    if (true == bMultiple)
    {
        value = 1024 - (1024 / value);
        //return SetSensorGain(value, false);
    }

    if (value > 978)
    {
        value = 978;        // 0 ~ 978
    }

    uchar buf[] = {MSB(value), LSB(value)};
    int ec = i2cWriteContinuous(SENSOR_GAIN_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("IMX398::Call WriteContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ERR_NoError;
}

int IMX398::GetSensorExposure(uint &value)
{
    uchar buf[] = {0, 0};
    int ec = i2cReadContinuous(EXPOSURE_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("IMX398::Call ReadContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    value = MAKEWORD(buf[0], buf[1]);
    return ERR_NoError;
}

int IMX398::GetSensorGain(uint &value)
{
    uchar buf[] = {0, 0};
    int ec = i2cReadContinuous(SENSOR_GAIN_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("IMX398::Call ReadContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    value = MAKEWORD(buf[0], buf[1]);
    return ERR_NoError;
}

int IMX398::readOtpData(ushort startAddr, ushort endAddr, uchar *buf, ushort page)
{
    if (nullptr == buf) {
        qCritical("IMX398::%s Error---buf is NULL", __FUNCTION__);
        return ERR_InvalidParameter;
    }

    uint bufferLen = endAddr - startAddr + 1;
    if (bufferLen > PAGE_LEN) {
        qCritical("IMX398::%s Error---nLen:%d is outside PAGE_LEN:%d", __FUNCTION__, bufferLen, PAGE_LEN);
        return ERR_InvalidParameter;
    }

    int ec = ERR_NoError;
    ec = I2cWrite(0x0A02, page, I2C_MODE_ADDR16_DATA8);     // select otp page
    ec |= I2cWrite(0x0A00, 0x01, I2C_MODE_ADDR16_DATA8);    // turn on otp read mode
    if (ec < 0) {
        qCritical("Turn on IMX398 otp read mode failed[%d]", ec);
        return ec;
    }

    bool checkFailed = true;
    ushort flag = 0;
    for (uint i = 0; i < 20; i++) { // check status repeat 20 times
        if (ERR_NoError == I2cRead(0x0A01, flag, I2C_MODE_ADDR16_DATA8)) {
            if ((flag & 0x05) == 0x05)//Fail
            {
                break;
            }
            else if ((flag & 0x01) == 0x01)
            {
                checkFailed = false;
                break;
            }
        }
        QThread::msleep(10);
    }
    if (checkFailed) {
        qCritical("Check IMX398 OTP status[0x%x] failed", flag);
        return ERR_Failed;
    }

    //read otp data to buff
    ec = i2cReadContinuous(startAddr, 2, buf, bufferLen);
    if (ec < 0) {
        qCritical("IMX398 read otp addr[0x%x] failed[%d]", startAddr, ec);
        return ec;
    }

    return ec;
}
