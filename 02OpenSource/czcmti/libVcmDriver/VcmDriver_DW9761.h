#ifndef VCMDRIVER_DW9761_H
#define VCMDRIVER_DW9761_H
#include "VcmDriver.h"

class VcmDriver_DW9761 : public VcmDriver
{
public:
    VcmDriver_DW9761(uint i2cAddr);

    int VcmReadCode(uint &value);
    int VcmWriteCode(uint value);
};

#endif // VCMDRIVER_DW9761_H
