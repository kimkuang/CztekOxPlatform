#include "OtpSensor.h"
#include <algorithm>
#include <QString>
#include <QDebug>
#include "EFLASH_DW9763.h"
#include "EFLASH_DW9767.h"
#include "OTP_S5K4H7YX.h"
#include "OTP_S5K5E8YX.h"
#include "OTP_S5K5E9YU.h"
#include "EEPROM_GT24C16.h"
#include "EEPROM_GT24C32.h"
#include "EEPROM_GT24C64.h"
#include "EEPROM_GT24P64A.h"
#include "EEPROM_GT24P64B.h"
#include "EEPROM_GT24C128.h"
#include "EEPROM_GT24P128B.h"
#include "EEPROM_GT24P128C.h"
#include "EEPROM_BL24SA64.h"
#include "EEPROM_Normal.h"

/** 器件子类创建定义处 **/
DRV_ITEM_ALLOC(EFLASH_DW9763)
DRV_ITEM_ALLOC(EFLASH_DW9767)
DRV_ITEM_ALLOC(OTP_S5K4H7YX)
DRV_ITEM_ALLOC(OTP_S5K5E8YX)
DRV_ITEM_ALLOC(OTP_S5K5E9YU)
DRV_ITEM_ALLOC(EEPROM_GT24C16)
DRV_ITEM_ALLOC(EEPROM_GT24C32)
DRV_ITEM_ALLOC(EEPROM_GT24C64)
DRV_ITEM_ALLOC(EEPROM_GT24P64A)
DRV_ITEM_ALLOC(EEPROM_GT24P64B)
DRV_ITEM_ALLOC(EEPROM_GT24C128)
DRV_ITEM_ALLOC(EEPROM_GT24P128B)
DRV_ITEM_ALLOC(EEPROM_GT24P128C)
DRV_ITEM_ALLOC(EEPROM_BL24SA64)

static const _OTP_SENSOR_TABLE_ OtpSensorTable[]=
{
    DRV_TAB_ITEM(EFLASH_DW9763),
    DRV_TAB_ITEM(EFLASH_DW9767),
    DRV_TAB_ITEM(OTP_S5K4H7YX),
    DRV_TAB_ITEM(OTP_S5K5E8YX),
    DRV_TAB_ITEM(OTP_S5K5E9YU),
    DRV_TAB_ITEM(EEPROM_GT24C16),
    DRV_TAB_ITEM(EEPROM_GT24C32),
    DRV_TAB_ITEM(EEPROM_GT24C64),
    DRV_TAB_ITEM(EEPROM_GT24P64A),
    DRV_TAB_ITEM(EEPROM_GT24P64B),
    DRV_TAB_ITEM(EEPROM_GT24C128),
    DRV_TAB_ITEM(EEPROM_GT24P128B),
    DRV_TAB_ITEM(EEPROM_GT24P128C),
    DRV_TAB_ITEM(EEPROM_BL24SA64)
};

OTPSENSOR_API void GetOtpSensorList(std::list<std::string> &otpSensorList)
{
    otpSensorList.clear();

    unsigned int i;
    QString strLog = "";
    for (i = 0; i < sizeof(OtpSensorTable) / sizeof(OtpSensorTable[0]); i++)
    {
        strLog = QString::asprintf("GetOtpSensorList()---i:%d SensorName:%s", i, OtpSensorTable[i].pchName);
        qDebug() << strLog;
        otpSensorList.push_back(OtpSensorTable[i].pchName);
    }
}

OTPSENSOR_API IOtpSensor *CreateOtpSensor(const std::string &sensorName, uint i2cAddr)
{
    std::string tSensorName = std::string(sensorName);
    std::transform(tSensorName.begin(), tSensorName.end(), tSensorName.begin(), ::toupper);
    IOtpSensor *otpSensor = nullptr;

    unsigned int i;
    for (i = 0; i < sizeof(OtpSensorTable) / sizeof(OtpSensorTable[0]); i++)
    {
        std::string strCurName = OtpSensorTable[i].pchName;
        std::transform(strCurName.begin(), strCurName.end(), strCurName.begin(), ::toupper);
        if (0 == tSensorName.compare(strCurName))
        {
            if (nullptr != OtpSensorTable[i].pGetOtp)
            {
                otpSensor = OtpSensorTable[i].pGetOtp(i2cAddr);
                break;
            }
        }
    }
    if (i >= sizeof(OtpSensorTable) / sizeof(OtpSensorTable[0]))
    {
        otpSensor = new OtpSensor(i2cAddr);
    }

    return otpSensor;
}

