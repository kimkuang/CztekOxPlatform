#ifndef VCMDRIVER_BU64241_H
#define VCMDRIVER_BU64241_H
#include "VcmDriver.h"

class VcmDriver_BU64241 : public VcmDriver
{
public:
    VcmDriver_BU64241(uint i2cAddr);

    int VcmReadCode(uint &value);
    int VcmWriteCode(uint value);
};

#endif // VCMDRIVER_BU64241_H
