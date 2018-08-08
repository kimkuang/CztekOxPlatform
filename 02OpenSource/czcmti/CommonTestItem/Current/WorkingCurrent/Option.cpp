#include "Option.h"
#include "czcmtidefs.h"

Option* Option::m_instance = nullptr;
Option::Option()
{
    RestoreDefaults();
}

Option::~Option()
{
}

Option* Option::GetInstance()
{
    if (m_instance == nullptr)
    {
        m_instance = new Option();
    }
    return m_instance;
}

void Option::FreeInstance()
{
    if (m_instance != nullptr)
    {
        delete m_instance; m_instance = nullptr;
    }
}

void Option::RestoreDefaults()
{
    DvddEnabled = true;
    AvddEnabled = true;
    DovddEnabled = true;
    AfvccEnabled = true;
    VppEnabled = true;
    TotalEnabled = true;

    DvddRange = (int)CurrentRange_mA;
    AvddRange = (int)CurrentRange_mA;
    DovddRange = (int)CurrentRange_mA;
    AfvccRange = (int)CurrentRange_mA;
    VppRange = (int)CurrentRange_mA;
    TotalRange = (int)CurrentRange_mA;

    DvddLowerLimit = 0;
    AvddLowerLimit = 0;
    DovddLowerLimit = 0;
    AfvccLowerLimit = 0;
    VppLowerLimit = 0;
    TotalLowerLimit = 0;

    DvddUpperLimit = 100;
    AvddUpperLimit = 100;
    DovddUpperLimit = 100;
    AfvccUpperLimit = 100;
    VppUpperLimit = 100;
    TotalUpperLimit = 400;
}
