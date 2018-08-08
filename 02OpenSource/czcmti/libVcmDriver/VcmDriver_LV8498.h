#ifndef VCMDRIVER_LV8498_H
#define VCMDRIVER_LV8498_H
#include "VcmDriver.h"

class VcmDriver_LV8498 : public VcmDriver
{
public:
    VcmDriver_LV8498(uint i2cAddr);

    int VcmReadCode(uint &value);
    int VcmWriteCode(uint value);
};

#endif // VCMDRIVER_LV8498_H
