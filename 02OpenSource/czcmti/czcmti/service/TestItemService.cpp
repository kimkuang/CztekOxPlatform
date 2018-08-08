#include "TestItemService.h"
#include <QFileInfo>
#include <QDebug>
#include "common/Global.h"
#include "IChannelController.h"
#include <chrono>
#include "DecoderService.h"
#include "PluginLibPool.h"

std::mutex TestItemService::m_mutex;
QMap<uint, TestItemService *> TestItemService::m_mapChnIdx2TestItemService;
TestItemService *TestItemService::GetInstance(uint chnIdx)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    (void)(locker);
    TestItemService *instance = nullptr;
    if (!m_mapChnIdx2TestItemService.contains(chnIdx)) {        
        qDebug("New test item service...[chnIdx: %u]", chnIdx);
        instance = new TestItemService(chnIdx);
        m_mapChnIdx2TestItemService.insert(chnIdx, instance);
    }
    else {
        instance = m_mapChnIdx2TestItemService[chnIdx];
    }
    return instance;
}

void TestItemService::FreeInstance(uint chnIdx)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    (void)(locker);
    if (m_mapChnIdx2TestItemService.contains(chnIdx)) {
        delete m_mapChnIdx2TestItemService[chnIdx];
        m_mapChnIdx2TestItemService.remove(chnIdx);
    }
}

void TestItemService::FreeAllInstances()
{
    QMutableMapIterator<uint, TestItemService*> it(m_mapChnIdx2TestItemService);
    while (it.hasNext()) {
        it.next();
        delete it.value();
        it.remove();
    }
}

TestItemService::TestItemService(uint chnIdx)
{
    m_chnIdx = chnIdx;
    m_channelContext = nullptr;
    m_openShortTest = nullptr;
    m_standbyCurrentTest = nullptr;
    m_hardwareTestThread = nullptr;
    m_sensorImageDecoder = nullptr;
    m_imageTestThread = nullptr;
    m_imageOfflineTestThread = nullptr;
    m_offlineTestRaw = nullptr;
}

TestItemService::~TestItemService()
{
    qDebug()<<__FUNCTION__;
    m_channelContext = nullptr; // stop post event
    StopTestThread();
    FreeAllPluginInstances();
}

void TestItemService::BindChannelContext(T_ChannelContext *context)
{
    qDebug()<<__FUNCTION__;
    m_channelContext = context;
    Q_ASSERT(m_channelContext != nullptr);
    Q_ASSERT(m_channelContext->EventReceiver != nullptr);
    Q_ASSERT(m_channelContext->RenderedEvent != nullptr);
    Q_ASSERT(m_channelContext->ChannelController != nullptr);
    // update test item's context
    QMutableMapIterator<QString, ITestItem*> it(m_mapName2Instance);
    while (it.hasNext()) {
        it.next();
        it.value()->BindChannelContext(context);
    }
}

int TestItemService::GetPluginInfo(const QString &fullInstanceName, T_PluginInfo &pluginInfo)
{
    ITestItem *instance = GetPluginInstance(fullInstanceName);
    if (instance != nullptr) {
        return instance->GetPluginInfo(pluginInfo);
    }
    return ERR_Failed;
}

ITestItem *TestItemService::GetPluginInstance(const QString &fullInstanceName)
{
    QStringList slTemp = fullInstanceName.split(QChar('@'), QString::KeepEmptyParts);
    if (2 != slTemp.size()) {
        qCritical("Unkown format of full instance name: %s", fullInstanceName.toLocal8Bit().data());
        return nullptr;
    }

    ITestItem *instance = nullptr;
    if (!m_mapName2Instance.contains(fullInstanceName)) {
        qDebug("Create test item...[chnIdx: %d, name: %s]", m_chnIdx, fullInstanceName.toLocal8Bit().data());
        QString libFullPathName = GlobalVars::PLUGIN_TESTITEM_PATH + slTemp[1];
        instance = static_cast<ITestItem *>(PluginLibPool::GetInstance().NewPlugin(libFullPathName, (void *)slTemp[0].toStdString().c_str()));
        if (instance == nullptr) {
            qCritical().noquote() << QString("Create test item instance failed. [instanceName: %1]").arg(fullInstanceName);
            return instance;
        }

        qDebug() << (m_channelContext == nullptr);
        instance->BindChannelContext(m_channelContext);
        m_mapName2Instance.insert(fullInstanceName, instance);
    }
    instance = m_mapName2Instance[fullInstanceName];
    return instance;
}

