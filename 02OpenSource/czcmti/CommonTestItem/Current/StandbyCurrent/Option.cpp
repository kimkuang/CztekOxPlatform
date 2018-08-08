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

    DvddRange = (int)CurrentRange_uA;
    AvddRange = (int)CurrentRange_uA;
    DovddRange = (int)CurrentRange_uA;
    AfvccRange = (int)CurrentRange_uA;
    VppRange = (int)CurrentRange_uA;
    TotalRange = (int)CurrentRange_uA;

    DvddUpperLimit = 50;
    AvddUpperLimit = 50;
    DovddUpperLimit = 50;
    AfvccUpperLimit = 50;
    VppUpperLimit = 50;
    TotalUpperLimit = 50;

    ContinueWhenFailed = false;
}
