#include "EEPROM_GT24C16.h"

EEPROM_GT24C16::EEPROM_GT24C16(uint i2cAddr)
            : EEPROM_Normal(i2cAddr)
{
    m_uI2cMode = /*0*/RB_NORMAL;
    m_uPageLen = 16;
    m_uEepromSize = (16 / 8) * 1024;
}

EEPROM_GT24C16::~EEPROM_GT24C16(void)
{

}

int EEPROM_GT24C16::OtpRead(uint startAddr, uint endAddr, uchar *buf, ushort page)
{
    return EEPROM_Normal::OtpRead(startAddr, endAddr, buf, page);
}

int EEPROM_GT24C16::OtpWrite(uint startAddr, uint endAddr, const uchar *buf, ushort page)
{
    return EEPROM_Normal::OtpWrite(startAddr, endAddr, buf, page);
}

