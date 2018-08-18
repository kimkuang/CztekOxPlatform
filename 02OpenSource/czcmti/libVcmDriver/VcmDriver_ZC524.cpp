
#define VCM_DRV_ZC524

#include "VcmDriver_ZC524.h"

VcmDriver_ZC524::VcmDriver_ZC524(uint i2cAddr) :
    VcmDriver(i2cAddr)
{
}

int VcmDriver_ZC524::VcmInitialize()
{
    QString strLog = "";
    int nCode = 0;
    ushort AFValueH = ( nCode & 0xFFF0) >> 4;
    ushort AFValueL = ( nCode & 0x0F) << 4;
    ushort regAddrs[] = {0xEC, 0x80, 0x81, 0xDC, AFValueH};
    ushort regVals[]  = {0xAB, 0x0C, 0x64, 0x59, (ushort)(AFValueL | (ushort)LSC_CTL_TWO_CODE_PER_STEP)};
    uint regNum = sizeof(regAddrs)/sizeof(regAddrs[0]);
    if (ERR_NoError != m_channelController->WriteDiscreteI2c(m_i2cAddr, 100, RB_ADDR8_DATA8, regAddrs, regVals, regNum))
    {
        strLog.sprintf("VcmDriver_DW9714::%s---WriteDiscreteI2c Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
}

int VcmDriver_ZC524::VcmReadCode(uint &value)
{
    (void)value;

    return ERR_NoError;
}

int VcmDriver_ZC524::VcmWriteCode(uint value)
{
    QString strLog = "";
    if (value >= 1023)
    {
        value = 1023;
    }

    uint slaveAddr = m_i2cAddr;
    uint speedkHz = 100;
    uint mode = RB_ADDR8_DATA8;
    /**
      * Normal模式下的Code值的写入方式是：
      *     1、把Reg1和Reg2两个寄存器凑成一个16位的数，即WORD(Reg1, Reg2)，Reg1是高8位，Reg2是低8位。
      *     2、Bit[15]:PD模式设置，0：PD OFF, 1：PD ON
      *     3、Bit[14]:FLAG位
      *     4、Bit[13:4]:Code值，范围0~1023
      *     5、Bit[3:2]:LSC Codes per Step
      *     6、Bit[1:0]:Step Period
      * ----20180411
    **/
    T_RegConf reg;
    reg.Addr = ( value & 0x3FF0) >> 4;
    reg.Value = (( value & 0x0F) << 4) | LSC_CTL_TWO_CODE_PER_STEP;
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

    strLog.sprintf("VcmDriver_DW9714::%s---WriteDiscreteI2c Error\r\n", __FUNCTION__);
    m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
    return ERR_Failed;
}
