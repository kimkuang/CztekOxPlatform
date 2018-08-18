#ifndef TEST_SFR_H
#define TEST_SFR_H
#include "sfr/SFR.h"

class Test_SFR
{
public:
    Test_SFR();

    double GetSfr(const void *pImg, int iImgW, int iImgH, int nBytesPerPixel);

private:
    SFR sfr;
};

#endif // TEST_SFR_H
