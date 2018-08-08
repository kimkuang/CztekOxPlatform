#include "EEPROM_GT24C64.h"

EEPROM_GT24C64::EEPROM_GT24C64(uint i2cAddr)
            : EEPROM_Normal(i2cAddr)
{
    m_uI2cMode = /*3*/RB_ADDR16_DATA8;
    m_uPageLen = 32;
    m_uEepromSize = (64 / 8) * 1024;
}

EEPROM_GT24C64::~EEPROM_GT24C64(void)
{

}

int EEPROM_GT24C64::OtpRead(uint startAddr, uint endAddr, uchar *buf, ushort page)
{
    return EEPROM_Normal::OtpRead(startAddr, endAddr, buf, page);
}

int EEPROM_GT24C64::OtpWrite(uint startAddr, uint endAddr, const uchar *buf, ushort page)
{
    return EEPROM_Normal::OtpWrite(startAddr, endAddr, buf, page);
}
