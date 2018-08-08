#include "VcmDriver_DW9767.h"

VcmDriver_DW9767::VcmDriver_DW9767(uint i2cAddr) :
    VcmDriver(i2cAddr)
{
}

int VcmDriver_DW9767::VcmInitialize()
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

    ushort regAddrs2[] = {0x06, 0x07, 0x08};
    ushort regVals2[] = {0x09, 0x01, 0x3D};
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

int VcmDriver_DW9767::VcmReadCode(uint &value)
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

int VcmDriver_DW9767::VcmWriteCode(uint value)
{
    if (value > 1023)
    {
        value = 1023;
    }

    ushort regAddrs[] = {0x03, 0x04};
    ushort regVals[] = {ushort(value>>8), ushort(value&0xff)};
    uint regNum = sizeof(regAddrs)/sizeof(regAddrs[0]);
    if (ERR_NoError == m_channelController->WriteDiscreteI2c(m_i2cAddr, 100, RB_ADDR8_DATA8, regAddrs, regVals, regNum))
    {
        return ERR_NoError;
    }

    return ERR_Failed;
}

/*int VcmDriver_DW9767::AutoFocus()
{	
	return 0;
}*/
