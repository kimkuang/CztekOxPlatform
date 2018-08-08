#include "S5K5E8YX.h"
#include <QThread>
#include <QVector>
#include "czcmtidefs.h"
#include "macrodefs.h"
#include <QDebug>
#include <QString>

// 参考文档《SEC_S5K5E8YX_EVT0_Application Note_Ver.0.05.pdf》
// 参考OTP读写操作的文档《S5K5E8YX_OTP_write_read_guide_v0.0_20150206.pdf》
#define GAIN_DEFAULT                0x0100

#define REG_CTL_RW_ADDR             0x0A00
#define INIT_CMD                    (0x00)
#define R_CMD                       (0x01 << 0)
#define W_CMD                       (0x03 << 0)
#define PENDING_ERROR               (0x01 << 2)

#define REG_ERR_FLGA_ADDR           0x0A01
#define R_RDY                       (0x01 << 0)
#define W_RDY                       (0x01 << 1)
#define DATA_CORRUPTED              (0x01 << 2)
#define IMPROPY_USAGE               (0x01 << 3)

#define REG_PAGE_SEL_ADDR           0x0A02

#define REG_DATA_START_ADDR         0x0A04
#define REG_DATA_END_ADDR           0x0A43
#define PAGE_LEN                    (REG_DATA_END_ADDR - REG_DATA_START_ADDR + 1)

/** Fuse ID Reg Addr Define **/
#define FUSE_ID_PAGE                0x00
#define FUSE_ID_ADDR_START          0x0A04
#define FUSE_ID_ADDR_END            0x0A0B
#define FUSE_ID_LEN                 (FUSE_ID_ADDR_END - FUSE_ID_ADDR_START + 1)

/**
  * AWB Gain的顺序:
  * 结合OV16B10的手册《OV16B10-Preliminary-Specification-COB_Version-1-0_huawei.pdf》中P81的Table53、P80的figure5-6的pixel pattern、P140的Table6-19
  * Pixel的顺序是BGGR，所以猜测P140的Table6-19的AwbGain0~AwbGain3的顺序是和BGGR的顺序一一对应的。
  * 因此，S5K5E8YX的Pixel的顺序也是GRBG，而AwbGain的地址是0x020E~0x0215，顺序也是GRBG。
  * ----20180414
 **/
#define AWB_GAIN_GR_REG_H_ADDR                                          0x020E
#define AWB_GAIN_GR_REG_L_ADDR                                          0x020F

#define AWB_GAIN_R_REG_H_ADDR                                           0x0210
#define AWB_GAIN_R_REG_L_ADDR                                           0x0211

#define AWB_GAIN_B_REG_H_ADDR                                           0x0212
#define AWB_GAIN_B_REG_L_ADDR                                           0x0213

#define AWB_GAIN_GB_REG_H_ADDR                                          0x0214
#define AWB_GAIN_GB_REG_L_ADDR                                          0x0215

/** Exposure Reg Addr Define **/
#define EXPOSURE_REG_H_ADDR                                             0x0202
#define EXPOSURE_REG_L_ADDR                                             0x0203

/** Anglog Gain Reg Addr Define **/
#define SENSOR_GAIN_REG_H_ADDR                                          0x0204
#define SENSOR_GAIN_REG_L_ADDR                                          0x0205


S5K5E8YX::S5K5E8YX(const T_SensorSetting &sensorSetting) :
    ImageSensor(sensorSetting)
{
}

int S5K5E8YX::GetTemperature(int &temperature)
{
    (void)temperature;
    return ERR_NoError;
}

