#include "GC2375.h"
#include <QThread>
#include <QVector>
#include "czcmtidefs.h"
#include "macrodefs.h"


/** Exposure Reg Addr Define **/
#define EXPOSURE_REG_H_ADDR         0x03
#define EXPOSURE_REG_L_ADDR         0x04

/** Analog Gain Reg Addr Define **/
#define SENSOR_GAIN_REG_ADDR      0xB6

GC2375::GC2375(const T_SensorSetting &sensorSetting) :
    ImageSensor(sensorSetting)
{
}

#if 0
int GC2375:: ReadReg(USHORT startAddr, USHORT endAddr, USHORT *buf,USHORT page)
{
	return 1;
}
int GC2375::WriteReg(USHORT startAddr, USHORT endAddr, USHORT *buf,USHORT page)
{
	return 1;
}

int GC2375::GetTemperature(USHORT &temperature)
{
	return 1;
}
int GC2375::GetFuseID(CString &FuseID) 
{
	return 1;
}
int GC2375::Init()
{
	return 1;
}
int GC2375::ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg)
{
	return 1;
}
int GC2375::ApplyLsc(BYTE* raw8,int width,int height,int nLSCTarget,int ob,UCHAR* nLenCReg,int nLenCRegCount,int LSCGroup)
{
	return 1;
}

int GC2375::spcCal(USHORT *imgBuf,short *pSPC)
{
	return 1;
}
int GC2375::ApplySpc(short *pSPC)
{
	return 1;
}
#endif

int GC2375::GetTemperature(int &temperature)
{
    (void)temperature;

    return ERR_NoError;
}

int GC2375::GetSensorFuseId(std::string &fuseId, bool bUseMasterI2C)
{
    (void)fuseId;
    (void)bUseMasterI2C;

    return ERR_NoError;
}

int GC2375::ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg)
{
    (void)rg;
    (void)bg;
    (void)Typical_rg;
    (void)Typical_bg;

    return ERR_NoError;
}

int GC2375::GetSensorExposure(uint &value)
{
#if 1
    uchar buf[] = {0, 0};
    int ec = i2cReadContinuous(EXPOSURE_REG_H_ADDR, 1, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("GC2375::Call ReadContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    value = MAKEWORD(buf[0], buf[1]);
    return ERR_NoError;
#else
	USHORT ExpHigh;
	USHORT ExpLow;
	if(I2cRead(0x03,&ExpHigh,0) != 1) return 0;
	if(I2cRead(0x04,&ExpLow,0) != 1) return 0;
	exp=(ExpHigh<<8)+(ExpLow&0xFF);
	return 1;
#endif
} 

int GC2375::SetSensorExposure(uint value)
{
#if 1
    uchar buf[] = {MSB(value), LSB(value)};
    int ec = i2cWriteContinuous(EXPOSURE_REG_H_ADDR, 1, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("GC2375::Call WriteContinuousI2c() failed[%2]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ERR_NoError;
#else
	USHORT ExpHigh=exp>>8;
	USHORT ExpLow=exp&0xFF;

	if(I2cWrite(0x03,ExpHigh,0) != 1) return 0;
	if(I2cWrite(0x04,ExpLow,0) != 1) return 0;
	return 1;
#endif
}


int GC2375::GetSensorGain(uint &value)
{
#if 1
    uchar buf = 0;
    int ec = i2cReadContinuous(SENSOR_GAIN_REG_ADDR, 1, &buf, sizeof(buf));
    if (ec < 0) {
        QString strLog = QString("GC2375::Call ReadContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    value = buf;
    return ERR_NoError;
#else
	USHORT GainHigh;
	if(I2cRead(0xB6,&GainHigh,0) != 1) return 0;
	gain=GainHigh;
	return 1;
#endif
} 
int GC2375::SetSensorGain(uint value, bool bMultiple)
{
#if 1
    /*if (true == bMultiple)      //1x,2x,4x,8x,16x
    {
        value = 1024 - (1024 / value);
        //return SetSensorGain(value, false);
    }

    if (value > 960)    //Gain: 0 ~ 960
    {
        value = 960;
    }*/
    (void)bMultiple;

    uchar buf = (uchar)value;
    int ec = i2cWriteContinuous(SENSOR_GAIN_REG_ADDR, 1, &buf, sizeof(buf));
    if (ec < 0) {
        QString strLog = QString("GC2375::Call WriteContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ERR_NoError;
#else
	if(I2cWrite(0xB6,gain&0xff,0) != 1) return 0;
	return 1;
#endif
} 
