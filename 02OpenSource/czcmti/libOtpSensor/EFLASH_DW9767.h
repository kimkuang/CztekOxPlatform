#ifndef __EFLASH_DW9767_H__
#define __EFLASH_DW9767_H__
#include "OtpSensor.h"

class EFLASH_DW9767 : public OtpSensor
{
public:
    EFLASH_DW9767(uint i2cAddr);
    ~EFLASH_DW9767(void);

    int OtpRead(uint startAddr, uint endAddr, uchar *buf, ushort page = 0);
    int OtpWrite(uint startAddr, uint endAddr, const uchar *buf, ushort page = 0);
    int OtpUnitTest();

private:
    enum {
        DW9767_BYTE_PER_PAGE  = 1024,
        DW9767_FLASH_SIZE = 8 * 1024,
        DW9767_FLASH_MAX_PAGE = DW9767_FLASH_SIZE / DW9767_BYTE_PER_PAGE,
        DW9767_ERASE_ALL_TIMEOUT = 10,
        DW9767_ERASE_PAGE_TIMEOUT = 2
    };

    int enableWriteProtection(bool enable);
    int eraseFlash(uint pageNo);
    int checkFlashIdle(uint timeout_ms);
    int readFlashData(uint startAddr, uchar *buf, uint bufferLen);
    int writeFlashOnePage(uint pageNo, const uchar *buf);

    int unitTest();
    int testWriteOnePage(uint pageNo);
    int testWriteAllPages();
    int testWriteRandom();

};

#endif
