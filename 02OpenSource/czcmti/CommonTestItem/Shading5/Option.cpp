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
    int xArray[] = { 50, 5, 5, 95, 95 };
    int yArray[] = { 50, 5, 95, 5, 95 };
    for (int i = 0; i < 5; i++) {
        m_RoiConfs[i].Enable = true;
        m_RoiConfs[i].XPercent = xArray[i];
        m_RoiConfs[i].YPercent = yArray[i];
    }
    m_Continue = false;
    m_BLC = 16;
    m_YDiff = 23;
    m_YMax = 50;
    m_YMin = 27;
    m_BGMin = 80;
    m_BGMax = 120;
    m_RGMax = 120;
    m_RGMin = 80;
    m_RoiWidthPercent = 10;
    m_RoiHeightPercent = 10;
}

