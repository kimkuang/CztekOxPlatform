#ifndef __EEPROM_GT24C16_H__
#define __EEPROM_GT24C16_H__
#include "OtpSensor.h"
#include "EEPROM_Normal.h"

class EEPROM_GT24C16 : public EEPROM_Normal
{
public:
    EEPROM_GT24C16(uint i2cAddr);
    ~EEPROM_GT24C16(void);

    int OtpRead(uint startAddr, uint endAddr, uchar *buf, ushort page = 0);
    int OtpWrite(uint startAddr, uint endAddr, const uchar *buf, ushort page = 0);
};

#endif