void TestItemService::FreePluginInstance(const QString &fullInstanceName)
{
    QStringList slTemp = fullInstanceName.split(QChar('@'), QString::KeepEmptyParts);
    if (2 != slTemp.size()) {
        qCritical("Unkown format of full instance name: %s", fullInstanceName.toLocal8Bit().data());
        return;
    }
    if (m_mapName2Instance.contains(fullInstanceName)) {
        qDebug().noquote() << QString("Delete test item instance...[%1]").arg(fullInstanceName);
        QString libFullPathName = GlobalVars::PLUGIN_TESTITEM_PATH + slTemp[1];
        ITestItem* &instance = m_mapName2Instance[fullInstanceName];
        PluginLibPool::GetInstance().DeletePlugin(libFullPathName, (ICzPlugin **)(&instance));
        m_mapName2Instance.remove(fullInstanceName);
    }
}

void TestItemService::FreeAllPluginInstances()
{
    QMutableMapIterator<QString, ITestItem *> it(m_mapName2Instance);
    while (it.hasNext()) {
        it.next();
        qDebug().noquote() << QString("Delete test item instance...[%1]").arg(it.key());
        QStringList slTemp = it.key().split(QChar('@'), QString::KeepEmptyParts);
        if (2 != slTemp.size())
            continue;
        QString libFullPathName = GlobalVars::PLUGIN_TESTITEM_PATH + slTemp[1];
        ITestItem* &instance = it.value();
        PluginLibPool::GetInstance().DeletePlugin(libFullPathName, (ICzPlugin **)(&instance));
        it.remove();
    }
}

void TestItemService::SetHardwareTestItems(const QVector<QString> &hardwareTestItems)
{
    m_openShortTest = nullptr;
    m_standbyCurrentTest = nullptr;
    m_hardwareTestItems.clear();
    for (int itemIdx = 0; itemIdx < hardwareTestItems.size(); itemIdx++) {
        QString fullInstanceName = hardwareTestItems[itemIdx];
        IHardwareTest *hwTest = static_cast<IHardwareTest *>(GetPluginInstance(fullInstanceName));
        if (hwTest != nullptr) {
            if (hwTest->GetHardwareType() == IHardwareTest::HardwareTest_OpenShort)
                m_openShortTest = hwTest;
            else if (hwTest->GetHardwareType() == IHardwareTest::HardwareTest_StandbyCurrent)
                m_standbyCurrentTest = hwTest;
            else
                m_hardwareTestItems.append(hwTest);
        }
    }
}

void TestItemService::SetImageTestGroupedItems(const QMap<int, QVector<T_ImageTestItem>> &mapGroup2GroupedImgTestItem,
                                               UiDefs::E_WorkMode workMode)
{
    m_mapGroup2GroupedImgTestItem = mapGroup2GroupedImgTestItem;
    m_workMode = workMode;
}

void TestItemService::RunImageTest(int groupIdx, int idxInGroup)
{
    if (!m_mapGroup2GroupedImgTestItem.contains(groupIdx))
        return;
    m_imgTestGroupContext.GroupIdx = groupIdx;
    m_imgTestGroupContext.IdxInGroup = idxInGroup;
    QVector<T_ImageTestItem> &groupedTestItem = m_mapGroup2GroupedImgTestItem[m_imgTestGroupContext.GroupIdx];
    m_imgTestGroupContext.MesUpdateIdx = -1;
    for (int idx = 0; idx < groupedTestItem.size(); idx++) {
        if (groupedTestItem[idx].IsMesUpdate) {
            m_imgTestGroupContext.MesUpdateIdx = idx;
            break;
        }
    }
    m_imgTestGroupContext.LastErrorCode = 0;
    m_imageEvalTestEnabled = true;
}

int TestItemService::StartTestThread()
{
    m_sensorImageDecoder = DecoderService::GetInstance(m_chnIdx)->GetCurrentDecoder();
    if (m_sensorImageDecoder == nullptr) {
        postEvent(new LoggerEvent(QObject::tr("Cannot load decoder!"), LoggerEvent::Log_Fail));
        return ERR_Failed;
    }

    m_sensorImageDecoder->BindChannelContext(m_channelContext);
    if (ERR_NoError != m_channelContext->ChannelController->GetFrameParam(m_frameParam)) {
        postEvent(new LoggerEvent(QObject::tr("Getting frame parameter failed."), LoggerEvent::Log_Fail));
        return ERR_Failed;
    }

    m_imageEvalTestEnabled = false;
    postEvent(new LoggerEvent(QObject::tr("Starting image test thread ..."), LoggerEvent::Log_Info));
    m_imageTestThreadRunning = true;
    m_imageTestThread = new std::thread(&TestItemService::imageTestThread, this);

    postEvent(new LoggerEvent(QObject::tr("Starting hardware test ..."), LoggerEvent::Log_Info));
    m_hardwareTestThreadRunning = true;
    m_hardwareTestThread = new std::thread(&TestItemService::hardwareTestThread, this);

    return ERR_NoError;
}

