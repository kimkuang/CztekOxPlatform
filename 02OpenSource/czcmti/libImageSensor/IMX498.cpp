#include "IMX498.h"
#include <QThread>
#include <QVector>
#include "czcmtidefs.h"
#include "macrodefs.h"

// 参考IMX498.cpp
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

#define DPGA_USE_GLOBAL_GAIN        0x300B
#define DIGITAL_GAIN_BY_COLOR       (0x00 << 0)
#define DIGITAL_GAIN_ALL_COLOR      (0x01 << 0)

/** OTP Reg Addr Define **/
#define REG_PAGE_SEL_ADDR           0x0A02
#define REG_DATA_START_ADDR         0x0A04
#define REG_DATA_END_ADDR           0x0A43
#define PAGE_LEN                    (REG_DATA_END_ADDR - REG_DATA_START_ADDR + 1)

/** Fuse ID Reg Addr Define **/
#define FUSE_ID_PAGE                0x17
#define FUSE_ID_ADDR_START          0x0A20
#define FUSE_ID_ADDR_END            0x0A2A
#define FUSE_ID_LEN                 (FUSE_ID_ADDR_END - FUSE_ID_ADDR_START + 1)


IMX498::IMX498(const T_SensorSetting &sensorSetting) :
    ImageSensor(sensorSetting)
{
}

