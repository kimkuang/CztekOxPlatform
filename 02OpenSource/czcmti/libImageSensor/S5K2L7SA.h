#ifndef __S5K2L7SA_H__
#define __S5K2L7SA_H__
#include "ImageSensor.h"

class S5K2L7SA : public ImageSensor
{
public:
    S5K2L7SA(const T_SensorSetting &sensorSetting);

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

    virtual int ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg);
    virtual int GetTemperature(int &temperature);
    virtual int GetSensorFuseId(std::string &fuseId, bool bMultiple=false);
    virtual int SetSensorExposure(uint value);
    virtual int SetSensorGain(uint value, bool bMultiple=false);
    virtual int GetSensorExposure(uint &value);
    virtual int GetSensorGain(uint &value);

private:
    int setDirectPage(ushort page);
    int readOtpData(ushort startAddr, ushort endAddr, uchar *buf, ushort page);
};

#endif  // __S5K2L7SA_H__
