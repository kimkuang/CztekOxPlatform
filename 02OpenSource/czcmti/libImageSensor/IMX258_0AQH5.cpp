
#include "IMX258_0AQH5.h"
#include <QThread>
#include <QVector>
#include "czcmtidefs.h"
#include "macrodefs.h"

/** Exposure Reg Addr Define **/
#define EXPOSURE_REG_H_ADDR         0x0202
#define EXPOSURE_REG_L_ADDR         0x0203

/** Analog Gain Reg Addr Define **/
#define SENSOR_GAIN_REG_H_ADDR      0x0204
#define SENSOR_GAIN_REG_L_ADDR      0x0205

/*#define GAIN_DEFAULT       0x0100
#define GAIN_GREEN1_ADDR   0x020E
#define GAIN_BLUE_ADDR     0x0212
#define GAIN_RED_ADDR      0x0210
#define GAIN_GREEN2_ADDR   0x0214*/

/** Channel GAIN Reg Addr Define **/
#define GAIN_DEFAULT                0x0100
#define GAIN_GREEN1_HIGH_ADDR       0x020E
#define GAIN_GREEN1_LOW_ADDR        0x020F
#define GAIN_BLUE_HIGH_ADDR         0x0212
#define GAIN_BLUE_LOW_ADDR          0x0213
#define GAIN_RED_HIGH_ADDR          0x0210
#define GAIN_RED_LOW_ADDR           0x0211
#define GAIN_GREEN2_HIGH_ADDR       0x0214
#define GAIN_GREEN2_LOW_ADDR        0x0215

/** OTP Reg Addr Define **/
#define REG_PAGE_SEL_ADDR           0x0A02
#define REG_DATA_START_ADDR         0x0A04
#define REG_DATA_END_ADDR           0x0A43
#define PAGE_LEN                    (REG_DATA_END_ADDR - REG_DATA_START_ADDR + 1)

/** Fuse ID Reg Addr Define **/
#define FUSE_ID_PAGE                15
#define FUSE_ID_ADDR_START          0x0A24
#define FUSE_ID_ADDR_END            0x0A2E
#define FUSE_ID_LEN                 (FUSE_ID_ADDR_END - FUSE_ID_ADDR_START + 1)

IMX258::IMX258(const T_SensorSetting &sensorSetting) :
    ImageSensor(sensorSetting)
{
}

int IMX258::GetTemperature(int &temperature)
{
    (void)temperature;
    return ERR_NoError;
}

int IMX258::GetSensorFuseId(std::string &fuseId, bool bUseMasterI2C)
{
#if 1
    (void)bUseMasterI2C;
    uchar tmpBuff[FUSE_ID_LEN] = {0};
    int ec = readOtpData(FUSE_ID_ADDR_START, FUSE_ID_ADDR_END, tmpBuff, FUSE_ID_PAGE);
    if (ec < 0) {
        QString strLog = QString("Read IMX258 FuseId failed[%1]").arg(ec);
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
    (void)bUseMasterI2C;
    FuseID = _T("");
    USHORT temp1[64] = {0};
    char section[256];
    int i=0;
    if(IMX258_ReadOTPPage(15,temp1) != 1) return 0;
    for (i=32;i<43;i++)
    {
        sprintf(section,"%02X",temp1[i]);
        FuseID+=section;
    }
    return 1;
#endif
}


int IMX258::ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg)
{
    if (rg == 0 || bg == 0 || Typical_rg == 0 || Typical_bg == 0)
    {
        return ERR_Failed;
    }

	USHORT r_ratio, b_ratio;
	
	r_ratio = 512 * (Typical_rg) /(rg);
	b_ratio = 512 * (Typical_bg) /(bg);
	
	 
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
	
#if 1
    QVector<ST_RegData> vtRegData;
    vtRegData << ST_RegData(GAIN_RED_HIGH_ADDR,        MSB(R_GAIN))
              << ST_RegData(GAIN_RED_LOW_ADDR,         LSB(R_GAIN))
              << ST_RegData(GAIN_GREEN1_HIGH_ADDR,     MSB(G_GAIN))
              << ST_RegData(GAIN_GREEN1_LOW_ADDR,      LSB(G_GAIN))
              << ST_RegData(GAIN_GREEN2_HIGH_ADDR,     MSB(G_GAIN))
              << ST_RegData(GAIN_GREEN2_LOW_ADDR,      LSB(G_GAIN))
              << ST_RegData(GAIN_BLUE_HIGH_ADDR,       MSB(B_GAIN))
              << ST_RegData(GAIN_BLUE_LOW_ADDR,        LSB(B_GAIN));
    return writeRegisters(vtRegData, I2C_MODE_ADDR16_DATA8);
#else
	if(I2cWrite(  GAIN_RED_ADDR, R_GAIN>>8, 3) != 1) return 0;
	if(I2cWrite(  GAIN_RED_ADDR+1, R_GAIN&0xff, 3) != 1) return 0;
	
	if(I2cWrite(  GAIN_BLUE_ADDR, B_GAIN>>8, 3) != 1) return 0;
	if(I2cWrite(  GAIN_BLUE_ADDR+1, B_GAIN&0xff, 3) != 1) return 0;
	 
	if(I2cWrite(  GAIN_GREEN1_ADDR, G_GAIN>>8, 3) != 1) return 0;
	if(I2cWrite(  GAIN_GREEN1_ADDR+1, G_GAIN&0xff, 3) != 1) return 0;
	 
	if(I2cWrite(  GAIN_GREEN2_ADDR, G_GAIN>>8, 3) != 1) return 0;
	if(I2cWrite(  GAIN_GREEN2_ADDR+1, G_GAIN&0xff, 3) != 1) return 0;

	return 1;
#endif
}

