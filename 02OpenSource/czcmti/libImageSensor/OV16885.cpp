
#define IMG_DRV_OV16885

#include "OV16885.h"
#include <QThread>
#include <QVector>
#include "czcmtidefs.h"
#include "macrodefs.h"
#include <QDebug>
#include <QString>

OV16885::OV16885(const T_SensorSetting &sensorSetting) :
    ImageSensor(sensorSetting)
{
    qDebug()<<__FUNCTION__;
}

int OV16885::GetTemperature(int &temperature)
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
     //QVector<ST_RegData> vtRegData;
     const ST_RegData RD_REG_SET_TAB[]=//手册没有说明，查看OV的其它Sensor，有的提过一下这几个寄存器，只是说明是Temperatrue Control寄存器而且，具体怎么设置没有说明.下面的值是原厂邮件发给李超的。----20180412
     {
         {0x4D00, 0x05},
         {0x4D01, 0x00},
         {0x4D02, 0xB7},
         {0x4D03, 0xCA},
         {0x4D04, 0x30},
         {0x4D05, 0x1D}
     };

     for (i = 0; i < ARRAY_SIZE(RD_REG_SET_TAB); i++)
     {
         if (ERR_NoError != I2cWrite(RD_REG_SET_TAB[i].RegAddr, RD_REG_SET_TAB[i].Data, I2C_MODE_ADDR16_DATA8)) //Bit[0]:1--trigger temperature calculation, then registers 0x4D12 and 0x4D13 will be the latched temperature value
         {
             strLog = QString::asprintf("OV24A1B::%s---i:%d I2cWrite Error---333\r\n", __FUNCTION__, i);
             m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
             return ERR_Failed;
         }

         /**
           * 李超给过来的代码中在写入上表中的每个寄存器后都要延时50ms。
           * 后来我调试发现，不延时会导致第一次温度出来的值不对，为0xFF，即-1°。
           * 进一步调试发现，只需要前三个寄存器的写入完后延时就行了，后面四个可以不要，这样能节省时间一两百毫秒时间。
           * 再次调试发现，延时由50ms改成30ms，也是可以的，进一步改下去不行。
           * ----20180419
         **/
         if (i < 3)
         {
             QThread::msleep(30);
         }
     }
#endif

     /** Step 2:设置温度采样触发寄存器 **/
     if (ERR_NoError != I2cWrite(TEMPERATURE_CTL_REG_ADDR, /*temp | */0x01, I2C_MODE_ADDR16_DATA8)) //Bit[0]:1--trigger temperature calculation, then registers 0x4D12 and 0x4D13 will be the latched temperature value
     {
         strLog = QString::asprintf("OV16885::%s---I2cWrite Error---333\r\n", __FUNCTION__);
         m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
         return ERR_Failed;
     }

     /**
       * 李超给过来的代码中，采样保持时间是1000ms，逐步调试发现给100ms就够了。
       * ----20180419
       * 发现有的Sensor延时100ms可以，有的不行，不行的读出来的温度值为0°，故延时调整为300ms。
       * ----20180419
     **/
     QThread::msleep(300);

     /** Step 3:获取温度值 **/
     if (ERR_NoError != I2cRead(TEMPERATURE_VAL_REG_ADDR, temp, I2C_MODE_ADDR16_DATA8))   //Latched temperature value, Integer Part
     {
         strLog = QString::asprintf("OV16885::%s---I2cRead Error---444\r\n", __FUNCTION__);
         m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
         return ERR_Failed;
     }

     /*
      * 按手册温度是有负值的，但我们不处理，接口也是个USHORT变量，传不了负数。温度范围：-64°~192°。
      * Val[0x4D13] > 0xC0，温度是负的，采用补码表示，Val[0x4D13] <= 0xC0，温度是正的，采用原码表示。
      * ----20180404
      */
     strLog = QString::asprintf("OV16885::%s---Temperature RegVal:0x%X\r\n", __FUNCTION__, temp);
     m_channelController->LogToWindow(strLog, qRgb(0, 0, 255));
     temperature = temp;
     if (temp > 0xC0)
     {
         /*strLog = QString::asprintf("OV16885::%s---Attention, temp:0x%X is negative, so fixed 0°\r\n", __FUNCTION__, temp);
         m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));*/
         temperature = (int)(temp - 256);   //按补码求原码
     }

     /** Step 4:退出温度采样过程 **/
     if (ERR_NoError != I2cWrite(TEMPERATURE_CTL_REG_ADDR, temp & 0xFE, I2C_MODE_ADDR16_DATA8)) //Clear Bit[0]
     {
         strLog = QString::asprintf("OV16885::%s---I2cWrite Error---555\r\n", __FUNCTION__);
         m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
         return ERR_Failed;
     }

     return ERR_NoError;
}

