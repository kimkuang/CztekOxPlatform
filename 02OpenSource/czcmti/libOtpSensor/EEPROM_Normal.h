#ifndef __EEPROM_Normal_H__
#define __EEPROM_Normal_H__
#include "OtpSensor.h"

class EEPROM_Normal : public OtpSensor
{
    enum {
        I2C_RW_LEN_MAX = 1024
    };

public:
    EEPROM_Normal(uint i2cAddr);
    ~EEPROM_Normal(void);

    virtual int OtpRead(uint startAddr, uint endAddr, uchar *buf, ushort page = 0);
    virtual int OtpWrite(uint startAddr, uint endAddr, const uchar *buf, ushort page = 0);

private:
    int roundUp(int x, int y);
protected:
    uint m_uI2cMode;
    uint m_uPageLen;
    uint m_uEepromSize;
};

#endif

