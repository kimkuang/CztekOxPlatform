#include "Option.h"
#include "EnumNameMap.h"
#include "czcmtidefs.h"
#include "macrodefs.h"

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
    SupplyVol_mV = 1400;
    SupplyCur_uA = 5000;
    OpenThreshold_mV = 1000;
    ShortThreshold_mV = 200;
    TestPinVector.clear();

    int defaultPinIds[] = {
        OSM_PIN_AVDD,
        OSM_PIN_DOVDD,
        OSM_PIN_DVDD,
        OSM_PIN_AFVCC,
        OSM_PIN_MCLK,
        OSM_PIN_SCL,
        OSM_PIN_SDA,
        OSM_PIN_PWDN,
        OSM_PIN_MIPI_D3P,
        OSM_PIN_MIPI_D3N,
        OSM_PIN_MIPI_D2P,
        OSM_PIN_MIPI_D2N,
        OSM_PIN_MIPI_D1P,
        OSM_PIN_MIPI_D1N,
        OSM_PIN_MIPI_D0P,
        OSM_PIN_MIPI_D0N,
        OSM_PIN_MIPI_CLKP,
        OSM_PIN_MIPI_CLKN,
        OSM_PIN_SGND1,
    };
    for (uint i = 0; i < ARRAY_SIZE(defaultPinIds); i++) {
        TestPinVector.push_back(defaultPinIds[i]);
    }

    ContinueWhenFailed = false;
}
