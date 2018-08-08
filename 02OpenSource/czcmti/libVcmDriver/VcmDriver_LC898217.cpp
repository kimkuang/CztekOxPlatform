
#define VCM_DRV_LC898217

#include "VcmDriver_LC898217.h"

VcmDriver_LC898217::VcmDriver_LC898217(uint i2cAddr) :
    VcmDriver(i2cAddr)
{
}

int VcmDriver_LC898217::VcmInitialize()
{
    QString strLog = "";

    int cnt = 0;
    ushort regAddrs[] = {0xF0};
    ushort regVals[] = {0x00};
    uint regNum = sizeof(regAddrs)/sizeof(regAddrs[0]);
    if (ERR_NoError != m_channelController->ReadDiscreteI2c(m_i2cAddr, 100, RB_ADDR8_DATA8, regAddrs, regVals, regNum))
    {
        strLog.sprintf("VcmDriver_LC898217::%s Error---111\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, LogRed);
        return ERR_Failed;
    }

    if (regVals[0] != 0x72)
    {
        strLog.sprintf("VcmDriver_LC898217::%s Error---Chip ID:0x%X\r\n", __FUNCTION__, regVals[0]);
        m_channelController->LogToWindow(strLog, LogRed);
        return ERR_Failed;
    }

    regAddrs[0] = 0xE0;
    regVals[0] = 0x01;

    if(ERR_NoError != m_channelController->WriteDiscreteI2c(m_i2cAddr, 100, RB_ADDR8_DATA8, regAddrs, regVals, regNum))
    {
        strLog.sprintf("VcmDriver_LC898217::%s Error---Download FW fail\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, LogRed);
        return ERR_Failed;
    }

    regAddrs[0] = 0xE0;
    regVals[0] = 0xFF;

    do
    {
        QThread::msleep(2);
        if (ERR_NoError != m_channelController->ReadDiscreteI2c(m_i2cAddr, 100, RB_ADDR8_DATA8, regAddrs, regVals, regNum))
        {
            strLog.sprintf("VcmDriver_LC898217::%s Error---Read Reg[0xE0] Fail\r\n", __FUNCTION__);
            m_channelController->LogToWindow(strLog, LogRed);
            return ERR_Failed;
        }
        cnt ++;
    }while((regVals[0] != 0) && (cnt < 50));

    if (cnt >= 50)
    {
        strLog.sprintf("VcmDriver_LC898217::%s Error---Onsemi area check NG\r\n", __FUNCTION__);
        m_channelController->LogToWindow(strLog, LogRed);
        return ERR_Failed;
    }

    return ERR_NoError;
}

int VcmDriver_LC898217::VcmReadCode(uint &value)
{
    ushort regAddrs[] = {0x84};
    ushort regVals[] = {0x00};
    uint regNum = sizeof(regAddrs)/sizeof(regAddrs[0]);
    if (ERR_NoError != m_channelController->ReadDiscreteI2c(m_i2cAddr, 100, RB_ADDR8_DATA16, regAddrs, regVals, regNum))
    {
        return ERR_Failed;
    }

    value = (int)(regVals[0] & 0x0FFF);
    return ERR_NoError;
}

int VcmDriver_LC898217::VcmWriteCode(uint value)
{
    QString strLog = "";

    if (value > 1023)
    {
        value = 1023;
    }

    uint slaveAddr = m_i2cAddr;
    uint speedkHz = 100;
    /*uint mode = RB_ADDR8_DATA8;
    ushort regAddrs[] = {0xE0};
    ushort regVals[] = {0x01};
    uint regNum = sizeof(regAddrs)/sizeof(regAddrs[0]);
    int ec = m_channelController->WriteDiscreteI2c(slaveAddr, speedkHz, mode, regAddrs, regVals, regNum);
    if (ERR_NoError != ec)
    {
        return ec;
    }*/

    uint mode = RB_ADDR8_DATA16;
    ushort regAddrs[] = {0x84};
    ushort regVals[] = {(ushort)((value & 0x0FFFu)/* | (VCM_CODE_BIT10 << 8)*/)};
    uint regNum = sizeof(regAddrs)/sizeof(regAddrs[0]);
    int ec = m_channelController->WriteDiscreteI2c(slaveAddr, speedkHz, mode, regAddrs, regVals, regNum);
    if (ERR_NoError != ec)
    {
        strLog.sprintf("VcmDriver_LC898217::%s Error---ec:%d\r\n", __FUNCTION__, ec);
        m_channelController->LogToWindow(strLog, LogRed);
        return ec;
    }

    return ERR_NoError;
}
