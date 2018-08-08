#include "EFLASH_DW9767.h"
#include <QTime>

#define ENABLE_UNIT_TEST    0

EFLASH_DW9767::EFLASH_DW9767(uint i2cAddr)
            : OtpSensor(i2cAddr)
{

}

EFLASH_DW9767::~EFLASH_DW9767(void)
{

}

int EFLASH_DW9767::OtpRead(uint startAddr, uint endAddr, uchar *buf, ushort page)
{
    (void)page;
#if ENABLE_UNIT_TEST > 0
    if (page >= DW9767_FLASH_MAX_PAGE)
        unitTest();
#endif

    if (startAddr >= DW9767_FLASH_SIZE ||
            endAddr >= DW9767_FLASH_SIZE ||
            endAddr < startAddr ||
            nullptr == buf) {
        QString strLog = QString("Input startAddr[%1] or endAddr[%2]").arg(startAddr).arg(endAddr);
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

int EFLASH_DW9767::OtpWrite(uint startAddr, uint endAddr, const uchar *buf, ushort page)
{
    (void)page;
    if (startAddr >= DW9767_FLASH_SIZE ||
            endAddr >= DW9767_FLASH_SIZE ||
            endAddr < startAddr ||
            nullptr == buf) {
        QString strLog = QString("Input startAddr[%1] or endAddr[%2]").arg(startAddr).arg(endAddr);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ERR_InvalidParameter;
    }

    //Step 1:擦除之前先读取数据备份，擦除之后，修改指定内容后写入
    int ec = ERR_NoError;
    uchar flashMap[DW9767_FLASH_SIZE];
    uint startPage = startAddr / DW9767_BYTE_PER_PAGE;
    uint endPage = (endAddr + DW9767_BYTE_PER_PAGE - 1) / DW9767_BYTE_PER_PAGE;
    ec = readFlashData(startPage * DW9767_BYTE_PER_PAGE, flashMap + startPage * DW9767_BYTE_PER_PAGE,
                       (endPage - startPage) * DW9767_BYTE_PER_PAGE);
    if (ec < 0) {
        QString strLog = QString("Call readFlashData() failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    //Step 2:Copy数据
    memcpy(flashMap + startAddr, buf, endAddr - startAddr + 1);

    //Step 3:关闭写保护功能
    ec = enableWriteProtection(false);
    if (ec < 0) {
        QString strLog = QString("Call enableWriteProtection(false) failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }

    //Step 4:按页写入数据
    for (uint page = startPage; page < endPage; page++) {
        ec = writeFlashOnePage(page, flashMap + page * DW9767_BYTE_PER_PAGE);
        if (ec < 0) {
            QString strLog = QString("Call writeFlashOnePage() failed[%1]").arg(ec);
            m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
            qCritical() << strLog;
            enableWriteProtection(true);
            return ec;
        }
    }

    //Step 5:打开写保护功能
    ec = enableWriteProtection(true);
    if (ec < 0) {
        QString strLog = QString("Call enableWriteProtection(true) failed[%1]").arg(ec);
        m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
        qCritical() << strLog;
        return ec;
    }
    return ec;
}

int EFLASH_DW9767::enableWriteProtection(bool enable)
{
    uint vcmI2cAddr = m_channelContext->ModuleSettings->ReadInteger("vcmDriverDb", "i2cAddr", 0x18);
    uint flashI2cAddr = m_i2cAddr;
    m_i2cAddr = vcmI2cAddr;
    ushort cmdData = 0x00;
    int ec = I2cRead(0x02, cmdData, RB_ADDR8_DATA8); // save PD bit
    if (ec < 0) {
        qCritical("Call I2cRead() failed[%d]", ec);
        return ec;
    }
    cmdData |= enable ? 0x00 : 0x10;
    ec = I2cWrite(0x02, cmdData, RB_ADDR8_DATA8);
    m_i2cAddr = flashI2cAddr;
    if (ec < 0) {
        qCritical("Call I2cWrite() failed[%d]", ec);
        return ec;
    }
    QThread::msleep(10);

    return ec;
}

int EFLASH_DW9767::eraseFlash(uint pageNo)
{
    uint checkIdletimeout_ms = 2;
    ushort eraseCmd = 0x82 | (pageNo << 2); // erase one page
    if (pageNo >= DW9767_FLASH_MAX_PAGE) {
        eraseCmd = 0x81; // erase all chip
        checkIdletimeout_ms = 10;
    }
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

int EFLASH_DW9767::checkFlashIdle(uint timeout_ms)
{
    uint vcmI2cAddr = m_channelContext->ModuleSettings->ReadInteger(QString("vcmDriverDb"), QString("i2cAddr"), 0x18);
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

int EFLASH_DW9767::readFlashData(uint startAddr, uchar *buf, uint bufferLen)
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

int EFLASH_DW9767::writeFlashOnePage(uint pageNo, const uchar *buf)
{
    int retryCnt = 3;
    do {
        // 1. erase page
        int ec = eraseFlash(pageNo);
        if (ec < 0) {
            qCritical("Call eraseFlash(pageNo[%d]) failed[%d]", pageNo, ec);
            return ec;
        }

        // 2. write data to flash
        ushort startAddr = pageNo << 10;
        uint packetCnt = (DW9767_BYTE_PER_PAGE  + I2C_RW_MAX_BYTES - 1) / I2C_RW_MAX_BYTES;
        uint offset = 0;
        for (uint i = 0; i < packetCnt; i++, offset += I2C_RW_MAX_BYTES) {
            ec = checkFlashIdle(DW9767_ERASE_PAGE_TIMEOUT);
            if (ec < 0) {
                qCritical("Call checkFlashIdle() failed[%d]", ec);
                return ec;
            }
            ushort len = qMin((uint)I2C_RW_MAX_BYTES, DW9767_BYTE_PER_PAGE - offset);
            ec = I2cWriteBlock(startAddr + offset, 2, buf + offset, len);
            if (ec < 0) {
                qCritical("Call I2cWriteBlock() failed[%d], addr[0x%x], len = %d",
                            ec, startAddr + offset, len);
                return ec;
            }
        }
        ec = checkFlashIdle(DW9767_ERASE_PAGE_TIMEOUT);
        if (ec < 0) {
            qCritical("Call checkFlashIdle() failed[%d]", ec);
            return ec;
        }

        // 3. read back
        uchar readBuf[DW9767_BYTE_PER_PAGE];
        memset(readBuf, 0x5A, sizeof(readBuf));
        ec = readFlashData(startAddr, readBuf, DW9767_BYTE_PER_PAGE);
        if (ec < 0) {
            qCritical("Call readFlashData() failed[%d]", ec);
            return ec;
        }

        // 4. verify
        if (memcmp(readBuf, buf, DW9767_BYTE_PER_PAGE) == 0)
            return ERR_NoError;
    } while(--retryCnt > 0);

    return ERR_Failed;
}

int EFLASH_DW9767::unitTest()
{
    int ec = ERR_NoError;
    uint errCnt = 0;
    for (uint page = 0; page < DW9767_FLASH_MAX_PAGE; page++) {
        ec = testWriteOnePage(page);
        if (ec < 0) {
            errCnt++;
            qCritical("Call testWriteOnePage(%d) failed", page);
        }
        qInfo("Test write %d page, result = %d", page, ec);
    }

    ec = testWriteAllPages();
    if (ec < 0) {
        errCnt++;
        qCritical("Call testWriteAllPages() failed");
    }
    qInfo("Test write all page, result = %d", ec);

    qInfo("Test write radom");
    for (uint i = 0; i < 100; i++) {
        ec = testWriteRandom();
        if (ec < 0) {
            errCnt++;
            qCritical("Call testWriteRadom() failed the idx = %d", i);
        }
    }
    QString strLog = QString("DW9767 FLASH TEST ERROR COUNT = %1").arg(errCnt);
    m_channelController->LogToWindow(strLog, qRgb(255, 0, 0));
    qInfo() << strLog;
    return ec;
}

int EFLASH_DW9767::testWriteOnePage(uint pageNo)
{
    int ec = ERR_NoError;
    uchar writeBuf[DW9767_FLASH_SIZE];
    uchar rdBuf[DW9767_FLASH_SIZE];

    uint startAddr = pageNo * DW9767_BYTE_PER_PAGE;
    uint endAddr = startAddr + DW9767_BYTE_PER_PAGE - 1;
    memset(writeBuf, 0, sizeof(writeBuf));
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

int EFLASH_DW9767::testWriteAllPages()
{
    int ec = ERR_NoError;
    uchar writeBuf[DW9767_FLASH_SIZE];
    uchar rdBuf[DW9767_FLASH_SIZE];

    uint startAddr = 0;
    uint endAddr = DW9767_FLASH_SIZE - 1;
    memset(writeBuf, 0x6B, sizeof(writeBuf));
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
            qCritical("addr[0x%x]:wr=0x%x, rd=0x%x", i + startAddr, writeBuf[i], rdBuf[i]);
        }
    }

    return ec;
}

int EFLASH_DW9767::testWriteRandom()
{
    qsrand(QTime::currentTime().msecsSinceStartOfDay());
    int ec = ERR_NoError;
    uchar writeBuf[DW9767_FLASH_SIZE];
    uchar rdBuf[DW9767_FLASH_SIZE];

    uint startAddr = qrand() % DW9767_FLASH_SIZE;
    uint endAddr = qrand() % DW9767_FLASH_SIZE;
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

 
