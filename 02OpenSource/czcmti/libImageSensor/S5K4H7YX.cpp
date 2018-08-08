#include "S5K4H7YX.h"
#include <QThread>
#include <QVector>
#include "czcmtidefs.h"
#include "macrodefs.h"

// 参考资料《SEC_S5K4H7YX_EVT0.0_Data Sheet_Ver.0.00 (Preliminary).pdf》
// 参考资料《[4H7]OTP_Read_Write_Guide_REV0.1_20170707.pdf》
#define GAIN_DEFAULT                0x0100

#define REG_OP_MODE_ADDR            0x0100  //Operating Mode Registers---0:Software Standby, 1:Streaming(Active Video)--I2C Communiction with sensor is possible, Core is power on.
#define SENSOR_OP_MODE_MASK         0x01
#define SENSOR_SOFTWARE_STANDBY     (0x00 << 0)
#define SENSOR_STREAMING_ON         (0x01 << 0)

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
  * 因此，S5K4H7的Pixel的顺序也是GRBG，而AwbGain的地址是0x020E~0x0215，顺序也是GRBG。
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
#define EXPOSURE_REG_H_ADDR         0x0202
#define EXPOSURE_REG_L_ADDR         0x0203

/** Anglog Gain Reg Addr Define **/
#define SENSOR_GAIN_REG_H_ADDR      0x0204
#define SENSOR_GAIN_REG_L_ADDR      0x0205

S5K4H7YX::S5K4H7YX(const T_SensorSetting &sensorSetting) :
    ImageSensor(sensorSetting)
{
}

int S5K4H7YX::GetTemperature(int &temperature)
{
    (void)temperature;

    return ERR_NoError;
}

