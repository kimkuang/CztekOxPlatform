#include "VcmDriver_A1457.h"
#include "czcmtidefs.h"

VcmDriver_A1457::VcmDriver_A1457(uint i2cAddr) :
    VcmDriver(i2cAddr)
{
}

int VcmDriver_A1457::VcmReadCode(uint &value)
{
    (void)value;
    return ERR_NotImplemented;
}

int VcmDriver_A1457::VcmWriteCode(uint value)
{
    T_RegConf ctrlRegs[3];
    // 输出使能寄存器
    ctrlRegs[0].Addr = 0x02;
    ctrlRegs[0].Value = 0x10;
    // 数据输出,需要先发送高位后发送低位
    ctrlRegs[1].Addr = 0x00;
    ctrlRegs[1].Value = (value >> 8) & 0x03;
    ctrlRegs[2].Addr = 0x02;
    ctrlRegs[2].Value = value & 0xFF;

    uint slaveAddr = m_i2cAddr;
    uint speedkHz = 100;
    uint mode = RB_NORMAL;
    ushort regAddrs[] = { (ushort)ctrlRegs[0].Addr, (ushort)ctrlRegs[1].Addr, (ushort)ctrlRegs[2].Addr };
    ushort regVals[] = { (ushort)ctrlRegs[0].Value, (ushort)ctrlRegs[1].Value, (ushort)ctrlRegs[2].Value };
    uint regNum = 3;

    for (int i = 0; i < 1; i++)
    {
        if (ERR_NoError == m_channelController->WriteDiscreteI2c(slaveAddr, speedkHz, mode, regAddrs, regVals, regNum))
        {
            return ERR_NoError;
        }
    }

    return ERR_Failed;
}
