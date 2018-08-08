#include "VcmDriver_DW9718.h"

VcmDriver_DW9718::VcmDriver_DW9718(uint i2cAddr) :
    VcmDriver(i2cAddr)
{
}

int VcmDriver_DW9718::VcmReadCode(uint &value)
{
    ushort regAddrs[] = {0x02, 0x03};
    ushort regVals[] = {0x00, 0x00};
    uint regNum = sizeof(regAddrs)/sizeof(regAddrs[0]);
    if (ERR_NoError != m_channelController->ReadDiscreteI2c(m_i2cAddr, 100, RB_ADDR8_DATA8, regAddrs, regVals, regNum))
    {
        return ERR_Failed;
    }

    value = ((regVals[0]&0x03)<<8) + regVals[1];
    return ERR_NoError;
}

int VcmDriver_DW9718::VcmInitialize()
{
    ushort code = 0;
    ushort regAddrs[] = {0x00, 0x00, 0x01, 0x05, 0x02,            0x03};
    ushort regVals[]  = {0x01, 0x00, 0x04, 0x00, ushort(code>>8), ushort(code&0xff)};
    uint regNum = sizeof(regAddrs)/sizeof(regAddrs[0]);
    if (ERR_NoError != m_channelController->WriteDiscreteI2c(m_i2cAddr, 100, RB_ADDR8_DATA8, regAddrs, regVals, regNum))
    {
        return ERR_Failed;
    }

    return ERR_NoError;
}

int VcmDriver_DW9718::VcmWriteCode(uint value)
{
    if (value > 1023)
    {
        value = 1023;
    }

//    if (ERR_NoError != Init())
//    {
//        return ERR_Failed;
//    }

    T_RegConf reg;
    reg.Addr = 0x02;        // VCM_CODE_MSB 地址0x02采用连续写的方式
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
