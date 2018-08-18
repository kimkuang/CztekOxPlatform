#include "FileIo.h"
#include <QFile>
#include <QByteArray>
#include <QDebug>
#include <QImage>

bool FileIo::SaveBmpFile(const QString &fileName, const uchar *pRgbBuf, int width, int height)
{
#if 0
    QImage bmpImg(pRgbBuf, width, height, QImage::Format_RGB888);
    return bmpImg.rgbSwapped().save(fileName);
#else
    BITMAPFILEHDR bitmapFileHdr;
    memset(&bitmapFileHdr, 0, sizeof(BITMAPFILEHDR));
    bitmapFileHdr.u16Type = ((WORD)('M'<<8) | 'B');
    bitmapFileHdr.u32Size = sizeof(BITMAPFILEHDR) + sizeof(BITMAPINFOHDR) + width * height * 3;
    bitmapFileHdr.u16Reserverd1 = 0;
    bitmapFileHdr.u16Reserverd2 = 0;
    bitmapFileHdr.u32OffBits = sizeof(BITMAPFILEHDR) + sizeof(BITMAPINFOHDR);

    BITMAPINFOHDR bitmapInfoHdr;
    memset(&bitmapInfoHdr, 0, sizeof(BITMAPINFOHDR));
    bitmapInfoHdr.u32Size = sizeof(BITMAPINFOHDR);
    bitmapInfoHdr.i32Width = width;
    bitmapInfoHdr.i32Height = height;
    bitmapInfoHdr.u16Planes = 1;
    bitmapInfoHdr.u16Bitcount = 24;
    bitmapInfoHdr.u32Compression = 0; // BI_RGB = 0
    bitmapInfoHdr.u32SizeImage = 0; // This may be set to zero for BI_RGB bitmaps
    bitmapInfoHdr.i32XPelsPerMeter = 0;
    bitmapInfoHdr.i32YPelsPerMeter = 0;
    bitmapInfoHdr.u32ClrUsed = 0;
    bitmapInfoHdr.u32ClrImportant = 0; // If this value is zero, all colors are required.

    QFile binFile(fileName);
    if (!binFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qDebug("Open bmp file fail!");
        return false;
    }

    int len = binFile.write((const char *)(&bitmapFileHdr), sizeof(BITMAPFILEHDR));
    if (len != sizeof(BITMAPFILEHDR)) {
        qCritical("Write bmp file header failed.");
        binFile.close();
        return false;
    }
    else {
        qDebug().noquote() << "BITMAPFILEHDR Len:" << sizeof(BITMAPFILEHDR) << "len:" << len;
    }
    len = binFile.write((const char *)(&bitmapInfoHdr), sizeof(BITMAPINFOHDR));
    if (len != sizeof(BITMAPINFOHDR)) {
        qCritical("Write bmp information header failed.");
        binFile.close();
        return false;
    }
    else {
        qDebug().noquote() << "BITMAPINFOHDR Len:" << sizeof(BITMAPINFOHDR) << "len:" << len;
    }

    uchar *pTmpBuff = new uchar[width * height * 3];
    vertFlipBuf(pTmpBuff, pRgbBuf, width, height);
    len = binFile.write((const char *)pTmpBuff, width * height * 3);
    delete[] pTmpBuff;
    if (len != width * height * 3) {
        qCritical("Write bmp RGB data failed.");
        binFile.close();
        return false;
    }
    else {
        qDebug("RGB data len: %d, len: %d", width * height * 3, len);
    }
    binFile.close();
    return true;
#endif
}

bool FileIo::ReadBmpFile(const QString &fileName, uchar *pRgbBuf, int &width, int &height)
{
    QFile bmpFile(fileName);
    if (!bmpFile.open(QIODevice::ReadOnly)) {
        qCritical().noquote() << "Loading bmp image failed. fileName:" << fileName;
        return false;
    }
    BITMAPFILEHDR bitmapFileHdr;
    memset(&bitmapFileHdr, 0, sizeof(BITMAPFILEHDR));
    UINT32 u32Size = sizeof(BITMAPFILEHDR);
    if (u32Size != bmpFile.read((char *)(&bitmapFileHdr), u32Size))
    {
        qCritical("Read left BITMAPFILEHDR failed");
        bmpFile.close();
        return false;
    }
    BITMAPINFOHDR bitmapInfoHdr;
    memset(&bitmapInfoHdr, 0, sizeof(BITMAPINFOHDR));
    u32Size = sizeof(BITMAPINFOHDR);
    if (u32Size != bmpFile.read((char *)(&bitmapInfoHdr), u32Size))
    {
        qCritical("Read left BITMAPINFOHDR failed");
        bmpFile.close();
        return false;
    }
    width = bitmapInfoHdr.i32Width;
    height = bitmapInfoHdr.i32Height;
    if (pRgbBuf != nullptr) { // read data
        uchar *pTmpBuf = new uchar[width * height * 3];
        if (width * height * 3 != bmpFile.read((char *)pTmpBuf, width * height * 3))
        {
            qCritical().noquote() << "Read bmp data failed. fileName:" << fileName;
            delete[] pTmpBuf;
            bmpFile.close();
            return false;
        }
        vertFlipBuf(pRgbBuf, pTmpBuf, width, height); // flip
        delete[] pTmpBuf;
    }
    bmpFile.close();

    return true;
}