int S5K4H7YX::GetSensorFuseId(std::string &fuseId, bool bUseMasterI2C)
{
    (void)bUseMasterI2C;

    uchar buf[FUSE_ID_LEN] = {0};
    int ec = readOtpData(FUSE_ID_ADDR_START, FUSE_ID_ADDR_END, buf, FUSE_ID_PAGE);
    if (ec < 0) {
        QString strLog = QString("S5K4H7YX: Read FuseId failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    fuseId = "";
    for (int i = 0; i < FUSE_ID_LEN; i++) {
        QString strFuseId = "";
        strFuseId = QString::asprintf("%02X", buf[i]);
        fuseId += strFuseId.toStdString();
    }

    return ERR_NoError;
}

int S5K4H7YX::ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg)
{
    QString strLog = "";
	USHORT r_ratio, b_ratio;

	r_ratio = 512.0 * (Typical_rg) /(rg) + 0.5;
	b_ratio = 512.0 * (Typical_bg) /(bg) + 0.5;

    if (!r_ratio || !b_ratio)
	{
        strLog = QString::asprintf("S5K4H7YX::%s Error---r_ratio:%d or b_ratio:%d is zero\r\n", __FUNCTION__, r_ratio, b_ratio);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
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

    if (ERR_NoError != I2cWrite(0x3C0F, 0x00, 3))
    {
        strLog = QString::asprintf("S5K4H7YX::%s Error---I2cWrite Reg[0x3C0F] Fail\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    if (ERR_NoError != I2cWrite(AWB_GAIN_GR_REG_H_ADDR, G_GAIN, I2C_MODE_ADDR16_DATA16))
    {
        strLog = QString::asprintf("S5K4H7YX::%s Error---I2cWrite AWB_GAIN_GR_REG_H_ADDR Fail\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    if (ERR_NoError != I2cWrite(AWB_GAIN_R_REG_H_ADDR, R_GAIN, I2C_MODE_ADDR16_DATA16))
    {
        strLog = QString::asprintf("S5K4H7YX::%s Error---I2cWrite AWB_GAIN_R_REG_H_ADDR Fail\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    if (ERR_NoError != I2cWrite(AWB_GAIN_B_REG_H_ADDR, B_GAIN, I2C_MODE_ADDR16_DATA16))
    {
        strLog = QString::asprintf("S5K4H7YX::%s Error---I2cWrite AWB_GAIN_B_REG_H_ADDR Fail\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    if (ERR_NoError != I2cWrite(AWB_GAIN_GB_REG_H_ADDR, G_GAIN, I2C_MODE_ADDR16_DATA16))
    {
        strLog = QString::asprintf("S5K4H7YX::%s Error---I2cWrite AWB_GAIN_GB_REG_H_ADDR Fail\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
}

int S5K4H7YX::ApplyLsc(BYTE* raw8, int width, int height, int nLSCTarget, int ob, uchar* nLenCReg, int nLenCRegCount, int LSCGroup)
{
    QString strLog = "";

    (void)raw8;
    (void)width;
    (void)height;
    (void)nLSCTarget;
    (void)ob;
    (void)nLenCReg;
    (void)nLenCRegCount;
    (void)LSCGroup;

    if (ERR_NoError != I2cWrite(0x3400, 0x00, 3))
    {
        strLog = QString::asprintf("S5K4H7YX::%s Error---I2cWrite Reg[0x3400] Fail\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    if (ERR_NoError != I2cWrite(0x0B00, 0x01, 3))
    {
        strLog = QString::asprintf("S5K4H7YX::%s Error---I2cWrite Reg[0x0B00] Fail\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
}

int S5K4H7YX::GetSensorExposure(uint &value)
{
    uchar buf[] = {0, 0};
    int ec = i2cReadContinuous(EXPOSURE_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("S5K4H7YX::Call ReadContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    value = MAKEWORD(buf[0], buf[1]);
    return ERR_NoError;
} 

int S5K4H7YX::SetSensorExposure(uint value)
{
    uchar buf[] = {MSB(value), LSB(value)};
    int ec = i2cWriteContinuous(EXPOSURE_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("S5K4H7YX::Call WriteContinuousI2c() failed[%2]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ERR_NoError;
} 

int S5K4H7YX::GetSensorGain(uint &value)
{
    uchar buf[] = {0, 0};
    int ec = i2cReadContinuous(SENSOR_GAIN_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("S5K4H7YX::Call ReadContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    value = MAKEWORD(buf[0], buf[1]);
    return ERR_NoError;
}

int S5K4H7YX::SetSensorGain(uint value, bool bMultiple)
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
        QString strLog = QString("S5K4H7YX::Call WriteContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ERR_NoError;
}

int S5K4H7YX::readOtpData(ushort startAddr, ushort endAddr, uchar *buf, ushort page)
{
    if (nullptr == buf) {
        qCritical("S5K4H7YX::%s Error---buf is NULL", __FUNCTION__);
        return ERR_InvalidParameter;
    }

    int nLen = endAddr - startAddr + 1;
    if (nLen > PAGE_LEN) {
        qCritical("S5K4H7YX::%s Error---nLen:%d is outside PAGE_LEN:%d", __FUNCTION__, nLen, PAGE_LEN);
        return ERR_InvalidParameter;
    }

    ushort stram_flag = 0;
    if (ERR_NoError != I2cRead(REG_OP_MODE_ADDR, stram_flag, I2C_MODE_ADDR16_DATA8)) {
        qCritical("S5K4H7YX::%s---I2cRead Error", __FUNCTION__);
        return ERR_Failed;
    }

    if (SENSOR_SOFTWARE_STANDBY == (stram_flag & SENSOR_OP_MODE_MASK)) {
        if (ERR_NoError != I2cWrite(REG_OP_MODE_ADDR, SENSOR_STREAMING_ON, I2C_MODE_ADDR16_DATA8)) {
            qCritical("S5K4H7YX::%s---I2cWrite REG_OP_MODE_ADDR Error", __FUNCTION__);
            return ERR_Failed;
        }
        QThread::msleep(50);
    }

    int ec = ERR_NoError;
    ec = I2cWrite(REG_PAGE_SEL_ADDR, page, I2C_MODE_ADDR16_DATA8);  // select otp page
    ec |= I2cWrite(REG_CTL_RW_ADDR, R_CMD, I2C_MODE_ADDR16_DATA8);  // turn on otp read mode
    if (ec < 0) {
        qCritical("Turn on S5K4H7YX otp read mode failed[%d]", ec);
        return ec;
    }

    bool checkFailed = true;
    ushort flag = 0;
    for (uint i = 0; i < 10; i++) { // check status repeat 10 times
        if (ERR_NoError == I2cRead(REG_ERR_FLGA_ADDR, flag, I2C_MODE_ADDR16_DATA8)) {
            if ((flag & 0x01) == 0x01) {
                checkFailed = false;
                break;
            }
        }
        QThread::msleep(50);
    }
    if (checkFailed) {
        qCritical("Check S5K4H7YX OTP status[0x%x] failed", flag);
        return ERR_Failed;
    }

    //read otp data to buff
    ec = i2cReadContinuous(startAddr, 2, buf, endAddr - startAddr + 1);
    if (ec < 0) {
        qCritical("S5K4H7YX read otp addr[0x%x] failed[%d]", startAddr, ec);
        return ec;
    }

    // Inital command
    ec = I2cWrite(REG_CTL_RW_ADDR, INIT_CMD, I2C_MODE_ADDR16_DATA8);
    if (ec < 0) {
        qCritical("S5K4H7YX::%s---I2cWrite REG_CTL_RW_ADDR Error\r\n", __FUNCTION__);
        return ec;
    }

    return ec;
}
