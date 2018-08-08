
#ifndef __VCMDRIVER_AK7374_H__
#define __VCMDRIVER_AK7374_H__
#include "VcmDriver.h"

class VcmDriver_AK7374 : public VcmDriver
{
public:
    VcmDriver_AK7374(uint i2cAddr);

    int VcmReadCode(uint &value);
    int VcmWriteCode(uint value);
    int VcmInitialize();
};

#endif

