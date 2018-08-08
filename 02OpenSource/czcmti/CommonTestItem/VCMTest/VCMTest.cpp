#include "VCMTest.h"
#include <QDebug>
#include <QThread>
#include "IVcmDriver.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

CZPLUGIN_API ICzPlugin *CreatePlugin(void *arg)
{
    return new VCMTest((const char *)arg);
}

CZPLUGIN_API void DestroyPlugin(ICzPlugin **plugin)
{
    VCMTest *pTestItem = (VCMTest *)(*plugin);
    if (pTestItem != nullptr) {
        delete pTestItem; pTestItem = nullptr;
        *plugin = nullptr;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

VCMTest::VCMTest(const char *instanceName)
{
    m_instanceName = QString::fromLatin1(instanceName);
    m_initialized = true;
    m_option = new Option();
    m_wdtConf = new ConfWidget(m_option);
    m_context = nullptr;
    m_i32Count = 0;
    m_u64VcmCodeTimeStamp = 0;
    m_uLastCode = 0;
}

VCMTest::~VCMTest()
{
    delete m_wdtConf;
    delete m_option;
}

int VCMTest::GetPluginInfo(T_PluginInfo &pluginInfo)
{
    pluginInfo.Version = 0x09000003;
    strcpy(pluginInfo.Description, "VCMTest.");
    strcpy(pluginInfo.FriendlyName, "VCMTest");
    strcpy(pluginInfo.InstanceName, m_instanceName.toLatin1().constData());
    strcpy(pluginInfo.VendorName, "CZTEK");
    pluginInfo.OptionDlgHandle = (uint64)m_wdtConf;

    return 0;
}

int VCMTest::LoadOption()
{
    if ((m_context == nullptr) || (m_context->ChannelController == nullptr))
        return ERR_Failed;
    QMap<QString, QString> configurations;
    m_context->ModuleSettings->ReadSection(m_instanceName, configurations);
    if (configurations.contains("ROIH")) {
        m_option->m_stShading.u32RoiH = configurations["ROIH"].toUInt();
    }
    if (configurations.contains("ROIW")) {
        m_option->m_stShading.u32RoiW = configurations["ROIW"].toUInt();
    }
    if (configurations.contains("BLC")) {
        m_option->m_stShading.u32Blc = configurations["BLC"].toUInt();
    }
    if (configurations.contains("VCMDelay")) {
        m_option->m_u32VcmDelay = configurations["VCMDelay"].toUInt();
    }
    if (configurations.contains("Y1Min")) {
        m_option->m_u32Y1Min = configurations["Y1Min"].toUInt();
    }
    if (configurations.contains("Y1Max")) {
        m_option->m_u32Y1Max = configurations["Y1Max"].toUInt();
    }
    if (configurations.contains("Y2Min")) {
        m_option->m_u32Y2Min = configurations["Y2Min"].toUInt();
    }
    if (configurations.contains("Y2Max")) {
        m_option->m_u32Y2Max = configurations["Y2Max"].toUInt();
    }
    if (configurations.contains("Y3Min")) {
        m_option->m_u32Y3Min = configurations["Y3Min"].toUInt();
    }
    if (configurations.contains("Y3Max")) {
        m_option->m_u32Y3Max = configurations["Y3Max"].toUInt();
    }
    if (configurations.contains("Y4Min")) {
        m_option->m_u32Y4Min = configurations["Y4Min"].toUInt();
    }
    if (configurations.contains("Y4Max")) {
        m_option->m_u32Y4Max = configurations["Y4Max"].toUInt();
    }

    QString strKey;
    for (int i=0; i<5; ++i)
    {
        strKey.sprintf("Roi%dX", i);
        if (configurations.contains(strKey)) {
            m_option->m_stShading.Roi[i].cx = configurations[strKey].toInt();
        }

        strKey.sprintf("Roi%dY", i);
        if (configurations.contains(strKey)) {
            m_option->m_stShading.Roi[i].cy = configurations[strKey].toInt();
        }
    }

    return m_wdtConf->Cache2Ui();
}

int VCMTest::SaveOption()
{
    if ((m_context == nullptr) || (m_context->ChannelController == nullptr))
        return ERR_Failed;

    int ec = m_wdtConf->Ui2Cache();
    if (ERR_NoError == ec)
    {
        QMap<QString, QString> configurations;
        configurations["ROIH"] = QString::number(m_option->m_stShading.u32RoiH);
        configurations["ROIW"] = QString::number(m_option->m_stShading.u32RoiH);
        configurations["BLC"] = QString::number(m_option->m_stShading.u32Blc);
        configurations["VCMDelay"] = QString::number(m_option->m_u32VcmDelay);
        configurations["Y1Min"] = QString::number(m_option->m_u32Y1Min);
        configurations["Y1Max"] = QString::number(m_option->m_u32Y1Max);
        configurations["Y2Min"] = QString::number(m_option->m_u32Y2Min);
        configurations["Y2Max"] = QString::number(m_option->m_u32Y2Max);
        configurations["Y3Min"] = QString::number(m_option->m_u32Y3Min);
        configurations["Y3Max"] = QString::number(m_option->m_u32Y3Max);
        configurations["Y4Min"] = QString::number(m_option->m_u32Y4Min);
        configurations["Y4Max"] = QString::number(m_option->m_u32Y4Max);

        QString strKey;
        for (int i = 0; i < 5; i++) {
            strKey.sprintf("Roi%dX", i);
            configurations[strKey] = QString::number(m_option->m_stShading.Roi[i].cx);
            strKey.sprintf("Roi%dY", i);
            configurations[strKey] = QString::number(m_option->m_stShading.Roi[i].cy);
        }

        // save to file
        ec = m_context->ModuleSettings->WriteSection(m_instanceName, configurations);
    }

    return ec;
}

int VCMTest::RestoreDefaults()
{
    qDebug();
    return m_wdtConf->RestoreDefaults();
}

int VCMTest::BindChannelContext(T_ChannelContext *context)
{
    m_context = context;
    LoadOption();
    return ERR_NoError;
}

ITestItem::E_ItemType VCMTest::GetItemType() const
{
    return ITestItem::ItemType_ImageEvaluation;
}

bool VCMTest::GetIsSynchronous() const
{
    return true;
}

QString VCMTest::GetReportHeader() const
{
    QString strHeader = "";
    for (int i=0; i<4; ++i)
    {
        strHeader.sprintf("VcmDiff%d,YDiffMin%d,YDiffMax%d,", i, i, i);
    }

    return strHeader;
}

QString VCMTest::GetReportContent() const
{
    QString strContent = "";
    uint YDiffMax[] = {m_option->m_u32Y1Max, m_option->m_u32Y2Max, m_option->m_u32Y3Max, m_option->m_u32Y4Max};
    uint YDiffMin[] = {m_option->m_u32Y1Min, m_option->m_u32Y2Min, m_option->m_u32Y3Min, m_option->m_u32Y4Min};
    for (int i=0; i<4; ++i)
    {
         strContent.sprintf("%0.2f,%d,%d,", m_fVcmYDiff[i], YDiffMin[i], YDiffMax[i]);
    }

    return strContent;
}

bool VCMTest::GetIsEngineeringMode() const
{
    return false;
}

int VCMTest::Initialize(const T_FrameParam &frameParam)
{
    m_FrameParam = frameParam;
    CalcImageRect();
    Option::SHADING& sShading = m_option->m_stShading;
    for (int i=0; i<5; i++)
    {
        m_roiInfos[i].X = sShading.Roi[i].pos.left();
        m_roiInfos[i].Y = sShading.Roi[i].pos.top();
        m_roiInfos[i].Width = sShading.Roi[i].pos.right() - sShading.Roi[i].pos.left() + 1;
        m_roiInfos[i].Height = sShading.Roi[i].pos.bottom() - sShading.Roi[i].pos.top() + 1;
    }
    for (int i=0; i<4; ++i)
    {
          m_fVcmYDiff[i] = 0;
    }
    return ERR_NoError;
}

int VCMTest::Evaluate(const uchar *image, uint64 &timestamp, std::vector<T_RoiInfo> &roiInfos)
{
    roiInfos.resize(5);
    for (uint i = 0; i < 5; i++)
        roiInfos[i] = m_roiInfos[i];
    if (0 == m_i32Count)
    {
        //下发100code值
        m_context->VcmDriver->VcmReadCode(m_uLastCode);
        m_context->VcmDriver->VcmWriteCode(100);
        m_i32Count += 1;
        return ERR_Continue;
    }

    uint timeDiff_ms = 0;
    if (0 == m_u64VcmCodeTimeStamp) {
        m_u64VcmCodeTimeStamp = m_context->ChannelController->GetCurrentTimestamp();
    }
    if (timestamp > m_u64VcmCodeTimeStamp) {
        timeDiff_ms = (uint)((timestamp - m_u64VcmCodeTimeStamp) / 1000);
    }
    if (m_option->m_u32VcmDelay > timeDiff_ms) {
        return ERR_Continue;
    }

    QString strLogInfo = "";
    //计算结果
    for(int i=0; i<5; i++)
    {
        GetAWBInfoRaw8(i, image);
    }

    Option::SHADING& sShading = m_option->m_stShading;
    if (1 == m_i32Count)
    {
        for (int i=1; i<5; ++i)
        {
            m_fLastY[i-1] = sShading.Roi[i].avgY;
        }

        //下发1000code值
        m_context->VcmDriver->VcmWriteCode(1000);
        m_u64VcmCodeTimeStamp = m_context->ChannelController->GetCurrentTimestamp();
        m_i32Count += 1;
        return ERR_Continue;
    }

    uint YDiffMax[] = {m_option->m_u32Y1Max, m_option->m_u32Y2Max, m_option->m_u32Y3Max, m_option->m_u32Y4Max};
    uint YDiffMin[] = {m_option->m_u32Y1Min, m_option->m_u32Y2Min, m_option->m_u32Y3Min, m_option->m_u32Y4Min};
    if (2 == m_i32Count)
    {
        uint failCnt = 0;
        for (int i=1; i<5; i++)
        {
            m_fVcmYDiff[i-1] = sShading.Roi[i].avgY - m_fLastY[i-1];
            strLogInfo.sprintf("diff: %0.2f", m_fVcmYDiff[i-1]);
            m_roiInfos[i].Label = strLogInfo.toStdString();
            m_roiInfos[i].MarkerColor = qRgb(0, 0, 0x80);
            if ((m_fVcmYDiff[i-1] > YDiffMax[i-1])
                        || (m_fVcmYDiff[i-1] < YDiffMin[i-1]))
            {
                m_roiInfos[i].MarkerColor = qRgb(255, 0, 0);
                strLogInfo.sprintf("VCMDiff = %.1f not in <%d, %d>", m_fVcmYDiff[i-1], YDiffMin[i-1], YDiffMax[i-1]);
                m_context->ChannelController->LogToWindow(strLogInfo, qRgb(255, 0, 0));
                ++failCnt;
            }
        }

        m_initialized = true;
        m_u64VcmCodeTimeStamp = 0;
        m_i32Count = 0;
        m_context->VcmDriver->VcmWriteCode(m_uLastCode);
        if (0 != failCnt)
        {
            strLogInfo.sprintf("VCM Test Failed.");
            m_context->ChannelController->LogToWindow(strLogInfo, qRgb(255, 0, 0));
            return ERR_Failed;
        }
        else
        {
            strLogInfo.sprintf("VCM Test Pass.");
            m_context->ChannelController->LogToWindow(strLogInfo, qRgb(0, 0, 255));
            return ERR_NoError;
        }
    }

    return ERR_NoError;
}

void VCMTest::CalcImageRect(void)
{
    int width  = m_FrameParam.Width;
    int height = m_FrameParam.Height;
    for(int i=0; i<5; i++)
    {
        //比例转化为实际像素点
        Option::SHADING& sShading = m_option->m_stShading;
        sShading.Roi[i].pos.setTop((int)((sShading.Roi[i].cy - sShading.u32RoiH/2.0)  * height/100));
        sShading.Roi[i].pos.setBottom((int)((sShading.Roi[i].cy + sShading.u32RoiH/2.0)  * height/100));
        sShading.Roi[i].pos.setLeft((int)((sShading.Roi[i].cx - sShading.u32RoiW/2.0)   * width/100));
        sShading.Roi[i].pos.setRight((int)((sShading.Roi[i].cx + sShading.u32RoiW/2.0)   * width/100));
        sShading.Roi[i].pos.setTop((sShading.Roi[i].pos.top()/4) * 4);
        sShading.Roi[i].pos.setBottom((sShading.Roi[i].pos.bottom()/4) * 4);
        sShading.Roi[i].pos.setLeft((sShading.Roi[i].pos.left()/4) * 4);
        sShading.Roi[i].pos.setRight((sShading.Roi[i].pos.right()/4) * 4);

        if(sShading.Roi[i].pos.left() < 0)
        {
            sShading.Roi[i].pos.setLeft(0);
        }

        if(sShading.Roi[i].pos.right() >= width)
        {
            sShading.Roi[i].pos.setRight(width - 1);
        }

        if(sShading.Roi[i].pos.top() < 0)
        {
            sShading.Roi[i].pos.setTop(0);
        }

        if(sShading.Roi[i].pos.bottom() >= height)
        {
            sShading.Roi[i].pos.setBottom(height - 1);
        }
    }
}

void VCMTest::GetAWBInfoRaw8(int block, const uchar *image)
{
    Option::SHADING& sShading = m_option->m_stShading;
    int  StartX = sShading.Roi[block].pos.left();
    int  StartY = sShading.Roi[block].pos.top();
    int  EndX   = sShading.Roi[block].pos.right();
    int  EndY   = sShading.Roi[block].pos.bottom();

    const ushort* pRawBuffer = (const ushort*)image;
    uint Count = 0;
    uint SumR = 0, SumGr = 0, SumGb = 0, SumB = 0;
    uint *pSumR = NULL, *pSumGr = NULL, *pSumGb = NULL, *pSumB = NULL;
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

    for (int iRow = StartY; iRow < EndY; iRow+=2)
    {
        for (int iCol = StartX; iCol < EndX; iCol+=2)
        {
            uint Pos1 = iCol + m_FrameParam.Width * iRow;
            uint Pos2 = iCol + m_FrameParam.Width * (iRow + 1);
            *pSumR  += (pRawBuffer[Pos1] >> 2);
            *pSumGr += (pRawBuffer[Pos1 + 1] >> 2);
            *pSumGb += (pRawBuffer[Pos2] >> 2);
            *pSumB  += (pRawBuffer[Pos2 + 1] >> 2);
            Count += 1;
        }
    }

    sShading.Roi[block].avgR  = (1.0 * SumR)  / Count;
    sShading.Roi[block].avgB  = (1.0 * SumB)  / Count;
    sShading.Roi[block].avgGr = (1.0 * SumGr) / Count;
    sShading.Roi[block].avgGb = (1.0 * SumGb) / Count;
    sShading.Roi[block].avgG  = (sShading.Roi[block].avgGr + sShading.Roi[block].avgGb) / 2.0;
    sShading.Roi[block].RGain = 512.0*(sShading.Roi[block].avgR-sShading.u32Blc)/(sShading.Roi[block].avgG-sShading.u32Blc) + 0.5;// Rave/Gave
    sShading.Roi[block].BGain = 512.0*(sShading.Roi[block].avgB-sShading.u32Blc)/(sShading.Roi[block].avgG-sShading.u32Blc) + 0.5;  //Bave/Gave
    sShading.Roi[block].avgY  = 0.299 *sShading.Roi[block].avgR + 0.587*sShading.Roi[block].avgG + 0.114*sShading.Roi[block].avgB;
}
