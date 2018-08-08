#include "S5K2L7SA.h"
#include <QThread>
#include <QVector>
#include "czcmtidefs.h"
#include "macrodefs.h"
#include <QDebug>
#include <QString>


#define GAIN_DEFAULT            0x0100

#define OTP_PAGE_ADDR           0x0A02
#define OTP_PAGE_OFFSET_START   0x0A04
#define OTP_PAGE_OFFSET_END     0x0A43
#define OTP_PAGE_LEN            64

/**
  * 读写都是通过特殊功能寄存器实现的，芯片内部采用32位地址总线和16位数据总线，地址采用【段地址 + 段内偏移地址】的方式，具体如下：
  * 1、读：段地址寄存器:0x602C、段内偏移地址寄存器:0x602E、数据寄存器:0x6F12
  * 2、写：段地址寄存器:0x6028、段内偏移地址寄存器:0x602A、数据寄存器:0x6F12
**/
#define REG_INDIRECT_RD_PAGE_ADDR       0x602C
#define REG_INDIRECT_RD_OFFSET_ADDR     0x602E
#define REG_INDIRECT_RD_DATA_ADDR       0x6F12

#define REG_INDIRECT_WR_PAGE_ADDR       0x6028
#define REG_INDIRECT_WR_OFFSET_ADDR     0x602A
#define REG_INDIRECT_WR_DATA_ADDR       0x6F12

#define PAGE_DEFAULT_DIRECT_WR_ADDR     0x4000  //直接存取的默认段地址寄存器
#define PAGE_DEFAULT_INDIRECT_WR_ADDR   0x0000  //间接存取的默认段地址寄存器

/** Exposure Reg Addr Define **/
#define EXPOSURE_REG_ADDR               0x0202

/** Anglog Gain Reg Addr Define **/
#define SENSOR_GAIN_REG_ADDR            0x0204


#define FUSE_ID_PAGE_NO                 0
#define FUSE_ID_OFFSET_START            0x0A24
#define FUSE_ID_OFFSET_END              0x0A31
#define FUSE_ID_LEN                     (FUSE_ID_OFFSET_END - FUSE_ID_OFFSET_START + 1)

#define FUSE_ID_ADDR_START              0x0A24
#define FUSE_ID_ADDR_END                0x0A31
#define FUSE_ID_PAGE                    0

S5K2L7SA::S5K2L7SA(const T_SensorSetting &sensorSetting) :
    ImageSensor(sensorSetting)
{
}



int S5K2L7SA::GetTemperature(int &temperature)
{
    (void)temperature;
    return ERR_NoError;
}


