
#define IMG_DRV_GC5025

#include "GC5025.h"
#include <QThread>
#include <QVector>
#include "czcmtidefs.h"
#include "macrodefs.h"

GC5025::GC5025(const T_SensorSetting &sensorSetting) :
    ImageSensor(sensorSetting)
{
}

int GC5025::GetTemperature(int &temperature)
{
    (void)temperature;

    return ERR_NoError;
}

int GC5025::GetSensorFuseId(std::string &fuseId, bool bMultiple)
{
#if 1
    (void)bMultiple;
    uchar tmpBuff[FUSE_ID_LEN] = {0};
    int ec = readOtpData(FUSE_ID_ADDR_START, FUSE_ID_ADDR_END, tmpBuff, FUSE_ID_PAGE);
    if (ec < 0) {
        QString strLog = QString("Read GC5025 FuseId failed[%1]").arg(ec);
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
#else
	char section[512] = {0};
	char temp[512] = {0};
	USHORT tempVal[9];
		
	for (int j=0;j<9;j++)
	{
		if(ReadReg(0x03B0+j*8,0x03B0+j*8,tempVal+j) != 1) return 0;
		sprintf(section, "%02X",tempVal[j]);
		FuseID += section;
	}
	return 1;
#endif
}

int GC5025::ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg)
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
              << ST_RegData(GAIN_RED_LOW_ADDR,         ((LSB(G_gain) & 0x07) << 4) | ((LSB(R_gain)) & 0x07) )
              << ST_RegData(GAIN_GREEN1_HIGH_ADDR,     MSB(G_gain))
              << ST_RegData(GAIN_GREEN2_HIGH_ADDR,     MSB(G_gain))
              << ST_RegData(GAIN_BLUE_HIGH_ADDR,       MSB(B_gain))
              << ST_RegData(GAIN_BLUE_LOW_ADDR,        ((LSB(B_gain) & 0x07) << 4) | ((LSB(G_gain)) & 0x07) );
    return writeRegisters(vtRegData, I2C_MODE_ADDR8_DATA8_NORMAL);
}

