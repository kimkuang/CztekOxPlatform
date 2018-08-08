#ifndef __OV16885_H__
#define __OV16885_H__

#include "ImageSensor.h"

#ifdef IMG_DRV_OV16885

#define TEMPERATURE_CTL_REG_ADDR                                        0x4D12
#define TEMPERATURE_VAL_REG_ADDR                                        0x4D13

#define AWB_GAIN_R_REG_H_ADDR                                           0x5106
#define AWB_GAIN_R_REG_L_ADDR                                           0x5107

#define AWB_GAIN_GB_REG_H_ADDR                                          0x5102
#define AWB_GAIN_GB_REG_L_ADDR                                          0x5103

#define AWB_GAIN_GR_REG_H_ADDR                                          0x5104
#define AWB_GAIN_GR_REG_L_ADDR                                          0x5105

#define AWB_GAIN_B_REG_H_ADDR                                           0x5100
#define AWB_GAIN_B_REG_L_ADDR                                           0x5101

#define EXPOSURE_REG_H_ADDR                                             0x3501
#define EXPOSURE_REG_L_ADDR                                             0x3502

#define SENSOR_GAIN_REG_H_ADDR                                          0x350C
#define SENSOR_GAIN_REG_L_ADDR                                          0x350D

#define ISP_CTL_REG_0                                                   0x5000
    #define OTP_EN                                                          (0x01 << 3)

/** Fuse ID Reg Addr Define **/
#define FUSE_ID_ADDR_START                                              0x6000
#define FUSE_ID_ADDR_END                                                0x600F
    #define FUSE_ID_LEN                                                 (FUSE_ID_ADDR_END - FUSE_ID_ADDR_START + 1)

#endif

class OV16885 :	public ImageSensor
{
public:
    OV16885(const T_SensorSetting &sensorSetting);

    virtual int ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg);
    virtual int GetTemperature(int &temperature);
    virtual int GetSensorFuseId(std::string &fuseId, bool bMultiple=false);
    virtual int SetSensorExposure(uint value);
    virtual int SetSensorGain(uint value, bool bMultiple=false);
    virtual int GetSensorExposure(uint &value);
    virtual int GetSensorGain(uint &value);

private:
    int readOtpData(ushort startAddr, ushort endAddr, uchar *buf, ushort page);
#if 0
	virtual int GetTemperature(USHORT &temperature) ;
	virtual int GetFuseID(CString &FuseID) ;
	virtual int Init() ;
	virtual int ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg);
	virtual int ApplyLsc(BYTE* raw8,int width,int height,int nLSCTarget,int ob,UCHAR* nLenCReg,int nLenCRegCount,int LSCGroup);
	virtual int spcCal(USHORT *imgBuf,short *pSPC);
	virtual int ApplySpc(short *pSPC);
	virtual int GetLsc(BYTE* P10Buff,int width,int height,int nLSCTarget,int ob,UCHAR* nLenCReg,int nLenCRegCount,int LSCGroup);


	virtual int ReadExp( int &exp);  
	virtual int WriteExp( int exp);  

	virtual int ReadGain( USHORT &gain);  
	virtual int WriteGain( USHORT gain); 
	int  OV16885_read_i2c(USHORT addr,USHORT & data);
	int OV16885_readAll(USHORT startAddr,USHORT endAddr,USHORT * out_buf);
	int OV16885_write_i2c(USHORT addr,USHORT val);
	virtual int ReadReg(USHORT startAddr, USHORT endAddr, USHORT *buf,USHORT page = 0);
	virtual int WriteReg(USHORT startAddr, USHORT endAddr, USHORT *buf,USHORT page = 0);
#endif
};
#endif
