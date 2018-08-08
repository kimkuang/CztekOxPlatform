#ifndef __GC5025_H__
#define __GC5025_H__
#include "ImageSensor.h"

#ifdef IMG_DRV_GC5025
#define REG_OTP_MODE_ADDR                                           0xD4
    #define OTP_EN                                                      (0x01 << 7)
    #define OTP_BUSY                                                    (0x01 << 5)
    #define OTP_WR_BYTE                                                 (0x00 << 4)
    #define OTP_WR_BIT                                                  (0x01 << 4)
    #define OTP_RD_BYTE                                                 (0x00 << 3)
    #define OTP_RD_BIT                                                  (0x01 << 3)
    #define OTP_PAGE0                                                   (0x00 << 2)
    #define OTP_PAGE1                                                   (0x01 << 2)

#define REG_OTP_ACCESS_ADDR_L                                       0xD5

#define REG_OTP_WR_OR_RD                                            0xF3
    #define OTP_WR                                                      (0x01 << 6)
    #define OTP_RD                                                      (0x01 << 5)

#define REG_PLL_MODE                                                0xF7
    #define PLL_EN                                                      (0x01 << 0)

#define REG_ANALOG_PWC                                              0xF9
    #define ANALOG_PWDN                                                 (0x01 << 0)

#define REG_OTP_WR_DATA_ADDR                                        0xD6
#define REG_OTP_RD_DATA_ADDR                                        0xD7

#define ANALOG_GAIN_1                                               64   // 1.00x
#define ANALOG_GAIN_2                                               92   // 1.445x

/** Channel GAIN Reg Addr Define **/
#define GAIN_DEFAULT                                                0x0100
#define GAIN_GREEN1_HIGH_ADDR                                       0xC6
#define GAIN_GREEN1_LOW_ADDR                                        0xC4    //Reg[0xC4]->Bit[6:4]
#define GAIN_BLUE_HIGH_ADDR                                         0xC8
#define GAIN_BLUE_LOW_ADDR                                          0xC5    //Reg[0xC5]->Bit[6:4]
#define GAIN_RED_HIGH_ADDR                                          0xC7
#define GAIN_RED_LOW_ADDR                                           0xC4    //Reg[0xC4]->Bit[2:0]
#define GAIN_GREEN2_HIGH_ADDR                                       0xC9
#define GAIN_GREEN2_LOW_ADDR                                        0xC5    //Reg[0xC5]->Bit[2:0]

/** Exposure Reg Addr Define **/
#define EXPOSURE_REG_H_ADDR                                         0x03
#define EXPOSURE_REG_L_ADDR                                         0x04

/** Analog Gain Reg Addr Define **/
#define SENSOR_GAIN_REG_ADDR                                        0xB6

/** Fuse ID Reg Addr Define **/
#define FUSE_ID_PAGE                                                0x01
#define FUSE_ID_ADDR_START                                          0x76    //Bit地址：0x03B0
#define FUSE_ID_ADDR_END                                            0x7E
#define FUSE_ID_LEN                                                 (FUSE_ID_ADDR_END - FUSE_ID_ADDR_START + 1)
#endif

class GC5025 : public ImageSensor
{
public:
    GC5025(const T_SensorSetting &sensorSetting);

    int ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg);
    int GetTemperature(int &temperature);
    int GetSensorFuseId(std::string &fuseId, bool bMultiple=false);
    int SetSensorExposure(uint value);
    int SetSensorGain(uint value, bool bMultiple=false);
    int GetSensorExposure(uint &value);
    int GetSensorGain(uint &value);

private:
    int readOtpData(ushort startAddr, ushort endAddr, uchar *pBuf, ushort page = 0);
    virtual int WriteReg(USHORT startAddr, USHORT endAddr, uchar *buf, USHORT page = 0);

    /*virtual int GetTemperature(USHORT &temperature) ;
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
    virtual int WriteReg(USHORT startAddr, USHORT endAddr, USHORT *buf,USHORT page = 0);*/
};
#endif  //end of __GC5025_H__
