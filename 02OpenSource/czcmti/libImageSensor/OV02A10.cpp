
#define IMG_DRV_OV02A10

#include "OV02A10.h"
#include <QThread>
#include <QVector>
#include "czcmtidefs.h"
#include "macrodefs.h"
#include <QDebug>

OV02A10::OV02A10(const T_SensorSetting &sensorSetting) :
    ImageSensor(sensorSetting)
{
    qDebug()<<__FUNCTION__;
}

int OV02A10::GetTemperature(int &temperature)
{
    (void)temperature;

    return ERR_NoError;
}

int OV02A10::ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg)
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

    R_gain = 0x80*nR_G_gain/(nBase_gain);
    B_gain = 0x80*nB_G_gain/(nBase_gain);
    G_gain = 0x80*nG_G_gain/(nBase_gain);

    QVector<ST_RegData> vtRegData;
    ST_RegData stRegData;
    if(R_gain>0x80)
    {
        stRegData.RegAddr = 0xfd;
        stRegData.Data = 0x02;
        vtRegData.push_back(stRegData);
        stRegData.RegAddr = 0x10;
        stRegData.Data = R_gain;
        vtRegData.push_back(stRegData);
        stRegData.RegAddr = 0x01;
        stRegData.Data = 0x01;
        vtRegData.push_back(stRegData);
    }

    if(G_gain>0x80)
    {
        stRegData.RegAddr = 0xfd;
        stRegData.Data = 0x02;
        vtRegData.push_back(stRegData);
        stRegData.RegAddr = 0x13;
        stRegData.Data = G_gain;
        vtRegData.push_back(stRegData);
        stRegData.RegAddr = 0x14;
        stRegData.Data = G_gain;
        vtRegData.push_back(stRegData);
        stRegData.RegAddr = 0x01;
        stRegData.Data = 0x01;
        vtRegData.push_back(stRegData);
    }

    if(B_gain>0x80)
    {
        stRegData.RegAddr = 0xfd;
        stRegData.Data = 0x02;
        vtRegData.push_back(stRegData);
        stRegData.RegAddr = 0x11;
        stRegData.Data = B_gain;
        vtRegData.push_back(stRegData);
        stRegData.RegAddr = 0x01;
        stRegData.Data = 0x01;
        vtRegData.push_back(stRegData);
    }

    return writeRegisters(vtRegData, 0);
}

int OV02A10::GetSensorFuseId(std::string &fuseId, bool bUseMasterI2C)
{
    (void)bUseMasterI2C;

    fuseId = "";

    return ERR_NoError;
}

int OV02A10::SetSensorExposure(uint value)
{
    QString strLog = "";
    ushort regAddr, regData;
    uchar Buf[] = {(uchar)(value >> 8), (uchar)((value & 0xFF))};

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    regAddr = 0xfd;
    regData = 0x01;
    if(ERR_NoError != m_channelController->WriteDiscreteI2c(i2cParam.Addr, i2cParam.Speed * 100, 0, &regAddr, &regData, 1))
    {
        strLog = QString::asprintf("OV02A10::%s---WriteDiscreteI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    if (ERR_NoError != m_channelController->WriteContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, EXPOSURE_REG_H_ADDR, 1, Buf, sizeof(Buf)))
    {
        strLog = QString::asprintf("OV02A10::%s---WriteContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    regAddr = 0x01;
    regData = 0x01;
    if(ERR_NoError != m_channelController->WriteDiscreteI2c(i2cParam.Addr, i2cParam.Speed * 100, 0, &regAddr, &regData, 1))
    {
        strLog = QString::asprintf("OV02A10::%s---WriteDiscreteI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
}

int OV02A10::SetSensorGain(uint value, bool bMultiple)
{
    QString strLog = "";
    ushort regAddr, regData;
    if (true == bMultiple)
    {
        value = value * 16;
    }

    if (value > 248) /* 1X ~ 15.5X */
    {
        value = 248;
    }

    ushort gain = value;

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    regAddr = 0xfd;
    regData = 0x01;
    if(ERR_NoError != m_channelController->WriteDiscreteI2c(i2cParam.Addr, i2cParam.Speed * 100, 0, &regAddr, &regData, 1))
    {
        strLog = QString::asprintf("OV02A10::%s---WriteDiscreteI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    regAddr = SENSOR_GAIN_REG_ADDR;
    if(ERR_NoError != m_channelController->WriteDiscreteI2c(i2cParam.Addr, i2cParam.Speed * 100, 0, &regAddr, &gain, 1))
    {
        strLog = QString::asprintf("OV02A10::%s---WriteContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    regAddr = 0x01;
    regData = 0x01;
    if(ERR_NoError != m_channelController->WriteDiscreteI2c(i2cParam.Addr, i2cParam.Speed * 100, 0, &regAddr, &regData, 1))
    {
        strLog = QString::asprintf("OV02A10::%s---WriteDiscreteI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
}

int OV02A10::GetSensorExposure(uint &value)
{
    QString strLog = "";
    ushort regAddr, regData;
    uchar Buf[] = {0, 0};

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    regAddr = 0xfd;
    regData = 0x01;
    if(ERR_NoError != m_channelController->WriteDiscreteI2c(i2cParam.Addr, i2cParam.Speed * 100, 0, &regAddr, &regData, 1))
    {
        strLog = QString::asprintf("OV02A10::%s---WriteDiscreteI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    if (ERR_NoError != m_channelController->ReadContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, EXPOSURE_REG_H_ADDR, 1, Buf, sizeof(Buf)))
    {
        strLog = QString::asprintf("OV02A10::%s---ReadContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    value = (Buf[0] << 8) + (Buf[1]);

    return ERR_NoError;
}

int OV02A10::GetSensorGain(uint &value)
{
    QString strLog = "";
    ushort regAddr, regData;
    USHORT gain;

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    regAddr = 0xfd;
    regData = 0x01;
    if(ERR_NoError != m_channelController->WriteDiscreteI2c(i2cParam.Addr, i2cParam.Speed * 100, 0, &regAddr, &regData, 1))
    {
        strLog = QString::asprintf("OV02A10::%s---WriteDiscreteI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    regAddr = SENSOR_GAIN_REG_ADDR;
    if(ERR_NoError != m_channelController->ReadDiscreteI2c(i2cParam.Addr, i2cParam.Speed * 100, 0, &regAddr, &gain, 1))
    {
        strLog = QString::asprintf("OV02A10::%s---ReadDiscreteI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    value = gain;

    return ERR_NoError;
}


