#include <QCoreApplication>
#include "common/FileIo.h"

#include "Test_SFR.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString testImgFileName = qApp->applicationDirPath() + "/../image/ROI0GrayScale_96x96_10bit.raw";

    Test_SFR testSFR;
    int iImgW = 96;
    int iImgH = 96;
    int nBytesPerPixel = 2;
    int len = iImgW * iImgH * nBytesPerPixel;
    uchar *pImg = new uchar[len];
    FileIo::ReadBinFile(testImgFileName, pImg, len);
    double sfrValue = testSFR.GetSfr(pImg, iImgW, iImgH, nBytesPerPixel);
    qDebug("sfr: %.10f", sfrValue);
    delete[] pImg;

    return a.exec();
}