bool FileIo::SaveRawAsBmpFile(const QString &fileName, const uchar *pRawBuf, int width, int height,
                                   uint rawBitWidth, uint blc)
{
    uchar *pRgb24Buf = new uchar[width * height * 3];
    if (nullptr == pRgb24Buf)
    {
        qCritical().noquote() << QObject::tr("Memory allocation for raw image failed!");
        return false;
    }
    unsigned short RAW_MAX_THR = (1 << rawBitWidth) - 1; // Max threshold
    uint rawBitShift = (rawBitWidth > 8) ? (rawBitWidth - 8) : 0;
    int idx = 0;
    for (int i = 0; i < width * height; i++)
    {
        uint rawTmpVal = (rawBitWidth > 8) ? *(ushort*)(pRawBuf + i) : *(pRawBuf + i);
        rawTmpVal = rawTmpVal + blc;
        rawTmpVal = rawTmpVal > RAW_MAX_THR ? RAW_MAX_THR : rawTmpVal;
        rawTmpVal = rawTmpVal >> rawBitShift;
        pRgb24Buf[idx++] = (unsigned char)rawTmpVal;
        pRgb24Buf[idx++] = (unsigned char)rawTmpVal;
        pRgb24Buf[idx++] = (unsigned char)rawTmpVal;
    }
    bool flag = SaveBmpFile(fileName, pRgb24Buf, width, height);
    delete[] pRgb24Buf;
    return flag;
}

bool FileIo::ReadBinFile(const QString &fileName, uchar *pBuf, int len)
{
    QFile binFile(fileName);
    if (!binFile.open(QIODevice::ReadOnly)) {
        qCritical().noquote() << QObject::tr("Open file %1 failed.").arg(fileName);
        return false;
    }
    if (len != binFile.read((char *)pBuf, len)) {
        qCritical().noquote() << QObject::tr("Read file %1 failed.").arg(fileName);
        binFile.close();
        return false;
    }
    binFile.close();
    return true;
}

bool FileIo::SaveBinFile(const QString &fileName, const uchar *pBuf, int len)
{
    QFile binFile(fileName);
    if (!binFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCritical().noquote() << QObject::tr("Open file %1 failed.").arg(fileName);
        return false;
    }
    if (len != binFile.write((const char *)pBuf, len)) {
        qCritical().noquote() << QObject::tr("Write file %1 failed.").arg(fileName);
        binFile.close();
        return false;
    }
    binFile.close();
    return true;
}

bool FileIo::SaveTxtFile(const QString &fileName, const uchar *pBuf, int len)
{
    QFile txtFile(fileName);
    if (!txtFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qCritical().noquote() << QObject::tr("Open file %1 failed.").arg(fileName);
        return false;
    }
    if (len != txtFile.write((const char *)pBuf, len)) {
        qCritical().noquote() << QObject::tr("Write file %1 failed.").arg(fileName);
        txtFile.close();
        return false;
    }
    txtFile.close();
    return true;
}

bool FileIo::SaveTxtFile(const QString &fileName, const QStringList &lineList)
{
    QFile txtFile(fileName);
    if (!txtFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qCritical().noquote() << QObject::tr("Open file %1 failed.").arg(fileName);
        return false;
    }
    foreach (const QString &line, lineList) {
        txtFile.write(line.toLocal8Bit());
    }
    txtFile.close();
    return true;
}

bool FileIo::SaveReport(const QString &fileName, const QString &title, const QStringList &contentLines)
{
    QFile txtFile(fileName);
    if (!txtFile.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text)) {
        qCritical().noquote() << QObject::tr("Open file %1 failed.").arg(fileName);
        return false;
    }
    qint64 offset = txtFile.pos();
    if (0 == offset)
        txtFile.write(title.toLocal8Bit());
    foreach (const QString &line, contentLines) {
        txtFile.write(line.toLocal8Bit());
    }
    txtFile.close();
    return true;
}

void FileIo::vertFlipBuf(uchar *pDestBuff, const uchar *pSrcBuff, int width, int height)
{
    Q_ASSERT(pDestBuff != pSrcBuff);
    for (int j = 0; j < height; j++)
        memcpy(pDestBuff + j * width * 3, pSrcBuff + (height - j - 1) * width * 3, width * 3);
}