OTPSENSOR_API void DestroyOtpSensor(IOtpSensor **otpSensor)
{
    OtpSensor *pOtpSensor = static_cast<OtpSensor *>(*otpSensor);
    if (pOtpSensor != nullptr) {
        delete pOtpSensor; pOtpSensor = nullptr;
    }
}

OtpSensor::OtpSensor(uint i2cAddr)
{
    m_channelContext = nullptr;
    m_channelController = nullptr;
    m_i2cAddr = i2cAddr;
}

OtpSensor::~OtpSensor()
{
    m_channelContext = nullptr;
    m_channelController = nullptr;
}

int OtpSensor::BindChannelContext(T_ChannelContext *context)
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
    if ((nullptr == m_channelContext) || (nullptr == m_channelController) || (nullptr == m_channelContext->ImageSensor))
    {
        strLog = QString::asprintf("VcmDriver::%s m_channelController is NULL", __FUNCTION__);
        m_channelController->LogToWindow(strLog, qRgb(255,0,0));
        return ERR_Failed;
    }

    return ERR_NoError;
}

int OtpSensor::OtpRead(uint startAddr, uint endAddr, uchar *data, ushort page)
{
    (void)startAddr;
    (void)endAddr;
    (void)data;
    (void)page;
    m_channelController->LogToWindow("Please select Otp Sensor", qRgb(255,0,0));
    return ERR_NoError;
}

int OtpSensor::OtpWrite(uint startAddr, uint endAddr, const uchar *data, ushort page)
{
    (void)startAddr;
    (void)endAddr;
    (void)data;
    (void)page;
    m_channelController->LogToWindow("Please select Otp Sensor", qRgb(255,0,0));
    return ERR_NoError;
}

int OtpSensor::I2cRead(ushort uRegAddr, ushort& uRegVal, ushort uMode)
{
    int ec = m_channelController->ReadDiscreteI2c(m_i2cAddr, 400, uMode, &uRegAddr, &uRegVal, 1);

    return ec;
}

int OtpSensor::I2cWrite(ushort uRegAddr, ushort uRegVal, ushort uMode)
{
    int ec = m_channelController->WriteDiscreteI2c(m_i2cAddr, 400, uMode, &uRegAddr, &uRegVal, 1);
    if (ec < 0) {
        qCritical("Call WriteDiscreteI2c() failed[%d].", ec);
        return ec;
    }

    return ec;
}

int OtpSensor::I2cReadBlock(ushort uRegAddr, ushort uRegSize, uchar* pData, ushort uSize)
{
    if (uSize > I2C_RW_MAX_BYTES) {
        qCritical("Input length[%d] is too large", uSize);
        return ERR_InvalidParameter;
    }

    ushort uCnt = 0;
    int iRet = ERR_NoError;
    for (uCnt = 0; uCnt < 10; ++uCnt)
    {
        if (ERR_NoError == (iRet = m_channelController->ReadContinuousI2c(m_i2cAddr, 400, uRegAddr, uRegSize, pData, uSize)))
        {
            break;
        }

        QThread::msleep(1);
    }

    return iRet;
}

int OtpSensor::I2cWriteBlock(ushort uRegAddr, ushort uRegSize, const uchar* pData, ushort uSize)
{ 
    if (uSize > I2C_RW_MAX_BYTES) {
        qCritical("Input length[%d] is too large", uSize);
        return ERR_InvalidParameter;
    }

    ushort uCnt = 0;
    int iRet = ERR_NoError;
    for (uCnt = 0; uCnt < 10; ++uCnt)
    {
        if (ERR_NoError == (iRet = m_channelController->WriteContinuousI2c(m_i2cAddr, 400, uRegAddr, uRegSize, pData, uSize)))
        {
            break;
        }

        QThread::msleep(3);
    }

    return iRet;
}

int OtpSensor::readRegisters(QVector<ST_RegData> &regData, ushort uMode)
{
    int iRet = ERR_NoError;
    for (int i = 0; i < regData.size(); i++)
    {
        iRet = I2cRead(regData[i].RegAddr, regData[i].Data, uMode);
        if (iRet < 0)
        {
            return iRet;
        }
    }

    return iRet;
}

int OtpSensor::writeRegisters(const QVector<ST_RegData> &regData, ushort uMode)
{
    int iRet = ERR_NoError;
    for (int i = 0; i < regData.size(); i++)
    {
        iRet = I2cWrite(regData[i].RegAddr, regData[i].Data, uMode);
        if (iRet < 0)
        {
            return iRet;
        }
    }

    return iRet;
}
