
#define IMG_DRV_OV08A10

#include "OV08A10.h"
#include <QThread>
#include <QVector>
#include "czcmtidefs.h"
#include "macrodefs.h"
#include <QDebug>

OV08A10::OV08A10(const T_SensorSetting &sensorSetting) :
    ImageSensor(sensorSetting)
{
    qDebug()<<__FUNCTION__;
}

int OV08A10::GetTemperature(int &temperature)
{
    QString strLog = "";
    USHORT temp = 0x00;
    USHORT te = 0x00;

    if (ERR_NoError != I2cRead(TEMPERATURE_CTL_REG_ADDR, temp, I2C_MODE_ADDR16_DATA8))
    {
        strLog = QString::asprintf("OV08A10::%s---I2cRead Error---\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    if (ERR_NoError != I2cWrite(TEMPERATURE_CTL_REG_ADDR, 0x01, I2C_MODE_ADDR16_DATA8))
    {
        strLog = QString::asprintf("OV08A10::%s---I2cWrite Error---\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    QThread::msleep(20);

    if (ERR_NoError != I2cRead(TEMPERATURE_VAL_REG_ADDR, te, I2C_MODE_ADDR16_DATA8))
    {
        strLog = QString::asprintf("OV08A10::%s---I2cRead Error---\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    temperature = te;

    if (ERR_NoError != I2cWrite(TEMPERATURE_CTL_REG_ADDR, temp, I2C_MODE_ADDR16_DATA8))
    {
        strLog = QString::asprintf("OV08A10::%s---I2cWrite Error---\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
}

int OV08A10::ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg)
{
    if(rg == 0 || bg == 0 || Typical_rg == 0 || Typical_bg == 0)
        return ERR_Failed;

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
        stRegData.RegAddr = 0x5106;
        stRegData.Data = R_gain >>8;
        vtRegData.push_back(stRegData);
        stRegData.RegAddr = 0x5107;
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
        stRegData.RegAddr = 0x5104;
        stRegData.Data = G_gain >>8;
        vtRegData.push_back(stRegData);
        stRegData.RegAddr = 0x5105;
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

int OV08A10::ReadReg(ushort startAddr, ushort endAddr, ushort *buf, ushort page)
{
    (void)page;
    QString strLog = "";
    if (nullptr == buf)
    {
        strLog = QString::asprintf("OV08A10::%s---Error---buf is null", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    for (int uAddr=startAddr; uAddr<=endAddr; uAddr++)
    {
        if (ERR_NoError != I2cWrite(uAddr, 0x00, 3))
        {
            return ERR_Failed;
        }
    }

    QVector<ST_RegData> vtRegData;
    ST_RegData stRegData;
    stRegData.RegAddr = 0x3d84;
    stRegData.Data = 0x40;
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
    for (int uAddr=startAddr; uAddr<=endAddr; uAddr++)
    {
        if (ERR_NoError != I2cWrite(uAddr, 0x00, 3))
        {
            return ERR_Failed;
        }
    }

    return ERR_NoError;
}


int OV08A10::GetSensorFuseId(std::string &fuseId, bool bUseMasterI2C)
{
    (void)bUseMasterI2C;
    QString strLog = "";
    ushort tempVal[FUSE_ID_LEN] = {0};
    if (ERR_NoError != ReadReg(FUSE_ID_ADDR_START, FUSE_ID_ADDR_END, tempVal))
    {
        strLog = QString::asprintf("OV08A10::%s---ReadReg Error\r\n", __FUNCTION__);
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

int OV08A10::SetSensorExposure(uint value)
{
    QString strLog = "";

    uchar Buf[] = {MSB(value), LSB(value)};

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    if (ERR_NoError != m_channelController->WriteContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, EXPOSURE_REG_H_ADDR, 2, Buf, sizeof(Buf)))
    {
        strLog = QString::asprintf("OV08A10::%s---WriteContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
}

int OV08A10::SetSensorGain(uint value, bool bMultiple)
{
    (void)bMultiple;
    QString strLog = "";

    if(value > 16)
    {
        value = 16;
    }

    uchar Buf[] = {(uchar)((value * 256) / 256), (uchar)((value * 256) % 256)};

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    if (ERR_NoError != m_channelController->WriteContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, SENSOR_GAIN_REG_H_ADDR, 2, Buf, sizeof(Buf)))
    {
        strLog = QString::asprintf("OV08A10::%s---WriteContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
}

int OV08A10::GetSensorExposure(uint &value)
{
    QString strLog = "";
    uchar Buf[] = {0, 0};

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    if (ERR_NoError != m_channelController->ReadContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, EXPOSURE_REG_H_ADDR, 2, Buf, sizeof(Buf)))
    {
        strLog = QString::asprintf("OV08A10::%s---ReadContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    value = MAKEWORD(Buf[0], Buf[1]);

    return ERR_NoError;
}

int OV08A10::GetSensorGain(uint &value)
{
    QString strLog = "";
    uchar Buf[] = {0, 0};

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    if (ERR_NoError != m_channelController->ReadContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, SENSOR_GAIN_REG_H_ADDR, 2, Buf, sizeof(Buf)))
    {
        strLog = QString::asprintf("OV08A10::%s---ReadContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    value = MAKEWORD(Buf[0], Buf[1]);

    return ERR_NoError;
}


