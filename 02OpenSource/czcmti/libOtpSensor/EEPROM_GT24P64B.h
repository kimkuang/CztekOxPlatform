#ifndef __EEPROM_GT24P64B_H__
#define __EEPROM_GT24P64B_H__
#include "OtpSensor.h"
#include "EEPROM_Normal.h"

#ifdef EEPROM_DRV_GT24P64B
#define WP_REG_ADDR                                                     0x8000
#define WP_EN                                                           (0x01 << 3)

#define WP_1_P_4_OF_MEM                                                 (0x00 << 1)
#define WP_2_P_4_OF_MEM                                                 (0x01 << 1)
#define WP_3_P_4_OF_MEM                                                 (0x02 << 1)
#define WP_4_P_4_OF_MEM                                                 (0x03 << 1)

#define WP_UNLOCK                                                       (0x00 << 0)
#define WP_LOCK                                                         (0x01 << 0)
#endif

class EEPROM_GT24P64B : public EEPROM_Normal
{
public:
    EEPROM_GT24P64B(uint i2cAddr);
    ~EEPROM_GT24P64B(void);

    int OtpRead(uint startAddr, uint endAddr, uchar *buf, ushort page = 0);
    int OtpWrite(uint startAddr, uint endAddr, const uchar *buf, ushort page = 0);
};
#endif
