  

#include "HI1333.h"
#include <QThread>
#include <QVector>
#include "czcmtidefs.h"
#include "macrodefs.h"


HI1333::HI1333(const T_SensorSetting &sensorSetting) :
    ImageSensor(sensorSetting)
{
}

int HI1333::GetTemperature(int &temperature)
{
    (void)temperature;

	return 1;
}

int HI1333::GetSensorFuseId(std::string &fuseId, bool bMultiple)
{
    (void)bMultiple;
    uchar tmpBuff[FUSE_ID_LEN] = {0};
    int ec = readOtpData(FUSE_ID_ADDR_START, FUSE_ID_ADDR_END, tmpBuff, /*FUSE_ID_PAGE*/0);
    if (ec < 0) {
        QString strLog = QString("HI1333::%1 Error---Read HI1333 FuseId failed[%2]").arg(__FUNCTION__).arg(ec);
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

int HI1333::ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg)
{
    if (rg == 0 || bg == 0 || Typical_rg == 0 || Typical_bg == 0)
    {
        return ERR_Failed;
    }

	int R_gain = 0x200;
	int G_gain = 0x200;
	int B_gain = 0x200;
	
	R_gain = 0x200 * Typical_rg / rg;
	B_gain = 0x200 * Typical_bg / bg;
	if (R_gain < B_gain)
	{
		if (R_gain < 0x200)
		{
			B_gain = 0x200 * B_gain / R_gain;
			G_gain = 0x200 * G_gain / R_gain;
			R_gain = 0x200;
		}
	}
	else
	{
		if (B_gain < 0x200)
		{
			R_gain = 0x200 * R_gain / B_gain;
			G_gain = 0x200 * G_gain / B_gain;
			B_gain = 0x200;
		}
	}

    QVector<ST_RegData> vtRegData;
    vtRegData << ST_RegData(GAIN_RED_HIGH_ADDR,        MSB(R_gain))
              << ST_RegData(GAIN_RED_LOW_ADDR,         LSB(R_gain))
              << ST_RegData(GAIN_GREEN1_HIGH_ADDR,     MSB(G_gain))
              << ST_RegData(GAIN_GREEN1_LOW_ADDR,      LSB(G_gain))
              << ST_RegData(GAIN_GREEN2_HIGH_ADDR,     MSB(G_gain))
              << ST_RegData(GAIN_GREEN2_LOW_ADDR,      LSB(G_gain))
              << ST_RegData(GAIN_BLUE_HIGH_ADDR,       MSB(B_gain))
              << ST_RegData(GAIN_BLUE_LOW_ADDR,        LSB(B_gain));
    return writeRegisters(vtRegData, I2C_MODE_ADDR16_DATA8);
}

int HI1333::GetSensorExposure(uint &value)
{
#if 1
    uchar buf[] = {0, 0};
    int ec = i2cReadContinuous(EXPOSURE_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("HI1333::%1 Error---Call ReadContinuousI2c() failed[%2]").arg(__FUNCTION__).arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    value = MAKEWORD(buf[0], buf[1]);
    return ERR_NoError;
#else
	USHORT ExpHigh;
	USHORT ExpLow;
	
	if(I2cRead(HI1333_REG_EXP_TIM_H, &ExpHigh, HI1333_I2C_MODE) != 1) return 0;
	if(I2cRead(HI1333_REG_EXP_TIM_L, &ExpLow, HI1333_I2C_MODE) != 1) return 0;
	exp = (ExpHigh << 8) + (ExpLow & 0xFF);

	return 1;
#endif
} 

int HI1333::SetSensorExposure(uint value)
{
#if 1
    uchar buf[] = {MSB(value), LSB(value)};
    int ec = i2cWriteContinuous(EXPOSURE_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("HI1333::Call i2cWriteContinuous() failed[%2]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ERR_NoError;
#else
	USHORT ExpHigh = exp >> 8;
	USHORT ExpLow = exp & 0xFF;
	 
	if(I2cWrite(HI1333_REG_EXP_TIM_H, ExpHigh, HI1333_I2C_MODE) != 1) return 0;
	if(I2cWrite(HI1333_REG_EXP_TIM_L, ExpLow, HI1333_I2C_MODE) != 1) return 0;

	return 1;
#endif
} 

int HI1333::GetSensorGain(uint &value)
{
#if 1
    uchar buf[] = {0, 0};
    int ec = i2cReadContinuous(SENSOR_GAIN_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("HI1333::%1 Error---Call ReadContinuousI2c() failed[%2]").arg(__FUNCTION__).arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    value = MAKEWORD(buf[0], buf[1]);
    return ERR_NoError;
#else
	USHORT GainHigh;
	USHORT GainLow;

	//Analog Gain, not Digital Gain
	if(I2cRead(HI1333_REG_AGAIN_H, &GainHigh, HI1333_I2C_MODE) != 1) return 0;
	if(I2cRead(HI1333_REG_AGAIN_L, &GainLow, HI1333_I2C_MODE) != 1) return 0;
	 
	gain = (GainHigh << 8) + (GainLow & 0xFF);

	return 1;
#endif
}

int HI1333::SetSensorGain(uint value, bool bMultiple)
{
#if 1
    if (true == bMultiple)      //1x~16x
    {
        value = (value - 1) * 16;
        //return SetSensorGain(value, false);
    }

    if (value > 240)    //Gain: 0 ~ 240
    {
        value = 240;
    }

    uchar buf[] = {MSB(value), LSB(value)};
    int ec = i2cWriteContinuous(SENSOR_GAIN_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("HI1333::%1 Error---Call WriteContinuousI2c() failed[%2]").arg(__FUNCTION__).arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ERR_NoError;
#else
	USHORT GainHigh = gain >> 8;
	USHORT GainLow = gain & 0xFF;

	//Analog Gain, not Digital Gain
	if(I2cWrite(HI1333_REG_AGAIN_H, GainHigh, HI1333_I2C_MODE) != 1) return 0;
	if(I2cWrite(HI1333_REG_AGAIN_L, GainLow, HI1333_I2C_MODE) != 1) return 0;
	
	return 1;
#endif
} 

#if 0
/******************************* 以下四个函数主要用来向Sensor里面写入AWB和LSC校准数据 *******************************/
void HI1333::HI1333_I2C_WtOtpBy(USHORT addr, unsigned char val)
{
	I2cWrite(HI1333_REG_OTP_ADDR_H, MSB(addr), HI1333_I2C_MODE); // start address H
	I2cWrite(HI1333_REG_OTP_ADDR_L, LSB(addr), HI1333_I2C_MODE); // start address L
	I2cWrite(HI1333_REG_OTP_CMD, HI1333_OTP_WT_MODE, HI1333_I2C_MODE); // OTP write mode
	I2cWrite(HI1333_REG_OTP_WDATA, val, HI1333_I2C_MODE); // otp data write
	Sleep(5); // delay 5ms
	/*If(data[i] != IIC_Read(0x0718)) // verify
	{
		Fail_address[k]=i;
		k++;
	}*/
}

unsigned char HI1333::HI1333_I2C_RdOtpBy(USHORT addr)
{
	unsigned char RdData;
	
	I2cWrite(HI1333_REG_OTP_ADDR_H, MSB(addr), HI1333_I2C_MODE); // start address H
	I2cWrite(HI1333_REG_OTP_ADDR_L, LSB(addr), HI1333_I2C_MODE); // start address L
	I2cWrite(HI1333_REG_OTP_CMD, HI1333_OTP_RD_MODE, HI1333_I2C_MODE); // OTP read mode
	RdData = I2cRead(HI1333_REG_OTP_RDATA, HI1333_I2C_MODE); // OTP data read

	return RdData;
}

BOOL HI1333::HI1333_I2C_WtOtpData(USHORT addr, __IN__ unsigned char *pBuf, unsigned int nLen)
{
	unsigned int i;

	if (NULL == pBuf)
	{
		return FALSE;
	}
	
	I2cWrite(HI1333_REG_OTP_ADDR_H, MSB(addr), HI1333_I2C_MODE); // start address H
	I2cWrite(HI1333_REG_OTP_ADDR_L, LSB(addr), HI1333_I2C_MODE); // start address L
	I2cWrite(HI1333_REG_OTP_CMD, HI1333_OTP_WT_MODE, HI1333_I2C_MODE); // OTP write mode
	for (i = 0; i < nLen; i++)
	{
		I2cWrite(HI1333_REG_OTP_WDATA, pBuf[i], HI1333_I2C_MODE); // otp data write
		Sleep(5); // delay 5ms
		/*If(data[i] != IIC_Read(0x0718)) // verify
		{
			Fail_address[k]=i;
			k++;
		}*/
	}

	return TRUE;
}

BOOL HI1333::HI1333_I2C_RdOtpData(USHORT addr, __OUT__ unsigned char *pBuf, unsigned int nLen)
{
	unsigned int i;

	if (NULL == pBuf)
	{
		return FALSE;
	}
	
	I2cWrite(HI1333_REG_OTP_ADDR_H, MSB(addr), HI1333_I2C_MODE); // start address H
	I2cWrite(HI1333_REG_OTP_ADDR_L, LSB(addr), HI1333_I2C_MODE); // start address L
	I2cWrite(HI1333_REG_OTP_CMD, HI1333_OTP_RD_MODE, HI1333_I2C_MODE); // OTP read mode
	for (i = 0; i < nLen; i++)
	{
		pBuf[i] = I2cRead(HI1333_REG_OTP_RDATA, HI1333_I2C_MODE); // OTP data read
	}

	return TRUE;
}
#endif

int HI1333::writeOtpData(ushort startAddr, ushort endAddr, __IN__ uchar *pBuf, ushort page)
{
#if 1
    (void)page;
    QString strLog = "";
    int ec = ERR_NoError;

    if (nullptr == pBuf) {
        qCritical("HI1333::%s Error---pBuf is NULL", __FUNCTION__);
        strLog = QString::asprintf("HI1333::%s I2cWrite() pBuf is NULL", __FUNCTION__);
        m_channelController->LogToWindow(strLog, LogRed);
        return ERR_InvalidParameter;
    }

    uint bufferLen = endAddr - startAddr + 1;

    //1).首先进行I2C写的操作：
    if (ERR_NoError != I2cWrite(0x0A02, 0x01, HI1333_I2C_MODE))
    {
        strLog = QString::asprintf("HI1333::%s I2cWrite() Reg[0x0A02] Error", __FUNCTION__);
        m_channelController->LogToWindow(strLog, LogRed);
        return ERR_Failed;
    }

    if (ERR_NoError != I2cWrite(0x0A00, 0x00, HI1333_I2C_MODE))
    {
        strLog = QString::asprintf("HI1333::%s I2cWrite() Reg[0x0A00] Error", __FUNCTION__);
        m_channelController->LogToWindow(strLog, LogRed);
        return ERR_Failed;
    }

    QThread::msleep(10);

    QVector<ST_RegData> vtRegData;
    vtRegData << ST_RegData(0x0F02, 0x00)
              << ST_RegData(0x071A, 0x01)
              << ST_RegData(0x071B, 0x09)
              << ST_RegData(0x0d04, 0x01)
              << ST_RegData(0x0d00, 0x07)
              << ST_RegData(0x003E, 0x10)
              << ST_RegData(0x0A00, 0x01)
              << ST_RegData(HI1333_REG_OTP_ADDR_H, MSB(startAddr))
              << ST_RegData(HI1333_REG_OTP_ADDR_L, LSB(startAddr))
              << ST_RegData(HI1333_REG_OTP_CMD, HI1333_OTP_WT_MODE);

    ec = writeRegisters(vtRegData, I2C_MODE_ADDR16_DATA8);
    if (ec < 0) {
        qCritical("writeRegisters failed[%d]", ec);
        strLog = QString::asprintf("HI1333::%s writeRegisters() Error", __FUNCTION__);
        m_channelController->LogToWindow(strLog, LogRed);
        return ec;
    }

    //2).然后进行I2C写的操作(举例说明)：
    for (uint i = 0; i < bufferLen; i++)
    {
        if (ERR_NoError != I2cWrite(HI1333_REG_OTP_WDATA, pBuf[i], HI1333_I2C_MODE))
        {
            strLog = QString::asprintf("HI1333::%s pBuf[i]() Reg[%d] Error", __FUNCTION__, i);
            m_channelContext->ChannelController->LogToWindow(strLog, LogRed);
            return ERR_Failed;
        }

        QThread::msleep(5);
    }

    //3).最后进行I2C写的操作（使图像恢复预览）：
    if (ERR_NoError != I2cWrite(0x0A00, 0x00, HI1333_I2C_MODE))	 // stand by on
    {
        strLog = QString::asprintf("HI1333::%s I2cWrite() Reg[0x0A00] Error---222", __FUNCTION__);
        m_channelController->LogToWindow(strLog, LogRed);
        return ERR_Failed;
    }
    QThread::msleep(10);
    if (ERR_NoError != I2cWrite(0x003E, 0x00, HI1333_I2C_MODE))   // display mode
    {
        strLog = QString::asprintf("HI1333::%s I2cWrite() Reg[0x003E] Error", __FUNCTION__);
        m_channelController->LogToWindow(strLog, LogRed);
        return ERR_Failed;
    }
    if (ERR_NoError != I2cWrite(0x0A00, 0x01, HI1333_I2C_MODE))	 // stand by off
    {
        strLog = QString::asprintf("HI1333::%s I2cWrite() Reg[0x0A00] Error---333", __FUNCTION__);
        m_channelController->LogToWindow(strLog, LogRed);
        return ERR_Failed;
    }

    return ERR_NoError;

#else
	unsigned int i;

	if (NULL == pBuf)
	{
		return FALSE;
	}

	//1).首先进行I2C写的操作：
	if(I2cWrite(0x0A02, 0x01, HI1333_I2C_MODE) != 1) return 0;
	if(I2cWrite(0x0A00, 0x00, HI1333_I2C_MODE) != 1) return 0;
	Sleep(10); 
	if(I2cWrite(0x0F02, 0x00, HI1333_I2C_MODE) != 1) return 0;
	if(I2cWrite(0x071A, 0x01, HI1333_I2C_MODE) != 1) return 0;
	if(I2cWrite(0x071B, 0x09, HI1333_I2C_MODE) != 1) return 0;
	if(I2cWrite(0x0d04, 0x01, HI1333_I2C_MODE) != 1) return 0;
	if(I2cWrite(0x0d00, 0x07, HI1333_I2C_MODE) != 1) return 0;
	if(I2cWrite(0x003E, 0x10, HI1333_I2C_MODE) != 1) return 0;
	if(I2cWrite(0x0A00, 0x01, HI1333_I2C_MODE) != 1) return 0;

	if(I2cWrite(HI1333_REG_OTP_ADDR_H, MSB(addr), HI1333_I2C_MODE) != 1) return 0;   // start address H 
	if(I2cWrite(HI1333_REG_OTP_ADDR_L, LSB(addr), HI1333_I2C_MODE) != 1) return 0;   //start address L 
	if(I2cWrite(HI1333_REG_OTP_CMD, HI1333_OTP_WT_MODE, HI1333_I2C_MODE) != 1) return 0;	 //OTP write mode
	 
	//2).然后进行I2C读的操作(举例说明)：
	for ( i = 0; i < nLen; i++)
	{ 
		if(I2cWrite(HI1333_REG_OTP_WDATA, pBuf[i], HI1333_I2C_MODE) != 1) return 0; // otp data write
		Sleep(5); // delay 5ms
	}

	//3).最后进行I2C写的操作（使图像恢复预览）：
	if(I2cWrite(0x0A00, 0x00, HI1333_I2C_MODE) != 1) return 0;  // stand by on 
	Sleep(10); 
	if(I2cWrite(0x003E, 0x00, HI1333_I2C_MODE) != 1) return 0;	 // display mode 
	if(I2cWrite(0x0A00, 0x01, HI1333_I2C_MODE) != 1) return 0;	 // stand by off 

	return TRUE;
#endif
}

int HI1333::readOtpData(ushort startAddr, ushort endAddr, __OUT__ uchar *pBuf, ushort page)
{
#if 1
    (void)page;
    QString strLog = "";
    int ec = ERR_NoError;

    if (nullptr == pBuf) {
        qCritical("HI1333::%s Error---pBuf is NULL", __FUNCTION__);
        strLog = QString::asprintf("HI1333::%s Error---pBuf is NULL", __FUNCTION__);
        m_channelController->LogToWindow(strLog, LogRed);
        return ERR_InvalidParameter;
    }

    uint bufferLen = endAddr - startAddr + 1;

    //1).首先进行I2C写的操作：
    if (ERR_NoError != I2cWrite(0x0A02, 0x01, HI1333_I2C_MODE))
    {
        strLog = QString::asprintf("HI1333::%s I2cWrite() Reg[0x0A02] Error", __FUNCTION__);
        m_channelController->LogToWindow(strLog, LogRed);
        return ERR_Failed;
    }

    if (ERR_NoError != I2cWrite(0x0A00, 0x00, HI1333_I2C_MODE))
    {
        strLog = QString::asprintf("HI1333::%s I2cWrite() Reg[0x0A00] Error", __FUNCTION__);
        m_channelController->LogToWindow(strLog, LogRed);
        return ERR_Failed;
    }

    QThread::msleep(10);

    QVector<ST_RegData> vtRegData;
    vtRegData << ST_RegData(0x0F02, 0x00)
              << ST_RegData(0x071A, 0x01)
              << ST_RegData(0x071B, 0x09)
              << ST_RegData(0x0d04, 0x01)
              << ST_RegData(0x0d00, 0x07)
              << ST_RegData(0x003E, 0x10)
              << ST_RegData(0x0A00, 0x01)
              << ST_RegData(HI1333_REG_OTP_ADDR_H, MSB(startAddr))
              << ST_RegData(HI1333_REG_OTP_ADDR_L, LSB(startAddr))
              << ST_RegData(HI1333_REG_OTP_CMD, HI1333_OTP_RD_MODE);

    ec = writeRegisters(vtRegData, I2C_MODE_ADDR16_DATA8);
    if (ec < 0) {
        qCritical("writeRegisters failed[%d]", ec);
        strLog = QString::asprintf("HI1333::%s writeRegisters() Error", __FUNCTION__);
        m_channelController->LogToWindow(strLog, LogRed);
        return ec;
    }

    //2).然后进行I2C读的操作(举例说明)：
    USHORT u16Tmp;
    for (uint i = 0; i < bufferLen; i++)
    {
        if (ERR_NoError != I2cRead(HI1333_REG_OTP_RDATA, u16Tmp, HI1333_I2C_MODE))
        {
            strLog = QString::asprintf("HI1333::%s I2cRead() Reg[%d] Error", __FUNCTION__, i);
            m_channelController->LogToWindow(strLog, LogRed);
            return ERR_Failed;
        }

        pBuf[i] = LSB(u16Tmp);
    }

    //3).最后进行I2C写的操作（使图像恢复预览）：
    if (ERR_NoError != I2cWrite(0x0A00, 0x00, HI1333_I2C_MODE))	 // stand by on
    {
        strLog = QString::asprintf("HI1333::%s I2cWrite() Reg[0x0A00] Error---222", __FUNCTION__);
        m_channelController->LogToWindow(strLog, LogRed);
        return ERR_Failed;
    }
    QThread::msleep(10);
    if (ERR_NoError != I2cWrite(0x003E, 0x00, HI1333_I2C_MODE))   // display mode
    {
        strLog = QString::asprintf("HI1333::%s I2cWrite() Reg[0x003E] Error", __FUNCTION__);
        m_channelController->LogToWindow(strLog, LogRed);
        return ERR_Failed;
    }
    if (ERR_NoError != I2cWrite(0x0A00, 0x01, HI1333_I2C_MODE))	 // stand by off
    {
        strLog = QString::asprintf("HI1333::%s I2cWrite() Reg[0x0A00] Error---333", __FUNCTION__);
        m_channelController->LogToWindow(strLog, LogRed);
        return ERR_Failed;
    }

    return ERR_NoError;
#else
	USHORT u16Tmp;
	unsigned int i;

	if (NULL == pBuf)
	{
		return FALSE;
	}

	//1).首先进行I2C写的操作：
	if(I2cWrite(0x0A02, 0x01, HI1333_I2C_MODE) != 1) return 0;
	if(I2cWrite(0x0A00, 0x00, HI1333_I2C_MODE) != 1) return 0;
    QThread::msleep(10);
	if(I2cWrite(0x0F02, 0x00, HI1333_I2C_MODE) != 1) return 0;
	if(I2cWrite(0x071A, 0x01, HI1333_I2C_MODE) != 1) return 0;
	if(I2cWrite(0x071B, 0x09, HI1333_I2C_MODE) != 1) return 0;
	if(I2cWrite(0x0d04, 0x01, HI1333_I2C_MODE) != 1) return 0;
	if(I2cWrite(0x0d00, 0x07, HI1333_I2C_MODE) != 1) return 0;
	if(I2cWrite(0x003E, 0x10, HI1333_I2C_MODE) != 1) return 0;
	if(I2cWrite(0x0A00, 0x01, HI1333_I2C_MODE) != 1) return 0;
	 
    if(I2cWrite(HI1333_REG_OTP_ADDR_H, MSB(startAddr), HI1333_I2C_MODE) != 1) return 0;   // start address H
    if(I2cWrite(HI1333_REG_OTP_ADDR_L, LSB(startAddr), HI1333_I2C_MODE) != 1) return 0;   //start address L
	if(I2cWrite(HI1333_REG_OTP_CMD, HI1333_OTP_RD_MODE, HI1333_I2C_MODE) != 1) return 0;   //OTP Read Enable
	 
	//2).然后进行I2C读的操作(举例说明)：
	for ( i = 0; i < nLen; i++)
	{
		if(I2cRead(HI1333_REG_OTP_RDATA, &u16Tmp, HI1333_I2C_MODE) != 1) return 0;
		pBuf[i] = LSB(u16Tmp);
	}

	//3).最后进行I2C写的操作（使图像恢复预览）：
	if(I2cWrite(0x0A00, 0x00, HI1333_I2C_MODE) != 1) return 0;	 // stand by on 
	Sleep(10); 
	if(I2cWrite(0x003E, 0x00, HI1333_I2C_MODE) != 1) return 0;   // display mode 
	if(I2cWrite(0x0A00, 0x01, HI1333_I2C_MODE) != 1) return 0;	 // stand by off 

	return TRUE;
#endif
}


