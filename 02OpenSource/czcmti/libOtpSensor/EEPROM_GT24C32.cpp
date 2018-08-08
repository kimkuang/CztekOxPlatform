#include "EEPROM_GT24C32.h"

EEPROM_GT24C32::EEPROM_GT24C32(uint i2cAddr)
            : EEPROM_Normal(i2cAddr)
{
    m_uI2cMode = /*3*/RB_ADDR16_DATA8;
    m_uPageLen = 32;
    m_uEepromSize = (32 / 8) * 1024;
}

EEPROM_GT24C32::~EEPROM_GT24C32(void)
{

}

int EEPROM_GT24C32::OtpRead(uint startAddr, uint endAddr, uchar *buf, ushort page)
{
    return EEPROM_Normal::OtpRead(startAddr, endAddr, buf, page);
}

int EEPROM_GT24C32::OtpWrite(uint startAddr, uint endAddr, const uchar *buf, ushort page)
{
    return EEPROM_Normal::OtpWrite(startAddr, endAddr, buf, page);
}