int TestItemService::StopTestThread()
{
    qDebug("Stopping image test thread...[chnIdx: %u]", m_chnIdx);
    m_imageEvalTestEnabled = false;
    m_imageTestThreadRunning = false;
    qDebug("Stopping hardware test thread...[chnIdx: %u]", m_chnIdx);
    m_hardwareTestThreadRunning = false;
    if ((m_imageTestThread != nullptr) && m_imageTestThread->joinable())
        m_imageTestThread->join();
    if ((m_hardwareTestThread != nullptr) && m_hardwareTestThread->joinable())
        m_hardwareTestThread->join();

    if ((m_channelContext != nullptr) && (m_channelContext->ChannelController != nullptr))
        m_channelContext->ChannelController->SetUseMesFlag(false); //关闭模组，不使用mes,在mescheck中打开该标志
    delete m_imageTestThread;
    m_imageTestThread = nullptr;
    delete m_hardwareTestThread;
    m_hardwareTestThread = nullptr;

    return ERR_NoError;
}

IHardwareTest *TestItemService::GetHardwareTest(IHardwareTest::E_HardwareTestType hwTestType)
{
    if (hwTestType == IHardwareTest::HardwareTest_OpenShort)
        return m_openShortTest;
    else if (hwTestType == IHardwareTest::HardwareTest_StandbyCurrent)
        return m_standbyCurrentTest;
    else
        return nullptr;
}

int TestItemService::RunHardwareTestOneTime(IHardwareTest* hardwareTest)
{
    int ec = ERR_NoError;
    if (hardwareTest != nullptr) {
        IHardwareTest::E_HardwareTestType hwTestType = hardwareTest->GetHardwareType();
        QString caption;
        if (hwTestType == IHardwareTest::HardwareTest_OpenShort) {
            caption = "Open Short";
        }
        else if (hwTestType == IHardwareTest::HardwareTest_StandbyCurrent) {
            caption = "Standby Current";
        }
        else {
            return ERR_NotImplemented;
        }
        std::vector<std::string> resultTable;
        QList<QStringList> qtResultTable;
        ec = hardwareTest->RunTest(resultTable);
        qtResultTable.clear();
        for (auto it = resultTable.begin(); it != resultTable.end(); ++it) {
            QString strTemp = QString::fromStdString(*it);
            qtResultTable.push_back(strTemp.split(QChar(',')));
        }
        postEvent(new TestResultEvent(ec, caption, qtResultTable));

        if ((ec != ERR_NoError) && hardwareTest->GetContinueWhenFailed())
            ec = ERR_NoError;
    }
    return ec;
}

int TestItemService::EnterOfflineTestThread(const uchar* pimge, const T_FrameParam &frameParam)
{
    m_offlineTestRaw = new uchar[frameParam.Size];
    if (nullptr == m_offlineTestRaw)
    {
        return ERR_NO_MEMEORY;
    }

    memcpy(m_offlineTestRaw, pimge, frameParam.Size);
    m_sensorImageDecoder = DecoderService::GetInstance(m_chnIdx)->GetCurrentDecoder();
    if (m_sensorImageDecoder == nullptr) {
        QString strLog = QObject::tr("Cannot load decoder!");
        postEvent(new LoggerEvent(strLog, LoggerEvent::Log_Fail));
        return ERR_Failed;
    }

    m_sensorImageDecoder->BindChannelContext(m_channelContext);
    m_frameParam = frameParam;
    postEvent(new LoggerEvent(QObject::tr("Starting image offline test thread ..."), LoggerEvent::Log_Info));
    m_imageOfflineTestThreadRunning = true;
    m_imageOfflineTestThread = new std::thread(&TestItemService::imageOfflineTestThread, this);
    m_imageEvalTestEnabled = false;

    return ERR_NoError;
}

int TestItemService::ExitOfflineTestThread()
{
    m_imageOfflineTestThreadRunning = false;
    if ((m_imageOfflineTestThread != nullptr) && m_imageOfflineTestThread->joinable())
        m_imageOfflineTestThread->join();

    m_imageEvalTestEnabled = false;
    m_channelContext->ChannelController->SetUseMesFlag(false); //关闭模组，不使用mes,在mescheck中打开该标志
    if (nullptr != m_imageOfflineTestThread)
    {
        delete m_imageOfflineTestThread;
        m_imageOfflineTestThread = nullptr;
    }

    if (nullptr != m_offlineTestRaw)
    {
        delete m_offlineTestRaw;
        m_offlineTestRaw = nullptr;
    }

    return ERR_NoError;
}

