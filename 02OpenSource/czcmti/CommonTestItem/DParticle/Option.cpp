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
    m_LeftSL = 0;
    m_RightSL = 0;
    m_TopSL = 0;
    m_BottomSL = 0;
    m_AreaWidth = 9;
    m_AreaHeight = 9;
    m_DeadUnit = 16;
    m_DeadSpec = 50;
    m_DeadType = 0;
    m_DefultExposure = 16;
    m_DefultGain = 16;
    m_SaveImage = false;
}

