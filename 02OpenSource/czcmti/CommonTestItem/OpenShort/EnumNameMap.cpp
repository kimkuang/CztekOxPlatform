#include "EnumNameMap.h"
#include "czcmtidefs.h"

EnumNameMap::EnumNameMap()
{
    initPinNameMap();
}

const std::map<int, std::string> &EnumNameMap::GetPinNameMap() const
{
    return m_pinNameMap;
}

int EnumNameMap::FindPinIdx(const std::string &name)
{
    for (auto it = m_pinNameMap.begin(); it != m_pinNameMap.end(); ++it) {
        if (it->second == name) {
            return it->first;
        }
    }
    return OSM_PIN_INVALID;
}

std::string EnumNameMap::FindPinName(int idx)
{
    if (m_pinNameMap.find(idx) != m_pinNameMap.end())
        return m_pinNameMap[idx];
    return "Unkown pin ID[" + std::to_string(idx) + "]";
}

void EnumNameMap::initPinNameMap()
{
    m_pinNameMap.clear();
    m_pinNameMap.insert(std::make_pair(OSM_PIN_AVDD, "AVDD"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_DOVDD, "DOVDD"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_DVDD, "DVDD"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_AFVCC, "AFVCC"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_VPP, "VPP"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_MCLK, "MCLK"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_SCL, "SCL"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_SDA, "SDA"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_PWDN, "PWDN"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_RST, "RST"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_PO1, "PO1"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_PO2, "PO2"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_PO3, "PO3"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_PO4, "PO4"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_MIPI_D3P, "D3P"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_MIPI_D3N, "D3N"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_MIPI_D2P, "D2P"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_MIPI_D2N, "D2N"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_MIPI_D1P, "D1P"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_MIPI_D1N, "D1N"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_MIPI_D0P, "D0P"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_MIPI_D0N, "D0N"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_MIPI_CLKP, "CLKP"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_MIPI_CLKN, "CLKN"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_SGND1, "SGND1"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_SGND2, "SGND2"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_SGND3, "SGND3"));
    m_pinNameMap.insert(std::make_pair(OSM_PIN_SGND4, "SGND4"));
}
