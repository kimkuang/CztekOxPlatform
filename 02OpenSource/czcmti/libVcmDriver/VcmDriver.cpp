#include "VcmDriver.h"
#include <iostream>
#include <QDebug>
#include "IVcmDriver.h"
#include "VcmDriver_AD5823.h"
#include "VcmDriver_AK7371.h"
#include "VcmDriver_AK7374.h"
#include "VcmDriver_BU64241.h"
#include "VcmDriver_DW9714.h"
#include "VcmDriver_DW9718.h"
#include "VcmDriver_DW9761.h"
#include "VcmDriver_A1457.h"
#include "VcmDriver_LV8498.h"
#include "VcmDriver_DW9800.h"
#include "VcmDriver_DW9763.h"
#include "VcmDriver_DW9767.h"
#include "VcmDriver_LC898217.h"
#include "VcmDriver_LC898219.h"
#include "VcmDriver_ZC524.h"

/** 器件子类创建定义处 **/
DRV_ITEM_ALLOC(VcmDriver_A1457)
DRV_ITEM_ALLOC(VcmDriver_AD5823)
DRV_ITEM_ALLOC(VcmDriver_AK7371)
DRV_ITEM_ALLOC(VcmDriver_AK7374)
DRV_ITEM_ALLOC(VcmDriver_BU64241)
DRV_ITEM_ALLOC(VcmDriver_DW9714)
DRV_ITEM_ALLOC(VcmDriver_DW9718)
DRV_ITEM_ALLOC(VcmDriver_DW9761)
DRV_ITEM_ALLOC(VcmDriver_DW9800)
DRV_ITEM_ALLOC(VcmDriver_LV8498)
DRV_ITEM_ALLOC(VcmDriver_DW9763)
DRV_ITEM_ALLOC(VcmDriver_DW9767)
DRV_ITEM_ALLOC(VcmDriver_LC898217)
DRV_ITEM_ALLOC(VcmDriver_LC898219)
DRV_ITEM_ALLOC(VcmDriver_ZC524)

static const _VCM_DRV_TABLE_ VcmDrvTable[]=
{
    DRV_TAB_ITEM(VcmDriver_A1457),
    DRV_TAB_ITEM(VcmDriver_AD5823),
    DRV_TAB_ITEM(VcmDriver_AK7371),
    DRV_TAB_ITEM(VcmDriver_AK7374),
    DRV_TAB_ITEM(VcmDriver_BU64241),
    DRV_TAB_ITEM(VcmDriver_DW9714),
    DRV_TAB_ITEM(VcmDriver_DW9718),
    DRV_TAB_ITEM(VcmDriver_DW9761),
    DRV_TAB_ITEM(VcmDriver_DW9800),
    DRV_TAB_ITEM(VcmDriver_LV8498),
    DRV_TAB_ITEM(VcmDriver_DW9763),
    DRV_TAB_ITEM(VcmDriver_DW9767),
    DRV_TAB_ITEM(VcmDriver_LC898217),
    DRV_TAB_ITEM(VcmDriver_LC898219),
    DRV_TAB_ITEM(VcmDriver_ZC524)
};

VCMDRIVER_API void GetVcmDriverIcList(std::vector<std::string> &driverIcList)
{
    driverIcList.clear();

    unsigned int i;
    for (i = 0; i < sizeof(VcmDrvTable) / sizeof(VcmDrvTable[0]); i++)
    {

        std::string str = VcmDrvTable[i].pchName;

        unsigned int inx = str.find('_', 0);
        if ((inx + 1) < str.length())
        {
            str = str.substr(inx + 1);
        }

        driverIcList.push_back(str);
    }
}

VCMDRIVER_API IVcmDriver *CreateVcmDriver(const std::string &driverIcName, uint i2cAddr)
{
    std::string tDriverIcName = std::string(driverIcName);
    std::transform(tDriverIcName.begin(), tDriverIcName.end(), tDriverIcName.begin(), ::toupper);

    IVcmDriver *vcmDriver = nullptr;

    unsigned int i;
    for (i = 0; i < sizeof(VcmDrvTable) / sizeof(VcmDrvTable[0]); i++)
    {

        std::string str = VcmDrvTable[i].pchName;
        unsigned int inx = str.find('_', 0);
        if ((inx + 1) < str.length())
        {
            str = str.substr(inx + 1);
        }

        std::string strCurName = str;
        std::transform(strCurName.begin(), strCurName.end(), strCurName.begin(), ::toupper);
        if (0 == tDriverIcName.compare(strCurName))
        {
            if (nullptr != VcmDrvTable[i].pGetVcmDrv)
            {
                vcmDriver = VcmDrvTable[i].pGetVcmDrv(i2cAddr);
                break;
            }
        }
    }
    if (i >= sizeof(VcmDrvTable) / sizeof(VcmDrvTable[0]))
    {
        vcmDriver = new VcmDriver(i2cAddr);
    }

    return vcmDriver;
}

VCMDRIVER_API void DestroyVcmDriver(IVcmDriver **vcmDriver)
{
    VcmDriver *pVcmDriver = static_cast<VcmDriver *>(*vcmDriver);
    if (pVcmDriver != nullptr) {
        delete pVcmDriver; pVcmDriver = nullptr;
    }
}

VcmDriver::VcmDriver(uint i2cAddr)
{
    m_i2cAddr = i2cAddr;
    m_channelContext = nullptr;
    m_channelController = nullptr;
}

VcmDriver::~VcmDriver()
{
    m_channelContext = nullptr;
    m_channelController = nullptr;
}

int VcmDriver::VcmInitialize()
{
    //m_channelController->LogToWindow("Please VCM DriverIC", qRgb(255, 0, 0));
    return ERR_NoError;
}

int VcmDriver::BindChannelContext(T_ChannelContext *context)
{
    QString strLog = "";

    if (nullptr == context)
    {
        strLog = QString::asprintf("VcmDriver::%s context is NULL", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255,0,0));
        return ERR_Failed;
    }

    m_channelContext = context;
    m_channelController = context->ChannelController;
    if (nullptr == m_channelController)
    {
        strLog = QString::asprintf("VcmDriver::%s m_channelController is NULL", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255,0,0));
        return ERR_Failed;
    }

    return ERR_NoError;
}

int VcmDriver::VcmReadCode(uint &value)
{
    (void)value;
    m_channelController->LogToWindow("Please VCM DriverIC", qRgb(255, 0, 0));
    return ERR_NoError;
}

int VcmDriver::VcmWriteCode(uint value)
{
    (void)value;
    m_channelController->LogToWindow("Please VCM DriverIC", qRgb(255, 0, 0));
    return ERR_NoError;
}

int VcmDriver::VcmFinalize()
{
    //m_channelController->LogToWindow("Please VCM DriverIC", qRgb(255, 0, 0));
    return ERR_NoError;
}
