#ifndef OPTION_H
#define OPTION_H
#include "czcmtidefs.h"
#include <vector>
#include <string>
#include <QRect>

class Option
{
public:
    typedef struct _StructShadingROIInfo
    {
        QRect pos;
        int cx;
        int cy;
        double avgY;
        double avgR;
        double avgG;
        double avgB;
        double avgGr;
        double avgGb;

        double RGain;
        double BGain;
    }ROIINFO;

    typedef struct _StructShading
    {
        ROIINFO Roi[5];
        uint u32RoiH;
        uint u32RoiW;
        uint u32Blc;
    }SHADING;

    SHADING m_stShading;

    uint m_u32VcmDelay;
    uint m_u32Y1Min;
    uint m_u32Y1Max;
    uint m_u32Y2Min;
    uint m_u32Y2Max;
    uint m_u32Y3Min;
    uint m_u32Y3Max;
    uint m_u32Y4Min;
    uint m_u32Y4Max;

public:
    Option();
    ~Option();
    void RestoreDefaults();

private:
    Option(const Option &) = delete;
    Option& operator = (const Option&) = delete;
};

#endif // OPTION_H
