
#define EEPROM_DRV_GT24P128C

#include "EEPROM_GT24P128C.h"

EEPROM_GT24P128C::EEPROM_GT24P128C(uint i2cAddr)
            : EEPROM_Normal(i2cAddr)
{
    m_uI2cMode = RB_ADDR16_DATA8;
    m_uPageLen = 64;
    m_uEepromSize = (128 / 8) * 1024;
}

EEPROM_GT24P128C::~EEPROM_GT24P128C(void)
{

}

int EEPROM_GT24P128C::OtpRead(uint startAddr, uint endAddr, uchar *buf, ushort page)
{
    return EEPROM_Normal::OtpRead(startAddr, endAddr, buf, page);
}

int EEPROM_GT24P128C::OtpWrite(uint startAddr, uint endAddr, const uchar *buf, ushort page)
{
    uchar ubTmp = 0x00;
#if 1
    ubTmp = 0x00;
    m_channelController->WriteContinuousI2c(m_i2cAddr, 400, WP_REG_ADDR, 2, &ubTmp, sizeof(ubTmp));
    QThread::msleep(5);
#else
    if (ERR_NoError != m_channelController->WriteContinuousI2c(0x80, 400, 0x00, 1, &ubTmp, 1))//关闭写保护
    {
        return ERR_Failed;
    }
#endif
    return EEPROM_Normal::OtpWrite(startAddr, endAddr, buf, page);
}

