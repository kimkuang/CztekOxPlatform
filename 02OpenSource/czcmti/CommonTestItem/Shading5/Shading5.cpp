#include "Shading5.h"
#include <QDebug>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

CZPLUGIN_API ICzPlugin *CreatePlugin(void *arg)
{
    qDebug("Create Shading5 plugin ...");
    return new Shading5((const char *)arg);
}

CZPLUGIN_API void DestroyPlugin(ICzPlugin **plugin)
{
    qDebug("Destroy Shading5 plugin ...");
    Shading5 *pTestItem = (Shading5 *)(*plugin);
    if (pTestItem != nullptr) {
        delete pTestItem; pTestItem = nullptr;
        *plugin = nullptr;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#define RESULT_FAIL 1
#define RESULT_PASS 0

Shading5::Shading5(const char *instanceName)
            : m_instanceName(instanceName)
{
    m_initialized = false;
    m_option = new Option();
    m_wdtConf = new ConfWidget(m_option);
    m_context = nullptr;
}

Shading5::~Shading5()
{
    delete m_wdtConf;
    delete m_option;
}

int Shading5::GetPluginInfo(T_PluginInfo &pluginInfo)
{
    pluginInfo.Version = 0x09000002;
    strcpy(pluginInfo.Description, "Shading5 evaluation algorithm.");
    strcpy(pluginInfo.FriendlyName, "Shading5");
    strcpy(pluginInfo.InstanceName, m_instanceName.toLatin1().constData());
    strcpy(pluginInfo.VendorName, "CZTEK");
    pluginInfo.OptionDlgHandle = (uint64)m_wdtConf;

    return 0;
}

int Shading5::LoadOption()
{
    if ((m_context == nullptr) || (m_context->ChannelController == nullptr))
        return ERR_Failed;
    QMap<QString, QString> configurations;
    m_context->ModuleSettings->ReadSection(m_instanceName, configurations);
    QString strKey;
    for (int i = 0; i < 5; i++) {
        strKey.sprintf("ROI%d_x", i);
        if (configurations.contains(strKey))
            m_option->m_RoiConfs[i].XPercent = configurations[strKey].toUInt();

        strKey.sprintf("ROI%d_y", i);
        if (configurations.contains(strKey))
            m_option->m_RoiConfs[i].YPercent = configurations[strKey].toUInt();

        strKey.sprintf("ROI%d_En", i);
        if (configurations.contains(strKey))
            m_option->m_RoiConfs[i].Enable = (configurations[strKey].toUInt() == 1);
    }

    strKey = "ROI_height";
    if (configurations.contains(strKey))
        m_option->m_RoiHeightPercent = configurations[strKey].toInt();

    strKey = "ROI_width";
    if (configurations.contains(strKey))
        m_option->m_RoiWidthPercent = configurations[strKey].toInt();

    strKey = "Continue";
    if (configurations.contains(strKey))
        m_option->m_Continue = (configurations[strKey] == "true");

    strKey = "BLC";
    if (configurations.contains(strKey))
        m_option->m_BLC = configurations[strKey].toInt();

    strKey = "m_RGMax";
    if (configurations.contains(strKey))
        m_option->m_RGMax = configurations[strKey].toInt();

    strKey = "m_RGMin";
    if (configurations.contains(strKey))
        m_option->m_RGMin = configurations[strKey].toInt();

    strKey = "m_BGMax";
    if (configurations.contains(strKey))
        m_option->m_BGMax = configurations[strKey].toInt();

    strKey = "m_BGMin";
    if (configurations.contains(strKey))
        m_option->m_BGMin = configurations[strKey].toInt();

    strKey = "m_YMax";
    if (configurations.contains(strKey))
        m_option->m_YMax = configurations[strKey].toInt();

    strKey = "m_YMin";
    if (configurations.contains(strKey))
        m_option->m_YMin = configurations[strKey].toInt();

    strKey = "m_YDiff";
    if (configurations.contains(strKey))
        m_option->m_YDiff = configurations[strKey].toInt();

    return m_wdtConf->Cache2Ui();
}

int Shading5::SaveOption()
{
    if ((m_context == nullptr) || (m_context->ChannelController == nullptr))
        return ERR_Failed;
    int ec = m_wdtConf->Ui2Cache();
    if (ec == 0) {
        QMap<QString, QString> configurations;
        for (int i = 0; i < 5; i++) {
            QString strKey;
            strKey.sprintf("ROI%d_x", i);
            configurations[strKey] = QString::number(m_option->m_RoiConfs[i].XPercent);

            strKey.sprintf("ROI%d_y", i);
            configurations[strKey] = QString::number(m_option->m_RoiConfs[i].YPercent);

            strKey.sprintf("ROI%d_En", i);
            configurations[strKey] = QString::number(m_option->m_RoiConfs[i].Enable);
        }

        configurations["ROI_height"] = QString::number(m_option->m_RoiHeightPercent);
        configurations["ROI_width"] =  QString::number(m_option->m_RoiWidthPercent);
        configurations["Continue"] =  m_option->m_Continue ? "true" : "false";
        configurations["BLC"] =  QString::number(m_option->m_BLC);
        configurations["m_RGMax"] =  QString::number(m_option->m_RGMax);
        configurations["m_RGMin"] =  QString::number(m_option->m_RGMin);
        configurations["m_BGMax"] =  QString::number(m_option->m_BGMax);
        configurations["m_BGMin"] =  QString::number(m_option->m_BGMin);
        configurations["m_YMax"] =  QString::number(m_option->m_YMax);
        configurations["m_YMin"] =  QString::number(m_option->m_YMin);
        configurations["m_YDiff"] =  QString::number(m_option->m_YDiff);

        // save to file
        ec = m_context->ModuleSettings->WriteSection(m_instanceName, configurations);
    }
    return ec;
}

int Shading5::RestoreDefaults()
{
    return m_wdtConf->RestoreDefaults();
}

int Shading5::BindChannelContext(T_ChannelContext *context)
{
    m_context = context;
    LoadOption();
    return ERR_NoError;
}

ITestItem::E_ItemType Shading5::GetItemType() const
{
    return ITestItem::ItemType_ImageEvaluation;
}

bool Shading5::GetIsSynchronous() const
{
    return true;
}

QString Shading5::GetReportHeader() const
{
    QString strTemp = "";
    QString strHeader = "";
    strHeader += "Y_Blk0,Y_Blk1,Y_Blk2,Y_Blk3,Y_Blk4,";
    for (int col = 0; col < 5; ++col)
    {
        strTemp.sprintf("B_%d-R/G,B_%d-B/G,", col, col);
        strHeader += strTemp;
    }

    for (int col = 0; col < 5; ++col)
    {
        strTemp.sprintf("B_%d-R,B_%d-Gr,B_%d-Gb,B_%d-B,", col, col, col, col);
        strHeader += strTemp;
    }

    strHeader += "Center_R/G,Center_B/G,TL_R/G,TL_B/G,BL_R/G,BL_B/G,TR_R/G,TR_B/G,BR_R/G,BR_B/G";
    return strHeader;
}

QString Shading5::GetReportContent() const
{
    QString strContent= "";
    QString strTemp = "";
    for (int i = 0; i < 5; i++)
    {
        strTemp.sprintf("%.4f,", Y[i]);
        strContent += strTemp;
    }

    for (int i = 0; i < 5; i++)
    {
        strTemp.sprintf("%.4f,%.4f,", ColorRGain[i], ColorBGain[i]);
        strContent += strTemp;
    }

    for (int i = 0; i < 5; i++)
    {
        strTemp.sprintf("%d,%d,%d,%d,", m_shading.ShadingInfo[i].aveR,
                        m_shading.ShadingInfo[i].aveGr,
                        m_shading.ShadingInfo[i].aveGb,
                        m_shading.ShadingInfo[i].aveB);
        strContent += strTemp;
    }

    for (int i = 0; i < 5; i++)
    {
        strTemp.sprintf("%.4f,%.4f", Rgain[i], Bgain[i]);
        strContent += strTemp;
    }

    return strContent;
}

bool Shading5::GetIsEngineeringMode() const
{
    return false;
}

int Shading5::Initialize(const T_FrameParam &frameParam)
{
    m_frameParam = frameParam;
    uint maxbufLen = 0;
    for (int i = 0; i < 5; i++) {
        calcPosition(i, frameParam, m_shading.ShadingInfo[i].ROI);
        if (m_shading.ShadingInfo[i].ROI.Height *  m_shading.ShadingInfo[i].ROI.Width > maxbufLen)
            maxbufLen = m_shading.ShadingInfo[i].ROI.Height * m_shading.ShadingInfo[i].ROI.Width;
    }
    for (int i = 0; i < 5; i++) {
        qDebug() << QString("block %1:").arg(i) <<
                    m_shading.ShadingInfo[i].ROI.X << m_shading.ShadingInfo[i].ROI.Y <<
                    m_shading.ShadingInfo[i].ROI.Width << m_shading.ShadingInfo[i].ROI.Height;
    }

    for (int i = 0; i < 5; i++)
    {
         Y[i] = 0.0;
         ColorRGain[i]  = 0.0;
         ColorBGain[i]  = 0.0;
         m_shading.ShadingInfo[i].aveR = 0;
         m_shading.ShadingInfo[i].aveGr = 0;
         m_shading.ShadingInfo[i].aveGb = 0;
         m_shading.ShadingInfo[i].aveB = 0;
         Rgain[i] = 0.0;
         Bgain[i] = 0.0;
    }

    return ERR_NoError;
}

int Shading5::Evaluate(const uchar *image, uint64 &timestamp, std::vector<T_RoiInfo> &roiInfos)
{
    (void)timestamp;
    roiInfos.resize(5);
    for (uint i = 0; i < 5; i++) {
        roiInfos[i] = m_shading.ShadingInfo[i].ROI;
        roiInfos[i].LabelColor = qRgb(0, 0, 255);
        roiInfos[i].LineWidth = 1;
        roiInfos[i].MarkerColor = qRgb(255, 0, 0);
    }
    for (uint i = 0; i < 5; i++) {
        getAWBInfoResult8bit(image, i);
    }
    // TODO LOG shanding 原始值
    QString strLogInfo = "";
    for (uint i = 0; i < 5; i++) {
        Rgain[i] = m_shading.ShadingInfo[i].RGain / 512.0;
        Bgain[i] = m_shading.ShadingInfo[i].BGain / 512.0;
        strLogInfo.sprintf("RGain_B%d=%d,BGain_B%d=%d", i, m_shading.ShadingInfo[i].RGain,
                           i, m_shading.ShadingInfo[i].BGain);
        qDebug() << strLogInfo;
        m_context->ChannelController->LogToWindow(strLogInfo, qRgb(0, 0x80, 0));
    }

    int CenterRG = m_shading.ShadingInfo[0].RGain;
    int CenterBG = m_shading.ShadingInfo[0].BGain;
    int CenterY = m_shading.ShadingInfo[0].aveY;
    if (0 == CenterRG || 0 == CenterBG || 0 == CenterY) {
        strLogInfo.sprintf("CenterRG[%d] or CenterBG[%d] or CenterY[%d] equal 0!!!",
                           CenterRG, CenterBG, CenterY);
        qCritical() << strLogInfo;
        m_context->ChannelController->LogToWindow(strLogInfo, qRgb(255, 0, 0));
        return ERR_Failed;
    }
    QString strLabel;
    for (uint i = 0; i < 5; i++) {
        ColorRGain[i] = m_shading.ShadingInfo[i].RGain * 1.0 / CenterRG;
        ColorBGain[i] = m_shading.ShadingInfo[i].BGain * 1.0 / CenterBG;
        Y[i] = m_shading.ShadingInfo[i].aveY * 1.0 / CenterY;
        m_shading.ShadingInfo[i].RGain = (int)(m_shading.ShadingInfo[i].RGain * 100.0 / CenterRG);
        m_shading.ShadingInfo[i].BGain = (int)(m_shading.ShadingInfo[i].BGain * 100.0 / CenterBG);
        m_shading.ShadingInfo[i].aveY = (int)(m_shading.ShadingInfo[i].aveY * 100.0 / CenterY);
        strLogInfo.sprintf("B%d: RG=%d, BG=%d, Y=%d", i, m_shading.ShadingInfo[i].RGain,
                         m_shading.ShadingInfo[i].BGain, m_shading.ShadingInfo[i].aveY);
        qDebug() << strLogInfo;
        m_context->ChannelController->LogToWindow(strLogInfo, qRgb(0, 0x80, 0));

        strLabel.sprintf("RG:%d BG:%d Y:%d", m_shading.ShadingInfo[i].RGain,
                         m_shading.ShadingInfo[i].BGain, m_shading.ShadingInfo[i].aveY);
        roiInfos[i].Label = strLabel.toStdString();
    }

    int centerR = m_shading.ShadingInfo[0].aveR;
    int centerGr = m_shading.ShadingInfo[0].aveGr;
    int centerGb = m_shading.ShadingInfo[0].aveGb;
    int centerB = m_shading.ShadingInfo[0].aveB;
    if (0 == centerR || 0 == centerGr || 0 == centerGb || 0 == centerB) {
        qCritical("centerR[%d] or centerGr[%d] or centerGb[%d] or centerB[%d] equal 0!!!",
                    centerR, centerGr, centerGb, centerB);
        return ERR_Failed;
    }
    for (uint i = 0 ; i < 5; i++) {
        m_shading.ShadingInfo[i].aveR = (int)(m_shading.ShadingInfo[i].aveR * 100.0 / centerR);
        m_shading.ShadingInfo[i].aveGr = (int)(m_shading.ShadingInfo[i].aveGr * 100.0 / centerGr);
        m_shading.ShadingInfo[i].aveGb = (int)(m_shading.ShadingInfo[i].aveGb * 100.0 / centerGb);
        m_shading.ShadingInfo[i].aveB = (int)(m_shading.ShadingInfo[i].aveB * 100.0 / centerB);
        m_shading.ShadingInfo[i].aveG = (m_shading.ShadingInfo[i].aveGr+m_shading.ShadingInfo[i].aveGb) /2 ;
        // TODO LOG
    }

    int RGMax = 0;
    int BGMax = 0;

    int RGMin = m_shading.ShadingInfo[0].RGain;
    int BGMin = m_shading.ShadingInfo[0].RGain;

    int blockRGMax = 0;
    int blockRGMin = 0;
    int blockBGMax = 0;
    int blockBGMin = 0;
    for(int i = 0; i < 5; i++) {
        if (m_shading.ShadingInfo[i].RGain > RGMax) {
            RGMax = m_shading.ShadingInfo[i].RGain;
            blockRGMax = i;
        }
        if (m_shading.ShadingInfo[i].RGain < RGMin) {
            RGMin = m_shading.ShadingInfo[i].RGain;
            blockRGMin = i;
        }
        if (m_shading.ShadingInfo[i].BGain > BGMax) {
            BGMax = m_shading.ShadingInfo[i].BGain;
            blockBGMax=i;
        }

        if (m_shading.ShadingInfo[i].BGain < BGMin) {
            BGMin = m_shading.ShadingInfo[i].BGain;
            blockBGMin=i;
        }
    }
    (void)(blockRGMax);
    (void)(blockRGMin);
    (void)(blockBGMax);
    (void)(blockBGMin);

#if 0 // TODO ui log
    CString TempMsg;
    TempMsg.Format(_T("Block_%d maxRG=%d   <%d,%d>"),blockRGMax,RGMax,pDlg->m_RGMin,pDlg->m_RGMax);
    m_pInterface->Ctrl_Addlog(CamID,TempMsg,COLOR_BLUE,200);
    TempMsg.Format(_T("Block_%d minRG=%d   <%d,%d>"),blockRGMin,RGMin,pDlg->m_RGMin,pDlg->m_RGMax);
    m_pInterface->Ctrl_Addlog(CamID,TempMsg,COLOR_BLUE,200);
    TempMsg.Format(_T("Block_%d maxBG=%d   <%d,%d>"),blockBGMax,BGMax,pDlg->m_BGMin,pDlg->m_BGMax);
    m_pInterface->Ctrl_Addlog(CamID,TempMsg,COLOR_BLUE,200);
    TempMsg.Format(_T("Block_%d minBG=%d   <%d,%d>"),blockBGMin,BGMin,pDlg->m_BGMin,pDlg->m_BGMax);
    m_pInterface->Ctrl_Addlog(CamID,TempMsg,COLOR_BLUE,200);
#endif
    strLogInfo.sprintf("Block_%d maxRG=%d   <%d,%d>", blockRGMax, RGMax, m_option->m_RGMin, m_option->m_RGMax);
    m_context->ChannelController->LogToWindow(strLogInfo, qRgb(0, 0, 255));
    strLogInfo.sprintf("Block_%d minRG=%d   <%d,%d>", blockRGMin, RGMin, m_option->m_RGMin, m_option->m_RGMax);
    m_context->ChannelController->LogToWindow(strLogInfo, qRgb(0, 0, 255));
    strLogInfo.sprintf("Block_%d maxBG=%d   <%d,%d>", blockBGMax, BGMax, m_option->m_BGMin, m_option->m_BGMax);
    m_context->ChannelController->LogToWindow(strLogInfo, qRgb(0, 0, 255));
    strLogInfo.sprintf("Block_%d minBG=%d   <%d,%d>", blockBGMin, BGMin, m_option->m_BGMin, m_option->m_BGMax);
    m_context->ChannelController->LogToWindow(strLogInfo, qRgb(0, 0, 255));
    //得到最终的值
    int YMax = 0;
    int YMin = m_shading.ShadingInfo[1].aveY;
    int YDiff;
    for (uint i = 1; i < 5; i++) {
        if (m_shading.ShadingInfo[i].aveY < YMin)
            YMin = m_shading.ShadingInfo[i].aveY;
        if (m_shading.ShadingInfo[i].aveY > YMax)
            YMax = m_shading.ShadingInfo[i].aveY;
        YDiff = YMax - YMin;
        if (YDiff > m_option->m_YDiff)
            m_shading.Ydif_result = RESULT_FAIL;
        else
            m_shading.Ydif_result = RESULT_PASS;

         // TODO ui log
//        CString TempMsg;
//        TempMsg.Format(_T("Y_Blk%d=%d"),i,m_shading.ShadingInfo[i].aveY);
//        m_pInterface->Ctrl_Addlog(CamID,TempMsg,COLOR_BLUE,200);
        strLogInfo.sprintf("Y_Blk%d=%d",i,m_shading.ShadingInfo[i].aveY);
        m_context->ChannelController->LogToWindow(strLogInfo, qRgb(0, 0x80, 0));
    }

    //记录结果
    for (uint i = 1; i < 5; i++) {
        //判断deltaY是否超标
        if (m_shading.ShadingInfo[i].aveY > m_option->m_YMax ||
                m_shading.ShadingInfo[i].aveY < m_option->m_YMin)
            m_shading.ShadingInfo[i].deltaY_result = RESULT_FAIL;
        else
            m_shading.ShadingInfo[i].deltaY_result = RESULT_PASS;

        if (m_shading.ShadingInfo[i].RGain > m_option->m_RGMax ||
                m_shading.ShadingInfo[i].RGain < m_option->m_RGMin)
            m_shading.ShadingInfo[i].RG_result = RESULT_FAIL;
        else
            m_shading.ShadingInfo[i].RG_result = RESULT_PASS;

        if (m_shading.ShadingInfo[i].BGain > m_option->m_BGMax ||
                m_shading.ShadingInfo[i].BGain < m_option->m_BGMin)
            m_shading.ShadingInfo[i].BG_result = RESULT_FAIL;
        else
            m_shading.ShadingInfo[i].BG_result = RESULT_PASS;

        //如果有一个超标，此ROI总体结果NG；
        if (m_shading.ShadingInfo[i].BG_result == RESULT_FAIL||
            m_shading.ShadingInfo[i].RG_result == RESULT_FAIL||
            m_shading.ShadingInfo[i].deltaY_result == RESULT_FAIL||
            m_shading.Ydif_result == RESULT_FAIL)
        {
            m_shading.ShadingInfo[i].result = RESULT_FAIL;
            roiInfos[i].MarkerColor = qRgb(255, 0, 0);
        }
        else
        {
            m_shading.ShadingInfo[i].result = RESULT_PASS;
            roiInfos[i].MarkerColor = qRgb(0, 0x80, 0);
        }
    }

    m_shading.ShadingInfo[0].BG_result = RESULT_PASS; //0 默认OK
    m_shading.ShadingInfo[0].RG_result = RESULT_PASS; //0 默认OK
    m_shading.ShadingInfo[0].deltaY_result = RESULT_PASS; //0 默认OK
    m_shading.ShadingInfo[0].result = RESULT_PASS; //0 默认OK

    //如果有一个ROI结果NG,整个测试项目结果NG
    int failCount = 0;
    for (int i = 0; i < 5; i++) {
        if (m_shading.ShadingInfo[i].result == RESULT_FAIL )
            failCount++;
    }

    int ec = failCount > 0 ? ERR_Failed : ERR_NoError;
    if (m_option->m_Continue)
        ec = ERR_Continue;
     if (ec != ERR_NoError)
         return ec;

    char buff[128];
    for (uint i = 0; i < 5; i++) {
        sprintf(buff, "Roi = %d:aveY = %d,aveR = %d,aveG= %d,aveB = %d", i,
                m_shading.ShadingInfo[i].aveY, m_shading.ShadingInfo[i].aveR,
                m_shading.ShadingInfo[i].aveG, m_shading.ShadingInfo[i].aveB);
        roiInfos[i].Label = std::string(buff);
    }

    return ec;
}

void Shading5::calcPosition(int idx, const T_FrameParam &frameParam, T_RoiInfo &roiInfo)
{
    int oriXPercent = m_option->m_RoiConfs[idx].XPercent;
    if (oriXPercent > 100)
        oriXPercent = 100;
    int oriYPercent = m_option->m_RoiConfs[idx].YPercent;
    if (oriYPercent > 100)
        oriYPercent = 100;
    roiInfo.Width = m_option->m_RoiWidthPercent * frameParam.Width / 100;
    roiInfo.Height = m_option->m_RoiHeightPercent * frameParam.Height / 100;
    roiInfo.X = (int)((oriXPercent - m_option->m_RoiWidthPercent / 2.0) * frameParam.Width / 100);
    roiInfo.Y = (int)((oriYPercent - m_option->m_RoiHeightPercent / 2.0) * frameParam.Height / 100);

    roiInfo.X = roiInfo.X / 4 * 4;
    roiInfo.Y = roiInfo.Y / 4 * 4;
    roiInfo.Width = roiInfo.Width / 4 * 4;
    roiInfo.Height = roiInfo.Height / 4 * 4;
}

void Shading5::raw10ToRaw8(const uchar *raw10, uint width, uint height,
                          uchar *raw8, uint startX, uint startY, uint roiWidth, uint roiHeight)
{
    (void)(height);
    for (uint r = startY; r < startY + roiHeight; r++) {
        const ushort *raw10Row = (const ushort*)raw10 + r * width ;
        uchar *raw8Row = raw8 + (r - startY) * roiWidth;
        for (uint c = startX; c < startX + roiWidth; c++) {
            raw8Row[c - startX] = (uchar)(raw10Row[c] >> 2);
        }
    }
}

int Shading5::getAWBInfoResult8bit(const uchar *image, int blockIdx)
{
    uint64 topLeftSum = 0;
    uint64 topRightSum = 0;
    uint64 bottomLeftSum = 0;
    uint64 bottomRightSum = 0;
    uint64 count = 0;

    uint startX = m_shading.ShadingInfo[blockIdx].ROI.X;
    uint endX   = startX + m_shading.ShadingInfo[blockIdx].ROI.Width;
    if (endX > m_frameParam.Width)
        endX = m_frameParam.Width;
    uint startY = m_shading.ShadingInfo[blockIdx].ROI.Y;
    uint endY   = startY + m_shading.ShadingInfo[blockIdx].ROI.Height;
    if (endY > m_frameParam.Height)
        endY = m_frameParam.Height;

    const ushort* raw10Buf = (const ushort*)image;
    for (uint y = startY; y < endY; y += 2) {
        uint currentRowIdx = m_frameParam.Width * y;
        uint nextRowIdx = m_frameParam.Width * (y + 1);
        for (uint x = startX; x < endX; x += 2) {
            // use high 8bit
            topLeftSum  += raw10Buf[currentRowIdx + x] >> 2;
            topRightSum += raw10Buf[currentRowIdx + x + 1] >> 2;
            bottomLeftSum += raw10Buf[nextRowIdx + x] >> 2;
            bottomRightSum  += raw10Buf[nextRowIdx + x + 1] >> 2;
            count++;
        }
    }
    if (0 == count) {
        qCritical("Input count equal zero error!!!");
        return ERR_InvalidParameter;
    }

    uint RSum = 0, BSum = 0, GrSum = 0, GbSum = 0;
    switch (m_frameParam.ImageMode) {
    case IMAGE_MODE_YCbYCr_RG_GB:
        RSum = topLeftSum;
        GrSum = topRightSum;
        GbSum = bottomLeftSum;
        BSum = bottomRightSum;
        break;
    case IMAGE_MODE_YCrYCb_GR_BG:
        GrSum = topLeftSum;
        RSum = topRightSum;
        BSum = bottomLeftSum;
        GbSum = bottomRightSum;
        break;
    case IMAGE_MODE_CbYCrY_GB_RG:
        GbSum = topLeftSum;
        BSum = topRightSum;
        RSum = bottomLeftSum;
        GrSum = bottomRightSum;
        break;
    case IMAGE_MODE_CrYCbY_BG_GR:
        BSum = topLeftSum;
        GbSum = topRightSum;
        GrSum = bottomLeftSum;
        RSum = bottomRightSum;
        break;
    default:
        qCritical("Can't support ImageMode[%d]", m_frameParam.ImageMode);
        return ERR_InvalidParameter;

    }

    float Gr = 1.0 * GrSum / count;
    float Gb = 1.0 * GbSum / count;
    float R  = 1.0 * RSum  / count;
    float B  = 1.0 * BSum  / count;
    m_shading.ShadingInfo[blockIdx].aveR  = (int)R;
    m_shading.ShadingInfo[blockIdx].aveB  = (int)B;
    m_shading.ShadingInfo[blockIdx].aveGr  = (int)Gr;
    m_shading.ShadingInfo[blockIdx].aveGb  = (int)Gb;
    m_shading.ShadingInfo[blockIdx].aveG  = int((Gr + Gb) / 2.0);
    m_shading.ShadingInfo[blockIdx].RGain = int(512.0 * (R - m_option->m_BLC ) /
                                                ((Gr + Gb) / 2.0 - m_option->m_BLC) + 0.5 );// Rave/Gave
    m_shading.ShadingInfo[blockIdx].BGain = int(512.0 * (B - m_option->m_BLC) /
                                                ((Gr + Gb) /2.0 - m_option->m_BLC) + 0.5 );  //Bave/Gave

    m_shading.ShadingInfo[blockIdx].aveY = (int)(0.299 * m_shading.ShadingInfo[blockIdx].aveR +
                                                 0.587 * m_shading.ShadingInfo[blockIdx].aveG +
                                                 0.114 * m_shading.ShadingInfo[blockIdx].aveB);
    return ERR_NoError;
}

