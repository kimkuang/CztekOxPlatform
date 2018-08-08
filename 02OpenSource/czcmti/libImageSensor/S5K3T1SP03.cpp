
#define IMG_DRV_S5K3T1SP03

#include "S5K3T1SP03.h"
#include <QThread>
#include <QVector>
#include "czcmtidefs.h"
#include "macrodefs.h"
#include <QDebug>
#include <QString>

S5K3T1SP03::S5K3T1SP03(const T_SensorSetting &sensorSetting) :
    ImageSensor(sensorSetting)
{
    qDebug()<<__FUNCTION__;
}

int S5K3T1SP03::GetTemperature(int &temperature)
{
    (void)temperature;

    return ERR_NoError;
}

int S5K3T1SP03::GetSensorFuseId(std::string &fuseId, bool bUseMasterI2C)
{
    (void)bUseMasterI2C;

    USHORT tempVal[64];
    if(I2cWrite(0x602a,0x0a02,   4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(0x6f12,0x0000,   4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(0x0a00,0x0100,   4) != ERR_NoError) return ERR_Failed;
    QThread::msleep(10);
    for(int j = 0; j < 3 ; j++)
    {
        if(I2cRead(0x0a24 + j * 2, tempVal[j], 4) != ERR_NoError) return ERR_Failed;
    }

    fuseId = "";
    for (int i = 0; i < 3; i++)
    {
        QString strFuseId = "";
        strFuseId = QString::asprintf("%04X", tempVal[i]);
        fuseId += strFuseId.toStdString();
    }
    if(I2cWrite(0x0a00,0x0000,   4) != ERR_NoError) return ERR_Failed;

    return ERR_NoError;
}

int S5K3T1SP03::ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg)
{
    if (rg == 0 || bg == 0 || Typical_rg == 0 || Typical_bg == 0)
    {
        return ERR_Failed;
    }

	USHORT r_ratio, b_ratio;

    r_ratio = 512 * (Typical_rg) / (rg);
    b_ratio = 512 * (Typical_bg) / (bg);

	USHORT R_GAIN;
	USHORT B_GAIN;
	USHORT Gr_GAIN;
	USHORT Gb_GAIN;
	USHORT G_GAIN;

    if (r_ratio >= 512 )
	{
        if (b_ratio >= 512)
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
        if (b_ratio >= 512)
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

    if(I2cWrite(0x0100,0x00,  3) != ERR_NoError) return ERR_Failed; // Stream off
    if(I2cWrite(0x3027,0x01,  3) != ERR_NoError) return ERR_Failed; // wb_En
    if(I2cWrite(0x0210,R_GAIN,4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(0x0212,B_GAIN,4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(0x020E,G_GAIN,4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(0x0214,G_GAIN,4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(0x0100,0x01,  3) != ERR_NoError) return ERR_Failed; // Stream on

    return ERR_NoError;
}

int S5K3T1SP03::GetSensorExposure(uint &value)
{
    QString strLog = "";

    uchar Buf[] = {0, 0};

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    if (ERR_NoError != m_channelController->ReadContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, EXPOSURE_REG_H_ADDR, 2, Buf, sizeof(Buf)))
    {
        strLog = QString::asprintf("S5K3T1SP03::%s---ReadContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    value = MAKEWORD(Buf[0], Buf[1]);

    return ERR_NoError;
}

int S5K3T1SP03::SetSensorExposure(uint value)
{
    QString strLog = "";

    uchar Buf[] = {MSB(value), LSB(value)};

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    if (ERR_NoError != m_channelController->WriteContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, EXPOSURE_REG_H_ADDR, 2, Buf, sizeof(Buf)))
    {
        strLog = QString::asprintf("S5K3T1SP03::%s---WriteContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
}

int S5K3T1SP03::GetSensorGain(uint &value)
{
    QString strLog = "";

    uchar Buf[] = {0, 0};

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    if (ERR_NoError != m_channelController->ReadContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, SENSOR_GAIN_REG_H_ADDR, 2, Buf, sizeof(Buf)))
    {
        strLog = QString::asprintf("S5K3T1SP03::%s---ReadContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    value = MAKEWORD(Buf[0], Buf[1]);

    return ERR_NoError;
}

int S5K3T1SP03::SetSensorGain(uint value, bool bMultiple)
{
    (void)bMultiple;
    QString strLog = "";

    if (true == bMultiple)
    {
        value = value * 0x20;
        //return SetSensorGain(value, false);
    }
    if (value > 512)
    {
        value = 512;  // 0 ~ 512
    }

    uchar Buf[] = {MSB(value), LSB(value)};

    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    if (ERR_NoError != m_channelController->WriteContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, SENSOR_GAIN_REG_H_ADDR, 2, Buf, sizeof(Buf)))
    {
        strLog = QString::asprintf("S5K3T1SP03::%s---WriteContinuousI2c() Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
}
