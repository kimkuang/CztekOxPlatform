#include "EEPROM_GT24C128.h"

EEPROM_GT24C128::EEPROM_GT24C128(uint i2cAddr)
            : EEPROM_Normal(i2cAddr)
{
    m_uI2cMode = /*3*/RB_ADDR16_DATA8;
    m_uPageLen = 64;
    m_uEepromSize = (128 / 8) * 1024;
}

EEPROM_GT24C128::~EEPROM_GT24C128(void)
{

}

int EEPROM_GT24C128::OtpRead(uint startAddr, uint endAddr, uchar *buf, ushort page)
{
    return EEPROM_Normal::OtpRead(startAddr, endAddr, buf, page);
}

int EEPROM_GT24C128::OtpWrite(uint startAddr, uint endAddr, const uchar *buf, ushort page)
{
    return EEPROM_Normal::OtpWrite(startAddr, endAddr, buf, page);
}