int OV16885::GetSensorFuseId(std::string &fuseId, bool bUseMasterI2C)
{
#if 1
    (void)bUseMasterI2C;
    QString strLog = "";
    uchar tempVal[FUSE_ID_LEN] = {0};
    if (ERR_NoError != readOtpData(FUSE_ID_ADDR_START, FUSE_ID_ADDR_END, tempVal, sizeof(tempVal) / sizeof(tempVal[0])))
    {
        strLog = QString::asprintf("OV16885::%s---ReadReg Error\r\n", __FUNCTION__);
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
#else
    FuseID = _T("");
    char section[512] = {0};
    USHORT tempVal[16];
    if(OV16885_readAll(0X6000, 0x600f,tempVal ) != 1) return 0;
    for ( int j=0x00; j<0x10; j++)
    {
        sprintf(section, "%02X",tempVal[j]);
        FuseID+=section;
    }
    return 1;
#endif
}


int OV16885::ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg)
{
    QString strLog = "";
    if (rg == 0 || bg == 0 || Typical_rg == 0 || Typical_bg == 0)
    {
        return 0;
    }

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
    //	 int DCW_Enable=OV16885_read_i2c(0x5000);
    //	 OV16885_write_i2c(0x5000,(DCW_Enable|0x02));
#if 1
    if (R_gain > 0x400)
    {
        if (ERR_NoError != I2cWrite(AWB_GAIN_R_REG_H_ADDR, MSB(R_gain), I2C_MODE_ADDR16_DATA8))
        {
            strLog = QString::asprintf("OV16885::%s---I2cWrite AWB_GAIN_R_REG_H_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, AWB_GAIN_R_REG_H_ADDR);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            return ERR_Failed;
        }
        if (ERR_NoError != I2cWrite(AWB_GAIN_R_REG_L_ADDR, LSB(R_gain), I2C_MODE_ADDR16_DATA8))
        {
            strLog = QString::asprintf("OV16885::%s---I2cWrite AWB_GAIN_R_REG_L_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, AWB_GAIN_R_REG_L_ADDR);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            return ERR_Failed;
        }
    }
    if (G_gain > 0x400)
    {
        if (ERR_NoError != I2cWrite(AWB_GAIN_GB_REG_H_ADDR, MSB(G_gain), I2C_MODE_ADDR16_DATA8))
        {
            strLog = QString::asprintf("OV16885::%s---I2cWrite AWB_GAIN_GB_REG_H_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, AWB_GAIN_GB_REG_H_ADDR);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            return ERR_Failed;
        }
        if (ERR_NoError != I2cWrite(AWB_GAIN_GB_REG_L_ADDR, LSB(G_gain), I2C_MODE_ADDR16_DATA8))
        {
            strLog = QString::asprintf("OV16885::%s---I2cWrite AWB_GAIN_GB_REG_L_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, AWB_GAIN_GB_REG_L_ADDR);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            return ERR_Failed;
        }

        if (ERR_NoError != I2cWrite(AWB_GAIN_GR_REG_H_ADDR, MSB(G_gain), I2C_MODE_ADDR16_DATA8))
        {
            strLog = QString::asprintf("OV16885::%s---I2cWrite AWB_GAIN_GR_REG_H_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, AWB_GAIN_GR_REG_H_ADDR);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            return ERR_Failed;
        }
        if (ERR_NoError != I2cWrite(AWB_GAIN_GR_REG_L_ADDR, LSB(G_gain), I2C_MODE_ADDR16_DATA8))
        {
            strLog = QString::asprintf("OV16885::%s---I2cWrite AWB_GAIN_GR_REG_L_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, AWB_GAIN_GR_REG_L_ADDR);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            return ERR_Failed;
        }
    }
    if (B_gain > 0x400)
    {
        if (ERR_NoError != I2cWrite(AWB_GAIN_B_REG_H_ADDR, MSB(B_gain), I2C_MODE_ADDR16_DATA8))
        {
            strLog = QString::asprintf("OV16885::%s---I2cWrite AWB_GAIN_B_REG_H_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, AWB_GAIN_B_REG_H_ADDR);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            return ERR_Failed;
        }
        if (ERR_NoError != I2cWrite(AWB_GAIN_B_REG_L_ADDR, LSB(B_gain), I2C_MODE_ADDR16_DATA8))
        {
            strLog = QString::asprintf("OV16885::%s---I2cWrite AWB_GAIN_B_REG_L_ADDR Reg[0x%X] Error\r\n", __FUNCTION__, AWB_GAIN_B_REG_L_ADDR);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            return ERR_Failed;
        }
    }

    return ERR_NoError;
#else
    if (R_gain > 0x400)
    {
        if(OV16885_write_i2c(0x5106,R_gain>>8) != 1) return 0;
        if(OV16885_write_i2c(0x5107,R_gain&0xff) != 1) return 0;
    }
    if (G_gain > 0x400)
    {
        if(OV16885_write_i2c(0x5102,G_gain>>8) != 1) return 0;
        if(OV16885_write_i2c(0x5103,G_gain&0xff) != 1) return 0;
        if(OV16885_write_i2c(0x5104,G_gain>>8) != 1) return 0;
        if(OV16885_write_i2c(0x5105,G_gain&0xff) != 1) return 0;
    }
    if (B_gain > 0x400)
    {
        if(OV16885_write_i2c(0x5100,B_gain>>8) != 1) return 0;
        if(OV16885_write_i2c(0x5101,B_gain&0xff) != 1) return 0;
    }
    return 1;
#endif
}


int OV16885::GetSensorExposure(uint &value)
{
    QString strLog = "";
#if 1
    uchar Buf[] = {0, 0};

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    if (ERR_NoError != m_channelController->ReadContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, EXPOSURE_REG_H_ADDR, 2, Buf, sizeof(Buf)))
    {
        strLog = QString::asprintf("OV16885::%s---ReadContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    value = MAKEWORD(Buf[0], Buf[1]);

    return ERR_NoError;
#else
    USHORT ExpHigh;
    USHORT ExpLow;
    if(I2cRead(0x3501,&ExpHigh,3) != 1) return 0;
    if(I2cRead(0x3502,&ExpLow,3) != 1) return 0;
    exp=(ExpHigh<<8)+(ExpLow&0xFF);

    return 1;
#endif
}

int OV16885::SetSensorExposure(uint value)
{
    QString strLog = "";
#if 1
    uchar Buf[] = {MSB(value), LSB(value)};

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    if (ERR_NoError != m_channelController->WriteContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, EXPOSURE_REG_H_ADDR, 2, Buf, sizeof(Buf)))
    {
        strLog = QString::asprintf("OV16885::%s---WriteContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
#else
    USHORT ExpHigh=exp>>8;
    USHORT ExpLow=exp&0xFF;
    if(I2cWrite(0x3501,ExpHigh,3) != 1) return 0;
    if(I2cWrite(0x3502,ExpLow,3) != 1) return 0;

    return 1;
#endif
}

int OV16885::GetSensorGain(uint &value)
{
    QString strLog = "";
#if 1
    uchar Buf[] = {0, 0};

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    if (ERR_NoError != m_channelController->ReadContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, SENSOR_GAIN_REG_H_ADDR, 2, Buf, sizeof(Buf)))
    {
        strLog = QString::asprintf("OV16885::%s---ReadContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    value = MAKEWORD(Buf[0], Buf[1]);

    return ERR_NoError;
#else
    USHORT GainHigh;
    USHORT GainLow;
    if(I2cRead(0x350C,&GainHigh,3) != 1) return 0;
    if(I2cRead(0x350D,&GainLow,3) != 1) return 0;
    gain=(GainHigh<<8)+(GainLow&0xFF);
    return 1;
#endif
}

int OV16885::SetSensorGain(uint value, bool bMultiple)
{
    (void)bMultiple;
    QString strLog = "";
#if 1
    if (true == bMultiple)
    {
        value = value * 256;
        //return SetSensorGain(value, false);
    }

    if (value > 4096)
    {
        value = 4096;   // (0 ~ 16)*256
    }

    uchar Buf[] = {MSB(value), LSB(value)};

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    if (ERR_NoError != m_channelController->WriteContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, SENSOR_GAIN_REG_H_ADDR, 2, Buf, sizeof(Buf)))
    {
        strLog = QString::asprintf("OV16885::%s---WriteContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
#else
    USHORT GainHigh=gain>>8;
    USHORT GainLow=gain&0xFF;
    if(I2cWrite(0x350C,GainHigh,3) != 1) return 0;
    if(I2cWrite(0x350D,GainLow,3) != 1) return 0;
    return 1;
#endif
}

#if 1
int OV16885::readOtpData(ushort startAddr, ushort endAddr, uchar *buf, ushort page)
{
    (void)page;
    QString strLog = "";

    if (startAddr > endAddr)
    {
        qDebug() << __FUNCTION__ << "  Error---startAddr larger than endAddr---startAddr:" << startAddr << " endAddr:" << endAddr;
        strLog = QString::asprintf("OV16885::%s---Error---startAddr larger than endAddr---startAddr:0x%X endAddr:0x%X", __FUNCTION__, startAddr, endAddr);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    if (nullptr == buf)
    {
        strLog = QString::asprintf("OV16885::%s---Error---buf is null", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    uchar IspCtlVal = 0x00;
    if (ERR_NoError != m_channelController->ReadContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, ISP_CTL_REG_0, 2, &IspCtlVal, sizeof(IspCtlVal)))
    {
        strLog = QString::asprintf("OV16885::%s---ReadContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    IspCtlVal &= (~OTP_EN);
    if (ERR_NoError != m_channelController->WriteContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, ISP_CTL_REG_0, 2, &IspCtlVal, sizeof(IspCtlVal)))
    {
        strLog = QString::asprintf("OV16885::%s---WriteContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    UINT nLen = endAddr - startAddr + 1;
    uchar *pBuf = new uchar[nLen];
    if (nullptr == pBuf)
    {
        strLog = QString::asprintf("OV16885::%s---new uchar Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    memset(pBuf, 0x00, nLen);
    if (ERR_NoError != m_channelController->WriteContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, startAddr, 2, pBuf, nLen))
    {
        delete []pBuf;
        strLog = QString::asprintf("OV16885::%s---WriteContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    QVector<ST_RegData> vtRegData;
    unsigned int i = 0;
    const ST_RegData RD_REG_SET_TAB[]=
    {
        {0x3D84,      0x40},
        {0x3D88,      MSB(startAddr)},
        {0x3D89,      LSB(startAddr)},
        {0x3D8A,      MSB(endAddr)},
        {0x3D8B,      LSB(endAddr)},
        {0x3D81,      0x01}
    };

    for (i = 0; i < sizeof (RD_REG_SET_TAB) / sizeof (RD_REG_SET_TAB[0]); i++)
    {
        vtRegData.push_back(RD_REG_SET_TAB[i]);
    }

    int iRet = writeRegisters(vtRegData, I2C_MODE_ADDR16_DATA8);
    if (ERR_NoError != iRet)
    {
        delete []pBuf;
        strLog = QString::asprintf("OV16B10::%s---writeRegisters Error--iRet:%d\r\n", __FUNCTION__, iRet);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return iRet;
    }
    QThread::msleep(30);

    //Read Data
    if (ERR_NoError != m_channelController->ReadContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, startAddr, 2, buf, nLen))
    {
        delete []pBuf;
        strLog = QString::asprintf("OV16885::%s---ReadContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    //clear buf
    if (ERR_NoError != m_channelController->WriteContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, startAddr, 2, pBuf, nLen))
    {
        delete []pBuf;
        strLog = QString::asprintf("OV16885::%s---WriteContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    delete []pBuf;

    //Set OTP Enable
    if (ERR_NoError != m_channelController->ReadContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, ISP_CTL_REG_0, 2, &IspCtlVal, sizeof(IspCtlVal)))
    {
        strLog = QString::asprintf("OV16885::%s---ReadContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    IspCtlVal |= OTP_EN;
    if (ERR_NoError != m_channelController->WriteContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, ISP_CTL_REG_0, 2, &IspCtlVal, sizeof(IspCtlVal)))
    {
        strLog = QString::asprintf("OV16885::%s---WriteContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
}
#else
int OV16885::OV16885_readAll(USHORT startAddr,USHORT endAddr,USHORT * out_buf)
{
    if (NULL == out_buf || endAddr < startAddr)
    {
        return 0;
    }

    USHORT tempData = 0;
    if(startAddr!=endAddr)
    {
        USHORT i;

        if(OV16885_read_i2c(0x5000,tempData) != 1) return 0;
        if(OV16885_write_i2c(0x5000,(tempData&(~0x08))) != 1) return 0;

        for (i=startAddr; i<=endAddr; i++)
        {
            if(OV16885_write_i2c(i, 0x00) != 1) return 0;
        }
        //enable partial OTP write mode
        //	tempData = OV16885_read_i2c(0x3d84);
        if(OV16885_write_i2c(0x3d84, 0x40) != 1) return 0;
        //partial mode OTP write start address
        if(OV16885_write_i2c(0x3d88,(startAddr>>8)&0xff) != 1) return 0;
        if(OV16885_write_i2c(0x3d89, startAddr&0xff) != 1) return 0;
        // partial mode OTP write end address
        if(OV16885_write_i2c(0x3d8A,(endAddr>>8)&0xff) != 1) return 0;
        if(OV16885_write_i2c(0x3d8B,endAddr&0xff) != 1) return 0;
        // read otp into buffer
        if(OV16885_write_i2c(0x3d81, 0x01) != 1) return 0;
        Sleep(30);

        //read buf

        for(i=startAddr;i<=endAddr;i++)
        {
            if(OV16885_read_i2c(i,tempData) != 1) return 0;
            out_buf[i-startAddr] = tempData;
        }

        //clear buf
        for (i=startAddr; i<=endAddr; i++)
        {
            if(OV16885_write_i2c(i, 0x00) != 1) return 0;
        }

        if(OV16885_read_i2c(0x5000,tempData) != 1) return 0;
        if(OV16885_write_i2c(0x5000,0x08|(tempData&(~0x08))) != 1) return 0;
    }
    else
    {
        if(OV16885_read_i2c(startAddr,tempData) != 1) return 0;
        out_buf[0] = tempData;
    }

    return 1;
}
#endif
