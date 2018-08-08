
#define IMG_DRV_OV13A40

#include "OV13A40.h"
#include <QThread>
#include <QVector>
#include "czcmtidefs.h"
#include "macrodefs.h"
#include <QDebug>
#include <QString>

OV13A40::OV13A40(const T_SensorSetting &sensorSetting) :
    ImageSensor(sensorSetting)
{
    qDebug()<<__FUNCTION__;
}

int OV13A40::GetTemperature(int &temperature)
{
    QString strLog = "";
    USHORT temp = 0x00;

    /** TEMPERATURE_CTL_REG_ADDR寄存器是只写的，不可读----20180412 **/
    /*if (ERR_NoError != I2cRead(TEMPERATURE_CTL_REG_ADDR, temp, I2C_MODE_ADDR16_DATA8))
    {
        strLog = QString::asprintf("%s---I2cRead Error---111\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    if (ERR_NoError != I2cWrite(TEMPERATURE_CTL_REG_ADDR, temp & 0xFE, I2C_MODE_ADDR16_DATA8)) //Clear Bit[0]
    {
        strLog = QString::asprintf("%s---I2cWrite Error---222\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    QThread::msleep(20);*/

    #if 0
    /** Step 1:设置温度控制寄存器 **/
    unsigned int i = 0;
    QVector<ST_RegData> vtRegData;
    const ST_RegData RD_REG_SET_TAB[]=//手册没有说明，查看OV的其它Sensor，有的提过一下这几个寄存器，只是说明是Temperatrue Control寄存器而且，具体怎么设置没有说明.下面的值是原厂邮件发给李超的。----20180412
    {
        {0x4D00, 0x05},
        {0x4D01, 0x00},
        {0x4D02, 0xB7},
        {0x4D03, 0xCA},
        {0x4D04, 0x30},
        {0x4D05, 0x1D}
    };

    for (i = 0; i < sizeof (RD_REG_SET_TAB) / sizeof (RD_REG_SET_TAB[0]); i++)
    {
        vtRegData.push_back(RD_REG_SET_TAB[i]);
        QThread::msleep(50);
    }

    int iRet = writeRegisters(vtRegData, I2C_MODE_ADDR16_DATA8);
    if (ERR_NoError != iRet)
    {
        strLog = QString::asprintf("OV13A40::%s---writeRegisters Error--iRet:%d\r\n", __FUNCTION__, iRet);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return iRet;
    }
    #endif

    /** Step 2:设置温度采样触发寄存器 **/
    if (ERR_NoError != I2cWrite(TEMPERATURE_CTL_REG_ADDR, /*temp | */0x01, I2C_MODE_ADDR16_DATA8)) //Bit[0]:1--trigger temperature calculation, then registers 0x4D12 and 0x4D13 will be the latched temperature value
    {
        strLog = QString::asprintf("OV13A40::%s---I2cWrite Error---333\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    QThread::msleep(300);

    /** Step 3:获取温度值 **/
    if (ERR_NoError != I2cRead(TEMPERATURE_VAL_REG_ADDR, temp, I2C_MODE_ADDR16_DATA8))   //Latched temperature value, Integer Part
    {
        strLog = QString::asprintf("OV13A40::%s---I2cRead Error---444\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    /*
     * 按手册温度是有负值的，但我们不处理，接口也是个USHORT变量，传不了负数。温度范围：-64°~192°。
     * Val[0x4D13] > 0xC0，温度是负的，采用补码表示，Val[0x4D13] <= 0xC0，温度是正的，采用原码表示。
     * ----20180404
     */
    strLog = QString::asprintf("OV13A40::%s---Temperature RegVal:0x%X\r\n", __FUNCTION__, temp);
    m_channelController->LogToWindow(strLog, qRgb(0, 0, 255));
    temperature = temp;
    if (temp > 0xC0)
    {
        /*strLog = QString::asprintf("OV13A40::%s---Attention, temp:0x%X is negative, so fixed 0°\r\n", __FUNCTION__, temp);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));*/
        temperature = (int)(temp - 256);   //按补码求原码
    }

    /** Step 4:退出温度采样过程 **/
    if (ERR_NoError != I2cWrite(TEMPERATURE_CTL_REG_ADDR, temp & 0xFE, I2C_MODE_ADDR16_DATA8)) //Clear Bit[0]
    {
        strLog = QString::asprintf("OV13A40::%s---I2cWrite Error---555\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    /*strLog = QString::asprintf("OV13A40::%s---temperature:%d temp:0x%X\r\n", __FUNCTION__, temperature, temp);
    m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));*/

    return ERR_NoError;
}

 int OV13A40::GetSensorFuseId(std::string &fuseId, bool bUseMasterI2C)
 {
     (void)bUseMasterI2C;
     QString strLog = "";
     USHORT tempVal[FUSE_ID_LEN] = {0};
     if (ERR_NoError != ReadReg(FUSE_ID_ADDR_START, FUSE_ID_ADDR_END, tempVal))
     {
         strLog = QString::asprintf("OV13A40::%s---ReadReg Error\r\n", __FUNCTION__);
         m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
         return ERR_Failed;
     }

     fuseId = "";
     QString strFuseId = "";
     for (unsigned int j = 0; j < sizeof(tempVal) / sizeof(tempVal[0]); j++)
     {
         strFuseId = QString::asprintf("%02X", tempVal[j]);
         fuseId += strFuseId.toStdString();
     }

     return ERR_NoError;
 }

int OV13A40::ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg)
{
    QString strLog = "";
    int nR_G_gain, nB_G_gain, nG_G_gain;
    int nBase_gain;
    int R_gain, B_gain, G_gain;

    nR_G_gain = (Typical_rg * 1000) / rg;
    nB_G_gain = (Typical_bg * 1000) / bg;
    nG_G_gain = 1000;

    if (nR_G_gain < 1000 || nB_G_gain < 1000)
    {
        if (nR_G_gain < nB_G_gain)
        {
            nBase_gain = nR_G_gain;
        }
        else
        {
            nBase_gain = nB_G_gain;
        }
    }
    else
    {
        nBase_gain = nG_G_gain;
    }

    R_gain = 0x400 * nR_G_gain / (nBase_gain);
    B_gain = 0x400 * nB_G_gain / (nBase_gain);
    G_gain = 0x400 * nG_G_gain / (nBase_gain);

    if (R_gain > 0x400)
    {
        if (ERR_NoError != I2cWrite(AWB_GAIN_R_REG_H_ADDR, MSB(R_gain), I2C_MODE_ADDR16_DATA8))
        {
            strLog = QString::asprintf("%s---I2cWrite AWB_GAIN_R_REG_H_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, AWB_GAIN_R_REG_H_ADDR);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            return ERR_Failed;
        }
        if (ERR_NoError != I2cWrite(AWB_GAIN_R_REG_L_ADDR, LSB(R_gain), I2C_MODE_ADDR16_DATA8))
        {
            strLog = QString::asprintf("%s---I2cWrite AWB_GAIN_R_REG_L_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, AWB_GAIN_R_REG_L_ADDR);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            return ERR_Failed;
        }
    }
    if (G_gain > 0x400)
    {
        if (ERR_NoError != I2cWrite(AWB_GAIN_GB_REG_H_ADDR, MSB(G_gain), I2C_MODE_ADDR16_DATA8))
        {
            strLog = QString::asprintf("%s---I2cWrite AWB_GAIN_GB_REG_H_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, AWB_GAIN_GB_REG_H_ADDR);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            return ERR_Failed;
        }
        if (ERR_NoError != I2cWrite(AWB_GAIN_GB_REG_L_ADDR, LSB(G_gain), I2C_MODE_ADDR16_DATA8))
        {
            strLog = QString::asprintf("%s---I2cWrite AWB_GAIN_GB_REG_L_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, AWB_GAIN_GB_REG_L_ADDR);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            return ERR_Failed;
        }

        if (ERR_NoError != I2cWrite(AWB_GAIN_GR_REG_H_ADDR, MSB(G_gain), I2C_MODE_ADDR16_DATA8))
        {
            strLog = QString::asprintf("%s---I2cWrite AWB_GAIN_GR_REG_H_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, AWB_GAIN_GR_REG_H_ADDR);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            return ERR_Failed;
        }
        if (ERR_NoError != I2cWrite(AWB_GAIN_GR_REG_L_ADDR, LSB(G_gain), I2C_MODE_ADDR16_DATA8))
        {
            strLog = QString::asprintf("%s---I2cWrite AWB_GAIN_GR_REG_L_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, AWB_GAIN_GR_REG_L_ADDR);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            return ERR_Failed;
        }
    }
    if (B_gain > 0x400)
    {
        if (ERR_NoError != I2cWrite(AWB_GAIN_B_REG_H_ADDR, MSB(B_gain), I2C_MODE_ADDR16_DATA8))
        {
            strLog = QString::asprintf("%s---I2cWrite AWB_GAIN_B_REG_H_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, AWB_GAIN_B_REG_H_ADDR);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            return ERR_Failed;
        }
        if (ERR_NoError != I2cWrite(AWB_GAIN_B_REG_L_ADDR, LSB(B_gain), I2C_MODE_ADDR16_DATA8))
        {
            strLog = QString::asprintf("%s---I2cWrite AWB_GAIN_B_REG_L_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, AWB_GAIN_B_REG_L_ADDR);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            return ERR_Failed;
        }
    }

    return ERR_NoError;
}


int OV13A40::GetSensorExposure(uint &value)
{
    QString strLog = "";
#if 1
    uchar Buf[] = {0, 0};

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    if (ERR_NoError != m_channelController->ReadContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, EXPOSURE_REG_H_ADDR, 2, Buf, sizeof(Buf)))
    {
        strLog = QString::asprintf("OV13A40::%s---ReadContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    value = MAKEWORD(Buf[0], Buf[1]);

    return ERR_NoError;
#else
    USHORT ExpHigh;
    USHORT ExpLow;

    if (ERR_NoError != I2cRead(EXPOSURE_REG_H_ADDR, ExpHigh, I2C_MODE_ADDR16_DATA8))
    {
        strLog = QString::asprintf("%s---I2cRead EXPOSURE_REG_H_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, EXPOSURE_REG_H_ADDR);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    if (ERR_NoError != I2cRead(EXPOSURE_REG_L_ADDR, ExpLow, I2C_MODE_ADDR16_DATA8))
    {
        strLog = QString::asprintf("%s---I2cRead EXPOSURE_REG_L_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, EXPOSURE_REG_L_ADDR);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    value = MAKEWORD(ExpHigh, ExpLow);

    return ERR_NoError;
#endif
}

int OV13A40::SetSensorExposure(uint value)
{
    QString strLog = "";
#if 1
    uchar Buf[] = {MSB(value), LSB(value)};

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    if (ERR_NoError != m_channelController->WriteContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, EXPOSURE_REG_H_ADDR, 2, Buf, sizeof(Buf)))
    {
        strLog = QString::asprintf("OV13A40::%s---WriteContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
#else
    USHORT ExpHigh = MSB(value);
    USHORT ExpLow = LSB(value);

    if (ERR_NoError != I2cWrite(EXPOSURE_REG_H_ADDR, ExpHigh, I2C_MODE_ADDR16_DATA8))
    {
        strLog = QString::asprintf("%s---I2cWrite EXPOSURE_REG_H_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, EXPOSURE_REG_H_ADDR);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    if (ERR_NoError != I2cWrite(EXPOSURE_REG_L_ADDR, ExpLow, I2C_MODE_ADDR16_DATA8))
    {
        strLog = QString::asprintf("%s---I2cWrite EXPOSURE_REG_L_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, EXPOSURE_REG_L_ADDR);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
#endif
}

int OV13A40::GetSensorGain(uint &value)
{
    QString strLog = "";
#if 1
    uchar Buf[] = {0, 0};

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    if (ERR_NoError != m_channelController->ReadContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, SENSOR_GAIN_REG_H_ADDR, 2, Buf, sizeof(Buf)))
    {
        strLog = QString::asprintf("OV13A40::%s---ReadContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    value = MAKEWORD(Buf[0], Buf[1]);

    return ERR_NoError;
#else
    USHORT GainHigh;
    USHORT GainLow;

    if (ERR_NoError != I2cRead(SENSOR_GAIN_REG_H_ADDR, GainHigh, I2C_MODE_ADDR16_DATA8))
    {
        strLog = QString::asprintf("%s---I2cRead SENSOR_GAIN_REG_H_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, SENSOR_GAIN_REG_H_ADDR);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    if (ERR_NoError != I2cRead(SENSOR_GAIN_REG_L_ADDR, GainLow, I2C_MODE_ADDR16_DATA8))
    {
        strLog = QString::asprintf("%s---I2cRead SENSOR_GAIN_REG_L_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, SENSOR_GAIN_REG_L_ADDR);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    value = MAKEWORD(GainHigh, GainLow);

    return ERR_NoError;
#endif
}

int OV13A40::SetSensorGain(uint value, bool bMultiple)
{
    (void)bMultiple;
    QString strLog = "";
#if 1
    /** OV13A40和OV13A10的Gain设置方式一样，故参考《13A10_Gain_LookupTable.xlsx》里面Real Gain那两列里面的计算公式 **/
    if (true == bMultiple)
    {
        value = value * 256;
        //return SetSensorGain(value, false);
    }

    if (value > 3968)
    {
        value = 3968;   // (0 ~ 15.5)*256
    }

    uchar Buf[] = {MSB(value), LSB(value)};

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    if (ERR_NoError != m_channelController->WriteContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, SENSOR_GAIN_REG_H_ADDR, 2, Buf, sizeof(Buf)))
    {
        strLog = QString::asprintf("OV13A40::%s---WriteContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
#else
    USHORT GainHigh = MSB(value);
    USHORT GainLow = LSB(value);

    if (ERR_NoError != I2cWrite(SENSOR_GAIN_REG_H_ADDR, GainHigh, I2C_MODE_ADDR16_DATA8))
    {
        strLog = QString::asprintf("%s---I2cWrite EXPOSURE_REG_H_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, SENSOR_GAIN_REG_H_ADDR);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    if (ERR_NoError != I2cWrite(SENSOR_GAIN_REG_L_ADDR, GainLow, I2C_MODE_ADDR16_DATA8))
    {
        strLog = QString::asprintf("%s---I2cWrite EXPOSURE_REG_L_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, SENSOR_GAIN_REG_L_ADDR);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
#endif
}

int OV13A40::WriteReg(USHORT startAddr, USHORT endAddr, USHORT *buf, USHORT page)
{
#if 1
    (void)page;
    QString strLog = "";
    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;

    if (startAddr > endAddr)
    {
        qDebug() << __FUNCTION__ << "  Error---startAddr larger than endAddr---startAddr:" << startAddr << " endAddr:" << endAddr;
        strLog = QString::asprintf("%s---Error---startAddr larger than endAddr---startAddr:0x%X endAddr:0x%X", __FUNCTION__, startAddr, endAddr);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    if (nullptr == buf)
    {
        strLog = QString::asprintf("OV13A40::%s---Error---buf is null", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    UINT nLen = endAddr - startAddr + 1;
    uchar *ptmpBuf = new uchar[nLen];
    if (nullptr == ptmpBuf)
    {
        qDebug() << __FUNCTION__ << "  Error---new ptmpBuf Error---" << startAddr << " endAddr:" << endAddr << " nLen:" << nLen;
        strLog = QString::asprintf("%s---Error---new ptmpBuf Error---startAddr:0x%X endAddr:0x%X nLen:%d", __FUNCTION__, startAddr, endAddr, nLen);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    for (unsigned int i = 0; i < nLen; i++)
    {
        ptmpBuf[i] = buf[i];
    }

    int ec = m_channelController->WriteContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, startAddr, 2, ptmpBuf, nLen);
    if (ec < 0)
    {
        qDebug() << __FUNCTION__ << "  WriteContinuousI2c Error";
        strLog = QString::asprintf("%s---WriteContinuousI2c Error", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        //return ec;
    }

    delete []ptmpBuf;

    return ec;
#else
    QString strLog = "";
    for (int uAddr = startAddr; uAddr <= endAddr; uAddr++)
    {
        if (ERR_NoError != I2cWrite(uAddr, buf[uAddr - startAddr], I2C_MODE_ADDR16_DATA8))
        {
            qDebug() << __FUNCTION__ << "  I2CWrite Error uAddr:" << uAddr;
            strLog = QString::asprintf("%s---I2CWrite Error uAddr:0x%X", __FUNCTION__, uAddr);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            return ERR_Failed;
        }
    }
#endif

    return ERR_NoError;
}

int OV13A40::ReadReg(USHORT startAddr, USHORT endAddr, USHORT *buf, USHORT page)
{
    (void)page;
    QString strLog = "";

    if (startAddr > endAddr)
    {
        qDebug() << __FUNCTION__ << "  Error---startAddr larger than endAddr---startAddr:" << startAddr << " endAddr:" << endAddr;
        strLog = QString::asprintf("OV13A40::%s---Error---startAddr larger than endAddr---startAddr:0x%X endAddr:0x%X", __FUNCTION__, startAddr, endAddr);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    if (nullptr == buf)
    {
        strLog = QString::asprintf("OV13A40::%s---Error---buf is null", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    UINT nLen = endAddr - startAddr + 1;
    uchar *ptmpBuf = new uchar[nLen];
    if (nullptr == ptmpBuf)
    {
        qDebug() << __FUNCTION__ << "  Error---new ptmpBuf Error---" << startAddr << " endAddr:" << endAddr << " nLen:" << nLen;
        strLog = QString::asprintf("OV13A40::%s---Error---new ptmpBuf Error---startAddr:0x%X endAddr:0x%X nLen:%d", __FUNCTION__, startAddr, endAddr, nLen);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    /*memset(ptmpBuf, 0x00, nLen);
    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    int ec = m_channelController->WriteContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, startAddr, 2, ptmpBuf, nLen);
    if (ec < 0)
    {
        qDebug() << __FUNCTION__ << "  WriteContinuousI2c Error";
        strLog = QString::asprintf("OV13A40::%s---WriteContinuousI2c Error---i2cParam.Addr:0x%X i2cParam.Speed:%d", __FUNCTION__, i2cParam.Addr, i2cParam.Speed);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        delete []ptmpBuf;
        return ec;
    }*/

    QVector<ST_RegData> vtRegData;
    unsigned int i = 0;
    const ST_RegData RD_REG_SET_TAB[]=
    {
        //{SOFT_RESET_MODE_CTRL,      SOFT_RESET_EN},
        {OTP_MODE_CTRL,             OTP_MODE_MANUAL_RW},
        {OTP_LOAD_EN_CTRL,          OTP_SW_LOAD_SETTING_EN_RW | OTP_HW_LOAD_SETTING_EN_RW | OTP_PWR_UP_LOAD_DATA_EN_RW | OTP_BIST_EN_RW},
        {OTP_START_ADDR_HIGH,       MSB(startAddr)},
        {OTP_START_ADDR_LOW,        LSB(startAddr)},
        {OTP_END_ADDR_HIGH,         MSB(endAddr)},
        {OTP_END_ADDR_LOW,          LSB(endAddr)},
        {OTP_LOAD_CTRL,             OTP_LOAD_EN_WO}
        //{STANDBY_MODE_CTRL,         STANDBY_MODE_CTRL_PWR_UP}
    };

    for (i = 0; i < sizeof (RD_REG_SET_TAB) / sizeof (RD_REG_SET_TAB[0]); i++)
    {
        vtRegData.push_back(RD_REG_SET_TAB[i]);
    }

    int iRet = writeRegisters(vtRegData, I2C_MODE_ADDR16_DATA8);
    if (ERR_NoError != iRet)
    {
        strLog = QString::asprintf("OV13A40::%s---writeRegisters Error--iRet:%d\r\n", __FUNCTION__, iRet);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        delete []ptmpBuf;
        return iRet;
    }

    //read buf
    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    iRet = m_channelController->ReadContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, startAddr, 2, ptmpBuf, nLen);
    if (ERR_NoError != iRet)
    {
        qDebug() << __FUNCTION__ << "  ReadContinuousI2c Error";
        strLog = QString::asprintf("OV13A40::%s---ReadContinuousI2c Error", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        delete []ptmpBuf;
        return ERR_Failed;
    }

    for (unsigned int i = 0; i < nLen; i++)
    {
        buf[i] = ptmpBuf[i];
    }

    //clear buf
    /*memset(ptmpBuf, 0x00, nLen);
    i2cParam = m_sensorSetting.I2cParam;
    iRet = m_channelController->WriteContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, startAddr, 2, ptmpBuf, nLen);
    if (ERR_NoError != iRet)
    {
        qDebug() << __FUNCTION__ << "  222---WriteContinuousI2c Error";
        strLog = QString::asprintf("OV13A40::%s---222---WriteContinuousI2c Error", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        delete []ptmpBuf;
        return ERR_Failed;
    }*/

	delete []ptmpBuf;

    return ERR_NoError;
}
