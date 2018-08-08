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
    m_b10BitCal = false;
    m_iSensorType = NORMAL;
    m_iRefMode = CHLPIXEL;
    m_iAECalMode = RTARGET;
    m_iAEMinVal = 190;
    m_iAEMaxVal = 210;
    m_iRoiW = 5;
    m_iRoiH = 5;
    m_iBlc = 16;
    m_iStartX = 50;
    m_iStartY = 50;
    m_iAETimes = 20;
    m_iAEInfiVal = 0x0180;
    m_iAERememberVal = 0x0180;
    m_bRememberAE = false;
    m_iAnalogGain = 0;      //默认1.0x
    m_iAEFrame = 5;
    m_iWaitTime = 100;
}
