#ifndef __IIMAGESENSOR_H__
#define __IIMAGESENSOR_H__
#include "czcmtidefs.h"
#include "IChannelController.h"
#include <string>
#include <QDebug>

#ifndef IMAGESENSOR_API
#ifdef IMAGESENSOR_EXPORTS
    #ifdef __GNUC__
        #define IMAGESENSOR_API __attribute__((visibility("default")))
    #else
        #define IMAGESENSOR_API __declspec(dllexport)
    #endif
#else
    #ifdef __GNUC__
        #define IMAGESENSOR_API __attribute__((visibility("default")))
    #else
        #define IMAGESENSOR_API __declspec(dllimport)
    #endif
#endif /* IMAGESENSOR_EXPORTS */
#endif /* IMAGESENSOR_API */

class IImageSensor
{
public:
    virtual int ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg) = 0;
    virtual int ApplyLsc(const uchar* raw8,int width,int height,int nLSCTarget,int ob,uchar* nLenCReg,int nLenCRegCount,int LSCGroup) = 0;
    virtual int ApplySpc(short *pSPC) = 0;

    virtual int BindChannelContext(T_ChannelContext *context) = 0;
    virtual int GetSensorFuseId(std::string &fuseId, bool bUseMasterI2C = false) = 0;
    virtual int SetSensorExposure(uint value) = 0;
    virtual int SetSensorGain(uint value, bool bMultiple=false) = 0;
    virtual int GetSensorExposure(uint &value) = 0;
    virtual int GetSensorGain(uint &value) = 0;
    virtual int GetTemperature(int &temperature) = 0;

    virtual int WriteRegisterList(const QList<T_RegConf> &regList) = 0;
};


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

IMAGESENSOR_API IImageSensor *CreateImageSensor(const T_SensorSetting &sensorSetting);
IMAGESENSOR_API void DestroyImageSensor(IImageSensor **imageSensor);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif // __IIMAGESENSOR_H__
