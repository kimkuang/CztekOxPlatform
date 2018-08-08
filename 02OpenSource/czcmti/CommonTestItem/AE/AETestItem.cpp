#include "AETestItem.h"
#include <QDebug>
#include <math.h>
#include "IImageSensor.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

CZPLUGIN_API ICzPlugin *CreatePlugin(void *arg)
{
    return new AETestItem((const char *)arg);
}

CZPLUGIN_API void DestroyPlugin(ICzPlugin **plugin)
{
    AETestItem *pTestItem = (AETestItem *)(*plugin);
    if (pTestItem != nullptr) {
        delete pTestItem; pTestItem = nullptr;
        *plugin = nullptr;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

AETestItem::AETestItem(const char *instanceName)
{
    m_instanceName = QString::fromLatin1(instanceName);
    m_initialized = false;
    m_option = new Option();
    m_wdtConf = new ConfWidget(m_option);
    m_context = nullptr;
    m_u64ExpGainTimeStamp = 0;
    m_u32Count = 0;
    m_fBrightness = 0;
}

AETestItem::~AETestItem()
{
    delete m_wdtConf;
    delete m_option;
}

int AETestItem::GetPluginInfo(T_PluginInfo &pluginInfo)
{
    pluginInfo.Version = 0x0900000C;
    strcpy(pluginInfo.Description, "Auto Exspoure(20180802)");
    strcpy(pluginInfo.FriendlyName, "AE");
    strcpy(pluginInfo.VendorName, "CZTEK");
    strcpy(pluginInfo.InstanceName, m_instanceName.toLocal8Bit().data());
    pluginInfo.OptionDlgHandle = (uint64)m_wdtConf;

    return 0;
}

int AETestItem::LoadOption()
{
    if ((m_context != nullptr) && (m_context->ChannelController == nullptr))
        return ERR_Failed;
    QMap<QString, QString> configurations;
    m_context->ModuleSettings->ReadSection(m_instanceName, configurations);
    {
        if (configurations.contains("10BitCal")) {
            m_option->m_b10BitCal = (configurations["10BitCal"].toInt() == 1);
        }
        if (configurations.contains("SensorType")) {
            m_option->m_iSensorType = configurations["SensorType"].toInt();
        }
        if (configurations.contains("RefMode")) {
            m_option->m_iRefMode = configurations["RefMode"].toInt();
        }
        if (configurations.contains("AECalMode")) {
            m_option->m_iAECalMode = configurations["AECalMode"].toInt();
        }
        if (configurations.contains("AEMinVal")) {
            m_option->m_iAEMinVal = configurations["AEMinVal"].toInt();
        }
        if (configurations.contains("AEMaxVal")) {
           m_option->m_iAEMaxVal = configurations["AEMaxVal"].toInt();
        }
        if (configurations.contains("RoiW")) {
           m_option->m_iRoiW = configurations["RoiW"].toInt();
        }
        if (configurations.contains("RoiH")) {
           m_option->m_iRoiH = configurations["RoiH"].toInt();
        }
        if (configurations.contains("Blc")) {
            m_option->m_iBlc = configurations["Blc"].toInt();
        }
        if (configurations.contains("StartX")) {
           m_option->m_iStartX = configurations["StartX"].toInt();
        }
        if (configurations.contains("StartY")) {
           m_option->m_iStartY = configurations["StartY"].toInt();
        }
        if (configurations.contains("AETimes")) {
           m_option->m_iAETimes = configurations["AETimes"].toInt();
        }
        if (configurations.contains("AEInfiVal")) {
            m_option->m_iAEInfiVal = configurations["AEInfiVal"].toInt();
        }
        if (configurations.contains("AERememberVal")) {
            m_option->m_iAERememberVal = configurations["AERememberVal"].toInt();
        }
        if (configurations.contains("RememberAE")) {
            m_option->m_bRememberAE = configurations["RememberAE"].toInt();
        }
        if (configurations.contains("AEFrame")) {
            m_option->m_iAEFrame = configurations["AEFrame"].toInt();
        }
        if (configurations.contains("WaitTime")) {
            m_option->m_iWaitTime = configurations["WaitTime"].toInt();
        }
        if (configurations.contains("AnalogGain")) {
            m_option->m_iAnalogGain = configurations["AnalogGain"].toInt();
        }
    }

    return m_wdtConf->Cache2Ui();
}

int AETestItem::SaveOption()
{
    if ((m_context != nullptr) && (m_context->ChannelController == nullptr))
        return ERR_Failed;

    int ec = m_wdtConf->Ui2Cache();
    if (ERR_NoError == ec)
    {
        QMap<QString, QString> configurations;
        configurations["10BitCal"] = QString::number(m_option->m_b10BitCal ? 1 : 0);
        configurations["SensorType"] = QString::number(m_option->m_iSensorType);
        configurations["RefMode"] = QString::number(m_option->m_iRefMode);
        configurations["AECalMode"] = QString::number(m_option->m_iAECalMode);
        configurations["AEMinVal"] = QString::number(m_option->m_iAEMinVal);
        configurations["AEMaxVal"] = QString::number(m_option->m_iAEMaxVal);
        configurations["RoiW"] = QString::number(m_option->m_iRoiW);
        configurations["ROIH"] = QString::number(m_option->m_iRoiH);
        configurations["Blc"] = QString::number(m_option->m_iBlc);
        configurations["StartX"] = QString::number(m_option->m_iStartX);
        configurations["StartY"] = QString::number(m_option->m_iStartY);
        configurations["AETimes"] = QString::number(m_option->m_iAETimes);
        configurations["AEInfiVal"] = QString::number(m_option->m_iAEInfiVal);
        configurations["AERememberVal"] = QString::number(m_option->m_iAERememberVal);
        configurations["RememberAE"] = QString::number(m_option->m_bRememberAE ? 1 : 0);
        configurations["AEFrame"] = QString::number(m_option->m_iAEFrame);
        configurations["WaitTime"] = QString::number(m_option->m_iWaitTime);
        configurations["AnalogGain"] = QString::number(m_option->m_iAnalogGain);
        // save to file
        ec = m_context->ModuleSettings->WriteSection(m_instanceName, configurations);
    }

    return ec;
}

int AETestItem::RestoreDefaults()
{
    return m_wdtConf->RestoreDefaults();
}

int AETestItem::BindChannelContext(T_ChannelContext *context)
{
    m_context = context;
    if ((nullptr == m_context) || (nullptr == m_context->ChannelController) || (nullptr == m_context->ImageSensor))
        return ERR_Failed;
    LoadOption();
    return ERR_NoError;
}

ITestItem::E_ItemType AETestItem::GetItemType() const
{
    return ITestItem::ItemType_ImageEvaluation;
}

bool AETestItem::GetIsSynchronous() const
{
    return true;
}

QString AETestItem::GetReportHeader() const
{
    QString strHeader = "Brightness,Exposure Time";

    return strHeader;
}

QString AETestItem::GetReportContent() const
{
    QString strContent = "";
    uint uExp = 0;
    if(ERR_NoError == m_context->ImageSensor->GetSensorExposure(uExp)) {
        strContent = QString::asprintf("%0.2f,0x%X", m_fBrightness, uExp);
    }
    else {
        strContent = QString::asprintf("%0.2f,NG", m_fBrightness);
    }
    return strContent;
}

bool AETestItem::GetIsEngineeringMode() const
{
    return false;
}

int AETestItem::Initialize(const T_FrameParam &frameParam)
{
    if (m_initialized) {
        return ERR_NoError;
    }
    m_fBrightness = 0.0;
    m_FrameParam = frameParam;
    int iStartX = 0, iStartY = 0;
    int iEndX   = 0, iEndY   = 0;
    CalROIPos(iStartX, iStartY, iEndX, iEndY);
    m_roiInfo.X = iStartX;
    m_roiInfo.Y = iStartY;
    m_roiInfo.Width = iEndX - iStartX + 1;
    m_roiInfo.Height = iEndY - iStartY + 1;

    if(m_option->m_bRememberAE)
    {
        qDebug() << "Remember AE Val: " << m_option->m_iAERememberVal;
        if (ERR_NoError != m_context->ImageSensor->SetSensorExposure((uint)m_option->m_iAERememberVal))
        {
            m_context->ChannelController->LogToWindow("Set Remember AE Val register failed.", qRgb(255, 0, 0));
            return ERR_Failed;
        }
    }
    else
    {
        qDebug() << "InfiVal: " << m_option->m_iAEInfiVal;
        if (ERR_NoError != m_context->ImageSensor->SetSensorExposure((uint)m_option->m_iAEInfiVal))
        {
            m_context->ChannelController->LogToWindow("Set initial AE Val register failed.", qRgb(255, 0, 0));
            return ERR_Failed;
        }
    }
    if (ERR_NoError != m_context->ImageSensor->SetSensorGain((uint)pow(2, m_option->m_iAnalogGain), true))
    {
        m_context->ChannelController->LogToWindow("Set Sensor Gain Reg Failed.", qRgb(255, 0, 0));
        return ERR_Failed;
    }
    m_initialized = true;

    return ERR_NoError;
}

int AETestItem::Evaluate(const uchar *image, uint64 &timestamp, std::vector<T_RoiInfo> &roiInfos)
{
    roiInfos.clear();
    if (0 == m_u64ExpGainTimeStamp) {
        m_u64ExpGainTimeStamp = m_context->ChannelController->GetCurrentTimestamp();
    }
    uint64 u64Diff = 0;
    if (timestamp > m_u64ExpGainTimeStamp) {
        u64Diff = (timestamp - m_u64ExpGainTimeStamp) / 1000;
    }
    if (m_option->m_iWaitTime > (int)u64Diff) {
        return ERR_Continue;
    }

    m_fBrightness = 0;
    float fFactor = 1.0f;
    QString strLogInfo = "";
    int i32Target = (m_option->m_iAEMaxVal + m_option->m_iAEMinVal) / 2;
    CenterBlockRawInfo(image, m_fBrightness);
    strLogInfo.sprintf("%d: brightness=%f <%d , %d>", m_u32Count, m_fBrightness, m_option->m_iAEMinVal, m_option->m_iAEMaxVal);
    m_context->ChannelController->LogToWindow(strLogInfo, qRgb(0, 0, 255));
    m_roiInfo.MarkerColor = qRgb(0, 0, 255);
    m_roiInfo.LabelColor = qRgb(0, 0, 255);
    m_roiInfo.Label = strLogInfo.toStdString();
    m_roiInfo.LineWidth = 3;
    roiInfos.push_back(m_roiInfo);

    if ((m_fBrightness < m_option->m_iAEMinVal) || (m_fBrightness > m_option->m_iAEMaxVal))
    {
        if (m_fBrightness > m_option->m_iBlc)
        {
            fFactor = (1.0 * (i32Target-m_option->m_iBlc)) / (m_fBrightness-m_option->m_iBlc);
            if (m_option->m_b10BitCal)
            {
                if(m_fBrightness >= 1023)//Raw10
                {
                    fFactor = fFactor / 2;
                }
            }
            else
            {
                if(m_fBrightness >= 255)//Raw8
                {
                    fFactor = fFactor / 2;
                }
            }
        }
        else if (0 != m_fBrightness)
        {
            fFactor = (1.0*i32Target) / m_fBrightness;
        }
        else
        {
            fFactor = (float)i32Target;
        }
    }
    else
    {
        uint uExp = 0;
        if(ERR_NoError != m_context->ImageSensor->GetSensorExposure(uExp))
        {
            m_context->ChannelController->LogToWindow("Get Sensor Gain Reg Failed.", LogRed);
            return ERR_Failed;
        }
        if(true == m_option->m_bRememberAE)
        {
            m_option->m_iAERememberVal = (int) ((uExp == 0)? 0x0180 : uExp);
            m_context->ModuleSettings->WriteInteger(m_instanceName, "AERememberVal", m_option->m_iAERememberVal);
        }

        qDebug() << "Auto expoure Success!";
        m_context->ChannelController->LogToWindow(QString("Auto expoure Success![Exposure: %1]").arg(uExp), qRgb(0, 0, 255));
        m_u32Count = 0;
        m_u64ExpGainTimeStamp = 0;
        m_initialized = false;
        return ERR_NoError;
    }

    m_u32Count++;
    if (m_u32Count > (uint)m_option->m_iAETimes)
    {
        m_u32Count = 0;
        m_u64ExpGainTimeStamp = 0;
        m_initialized = false;
        qDebug() << "Auto expoure fail!";
        m_context->ChannelController->LogToWindow("Auto expoure fail!", qRgb(255, 0, 0));
        return ERR_Failed;
    }

    uint uExp = 0;
    if(ERR_NoError != m_context->ImageSensor->GetSensorExposure(uExp))
    {
        m_context->ChannelController->LogToWindow("Get Sensor Gain Reg Failed.", LogRed);
        return ERR_Failed;
    }
    if(ERR_NoError != m_context->ImageSensor->SetSensorExposure((uint)(uExp*fFactor)))
    {
        m_context->ChannelController->LogToWindow("Set Sensor Gain Reg Failed.", LogRed);
        return ERR_Failed;
    }
    m_u64ExpGainTimeStamp = m_context->ChannelController->GetCurrentTimestamp();
    qDebug() << "Exp: " << uExp * fFactor;
    return ERR_Continue;
}

void AETestItem::CalROIPos(int& iStartX, int& iStartY, int& iEndX, int& iEndY)
{
    int iRoiW = m_option->m_iRoiW;
    int iRoiH = m_option->m_iRoiH;
    if ((iRoiW <= 100) || (iRoiH <= 100))
    {
        iRoiW = m_FrameParam.Width / m_option->m_iRoiW;
        iRoiH = m_FrameParam.Height / m_option->m_iRoiH;
    }

    iStartX = ((m_FrameParam.Width * m_option->m_iStartX / 100) - (iRoiW / 2)) / 4 * 4;
    iStartY = ((m_FrameParam.Height * m_option->m_iStartY / 100) - (iRoiH / 2)) / 4 * 4;
    iEndX = (iStartX + iRoiW) / 4 * 4;
    iEndY = (iStartY + iRoiH) / 4 * 4;
}

void AETestItem::CalROIChlSum(const ushort* pRawBuffer, uint* pSumR, uint* pSumGr, uint* pSumGb, uint* pSumB, int iCol, int iRow)
{
    int Pos1 = iCol + m_FrameParam.Width * iRow;
    int Pos2 = iCol + m_FrameParam.Width * (iRow + 1);
    int Pos3 = iCol + m_FrameParam.Width * (iRow + 2);
    int Pos4 = iCol + m_FrameParam.Width * (iRow + 3);

    int bit = (true == m_option->m_b10BitCal) ? 0 : 2;
    if (Option::FOURCELL == m_option->m_iSensorType)
    {
        *pSumR  += ((pRawBuffer[Pos1]>>bit) + (pRawBuffer[Pos1+1]>>bit) + (pRawBuffer[Pos2]>>bit) + (pRawBuffer[Pos2+1]>>bit)) / 4;
        *pSumGr += ((pRawBuffer[Pos1+2]>>bit) + (pRawBuffer[Pos1+3]>>bit) + (pRawBuffer[Pos2+2]>>bit) + (pRawBuffer[Pos2+3]>>bit)) / 4;
        *pSumGb += ((pRawBuffer[Pos3]>>bit) + (pRawBuffer[Pos3+1]>>bit) + (pRawBuffer[Pos4]>>bit) + (pRawBuffer[Pos4+1]>>bit)) / 4;
        *pSumB  += ((pRawBuffer[Pos3+2]>>bit) + (pRawBuffer[Pos3+3]>>bit) + (pRawBuffer[Pos4+2]>>bit) + (pRawBuffer[Pos4+3]>>bit)) / 4;
    }
    else if (Option::TWOPD == m_option->m_iSensorType)
    {
        *pSumR  += ((pRawBuffer[Pos1] >> bit) + (pRawBuffer[Pos1+1] >> bit)) / 2;
        *pSumGr += ((pRawBuffer[Pos1+2] >> bit) + (pRawBuffer[Pos1+3] >> bit)) / 2;
        *pSumGb += ((pRawBuffer[Pos2] >> bit) + (pRawBuffer[Pos2+1] >> bit)) / 2;
        *pSumB  += ((pRawBuffer[Pos2+2] >> bit) + (pRawBuffer[Pos2+3] >> bit)) / 2;
    }
    else
    {
        *pSumR  += (pRawBuffer[Pos1] >> bit);
        *pSumGr += (pRawBuffer[Pos1 + 1] >> bit);
        *pSumGb += (pRawBuffer[Pos2] >> bit);
        *pSumB  += (pRawBuffer[Pos2 + 1] >> bit);
    }
}

void AETestItem::CenterBlockRawInfo(const uchar* pImg, float& fAvgVal)
{
    ushort* pRawBuffer = (ushort*)pImg;
    uint  Count = 0;
    uint SumR = 0, SumGr = 0, SumGb = 0, SumB = 0;
    uint *pSumR = NULL, *pSumGr = NULL, *pSumGb = NULL, *pSumB = NULL;
    float	R = 0.0, Gr = 0.0, Gb = 0.0, B = 0.0;
    if (IMAGE_MODE_YCbYCr_RG_GB == m_FrameParam.ImageMode)      //RGGB
    {
        pSumR = &SumR;
        pSumGr = &SumGr;
        pSumGb = &SumGb;
        pSumB = &SumB;
    }
    else if (IMAGE_MODE_YCrYCb_GR_BG == m_FrameParam.ImageMode) //GRBG
    {
        pSumR = &SumGr;
        pSumGr = &SumR;
        pSumGb = &SumB;
        pSumB = &SumGb;
    }
    else if (IMAGE_MODE_CbYCrY_GB_RG == m_FrameParam.ImageMode) //GBRG
    {
        pSumR = &SumGb;
        pSumGr = &SumB;
        pSumGb = &SumR;
        pSumB = &SumGr;
    }
    else if (IMAGE_MODE_CrYCbY_BG_GR == m_FrameParam.ImageMode)//BGGR
    {
        pSumR = &SumB;
        pSumGr = &SumGb;
        pSumGb = &SumGr;
        pSumB = &SumR;
    }

    int iStartX = 0, iStartY = 0;
    int iEndX   = 0, iEndY   = 0;
    int iRowBase = 2, iColBase = 2;
    if (Option::TWOPD == m_option->m_iSensorType)
    {
        iColBase = 4;
    }
    else if (Option::FOURCELL == m_option->m_iSensorType)
    {
        iRowBase = 4;
        iColBase = 4;
    }

    CalROIPos(iStartX, iStartY, iEndX, iEndY);
    if (Option::ONE28PIXEL != m_option->m_iRefMode) //只与128固定值比较情况下，不需要计算，提升效率
    {
        for (int iRow = iStartY; iRow < iEndY; iRow+=iRowBase)
        {
            for (int iCol = iStartX; iCol < iEndX; iCol+=iColBase)
            {
                CalROIChlSum(pRawBuffer, pSumR, pSumGr, pSumGb, pSumB, iCol, iRow);
                Count += 1;
            }
        }

        Gr = (1.0*SumGr) / Count;
        Gb = (1.0*SumGb) / Count;
        R  = (1.0*SumR)  / Count;
        B  = (1.0*SumB)  / Count;
    }

    float RawAvgVal[Option::MAXTARGET] = {
                                           R,
                                           Gr,
                                           Gb,
                                           B,
                                           (float)(R * 299 + ((Gr + Gb) / 2 + 0.5) * 587 + B * 114) / 1000,
                                           (float)(Gr + Gb) / 2.0f,
                                         };

    fAvgVal = RawAvgVal[m_option->m_iAECalMode];
    qDebug() << "测试所有pixel平均值...";
    if (Option::CHLPIXEL != m_option->m_iRefMode)
    {
        float RefVal = fAvgVal;
        if (Option::ONE28PIXEL == m_option->m_iRefMode)
        {
            RefVal = 128.0f;
        }

        Count = 0;
        SumR = SumGr = SumGb = SumB = 0;
        uint BakSumR = 0, BakSumGr = 0, BakSumGb = 0, BakSumB = 0;
        for (int iRow = iStartY; iRow < iEndY; iRow+=iRowBase)
        {
            for (int iCol = iStartX; iCol < iEndX; iCol+=iColBase)
            {
                BakSumR  = SumR;
                BakSumGr = SumGr;
                BakSumGb = SumGb;
                BakSumB  = SumB;
                CalROIChlSum(pRawBuffer, pSumR, pSumGr, pSumGb, pSumB, iCol, iRow);
                if ((((SumGr-BakSumGr) + (SumGb-BakSumGb))/2.0) <= RefVal)
                {
                    SumR  = BakSumR;
                    SumGr = BakSumGr;
                    SumGb = BakSumGb;
                    SumB  = BakSumB;
                }
                else
                {
                    Count += 1;
                }
            }
        }

        if(0 == Count)
        {
            Count = 1;
        }

        Gr = 1.0 * SumGr / Count;
        Gb = 1.0 * SumGb / Count;
        fAvgVal  = (Gr + Gb) / 2.0;

        QString strLogInfo = "";
        strLogInfo.sprintf("测试大于%0.2f的pixel...", RefVal);
        qDebug() << strLogInfo;
        m_context->ChannelController->LogToWindow(strLogInfo, qRgb(0, 0, 0x87));
    }
}
