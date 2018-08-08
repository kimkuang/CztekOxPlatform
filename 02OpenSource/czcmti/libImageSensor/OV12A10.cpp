
#define IMG_DRV_OV12A10

#include "OV12A10.h"
#include <QThread>
#include <QVector>
#include "czcmtidefs.h"
#include "macrodefs.h"
#include <QDebug>

OV12A10::OV12A10(const T_SensorSetting &sensorSetting) :
    ImageSensor(sensorSetting)
{
    qDebug()<<__FUNCTION__;
}

int OV12A10::GetTemperature(int &temperature)
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

    /** Step 1:设置温度控制寄存器 **/
    //手册没有说明，查看OV的其它Sensor，有的提过一下这几个寄存器，只是说明是Temperatrue Control寄存器而且，
    //具体怎么设置没有说明.下面的值是原厂邮件发给李超的。----20180412
    QVector<ST_RegData> vtRegData;
    vtRegData << ST_RegData(0x301B, 0xF0)
              << ST_RegData(0x4D00, 0x03)
              << ST_RegData(0x4D01, 0xE2)
              << ST_RegData(0x4D02, 0xBB)
              << ST_RegData(0x4D03, 0x93)
              << ST_RegData(0x4D04, 0x48)
              << ST_RegData(0x4D05, 0x68);
    int iRet = writeRegisters(vtRegData, I2C_MODE_ADDR16_DATA8);
    if (ERR_NoError != iRet)
    {
        strLog = QString::asprintf("OV12A10::%s---writeRegisters Error--iRet:%d\r\n", __FUNCTION__, iRet);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return iRet;
    }

    /** Step 1:设置温度采样触发寄存器 **/
    if (ERR_NoError != I2cWrite(TEMPERATURE_CTL_REG_ADDR, /*temp | */0x00, I2C_MODE_ADDR16_DATA8)) //Bit[0]:1--trigger temperature calculation, then registers 0x4D12 and 0x4D13 will be the latched temperature value
    {
        strLog = QString::asprintf("OV12A10::%s---I2cWrite Error---333\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    QThread::msleep(100);

    /** Step 2:设置温度采样触发寄存器 **/
    if (ERR_NoError != I2cWrite(TEMPERATURE_CTL_REG_ADDR, /*temp | */0x01, I2C_MODE_ADDR16_DATA8)) //Bit[0]:1--trigger temperature calculation, then registers 0x4D12 and 0x4D13 will be the latched temperature value
    {
        strLog = QString::asprintf("OV12A10::%s---I2cWrite Error---333\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    //QThread::msleep(1000);
  /**
    * 李超给过来的代码中，采样保持时间是1000ms，逐步调试发现给100ms就够了。
    * ----20180419
    * 发现有的Sensor延时100ms可以，有的不行，不行的读出来的温度值为0°，故延时调整为300ms。
    * ----20180419
  **/
    QThread::msleep(500);

    /** Step 3:获取温度值 **/
    if (ERR_NoError != I2cRead(TEMPERATURE_VAL_REG_ADDR, temp, I2C_MODE_ADDR16_DATA8))   //Latched temperature value, Integer Part
    {
        strLog = QString::asprintf("OV12A10::%s---I2cRead Error---444\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    /*
     * 按手册温度是有负值的，但我们不处理，接口也是个USHORT变量，传不了负数。温度范围：-64°~192°。
     * Val[0x4D13] > 0xC0，温度是负的，采用补码表示，Val[0x4D13] <= 0xC0，温度是正的，采用原码表示。
     * ----20180404
     */
    strLog = QString::asprintf("OV12A10::%s---Temperature RegVal:0x%X\r\n", __FUNCTION__, temp);
    m_channelController->LogToWindow(strLog, qRgb(0, 0, 255));
    temperature = temp;
    if (temp > 0xC0)
    {
        /*strLog = QString::asprintf("OV12A10::%s---Attention, temp:0x%X is negative, so fixed 0°\r\n", __FUNCTION__, temp);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));*/
        temperature = (int)(temp - 256);   //按补码求原码
    }

    /** Step 4:退出温度采样过程 **/
    if (ERR_NoError != I2cWrite(TEMPERATURE_CTL_REG_ADDR, temp & 0xFE, I2C_MODE_ADDR16_DATA8)) //Clear Bit[0]
    {
        strLog = QString::asprintf("OV12A10::%s---I2cWrite Error---555\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    /*strLog = QString::asprintf("OV12A10::%s---temperature:%d temp:0x%X\r\n", __FUNCTION__, temperature, temp);
    m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));*/

    return ERR_NoError;
}

int OV12A10::ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg)
{
    int nR_G_gain, nB_G_gain, nG_G_gain;
    int nBase_gain;
    int R_gain, B_gain, G_gain;

    nR_G_gain = (Typical_rg*1000)/rg;
    nB_G_gain = (Typical_bg*1000)/bg;
    nG_G_gain = 1000;

    if(nR_G_gain<1000 || nB_G_gain<1000)
    {
        if(nR_G_gain < nB_G_gain)
            nBase_gain = nR_G_gain;
        else
            nBase_gain = nB_G_gain;
    }
    else
    {
        nBase_gain = nG_G_gain;
    }

    R_gain = 0x400*nR_G_gain/(nBase_gain);
    B_gain = 0x400*nB_G_gain/(nBase_gain);
    G_gain = 0x400*nG_G_gain/(nBase_gain);

    QVector<ST_RegData> vtRegData;
    ST_RegData stRegData;
    if(R_gain>0x400)
    {
        stRegData.RegAddr = 0x5104;
        stRegData.Data = R_gain >>8;
        vtRegData.push_back(stRegData);
        stRegData.RegAddr = 0x5105;
        stRegData.Data = R_gain & 0x00FF;
        vtRegData.push_back(stRegData);
    }

    if(G_gain>0x400)
    {
        stRegData.RegAddr = 0x5102;
        stRegData.Data = G_gain >>8;
        vtRegData.push_back(stRegData);
        stRegData.RegAddr = 0x5103;
        stRegData.Data = G_gain & 0x00FF;
        vtRegData.push_back(stRegData);
    }

    if(B_gain>0x400)
    {
        stRegData.RegAddr = 0x5100;
        stRegData.Data = B_gain >>8;
        vtRegData.push_back(stRegData);
        stRegData.RegAddr = 0x5101;
        stRegData.Data = B_gain & 0x00FF;
        vtRegData.push_back(stRegData);
    }

    return writeRegisters(vtRegData, 3);
}

int OV12A10::ReadReg(ushort startAddr, ushort endAddr, ushort *buf, ushort page)
{
    (void)page;
    QString strLog = "";
    if (nullptr == buf)
    {
        strLog = QString::asprintf("OV12A10::%s---Error---buf is null", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    for (ushort uAddr=startAddr; uAddr<=endAddr; uAddr++)
    {
        if (ERR_NoError != I2cWrite(uAddr, 0x00, 3))
        {
            return ERR_Failed;
        }
    }

    QVector<ST_RegData> vtRegData;
    ST_RegData stRegData;
    stRegData.RegAddr = 0x3d84;
    stRegData.Data = 0xC0;
    vtRegData.push_back(stRegData);

    stRegData.RegAddr = 0x3d88;
    stRegData.Data = (startAddr>>8)&0xff;
    vtRegData.push_back(stRegData);

    stRegData.RegAddr = 0x3d89;
    stRegData.Data = startAddr&0xff;
    vtRegData.push_back(stRegData);

    stRegData.RegAddr = 0x3d8A;
    stRegData.Data = (endAddr>>8)&0xff;
    vtRegData.push_back(stRegData);

    stRegData.RegAddr = 0x3d8B;
    stRegData.Data = endAddr&0xff;
    vtRegData.push_back(stRegData);

    stRegData.RegAddr = 0x3d81;
    stRegData.Data = 0x01;
    vtRegData.push_back(stRegData);

    int iRet = writeRegisters(vtRegData, 3);
    if (ERR_NoError != iRet)
    {
        return iRet;
    }

    QThread::msleep(30);
    //read buf
    for (ushort uAddr=startAddr; uAddr<=endAddr; uAddr++)
    {
        if (ERR_NoError != I2cRead(uAddr, buf[uAddr-startAddr], 3))
        {
            return ERR_Failed;
        }
    }

    //clear buf
    for (ushort uAddr=startAddr; uAddr<=endAddr; uAddr++)
    {
        if (ERR_NoError != I2cWrite(uAddr, 0x00, 3))
        {
            return ERR_Failed;
        }
    }

    return ERR_NoError;
}


int OV12A10::GetSensorFuseId(std::string &fuseId, bool bUseMasterI2C)
{
    (void)bUseMasterI2C;
    QString strLog = "";
    ushort tempVal[FUSE_ID_LEN] = {0};
    if (ERR_NoError != ReadReg(FUSE_ID_ADDR_START, FUSE_ID_ADDR_END, tempVal))
    {
        strLog = QString::asprintf("OV12A10::%s---ReadReg Error\r\n", __FUNCTION__);
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

int OV12A10::SetSensorExposure(uint value)
{
    QString strLog = "";
#if 1
    uchar Buf[] = {(uchar)(value >> 12), (uchar)((value >> 4) & 0xFF), (uchar)((value & 0x0F) << 4)};

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    if (ERR_NoError != m_channelController->WriteContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, EXPOSURE_REG_H_ADDR, 2, Buf, sizeof(Buf)))
    {
        strLog = QString::asprintf("OV12A10::%s---WriteContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
#else
    QVector<ST_RegData> vtRegData;
    ST_RegData stRegData;
    stRegData.RegAddr = 0x3500;
    stRegData.Data = value >> 12;
    vtRegData.push_back(stRegData);
    stRegData.RegAddr = 0x3501;
    stRegData.Data = (value>>4) & 0xFF;
    vtRegData.push_back(stRegData);
    stRegData.RegAddr = 0x3502;
    stRegData.Data = (value&0xF) << 4;
    vtRegData.push_back(stRegData);
    int iRet = writeRegisters(vtRegData, 3);

    return iRet;
#endif
}

int OV12A10::SetSensorGain(uint value, bool bMultiple)
{
    QString strLog = "";
    /** real gain, not sensor gain, 前提条件是Reg[0x3503]的Bit[2]=0 **/
    if (true == bMultiple)
    {
        value = value * 128;
        //return SetSensorGain(value, false);
    }

    if (value > 1984)
    {
        value = 1984;   // (0 ~ 15.5)*128
    }

#if 1
    uchar Buf[] = {MSB(value), LSB(value)};

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    if (ERR_NoError != m_channelController->WriteContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, SENSOR_GAIN_REG_H_ADDR, 2, Buf, sizeof(Buf)))
    {
        strLog = QString::asprintf("OV12A10::%s---WriteContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
#else
    QVector<ST_RegData> vtRegData;
    ST_RegData stRegData;
    stRegData.RegAddr = 0x3508;
    stRegData.Data = value >> 8;
    vtRegData.push_back(stRegData);
    stRegData.RegAddr = 0x3509;
    stRegData.Data = value & 0xFF;
    vtRegData.push_back(stRegData);
    int iRet = writeRegisters(vtRegData, 3);

    return iRet;
#endif
}

int OV12A10::GetSensorExposure(uint &value)
{
#if 1
    QString strLog = "";
    uchar Buf[] = {0, 0, 0};

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    if (ERR_NoError != m_channelController->ReadContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, EXPOSURE_REG_H_ADDR, 2, Buf, sizeof(Buf)))
    {
        strLog = QString::asprintf("OV12A10::%s---ReadContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    value = ((Buf[0] & 0x0F) << 12) + (Buf[1] << 4) + ((Buf[2] >> 4) & 0x0F);

    return ERR_NoError;
#else
    QVector<ST_RegData> vtRegData;
    ST_RegData stRegData;
    stRegData.RegAddr = 0x3500;
    vtRegData.push_back(stRegData);
    stRegData.RegAddr = 0x3501;
    vtRegData.push_back(stRegData);
    stRegData.RegAddr = 0x3502;
    vtRegData.push_back(stRegData);
    int iRet = readRegisters(vtRegData, 3);
    if (ERR_NoError == iRet)
    {
        value = ((vtRegData[0].Data&0x0F)<<12) + (vtRegData[1].Data<<4) + ((vtRegData[2].Data>>4)&0x0F);
    }

    return iRet;
#endif
}

int OV12A10::GetSensorGain(uint &value)
{
#if 1
    QString strLog = "";
    uchar Buf[] = {0, 0};

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    if (ERR_NoError != m_channelController->ReadContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, SENSOR_GAIN_REG_H_ADDR, 2, Buf, sizeof(Buf)))
    {
        strLog = QString::asprintf("OV12A10::%s---ReadContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    value = MAKEWORD(Buf[0], Buf[1]);

    return ERR_NoError;
#else
    QVector<ST_RegData> vtRegData;
    ST_RegData stRegData;
    stRegData.RegAddr = 0x3508;
    vtRegData.push_back(stRegData);
    stRegData.RegAddr = 0x3509;
    vtRegData.push_back(stRegData);
    int iRet = readRegisters(vtRegData, 3);
    if (ERR_NoError == iRet)
    {
        value = (vtRegData[0].Data<<8) + (vtRegData[1].Data&0xFF);
    }

    return iRet;
#endif
}


