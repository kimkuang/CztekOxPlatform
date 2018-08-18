#include "Test_SFR.h"
#include <QDebug>

Test_SFR::Test_SFR()
{

}

double Test_SFR::GetSfr(const void *pImg, int iImgW, int iImgH, int nBytesPerPixel)
{
    sfr.Calc(pImg, iImgW, iImgH, nBytesPerPixel);
    double sfrValue = 0.0f;
    int ec = sfr.GetSfrFromFreq(sfrValue, 0.125);
    if (ec != 0) {
        qCritical("GetSfrFromFreq error!");
    }
    return sfrValue;
}
