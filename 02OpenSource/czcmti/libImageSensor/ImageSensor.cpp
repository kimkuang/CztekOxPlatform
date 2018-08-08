#include "ImageSensor.h"
#include <typeinfo>
#include "GC2375.h"
#include "GC5025.h"
#include "HI1333.h"
#include "IMX258_0AQH5.h"
#include "IMX350.h"
#include "IMX351.h"
#include "IMX355.h"
#include "IMX363.h"
#include "IMX376.h"
#include "IMX380.h"
#include "IMX398.h"
#include "IMX476.h"
#include "IMX486.h"
#include "IMX498.h"
#include "IMX519.h"
#include "IMX550.h"
#include "IMX576.h"
#include "IMX600.h"
#include "OV02A10.h"
#include "OV08A10.h"
#include "OV12A10.h"
#include "OV13A40.h"
#include "OV16885.h"
#include "OV16B10.h"
#include "OV24A1B.h"
#include "S5K2L7SA.h"
#include "S5K3T1SP03.h"
#include "S5K4H7YX.h"
#include "S5K5E8YX.h"
#include "S5K5E9YU.h"
#include <algorithm>
#include <QString>
#include <QDebug>
#ifdef __GNUC__
#include <cxxabi.h>
#endif

/** 器件子类创建定义处 **/
DRV_ITEM_ALLOC(GC2375)
DRV_ITEM_ALLOC(GC5025)
DRV_ITEM_ALLOC(HI1333)
DRV_ITEM_ALLOC(IMX258)
DRV_ITEM_ALLOC(IMX350)
DRV_ITEM_ALLOC(IMX351)
DRV_ITEM_ALLOC(IMX355)
DRV_ITEM_ALLOC(IMX363)
DRV_ITEM_ALLOC(IMX376)
DRV_ITEM_ALLOC(IMX380)
DRV_ITEM_ALLOC(IMX398)
DRV_ITEM_ALLOC(IMX476)
DRV_ITEM_ALLOC(IMX486)
DRV_ITEM_ALLOC(IMX498)
DRV_ITEM_ALLOC(IMX519)
DRV_ITEM_ALLOC(IMX550)
DRV_ITEM_ALLOC(IMX576)
DRV_ITEM_ALLOC(IMX600)
DRV_ITEM_ALLOC(OV02A10)
DRV_ITEM_ALLOC(OV08A10)
DRV_ITEM_ALLOC(OV12A10)
DRV_ITEM_ALLOC(OV13A40)
DRV_ITEM_ALLOC(OV16885)
DRV_ITEM_ALLOC(OV16B10)
DRV_ITEM_ALLOC(OV24A1B)
DRV_ITEM_ALLOC(S5K2L7SA)
DRV_ITEM_ALLOC(S5K3T1SP03)
DRV_ITEM_ALLOC(S5K4H7YX)
DRV_ITEM_ALLOC(S5K5E8YX)
DRV_ITEM_ALLOC(S5K5E9YU)

static const _IMAGE_SENSOR_TABLE_ ImgSensorTable[]=
{
    DRV_TAB_ITEM(GC2375),
    DRV_TAB_ITEM(GC5025),
    DRV_TAB_ITEM(HI1333),
    DRV_TAB_ITEM(IMX258),
    DRV_TAB_ITEM(IMX350),
    DRV_TAB_ITEM(IMX351),
    DRV_TAB_ITEM(IMX355),
    DRV_TAB_ITEM(IMX363),
    DRV_TAB_ITEM(IMX376),
    DRV_TAB_ITEM(IMX380),
    DRV_TAB_ITEM(IMX398),
    DRV_TAB_ITEM(IMX476),
    DRV_TAB_ITEM(IMX486),
    DRV_TAB_ITEM(IMX498),
    DRV_TAB_ITEM(IMX519),
    DRV_TAB_ITEM(IMX550),
    DRV_TAB_ITEM(IMX576),
    DRV_TAB_ITEM(IMX600),
    DRV_TAB_ITEM(OV02A10),
    DRV_TAB_ITEM(OV08A10),
    DRV_TAB_ITEM(OV12A10),
    DRV_TAB_ITEM(OV13A40),
    DRV_TAB_ITEM(OV16885),
    DRV_TAB_ITEM(OV16B10),
    DRV_TAB_ITEM(OV24A1B),
    DRV_TAB_ITEM(S5K2L7SA),
    DRV_TAB_ITEM(S5K3T1SP03),
    DRV_TAB_ITEM(S5K4H7YX),
    DRV_TAB_ITEM(S5K5E8YX),
    DRV_TAB_ITEM(S5K5E9YU)
};

