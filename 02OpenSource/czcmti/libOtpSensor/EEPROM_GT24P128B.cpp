
#include "EEPROM_GT24P128B.h"

EEPROM_GT24P128B::EEPROM_GT24P128B(uint i2cAddr)
            : EEPROM_Normal(i2cAddr)
{
    m_uI2cMode = RB_ADDR16_DATA8;
    m_uPageLen = 64;
    m_uEepromSize = (128 / 8) * 1024;
}

EEPROM_GT24P128B::~EEPROM_GT24P128B(void)
{

}

int EEPROM_GT24P128B::OtpRead(uint startAddr, uint endAddr, uchar *buf, ushort page)
{
    return EEPROM_Normal::OtpRead(startAddr, endAddr, buf, page);
}

int EEPROM_GT24P128B::OtpWrite(uint startAddr, uint endAddr, const uchar *buf, ushort page)
{
    uchar ubTmp = 0x00;
#if 1
    /**
      * 取消写保护的操作是只发出地址字节，但是总线上没有0x80的地址，所以主机将接受不到从机的ACK信号，故I2C驱动会报错，所以此处不能判断错误。
      * 日志里面在此处会有一条错误信息，是正常的，不用理会。
      * --Added by LiHai--20180410
    **/
    m_channelController->WriteContinuousI2c(0x80, 400, 0x00, 1, &ubTmp, 1);
    QThread::msleep(5);
#else
    if (ERR_NoError != m_channelController->WriteContinuousI2c(0x80, 400, 0x00, 1, &ubTmp, 1))//关闭写保护
    {
        return ERR_Failed;
    }
#endif
    return EEPROM_Normal::OtpWrite(startAddr, endAddr, buf, page);
}