int S5K2L7SA::GetSensorFuseId(std::string &fuseId, bool bUseMasterI2C)
{
    (void)bUseMasterI2C;
#if 1
    uchar buf[FUSE_ID_LEN] = {0};
    int ec = readOtpData(FUSE_ID_ADDR_START, FUSE_ID_ADDR_END, buf, FUSE_ID_PAGE);
    if (ec < 0) {
        QString strLog = QString("S5K2L7SA: Read FuseId failed[%1]").arg(ec);
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

#else
    QString strLog = "";
    unsigned int i = 0;
    QVector<ST_RegData> vtRegData;
    const ST_RegData RD_REG_SET_TAB[]=
    {
        {REG_INDIRECT_WR_OFFSET_ADDR, 0x0A02},
        {REG_INDIRECT_WR_DATA_ADDR, 0x0000},
        {REG_INDIRECT_WR_OFFSET_ADDR, 0x0A00},
        {REG_INDIRECT_WR_DATA_ADDR, 0x0100}
    };

    for (i = 0; i < sizeof (RD_REG_SET_TAB) / sizeof (RD_REG_SET_TAB[0]); i++)
    {
        vtRegData.push_back(RD_REG_SET_TAB[i]);
    }

    int iRet = writeRegisters(vtRegData, I2C_MODE_ADDR16_DATA16);
    if (ERR_NoError != iRet)
    {
        strLog = QString::asprintf("S5K2L7SA::%s---writeRegisters Error--iRet:%d\r\n", __FUNCTION__, iRet);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return iRet;
    }

    uchar tempVal[OTP_PAGE_LEN];
    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    iRet = m_channelController->ReadContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100, OTP_PAGE_OFFSET_START, 2, tempVal, sizeof(tempVal));
    if (ERR_NoError != iRet)
    {
        strLog = QString::asprintf("OV24A1B::%s---ReadContinuousI2c Error", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    fuseId = "";
    for (int j = FUSE_ID_OFFSET_START; j <= FUSE_ID_OFFSET_END; j++)
    {
        QString strFuseId = "";
        strFuseId = QString::asprintf("%02X", tempVal[j - OTP_PAGE_OFFSET_START]);
        fuseId += strFuseId.toStdString();
    }

    return ERR_NoError;
#endif
}

int S5K2L7SA::ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg)
{
    if(rg == 0 || bg == 0 || Typical_rg == 0 || Typical_bg == 0) return ERR_Failed;

	USHORT r_ratio, b_ratio;

	r_ratio = 512 * (Typical_rg) /(rg);
	b_ratio = 512 * (Typical_bg) /(bg);

	if (!r_ratio || !b_ratio)
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

#if 0
	if(I2cWrite(0x6028, 0x4000,4) != 1) return 0;
	if(I2cWrite(0x602a, 0x0100,4) != 1) return 0;
	if(I2cWrite(0x6f12, 0x00, 3 ) != 1) return 0;// Stream off
	Sleep(200);

	if(I2cWrite(0x6028, 0x4000,4) != 1) return 0;	
	if(I2cWrite(0x602a, 0x306A,4) != 1) return 0;
	if(I2cWrite(0x6f12, 0x0100,4) != 1) return 0;

	if(I2cWrite(0x602a, 0x0210,4) != 1) return 0;
	if(I2cWrite(0x6f12, R_GAIN,4) != 1) return 0;
	if(I2cWrite(0x602a, 0x0218,4) != 1) return 0;
	if(I2cWrite(0x6f12, R_GAIN,4) != 1) return 0;

	if(I2cWrite(0x602a, 0x0212,4) != 1) return 0;
	if(I2cWrite(0x6f12, B_GAIN,4) != 1) return 0;
	if(I2cWrite(0x602a, 0x021A,4) != 1) return 0;
	if(I2cWrite(0x6f12, B_GAIN,4) != 1) return 0;

	if(I2cWrite(0x602a, 0x020e,4) != 1) return 0;
	if(I2cWrite(0x6f12, G_GAIN,4) != 1) return 0;
	if(I2cWrite(0x602a, 0x0216,4) != 1) return 0;
	if(I2cWrite(0x6f12, G_GAIN,4) != 1) return 0;

	if(I2cWrite(0x602a, 0x0214,4) != 1) return 0;
	if(I2cWrite(0x6f12, G_GAIN,4) != 1) return 0;
	if(I2cWrite(0x602a, 0x021C,4) != 1) return 0;
	if(I2cWrite(0x6f12, G_GAIN,4) != 1) return 0;

	if(I2cWrite(0x6028, 0x4000,4) != 1) return 0;
	if(I2cWrite(0x602a, 0x0100,4) != 1) return 0;
	if(I2cWrite(0x6f12, 0x01, 3 ) != 1) return 0;// Stream on

	return 1;
#else
    if(I2cWrite(REG_INDIRECT_WR_PAGE_ADDR, PAGE_DEFAULT_DIRECT_WR_ADDR,4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(REG_INDIRECT_WR_OFFSET_ADDR, 0x0100,4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(REG_INDIRECT_WR_DATA_ADDR, 0x00, 3 ) != ERR_NoError) return ERR_Failed;// Stream off
    QThread::msleep(200);

    if(I2cWrite(REG_INDIRECT_WR_PAGE_ADDR, PAGE_DEFAULT_DIRECT_WR_ADDR,4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(REG_INDIRECT_WR_OFFSET_ADDR, 0x306A,4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(REG_INDIRECT_WR_DATA_ADDR, 0x0100,4) != ERR_NoError) return ERR_Failed;

    if(I2cWrite(REG_INDIRECT_WR_OFFSET_ADDR, 0x0210,4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(REG_INDIRECT_WR_DATA_ADDR, R_GAIN,4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(REG_INDIRECT_WR_OFFSET_ADDR, 0x0218,4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(REG_INDIRECT_WR_DATA_ADDR, R_GAIN,4) != ERR_NoError) return ERR_Failed;

    if(I2cWrite(REG_INDIRECT_WR_OFFSET_ADDR, 0x0212,4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(REG_INDIRECT_WR_DATA_ADDR, B_GAIN,4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(REG_INDIRECT_WR_OFFSET_ADDR, 0x021A,4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(REG_INDIRECT_WR_DATA_ADDR, B_GAIN,4) != ERR_NoError) return ERR_Failed;

    if(I2cWrite(REG_INDIRECT_WR_OFFSET_ADDR, 0x020e,4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(REG_INDIRECT_WR_DATA_ADDR, G_GAIN,4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(REG_INDIRECT_WR_OFFSET_ADDR, 0x0216,4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(REG_INDIRECT_WR_DATA_ADDR, G_GAIN,4) != ERR_NoError) return ERR_Failed;

    if(I2cWrite(REG_INDIRECT_WR_OFFSET_ADDR, 0x0214,4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(REG_INDIRECT_WR_DATA_ADDR, G_GAIN,4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(REG_INDIRECT_WR_OFFSET_ADDR, 0x021C,4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(REG_INDIRECT_WR_DATA_ADDR, G_GAIN,4) != ERR_NoError) return ERR_Failed;

    if(I2cWrite(REG_INDIRECT_WR_PAGE_ADDR, 0x4000,4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(REG_INDIRECT_WR_OFFSET_ADDR, 0x0100,4) != ERR_NoError) return ERR_Failed;
    if(I2cWrite(REG_INDIRECT_WR_DATA_ADDR, 0x01, 3 ) != ERR_NoError) return ERR_Failed;// Stream on

    return ERR_NoError;
#endif
}

int S5K2L7SA::GetSensorExposure(uint &value)
{
    int ec = setDirectPage(PAGE_DEFAULT_DIRECT_WR_ADDR);
    if (ec < 0) {
        QString strLog = QString("S5K2L7SA::Call setDirectPage() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }
    ushort exposure;
    ec = I2cRead(EXPOSURE_REG_ADDR, exposure, I2C_MODE_ADDR16_DATA16);
    if (ec < 0) {
        QString strLog = QString("S5K2L7SA::Call I2cRead() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    value = exposure;
    return ec;
} 

int S5K2L7SA::SetSensorExposure(uint value)
{
    int ec = setDirectPage(PAGE_DEFAULT_DIRECT_WR_ADDR);
    if (ec < 0) {
        QString strLog = QString("S5K2L7SA::Call setDirectPage() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }
    ec = I2cWrite(EXPOSURE_REG_ADDR, (ushort)value, I2C_MODE_ADDR16_DATA16);
    if (ec < 0) {
        QString strLog = QString("S5K2L7SA::Call I2cWrite() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ec;
} 

int S5K2L7SA::GetSensorGain(uint &value)
{
    int ec = setDirectPage(PAGE_DEFAULT_DIRECT_WR_ADDR);
    if (ec < 0) {
        QString strLog = QString("S5K2L7SA::Call setDirectPage() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }
    ushort gain;
    ec = I2cRead(SENSOR_GAIN_REG_ADDR, gain, I2C_MODE_ADDR16_DATA16);
    if (ec < 0) {
        QString strLog = QString("S5K2L7SA::Call I2cRead() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    value = gain;
    return ec;
}

int S5K2L7SA::SetSensorGain(uint value, bool bMultiple)
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

    int ec = setDirectPage(PAGE_DEFAULT_DIRECT_WR_ADDR);
    if (ec < 0) {
        QString strLog = QString("S5K2L7SA::Call setDirectPage() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }
    ec = I2cWrite(SENSOR_GAIN_REG_ADDR, (ushort)value, I2C_MODE_ADDR16_DATA16);
    if (ec < 0) {
        QString strLog = QString("S5K2L7SA::Call I2cWrite() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ec;
} 

int S5K2L7SA::setDirectPage(ushort page)
{
    return I2cWrite(0x6028, page, I2C_MODE_ADDR16_DATA16);
}


int S5K2L7SA::readOtpData(ushort startAddr, ushort endAddr, uchar *buf, ushort page)
{
    if (nullptr == buf) {
        qCritical("S5K2L7SA::%s Error---buf is NULL", __FUNCTION__);
        return ERR_InvalidParameter;
    }

    uint nLen = endAddr - startAddr + 1;
    if (nLen > OTP_PAGE_LEN) {
        qCritical("S5K2L7SA::%s Error---nLen:%d is outside PAGE_LEN:%d", __FUNCTION__, nLen, OTP_PAGE_LEN);
        return ERR_InvalidParameter;
    }

    int ec = setDirectPage(PAGE_DEFAULT_DIRECT_WR_ADDR);
    if (ec < 0) {
        QString strLog = QString("S5K2L7SA::Call setDirectPage() failed[%1]").arg(ec);
        qCritical() << strLog;
        return ec;
    }
    ushort stram_flag = 0;
    if (ERR_NoError != I2cRead(0x0100, stram_flag, I2C_MODE_ADDR16_DATA16)) {
        qCritical("S5K2L7SA::%s---I2cRead Error", __FUNCTION__);
        return ERR_Failed;
    }

    if (0x0100 == (stram_flag & 0x0100)) {
        if (ERR_NoError != I2cWrite(0x0100, 0x0100, I2C_MODE_ADDR16_DATA16)) {
            qCritical("S5K2L7SA::%s---I2cWrite 0x100 Error", __FUNCTION__);
            return ERR_Failed;
        }
        QThread::msleep(50);
    }

    ec = I2cWrite(0x0A02, page, I2C_MODE_ADDR16_DATA16);  // select otp page
    ec |= I2cWrite(0x0A00, 0x0100, I2C_MODE_ADDR16_DATA16);  // otp read start
    if (ec < 0) {
        qCritical("Turn on S5K2L7SA otp read mode failed[%d]", ec);
        return ec;
    }
    QThread::msleep(2);

    //read otp data to buff
    ec = i2cReadContinuous(startAddr, 2, buf, endAddr - startAddr + 1);
    if (ec < 0) {
        qCritical("S5K2L7SA read otp addr[0x%x] failed[%d]", startAddr, ec);
        return ec;
    }

    // Inital command
    ec = I2cWrite(0x0A00, 0x0000, I2C_MODE_ADDR16_DATA16);
    if (ec < 0) {
        qCritical("S5K2L7SA::%s---I2cWrite 0x0A00 Error\r\n", __FUNCTION__);
        return ec;
    }

    return ec;
}

