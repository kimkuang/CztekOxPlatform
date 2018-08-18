
#include "VcmDriver_AK7371.h"

VcmDriver_AK7371::VcmDriver_AK7371(uint i2cAddr) :
    VcmDriver(i2cAddr)
{
}

int VcmDriver_AK7371::VcmInitialize()
{
	//ID PIN 0  VCM slaveID= 0x1c,eFlashID = 0xa0
	//ID PIN 1  VCM salveID = 0X18,eFlashID = 0xb0;
    QString strLog = "";
    uchar AFVal = 0x00;
    if (ERR_NoError != m_channelController->WriteContinuousI2c(m_i2cAddr, 100, 0x02, 1, &AFVal, sizeof(AFVal)))
    {
        strLog.sprintf("VcmDriver_AK7371::%s---I2cWrite Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
}

int VcmDriver_AK7371::VcmReadCode(uint &value)
{
    (void)value;
	//USHORT ecode=0;
	//USHORT Msb =0;
	//USHORT Lsb =0;
	//BOOL bFlag = FALSE;
	////bFlag = I2cWrite(sensorId, 0x02, 0x02, 0);
	////0x02 Control bit0--PD 0 normal ,1 power down;
	////             bit1---Ringing control mode 0 Direct, 1Ringing control mode
	//bFlag = I2cRead(sensorId, 0x03, &Msb, 0);
	//bFlag = I2cRead(sensorId,0x04,&Lsb,0);//0-255
	//ecode = ((Msb&0x03)<<8) + Lsb;
	//*code = ecode;
	return 0;
}

int VcmDriver_AK7371::VcmWriteCode(uint value)
{
    if (value >= 1023)
    {
        value = 1023;
    }

    uint slaveAddr = m_i2cAddr;
    uint speedkHz = 100;

    uchar AFVal[2] = {0};
    AFVal[0] = (value >> 2) & 0xFF; //H
    AFVal[1] = (value << 6) & 0xFF; //L
    if (ERR_NoError != m_channelController->WriteContinuousI2c(slaveAddr, speedkHz, 0x00, 1, AFVal, sizeof(AFVal)))
    {
        return ERR_Failed;
    }

    QThread::msleep(100);

    uchar RdAFVal[2] = {0};
    if (ERR_NoError != m_channelController->ReadContinuousI2c(slaveAddr, speedkHz, 0x86, 1, RdAFVal, sizeof(RdAFVal)))
    {
        return ERR_Failed;
    }

    USHORT ReadCodeVal = (RdAFVal[1] >> 6) + (RdAFVal[0] << 2);

    if (ReadCodeVal < (value * 0.8))
    {
        return ERR_Failed;
    }

    if (value == 0)
    {
        QThread::msleep(100);
    }

    return ERR_NoError;
}


