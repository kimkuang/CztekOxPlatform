#ifndef IVCMDRIVER_H
#define IVCMDRIVER_H
#include "czcmtidefs.h"
#include "IChannelController.h"
#include <string>
#include <QThread>

#ifndef VCMDRIVER_API
#ifdef VCMDRIVER_EXPORTS
    #ifdef __GNUC__
        #define VCMDRIVER_API __attribute__((visibility("default")))
    #else
        #define VCMDRIVER_API __declspec(dllexport)
    #endif
#else
    #ifdef __GNUC__
        #define VCMDRIVER_API __attribute__((visibility("default")))
    #else
        #define VCMDRIVER_API __declspec(dllimport)
    #endif
#endif /* VCMDRIVER_EXPORTS */
#endif /* VCMDRIVER_API */

class IVcmDriver
{
public:
    virtual int BindChannelContext(T_ChannelContext *context) = 0;
    virtual int VcmInitialize() = 0;
    virtual int VcmReadCode(uint &value) = 0;
    virtual int VcmWriteCode(uint value) = 0;
    virtual int VcmFinalize() = 0;
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

VCMDRIVER_API void GetVcmDriverIcList(std::vector<std::string> &driverIcList);
VCMDRIVER_API IVcmDriver *CreateVcmDriver(const std::string &driverIcName, uint i2cAddr);
VCMDRIVER_API void DestroyVcmDriver(IVcmDriver **vcmDriver);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // IVCMDRIVER_H
