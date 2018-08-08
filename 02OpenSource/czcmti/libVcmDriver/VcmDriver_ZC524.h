#ifndef __VCMDRIVER_ZC524_H__
#define __VCMDRIVER_ZC524_H__
#include "VcmDriver.h"

#ifdef VCM_DRV_ZC524
/** Normal Mode Reg Define **/
#define LSC_CTL_NO_SRC                                          (0x00 << 2)
#define LSC_CTL_ONE_CODE_PER_STEP                               (0x01 << 2)
#define LSC_CTL_TWO_CODE_PER_STEP                               (0x02 << 2)
#define LSC_CTL_FOUR_CODE_PER_STEP                              (0x03 << 2)
#endif

class VcmDriver_ZC524 : public VcmDriver
{
public:
    VcmDriver_ZC524(uint i2cAddr);

    int VcmReadCode(uint &value);
    int VcmWriteCode(uint value);
    int VcmInitialize();
};
#endif  //end of __VCMDRIVER_ZC524_H__