int IMX498::GetSensorFuseId(std::string &fuseId, bool bUseMasterI2C)
{
	(void)bUseMasterI2C;
    uchar tmpBuff[FUSE_ID_LEN] = {0};
    int ec = readOtpData(FUSE_ID_ADDR_START, FUSE_ID_ADDR_END, tmpBuff, FUSE_ID_PAGE);
    if (ec < 0) {
        QString strLog = QString("Read IMX498 FuseId failed[%1]").arg(ec);
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

int IMX498::ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg)
{
    QString strLog = "";

    if (rg == 0 || bg == 0 || Typical_rg == 0 || Typical_bg == 0)
    {
        strLog = QString::asprintf("IMX498::%s Error---rg or bg or Typical_rg or Typical_bg is zero\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    USHORT r_ratio, b_ratio;

    r_ratio = 512 * (Typical_rg) /(rg);
    b_ratio = 512 * (Typical_bg) /(bg);

    if ( !r_ratio || !b_ratio)
    {
        strLog = QString::asprintf("IMX498::%s Error---r_ratio or b_ratio is zero\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    USHORT R_GAIN;
    USHORT B_GAIN;
    USHORT Gr_GAIN;
    USHORT Gb_GAIN;
    USHORT G_GAIN;

    if(r_ratio >= 512 )
    {
        if(b_ratio>=512) 
        {
            R_GAIN = (USHORT)(GAIN_DEFAULT * r_ratio / 512);
            G_GAIN = GAIN_DEFAULT;	
            B_GAIN = (USHORT)(GAIN_DEFAULT * b_ratio / 512);
        }
        else
        {
            R_GAIN =  (USHORT)(GAIN_DEFAULT * r_ratio / b_ratio);
            G_GAIN = (USHORT)(GAIN_DEFAULT * 512 / b_ratio);
            B_GAIN = GAIN_DEFAULT;	
        }
    }
    else 			
    {
        if(b_ratio >= 512)
        {
            R_GAIN = GAIN_DEFAULT;	
            G_GAIN =(USHORT)(GAIN_DEFAULT * 512 / r_ratio);
            B_GAIN =(USHORT)(GAIN_DEFAULT *  b_ratio / r_ratio);

        } 
        else 
        {
            Gr_GAIN = (USHORT)(GAIN_DEFAULT * 512 / r_ratio );
            Gb_GAIN = (USHORT)(GAIN_DEFAULT * 512 / b_ratio );

            if(Gr_GAIN >= Gb_GAIN)
            {
                R_GAIN = GAIN_DEFAULT;
                G_GAIN = (USHORT)(GAIN_DEFAULT * 512 / r_ratio );
                B_GAIN = (USHORT)(GAIN_DEFAULT * b_ratio / r_ratio);
            } 
            else
            {
                R_GAIN =  (USHORT)(GAIN_DEFAULT * r_ratio / b_ratio );
                G_GAIN = (USHORT)(GAIN_DEFAULT * 512 / b_ratio );
                B_GAIN = GAIN_DEFAULT;
            }
        }	
    }

    QVector<ST_RegData> vtRegData;
    vtRegData << ST_RegData(DPGA_USE_GLOBAL_GAIN,      DIGITAL_GAIN_BY_COLOR)
              << ST_RegData(GAIN_RED_HIGH_ADDR,        MSB(R_GAIN))
              << ST_RegData(GAIN_RED_LOW_ADDR,         LSB(R_GAIN))
              << ST_RegData(GAIN_GREEN1_HIGH_ADDR,     MSB(G_GAIN))
              << ST_RegData(GAIN_GREEN1_LOW_ADDR,      LSB(G_GAIN))
              << ST_RegData(GAIN_GREEN2_HIGH_ADDR,     MSB(G_GAIN))
              << ST_RegData(GAIN_GREEN2_LOW_ADDR,      LSB(G_GAIN))
              << ST_RegData(GAIN_BLUE_HIGH_ADDR,       MSB(B_GAIN))
              << ST_RegData(GAIN_BLUE_LOW_ADDR,        LSB(B_GAIN));
    return writeRegisters(vtRegData, I2C_MODE_ADDR16_DATA8);
}

int IMX498::GetSensorExposure(uint &value)
{
    uchar buf[] = {0, 0};
    int ec = i2cReadContinuous(EXPOSURE_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("IMX498::Call ReadContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    value = MAKEWORD(buf[0], buf[1]);
    return ERR_NoError;
} 

int IMX498::SetSensorExposure(uint value)
{
    uchar buf[] = {MSB(value), LSB(value)};
    int ec = i2cWriteContinuous(EXPOSURE_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("IMX498::Call WriteContinuousI2c() failed[%2]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ERR_NoError;
} 

int IMX498::GetSensorGain(uint &value)
{
    uchar buf[] = {0, 0};
    int ec = i2cReadContinuous(SENSOR_GAIN_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("IMX498::Call ReadContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    value = MAKEWORD(buf[0], buf[1]);
    return ERR_NoError;
}

int IMX498::SetSensorGain(uint value, bool bMultiple)
{
    if (true == bMultiple)
    {
        value = 1024 - (1024 / value);
        //return SetSensorGain(value, false);
    }

    if (value > 960)
    {
        value = 960;
    }

    uchar buf[] = {MSB(value), LSB(value)};
    int ec = i2cWriteContinuous(SENSOR_GAIN_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("IMX498::Call WriteContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ERR_NoError;
} 

int IMX498::ApplySpc(short *pSPC)
{
    if(NULL == pSPC) return 0;

    if(I2cWrite(0x0100, 0x00, 3) != 1) return 0;
    if(I2cWrite(0x0101, 0x00, 3) != 1) return 0; //V and H are both set by this.
    for (int i=0; i<70; i++)//48
    {
        if(I2cWrite(0x7520+i, pSPC[i],   3) != 1) return 0;
        if(I2cWrite(0x7568+i, pSPC[i+70],3) != 1) return 0;
    }
    if(I2cWrite(0x0101, 0x00, 3) != 1) return 0; //setting 2
    if(I2cWrite(0x0b00, 0x00, 3) != 1) return 0;
    if(I2cWrite(0x3606, 0x00, 3) != 1) return 0;
    if(I2cWrite(0x3e3a, 0x00, 3) != 1) return 0;
    if(I2cWrite(0x3e39, 0x01, 3) != 1) return 0;

    if(I2cWrite(0x0100, 0x01, 3) != 1) return 0;

    QThread::msleep(500);
    return ERR_NoError;
}

int IMX498::readOtpData(ushort startAddr, ushort endAddr, uchar *buf, ushort page)
{
    if (nullptr == buf) {
        qCritical("IMX498::%s Error---buf is NULL", __FUNCTION__);
        return ERR_InvalidParameter;
    }

    uint bufferLen = endAddr - startAddr + 1;
    if (bufferLen > PAGE_LEN) {
        qCritical("IMX498::%s Error---nLen:%d is outside PAGE_LEN:%d", __FUNCTION__, bufferLen, PAGE_LEN);
        return ERR_InvalidParameter;
    }

    int ec = ERR_NoError;
    ec = I2cWrite(0x0A02, page, I2C_MODE_ADDR16_DATA8);     // select otp page
    ec |= I2cWrite(0x0A00, 0x01, I2C_MODE_ADDR16_DATA8);    // turn on otp read mode
    if (ec < 0) {
        qCritical("Turn on IMX498 otp read mode failed[%d]", ec);
        return ec;
    }

    bool checkFailed = true;
    ushort flag = 0;
    for (uint i = 0; i < 20; i++) { // check status repeat 20 times
        if (ERR_NoError == I2cRead(0x0A01, flag, I2C_MODE_ADDR16_DATA8)) {
            if ((flag & 0x01) == 0x01) {
                checkFailed = false;
                break;
            }
        }
        QThread::msleep(10);
    }
    if (checkFailed) {
        qCritical("Check IMX498 OTP status[0x%x] failed", flag);
        return ERR_Failed;
    }

    //read otp data to buff
    ec = i2cReadContinuous(startAddr, 2, buf, bufferLen);
    if (ec < 0) {
        qCritical("IMX498 read otp addr[0x%x] failed[%d]", startAddr, ec);
        return ec;
    }

    return ec;
}
