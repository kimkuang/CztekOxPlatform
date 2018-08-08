#ifndef VcmDriver_DW9763_H
#define VcmDriver_DW9763_H
#include "VcmDriver.h"

class VcmDriver_DW9763 : public VcmDriver
{
public:
    VcmDriver_DW9763(uint i2cAddr);
    int VcmInitialize();
    int VcmReadCode(uint &value);
    int VcmWriteCode(uint value);
};

#endif // VcmDriver_DW9763_H
