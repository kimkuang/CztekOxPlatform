#ifndef VCMDRIVER_LC898219_H
#define VCMDRIVER_LC898219_H
#include "VcmDriver.h"

class VcmDriver_LC898219 : public VcmDriver
{
public:
    VcmDriver_LC898219(uint i2cAddr);

    int VcmInitialize();
    int VcmReadCode(uint &value);
    int VcmWriteCode(uint value);
};

#endif // VCMDRIVER_LV8498_H
