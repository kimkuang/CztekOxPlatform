#include "VcmDriver_DW9761.h"

VcmDriver_DW9761::VcmDriver_DW9761(uint i2cAddr) :
    VcmDriver(i2cAddr)
{
}

int VcmDriver_DW9761::VcmReadCode(uint &value)
{
    (void)value;
    return ERR_NotImplemented;
}

int VcmDriver_DW9761::VcmWriteCode(uint value)
{
    T_RegConf reg;
    reg.Addr = 0x03;        // VCM_CODE_MSB 地址0x03采用连续写的方式
    reg.Value = value;   // 数据

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
