#ifndef __EEPROM_GT24P128C_H__
#define __EEPROM_GT24P128C_H__
#include "OtpSensor.h"
#include "EEPROM_Normal.h"

#ifdef EEPROM_DRV_GT24P128C
#define WP_REG_ADDR                                                     0x8000
#endif

class EEPROM_GT24P128C : public EEPROM_Normal
{
public:
    EEPROM_GT24P128C(uint i2cAddr);
    ~EEPROM_GT24P128C(void);

    int OtpRead(uint startAddr, uint endAddr, uchar *buf, ushort page = 0);
    int OtpWrite(uint startAddr, uint endAddr, const uchar *buf, ushort page = 0);
};
#endif
