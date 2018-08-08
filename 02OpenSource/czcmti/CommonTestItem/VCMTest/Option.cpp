#include "Option.h"
#include <stdio.h>

Option::Option()
{
    RestoreDefaults();
}

Option::~Option()
{
}

void Option::RestoreDefaults()
{
    //memset(&m_stShading, 0, sizeof(m_stShading));
    m_stShading.u32RoiH = m_stShading.u32RoiW = 5;
    m_stShading.u32Blc = 16;
    m_stShading.Roi[0].cx = m_stShading.Roi[0].cy = 50;
    m_stShading.Roi[1].cx = m_stShading.Roi[1].cy = 5;
    m_stShading.Roi[2].cx = 95;
    m_stShading.Roi[2].cy = 5;
    m_stShading.Roi[3].cx = 5;
    m_stShading.Roi[3].cy = 95;
    m_stShading.Roi[4].cx = m_stShading.Roi[4].cy = 95;
    m_u32VcmDelay = 1000;
    m_u32Y1Min = m_u32Y2Min = m_u32Y3Min = m_u32Y4Min = 0;
    m_u32Y1Max = m_u32Y2Max = m_u32Y3Max = m_u32Y4Max = 0;
}