#if 0
int IMX258::ApplyLsc(BYTE* raw8,int width,int height,int nLSCTarget,int ob,UCHAR* nLenCReg,int nLenCRegCount,int LSCGroup)
{
	return 1;
}
int IMX258::spcCal(USHORT *imgBuf,short *pSPC)
{  
	if(NULL == imgBuf || NULL == pSPC) return 0;

	CString path;
	GetCurentPath(path);
	path = path + _T("T_SPC\\IMX_258\\");
	USHORT *Calcbuffer = new USHORT[4208*3122+2];
	if(NULL == Calcbuffer) return 0;
	 
	for (int i=0;i<(4208*3122+2);i++)
	{
		if(i<8418) Calcbuffer[i]=0; //前面两行设置为0
		else Calcbuffer[i]=imgBuf[i-8418]; 
	}
    
	Calcbuffer[0]=4208;
	Calcbuffer[1]=3122;
	CString IniSpcGainPath;
	IniSpcGainPath=path+ _T("spc_gain_258_NML.ini"); 
	
	typedef int (*lpFun)(unsigned short *ImageBuffer, char *IniFilePath, short *pSPC);

	HINSTANCE hDll;   //DLL句柄 
	CString toolDllPath = 	path + _T("IMX258_SPC_DLL.dll");
	hDll = LoadLibrary(toolDllPath);

	lpFun GainCorrection = (lpFun)GetProcAddress(hDll,"calcSPCgainmain");
	if (NULL==GainCorrection)
	{
		delete [] Calcbuffer;
		Calcbuffer = NULL;
		Addlog(_T("DLL 加载失败!\n"));
		FreeLibrary(hDll);
		return FALSE;
	} 
	char IniFilePath[1024]; 
	sprintf(IniFilePath,"%s",IniSpcGainPath.GetBuffer(IniSpcGainPath.GetLength()));
	int flag = GainCorrection(Calcbuffer,IniFilePath,pSPC);
	delete [] Calcbuffer;
	Calcbuffer = NULL;
	FreeLibrary(hDll);
	return 1;
}
int IMX258::ApplySpc(short *pSPC)
{
	if(NULL == pSPC) return 0;

	typedef int (*lpFun2)(unsigned short *ImageBuffer, char *IniFilePath, char *outname);
	
	HINSTANCE hDll;   //DLL句柄 
	CString m_szDirectory;
	GetCurentPath(m_szDirectory);
	m_szDirectory = m_szDirectory + _T("T_SPC\\IMX_258\\IMX258_SPC_DLL.dll");	
	hDll = LoadLibrary(m_szDirectory);
	if (NULL==hDll)
	{
		FreeLibrary(hDll);
		return FALSE;
	}
	
	lpFun2 JudgeSPCgain = (lpFun2)GetProcAddress(hDll,"judgeSPCgainmain");
	if (NULL==JudgeSPCgain)
	{
		FreeLibrary(hDll);
		return FALSE;
	} 
	if(I2cWrite( 0x0101,0,3) != 1) return 0; //V and H are both set by this. 	
	
	for (int i = 0;i<126;i++)
	{
		BYTE SPCData=0;
		SPCData=pSPC[i]&0xff;
		if(i<63)
			if(I2cWrite( 0xD04C+i,SPCData,3) != 1) return 0;
		else
			if(I2cWrite( 0xD08C+(i-63),SPCData,3) != 1) return 0;		
	}

	if(I2cWrite( 0x3051,0,3) != 1) return 0; //Ver2.0
	if(I2cWrite( 0x3052,0,3) != 1) return 0;
	if(I2cWrite( 0x7BCA,0,3) != 1) return 0;
	if(I2cWrite( 0x7BCB,0,3) != 1) return 0;
	if(I2cWrite( 0x7BC8,1,3) != 1) return 0;
	FreeLibrary(hDll);
	return 1; 
}
#endif

