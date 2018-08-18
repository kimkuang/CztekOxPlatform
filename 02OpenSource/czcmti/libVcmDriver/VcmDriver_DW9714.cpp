
/**
  * DW9714的操作模式分为Normal Mode和Advanced Mode两种，这两种的I2C操作方式是不一样的。查看手册时，仔细查看，发现这两种模式的I2C时序描述不同，具体为：
  *     1、Normal Mode：根据手册描述，I2C时序图中看不出有【寄存器地址】这个字节，而是采用【起始 + 芯片地址 + 寄存器1 + 寄存器2 + 停止】波形序列。
  *     2、Advanced Mode：根据I2C时序图，此模式下的I2C时序就是标准的I2C时序，即【起始 + 芯片地址 + 寄存器地址 + 寄存器值 + 停止】波形序列。
  * 根据DW9714手册中的上电状态机转换图可知，DW9714开机后默认进入Noraml模式，所以，上电开机后只能使用Noraml的I2C波形序列来操作。
  * 注意：本驱动代码也是使用的Noraml模式的I2C时序图波形来操作的。
  * ----20180411
**/
#include "VcmDriver_DW9714.h"

VcmDriver_DW9714::VcmDriver_DW9714(uint i2cAddr) :
    VcmDriver(i2cAddr)
{
}

/**
  * 注意：本驱动代码也是使用的Noraml模式的I2C时序图波形来操作的。
  * ----20180411
**/
int VcmDriver_DW9714::VcmInitialize()
{
    QString strLog = "";
    int nCode = 0;
    ushort AFValueH = ( nCode & 0xFFF0) >> 4;
    ushort AFValueL = ( nCode & 0x0F) << 4;
    ushort regAddrs[] = {0xEC, 0xA1, 0xF2, 0xDC, AFValueH};
    ushort regVals[]  = {0xA3, 0x0D, 0xF8, 0x51, (ushort)(AFValueL | (ushort)LSC_CTL_TWO_CODE_PER_STEP)};
    uint regNum = sizeof(regAddrs)/sizeof(regAddrs[0]);
    if (ERR_NoError != m_channelController->WriteDiscreteI2c(m_i2cAddr, 100, RB_ADDR8_DATA8, regAddrs, regVals, regNum))
    {
        strLog.sprintf("VcmDriver_DW9714::%s---WriteDiscreteI2c Error\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        return ERR_Failed;
    }

    return ERR_NoError;
}

/*int VcmDriver_DW9714::Standby()
{
	sensorId = SlaveID;
	int bFlag = 0;
	bFlag = I2cWrite(sensorId, 0x80, 0x00, 0);
	Sleep(50);
	if (bFlag < 1)
	{
		return -1;
	}
	return 0;
}*/

int VcmDriver_DW9714::VcmReadCode(uint &value)
{
#if 0   //Normal模式下读操作，目前的I2C驱动发不出满足要求的波形，因为Normal模式下的读操作的波形时序图为【起始 + 芯片地址(读) + 数据1 + 数据2 + 停止】，没有【起始 + 芯片地址(写) + 寄存器地址】这个前缀时序----20180411
    ushort regAddrs[] = {0x03, 0x04};
    ushort regVals[] = {0x00, 0x00};
    uint regNum = sizeof(regAddrs)/sizeof(regAddrs[0]);
    if (ERR_NoError != m_channelController->ReadDiscreteI2c(m_i2cAddr, 100, RB_ADDR8_DATA8, regAddrs, regVals, regNum))
    {
        return ERR_Failed;
    }

    value = ((regVals[0] & 0x03) << 8) + regVals[1];
#else
    (void)value;
#endif
    return ERR_NoError;
}

/**
  * 注意：本驱动代码也是使用的Noraml模式的I2C时序图波形来操作的。
  * ----20180411
**/
int VcmDriver_DW9714::VcmWriteCode(uint value)
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

/*int VcmDriver_DW9714::AutoFocus()
{	
	return 0;
}*/
