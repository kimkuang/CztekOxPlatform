#include "VcmDriver_LV8498.h"

VcmDriver_LV8498::VcmDriver_LV8498(uint i2cAddr) :
    VcmDriver(i2cAddr)
{
}

int VcmDriver_LV8498::VcmReadCode(uint &value)
{
    (void)value;
    return ERR_NotImplemented;
}

int VcmDriver_LV8498::VcmWriteCode(uint value)
{
    T_RegConf reg;
    reg.Addr = (value & 0x3F) << 6;
    reg.Value = (value & 0xF0) << 8;

    uint slaveAddr = m_i2cAddr;
    uint speedkHz = 100;
    uint mode = RB_ADDR8_DATA16;
    ushort regAddrs[] = { (ushort)reg.Addr };
    ushort regVals[] = { (ushort)reg.Value };
    uint regNum = 1;

    for (int i = 0; i < 1; i++)
    {
        if (ERR_NoError == m_channelController->WriteDiscreteI2c(slaveAddr, speedkHz, mode, regAddrs, regVals, regNum))
        {
            return ERR_NoError;
        }
    }

    return ERR_Failed;
}
