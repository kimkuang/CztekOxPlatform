#ifndef __EFLASH_DW9763_H__
#define __EFLASH_DW9763_H__
#include "OtpSensor.h"

class EFLASH_DW9763 : public OtpSensor
{
public:
    EFLASH_DW9763(uint i2cAddr);
    ~EFLASH_DW9763(void);

    int OtpRead(uint startAddr, uint endAddr, uchar *buf, ushort page = 0);
    int OtpWrite(uint startAddr, uint endAddr, const uchar *buf, ushort page = 0);

private:
    enum {
        DW9763_START_ADDR = 0x0000,
        DW9763_END_ADDR = 0x1FFF,
        DW9763_BYTE_PER_PAGE = 1024,
        DW9763_FLASH_SIZE = DW9763_END_ADDR - DW9763_START_ADDR + 1,
        DW9763_FLASH_MAX_PAGE = DW9763_FLASH_SIZE / DW9763_BYTE_PER_PAGE,
        DW9763_ERASE_ALL_TIMEOUT = 10,  /* Minimal Value, Unit: ms */
    };

    /*
     * DW9763 use WRTEN pin to enable write protection
     * No MPK bit in Control Register like DW9767
    */
    int enableWriteProtection(bool enable);
    /*
     * DW9763 use macro erase(erase 8K) function
     * No page erase(erase 1K) function like DW9767
    */
    int eraseWholeFlash(void);
    int checkFlashIdle(uint timeout_ms);
    int readFlashData(uint startAddr, uchar *buf, uint bufferLen);
    int writeWholeFlash(const uchar *buf);

    /* Software Data protection: Memory protection code */
    int memoryProtection(bool off);

    /*
     * Unit test case functions
    */
    int unitTest();
    int testWriteAllPages();
    int testWriteRandom();

};
#endif

