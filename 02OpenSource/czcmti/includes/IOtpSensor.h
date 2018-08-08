#ifndef __IOTPSENSOR_H__
#define __IOTPSENSOR_H__
#include "czcmtidefs.h"
#include "IChannelController.h"
#include <string>
#include <list>
#include <QDebug>

#ifndef OTPSENSOR_API
#ifdef OTPSENSOR_EXPORTS
    #ifdef __GNUC__
        #define OTPSENSOR_API __attribute__((visibility("default")))
    #else
        #define OTPSENSOR_API __declspec(dllexport)
    #endif
#else
    #ifdef __GNUC__
        #define OTPSENSOR_API __attribute__((visibility("default")))
    #else
        #define OTPSENSOR_API __declspec(dllimport)
    #endif
#endif /* OTPSENSOR_EXPORTS */
#endif /* OTPSENSOR_API */

class IOtpSensor
{
public:
    virtual int BindChannelContext(T_ChannelContext *context) = 0;
    virtual int OtpRead(uint startAddr, uint endAddr, uchar *data, ushort page = 0) = 0;
    virtual int OtpWrite(uint startAddr, uint endAddr, const uchar *data, ushort page = 0) = 0;
};


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

OTPSENSOR_API void GetOtpSensorList(std::list<std::string> &otpSensorList);
OTPSENSOR_API IOtpSensor *CreateOtpSensor(const std::string &sensorName, uint i2cAddr);
OTPSENSOR_API void DestroyOtpSensor(IOtpSensor **otpSensor);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif // __IOTPSENSOR_H__