int TestItemService::GetItemType(const QString &fullInstanceName, ITestItem::E_ItemType &itemType)
{
    ITestItem *pTestItem = GetPluginInstance(fullInstanceName);
    if (pTestItem != nullptr) {
        itemType = pTestItem->GetItemType();
        return ERR_NoError;
    }
    return ERR_Failed;
}

int TestItemService::RestoreDefaults(const QString &fullInstanceName)
{
    ITestItem *pTestItem = GetPluginInstance(fullInstanceName);
    if (pTestItem != nullptr) {
        return pTestItem->RestoreDefaults();
    }
    return ERR_Failed;
}

int TestItemService::LoadSettings(const QString &fullInstanceName)
{
    ITestItem *pTestItem = GetPluginInstance(fullInstanceName);
    if (pTestItem != nullptr) {
        try {
            if ((nullptr == m_channelContext) || (nullptr == m_channelContext->ModuleSettings))
                return ERR_Failed;
            return pTestItem->LoadOption();
        }
        catch (...) {
            return ERR_Failed;
        }
    }
    return ERR_Failed;
}

int TestItemService::SaveSettings(const QString &fullInstanceName)
{
    ITestItem *pTestItem = GetPluginInstance(fullInstanceName);
    if (pTestItem != nullptr) {
        try {
            if ((nullptr == m_channelContext) || (nullptr == m_channelContext->ModuleSettings)) {
                qCritical("Context is nullptr!");
                return ERR_Failed;
            }
            return pTestItem->SaveOption();
        }
        catch (...) {
            return ERR_Failed;
        }
    }
    return ERR_Failed;
}

