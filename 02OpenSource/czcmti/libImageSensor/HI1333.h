  
#ifndef __HI1333_H__
#define __HI1333_H__
#include "ImageSensor.h"

//I2C Mode    :0:Normal 8Addr,8Data;  1:Samsung 8 Addr,8Data; 
//I2C Mode    :2:Micron 8 Addr,16Data
//I2C Mode    :3:Stmicro 16Addr,8Data;4:Micron2 16 Addr,16Data
#define HI1333_I2C_MODE									3

/*#define HI1333_REG_EXP_TIM_HW							0x0073
#define HI1333_REG_EXP_TIM_H							0x0074
#define HI1333_REG_EXP_TIM_L							0x0075

#define HI1333_REG_AGAIN_H								0x0076
#define HI1333_REG_AGAIN_L								0x0077*/

/*#define HI1333_REG_DGAIN_GR_H							0x0078
#define HI1333_REG_DGAIN_GR_L							0x0079
#define HI1333_REG_DGAIN_GB_H							0x007A
#define HI1333_REG_DGAIN_GB_L							0x007B
#define HI1333_REG_DGAIN_R_H							0x007C
#define HI1333_REG_DGAIN_R_L							0x007D
#define HI1333_REG_DGAIN_B_H							0x007E
#define HI1333_REG_DGAIN_B_L							0x007F*/

#define HI1333_REG_OTP_CMD								0x0702	//OTP读写方向寄存器
#define HI1333_REG_OTP_WDATA							0x0706	//I2C写数据寄存器
#define HI1333_REG_OTP_RDATA							0x0708	//I2C读数据寄存器

#define HI1333_REG_OTP_ADDR_H							0x070A	//OTP地址寄存器高
#define HI1333_REG_OTP_ADDR_L							0x070B	//OTP地址寄存器低

#define HI1333_REG_MODEL_ID_H							0x0F16
#define HI1333_REG_MODEL_ID_L							0x0F17

/** Exposure Reg Addr Define **/
#define EXPOSURE_REG_H_ADDR                             0x0074
#define EXPOSURE_REG_L_ADDR                             0x0075

/** Analog Gain Reg Addr Define **/
#define SENSOR_GAIN_REG_H_ADDR                          0x0076
#define SENSOR_GAIN_REG_L_ADDR                          0x0077

/** Channel GAIN Reg Addr Define **/
#define GAIN_DEFAULT                                    0x0100
#define GAIN_GREEN1_HIGH_ADDR                           0x0078
#define GAIN_GREEN1_LOW_ADDR                            0x0079
#define GAIN_BLUE_HIGH_ADDR                             0x007E
#define GAIN_BLUE_LOW_ADDR                              0x007F
#define GAIN_RED_HIGH_ADDR                              0x007C
#define GAIN_RED_LOW_ADDR                               0x007D
#define GAIN_GREEN2_HIGH_ADDR                           0x007A
#define GAIN_GREEN2_LOW_ADDR                            0x007B

/** Fuse ID Reg Addr Define **/
#define FUSE_ID_ADDR_START                              0x0001
#define FUSE_ID_ADDR_END                                0x0009
#define FUSE_ID_LEN                                     (FUSE_ID_ADDR_END - FUSE_ID_ADDR_START + 1)

#define __IN__
#define __OUT__

#define HI1333_OTP_RD_MODE								0x01
#define HI1333_OTP_WT_MODE								0x02

class HI1333 : public ImageSensor
{
public:
    HI1333(const T_SensorSetting &sensorSetting);

#if 1
    int ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg);
    int GetTemperature(int &temperature);
    int GetSensorFuseId(std::string &fuseId, bool bMultiple=false);
    int SetSensorExposure(uint value);
    int SetSensorGain(uint value, bool bMultiple=false);
    int GetSensorExposure(uint &value);
    int GetSensorGain(uint &value);

private:
    int writeOtpData(ushort startAddr, ushort endAddr, __IN__ uchar *pBuf, ushort page);
    int readOtpData(ushort startAddr, ushort endAddr, __OUT__ uchar *pBuf, ushort page);
#else
	BOOL ReadOTPPage(int page, USHORT *ReadData);
	virtual int GetTemperature(USHORT &temperature) ;
	virtual int GetFuseID(CString &FuseID) ;
	virtual int Init() ;
	virtual int ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg);
	virtual int ApplyLsc(BYTE* raw8,int width,int height,int nLSCTarget,int ob,UCHAR* nLenCReg,int nLenCRegCount,int LSCGroup);
	virtual int spcCal(USHORT *imgBuf,short *pSPC);
	virtual int ApplySpc(short *pSPC);

	virtual int ReadExp( int &exp);  
	virtual int WriteExp( int exp);  

	virtual int ReadGain( USHORT &gain);  
	virtual int WriteGain( USHORT gain);  
	virtual int ReadReg(USHORT startAddr, USHORT endAddr, USHORT *buf,USHORT page = 0);
	virtual int WriteReg(USHORT startAddr, USHORT endAddr, USHORT *buf,USHORT page = 0);
#if 0
private:
	void HI1333_I2C_WtOtpBy(USHORT addr, unsigned char val);
	USHORT HI1333_I2C_RdOtpBy(USHORT addr);
	BOOL HI1333_I2C_WtOtpData(USHORT addr, __IN__ unsigned char *pBuf, unsigned int nLen);
	BOOL HI1333_I2C_RdOtpData(USHORT addr, __OUT__ unsigned char *pBuf, unsigned int nLen);
#endif
	BOOL HI1333_I2C_WtOtpData(USHORT addr, __IN__ unsigned char *pBuf, unsigned int nLen);
	BOOL HI1333_I2C_RdOtpData(USHORT addr, __OUT__ unsigned char *pBuf, unsigned int nLen);
#endif
};
#endif  //end of __HI1333_H__
