#ifndef VCMDRIVER_LC898217_H
#define VCMDRIVER_LC898217_H
#include "VcmDriver.h"

#ifdef VCM_DRV_LC898217
#define VCM_CODE_BIT12                              (0x00 << 6)
#define VCM_CODE_BIT10                              (0x01 << 6)
#endif

class VcmDriver_LC898217 : public VcmDriver
{
public:
    VcmDriver_LC898217(uint i2cAddr);

    int VcmInitialize();
    int VcmReadCode(uint &value);
    int VcmWriteCode(uint value);
};

#endif // VCMDRIVER_LV8498_H