void TestItemService::hardwareTestThread()
{
    QElapsedTimer elapsedTimer;
    elapsedTimer.restart();
    std::vector<std::string> resultTable;
    QList<QStringList> qtResultTable;
    int ec = ERR_Failed;
    T_PluginInfo pluginInfo;
    bool bFirstFlag = true;
    while (m_hardwareTestThreadRunning) {
        for (int testItemIdx = 0; (m_hardwareTestThreadRunning && (testItemIdx < (int)m_hardwareTestItems.size())); testItemIdx++) {
            IHardwareTest *pTestItem = static_cast<IHardwareTest *>(m_hardwareTestItems[testItemIdx]);
            if (pTestItem != nullptr) {
                ec = pTestItem->RunTest(resultTable);
                if (ERR_NoError == pTestItem->GetPluginInfo(pluginInfo)) {
                    if (bFirstFlag) {
                        QString strHeader = pTestItem->GetReportHeader();
                        QString strContent = pTestItem->GetReportContent();
                        m_channelContext->ChannelController->SaveReport(QString::fromLocal8Bit(pluginInfo.InstanceName),
                            pTestItem->GetItemType(), strHeader, strContent, (ec==ERR_NoError));
                        bFirstFlag = false;
                    }
                    qtResultTable.clear();
                    for (auto it = resultTable.begin(); it != resultTable.end(); ++it) {
                        QString strTemp = QString::fromStdString(*it);
                        qtResultTable.push_back(strTemp.split(QChar(',')));
                    }
                    postEvent(new TestResultEvent(ec, QString::fromLocal8Bit(pluginInfo.FriendlyName), qtResultTable));
                }
            }
        }
        if (elapsedTimer.elapsed() > 5 * 60 * 1000) { // every 5 minutes
            uint64_t totalSize;
            uint64_t availableSize;
            Utils::GetDiskSizeInfoByFsType("ext4", totalSize, availableSize);
            postEvent(new DiskSizeChangedEvent(totalSize, availableSize));
            elapsedTimer.restart();
        }
        for (int i = 0; (m_hardwareTestThreadRunning && (i < 100)); i++)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    qInfo()<<__FUNCTION__<<"exited.";
}

void TestItemService::imageTestThread()
{
    float fps;
    FpsCalculator testFpsCalculator(FpsCalculator::CC_ByElapsedTime, 1000);
    FpsCalculator displayFpsCalculator(FpsCalculator::CC_ByElapsedTime, 1000);
    int bufIdx = 0;
    uint size = 0;
    uint64 timestamp = 0;
    uchar *pSensorImgBuffer = nullptr, *pRgb24ImgBuffer = nullptr;
    T_FrameParam rgb24FrameParam(IMAGE_FMT_RGB24, IMAGE_MODE_INVALID, 0, 0);
    bool itemInitialized = false;
    std::vector<T_RoiInfo> roiInfos;
    QVector<std::shared_future<T_ImageTestItem>> vtFuctureAsync;
    while (m_imageTestThreadRunning) {
        if (m_channelContext->ChannelController->DequeueFrameBuffer(bufIdx, pSensorImgBuffer, size, timestamp) != ERR_NoError) {
            std::this_thread::sleep_for(std::chrono::milliseconds(0));
            continue;
        }

        if (m_imageEvalTestEnabled) {
            executeOneImageTest(pSensorImgBuffer, timestamp, roiInfos, itemInitialized, vtFuctureAsync);
        }
        if (!m_imageTestThreadRunning)
            break;
        if (timestamp != 0) {
            if (testFpsCalculator.Update(fps))
                postEvent(new FpsEvent(fps, FpsEvent::Fps_Test));
            if (m_channelContext->RenderedEvent->WaitOne(0)) { // Render
                m_sensorImageDecoder->Decode(pSensorImgBuffer, m_frameParam, roiInfos, pRgb24ImgBuffer, rgb24FrameParam);
                postEvent(new RenderFrameEvent(pRgb24ImgBuffer, rgb24FrameParam));
                if (displayFpsCalculator.Update(fps))
                    postEvent(new FpsEvent(fps, FpsEvent::Fps_Display));
            }
            m_channelContext->ChannelController->EnqueueFrameBuffer(bufIdx);
        }
        else {
            if (ERR_NoError != m_channelContext->ChannelController->GetFrameParam(m_frameParam)) {
                postEvent(new LoggerEvent(QObject::tr("Getting frame parameter failed."), LoggerEvent::Log_Fail));
                break;
            }
        }
    }
    m_channelContext->RenderedEvent->WaitOne(1000);
    qInfo()<<__FUNCTION__<<"exited.";
}

void TestItemService::executeOneImageTest(const uchar *pSensorImgBuffer, uint64 &timestamp,
    std::vector<T_RoiInfo> &roiInfos, bool &itemInitialized, QVector<std::shared_future<T_ImageTestItem>> &vtFutureAsync)
{
    auto fnTestItemUpdate = [this](T_ImageTestItem &theTestItem, IImageEvaluation *theImageEval) {
        QString strHeader = theImageEval->GetReportHeader();
        QString strContent = theImageEval->GetReportContent();
        m_channelContext->ChannelController->SaveReport(theTestItem.InstanceName, theImageEval->GetItemType(),
            strHeader, strContent, (theTestItem.ErrorCode == ERR_NoError));

        theTestItem.EndTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        theTestItem.Elapsed = (int)(theTestItem.EndTimestamp - theTestItem.StartTimestamp);
        qDebug() << "testItem.EndTimestamp:" << theTestItem.EndTimestamp << theTestItem.Elapsed;
        QString strLog = QObject::tr("[%1]Finished. ec: %2, elapsed time: %L3ms")
                .arg(theTestItem.InstanceName).arg(theTestItem.ErrorCode).arg(theTestItem.Elapsed);
        postEvent(new LoggerEvent(strLog, LoggerEvent::Log_Info));

        postEvent(new TestItemStateChangedEvent(m_chnIdx, theTestItem.GroupIdx, theTestItem.IdxInGroup,
            TestItemStateChangedEvent::State_Finished, theTestItem.ErrorCode, theTestItem.Elapsed));
    };
    int groupIdx = m_imgTestGroupContext.GroupIdx;
    int idxInGroup = m_imgTestGroupContext.IdxInGroup;
    if (!m_mapGroup2GroupedImgTestItem.contains(groupIdx))
        return;
    QString strLog;
    QVector<T_ImageTestItem> &groupedTestItem = m_mapGroup2GroupedImgTestItem[groupIdx];
    Q_ASSERT(idxInGroup < groupedTestItem.size());
    T_ImageTestItem &testItem = groupedTestItem[idxInGroup];
    IImageEvaluation *imageEval = static_cast<IImageEvaluation *>(GetPluginInstance(testItem.FullInstanceName));
    Q_ASSERT(imageEval != nullptr);
    if (!itemInitialized) {
        testItem.StartTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        qDebug() << "testItem.StartTimestamp:" << testItem.StartTimestamp;
        strLog = QObject::tr("[%1]Start test. chnIdx: %2, groupIdx: %3, idxInGroup: %4")
                .arg(testItem.InstanceName).arg(m_chnIdx).arg(groupIdx).arg(idxInGroup);
        postEvent(new LoggerEvent(strLog, LoggerEvent::Log_Info));
        testItem.ErrorCode = imageEval->Initialize(m_frameParam);
        itemInitialized = (testItem.ErrorCode == ERR_NoError);
    }
    bool isAsync = false;
    if (itemInitialized) // initial ok
    {
        postEvent(new TestItemStateChangedEvent(m_chnIdx, groupIdx, idxInGroup,
            TestItemStateChangedEvent::State_Initialized, 0, 0));
        qDebug() << "imageEval->GetIsSynchronous():" << imageEval->GetIsSynchronous() << vtFutureAsync.size();
        if (imageEval->GetIsSynchronous() || (vtFutureAsync.size() > MAX_SUPPORTED_ASYNC_ITEMS)) {
            testItem.ErrorCode = imageEval->Evaluate(pSensorImgBuffer, timestamp, roiInfos);
        }
        else {
            std::shared_future<T_ImageTestItem> future = std::async(std::launch::async,
                [](const T_ImageTestItem &_testItem, IImageEvaluation *_imageEval, const uchar *_image,
                   int _size, uint64 _timestamp, std::function<void(T_ImageTestItem &, IImageEvaluation *)> fnUpdate)
            {
                qDebug().noquote() << "Begin async invoke:" << _testItem.FullInstanceName;
                uchar *imageBuf = new uchar[_size];
                memcpy(imageBuf, _image, _size);
                T_ImageTestItem theTestItem = _testItem;
                std::vector<T_RoiInfo> roiInfos;
                theTestItem.ErrorCode = _imageEval->Evaluate(imageBuf, _timestamp, roiInfos);
                delete[] imageBuf;
                fnUpdate(theTestItem, _imageEval);
                qDebug().noquote() << "Refresh async item:" << theTestItem.FullInstanceName;
                return theTestItem;
            }, testItem, imageEval, pSensorImgBuffer, m_frameParam.Size, timestamp, fnTestItemUpdate);
            vtFutureAsync.push_back(future);
            testItem.ErrorCode = ERR_NoError;
            isAsync = true;
        }
    }
    if (testItem.ErrorCode <= 0)
    {
        // check this group async items
        if (idxInGroup == groupedTestItem.size() - 1) {
            QVectorIterator<std::shared_future<T_ImageTestItem>> it(vtFutureAsync);
            while (it.hasNext()) {
                T_ImageTestItem theTestItem = it.next().get();
                if (theTestItem.ErrorCode < 0) {
                    m_imgTestGroupContext.LastErrorCode = theTestItem.ErrorCode;
                }
            }
            vtFutureAsync.clear();
        }
        if (!isAsync) {
            qDebug().noquote() << "Refresh sync item:" << testItem.FullInstanceName;
            fnTestItemUpdate(testItem, imageEval);
        }

        bool hasMesUpdateLater = ((m_imgTestGroupContext.MesUpdateIdx != -1) && (!testItem.IsMesUpdate)); // has mes update, and current item is not mes update
        bool groupFinished = ((idxInGroup == groupedTestItem.size() - 1) || // last item
            ((testItem.ErrorCode < 0) && !hasMesUpdateLater) || // current error and has not mes update later
            (testItem.IsMesUpdate && (m_imgTestGroupContext.LastErrorCode < ERR_NoError))); // this is mes update

        itemInitialized = false;
        if (testItem.ErrorCode == ERR_NoError) {
            if (groupFinished) { // End of this group
                m_imgTestGroupContext.GroupIdx++; m_imgTestGroupContext.IdxInGroup = 0;
                m_imageEvalTestEnabled = (m_workMode != UiDefs::WorkMode_Machine);
            }
            else {
                m_imgTestGroupContext.IdxInGroup++;
            }
        }
        else { //  if (testItem.ErrorCode < 0)
            m_imgTestGroupContext.LastErrorCode = testItem.ErrorCode;
            if (m_workMode == UiDefs::WorkMode_Machine)
            {
                if (hasMesUpdateLater) {
                    strLog = QObject::tr("Skip to mes update. indexInGroup: %1").arg(m_imgTestGroupContext.MesUpdateIdx);
                    postEvent(new LoggerEvent(strLog, LoggerEvent::Log_Info));
                    m_imgTestGroupContext.IdxInGroup = m_imgTestGroupContext.MesUpdateIdx;
                }
                else {
                    m_imageEvalTestEnabled = false;
                }
            }
        }
        if (m_workMode == UiDefs::WorkMode_Manual) {
            m_imageEvalTestEnabled = false;
        }

        if (groupFinished) {
            postEvent(new TestItemStateChangedEvent(m_chnIdx, groupIdx, idxInGroup,
                TestItemStateChangedEvent::State_GroupFinished, testItem.ErrorCode, testItem.Elapsed));
        }
    }
}

void TestItemService::imageOfflineTestThread()
{
    QString strLog;
    int ec = 0;
    float fps;
    uint64 timestamp = 0;
    FpsCalculator testFpsCalculator(FpsCalculator::CC_ByElapsedTime, 1000);
    FpsCalculator displayFpsCalculator(FpsCalculator::CC_ByElapsedTime, 1000);
    uchar *pRgb24ImgBuffer = nullptr;
    T_FrameParam rgb24FrameParam(IMAGE_FMT_RGB24, IMAGE_MODE_INVALID, 0, 0);
    bool itemInitialized = false;
    std::vector<T_RoiInfo> roiInfos;
    QElapsedTimer singleItemTimer;
    //int state = 0;
    while (m_imageOfflineTestThreadRunning) {
        if (m_imageEvalTestEnabled)
        {
            if (m_mapGroup2GroupedImgTestItem.contains(m_imgTestGroupContext.GroupIdx))
            {
//                QVector<T_ImageTestItem> &groupedTestItem = m_mapGroup2GroupedImgTestItem[m_imgTestGroupContext.GroupIdx];
//                Q_ASSERT(m_imgTestGroupContext.IdxInGroup < groupedTestItem.size());
//                T_ImageTestItem &testItem = groupedTestItem[m_imgTestGroupContext.IdxInGroup];
//                IImageEvaluation *imageEval = static_cast<IImageEvaluation *>(GetPluginInstance(testItem.FullInstanceName));
//                Q_ASSERT(imageEval != nullptr);
//                switch (state)
//                {
//                case 0:
//                    state = 1;
//                    singleItemTimer.restart();
//                    strLog = QObject::tr("[%1]Start test. chnIdx: %2, groupIdx: %3, idxInGroup: %4")
//                            .arg(testItem.InstanceName).arg(m_chnIdx)
//                            .arg(m_imgTestGroupContext.GroupIdx).arg(m_imgTestGroupContext.IdxInGroup);
//                    postEvent(new LoggerEvent(strLog, LoggerEvent::Log_Info));
//                    break;
//                case 1:
//                    ec = imageEval->Initialize(m_frameParam);
//                    postEvent(new TestItemStateChangedEvent(m_chnIdx, m_imgTestGroupContext.GroupIdx, m_imgTestGroupContext.IdxInGroup,
//                        TestItemStateChangedEvent::ItemPosition_First, ec, 0));
//                    if (ERR_NoError == ec)
//                    {
//                       state = 2;
//                    }
//                    else
//                    {
//                        state = 3;
//                    }
//                    break;
//                case 2:
//                    timestamp = m_channelContext->ChannelController->GetCurrentTimestamp();
//                    ec = imageEval->Evaluate(m_offlineTestRaw, timestamp, roiInfos);
//                    if (ERR_Continue == ec)
//                    {
//                        break;
//                    }
//                    else
//                    {
//                        state = 3;
//                    }
//                    break;
//                case 3:
//                {
//                    bool groupFinished = (m_imgTestGroupContext.IdxInGroup == groupedTestItem.size() - 1);
//                    strLog = QObject::tr("groupFinished: %1, chnIdx: %2, groupIdx: %3, idxInGroup: %4")
//                            .arg(groupFinished).arg(m_chnIdx).arg(m_imgTestGroupContext.GroupIdx).arg(m_imgTestGroupContext.IdxInGroup);
//                    postEvent(new LoggerEvent(strLog, LoggerEvent::Log_Info));

//                    qint64 elapsed = singleItemTimer.elapsed();
//                    TestItemStateChangedEvent::E_ItemIndexInGroupPosition position = TestItemStateChangedEvent::ItemPosition_Middle;
//                    if (groupFinished)
//                        position = TestItemStateChangedEvent::ItemPosition_Last;
//                    postEvent(new TestItemStateChangedEvent(m_chnIdx, m_imgTestGroupContext.GroupIdx, m_imgTestGroupContext.IdxInGroup,
//                        position, ec, elapsed));

//                    QString strHeader = imageEval->GetReportHeader();
//                    QString strContent = imageEval->GetReportContent();
//                    m_channelContext->ChannelController->SaveReport(testItem.InstanceName, imageEval->GetItemType(), strHeader, strContent, (ec==ERR_NoError));
//                    strLog = QObject::tr("[%1]Finished. Elapsed Time: %2ms").arg(testItem.InstanceName).arg(elapsed);
//                    postEvent(new LoggerEvent(strLog, LoggerEvent::Log_Info));
//                    if (ec == ERR_NoError) {
//                        if (groupFinished) { // End of this group
//                            m_imgTestGroupContext.GroupIdx++;
//                            m_imgTestGroupContext.IdxInGroup = 0;
//                        }
//                    }

//                    m_imageEvalTestEnabled = false;
//                    state = 0;
//                }
//                    break;
//                default:
//                    break;
//                }
                QVector<T_ImageTestItem> &groupedTestItem = m_mapGroup2GroupedImgTestItem[m_imgTestGroupContext.GroupIdx];
                Q_ASSERT(m_imgTestGroupContext.IdxInGroup < groupedTestItem.size());
                T_ImageTestItem &testItem = groupedTestItem[m_imgTestGroupContext.IdxInGroup];
                IImageEvaluation *imageEval = static_cast<IImageEvaluation *>(GetPluginInstance(testItem.FullInstanceName));
                Q_ASSERT(imageEval != nullptr);
                //postEvent(new LoggerEvent(QString("11111"), LoggerEvent::Log_Info));
                if (!itemInitialized) {
                    singleItemTimer.restart();
                    strLog = QObject::tr("[%1]Start test. chnIdx: %2, groupIdx: %3, idxInGroup: %4")
                            .arg(testItem.InstanceName).arg(m_chnIdx)
                            .arg(m_imgTestGroupContext.GroupIdx).arg(m_imgTestGroupContext.IdxInGroup);
                    postEvent(new LoggerEvent(strLog, LoggerEvent::Log_Info));
                    ec = imageEval->Initialize(m_frameParam);
                    itemInitialized = (ec == ERR_NoError);
                }

                if (itemInitialized)
                {
                    postEvent(new TestItemStateChangedEvent(m_chnIdx, m_imgTestGroupContext.GroupIdx, m_imgTestGroupContext.IdxInGroup,
                        TestItemStateChangedEvent::State_Initialized, ec, 0));
                    timestamp = m_channelContext->ChannelController->GetCurrentTimestamp();
                    ec = imageEval->Evaluate(m_offlineTestRaw, timestamp, roiInfos);
                }
                if (ec <= 0)
                {
                    QString strHeader = imageEval->GetReportHeader();
                    QString strContent = imageEval->GetReportContent();
                    m_channelContext->ChannelController->SaveReport(testItem.InstanceName, imageEval->GetItemType(), strHeader, strContent, (ec==ERR_NoError));

                    qint64 elapsed = singleItemTimer.elapsed();
                    bool groupFinished = (m_imgTestGroupContext.IdxInGroup == groupedTestItem.size() - 1) || (ec < 0);
                    strLog = QObject::tr("[%1]Finished. ec: %2, elapsed time: %3ms, groupFinished: %4")
                            .arg(testItem.InstanceName).arg(ec).arg(elapsed).arg(groupFinished);
                    postEvent(new LoggerEvent(strLog, LoggerEvent::Log_Info));

                    TestItemStateChangedEvent::E_State state = groupFinished ?
                        TestItemStateChangedEvent::State_GroupFinished : TestItemStateChangedEvent::State_Finished;
                    postEvent(new TestItemStateChangedEvent(m_chnIdx, m_imgTestGroupContext.GroupIdx, m_imgTestGroupContext.IdxInGroup,
                        state, ec, elapsed));

                    itemInitialized = false;
                    if (ec == ERR_NoError) {
                        if (groupFinished) { // End of this group
                            m_imgTestGroupContext.GroupIdx++;
                            m_imgTestGroupContext.IdxInGroup = 0;
                        }
                    }

                    m_imageEvalTestEnabled = false;
                }
            }
        }

        if (testFpsCalculator.Update(fps))
            postEvent(new FpsEvent(fps, FpsEvent::Fps_Test));

        if (m_channelContext->RenderedEvent->WaitOne(0)) { // Render
            ec = m_sensorImageDecoder->Decode(m_offlineTestRaw, m_frameParam, roiInfos, pRgb24ImgBuffer, rgb24FrameParam);
            if (ec == ERR_NoError) {
                postEvent(new RenderFrameEvent(pRgb24ImgBuffer, rgb24FrameParam));
                if (displayFpsCalculator.Update(fps))
                    postEvent(new FpsEvent(fps, FpsEvent::Fps_Display));
            }
            else
            {
                postEvent(new LoggerEvent(QString("Decode error."), LoggerEvent::Log_Info));
            }
        }
    }
    m_channelContext->RenderedEvent->WaitOne(1000);
    qInfo()<<__FUNCTION__<<"exited.";
}

void TestItemService::postEvent(QEvent *event, int priority)
{
    if ((m_channelContext != nullptr) && (m_channelContext->EventReceiver != nullptr))
        QCoreApplication::postEvent(m_channelContext->EventReceiver, event, priority);
}
