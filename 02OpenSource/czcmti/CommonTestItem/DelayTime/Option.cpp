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
    m_Enable = FALSE;
    m_DelayTime = 100;
}

