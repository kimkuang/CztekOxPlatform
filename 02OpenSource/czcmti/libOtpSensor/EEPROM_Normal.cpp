#include "EEPROM_Normal.h"

EEPROM_Normal::EEPROM_Normal(uint i2cAddr)
            : OtpSensor(i2cAddr)
{
    m_uI2cMode = 3;
    m_uPageLen = 32;
    m_uEepromSize = 1024 * 8;
}

EEPROM_Normal::~EEPROM_Normal(void)
{

}

int EEPROM_Normal::OtpRead(uint startAddr, uint endAddr, uchar *buf, ushort page)
{
    (void)page;
    uint u32Len = endAddr - startAddr + 1;
    if (u32Len > m_uEepromSize)
    {
        qCritical("startAddr[%d] + count[%d] is out of memory[%d]!\n", startAddr, u32Len, m_uEepromSize);
        return ERR_InvalidParameter;
    }


    int iRet = ERR_NoError;
    uint uRegAddrSize = 1;
    if ((/*3*/RB_ADDR16_DATA8 == m_uI2cMode) || (/*4*/RB_ADDR16_DATA16 == m_uI2cMode))
    {
        uRegAddrSize = 2;
    }

    ushort uGroupSize = I2C_RW_LEN_MAX;
    for (ushort uAddr = startAddr; uAddr <= endAddr; uAddr += I2C_RW_LEN_MAX)
    {
        if ((endAddr - uAddr + 1) < I2C_RW_LEN_MAX)
        {
            uGroupSize = endAddr - uAddr + 1;
        }

        iRet = I2cReadBlock(uAddr, uRegAddrSize, buf + (uAddr - startAddr), uGroupSize);
        if (ERR_NoError != iRet)
        {
            return iRet;
        }
    }

    return ERR_NoError;
}

int EEPROM_Normal::OtpWrite(uint startAddr, uint endAddr, const uchar *buf, ushort page)
{
    (void)page;
    uint u32Len = endAddr - startAddr + 1;
    if (u32Len > m_uEepromSize)
    {
        qCritical("startAddr[%d] + count[%d] is out of memory[%d]!\n", startAddr, u32Len, m_uEepromSize);
        return ERR_InvalidParameter;
    }


    int iRet = ERR_NoError;
    uint uRegAddrSize = 1;
    if ((/*3*/RB_ADDR16_DATA8 == m_uI2cMode) || (/*4*/RB_ADDR16_DATA16 == m_uI2cMode))
    {
        uRegAddrSize = 2;
    }

    uint uFirstGroupSize = 0;
    uint uNextPageStartAddr = roundUp(startAddr + 1, m_uPageLen);
    if ((startAddr + u32Len) > uNextPageStartAddr)  //待写入的数据会超过本Page范围
    {
        uFirstGroupSize = uNextPageStartAddr - startAddr;   //本Page需要写入的字节数
    }

    if (0 != uFirstGroupSize)
    {
        if (ERR_NoError != (iRet = I2cWriteBlock(startAddr, uRegAddrSize, buf, uFirstGroupSize)))
        {
            return iRet;
        }

        startAddr += uFirstGroupSize;
    }


    uint uTempLen = m_uPageLen;
    for (ushort uAddr = startAddr; uAddr <= endAddr; uAddr += m_uPageLen)
    {
        if ((endAddr - uAddr + 1) < m_uPageLen)
        {
            uTempLen = endAddr - uAddr + 1;
        }

        iRet = I2cWriteBlock(uAddr, uRegAddrSize, buf + (uAddr - startAddr + uFirstGroupSize), uTempLen);
        if (ERR_NoError != iRet)
        {
            break;
        }
    }

    return iRet;
}

int EEPROM_Normal::roundUp(int x, int y)    //计算下一个Page的起始地址
{
    return (x + (y - 1)) / y * y;
}
