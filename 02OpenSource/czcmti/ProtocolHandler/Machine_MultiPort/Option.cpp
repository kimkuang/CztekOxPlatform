#include "Option.h"

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
    ListenPort1 = 4321;
    ListenPort2 = 4322;
    ListenPort3 = 4323;
    ListenPort4 = 4324;
    ListenIp = "";
    HandshakeRequest = "";
    HandshakeResponse = "";
}