int S5K5E8YX::GetSensorFuseId(std::string &fuseId, bool bUseMasterI2C)
{
    (void)bUseMasterI2C;
    uchar tmpBuff[FUSE_ID_LEN] = {0};
    int ec = readOtpData(FUSE_ID_ADDR_START, FUSE_ID_ADDR_END, tmpBuff, FUSE_ID_PAGE);
    if (ec < 0) {
        QString strLog = QString("Read S5K5E8YX FuseId failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    QString str = "";
    fuseId = "";
    for (uint i = 0; i < ARRAY_SIZE(tmpBuff); i++) {
        str = QString::asprintf("%02X", tmpBuff[i]);
        fuseId += str.toStdString();
    }

    return ERR_NoError;
}

int S5K5E8YX::ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg)
{
    QString strLog = "";
    if(rg == 0 || bg == 0 || Typical_rg == 0 || Typical_bg == 0)
    {
        return ERR_Failed;
    }

	USHORT r_ratio, b_ratio;

	r_ratio = 512.0 * (Typical_rg) /(rg) + 0.5;
	b_ratio = 512.0 * (Typical_bg) /(bg) + 0.5;

	if( !r_ratio || !b_ratio)
	{
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

    if (ERR_NoError != I2cWrite(AWB_GAIN_GR_REG_H_ADDR, G_GAIN, I2C_MODE_ADDR16_DATA16))
    {
        strLog = QString::asprintf("S5K5E8YX::%s Error---I2cWrite AWB_GAIN_GR_REG_H_ADDR Fail\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    if (ERR_NoError != I2cWrite(AWB_GAIN_R_REG_H_ADDR, R_GAIN, I2C_MODE_ADDR16_DATA16))
    {
        strLog = QString::asprintf("S5K5E8YX::%s Error---I2cWrite AWB_GAIN_R_REG_H_ADDR Fail\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    if (ERR_NoError != I2cWrite(AWB_GAIN_B_REG_H_ADDR, B_GAIN, I2C_MODE_ADDR16_DATA16))
    {
        strLog = QString::asprintf("S5K5E8YX::%s Error---I2cWrite AWB_GAIN_B_REG_H_ADDR Fail\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    if (ERR_NoError != I2cWrite(AWB_GAIN_GB_REG_H_ADDR, G_GAIN, I2C_MODE_ADDR16_DATA16))
    {
        strLog = QString::asprintf("S5K5E8YX::%s Error---I2cWrite AWB_GAIN_GB_REG_H_ADDR Fail\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
}

int S5K5E8YX::GetSensorExposure(uint &value)
{
    uchar buf[] = {0, 0};
    int ec = i2cReadContinuous(EXPOSURE_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("S5K5E8YX::Call ReadContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    value = MAKEWORD(buf[0], buf[1]);
    return ERR_NoError;
} 

int S5K5E8YX::SetSensorExposure(uint value)
{
    if (value > 0x07bf)
    {
        value = 0x07bf;     //防止5E8过暴花屏
    }
    uchar buf[] = {MSB(value), LSB(value)};
    int ec = i2cWriteContinuous(EXPOSURE_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("S5K5E8YX::Call WriteContinuousI2c() failed[%2]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ERR_NoError;
} 

int S5K5E8YX::GetSensorGain(uint &value)
{
    uchar buf[] = {0, 0};
    int ec = i2cReadContinuous(SENSOR_GAIN_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("S5K5E8YX::Call ReadContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    value = MAKEWORD(buf[0], buf[1]);
    return ERR_NoError;
}

int S5K5E8YX::SetSensorGain(uint value, bool bMultiple)
{
    if (true == bMultiple)
    {
        value = value * 0x20;
        //return SetSensorGain(value, false);
    }
    if (value > 512)
    {
        value = 512;  // 0 ~ 512
    }

    uchar buf[] = {MSB(value), LSB(value)};
    int ec = i2cWriteContinuous(SENSOR_GAIN_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("S5K5E8YX::Call WriteContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ERR_NoError;
}

int S5K5E8YX::readOtpData(ushort startAddr, ushort endAddr, uchar *buf, ushort page)
{
    if (nullptr == buf) {
        qCritical("S5K5E8YX::%s Error---buf is NULL", __FUNCTION__);
        return ERR_InvalidParameter;
    }

    int bufferLen = endAddr - startAddr + 1;
    if (bufferLen > PAGE_LEN) {
        qCritical("S5K5E8YX::%s Error---nLen:%d is outside PAGE_LEN:%d", __FUNCTION__, bufferLen, PAGE_LEN);
        return ERR_InvalidParameter;
    }

    int ec = ERR_NoError;
    if ((ec = I2cWrite(0x0A00, 0x04, I2C_MODE_ADDR16_DATA8)) < 0) //make initial state
        return ec;
    if ((ec = I2cWrite(0x0A02, page, I2C_MODE_ADDR16_DATA8)) < 0) // select otp page
        return ec;
    if ((ec = I2cWrite(0x0A00, 0x01, I2C_MODE_ADDR16_DATA8)) < 0) //set read mode of NVM controller Interface1
        return ec;
    QThread::msleep(2); // to wait Tmin = 47us(the time to transfer 1page data from OTP to buffer)

    //read otp data to buff
    ec = i2cReadContinuous(startAddr, 2, buf, bufferLen);
    if (ec < 0) {
        qCritical("S5K5E8YX::Call i2cReadContinuous() failed[%d]", ec);
        return ec;
    }

    // disable NVM controller
    ec = I2cWrite(0x0A00, 0x0, I2C_MODE_ADDR16_DATA8);
    if (ec < 0) {
        qCritical("S5K5E8YX:: Call I2cWrite() failed[%d]", ec);
        return ec;
    }

    return ec;
}
