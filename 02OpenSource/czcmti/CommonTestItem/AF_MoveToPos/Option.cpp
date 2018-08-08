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
    m_MoveToPosMethod = Move_InfinityPos;
    m_bSaveAfCode = false;
    m_fEFL = 3.49f;
    m_fInifiDistance = 10.0f;
    m_fMacroDistance = 0.07f;
    m_fMoveDistance = 0.00f;
    m_uInifiAddrH = 0x0044;
    m_uInifiAddrL = 0x0045;
    m_uMacroAddrH = 0x0046;
    m_uMacroAddrL = 0x0047;
    m_uOtherCode = 1023;
    m_uVcmDelay = 100;
}
