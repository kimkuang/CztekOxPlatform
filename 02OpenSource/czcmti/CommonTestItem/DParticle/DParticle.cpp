#include "DParticle.h"
#include <QDebug>
#include <QDateTime>
#include "IImageSensor.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

CZPLUGIN_API ICzPlugin *CreatePlugin(void *arg)
{
    return new Dparticle((const char *)arg);
}

CZPLUGIN_API void DestroyPlugin(ICzPlugin **plugin)
{
    Dparticle *pTestItem = (Dparticle *)(*plugin);
    if (pTestItem != nullptr) {
        delete pTestItem; pTestItem = nullptr;
        *plugin = nullptr;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

Dparticle::Dparticle(const char *instanceName)
{
    m_instanceName = QString::fromLatin1(instanceName);
    m_option = new Option();
    m_wdtConf = new ConfWidget(m_option);
    m_context = nullptr;

    for (int i = 0; i < 4; i++) {
        m_raw8ChannlesBuf[i] = nullptr;
    }

    m_deadStack.pBrightDeadStk = nullptr;
    m_deadStack.DeadCnt = 0;

    m_whiteSpotResult.DfcStk = nullptr;
}

Dparticle::~Dparticle()
{
    delete m_wdtConf;
    delete m_option;
    freeMemory();
}

int Dparticle::GetPluginInfo(T_PluginInfo &pluginInfo)
{
    pluginInfo.Version = 0x09000007;
    strcpy(pluginInfo.Description, "Dparticle(20180614)");
    strcpy(pluginInfo.FriendlyName, "Dparticle");
    strcpy(pluginInfo.InstanceName, m_instanceName.toLatin1().constData());
    strcpy(pluginInfo.VendorName, "CZTEK");
    pluginInfo.OptionDlgHandle = (uint64)m_wdtConf;

    return 0;
}

int Dparticle::LoadOption()
{
    if ((m_context == nullptr) || (m_context->ChannelController == nullptr))
        return ERR_Failed;
    QMap<QString, QString> configurations;
    m_context->ModuleSettings->ReadSection(m_instanceName, configurations);
    QString strKey;
    strKey.sprintf("AreaHeight");
    if (configurations.contains(strKey))
        m_option->m_AreaHeight = configurations[strKey].toInt();

    strKey.sprintf("AreaWidth");
    if (configurations.contains(strKey))
        m_option->m_AreaWidth = configurations[strKey].toInt();

    strKey.sprintf("BottomSL");
    if (configurations.contains(strKey))
        m_option->m_BottomSL = configurations[strKey].toInt();

    strKey.sprintf("TopSL");
    if (configurations.contains(strKey))
        m_option->m_TopSL = configurations[strKey].toInt();

    strKey.sprintf("LeftSL");
    if (configurations.contains(strKey))
        m_option->m_LeftSL = configurations[strKey].toInt();

    strKey.sprintf("RightSL");
    if (configurations.contains(strKey))
        m_option->m_RightSL = configurations[strKey].toInt();

    strKey.sprintf("DeadSpec");
    if (configurations.contains(strKey))
        m_option->m_DeadSpec = configurations[strKey].toInt();

    strKey.sprintf("DeadType");
    if (configurations.contains(strKey))
        m_option->m_DeadType = configurations[strKey].toInt();

    strKey.sprintf("DeadUnit");
    if (configurations.contains(strKey))
        m_option->m_DeadUnit = configurations[strKey].toInt();

    strKey.sprintf("DefultExposure");
    if (configurations.contains(strKey))
        m_option->m_DefultExposure = configurations[strKey].toInt();

    strKey.sprintf("DefultGain");
    if (configurations.contains(strKey))
        m_option->m_DefultGain = configurations[strKey].toInt();

    strKey.sprintf("bSaveImage");
    if (configurations.contains(strKey))
        m_option->m_SaveImage = (configurations[strKey].toInt() == 1);

    return m_wdtConf->Cache2Ui();
}

int Dparticle::SaveOption()
{
    if ((m_context == nullptr) || (m_context->ChannelController == nullptr))
        return ERR_Failed;
    int ec = m_wdtConf->Ui2Cache();
    if (ec == 0) {
        QMap<QString, QString> configurations;
        configurations["AreaHeight"] = QString::number(m_option->m_AreaHeight);
        configurations["AreaWidth"] = QString::number(m_option->m_AreaWidth);
        configurations["BottomSL"] = QString::number(m_option->m_BottomSL);
        configurations["TopSL"] = QString::number(m_option->m_TopSL);
        configurations["LeftSL"] = QString::number(m_option->m_LeftSL);
        configurations["RightSL"] = QString::number(m_option->m_RightSL);

        configurations["DeadSpec"] = QString::number(m_option->m_DeadSpec);
        configurations["DeadType"] = QString::number(m_option->m_DeadType);
        configurations["DeadUnit"] = QString::number(m_option->m_DeadUnit);
        configurations["DefultExposure"] = QString::number(m_option->m_DefultExposure);
        configurations["DefultGain"] = QString::number(m_option->m_DefultGain);
        configurations["bSaveImage"] = QString::number(m_option->m_SaveImage ? 1 : 0);

        // save to file
        ec = m_context->ModuleSettings->WriteSection(m_instanceName, configurations);
    }
    return ec;
}

int Dparticle::RestoreDefaults()
{
    return m_wdtConf->RestoreDefaults();
}

int Dparticle::BindChannelContext(T_ChannelContext *context)
{
    m_context = context;
    if ((nullptr == m_context) || (nullptr == m_context->ChannelController) || (nullptr == m_context->ImageSensor))
        return ERR_Failed;
    LoadOption();
    return ERR_NoError;
}

ITestItem::E_ItemType Dparticle::GetItemType() const
{
    return ITestItem::ItemType_ImageEvaluation;
}

bool Dparticle::GetIsSynchronous() const
{
    return true;
}

QString Dparticle::GetReportHeader() const
{
    QString strHeader = "Partical Result";
    return strHeader;
}

QString Dparticle::GetReportContent() const
{
    QString strContent = "";
    strContent.sprintf("%d", m_bTestResult ? 0 : 1);
    return strContent;
}

bool Dparticle::GetIsEngineeringMode() const
{
    return m_option->m_SaveImage;
}

int Dparticle::allocateMemory()
{
    uint bufLen = m_frameParam.Width * m_frameParam.Height / 4;
    for (uint i = 0; i < 4; i++) {
        if (m_raw8ChannlesBuf[i] != nullptr) {
            delete[] m_raw8ChannlesBuf[i];
            m_raw8ChannlesBuf[i] = nullptr;
        }
        m_raw8ChannlesBuf[i] = new uchar[bufLen];
        if (nullptr == m_raw8ChannlesBuf[i]) {
            qCritical("Malloc raw8 channle buffer failed");
            return ERR_NO_MEMEORY;
        }
    }

    m_deadStack.DeadCnt = 0;
    if (nullptr == m_deadStack.pBrightDeadStk) {
        m_deadStack.pBrightDeadStk = new T_WPixInfo[E_MaxDeadCount];
        if (nullptr == m_deadStack.pBrightDeadStk) {
            qCritical("Malloc pBrightDeadStk failed");
            return ERR_NO_MEMEORY;
        }
    }

    m_whiteSpotResult.DeadNum = 0;
    if (nullptr == m_whiteSpotResult.DfcStk) {
        m_whiteSpotResult.DfcStk = new T_WPixInfo[E_MaxDeadCount];
        if (nullptr == m_whiteSpotResult.DfcStk) {
            qCritical("Malloc DfcStk failed");
            return ERR_NO_MEMEORY;
        }
    }

    return ERR_NoError;
}

void Dparticle::freeMemory()
{
    for (int i = 0; i < 4; i++) {
        if (m_raw8ChannlesBuf[i] != nullptr) {
            delete[] m_raw8ChannlesBuf[i];
            m_raw8ChannlesBuf[i] = nullptr;
        }
    }

    if (m_deadStack.pBrightDeadStk != nullptr) {
        delete[] m_deadStack.pBrightDeadStk;
        m_deadStack.pBrightDeadStk = nullptr;
    }

    if (m_whiteSpotResult.DfcStk != nullptr) {
        delete[] m_whiteSpotResult.DfcStk;
        m_whiteSpotResult.DfcStk = nullptr;
    }
}

int Dparticle::Initialize(const T_FrameParam &frameParam)
{
    m_frameParam = frameParam;
    m_hasSetExpGain = false;
    m_bTestResult = false;
    return ERR_NoError;
}

int Dparticle::Evaluate(const uchar *image, uint64 &timestamp, std::vector<T_RoiInfo> &roiInfos)
{
    roiInfos.clear();
    int ec = ERR_NoError;
    if (!m_hasSetExpGain) {
        m_context->ImageSensor->GetSensorExposure(m_lastExp);
        m_context->ImageSensor->GetSensorGain(m_lastGain);

        ec = m_context->ImageSensor->SetSensorExposure(m_option->m_DefultExposure);
        if (ec < 0) {
            qCritical("Call SetSensorExposure() failed[%d]", ec);
            return ec;
        }
        ec = m_context->ImageSensor->SetSensorGain(m_option->m_DefultGain, true);
        if (ec < 0) {
            qCritical("Call SetSensorGain() failed[%d]", ec);
            return ec;
        }
        m_hasSetExpGain = true;
        m_setExpGainTimeStamp_us = m_context->ChannelController->GetCurrentTimestamp();
        return ERR_Continue;
    }

    uint64 diff = 0;
    if (timestamp > m_setExpGainTimeStamp_us) {
        diff = (timestamp - m_setExpGainTimeStamp_us) / 1000;
    }
    if (diff < 100) { // wait 100ms
        return ERR_Continue;
    }

    ec = allocateMemory();
    if (ec < 0) {
        freeMemory();
        qCritical("Call allocateMemory() falied[%d]", ec);
        m_context->ImageSensor->SetSensorExposure(m_lastExp);
        m_context->ImageSensor->SetSensorGain(m_lastGain, false);
        return ec;
    }

    m_whiteSpotResult.DefectStackCnt = 0;
    m_whiteSpotResult.DeadNum = 0;
    for (int k = 0; k < E_MaxDeadCount; k++) {
        m_whiteSpotResult.DfcStk[k].xIndex = 0;
        m_whiteSpotResult.DfcStk[k].Y_Value = 0;
        m_whiteSpotResult.DfcStk[k].yIndex = 0;
    }
    ec = whiteSpotTestInner(image, m_frameParam, &m_whiteSpotResult);
    if (true == m_option->m_SaveImage)
    {
        QString strImg = "";
        QDateTime curTime = QDateTime::currentDateTime();
        const char* outFormat[] = {"RGGB", "GRBG", "GBRG", "BGGR"};
        strImg.sprintf("%s_%s_%dx%d_%s_%d.raw", curTime.toString("yyMMddhhmmss").toStdString().c_str(),
                       m_instanceName.toLatin1().constData(), m_frameParam.Width, m_frameParam.Height,
                       outFormat[m_frameParam.ImageMode], m_context->ChnIdx);
        if (ERR_NoError != m_context->ChannelController->SaveImage(strImg, Image_MyRaw, image, m_frameParam.Size))
        {
            m_context->ChannelController->LogToWindow("Don not Save Image", qRgb(0xff, 0, 0));
        }
    }

    // TODO save log and raw data
    showDParticlePositon(m_whiteSpotResult, roiInfos);
    freeMemory();
    m_context->ImageSensor->SetSensorExposure(m_lastExp);
    m_context->ImageSensor->SetSensorGain(m_lastGain);
    m_bTestResult = (ec == ERR_NoError) ? true : false;
    return ec;
}

int Dparticle::splitRaw10ToRaw8Channels(const uchar *raw10, uint width, uint height, uchar *raw8[])
{
    uint cnt = 0;
    for (uint y = 0; y < height; y += 2) {
        const ushort *currentRow = (const ushort*)raw10 + y * width;
        const ushort *nextRow = currentRow + width;
        for (uint x = 0; x < width; x += 2) {
            // get high 8 bits
            raw8[0][cnt] = currentRow[x] >> 2;
            raw8[1][cnt] = currentRow[x + 1] >> 2;
            raw8[2][cnt] = nextRow[x] >> 2;
            raw8[3][cnt] = nextRow[x + 1] >> 2;
            cnt++;
        }
    }

    return ERR_NoError;
}

int Dparticle::whiteSpotTestInner(const uchar *image, const T_FrameParam &frameParam,
                                  T_WDefectStack *DfcStk)
{
    int ec = splitRaw10ToRaw8Channels(image, frameParam.Width, frameParam.Height, m_raw8ChannlesBuf);
    if (ec < 0) {
        qCritical("Call splitRaw10ToRaw8Channels() failed");
        return ec;
    }

    int DeadType = m_option->m_DeadType;
//    int nParticleNum = 0;
    int nDeadNum = 0;
//    int nDarkNum = 0;
//    int nWoundNum = 0;

    for (int chnidx = 0; chnidx < CHANNEL_COUNT; chnidx++) {
        // TODO save raw data ????
        m_deadStack.DeadCnt = 0;
        getDeadPixelFromPannel(m_raw8ChannlesBuf[chnidx], frameParam.Width / 2,
                                frameParam.Height / 2, &m_deadStack);
        getClusterFromStack(m_deadStack.pBrightDeadStk, m_deadStack.DeadCnt, nDeadNum, DeadType,
                            frameParam.Width / 2, DfcStk);
        if (nDeadNum >0) {
            DfcStk->DeadNum = nDeadNum;
            getDeadPixelPositionInFullImage(DfcStk, chnidx);
            DfcStk->Podtype = 1;
            ec = ERR_Failed;
            break;
        }
    }
    if (ec < 0) {
        // TODO pixel positon

        if (m_option->m_SaveImage) {
            // TODO save raw data
        }
    }

    return ec;
}

void Dparticle::getDeadPixelFromPannel(const uchar *pChannel, uint channelWidth, uint channelHeight,
                                       T_WPixInfoStack *InfoStack)
{
    uint finalWidth = channelWidth;
    uint finalHeight = channelHeight;
    int nTopSL = m_option->m_TopSL; //shift several lines
    int nBottomSL = m_option->m_BottomSL; //shift several columns
    int nLeftSL  = m_option->m_LeftSL;
    int nRightSL = m_option->m_RightSL;
    int DeadUnit  = m_option->m_DeadUnit;
    int DeadSpec = m_option->m_DeadSpec;
    int nAreaWidth = m_option->m_AreaWidth;
    int nAreaHeight = m_option->m_AreaHeight;

//    uint avg_Y = 0;
//    uint sum_Y = 0;
    float temp = 0;
    float realSpec = 0;
    float spec = 0;
    uint tempSUM = 0;
    uint NowSum = 0;
    uint LastCnt = 0;
    for (uint y = nTopSL; y < finalHeight - nBottomSL; y++ ) {
        for (uint x = nLeftSL; x < finalWidth - nRightSL; x++) {
            temp = getSurroundingAreaAve(pChannel, finalWidth, finalHeight, x, y,
                           nAreaWidth, nAreaHeight, tempSUM, NowSum, LastCnt);
            tempSUM = NowSum;
#if 1
            if (DeadUnit)
                realSpec = (pChannel[y * finalWidth + x] - temp) / temp * 100;
            else
                realSpec = (pChannel[y * finalWidth + x] * 1.0 - temp);
            spec = DeadSpec;
            if (realSpec > spec) {
                T_WPixInfo wPixel;
                wPixel.Average = temp;
                wPixel.xIndex = x;
                wPixel.yIndex = y;
                wPixel.Y_Value = pChannel[y * finalWidth + x];
                pushDeadIntoStack(InfoStack->pBrightDeadStk, wPixel, InfoStack->DeadCnt,
                                   channelWidth, channelHeight, E_MaxDeadCount);
                InfoStack->DeadCnt++;
            }
#else
            int lumDiff = 0;
            if (DeadUnit) {
                lumDiff = ((int)pChannel[y * finalWidth + x] - DeadSpec * temp / 100 + temp);
            }
            else
                lumDiff = (pChannel[y * finalWidth + x] - temp - DeadSpec);
            if (lumDiff > 0) {
                T_WPixInfo wPixel;
                wPixel.Average = temp;
                wPixel.xIndex = x;
                wPixel.yIndex = y;
                wPixel.Y_Value = pChannel[y * finalWidth + x];
                PushDeadIntoStack(InfoStack->pBrightDeadStk, wPixel, InfoStack->DeadCnt,
                                   channelWidth, channelHeight, E_MaxDeadCount);
                InfoStack->DeadCnt++;
            }
#endif
        }
    }
}


float Dparticle::getSurroundingAreaAve(const uchar *pChannel, int width, int height, int X, int Y,
                                        int KenalW, int KenalH, uint LastSum, uint &NowSum, uint &lastCnt)
{
    uint sum = 0;
    uint count = 0;

    int radusw = (KenalW - 1) / 2;
    int radush = (KenalH - 1) /2;

    if (LastSum != 0) {
        count = lastCnt;
        int startY = std::max((int)(Y - radush), 0);
        int endY = std::min((int)(Y + radush), (int)(height - 1));
        int j = X - radusw - 1;
        if (j >= 0) {
            for (int i = startY; i <= endY; i++) {
                sum += pChannel[i * width + j];
                count--;
            }
        }
        sum = LastSum - sum;
        j = X + radusw;
        if (j < width) {
            for (int i = startY; i <= endY; i++) {
                sum += pChannel[i * width + j];
                count++;
            }
        }
    }
    else{
        int startY = std::max((int)(Y - radush), 0);
        int endY = std::min((int)(Y + radush), (int)(height - 1));
        int startX = std::max((int)(X - radusw), 0);
        int endX = std::min((int)(X+radusw), (int)(width - 1));
        for (int i = startY; i <= endY; i++ ) {
            for (int j = startX; j <= endX; j++ ) {
                sum += pChannel[i * width + j];
                count++;
            }
        }
    }
    NowSum = sum;
    lastCnt = count;
    return (sum * 1.0 / count);
}

void Dparticle::pushDeadIntoStack(T_WPixInfo *stack, const T_WPixInfo &deadPix, int count,
                                  uint imgWidth, uint imgHeight, int stackMaxSize)
{
    (void)imgHeight;
    if (count >= stackMaxSize)
        return;

    uint tempIndex = deadPix.yIndex * imgWidth + deadPix.xIndex;
    uint i;
    for (i = count; i > 0; i--) {
        if (stack[i - 1].yIndex * imgWidth + stack[i - 1].xIndex > tempIndex)
            stack[i] = stack[i-1];
        else
            break;
    }

    stack[i] = deadPix;
    count++; // TODO ???
}


void Dparticle::getClusterFromStack(T_WPixInfo* SingleDeadStack, int StackSize, int &nParticleNum,
                                    int nType, int nImageWidth, T_WDefectStack *DfcStk)
{
    int Distance = nImageWidth*10;
    nParticleNum = 0;

    if( (1 == nType) || StackSize >= 999 ) //如果Cluster的类型为1，则直接将SingleDeadStact划入
    {
        StackSize = std::min(StackSize, 30);

        for( int i = 0; i < StackSize; i++ )
        {
            DfcStk->DfcStk[DfcStk->DefectStackCnt++] = SingleDeadStack[i];
            nParticleNum++;
        }

        return;
    }

    T_WPixInfo tempStack[100];
    int tempStackSize = 0;

    for( int i = 0; i < StackSize-1; i++ )
    {
        tempStack[0] = SingleDeadStack[i];
        tempStackSize = 1;

        for( int j = i+1; j < StackSize; j++ )
        {
            //看两个Pixel之间的距离是否足够远，如果足够远，则没有必要进行相邻性确认，直接进行下一个确认。
            if( ( SingleDeadStack[j].yIndex - tempStack[0].yIndex )*nImageWidth + ( SingleDeadStack[j].xIndex - tempStack[0].xIndex ) > Distance )
            {
                j = StackSize;
                continue;
            }

            int tempStackSizeCurrent = tempStackSize;

            for( int k = 0; k < tempStackSizeCurrent; k++ )
            {
                if( (abs( SingleDeadStack[j].yIndex - tempStack[k].yIndex ) <=1 )  && (abs( SingleDeadStack[j].xIndex - tempStack[k].xIndex ) <= 1 ) )
                {
                    tempStack[tempStackSize] = SingleDeadStack[j];

                    tempStackSize++;

                    if( tempStackSize >=  nType )
                    {

                        DfcStk->DfcStk[DfcStk->DefectStackCnt++]  = SingleDeadStack[i];
                        nParticleNum++;

                        if( nParticleNum >= 30 )
                        {
                            return;
                        }
                        else
                        {
                            k = tempStackSizeCurrent; //已经确定该点为一个坏点，进入下一个点的判断。
                            j = StackSize;
                            continue;
                        }
                    }


                    //已经确认该Pixel是相邻的，将该Pixel从待确认列表中移除。
                    for( int index = j+1; index < StackSize; index++ )
                    {
                        SingleDeadStack[index-1] = SingleDeadStack[index];
                    }

                    StackSize--;
                    j = j-1; //由于栈中所有数据都往前移了一位，所以计数器也必须往前移一位，否则就少判断一个点了。

                    k = tempStackSizeCurrent; //已经确定该点为一个坏点，进入下一个点的判断。
                    continue;
                }
            }
        }
    }
}

int Dparticle::getDeadPixelPositionInFullImage(T_WDefectStack *DfcStk, int chnIdx)
{
    int offsetX = 0;
    int offsetY = 0;
    switch (chnIdx) {
    case CHANNEL_R0C0:
        offsetX = 0;
        offsetY = 0;
        break;
    case CHANNEL_R0C1:
        offsetX = 1;
        offsetY = 0;
        break;
    case CHANNEL_R1C0:
        offsetX = 0;
        offsetY = 1;
        break;
    case CHANNEL_R1C1:
        offsetX = 1;
        offsetY = 1;
        break;
    default:
        qCritical("Can't support %d channel", chnIdx);
        return ERR_InvalidParameter;
    }

    for (int i = 0; i < DfcStk->DefectStackCnt; i++) {
        DfcStk->DfcStk[i].xIndex = 2 * DfcStk->DfcStk[i].xIndex + offsetX;
        DfcStk->DfcStk[i].yIndex = 2 * DfcStk->DfcStk[i].yIndex + offsetY;
    }

    return ERR_NoError;
}

void Dparticle::showDParticlePositon(const T_WDefectStack &whiteSpotResult, std::vector<T_RoiInfo> &roiInfos)
{
    roiInfos.clear();
    for (int i = 0; i < whiteSpotResult.DeadNum; i++) {
        T_RoiInfo info;
        info.X = std::max((int)whiteSpotResult.DfcStk[i].xIndex - 50, 0);
        info.Y = std::max((int)whiteSpotResult.DfcStk[i].yIndex - 50, 0);
        info.Width = 100;
        info.Height = 100;
        info.MarkerColor = qRgb(255, 0, 0);
        roiInfos.push_back(info);
    }
}

