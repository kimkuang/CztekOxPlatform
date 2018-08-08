#include "EEPROM_BL24SA64.h"

EEPROM_BL24SA64::EEPROM_BL24SA64(uint i2cAddr)
            : EEPROM_Normal(i2cAddr)
{
    m_uI2cMode = /*3*/RB_ADDR16_DATA8;
    m_uPageLen = 32;
    m_uEepromSize = (64 / 8) * 1024;
}

EEPROM_BL24SA64::~EEPROM_BL24SA64(void)
{

}

int EEPROM_BL24SA64::OtpRead(uint startAddr, uint endAddr, uchar *buf, ushort page)
{
    return EEPROM_Normal::OtpRead(startAddr, endAddr, buf, page);
}

int EEPROM_BL24SA64::OtpWrite(uint startAddr, uint endAddr, const uchar *buf, ushort page)
{
    int ret;
    BYTE protectvalue = 0;
    uchar ubTmp;
    m_channelController->ReadContinuousI2c(m_i2cAddr, 400, 0x8000, 2, &protectvalue, 1);
    if(protectvalue != 0x00)
    {
        ubTmp = 0x00;
        m_channelController->WriteContinuousI2c(m_i2cAddr, 400, 0x8000, m_uI2cMode, &ubTmp, sizeof(ubTmp));
        QThread::msleep(5);
    }

    ret = EEPROM_Normal::OtpWrite(startAddr, endAddr, buf, page);

    m_channelController->ReadContinuousI2c(m_i2cAddr, 400, 0x8000, 2, &protectvalue, 1);

    if(protectvalue != 0x0E)
    {
        ubTmp = 0x0E;
        m_channelController->WriteContinuousI2c(m_i2cAddr, 400, 0x8000, m_uI2cMode, &ubTmp, sizeof(ubTmp));
    }

    return ret;
}
