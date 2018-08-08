#ifndef __VCMDRIVER_DW9714_H__
#define __VCMDRIVER_DW9714_H__
#include "VcmDriver.h"

/** Normal Mode Reg Define **/
#define LSC_CTL_NO_SRC                                          (0x00 << 2)
#define LSC_CTL_ONE_CODE_PER_STEP                               (0x01 << 2)
#define LSC_CTL_TWO_CODE_PER_STEP                               (0x02 << 2)
#define LSC_CTL_FOUR_CODE_PER_STEP                              (0x03 << 2)

class VcmDriver_DW9714 : public VcmDriver
{
public:
    VcmDriver_DW9714(uint i2cAddr);

    int VcmReadCode(uint &value);
    int VcmWriteCode(uint value);
    int VcmInitialize();
};

#endif
