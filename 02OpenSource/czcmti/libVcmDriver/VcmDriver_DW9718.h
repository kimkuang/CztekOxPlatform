#ifndef VCMDRIVER_DW9718_H
#define VCMDRIVER_DW9718_H
#include "VcmDriver.h"

class VcmDriver_DW9718 : public VcmDriver
{
public:
    VcmDriver_DW9718(uint i2cAddr);

    int VcmReadCode(uint &value);
    int VcmWriteCode(uint value);
    int VcmInitialize();
};

#endif // VCMDRIVER_DW9718_H