IMAGESENSOR_API IImageSensor *CreateImageSensor(const T_SensorSetting &sensorSetting)
{
    std::string tSensorName = std::string(sensorSetting.ChipName);
    std::transform(tSensorName.begin(), tSensorName.end(), tSensorName.begin(), ::toupper);
    IImageSensor *imageSensor = nullptr;

    unsigned int i;
    for (i = 0; i < sizeof(ImgSensorTable) / sizeof(ImgSensorTable[0]); i++)
    {
        std::string strCurName = ImgSensorTable[i].pchName;
        std::transform(strCurName.begin(), strCurName.end(), strCurName.begin(), ::toupper);
        if (0 == tSensorName.compare(strCurName))
        {
            if (nullptr != ImgSensorTable[i].pGetSensor)
            {
                imageSensor = ImgSensorTable[i].pGetSensor(sensorSetting);
                break;
            }
        }
    }

    if (i >= sizeof(ImgSensorTable) / sizeof(ImgSensorTable[0]))
    {
        imageSensor = new ImageSensor(sensorSetting);
    }

    return imageSensor;
}

IMAGESENSOR_API void DestroyImageSensor(IImageSensor **imageSensor)
{
    ImageSensor *pImageSensor = static_cast<ImageSensor *>(*imageSensor);
    if (pImageSensor != nullptr) {
        delete pImageSensor; pImageSensor = nullptr;
    }
}

ImageSensor::ImageSensor(const T_SensorSetting &sensorSetting)
{
    m_sensorSetting = sensorSetting;

    m_channelController = nullptr;
    m_channelContext = nullptr;
}

ImageSensor::~ImageSensor()
{
    m_channelController = nullptr;
    m_channelContext = nullptr;
}

int ImageSensor::ApplyAWBGain(int rg, int bg, int Typical_rg, int Typical_bg)
{
    (void)rg;
    (void)bg;
    (void)Typical_rg;
    (void)Typical_bg;
    m_channelController->LogToWindow("Please select image sensor", qRgb(255,0,0));
    return ERR_NoError;
}

int ImageSensor::ApplyLsc(const uchar* raw8,int width,int height,int nLSCTarget,int ob,uchar* nLenCReg,int nLenCRegCount,int LSCGroup)
{
    (void)raw8;
    (void)width;
    (void)height;
    (void)nLSCTarget;
    (void)ob;
    (void)nLenCReg;
    (void)nLenCRegCount;
    (void)LSCGroup;
    m_channelController->LogToWindow("Please select image sensor", qRgb(255,0,0));
    return ERR_NoError;
}

int ImageSensor::ApplySpc(short *pSPC)
{
    (void)pSPC;
    m_channelController->LogToWindow("Please select image sensor", qRgb(255,0,0));
    return ERR_NoError;
}

int ImageSensor::GetTemperature(int &temperature)
{
    (void)temperature;
    m_channelController->LogToWindow("Please select image sensor", qRgb(255,0,0));
    return ERR_NoError;
}

int ImageSensor::WriteRegisterList(const QList<T_RegConf> &regList)
{
    const T_I2CCommParam &i2cParam = m_sensorSetting.I2cParam;
    int ec = m_channelController->WriteRegisterList(i2cParam, regList);
    if (ec < 0) {
        m_channelController->LogToWindow(QObject::tr("Write register list failed."), qRgb(255,0,0));
        return ec;
    }
    return ec;
}

