#ifndef VCMDRIVER_A1457_H
#define VCMDRIVER_A1457_H
#include "VcmDriver.h"

class VcmDriver_A1457 : public VcmDriver
{
public:
    VcmDriver_A1457(uint i2cAddr);

    int VcmReadCode(uint &value);
    int VcmWriteCode(uint value);
};

#endif // VCMDRIVER_A1457_H
