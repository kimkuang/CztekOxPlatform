
#ifndef __VCMDRIVER_AK7371_H__
#define __VCMDRIVER_AK7371_H__
#include "VcmDriver.h"

class VcmDriver_AK7371 : public VcmDriver
{
public:
    VcmDriver_AK7371(uint i2cAddr);

    int VcmReadCode(uint &value);
    int VcmWriteCode(uint value);
    int VcmInitialize();
};
#endif