int ImageSensor::BindChannelContext(T_ChannelContext *context)
{
    if (nullptr == context)
    {
        m_channelController->LogToWindow("context is NULL", qRgb(255,0,0));
        return ERR_Failed;
    }

    m_channelContext = context;
    m_channelController = context->ChannelController;
    if (m_channelController != nullptr) {
        std::string className;
#ifdef __GNUC__
        char* realName = abi::__cxa_demangle(typeid(*this).name(), nullptr, nullptr, nullptr);
        className = std::string(realName);
        free(realName);
#else
        className = typeid(*this).name();
#endif
        std::string upperClassName = className;
        std::transform(upperClassName.begin(), upperClassName.end(), upperClassName.begin(), ::toupper);

        std::string chipName = std::string(m_sensorSetting.ChipName);
        std::string upperChipName = chipName;
        std::transform(upperChipName.begin(), upperChipName.end(), upperChipName.begin(), ::toupper);        
        if (upperClassName.find(upperChipName) == std::string::npos)
            m_channelController->LogToWindow("Image Sensor Name: " + QString::fromStdString(chipName) + "!= Class Name: " + QString::fromStdString(className), qRgb(255, 0, 0));
    }

    return ERR_NoError;
}

int ImageSensor::GetSensorFuseId(std::string &fuseId, bool bUseMasterI2C)
{
    (void)bUseMasterI2C;
    fuseId = "";
    m_channelController->LogToWindow("Please select image sensor", qRgb(255,0,0));
    return ERR_NoError;
}

int ImageSensor::SetSensorExposure(uint value)
{
    (void)(value);
    m_channelController->LogToWindow("Please select image sensor", qRgb(255,0,0));
    return ERR_NoError;
}

int ImageSensor::SetSensorGain(uint value, bool bMultiple)
{
    (void)(value);
    (void)(bMultiple);
    m_channelController->LogToWindow("Please select image sensor", qRgb(255,0,0));
    return ERR_NoError;
}

int ImageSensor::GetSensorExposure(uint &value)
{
    value = 0;
    m_channelController->LogToWindow("Please select image sensor", qRgb(255,0,0));
    return ERR_NoError;
}

int ImageSensor::GetSensorGain(uint &value)
{
    value = 0;
    m_channelController->LogToWindow("Please select image sensor", qRgb(255,0,0));
    return ERR_NoError;
}

int ImageSensor::I2cRead(ushort uRegAddr, ushort &uRegVal, ushort uMode)
{
    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    int ec = m_channelController->ReadDiscreteI2c(i2cParam.Addr, i2cParam.Speed * 100, uMode,
                                &uRegAddr, &uRegVal, 1);

    return ec;
}

int ImageSensor::I2cWrite(ushort uRegAddr, ushort uRegVal, ushort uMode)
{
    T_I2CCommParam i2cParam = m_sensorSetting.I2cParam;
    int ec = m_channelController->WriteDiscreteI2c(i2cParam.Addr, i2cParam.Speed * 100, uMode,
                                  &uRegAddr, &uRegVal, 1);
    if (ec < 0) {
        qCritical("Call WriteDiscreteI2c() failed[%d].", ec);
        return ec;
    }

    return ec;
}

int ImageSensor::readRegisters(QVector<ST_RegData> &regData, ushort uMode)
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

int ImageSensor::writeRegisters(const QVector<ST_RegData> &regData, ushort uMode)
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

int ImageSensor::i2cReadContinuous(uint regAddr, uint regAddrSize, uchar *data, uint dataSize)
{
    const T_I2CCommParam &i2cParam = m_sensorSetting.I2cParam;
    return m_channelController->ReadContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100,
                                                    regAddr, regAddrSize, data, dataSize);

}

int ImageSensor::i2cWriteContinuous(uint regAddr, uint regAddrSize, const uchar *data, uint dataSize)
{
    const T_I2CCommParam &i2cParam = m_sensorSetting.I2cParam;
    return m_channelController->WriteContinuousI2c(i2cParam.Addr, i2cParam.Speed * 100,
                                                    regAddr, regAddrSize, data, dataSize);
}
