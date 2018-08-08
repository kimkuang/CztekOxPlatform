
#define EEPROM_DRV_GT24P64B

#include "EEPROM_GT24P64B.h"

EEPROM_GT24P64B::EEPROM_GT24P64B(uint i2cAddr)
            : EEPROM_Normal(i2cAddr)
{
    m_uI2cMode = RB_ADDR16_DATA8;
    m_uPageLen = 32;
    m_uEepromSize = (64 / 8) * 1024;
}

EEPROM_GT24P64B::~EEPROM_GT24P64B(void)
{

}

int EEPROM_GT24P64B::OtpRead(uint startAddr, uint endAddr, uchar *buf, ushort page)
{
    return EEPROM_Normal::OtpRead(startAddr, endAddr, buf, page);
}

int EEPROM_GT24P64B::OtpWrite(uint startAddr, uint endAddr, const uchar *buf, ushort page)
{
    uchar ubTmp = 0x00;
#if 1
    /**
      * 取消写保护的操作是给写保护寄存器写入对应的控制字：
      * 1、写保护寄存器地址：0x8000
      * 2、WPRegVal[3]:写保护使能，0：No Write protected, 1:Has Write protected
      *    WPRegVal[2:1]:Block Protect, 00:1/4区域被写保护，01:2/4被写保护，02:3/4被写保护，03:4/4被写保护
      *    WPRegVal[0]:Write Protect Lock, 0:Bit[7:0] can be modified, 1:Bit[7:0] cannot be modified and wp is frozen
      * --Added by LiHai--20180410
    **/
    ubTmp = WP_EN | WP_4_P_4_OF_MEM | WP_UNLOCK;
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