int IMX258::GetSensorExposure(uint &value)
{
#if 1
    uchar buf[] = {0, 0};
    int ec = i2cReadContinuous(EXPOSURE_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("IMX258::Call ReadContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    value = MAKEWORD(buf[0], buf[1]);
    return ERR_NoError;
#else
	USHORT ExpHigh;
	USHORT ExpLow;
	if(I2cRead(0x0202,&ExpHigh,3) != 1) return 0;
	if(I2cRead(0x0203,&ExpLow,3) != 1) return 0;
	exp=(ExpHigh<<8)+(ExpLow&0xFF);

	return 1;
#endif
} 
int IMX258::SetSensorExposure(uint value)
{
#if 1
    uchar buf[] = {MSB(value), LSB(value)};
    int ec = i2cWriteContinuous(EXPOSURE_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("IMX258::Call WriteContinuousI2c() failed[%2]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ERR_NoError;
#else
	USHORT ExpHigh=exp>>8;
	USHORT ExpLow=exp&0xFF;
	if(I2cWrite(0x0202,ExpHigh,3) != 1) return 0;
	if(I2cWrite(0x0203,ExpLow,3) != 1) return 0;

	return 1;
#endif
} 

int IMX258::GetSensorGain(uint &value)
{
#if 1
    uchar buf[] = {0, 0};
    int ec = i2cReadContinuous(SENSOR_GAIN_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("IMX258::Call ReadContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    value = MAKEWORD(buf[0], buf[1]);
    return ERR_NoError;
#else
	USHORT GainHigh;
	USHORT GainLow;
	if(I2cRead(0x0204,&GainHigh,3) != 1) return 0;
	if(I2cRead(0x0205,&GainLow,3) != 1) return 0;
	gain=(GainHigh<<8)+(GainLow&0xFF);

	return 1;
#endif
} 
int IMX258::SetSensorGain(uint value, bool bMultiple)
{
#if 1
    if (true == bMultiple)      //1x,2x,4x,8x,16x
    {
        value = 512 - (512 / value);
        //return SetSensorGain(value, false);
    }

    if (value > 480)    //Gain: 0 ~ 480
    {
        value = 480;
    }

    uchar buf[] = {MSB(value), LSB(value)};
    int ec = i2cWriteContinuous(SENSOR_GAIN_REG_H_ADDR, 2, buf, ARRAY_SIZE(buf));
    if (ec < 0) {
        QString strLog = QString("IMX258::Call WriteContinuousI2c() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ERR_NoError;
#else
	USHORT GainHigh=gain>>8;
	USHORT GainLow=gain&0xFF;
	if(I2cWrite(0x0204,GainHigh,3) != 1) return 0;
	if(I2cWrite(0x0205,GainLow,3) != 1) return 0;

	return 1;
#endif
} 


int IMX258::readOtpData(ushort startAddr, ushort endAddr, uchar *buf, ushort page)
{
#if 1
    if (nullptr == buf) {
        qCritical("IMX258::%s Error---buf is NULL", __FUNCTION__);
        return ERR_InvalidParameter;
    }

    uint bufferLen = endAddr - startAddr + 1;
    if (bufferLen > PAGE_LEN) {
        qCritical("IMX258::%s Error---nLen:%d is outside PAGE_LEN:%d", __FUNCTION__, bufferLen, PAGE_LEN);
        return ERR_InvalidParameter;
    }

    int ec = ERR_NoError;
    ec = I2cWrite(0x0A02, page, I2C_MODE_ADDR16_DATA8);     // select otp page
    ec |= I2cWrite(0x0A00, 0x01, I2C_MODE_ADDR16_DATA8);    // turn on otp read mode
    if (ec < 0) {
        qCritical("Turn on IMX258 otp read mode failed[%d]", ec);
        return ec;
    }

    bool checkFailed = true;
    ushort flag = 0;
    for (uint i = 0; i < 20; i++) { // check status repeat 20 times
        if (ERR_NoError == I2cRead(0x0A01, flag, I2C_MODE_ADDR16_DATA8)) {
            if ((flag & 0x01) == 0x01) {
                checkFailed = false;
                break;
            }
        }
        QThread::msleep(10);
    }
    if (checkFailed) {
        qCritical("Check IMX258 OTP status[0x%x] failed", flag);
        return ERR_Failed;
    }

    //read otp data to buff
    ec = i2cReadContinuous(startAddr, 2, buf, bufferLen);
    if (ec < 0) {
        qCritical("IMX258 read otp addr[0x%x] failed[%d]", startAddr, ec);
        return ec;
    }

    return ec;
#else
    if(NULL == ReadData) return 0;

    if(I2cWrite(0x0a02, Page, 3) != 1) return 0;
    if(I2cWrite(0x0a00, 0x01, 3) != 1) return 0;
    USHORT flag = 0;
    int cnt = 0;
    do
    {
        if(I2cRead(0x0a01, &flag, 3) != 1) return 0;
        cnt++;
    } while ( ( flag != 0x01 ) && ( cnt < 100 ) );
    if ( cnt >=100 )
    {
        return 0;
    }
    for ( int i = 0; i < 64; i++ )
    {
        if(I2cRead(0x0a04+i, ReadData+i, 3) != 1) return 0;
    }
    return 1;
#endif
}

