#ifndef __SFR_H__
#define __SFR_H__
#include "library_global.h"

class SFRPrivate;
class LIBRARY_API SFR
{
public:
    SFR();
    ~SFR();

public:
    // 计算SFR pImg - raw10/raw8 image buffer
    void Calc(const void *pImg, int iImgW, int iImgH, int nBytesPerPixel);
    // 获取指定频率下的sfr值
    int GetSfrFromFreq(double& sfr, double freq);
    // 获取指定解析力（sfr值）下的频率
    int GetFreqFromSfr(double& freq, double sfr);

private:
    SFRPrivate * const d_ptr;
};

#endif /* __SFR_H__ */

