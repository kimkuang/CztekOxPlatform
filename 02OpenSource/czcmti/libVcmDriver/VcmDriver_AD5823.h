#ifndef VCMDRIVER_AD5823_H
#define VCMDRIVER_AD5823_H
#include "VcmDriver.h"

class VcmDriver_AD5823 : public VcmDriver
{
public:
    VcmDriver_AD5823(uint i2cAddr);

    int VcmReadCode(uint &value);
    int VcmWriteCode(uint value);
};

#endif // VCMDRIVER_AD5823_H
