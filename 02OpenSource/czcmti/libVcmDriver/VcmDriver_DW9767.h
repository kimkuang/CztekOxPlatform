#ifndef VcmDriver_DW9767_H
#define VcmDriver_DW9767_H
#include "VcmDriver.h"

class VcmDriver_DW9767 : public VcmDriver
{
public:
    VcmDriver_DW9767(uint i2cAddr);
    int VcmInitialize();
    int VcmReadCode(uint &value);
    int VcmWriteCode(uint value);
};

#endif // VcmDriver_DW9767_H
