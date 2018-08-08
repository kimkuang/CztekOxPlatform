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
    ListenPort = 4321;
    ForwardPort = 4321;
    ListenIp = "";
    ForwardIp = "";
    HandshakeRequest = "";
    HandshakeResponse = "";
}

