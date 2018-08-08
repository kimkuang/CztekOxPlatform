#ifndef __GC2375_H__
#define __GC2375_H__
#include "ImageSensor.h"

class GC2375 : public ImageSensor
{
public:
    GC2375(const T_SensorSetting &sensorSetting);
    /*virtual int ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg);
	virtual int ApplyLsc(BYTE* raw8,int width,int height,int nLSCTarget,int ob,UCHAR* nLenCReg,int nLenCRegCount,int LSCGroup);
	virtual int spcCal(USHORT *imgBuf,short *pSPC);
    virtual int ApplySpc(short *pSPC);*/

    int ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg);
    int GetTemperature(int &temperature);
    int GetSensorFuseId(std::string &fuseId, bool bMultiple=false);
    int SetSensorExposure(uint value);
    int SetSensorGain(uint value, bool bMultiple=false);
    int GetSensorExposure(uint &value);
    int GetSensorGain(uint &value);
};
#endif  //end of __GC2375_H__
