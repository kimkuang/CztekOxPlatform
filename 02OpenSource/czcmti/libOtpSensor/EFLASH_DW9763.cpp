#include "EFLASH_DW9763.h"
#include <QTime>

#define ENABLE_UNIT_TEST    0

EFLASH_DW9763::EFLASH_DW9763(uint i2cAddr)
            : OtpSensor(i2cAddr)
{

}

EFLASH_DW9763::~EFLASH_DW9763(void)
{

}

int EFLASH_DW9763::OtpRead(uint startAddr, uint endAddr, uchar *buf, ushort page)
{
    (void)page;
#if ENABLE_UNIT_TEST > 0
    if (page >= DW9763_FLASH_MAX_PAGE)
        unitTest();
#endif

    if ((startAddr >= DW9763_FLASH_SIZE) || \
        (endAddr >= DW9763_FLASH_SIZE)   || \
        (startAddr > endAddr)            || \
        (buf == nullptr))
    {
        QString strLog = QString("Input startAddr[%1] or endAddr[%2] or buffer is null").arg(startAddr).arg(endAddr);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ERR_InvalidParameter;
    }

    int ec = readFlashData(startAddr, buf, endAddr - startAddr + 1);
    if (ec < 0) {
        QString strLog = QString("Call readFlashData() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    return ec;
}

int EFLASH_DW9763::OtpWrite(uint startAddr, uint endAddr, const uchar *buf, ushort page)
{
    (void)page;
    if ((startAddr >= DW9763_FLASH_SIZE) || \
        (endAddr >= DW9763_FLASH_SIZE)   || \
        (startAddr > endAddr)            || \
        (buf == nullptr))
    {
        QString strLog = QString("Input startAddr[%1] or endAddr[%2] or buffer if null").arg(startAddr).arg(endAddr);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ERR_InvalidParameter;
    }

    //Step 1: Read the whole flash to flashMap memory
    int ec = ERR_NoError;
    uchar flashMap[DW9763_FLASH_SIZE];

    ec = readFlashData(DW9763_START_ADDR, flashMap, DW9763_FLASH_SIZE);
    if (ec < 0) {
        QString strLog = QString("Call readFlashData() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    //Step 2: Copy data writen to flashMap memory
    memcpy(flashMap + startAddr, buf, endAddr - startAddr + 1);

    //Step 3: Disable write protection
    ec = enableWriteProtection(false);
    if (ec < 0) {
        QString strLog = QString("Call enableWriteProtection(false) failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    //Step 4: Memory Protection Off
    ec = memoryProtection(true);
    if (ec < 0) {
        QString strLog = QString("Memory Protection Off failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    //Step 5. Erase whole flash
    ec = eraseWholeFlash();
    if (ec < 0) {
        qCritical("Call eraseWholeFlash failed[%d]", ec);
        return ec;
    }

    //Step 6: Write flashMap memory back to the flash
    ec = writeWholeFlash(flashMap);
    if(ec < 0)
    {
        qCritical() << "writeWholeFlash failed";
        return ec;
    }

    //Step 7: Memory Protection On
    ec = memoryProtection(false);
    if (ec < 0) {
        QString strLog = QString("Memory Protection On failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    //Step 8: Enable write protection
    ec = enableWriteProtection(true);
    if (ec < 0) {
        QString strLog = QString("Call enableWriteProtection(true) failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }
    return ec;
}

/*
 * DW9763 use WRTEN pin to enable write protection
 * No MPK bit in Control Register like DW9767
*/
int EFLASH_DW9763::enableWriteProtection(bool enable)
{
    /*
     * Hardware Data Protection: WRTEN control
     * WRTEN = “L” (read only)
     * WRTEN = “H” (read, write, erase)
    */
    if(enable == true)
    {
        /* WRTEN = “L” (read only) */
    }
    else
    {
        /* WRTEN = “H” (read, write, erase) */
    }

    return ERR_NoError;
}

/*
 * DW9763 use macro erase(erase 8K) function
 * No page erase(erase 1K) function like DW9767
*/
int EFLASH_DW9763::eraseWholeFlash(void)
{
    uint checkIdletimeout_ms = DW9763_ERASE_ALL_TIMEOUT;
    ushort eraseCmd = 0x81;

    int ec = I2cWrite(eraseCmd, 0xEE, RB_ADDR8_DATA8);
    if (ec < 0) {
        qCritical("Call I2cWrite() failed[%d]", ec);
        return ec;
    }
    ec = checkFlashIdle(checkIdletimeout_ms);
    if (ec < 0) {
        qCritical("Call checkFlashIdle() failed[%d]", ec);
        return ec;
    }

    return ec;
}

/* Software Data protection: Memory protection code */
int EFLASH_DW9763::memoryProtection(bool off)
{
    uint vcmI2cAddr = m_channelContext->ModuleSettings->ReadInteger("vcmDriverDb", "i2cAddr", 0x18);
    ushort reg[2], val[2];
    uint flashI2cAddr = m_i2cAddr;
    m_i2cAddr = vcmI2cAddr;

    if(off)
    {
        reg[0] = 0xF9;
        reg[1] = 0xA0;
        val[0] = 0x7F;
        val[1] = 0x04;
    }
    else
    {
        reg[0] = 0xA0;
        reg[1] = 0xED;
        val[0] = 0x00;
        val[1] = 0x9D;
    }

    if(ERR_NoError != I2cWrite(reg[0], val[0], RB_ADDR8_DATA8))
    {
        qCritical("[memoryProtection] I2cWrite error");
        return ERR_Failed;
    }

    if(ERR_NoError != I2cWrite(reg[1], val[1], RB_ADDR8_DATA8))
    {
        qCritical("[memoryProtection] I2cWrite error");
        return ERR_Failed;
    }

    m_i2cAddr = flashI2cAddr;

    return ERR_NoError;
}

int EFLASH_DW9763::checkFlashIdle(uint timeout_ms)
{
    uint vcmI2cAddr = m_channelContext->ModuleSettings->ReadInteger("vcmDriverDb", "i2cAddr", 0x18);
    uint flashI2cAddr = m_i2cAddr;
    m_i2cAddr = vcmI2cAddr;
    bool checkFailed = true;
    for (uint i = 0; i < 10; i++) {
        QThread::msleep(timeout_ms);
        ushort status = 0xff;
        if (ERR_NoError == I2cRead(0x05, status, RB_ADDR8_DATA8)) {
            if ((status & 0x02) == 0x00) {
                checkFailed = false;
                break;
            }
        }
    }
    m_i2cAddr = flashI2cAddr;
    if (checkFailed) {
        qCritical("Check flash idle timeout");
        return ERR_Failed;
    }

    return ERR_NoError;
}

int EFLASH_DW9763::readFlashData(uint startAddr, uchar *buf, uint bufferLen)
{
    int ec = ERR_NoError;
    uint packetCnt = (bufferLen + I2C_RW_MAX_BYTES - 1) / I2C_RW_MAX_BYTES;
    uint offset = 0;
    for (uint i = 0; i < packetCnt; i++, offset += I2C_RW_MAX_BYTES) {
        ushort len = qMin((uint)I2C_RW_MAX_BYTES, bufferLen - offset);
        ec = I2cReadBlock(startAddr + offset, 2, buf + offset, len);
        if (ec < 0) {
            qCritical("Call I2cReadBlock() faile[%d], addr[0x%x], len = %d",
                        ec, startAddr + offset, len);
            return ec;
        }
    }

    return ec;
}

int EFLASH_DW9763::writeWholeFlash(const uchar *buf)
{
    int retryCnt = 2;

    do
    {
        int ec;
        uint offset = 0;
        ushort startAddr = DW9763_START_ADDR;
        uint packetCnt = (DW9763_FLASH_SIZE + I2C_RW_MAX_BYTES - 1) / I2C_RW_MAX_BYTES;

        // 1. Write data to flash
        for (uint i = 0; i < packetCnt; i++, offset += I2C_RW_MAX_BYTES) {
            ushort len = qMin((uint)I2C_RW_MAX_BYTES, DW9763_FLASH_SIZE - offset);
            ec = I2cWriteBlock(startAddr + offset, 2, buf + offset, len);
            if (ec < 0) {
                qCritical("Call I2cWriteBlock() failed[%d], addr[0x%x], len = %d",
                            ec, startAddr + offset, len);
                return ec;
            }
            ec = checkFlashIdle(DW9763_ERASE_ALL_TIMEOUT);
            if (ec < 0) {
                qCritical("Call checkFlashIdle() failed[%d]", ec);
                return ec;
            }
        }

        // 2. Read data from flash
        uchar readBuf[DW9763_FLASH_SIZE];
        memset(readBuf, 0xA5, sizeof(readBuf));
        ec = readFlashData(startAddr, readBuf, DW9763_FLASH_SIZE);
        if (ec < 0) {
            qCritical("Call readFlashData() failed[%d]", ec);
            return ec;
        }

        // 3. Verify
        if (memcmp(readBuf, buf, DW9763_FLASH_SIZE) == 0)
            return ERR_NoError;

    } while(--retryCnt > 0);

    qCritical("Verify Data Failed");
    return ERR_Failed;
}

int EFLASH_DW9763::unitTest()
{
    qDebug("i2cAddr: 0x%02x", m_i2cAddr);
    int ec = ERR_NoError;
    uint errCnt = 0;

    qInfo("Test write all page, result = %d", ec);
    ec = testWriteAllPages();
    if (ec < 0) {
        errCnt++;
        qCritical("Call testWriteAllPages() failed");
    }

    qInfo("Test write radom");
    for (uint i = 0; i < 100; i++) {
        ec = testWriteRandom();
        if (ec < 0) {
            errCnt++;
            qCritical("Call testWriteRadom() failed the idx = %d", i);
        }
    }
    QString strLog = QString("DW9763 FLASH TEST ERROR COUNT = %1").arg(errCnt);
    m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
    qInfo() << strLog;
    return ec;
}

int EFLASH_DW9763::testWriteAllPages()
{
    int ec = ERR_NoError;
    uchar writeBuf[DW9763_FLASH_SIZE];
    uchar rdBuf[DW9763_FLASH_SIZE];

    uint startAddr = 0;
    uint endAddr = DW9763_FLASH_SIZE - 1;
    memset(writeBuf, 0x5A, sizeof(writeBuf));
    ec = OtpWrite(startAddr, endAddr, writeBuf);
    if (ec < 0) {
        QString strLog = QString("Call OtpWrite() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    memset(rdBuf, 0x6B, sizeof(rdBuf));
    ec = OtpRead(startAddr, endAddr, rdBuf);
    if (ec < 0) {
        QString strLog = QString("Call OtpRead() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    uint checkErrCnt = 0;
    for (uint i = 0; i < endAddr - startAddr + 1; i++) {
        if (rdBuf[i] != writeBuf[i]) {
            checkErrCnt++;
            qCritical("addr[0x%x]:wr=0x%x, rd=0x%x", i + startAddr, writeBuf[i], rdBuf[i]);
        }
    }

    return ec;
}

int EFLASH_DW9763::testWriteRandom()
{
    qsrand(QTime::currentTime().msecsSinceStartOfDay());
    int ec = ERR_NoError;
    uchar writeBuf[DW9763_FLASH_SIZE];
    uchar rdBuf[DW9763_FLASH_SIZE];

    uint startAddr = qrand() % DW9763_FLASH_SIZE;
    uint endAddr = qrand() % DW9763_FLASH_SIZE;
    if (startAddr > endAddr) {
        uint temp = startAddr;
        startAddr = endAddr;
        endAddr = temp;
    }
    qInfo("startAddr = 0x%x, endAddr = 0x%x, len = %d", startAddr, endAddr, endAddr - startAddr + 1);

    for (uint i = 0; i < endAddr - startAddr + 1; i++) {
        writeBuf[i] = (uchar)(qrand() % 256);
    }

    ec = OtpWrite(startAddr, endAddr, writeBuf);
    if (ec < 0) {
        QString strLog = QString("Call OtpWrite() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    memset(rdBuf, 0xff, sizeof(rdBuf));
    ec = OtpRead(startAddr, endAddr, rdBuf);
    if (ec < 0) {
        QString strLog = QString("Call OtpRead() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    uint checkErrCnt = 0;
    for (uint i = 0; i < endAddr - startAddr + 1; i++) {
        if (rdBuf[i] != writeBuf[i]) {
            checkErrCnt++;
            ec = ERR_Failed;
            qCritical("addr[0x%x]:wr=0x%x, rd=0x%x", i + startAddr, writeBuf[i], rdBuf[i]);
        }
    }

    return ec;
}

