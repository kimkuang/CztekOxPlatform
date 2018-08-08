#include "VcmDriver_DW9800.h"

VcmDriver_DW9800::VcmDriver_DW9800(uint i2cAddr) :
    VcmDriver(i2cAddr)
{
}

int VcmDriver_DW9800::VcmInitialize()
{
    uint slaveAddr = m_i2cAddr;
    uint speedkHz = 100;
    uint mode = RB_NORMAL;

    ushort regAddrs[] = {0x02, 0x02};
    ushort regVals[] = {0x01, 0x00};
    uint regNum = sizeof(regAddrs)/sizeof(regAddrs[0]);

    if (ERR_NoError != m_channelController->WriteDiscreteI2c(slaveAddr, speedkHz, mode, regAddrs, regVals, regNum))
    {
        return ERR_Failed;
    }

    QThread::msleep(30);

    ushort regAddrs2[] = {0x02, 0x06, 0x07};
    ushort regVals2[] = {CTL_CMD_RING_MODE, SAC_MODE_3, 0x60};
    regNum = sizeof(regAddrs2)/sizeof(regAddrs2[0]);

    if (ERR_NoError != m_channelController->WriteDiscreteI2c(slaveAddr, speedkHz, mode, regAddrs2, regVals2, regNum))
    {
        return ERR_Failed;
    }

    QThread::msleep(30);

    ushort regAddrs3[] = {0x03, 0x04};
    ushort regVals3[] = {0x02, 0x00};
    regNum = sizeof(regAddrs3)/sizeof(regAddrs3[0]);

    if (ERR_NoError != m_channelController->WriteDiscreteI2c(slaveAddr, speedkHz, mode, regAddrs3, regVals3, regNum))
    {
        return ERR_Failed;
    }

    QThread::msleep(100);

    return ERR_NoError;
}

int VcmDriver_DW9800::VcmReadCode(uint &value)
{
    ushort regAddrs[] = {0x03, 0x04};
    ushort regVals[] = {0x00, 0x00};
    uint regNum = sizeof(regAddrs)/sizeof(regAddrs[0]);
    if (ERR_NoError != m_channelController->ReadDiscreteI2c(m_i2cAddr, 100, RB_ADDR8_DATA8, regAddrs, regVals, regNum))
    {
        return ERR_Failed;
    }

    value = ((regVals[0]&0x03)<<8) + regVals[1];
    return ERR_NoError;
}

int VcmDriver_DW9800::VcmWriteCode(uint value)
{
#if 1
    /**
      * DW9800是一款中置马达，Code寄存器是10Bit，范围0x00~0x3FF，对应的电流范围-60mA~60mA。所以，本函数的参数Code
      * 值可以直接写入寄存器，Code值从0变化到1023时，电流从~60mA~0mA~60mA。芯片复位后，输出电流为0mA，对应的Code值
      * 为512。马达处于中间位置。
      * ----20180409
    **/
    if (value >= 1023)
    {
        value = 1023;
    }

    T_RegConf reg;
    reg.Addr = 0x03;        // VCM_CODE_MSB 地址0x02采用连续写的方式
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
#else
    value = 512 + value / 2; // 正电流范围是512~1023
    T_RegConf reg;
    // 向MSB0x03,LSB0x04写入位置数据
    reg.Addr = 0x03;
    reg.Value = value;

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
#endif
}
