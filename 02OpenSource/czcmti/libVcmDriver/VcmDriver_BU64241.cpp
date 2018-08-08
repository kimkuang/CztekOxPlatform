#include "VcmDriver_BU64241.h"

VcmDriver_BU64241::VcmDriver_BU64241(uint i2cAddr) :
    VcmDriver(i2cAddr)
{
}

int VcmDriver_BU64241::VcmReadCode(uint &value)
{
    (void)value;
    return ERR_NotImplemented;
}

int VcmDriver_BU64241::VcmWriteCode(uint value)
{
    T_RegConf reg;
    reg.Addr  = 0xC4;          // 输出使能, ISRC Mode Enabled
    reg.Addr |= value >> 8; // 高字节
    reg.Value = value;      // 低字节

    uint slaveAddr = m_i2cAddr;
    uint speedkHz = 100;
    uint mode = RB_NORMAL;
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
