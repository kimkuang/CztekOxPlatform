#ifndef VCMDRIVER_DW9800_H
#define VCMDRIVER_DW9800_H
#include "VcmDriver.h"

/** Reg[0x02] **/
#define CTL_CMD_PD_MODE                                             (0x01 << 0)
#define CTL_CMD_RING_MODE                                           (0x01 << 1)

/** Reg[0x05] **/
#define REG_STATUS_BUSY                                             (0x01 << 0)
#define REG_STATUS_TSD                                              (0x01 << 4)

/** Reg[0x06] **/
#define SAC_MODE_2                                                  (0x00 << 6)
#define SAC_MODE_3                                                  (0x01 << 6)
#define SAC_MODE_4                                                  (0x02 << 6)
#define SAC_MODE_5                                                  (0x03 << 6)

/** Reg[0x07] **/
//#define REG_STATUS_BUSY                                             (0x01 << 0)
//#define REG_STATUS_TSD                                              (0x01 << 4)

class VcmDriver_DW9800 : public VcmDriver
{
public:
    VcmDriver_DW9800(uint i2cAddr);

    int VcmInitialize();
    int VcmReadCode(uint &value);
    int VcmWriteCode(uint value);
};

#endif // VCMDRIVER_DW9800_H
