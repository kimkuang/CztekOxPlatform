#ifndef FILEIO_H
#define FILEIO_H
#include <QString>
#include "czcmtidefs.h"

class FileIo
{
public:
    static bool SaveBmpFile(const QString &fileName, const uchar *pRgbBuf, int width, int height);
    static bool ReadBmpFile(const QString &fileName, uchar *pRgbBuf, int &width, int &height);
    static bool SaveRawAsBmpFile(const QString &fileName, const uchar *pRawBuf, int width, int height,
                                 uint rawBitWidth, uint blc);
    static bool ReadBinFile(const QString &fileName, uchar *pBuf, int len);
    static bool SaveBinFile(const QString &fileName, const uchar *pBuf, int len);
    static bool SaveTxtFile(const QString &fileName, const uchar *pBuf, int len);
    static bool SaveTxtFile(const QString &fileName, const QStringList &lineList);
    static bool SaveReport(const QString &fileName, const QString &title, const QStringList &contentLines);

private:
    static void vertFlipBuf(uchar *pDestBuff, const uchar *pSrcBuff, int width, int height);

private:
#pragma pack(push, 1)
typedef struct tagBITMAPFILEHEDR
{
    uint16       u16Type;       //位图文件的类型，必须为BM
    uint32       u32Size;       //文件大小，以字节为单位
    uint16       u16Reserverd1; //位图文件保留字，必须为0
    uint16       u16Reserverd2; //位图文件保留字，必须为0
    uint32       u32OffBits;    //位图文件头到数据的偏移量，以字节为单位
} BITMAPFILEHDR;

typedef struct tagBITMAPINFOHEDR
{
    uint32  u32Size;                 //该结构大小，字节为单位
    INT32   i32Width;                //图形宽度以象素为单位
    INT32   i32Height;               //图形高度以象素为单位
    uint16  u16Planes;               //目标设备的级别，必须为1
    uint16  u16Bitcount;             //颜色深度，每个象素所需要的位数
    uint32  u32Compression;          //位图的压缩类型
    uint32  u32SizeImage;            //位图的大小，以字节为单位
    INT32   i32XPelsPerMeter;        //位图水平分辨率，每米像素数
    INT32   i32YPelsPerMeter;        //位图垂直分辨率，每米像素数
    uint32  u32ClrUsed;              //位图实际使用的颜色表中的颜色数
    uint32  u32ClrImportant;         //位图显示过程中重要的颜色数
} BITMAPINFOHDR;
#pragma pack(pop)
};

#endif // FILEIO_H