int GC5025::GetSensorExposure(uint &value)
{
#if 1
    uchar buf[] = {0, 0};
    int ec = i2cReadContinuous(EXPOSURE_REG_H_ADDR, 1, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("GC5025::%1 Error---Call i2cReadContinuous() failed[%1]").arg(__FUNCTION__).arg(ec);
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
	if(I2cRead(0x04,&ExpLow, 0) != 1) return 0;   

	exp=(ExpHigh<<8)+(ExpLow&0xFF);
	return 1;
#endif
} 

int GC5025::SetSensorExposure(uint value)
{
#if 1
    uchar buf[] = {MSB(value), LSB(value)};
    int ec = i2cWriteContinuous(EXPOSURE_REG_H_ADDR, 1, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("GC5025::%1 Error---Call i2cWriteContinuous() failed[%2]").arg(__FUNCTION__).arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ERR_NoError;
#else
	USHORT ExpHigh=exp>>8;
	USHORT ExpLow=exp&0xFF;

	if(I2cWrite(0x03,ExpHigh,0) != 1) return 0; 
	if(I2cWrite(0x04,ExpLow, 0) != 1) return 0; 

	return 1;
#endif
}

int GC5025::GetSensorGain(uint &value)
{
#if 1
    uchar buf = 0;
    int ec = i2cReadContinuous(SENSOR_GAIN_REG_ADDR, 1, &buf, sizeof(buf));
    if (ec < 0) {
        QString strLog = QString("GC5025::%1 Error---Call i2cReadContinuous() failed[%2]").arg(__FUNCTION__).arg(ec);
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

#if 0   //以下是原厂参考代码
#define ANALOG_GAIN_1 64   // 1.00x
#define ANALOG_GAIN_2 92   // 1.445x

static void set_gain(kal_uint16 gain)//微信向GC鲁意请教，我们设置n倍，参数gain就传入(n*0x40)即可。
{
    kal_uint16 iReg,temp;

    iReg = gain;

    if(iReg < 0x40)
        iReg = 0x40;//1x gain

    if((ANALOG_GAIN_1<= iReg)&&(iReg < ANALOG_GAIN_2))
    {
        write_cmos_sensor(0xfe,  0x00); //I2C write function
        write_cmos_sensor(0xb6,  0x00);
        temp = iReg;
        write_cmos_sensor(0xb1, temp >>6 );
        write_cmos_sensor(0xb2, (temp <<2) & 0xfc);
        LOG_INF("GC5025MIPI analogic gain 1x, GC5025MIPI add pregain = %d\n",temp);
    }
    else
    {
        write_cmos_sensor(0xfe,  0x00);
        write_cmos_sensor(0xb6,  0x01);
        temp = 64 * iReg / ANALOG_GAIN_2;
        write_cmos_sensor(0xb1, temp >> 6);
        write_cmos_sensor(0xb2, (temp << 2) & 0xfc);
        LOG_INF("GC5025MIPI analogic gain 1.4x, GC5025MIPI add pregain = %d\n",temp);
    }
}    /*    set_gain  */
#endif
int GC5025::SetSensorGain(uint value, bool bMultiple)
{
#if 1
    if (true == bMultiple)      //1x~8x
    {
        value = value * 64;
        //return SetSensorGain(value, false);
    }

    if (value > 512)    //Gain: 0 ~ 8
    {
        value = 512;
    }

    QVector<ST_RegData> vtRegData;
    if ((ANALOG_GAIN_1 <= value) && (value < ANALOG_GAIN_2))
    {
        vtRegData << ST_RegData(0xB6, 0x00)
                  << ST_RegData(0xB1, value >> 6)
                  << ST_RegData(0xB2, (value << 2) & 0xFC);
    }
    else
    {
        vtRegData << ST_RegData(0xB6, 0x01)
                  << ST_RegData(0xB1, value >> 6)
                  << ST_RegData(0xB2, (value << 2) & 0xFC);
    }

    return writeRegisters(vtRegData, I2C_MODE_ADDR8_DATA8_NORMAL);
#else
	if(I2cWrite(0xB6,gain&0xff,0) != 1) return 0;
	return 1;
#endif
} 

int GC5025::readOtpData(ushort startAddr, ushort endAddr, uchar *pBuf, ushort page)
{
#if 1
    (void)page;
    QString strLog = "";
    int ec = ERR_NoError;

    if (nullptr == pBuf) {
        qCritical("GC5025::%s Error---pBuf is NULL", __FUNCTION__);
        strLog = QString::asprintf("GC5025::%s I2cWrite() pBuf is NULL", __FUNCTION__);
        m_channelController->LogToWindow(strLog, LogRed);
        return ERR_InvalidParameter;
    }

    uint bufferLen = endAddr - startAddr + 1;
    /**
      * GC5025内部2K Bit OTP空间，分两个Page，一个Page 1K Bit空间。而以下0xD4、0xD5寄存器内设置的地址其实是按Bit计算的，所以地址范围是0~1023，共需要
      * 10Bit来表示。
      * Page0:GC保留空间。
      * Page1:OTP空间，可以烧录OTP数据的。
      * ----20180530
    **/
    ushort startAddrBit = startAddr << 3;
    BYTE Addr_hig = (BYTE)((startAddrBit >> 8) & 0x03);
    BYTE Addr_low = (BYTE)(startAddrBit & 0xFF);

    QVector<ST_RegData> vtRegData;
    vtRegData << ST_RegData(0xF7, 0x01)
              << ST_RegData(0xF9, 0x00)
              << ST_RegData(0xFC, 0x2E)
              << ST_RegData(0xFA, 0xB0);
            /*<< ST_RegData(REG_OTP_MODE_ADDR, (OTP_EN | OTP_PAGE1 | Addr_hig))
              << ST_RegData(REG_OTP_ACCESS_ADDR_L, Addr_low)
              << ST_RegData(REG_OTP_WR_OR_RD, OTP_RD);*/

    ec = writeRegisters(vtRegData, I2C_MODE_ADDR8_DATA8_NORMAL);
    if (ec < 0) {
        qCritical("writeRegisters failed[%d]", ec);
        strLog = QString::asprintf("GC5025::%s writeRegisters() Error", __FUNCTION__);
        m_channelController->LogToWindow(strLog, LogRed);
        return ec;
    }

    USHORT u16Tmp;
    for (uint i = 0; i < bufferLen; i++)
    {
        startAddrBit = ((startAddr + i) << 3);
        Addr_hig = (BYTE)((startAddrBit >> 8) & 0x03);
        Addr_low = (BYTE)(startAddrBit & 0xFF);

        /**
          * 每读一个字节，需要先写入地址(注意地址是Bit地址)，然后写入0xF3寄存器触发将数据传入DATA寄存器，最后通过DATA寄存器把数据读出来。
          * ----20180530
        **/
        vtRegData << ST_RegData(REG_OTP_MODE_ADDR, (OTP_EN | OTP_PAGE1 | Addr_hig))
                  << ST_RegData(REG_OTP_ACCESS_ADDR_L, Addr_low)
                  << ST_RegData(REG_OTP_WR_OR_RD, OTP_RD);

        ec = writeRegisters(vtRegData, I2C_MODE_ADDR8_DATA8_NORMAL);
        if (ec < 0) {
            qCritical("writeRegisters failed[%d]", ec);
            strLog = QString::asprintf("GC5025::%s writeRegisters() Error", __FUNCTION__);
            m_channelController->LogToWindow(strLog, LogRed);
            return ec;
        }

        if (ERR_NoError != I2cRead(REG_OTP_RD_DATA_ADDR, u16Tmp, I2C_MODE_ADDR8_DATA8_NORMAL))
        {
            strLog = QString::asprintf("GC5025::%s I2cRead() Reg[%d] Error", __FUNCTION__, i);
            m_channelController->LogToWindow(strLog, LogRed);
            return ERR_Failed;
        }

        pBuf[i] = LSB(u16Tmp);
    }

    return ERR_NoError;
#else
    if(NULL == buf) return 0;

    BYTE Addr_hig=(startAddr>>8)&0x03;
    BYTE Addr_low=startAddr&0xFF;
    USHORT Value;
    int SensorMode=0;
    if(I2cWrite(0xf7,0x01,SensorMode) != 1) return 0;
    if(I2cWrite(0xf9,0x00,SensorMode) != 1) return 0;
    if(I2cWrite(0xfc,0x2e,SensorMode) != 1) return 0;
    if(I2cWrite(0xfa,0xb0,SensorMode) != 1) return 0; //OTP clk enable

    if(I2cWrite(0xd4,0x84,SensorMode) != 1) return 0; //OTP enable[7] OTP page select[2]

    if(I2cWrite(0xd4,0x84+Addr_hig,SensorMode)!= 1) return 0;//OTP address select high bit [9:8]
    if(I2cWrite(0xd5,Addr_low,SensorMode)!= 1) return 0;//OTP address select low bit [7:0]

    if(I2cWrite(0xf3,0x20,SensorMode)!= 1) return 0;//OTP read

    if(I2cRead(0xd7,buf,SensorMode)!= 1) return 0;//OTP value

    return 1;
#endif
}


int GC5025::WriteReg(USHORT startAddr, USHORT endAddr, uchar *buf, USHORT page)
{
#if 1
    (void)startAddr;
    (void)endAddr;
    (void)buf;
    (void)page;

    return ERR_NoError;
#else
    if(NULL == buf) return 0;
    BYTE Addr_hig=(startAddr>>8)&0x03;
    BYTE Addr_low=startAddr&0xFF;
    int ret=1;
    USHORT readback[2]={0};
    int SensorMode=0;
    if(I2cWrite(0xf7,0x01,SensorMode) != 1) return 0;
    if(I2cWrite(0xf9,0x00,SensorMode) != 1) return 0;
    if(I2cWrite(0xfc,0x2e,SensorMode) != 1) return 0;
    if(I2cWrite(0xfa,0xb0,SensorMode) != 1) return 0;  //OTP clk enable
    if(I2cWrite(0xd4,0x84,SensorMode) != 1) return 0;  //OTP enable[7] OTP page select[2]
    if(I2cWrite(0xd4,0x84+Addr_hig,SensorMode) != 1) return 0; //OTP address select high bit [9:8]
    if(I2cWrite(0xd5,Addr_low,SensorMode) != 1) return 0;//OTP address select low bit [7:0]
    if(I2cWrite(0xd6,buf[0],SensorMode) != 1) return 0;//OTP value
    Sleep(5);//Delay 1ms
    if(I2cWrite(0xd8,0x20,SensorMode) != 1) return 0;//Select T1
    for (int i=0;i<5;i++)//Repeat 5 times
    {
        if(I2cWrite(0xf3,0x42,SensorMode) != 1) return 0;//OTP write
        Sleep(25);//Delay 24ms
    }
    if(I2cWrite(0xd8,0x10,SensorMode)!= 1) return 0;
    Sleep(5);//Delay 3ms (Check if the value is correct or not;)

    if(I2cWrite(0xf3,0x20,SensorMode) != 1) return 0;//OTP read
    if(I2cRead(0xd7,readback,SensorMode) != 1) return 0;//OTP value
    if (readback[0]!=buf[0])
    {
        ret=0;
    }

    if(I2cWrite(0xd8,0x40,SensorMode) != 1) return 0; //Select T2 (First)

    for (int i=0;i<5;i++)//Repeat 5 times
    {
        if(I2cWrite(0xf3,0x42,SensorMode) != 1) return 0;//OTP write
        Sleep(25);//Delay 24ms
    }
    if(I2cWrite(0xd8,0x10,SensorMode) != 1) return 0;
    Sleep(5);//Delay 3ms (Check if the value is correct or not;
    if(I2cWrite(0xf3,0x20,SensorMode) != 1) return 0;//OTP read
    if(I2cRead(0xd7,readback,SensorMode)  != 1) return 0;//OTP value
    if (readback[0]!=buf[0])
    {
        ret=0;
        return 0;
    }

    return 1;
#endif
}
