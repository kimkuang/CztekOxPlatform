#include "ChannelController.h"
#include "hal/HalOx.h"
#include "common/Global.h"
#include "common/TlvParser.h"
#include "common/FileIo.h"
#include <QDir>
#include <QFile>
#include <QtEndian>
#include "UiDefs.h"
#include "send_recv_api.h"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
using namespace cv;

std::mutex ChannelController::m_singleInstanceMutex;
QMap<uint, ChannelController *> ChannelController::m_mapChnIdx2ChannelController;
ChannelController *ChannelController::GetInstance(uint chnIdx)
{
    std::lock_guard<std::mutex> locker(m_singleInstanceMutex);
    (void)(locker);
    ChannelController *instance = nullptr;
    if (!m_mapChnIdx2ChannelController.contains(chnIdx)) {
        qDebug("New channel controller...[%d]", chnIdx);
        instance = new ChannelController(chnIdx);
        m_mapChnIdx2ChannelController.insert(chnIdx, instance);
    }
    else {
        instance = m_mapChnIdx2ChannelController[chnIdx];
    }
    return instance;
}

void ChannelController::FreeInstance(uint chnIdx)
{
    std::lock_guard<std::mutex> locker(m_singleInstanceMutex);
    (void)(locker);
    if (m_mapChnIdx2ChannelController.contains(chnIdx)) {
        delete m_mapChnIdx2ChannelController[chnIdx];
        m_mapChnIdx2ChannelController.remove(chnIdx);
    }
}

void ChannelController::FreeAllInstances()
{
    std::lock_guard<std::mutex> locker(m_singleInstanceMutex);
    (void)(locker);
    QMutableMapIterator<uint, ChannelController*> it(m_mapChnIdx2ChannelController);
    while (it.hasNext()) {
        it.next();
        delete it.value();
        it.remove();
    }
}

ChannelController::ChannelController(uint chnIdx)
{
    m_chnIdx = chnIdx;
    m_systemSettings = nullptr;
    m_moduleSettings = nullptr;
    m_sensorSetting.Id = 0;
    qDebug("New sensorbll...[chnIdx: %u]", m_chnIdx);
    m_sensorBll = new SensorBLL();
    qDebug("Initialize Ox platform...[chnIdx: %u]", m_chnIdx);
    m_halService = new HalOx(chnIdx);
    m_halService->SetCaptureTimeout(2000);
    m_testItemService = TestItemService::GetInstance(m_chnIdx);

    qInfo("Newing captue thread...[chnIdx: %u]", m_chnIdx);
    m_captureThread = new CaptureThread(m_chnIdx);
    connect(m_captureThread, &CaptureThread::fpsChanged, this, [&](float fps) {
        postEvent(new FpsEvent(fps, FpsEvent::Fps_Capture));
    });
    connect(m_captureThread, &CaptureThread::errorOccured, this, [&](int ec) {
        QString strLog = tr("Capture image failed.[ec:%1]").arg(ec);
        postEvent(new ExceptionEvent(ExceptionEvent::Exception_ApplicationError, ec, ReservedCategory_VideoInterrupt, strLog));
        CloseCamera(true);
    });
    m_channelContext = nullptr;
    m_capTransBuffer = nullptr;
    setChannelState(ChannelStateEvent::State_PowerOff);

    m_mesResult = true;
    m_bUseMes = false;
    m_bAntiShakeMode = false;
    m_pszRpcResult = nullptr;
    m_fuseId = "";
    m_bNeedReloadSensorParam = false;
}

ChannelController::~ChannelController()
{
    qDebug()<<__FUNCTION__;
    CloseCamera();
    ExitOfflineTest();
    qInfo("Deleting captue thread...[Index: %u]", m_chnIdx);
    delete m_captureThread;
    qInfo("Destroying hal...[Index: %u]", m_chnIdx);
    if (m_halService != nullptr)
        delete m_halService;
    if (m_capTransBuffer != nullptr) {
        qInfo("Deleting capture transmitter buffer pool...[Index: %u]", m_chnIdx);
        delete m_capTransBuffer; m_capTransBuffer = nullptr;
    }
    qInfo("Destorying sensor bll...[chnIdx: %u]", m_chnIdx);
    delete m_sensorBll;

    if(m_pszRpcResult != nullptr)
    {
        delete []m_pszRpcResult;
        m_pszRpcResult = nullptr;
    }
    UnloadSensorParam();
    m_channelContext = nullptr;
}

void ChannelController::BindChannelContext(T_ChannelContext *context)
{
    context->ChannelController = this;
    m_channelContext = context;
    Q_ASSERT(m_channelContext != nullptr);
    m_systemSettings = static_cast<SystemSettings *>(m_channelContext->SystemSettings);
    m_moduleSettings = static_cast<ModuleSettings *>(m_channelContext->ModuleSettings);
    m_testItemService->BindChannelContext(m_channelContext);
}

int ChannelController::LoadSensorParam(const QString &sensorName)
{
    Q_ASSERT(m_channelContext != nullptr);
    UnloadSensorParam();
    T_SENSOR_CFG sensorBo;
    if (sensorName.isEmpty() || !m_sensorBll->LoadSensorParam(sensorBo, sensorName)) {
        postEvent(new LoggerEvent(tr("Loading sensor parameter failed."), LoggerEvent::Log_Fail));
        return ERR_Failed;
    }
    sensorBo2sensorSetting(sensorBo, m_sensorSetting);
    // load image sensor
    m_channelContext->ImageSensor = CreateImageSensor(m_sensorSetting);
    m_channelContext->ImageSensor->BindChannelContext(m_channelContext);

    if (m_moduleSettings->VcmDriverDb.Enabled)
        m_channelContext->VcmDriver = CreateVcmDriver(m_moduleSettings->VcmDriverDb.DriverIcName.toStdString(), m_moduleSettings->VcmDriverDb.I2cAddr);
    else
        m_channelContext->VcmDriver = CreateVcmDriver("NC", 0x00);
    m_channelContext->VcmDriver->BindChannelContext(m_channelContext);

    if (m_moduleSettings->OtpSensorDb.Enabled) {
        m_channelContext->OtpSensor = CreateOtpSensor(m_moduleSettings->OtpSensorDb.OtpSensorName.toStdString(), m_moduleSettings->OtpSensorDb.I2cAddr);
        qDebug("chnIdx: %u, OtpSensor: %p", m_chnIdx, m_channelContext->OtpSensor);
    }
    else
        m_channelContext->OtpSensor = CreateOtpSensor("NC",0x00);
    m_channelContext->OtpSensor->BindChannelContext(m_channelContext);

    m_bNeedReloadSensorParam = false;
    return ERR_NoError;
}

int ChannelController::openCameraWithoutLock()
{
    if (m_channelState >= ChannelStateEvent::State_PowerOnStreamOn) {
        postEvent(new LoggerEvent(tr("The sensor has been stream on"), qRgb(255, 0, 0)));
        return ERR_NoError;
    }

    int ec = 0;
    if (m_bNeedReloadSensorParam) {
        LoadSensorParam(m_moduleSettings->SensorDb.SchemeName);
    }
    if (m_sensorSetting.Id < 1) {
        postEvent(new LoggerEvent(tr("Have not loaded settings!"), LoggerEvent::Log_Fail));
        return ERR_Failed;
    }
    qDebug("Open short...[m_chnIdx: %d]", m_chnIdx);
    QVector<T_ReportData> reportDataList;
    T_ReportData osReportData;
    IHardwareTest *hardwareTest = m_testItemService->GetHardwareTest(IHardwareTest::HardwareTest_OpenShort);
    if (hardwareTest != nullptr) { // open short
        ec = m_testItemService->RunHardwareTestOneTime(hardwareTest);
        osReportData.ErrorCode = ec;
        if (ec < 0) {
            postEvent(new LoggerEvent(tr("O/S failed!"), qRgb(255, 0, 0)));
            return ec;
        }
        PowerOff();
    }
    qDebug("Power on...[m_chnIdx: %d]", m_chnIdx);
    ec = PowerOn();
    if (ec < 0) {
        postEvent(new LoggerEvent(tr("Power on failed!"), qRgb(255, 0, 0)));
        return ec;
    }
    T_PluginInfo pluginInfo;
    if (hardwareTest != nullptr) { // MUST after power on
        if (ERR_NoError == hardwareTest->GetPluginInfo(pluginInfo)) {
            osReportData.Header = hardwareTest->GetReportHeader();
            osReportData.Content = hardwareTest->GetReportContent();
            osReportData.Caption = QString::fromLocal8Bit(pluginInfo.InstanceName);
            reportDataList.push_back(osReportData);
        }
    }

    ec = checkFlagRegisters(m_sensorSetting.I2cParam);
    if (ec < 0) {
        PowerOff();
        return ec;
    }

    hardwareTest = m_testItemService->GetHardwareTest(IHardwareTest::HardwareTest_StandbyCurrent);
    if (hardwareTest != nullptr) { // Standby current
        qDebug("Checking standby current...[Index: %u]", m_chnIdx);
        // Enter standby mode
        qDebug("Batch writting sleep registers ...");
        QList<T_RegConf> regList;
        for (int i = 0; i < m_sensorSetting.SleepParamCount; i++)
            regList.append(m_sensorSetting.SleepParams[i]);
        ec = WriteRegisterList(m_sensorSetting.I2cParam, regList);
        if (ec < 0) {
            PowerOff();
            return ec;
        }

        ushort resetPwdnPin = IO_Reset | IO_Pwdn1;
        ushort resetPwdnLevel = 0;
        if (m_sensorSetting.Reset)
            resetPwdnLevel |= IO_Reset;
        if (m_sensorSetting.Pwdn)
            resetPwdnLevel |= IO_Pwdn1;
        qDebug("SetGpioValue(reversedPwdn: 0x%02x)...", resetPwdnLevel);
        ec = m_halService->SetSensorGpioPinLevel(resetPwdnPin, resetPwdnLevel ^ 0x01); // reverse pwdn
        if (ec < 0) {
            qCritical("Calling SetSensorGpioPinLevel() failed.");
            PowerOff();
            return ec;
        }
        QThread::msleep(100);

        ec = m_testItemService->RunHardwareTestOneTime(hardwareTest);
        if (ec < 0) {
            PowerOff();
            return ec;
        }
        if (ERR_NoError == hardwareTest->GetPluginInfo(pluginInfo)) {
            T_ReportData standbyCurrentReport;
            standbyCurrentReport.Caption = QString::fromLocal8Bit(pluginInfo.InstanceName);
            standbyCurrentReport.Header = hardwareTest->GetReportHeader();
            standbyCurrentReport.Content = hardwareTest->GetReportContent();
            standbyCurrentReport.ErrorCode = ec;
            reportDataList.push_back(standbyCurrentReport);
        }

        qDebug("Restore SetGpioValue(resetPwdnPin: %d)...", resetPwdnLevel);
        ec = m_halService->SetSensorGpioPinLevel(resetPwdnPin, resetPwdnLevel);
        if (ec < 0) {
            PowerOff();
            qCritical()<<"Calling SetSensorGpioPinLevel() failed.";
            return ec;
        }
    }

    ec = StartVideo(true);
    if (ec < 0) {
        PowerOff();
        postEvent(new LoggerEvent(tr("Open camera failed."), LoggerEvent::Log_Fail));
        return ec;
    }

    // GetFuseId
    m_fuseId = GetSensorFuseId();
    postEvent(new LoggerEvent(QObject::tr("Fuse Id: %1").arg(m_fuseId), qRgb(0, 0, 0xff)));
    // Save hardware test report
    foreach (const T_ReportData &report, reportDataList) {
        SaveReport(report.Caption, ITestItem::ItemType_Hardware, report.Header, report.Content,
                   (report.ErrorCode == ERR_NoError));
    }

    if ((m_channelContext != nullptr) && (m_channelContext->VcmDriver != nullptr)) {
        ec = m_channelContext->VcmDriver->VcmInitialize();
        if (ec != ERR_NoError) {
            StopVideo();
            PowerOff();
            postEvent(new LoggerEvent(tr("VCM initialization failed!"), qRgb(0xff, 0, 0)));
            return ec;
        }
    }

    // Start test thread
    if (m_channelState < ChannelStateEvent::State_Playing) {
        ec = m_testItemService->StartTestThread();
        if (ec < 0) {
            StopVideo();
            PowerOff();
            postEvent(new LoggerEvent(tr("Start test thread failed."), LoggerEvent::Log_Fail));
            return ec;
        }
        setChannelState(ChannelStateEvent::State_Playing);
    }
    postEvent(new LoggerEvent(tr("Open camera success."), LoggerEvent::Log_Pass));

    return ec;
}

int ChannelController::OpenCamera()
{
    m_hightlightMessage.clear();
    qDebug("%s try to acquire lock. m_chnIdx: %d", __FUNCTION__, m_chnIdx);
    int ec = ERR_NotFinished;
    if (m_mutex.try_lock_for(std::chrono::milliseconds(3000))) {
        qDebug("%s acquired lock. m_chnIdx: %d", __FUNCTION__, m_chnIdx);
        ec = openCameraWithoutLock();
        m_mutex.unlock();
    }
    else {
        postEvent(new LoggerEvent(tr("Failed to acquire lock in %1.").arg(__FUNCTION__), LoggerEvent::Log_Fail));
        ec = ERR_Failed;
    }
    postEvent(new AsyncInvokeEvent(AsyncInvokeEvent::AsyncCommand_OpenCamera, ec));
    return ec;
}

int ChannelController::closeCameraWithoutLock()
{
    m_testItemService->StopTestThread();
    setChannelState(ChannelStateEvent::State_PowerOnStreamOn);

//    m_future = std::async(std::launch::async, [&]() {
        StopVideo();
        if ((m_channelContext != nullptr) && (m_channelContext->VcmDriver != nullptr))
            m_channelContext->VcmDriver->VcmFinalize();
        m_fuseId = "";
        return PowerOff();
//    });
//    m_future.wait_for(std::chrono::milliseconds(500));

//    return ERR_NoError;
}

int ChannelController::CloseCamera(bool isException)
{
    qDebug("%s try to acquire lock. m_chnIdx: %d", __FUNCTION__, m_chnIdx);
    int ec = ERR_NotFinished;
    if (m_mutex.try_lock_for(std::chrono::milliseconds(3000))) {
        qDebug("%s acquired lock. m_chnIdx: %d", __FUNCTION__, m_chnIdx);
        ec = closeCameraWithoutLock();
        m_mutex.unlock();
    }
    else {
        postEvent(new LoggerEvent(tr("Failed to acquire lock in %1.").arg(__FUNCTION__), LoggerEvent::Log_Fail));
        ec = ERR_Failed;
    }
    int sessionId = AsyncInvokeEvent::AsyncCommand_CloseCamera;
    if (isException)
        sessionId |= AsyncInvokeEvent::AsyncCommand_Exception;
    postEvent(new AsyncInvokeEvent(sessionId, ec));
    return ec;
}

int ChannelController::UnloadSensorParam()
{
    if ((m_channelContext != nullptr) && (m_channelContext->VcmDriver != nullptr)) {
        DestroyVcmDriver(&m_channelContext->VcmDriver);
        m_channelContext->VcmDriver = nullptr;
    }
    if ((m_channelContext != nullptr) && (m_channelContext->OtpSensor != nullptr)) {
        DestroyOtpSensor(&m_channelContext->OtpSensor);
        m_channelContext->OtpSensor = nullptr;
    }
    if ((m_channelContext != nullptr) && (m_channelContext->ImageSensor != nullptr)) {
        DestroyImageSensor(&m_channelContext->ImageSensor);
        m_channelContext->ImageSensor = nullptr;
    }
    freeSensorSetting(m_sensorSetting);
    return ERR_NoError;
}

int ChannelController::PowerOn()
{
    if (m_sensorSetting.Id < 1) {
        postEvent(new LoggerEvent(tr("Have not loaded settings."), LoggerEvent::Log_Fail));
        return ERR_Failed;
    }
    postEvent(new LoggerEvent(tr("Setting sensor power[ChnIdx: %1] ...").arg(m_chnIdx), LoggerEvent::Log_Info));
    QVector<uint> powerIds(m_sensorSetting.PowerCount);
    QVector<uint> voltages(m_sensorSetting.PowerCount);
    QVector<uint> delays(m_sensorSetting.PowerCount);
    for (int i = 0; i < m_sensorSetting.PowerCount; i++) {
        powerIds[i] = m_sensorSetting.Powers[i].Id;
        voltages[i] = m_sensorSetting.Powers[i].Value;
        delays[i] = m_sensorSetting.Powers[i].Delay_ms;
    }
    qDebug()<<powerIds<<voltages<<delays;
    int ec = m_halService->SetSensorPower(powerIds.constData(), voltages.constData(), delays.constData(), m_sensorSetting.PowerCount);
    if (ec < 0) {
        postEvent(new LoggerEvent(tr("Setting sensor power failed.[chnIdx: %1]").arg(m_chnIdx), LoggerEvent::Log_Fail));
        return ec;
    }

    qDebug().noquote() << tr("Setting sensor clock...[chnIdx: %1, val: %2]").arg(m_chnIdx).arg(m_sensorSetting.Mclk_kHz / 100);
    ec = m_halService->SetSensorClock(m_sensorSetting.Mclk_kHz / 100);
    if (ec < 0) {
        postEvent(new LoggerEvent(tr("Setting sensor clock failed."), LoggerEvent::Log_Fail));
        return ec;
    }

    qDebug().noquote() << tr("Setting sensor gpio pin...[chnIdx: %1]").arg(m_chnIdx);
    // set Pwdn valid
    ushort resetPwdnPin = IO_Pwdn1;
    ushort resetPwdnLevel = 0;
    if (m_sensorSetting.Pwdn)
        resetPwdnLevel |= IO_Pwdn1;
    qDebug("ChnIdx: %u, pwdn = %d, pwdnLevel = 0x%x", m_chnIdx, m_sensorSetting.Pwdn, resetPwdnLevel);
    ec = m_halService->SetSensorGpioPinLevel(resetPwdnPin, resetPwdnLevel);
    if (ec < 0) {
        postEvent(new LoggerEvent(tr("Setting sensor pwdn pin failed."), LoggerEvent::Log_Fail));
        return ec;
    }
    QThread::msleep(20); // 最好是从配置文件读取，通常需要10ms左右

    // set Reset invalid
    resetPwdnPin = IO_Reset;
    resetPwdnLevel = IO_Reset;
    if (m_sensorSetting.Reset)
        resetPwdnLevel = 0;
    ec = m_halService->SetSensorGpioPinLevel(resetPwdnPin, resetPwdnLevel);
    if (ec < 0) {
        postEvent(new LoggerEvent(tr("Setting sensor reset pin failed."), LoggerEvent::Log_Fail));
        return ec;
    }
    QThread::msleep(20);

    // set Reset valid
    resetPwdnLevel = 0;
    if (m_sensorSetting.Reset)
        resetPwdnLevel |= IO_Reset;
    qDebug("ChnIdx: %u, reset = %d, restLevel = 0x%x", m_chnIdx, m_sensorSetting.Reset, resetPwdnLevel);
    ec = m_halService->SetSensorGpioPinLevel(resetPwdnPin, resetPwdnLevel);
    if (ec < 0) {
        postEvent(new LoggerEvent(tr("Setting sensor reset pin failed."), LoggerEvent::Log_Fail));
        return ec;
    }
    QThread::msleep(20); // 最好是从配置文件读取，通常需要10ms左右
    setChannelState(ChannelStateEvent::State_PowerOnStreamOff);

    return ec;
}

int ChannelController::StartVideo(bool loadRegisters)
{
    if (m_channelState >= ChannelStateEvent::State_PowerOnStreamOn) {
        postEvent(new LoggerEvent(tr("The sensor has been stream on"), qRgb(255, 0, 0)));
        return ERR_NoError;
    }

    int ec = 0;
    // Enter working mode
    if (loadRegisters) {
        postEvent(new LoggerEvent(tr("Batch writting full mode registers ..."), LoggerEvent::Log_Info));
        QList<T_RegConf> regList;
        for (int i = 0; i < m_sensorSetting.FullModeParamCount; i++)
            regList.append(m_sensorSetting.FullModeParams[i]);
        ec = WriteRegisterList(m_sensorSetting.I2cParam, regList);
        if (ec < 0) {
            qCritical("Writing full mode params failed.");
            postEvent(new LoggerEvent(tr("Batch writting full mode registers failed."), LoggerEvent::Log_Fail));
            return ec;
        }
    }

    qDebug().noquote() << tr("Setting frame parameter...[chnIdx: %1]").arg(m_chnIdx);
    uint outImageFormat = m_sensorSetting.ImageFormat;
    ec = SetFrameParam(m_sensorSetting.ImageFormat, m_sensorSetting.ImageMode, m_sensorSetting.PixelWidth,
                       m_sensorSetting.PixelHeight, outImageFormat, m_sensorSetting.CropParam.X, m_sensorSetting.CropParam.Y,
                       m_sensorSetting.CropParam.Width, m_sensorSetting.CropParam.Height);
    if (ec < 0) {
        postEvent(new LoggerEvent(tr("Setting frame parameter failed."), LoggerEvent::Log_Fail));
        return ec;
    }

    qDebug().noquote() << tr("Setting mipi parameter...[chnIdx: %1]").arg(m_chnIdx);
    ec = m_halService->SetMipiParam(m_sensorSetting.Lanes, m_sensorSetting.MipiFreq, 0);
    if (ec != 0) {
        postEvent(new LoggerEvent(tr("Setting mipi parameter failed."), LoggerEvent::Log_Fail));
        return ec;
    }

    postEvent(new LoggerEvent(tr("Starting video...[chnIdx: %1]").arg(m_chnIdx), LoggerEvent::Log_Info));
    ec = m_halService->SetVideoControl(1);
    if (ec < 0) {
        postEvent(new LoggerEvent(tr("Starting video failed."), LoggerEvent::Log_Fail));
        return ec;
    }

    T_FrameParam frameParam;
    ec = GetFrameParam(frameParam);
    if (ec < 0)
        return ec;
    qDebug("F: %d, M: %d, W: %d, H: %d", frameParam.ImageFormat, frameParam.ImageMode, frameParam.Width, frameParam.Height);
    qInfo("New capture transmitter ping pong buffer...[Size: %d, ChnIdx: %u]", frameParam.Size, m_chnIdx);
    m_capTransBuffer = new PingPongBuffer(frameParam.Size);

    qInfo("Binding capture thread's hal...[Index: %u]", m_chnIdx);
    m_captureThread->BindHal(m_halService);
    qInfo("Binding capture transmitter ping pong buffer...[Index: %u]", m_chnIdx);
    m_captureThread->BindCapTransBufferPool(m_capTransBuffer);
    qInfo("Starting capture thread...[Index: %u]", m_chnIdx);
    m_captureThread->StartRunning();

    setChannelState(ChannelStateEvent::State_PowerOnStreamOn);
    return ec;
}

int ChannelController::StartTest(int groupIdx, int idxInGroup)
{
    int ec = ERR_NoError;
    qDebug("%s try to acquire lock. m_chnIdx: %d", __FUNCTION__, m_chnIdx);
    if (m_mutex.try_lock_for(std::chrono::milliseconds(3000))) {
        qDebug("%s acquired lock. m_chnIdx: %d", __FUNCTION__, m_chnIdx);
        if (m_channelState < ChannelStateEvent::State_Playing) {
            postEvent(new LoggerEvent("The sensor has not open!", qRgb(255, 0, 0)));
            m_mutex.unlock();
            return ERR_Failed;
        }
        m_testItemService->RunImageTest(groupIdx, idxInGroup);
        m_mutex.unlock();
    }
    else {
        postEvent(new LoggerEvent(tr("Failed to acquire lock in %1.").arg(__FUNCTION__), LoggerEvent::Log_Fail));
        ec = ERR_Failed;
    }

    return ec;
}

int ChannelController::PowerOff()
{
    postEvent(new LoggerEvent(tr("Power off...[chnIdx: %1]").arg(m_chnIdx), LoggerEvent::Log_Info));
    // 对于下电延时一般没有要求
    m_halService->SetSensorClock(0);
    m_halService->SetSensorGpioPinLevel(IO_Reset | IO_Pwdn1, 0);
    QVector<uint> powerIds(PowerId_MaxPowerCount);
    QVector<uint> voltages(PowerId_MaxPowerCount);
    QVector<uint> delays(PowerId_MaxPowerCount);
    for (int i = PI_DVDD; i < PowerId_MaxPowerCount; i++) {
        powerIds[i] = i;
        voltages[i] = 0;
        delays[i] = 0;
    }
    m_halService->SetSensorPower(powerIds.constData(), voltages.constData(), delays.constData(), PowerId_MaxPowerCount);
    setChannelState(ChannelStateEvent::State_PowerOff);

    return ERR_NoError;
}

IHalOx *ChannelController::GetHalService()
{
    return m_halService;
}

QString ChannelController::GetBarcode()
{
    char buffer[64];
    int ec = m_halService->GetBarcode(buffer, sizeof(buffer));
    QString barcode;
    if (ERR_NoError == ec)
        barcode = QString::fromLocal8Bit(buffer);
    return barcode;
}

int ChannelController::SetBeepOn(uint ms)
{
    return m_halService->BeepOn(ms);
}

QString ChannelController::GetDeviceVersion()
{
    char buffer[64];
    int ec = m_halService->GetDriverVersion(buffer, sizeof(buffer));
    QString version;
    if (ERR_NoError == ec)
        version = QString::fromLocal8Bit(buffer);
    return version;
}

QString ChannelController::GetHalVersion()
{
    char buffer[64];
    int ec = m_halService->GetHalVersion(buffer, sizeof(buffer));
    QString version;
    if (ERR_NoError == ec)
        version = QString::fromLocal8Bit(buffer);
    return version;
}

int ChannelController::writeSensorMultiRegs(uint slaveAddr, uint speedkHz, uint mode, const uint *pRegParam, int length)
{
//    qDebug("length: %d", length);
    ushort *regAddrs = new ushort[length / 3];
    ushort *regVals = new ushort[length / 3];
    ushort *regDelays = new ushort[length / 3];
    int regIdx = 0;
    int ec = 0;
    for (int i = 0; i < length; i += 3) {
        uint reg = pRegParam[i];
        uint val = pRegParam[i + 1];
        uint delay = pRegParam[i + 2];
        if (reg == ESC_MODE) {
            ec = writeSensorMultiRegsWithDelay(slaveAddr, speedkHz, mode, regAddrs, regVals, regDelays, regIdx);
            regIdx = 0;

            mode = val;
        }
        else if (reg == ESC_ADDR) {
            ec = writeSensorMultiRegsWithDelay(slaveAddr, speedkHz, mode, regAddrs, regVals, regDelays, regIdx);
            regIdx = 0;

            slaveAddr = val;
        }
        else {
            regAddrs[regIdx] = reg;
            regVals[regIdx] = val;
            regDelays[regIdx] = delay;
            regIdx++;
        }
        if (ec != 0)
            break;
    }
    if (regIdx != 0)
        ec = writeSensorMultiRegsWithDelay(slaveAddr, speedkHz, mode, regAddrs, regVals, regDelays, regIdx);
    delete[] regAddrs; delete[] regVals; delete[] regDelays;
    return ec;
}

int ChannelController::writeSensorMultiRegsWithDelay(uint slaveAddr, uint speedkHz, uint mode, const ushort regAddr[],
    const ushort regData[], const ushort regDelay[], int regNum)
{
    if (regNum < 1)
        return ERR_NoError;

    const int MAX_CNT_PER_PACKET = MAX_REG_COUNT_PER_PACKET; //(GvcpPdu::GVCP_MAX_PAYLOAD_SIZE - 4 - 4 - 4) / 4; // Id(4) + Len(4) + Param(4)
    const ushort *pRegAddr = regAddr;
    const ushort *pRegDatas = regData;
    int currPacketRegCnt = 0;
    int ec = ERR_Failed;
    for (int i = 0; i < regNum; i++) {
        currPacketRegCnt++;
        if ((currPacketRegCnt < MAX_CNT_PER_PACKET) && (regDelay[i] == 0) && (i != regNum - 1))
            continue;
        ec = WriteDiscreteI2c(slaveAddr, speedkHz, mode, pRegAddr, pRegDatas, currPacketRegCnt);
        if (ec < 0)
            return ec;
        pRegAddr += currPacketRegCnt; pRegDatas += currPacketRegCnt;
        currPacketRegCnt = 0;
        if (regDelay[i] > 0)
            QThread::msleep(regDelay[i]);
    }
    return ERR_NoError;
}

int ChannelController::WriteDiscreteI2c(uint slaveAddr, uint speedkHz, uint mode, const ushort regAddrs[], const ushort regVals[], uint regNum)
{
    return m_halService->WriteDiscreteI2c(slaveAddr, speedkHz, mode, regAddrs, regVals, regNum);
}

int ChannelController::ReadDiscreteI2c(uint slaveAddr, uint speedkHz, uint mode, const ushort regAddrs[], ushort regVals[], uint regNum)
{
    return m_halService->ReadDiscreteI2c(slaveAddr, speedkHz, mode, regAddrs, regVals, regNum);
}

int ChannelController::WriteContinuousI2c(uint slaveAddr, uint speedkHz, uint regAddr, uint regAddrSize, const uchar *data, uint dataSize)
{
    return m_halService->WriteContinuousI2c(slaveAddr, speedkHz, regAddr, regAddrSize, data, dataSize);
}

int ChannelController::ReadContinuousI2c(uint slaveAddr, uint speedkHz, uint regAddr, uint regAddrSize, uchar *data, uint dataSize)
{
    return m_halService->ReadContinuousI2c(slaveAddr, speedkHz, regAddr, regAddrSize, data, dataSize);
}

int ChannelController::SetFrameParam(uint imageFormat, uint imageMode, uint width, uint height, uint size,
                           uint cropLeft, uint cropTop, uint cropWidth, uint cropHeight)
{
    if (m_channelState >= ChannelStateEvent::State_PowerOnStreamOn)
        CloseCamera();
    return m_halService->SetFrameParam(imageFormat, imageMode, width, height, size, cropLeft, cropTop, cropWidth, cropHeight);
}

int ChannelController::GetFrameParam(uint &imageFormat, uint &imageMode, uint &width, uint &height, uint &size,
                           uint &cropLeft, uint &cropTop, uint &cropWidth, uint &cropHeight)
{
    return m_halService->GetFrameParam(imageFormat, imageMode, width, height, size, cropLeft, cropTop, cropWidth, cropHeight);
}


int ChannelController::GetFrameParam(uint &imageFormat, uint &imageMode, uint &width, uint &height, uint &size)
{
    uint cropLeft, cropTop, cropWidth, cropHeight;
    return this->GetFrameParam(imageFormat, imageMode, width, height, size,
                               cropLeft, cropTop, cropWidth, cropHeight);
}

int ChannelController::GetFrameParam(T_FrameParam &frameParam)
{
    uint imageFormat, imageMode, width, height, size;
    uint cropLeft, cropTop, cropWidth, cropHeight;
    int ec = GetFrameParam(imageFormat, imageMode, width, height, size,
                           cropLeft, cropTop, cropWidth, cropHeight);
    if (ec < 0) {
        return ERR_GetFramParam;
    }
    frameParam.ImageFormat = (E_ImageFormat)imageFormat;
    frameParam.ImageMode = (E_ImageMode)imageMode;
    frameParam.Width = width;
    frameParam.Height = height;
    frameParam.Size = size;
    return ec;
}

int ChannelController::VideoControl(uint ctrl)
{
    int ec = (ctrl >= 1) ? StartVideo(false) : StopVideo();
    if (ctrl > 1)
        setChannelState(ChannelStateEvent::State_Playing);
    return ec;
}

int ChannelController::StopVideo()
{
    if (m_channelState < ChannelStateEvent::State_PowerOnStreamOn)
        return ERR_NoError;

    qInfo("Stopping capture thread...[Index: %u]", m_chnIdx);
    m_captureThread->StopRunning();
    if (m_capTransBuffer != nullptr) {
        qInfo("Delete capture transmitter buffer pool...[Index: %u]", m_chnIdx);
        delete m_capTransBuffer; m_capTransBuffer = nullptr;
    }

    postEvent(new LoggerEvent(tr("Stopping video...[chnIdx: %1]").arg(m_chnIdx), LoggerEvent::Log_Info));
    int ec = m_halService->SetVideoControl(0);
    if (ec < 0) {
        postEvent(new LoggerEvent(tr("Stopping video failed! [chnIdx: %1]").arg(m_chnIdx), qRgb(255, 0, 0)));
    }
    setChannelState(ChannelStateEvent::State_PowerOnStreamOff);
    return ec;
}

int ChannelController::GrabFrame(uchar *pbuffer, int bufferLen, T_FrameParam &/*frameParam*/)
{
    uint size = bufferLen;
    uint64 timestamp = 0;
    if (m_capTransBuffer->ReadBuffer(pbuffer, size, timestamp))
        return 0;
    return ERR_Failed;
}

int ChannelController::DequeueFrameBuffer(int &index, uchar* &pbuffer, uint &size, uint64 &timestamp)
{
    PingPongBuffer::PDataItem pDataItem;
    if (m_capTransBuffer->BeginReadBuffer(index, pDataItem)) {
        pbuffer = pDataItem->PtrAllocatedBuffer;
        size = pDataItem->BufferSize;
        timestamp = pDataItem->Timestamp;
//        qDebug()<<pbuffer<<size<<timestamp;
        return ERR_NoError;
    }
    return ERR_EmptyBufferPool;
}

int ChannelController::EnqueueFrameBuffer(int index)
{
    m_capTransBuffer->EndReadBuffer(index);
    return ERR_NoError;
}

int ChannelController::SetOsTestConfig(uint supplyVol_uV, uint supplyCurrent_uA, const uint pinsId[], const uint openStdVols_uV[], const uint shortStdVols_uV[], uint pinCount)
{
    return m_halService->SetOsTestConfig(supplyVol_uV, supplyCurrent_uA, pinsId, openStdVols_uV, shortStdVols_uV, pinCount);
}

int ChannelController::ReadOsTestResult(const uint pinsId[], uint openVols_uV[], uint shortVol_uV[], uint results[], uint pinCount)
{
    return m_halService->ReadOsTestResult(pinsId, openVols_uV, shortVol_uV, results, pinCount);
}

int ChannelController::GetCurrent(const uint powerIds[], const uint currentRange[], float current_nA[], uint count)
{
    return m_halService->GetCurrent(powerIds, currentRange, current_nA, count);
}

int ChannelController::QueryOverCurrent(uint &count, QString &powerNames)
{
    char powerNameBuff[64];
    int ec = m_halService->QueryOverCurrent(count, powerNameBuff);
    if (ec == ERR_NoError) {
        powerNames = QString::fromLatin1(powerNameBuff);
    }
    return ec;
}

QString ChannelController::GetProjectDir()
{
    return GlobalVars::PROJECT_PATH + m_systemSettings->ProjectName + "/";
}

//int ChannelController::GrabLatestFrame(uchar *pBuffer, int bufferLen, int timeout)
//{
//    if (m_isConsumerRunning && (m_capTransBuffer != nullptr)) {
//        uint64 timestamp = 0, currTimestamp = 0;
//        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
//        std::chrono::steady_clock::time_point t2;
//        uint size = bufferLen;
//        std::int64_t diff = 0;
//        do {
//            if (m_capTransBuffer->ReadBuffer(pBuffer, size, timestamp))
//                return ERR_NoError;
//            t2 = std::chrono::steady_clock::now();
//            diff = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
//            currTimestamp = std::chrono::duration_cast<std::chrono::microseconds>(t2.time_since_epoch()).count();
//            if (diff < timeout)
//                std::this_thread::sleep_for(std::chrono::milliseconds(20));
//        } while ((diff < timeout) && (currTimestamp > timestamp));
//        return ERR_EmptyBufferPool;
//    }
//    return ERR_Failed;
//}

uint64 ChannelController::GetCurrentTimestamp()
{
#if 1
    std::int64_t timestamp = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64 timestamp = tv.tv_sec * 1000000 + tv.tv_usec;
#endif
    return timestamp;
}

int ChannelController::SetCacheData(const QString &key, const QByteArray &data)
{
    m_tempCacheData[key] = data;
    return ERR_NoError;
}

int ChannelController::GetCacheData(const QString &key, QByteArray &data, bool removeIt)
{
    if (!m_tempCacheData.contains(key))
        return ERR_Failed;
    data = m_tempCacheData[key];
    if (removeIt)
        m_tempCacheData.remove(key);
    return ERR_NoError;
}

QString ChannelController::GetSensorFuseId()
{
    if ((m_channelContext->ImageSensor != nullptr) && m_fuseId.isEmpty()) {
        std::string fuseId;
        m_channelContext->ImageSensor->GetSensorFuseId(fuseId, m_moduleSettings->SensorDb.UseMasterI2c);
        m_fuseId = QString::fromStdString(fuseId);
    }
    return m_fuseId;
}

void ChannelController::setChannelState(ChannelStateEvent::E_ChannelState state)
{
    m_channelState = state;
    postEvent(new ChannelStateEvent(state), Qt::HighEventPriority);
}

void ChannelController::sensorBo2sensorSetting(const T_SENSOR_CFG &sensorBo, T_SensorSetting &sensorSetting)
{
    freeSensorSetting(sensorSetting);

    sensorSetting.Id = sensorBo.id;                 // greater than 0
    sensorSetting.SchemeName = new char[sensorBo.SchemeName.size() + 1];
    strcpy(sensorSetting.SchemeName, sensorBo.SchemeName.c_str());
    sensorSetting.ChipName = new char[sensorBo.ChipName.size() + 1];
    strcpy(sensorSetting.ChipName, sensorBo.ChipName.c_str());
    sensorSetting.Mclk_kHz = sensorBo.Mclk_kHz;
    sensorSetting.InterfaceType = sensorBo.InterfaceType;
    sensorSetting.MipiFreq = sensorBo.MipiFreq;
    sensorSetting.Lanes = sensorBo.Lanes;
    sensorSetting.DataWidth = sensorBo.DataWidth;
    sensorSetting.PclkPol = sensorBo.PclkPol;
    sensorSetting.DataPol = sensorBo.DataPol;
    sensorSetting.HsyncPol = sensorBo.HsyncPol;
    sensorSetting.VsyncPol = sensorBo.VsyncPol;
    sensorSetting.Pwdn = sensorBo.pwdn.val;
    sensorSetting.Reset = sensorBo.reset.val;

    sensorSetting.ImageFormat = sensorBo.ImageFormat;
    sensorSetting.ImageMode = sensorBo.ImageMode;
    sensorSetting.PixelWidth = sensorBo.PixelWidth;
    sensorSetting.PixelHeight = sensorBo.PixelHeight;
    sensorSetting.QuickWidth = sensorBo.QuickWidth;
    sensorSetting.QuickHeight = sensorBo.QuickHeight;
    sensorSetting.CropParam = sensorBo.CropParam;
    sensorSetting.I2cParam = sensorBo.CommIntfConf.I2C;

    sensorSetting.FullModeParamCount = (int)sensorBo.FullModeParams.size();
    if (sensorSetting.FullModeParamCount > 0)
        sensorSetting.FullModeParams = new T_RegConf[sensorSetting.FullModeParamCount];
    for (int i = 0; i < sensorSetting.FullModeParamCount; i++) {
        sensorSetting.FullModeParams[i] = sensorBo.FullModeParams[i];
    }
    sensorSetting.OtpInitParamCount = (int)sensorBo.OtpInitParams.size();
    if (sensorSetting.OtpInitParamCount > 0)
        sensorSetting.OtpInitParams = new T_RegConf[sensorSetting.OtpInitParamCount];
    for (int i = 0; i < sensorSetting.OtpInitParamCount; i++) {
        sensorSetting.OtpInitParams[i] = sensorBo.OtpInitParams[i];
    }
    sensorSetting.SleepParamCount = (int)sensorBo.SleepParams.size();
    if (sensorSetting.SleepParamCount > 0)
        sensorSetting.SleepParams = new T_RegConf[sensorSetting.SleepParamCount];
    for (int i = 0; i < sensorSetting.SleepParamCount; i++) {
        sensorSetting.SleepParams[i] = sensorBo.SleepParams[i];
    }
    sensorSetting.AfInitParamCount = (int)sensorBo.AfInitParams.size();
    if (sensorSetting.AfInitParamCount > 0)
        sensorSetting.AfInitParams = new T_RegConf[sensorSetting.AfInitParamCount];
    for (int i = 0; i < sensorSetting.AfInitParamCount; i++) {
        sensorSetting.AfInitParams[i] = sensorBo.AfInitParams[i];
    }
    sensorSetting.AfAutoParamCount = (int)sensorBo.AfAutoParams.size();
    if (sensorSetting.AfAutoParamCount > 0)
        sensorSetting.AfAutoParams = new T_RegConf[sensorSetting.AfAutoParamCount];
    for (int i = 0; i < sensorSetting.AfAutoParamCount; i++) {
        sensorSetting.AfAutoParams[i] = sensorBo.AfAutoParams[i];
    }
    sensorSetting.AfFarParamCount = (int)sensorBo.AfFarParams.size();
    if (sensorSetting.AfFarParamCount > 0)
        sensorSetting.AfFarParams = new T_RegConf[sensorSetting.AfFarParamCount];
    for (int i = 0; i < sensorSetting.AfFarParamCount; i++) {
        sensorSetting.AfFarParams[i] = sensorBo.AfFarParams[i];
    }
    sensorSetting.AfNearParamCount = (int)sensorBo.AfNearParams.size();
    if (sensorSetting.AfNearParamCount > 0)
        sensorSetting.AfNearParams = new T_RegConf[sensorSetting.AfNearParamCount];
    for (int i = 0; i < sensorSetting.AfNearParamCount; i++) {
        sensorSetting.AfNearParams[i] = sensorBo.AfNearParams[i];
    }
#if 0
    sensorSetting.ExposureParamCount = (int)sensorBo.ExposureParam.size();
    if (sensorSetting.ExposureParamCount > 0)
        sensorSetting.ExposureParams = new T_RegConf[sensorSetting.ExposureParamCount];
    for (int i = 0; i < sensorSetting.ExposureParamCount; i++) {
        sensorSetting.ExposureParams[i] = sensorBo.ExposureParam[i];
    }
    sensorSetting.GainParamCount = (int)sensorBo.GainParam.size();
    if (sensorSetting.GainParamCount > 0)
        sensorSetting.GainParams = new T_RegConf[sensorSetting.GainParamCount];
    for (int i = 0; i < sensorSetting.GainParamCount; i++) {
        sensorSetting.GainParams[i] = sensorBo.GainParam[i];
    }
#else
    sensorSetting.ExposureParamCount = 0;
    sensorSetting.GainParamCount = 0;
#endif
    sensorSetting.FlagRegisterCount = (int)sensorBo.FlagRegisters.size();
    if (sensorSetting.FlagRegisterCount > 0)
        sensorSetting.FlagRegisters = new T_RegConf[sensorSetting.FlagRegisterCount];
    for (int i = 0; i < sensorSetting.FlagRegisterCount; i++) {
        sensorSetting.FlagRegisters[i] = sensorBo.FlagRegisters[i];
    }

    sensorSetting.PowerCount = (int)sensorBo.Domains.size();
    if (sensorSetting.PowerCount > 0)
        sensorSetting.Powers = new T_Power[sensorSetting.PowerCount];
    for (int i = 0; i < sensorSetting.PowerCount; i++) {
        sensorSetting.Powers[i] = sensorBo.Domains[i];
    }
}

void ChannelController::freeSensorSetting(T_SensorSetting &sensorSetting)
{
    if (sensorSetting.Id > 0) {
        delete[] sensorSetting.SchemeName;
        delete[] sensorSetting.ChipName;
        if (sensorSetting.FullModeParamCount > 0)
            delete[] sensorSetting.FullModeParams;
        if (sensorSetting.OtpInitParamCount > 0)
            delete[] sensorSetting.OtpInitParams;
        if (sensorSetting.SleepParamCount > 0)
            delete[] sensorSetting.SleepParams;
        if (sensorSetting.AfInitParamCount > 0)
            delete[] sensorSetting.AfInitParams;
        if (sensorSetting.AfAutoParamCount > 0)
            delete[] sensorSetting.AfAutoParams;
        if (sensorSetting.AfFarParamCount > 0)
            delete[] sensorSetting.AfFarParams;
        if (sensorSetting.AfNearParamCount > 0)
            delete[] sensorSetting.AfNearParams;
        if (sensorSetting.ExposureParamCount > 0)
            delete[] sensorSetting.ExposureParams;
        if (sensorSetting.GainParamCount > 0)
            delete[] sensorSetting.GainParams;
        if (sensorSetting.FlagRegisterCount > 0)
            delete[] sensorSetting.FlagRegisters;
        if (sensorSetting.PowerCount > 0)
            delete[] sensorSetting.Powers;
        sensorSetting.Id = 0;
    }
}

int ChannelController::LogToWindow(const QString &text, QRgb rgb)
{
    postEvent(new LoggerEvent(text, rgb));
    return ERR_NoError;
}

void ChannelController::SaveReport(const QString &strItemInstName, int itemType, const QString &strHeader, const QString &strContent, bool result)
{
    if (m_mutex.try_lock_for(std::chrono::milliseconds(3000))) {
        m_mesResult = result;
        saveReportToFile(strItemInstName, strHeader, strContent);
        setMesUpdateData(strItemInstName, itemType, strHeader, strContent);
        m_mutex.unlock();
    }
    else {
        postEvent(new LoggerEvent(tr("Failed to acquire lock in %1.").arg(__FUNCTION__), LoggerEvent::Log_Fail));
    }
}

int ChannelController::SetBindCode(const QString& bindCode)
{
    LogToWindow(QString("").sprintf("BindCode: %s", bindCode.toStdString().c_str()), LogBlue);
    QStringList ltCodeInfo = bindCode.split(':', QString::SkipEmptyParts);
    if (2 != ltCodeInfo.size())
    {
        return ERR_Failed;
    }

    //RunCard + BarCode
    m_bindCode = ltCodeInfo[0] + "," + ltCodeInfo[1];
    return ERR_Failed;
}

QString ChannelController::GetBindCode(void)
{
    return m_bindCode;
}

int ChannelController::GetMesData(int type, QStringList &mesData)
{
    if (!m_bUseMes)    //no MesCheck and has MesUpdate
    {
        mesData.clear();
        return ERR_Failed;
    }

    mesData.clear();
    switch (type) {
    case Mes_Check: { // MES Check
        mesData.push_back(m_fuseId);
        m_imageTestMesData.clear();
        break;
    }
    case Mes_Binding: { // MES Binding
        mesData.push_back(m_fuseId);
        mesData.push_back(m_bindCode);
        m_imageTestMesData.clear();
        break;
    }
    case Mes_Update: { // MES Update: bindCode,fuseId,result[PASS/FAIL],item1:data1;item2:data2;item_n:data_n;
        mesData.push_back(m_bindCode);
        mesData.push_back(m_fuseId);
        mesData.push_back(m_mesResult ? "PASS" : "FAIL");
        QString strDetail = m_imageTestMesData.join("");
        strDetail += m_hardwareTestData.join("");
        mesData.push_back(strDetail);
        m_bindCode.clear();
        m_hardwareTestData.clear(); //需要清空硬件测试数据
        //m_imageTestMesData.clear();  //不能清空，需要多次上传，保证mescheck时清空就行
        break;
    }
    case Mes_Unbinding: {   //MES unbinding
        mesData.push_back(m_fuseId);
        break;
    }
    default:
        return ERR_Failed;
    }

    return ERR_NoError;
}

int ChannelController::SetUseMesFlag(bool flag)
{
    m_bUseMes = flag;
    return ERR_NoError;
}

// call in thread
int ChannelController::RemoteProcedureCall(const QVector<T_TagLenVal> &inParams, QVector<T_TagLenVal> &outParams, int timeoutMs)
{
    TlvParser tlvParser;
    uint32 rpcInPacketLen = tlvParser.CalcTlvBufLength(inParams);
    qDebug("rpcInPacketLen = %d", rpcInPacketLen);
    uchar *rpcInPacket = new uchar[rpcInPacketLen];
    if (nullptr == rpcInPacket) {
        postEvent(new LoggerEvent(QObject::tr("Out of memory. [expected size: %1]").arg(rpcInPacketLen), qRgb(255, 0, 0)));
        return ERR_NO_MEMEORY;
    }
    tlvParser.TlvToBuffer(inParams, rpcInPacket, rpcInPacketLen);

    QString serverIp = m_systemSettings->Middleware.RpcServerIp;
    uint16_t serverPort = m_systemSettings->Middleware.RpcServerPort;
    uint32_t dwResultBufLen = 15*1024*1024;
    if(m_pszRpcResult == nullptr)
    {
        m_pszRpcResult = new char[dwResultBufLen];
        if(m_pszRpcResult == nullptr)
        {
            postEvent(new LoggerEvent(QObject::tr("Out of memory. [expected size: %1]").arg(rpcInPacketLen), qRgb(255, 0, 0)));
            return ERR_NO_MEMEORY;
        }
    }

    QElapsedTimer elapsedTimer;
    postEvent(new LoggerEvent(QObject::tr("RPC IP: %1, port: %2").arg(serverIp).arg(serverPort), qRgb(0, 0, 255)));
    int ec = 0;
    for (int i = 0; i < 3; i++) {
        elapsedTimer.restart();
        ec = RpcSendReq((char *)rpcInPacket, rpcInPacketLen, serverIp.toLatin1().data(), serverPort, timeoutMs, m_pszRpcResult, dwResultBufLen);
        postEvent(new LoggerEvent(QObject::tr("RPC call return code: %1, elapsed time: %2.").arg(ec).arg(elapsedTimer.elapsed()), qRgb(0, 0, 255)));
        if (ec == ERR_NoError)
            break;
        postEvent(new LoggerEvent(QObject::tr("RPC retry...[i: %1]").arg(i), qRgb(0, 0, 255)));
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    outParams.clear();
    if (ec == ERR_NoError) {
//        qDebug() << "Receive finished!" << dwResultBufLen;
        tlvParser.BufferToTlv((const uchar *)m_pszRpcResult, dwResultBufLen, outParams);
    }
    else {
        postEvent(new LoggerEvent(QObject::tr("Network transfer failed!"), qRgb(255, 0, 0)));
    }
    delete[] rpcInPacket;
    return ec;
}

int ChannelController::EnterOfflineTest(const QString &strImgPath, const T_FrameParam &frameParam, uint /*showType*/)
{ 
    int ec = ERR_NoError;
    PowerOn();
    setChannelState(ChannelStateEvent::State_Playing);
    QFileInfo imgInfo(strImgPath);
    QString ext = imgInfo.suffix();
    if (ext == "raw")
    {
        QFile rawFile(strImgPath);
        if (false == rawFile.open(QIODevice::ReadOnly))
        {
            return ERR_FileNotFound;
        }

        uchar *rawBuf = new uchar[frameParam.Size];
        if (nullptr == rawBuf)
        {
            rawFile.close();
            return ERR_NO_MEMEORY;
        }

        if (frameParam.Size != rawFile.read((char*)rawBuf, frameParam.Size))
        {
            rawFile.close();
            delete[] rawBuf;
            return ERR_GEV_READ;
        }

        rawFile.close();
        ec = m_testItemService->EnterOfflineTestThread(rawBuf, frameParam);
        delete[] rawBuf;
        rawBuf = nullptr;
    }
    else
    {
        ushort *rawBuf = new ushort[frameParam.Width * frameParam.Height];
        if (nullptr == rawBuf)
        {
            return ERR_NO_MEMEORY;
        }

        Mat imgMat = imread(strImgPath.toStdString(), IMREAD_COLOR);
        if (IMAGE_MODE_YCbYCr_RG_GB == frameParam.ImageMode)
        {
            RGB24toRawForRGGB(imgMat.data, rawBuf, imgMat.cols, imgMat.rows, frameParam.ImageFormat);
        }
        else if (IMAGE_MODE_YCrYCb_GR_BG == frameParam.ImageMode)
        {
            RGB24toRawForGRBG(imgMat.data, rawBuf, imgMat.cols, imgMat.rows, frameParam.ImageFormat);
        }
        else if (IMAGE_MODE_CbYCrY_GB_RG == frameParam.ImageMode)
        {
            RGB24toRawForGBRG(imgMat.data, rawBuf, imgMat.cols, imgMat.rows, frameParam.ImageFormat);
        }
        else if (IMAGE_MODE_CrYCbY_BG_GR == frameParam.ImageMode)
        {
            RGB24toRawForBGGR(imgMat.data, rawBuf, imgMat.cols, imgMat.rows, frameParam.ImageFormat);
        }

        ec = m_testItemService->EnterOfflineTestThread((const uchar*)rawBuf, frameParam);
        delete[] rawBuf;
        rawBuf = nullptr;
    }

    return ec;
}

int ChannelController::ExitOfflineTest()
{
    PowerOff();
    return m_testItemService->ExitOfflineTestThread();
}

int ChannelController::SaveImage(const QString &imgFullPath, int imgType, const uchar *imgBuf, uint imgSize, bool bDiskOrCloud)
{
    int ec = ERR_NoError;
    switch(imgType)
    {
    case Image_JPG:
    case Image_BMP:
        ec = saveRGB(imgFullPath, imgType);
        break;
    case Image_Raw8:
    case Image_Raw10:
        ec = saveRaw(imgFullPath, imgType);
        break;
    case Image_MyRaw:
        ec = saveRaw(imgFullPath, imgBuf, imgSize);
        break;
    }

    return ec;
}

void ChannelController::SetAntiShakeMode(bool mode)
{
    m_bAntiShakeMode = mode;
}

bool ChannelController::GetAnitShakeMode()
{
    return m_bAntiShakeMode;
}

int ChannelController::HighlightMessage(const QString &message)
{
    m_hightlightMessage = message;
    return ERR_NoError;
}

int ChannelController::GetSensorSettingNameList(QStringList &settingNameList)
{
    settingNameList.clear();
    QList<Sensor> sensorList;
    if ((m_sensorSetting.Id > 0) && !m_sensorBll->SelectByChipNameI2cAddr(sensorList, m_sensorSetting.ChipName, m_sensorSetting.I2cParam.Addr)) {
        qCritical()<<"Loading sensor list by chip name i2c address failed.";
        LogToWindow(QObject::tr("Loading sensor list by chip name and i2c address failed."), qRgb(0xff, 0, 0));
        return ERR_Failed;
    }
    for (int i = 0; i < sensorList.size(); i++) {
        settingNameList.append(sensorList[i].SchemeName);
    }
    return ERR_NoError;
}

int ChannelController::SwitchSensorSetting(const QString &schemeName)
{    
    if (!m_channelContext->RenderedEvent->WaitOne(3000)) {
        LogToWindow("Wait for renderedEvent timeout!", qRgb(0, 0, 0x80));
    }
    // stream off
    int ec = StopVideo();
    if (ec < 0){
        LogToWindow("StopVideo Fail", qRgb(255, 0, 0));
        return ec;
    }

    ec = LoadSensorParam(schemeName);
    if (ec < 0) {
        LogToWindow(QObject::tr("Loading setting[%1] failed.").arg(schemeName), qRgb(0xff, 0, 0));
        return ERR_Failed;
    }
    PowerOn(); // re-power on
    // stream on
    ec = StartVideo(true);
    if (ec < 0){
        LogToWindow("StartVideo Fail", qRgb(255, 0, 0));
        return ec;
    }
    setChannelState(ChannelStateEvent::State_Playing);
    LogToWindow("StartVideo successful", qRgb(0, 0, 0x80));
    m_channelContext->RenderedEvent->Set();
    m_bNeedReloadSensorParam = true;

    return ERR_NoError;
}

int ChannelController::PauseCapture()
{
    m_captureThread->Pause();
    return ERR_NoError;
}

int ChannelController::ResumeCapture()
{
    m_captureThread->Resume();
    return ERR_NoError;
}

IOtpSensor *ChannelController::GetOtpSensor()
{
    return m_channelContext->OtpSensor;
}

void ChannelController::saveReportToFile(const QString &strItemInstName, const QString &strHeader, const QString &strContent)
{
    if (!m_systemSettings->Middleware.bSaveData)
    {
        LogToWindow("Test Data No Save", qRgb(255, 0, 0));
        return;
    }
    QString strReportPath = QString("%1/%2/Cam%3").arg(m_systemSettings->Middleware.ReportPath)
        .arg(m_systemSettings->ProjectName).arg(m_chnIdx);
    QDir dir(strReportPath);
    if (!dir.exists())
    {
        if(!dir.mkpath(strReportPath)) {
            LogToWindow("Report Data Save Fail", qRgb(0xff, 0, 0));
            return;
        }
    }
    QString fileName = QString("%1/%2_%3.csv").arg(strReportPath)
        .arg(m_moduleSettings->SensorDb.SchemeName).arg(strItemInstName);

    QString title = QString("Fuse ID,Test Time,Test Result,%1\n").arg(strHeader);
    QString strLine = QString("%1#,%2,%3,%4\n").arg(m_fuseId)
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
            .arg(m_mesResult ? "Pass" : "NG").arg(strContent);
    if (FileIo::SaveReport(fileName, title, QStringList() << strLine))
    {
        LogToWindow("Test Data Saved OK", qRgb(0, 0x80, 0));
    }
    else
    {
        LogToWindow("Test Data No Save", qRgb(255, 0, 0));
    }
}

void ChannelController::setMesUpdateData(const QString &strItemInstName, int itemType, const QString &strHeader, const QString &strContent)
{
    if (!m_bUseMes)
        return;

    QStringList lstHeader = strHeader.split(QChar(','), QString::SkipEmptyParts);
    QStringList lstContent = strContent.split(QChar(','), QString::SkipEmptyParts);
    if (lstHeader.size() != lstContent.size())
    {
        qCritical() << lstHeader << lstContent;
        LogToWindow("The Number Of Test Item Is Not Equal", qRgb(255, 0, 0));
    }
    else
    {
        QString strLine = "";
        if ((lstContent.size() < 1) || ((1 == lstContent.size()) && (lstContent[0].isEmpty())))
        {
            strLine += QString("%1_Result:%2;").arg(strItemInstName).arg(m_mesResult ? "PASS" : "FAIL");
        }
        else
        {
            for (int i = 0; i < lstContent.size(); ++i)
            {
                strLine += QString("%1:%2;").arg(lstHeader[i]).arg(lstContent[i]);
            }
        }
        strLine += QString("%1:%2;").arg(strItemInstName).arg(m_mesResult ? "OK" : "NG");

        if (ITestItem::ItemType_Hardware == itemType)
        {
            m_hardwareTestData << strLine;
        }
        else //if (ITestItem::ItemType_ImageEvaluation == itemType)
        {
            m_imageTestMesData << strLine;
        }
    }
}

int ChannelController::saveRGB(const QString &imgFullPath, int /*imgType*/)
{
    T_FrameParam frameParam;
    if (ERR_NoError != GetFrameParam(frameParam))
    {
        return ERR_GetFramParam;
    }

    int bufIdx = 0;
    uint size = 0;
    uint64 timestamp = 0;
    uchar *pSensorImgBuffer = nullptr;
    while (ERR_NoError != DequeueFrameBuffer(bufIdx, pSensorImgBuffer, size, timestamp))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(0));
        continue;
    }

    bool bRet = false;
    if (IMAGE_FMT_RAW8 == frameParam.ImageFormat)
    {
        Mat rawMat(frameParam.Height, frameParam.Width, CV_8UC1, pSensorImgBuffer);
        Mat rgbMat;
        if (IMAGE_MODE_YCbYCr_RG_GB == frameParam.ImageMode)
        {
            cvtColor(rawMat, rgbMat, COLOR_BayerRG2BGR);
        }
        else if (IMAGE_MODE_YCrYCb_GR_BG == frameParam.ImageMode)
        {
            cvtColor(rawMat, rgbMat, COLOR_BayerGR2BGR);
        }
        else if (IMAGE_MODE_CbYCrY_GB_RG == frameParam.ImageMode)
        {
            cvtColor(rawMat, rgbMat, COLOR_BayerGB2BGR);
        }
        else if (IMAGE_MODE_CrYCbY_BG_GR == frameParam.ImageMode)
        {
            cvtColor(rawMat, rgbMat, COLOR_BayerBG2BGR);
        }

        bRet = imwrite(imgFullPath.toStdString(), rgbMat);
    }
    else if (IMAGE_FMT_RAW10 == frameParam.ImageFormat)
    {
        Mat rawMat(frameParam.Height, frameParam.Width, CV_16UC1, pSensorImgBuffer);
        Mat rgbMat;
        if (IMAGE_MODE_YCbYCr_RG_GB == frameParam.ImageMode)
        {
            cvtColor(rawMat, rgbMat, COLOR_BayerRG2BGR);
        }
        else if (IMAGE_MODE_YCrYCb_GR_BG == frameParam.ImageMode)
        {
            cvtColor(rawMat, rgbMat, COLOR_BayerGR2BGR);
        }
        else if (IMAGE_MODE_CbYCrY_GB_RG == frameParam.ImageMode)
        {
            cvtColor(rawMat, rgbMat, COLOR_BayerGB2BGR);
        }
        else if (IMAGE_MODE_CrYCbY_BG_GR == frameParam.ImageMode)
        {
            cvtColor(rawMat, rgbMat, COLOR_BayerBG2BGR);
        }

        bRet = imwrite(imgFullPath.toStdString(), rgbMat);
    }
    else
    {
        LogToWindow("Can not save Raw", qRgb(255, 0, 0));
    }

    if (false == bRet)
    {
        LogToWindow(QString("Save %1 failed.").arg(imgFullPath), qRgb(255, 0, 0));
    }

    return EnqueueFrameBuffer(bufIdx);
}

int ChannelController::saveRaw(const QString &imgFullPath, int imgType)
{
    T_FrameParam frameParam;
    if (ERR_NoError != GetFrameParam(frameParam))
    {
        return ERR_GetFramParam;
    }

    int bufIdx = 0;
    uint size = 0;
    uint64 timestamp = 0;
    uchar *pSensorImgBuffer = nullptr;
    int ec = ERR_NoError;
    while (ERR_NoError != DequeueFrameBuffer(bufIdx, pSensorImgBuffer, size, timestamp))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(0));
        continue;
    }

    if (IMAGE_FMT_RAW8 == frameParam.ImageFormat)
    {
        if (Image_Raw8 == imgType)
        {
            ec = saveRaw(imgFullPath, pSensorImgBuffer, frameParam.Size);
        }
        else
        {
            uchar* pImg = new uchar[frameParam.Size*2];
            if (nullptr == pImg)
            {
                return ERR_Failed;
            }

            Raw8ToRaw(pSensorImgBuffer, pImg, frameParam.Width, frameParam.Height, 10);
            ec = saveRaw(imgFullPath, pImg, frameParam.Size*2);
            delete[] pImg;
            pImg = nullptr;
        }
    }
    else if (IMAGE_FMT_RAW10 == frameParam.ImageFormat)
    {
        if (Image_Raw10 == imgType)
        {
            ec = saveRaw(imgFullPath, pSensorImgBuffer, frameParam.Size);
        }
        else
        {
            uchar* pImg = new uchar[frameParam.Size/2];
            if (nullptr == pImg)
            {
                return ERR_Failed;
            }

            RawToRaw8(pSensorImgBuffer, pImg, frameParam.Width, frameParam.Height, 10);
            ec = saveRaw(imgFullPath, pImg, frameParam.Size/2);
            delete[] pImg;
            pImg = nullptr;
        }
    }
    else
    {
        LogToWindow("Can not save Raw", qRgb(255, 0, 0));
    }

    if (ERR_NoError != ec)
    {
        EnqueueFrameBuffer(bufIdx);
        return ec;
    }

    return EnqueueFrameBuffer(bufIdx);
}

int ChannelController::saveRaw(const QString &imgFullPath, const uchar *imgBuf, uint imgSize)
{
    QFile rawFile(imgFullPath);
    if (false == rawFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return ERR_FileNotFound;
    }

    if (imgSize != (uint)rawFile.write((const char*)imgBuf, imgSize))
    {
        rawFile.close();
        return ERR_GEV_WRITE;
    }

    rawFile.close();
    return ERR_NoError;
}

void ChannelController::RawToRaw8(const uchar *pSrc, uchar *pDest, int width, int height, uchar bits)
{
    if ((nullptr == pSrc) || (nullptr == pDest))
    {
        return;
    }

    if (bits <= 8)
    {
        return;
    }

    uchar shift = bits -8;
    const ushort *pImg = (const ushort*)pSrc;
    for (int s = 0; s<width*height; s++)
    {
        pDest[s] = (pImg[s]>>shift)&0xff;
    }
}

void ChannelController::Raw8ToRaw(const uchar *pSrc, uchar *pDest, int width, int height, uchar bits)
{
    if ((nullptr == pSrc) || (nullptr == pDest))
    {
        return;
    }

    if (bits < 16)
    {
        return;
    }

    ushort *pImg = (ushort*)pDest;
    for (int s = 0; s<width*height; s++)
    {
        pImg[s] = pSrc[s];
    }
}

int ChannelController::WriteRegisterList(const T_I2CCommParam &i2cParam, const QList<T_RegConf> &regConfs)
{
    int ec = 0;
    if (regConfs.size() > 0) {
        QVector<uint> regParam;
        for (int i = 0; i < regConfs.size(); i++) {
            regParam.push_back(regConfs[i].Addr);
            regParam.push_back(regConfs[i].Value);
            regParam.push_back(regConfs[i].Delay_ms);
        }
        ec = writeSensorMultiRegs(i2cParam.Addr, i2cParam.Speed * 100, i2cParam.RegBitsMode,
                                      regParam.constData(), regParam.size());
        if (ec < 0) {
            qCritical("Writing register list failed.");
            return ec;
        }
    }
    return ec;
}

int ChannelController::checkFlagRegisters(const T_I2CCommParam &i2cParam)
{
    QVector<ushort> regs, vals, masks;
    QVector<ushort> rdVals(m_sensorSetting.FlagRegisterCount);
    for (int i = 0; i < m_sensorSetting.FlagRegisterCount; i++) {
        regs.push_back(m_sensorSetting.FlagRegisters[i].Addr);
        vals.push_back(m_sensorSetting.FlagRegisters[i].Value);
        masks.push_back(m_sensorSetting.FlagRegisters[i].Mask);
    }
    int ec = 0;
    if (m_sensorSetting.FlagRegisterCount > 0) {
        ec = ReadDiscreteI2c(i2cParam.Addr, i2cParam.Speed * 100, i2cParam.RegBitsMode,
                                regs.constData(), rdVals.data(), m_sensorSetting.FlagRegisterCount);
        if (ec < 0) {
            postEvent(new LoggerEvent(tr("Read flag registers failed[%1].").arg(ec), LoggerEvent::Log_Fail));
            qCritical("Read flag registers failed[%d].", ec);
            return ec;
        }
        for (int i = 0; i < m_sensorSetting.FlagRegisterCount; i++) {
            if ((vals[i] & masks[i]) != (rdVals[i] & masks[i])) {
                postEvent(new LoggerEvent(tr("Dismatched flags. i: %1 vals[i]: %2, rdVals[i]: %3").arg(i).arg(vals[i]).arg(rdVals[i]), LoggerEvent::Log_Fail));
                qCritical("Dismatched flags. i: %d vals[i]: %u, rdVals[i]: %u", i, vals[i], rdVals[i]);
                return ERR_DismatchedFlags;
            }
        }
    }
    return ec;
}

bool ChannelController::RGB24toRawForBGGR(const uchar * pSrc, ushort *pDest, int width, int height, int outformat)
{
    if (!pSrc) return false;
    if (!pDest) return false;
    uchar bits = outformat * 2;
    for (int i = 0; i < height; i += 2)
    {
        for (int j = 0; j < width; j += 2)
        {
            //B
            pDest[i*width + j]        = pSrc[i*width*3 + j*3] << bits;
            //Gb
            pDest[i*width + (j+1)]    = pSrc[i*width*3 + (j+1)*3 + 1] << bits;
            //Gr
            pDest[(i+1)*width + j]    = pSrc[(i+1)*width*3 + j*3 + 1] << bits;
            //R
            pDest[(i+1)*width +(j+1)] = pSrc[(i+1)*width*3 + (j+1)*3 + 2] << bits;
        }
    }

    return true;
}

bool ChannelController::RGB24toRawForGRBG(const uchar * pSrc, ushort *pDest, int width, int height, int outformat)
{
    if (!pSrc) return false;
    if (!pDest) return false;
    uchar bits = outformat * 2;
    for (int i = 0; i < height; i += 2)
    {
        for (int j = 0; j < width; j+=2)
        {
            //Gr
            pDest[i*width + j] = pSrc[i*width*3 + j*3 + 1] << bits;
            //R
            pDest[i*width + (j+1)] = pSrc[i*width*3 + (j+1)*3 + 2] << bits;
            //B
            pDest[(i+1)*width + j] = pSrc[(i+1)*width*3 +j*3] << bits;
            //Gb
            pDest[(i+1)*width +(j+1)] = pSrc[(i+1)*width*3 + (j+1)*3 + 1] << bits;
        }
    }

    return true;
}

bool ChannelController::RGB24toRawForGBRG(const uchar * pSrc, ushort *pDest, int width, int height, int outformat)
{
    if (!pSrc) return false;
    if (!pDest) return false;
    uchar bits = outformat * 2;
    for (int i = 0; i < height; i += 2)
    {
        for (int j = 0; j < width; j += 2)
        {
            //Gb
            pDest[i*width + j] = pSrc[i*width*3 + j*3 + 1] << bits;
            //B
            pDest[i*width + (j+1)] = pSrc[i*width*3 + (j+1)*3] << bits;
            //R
            pDest[(i+1)*width + j] = pSrc[(i+1)*width*3 +j*3 + 2] << bits;
            //Gr
            pDest[(i+1)*width +(j+1)] = pSrc[(i+1)*width*3 + (j+1)*3 + 1] << bits;
        }
    }

    return false;
}

bool ChannelController::RGB24toRawForRGGB(const uchar * pSrc, ushort *pDest, int width, int height, int outformat)
{
    if (!pSrc) return false;
    if (!pDest) return false;
    uchar bits = outformat * 2;
    for (int i = 0; i < height; i += 2)
    {
        for (int j = 0; j < width; j += 2)
        {
            //R
            pDest[i*width + j] = pSrc[i*width*3 + j*3 + 2] << bits;
            //Gr
            pDest[i*width + (j+1)] = pSrc[i*width*3 + (j+1)*3 + 1] << bits;
            //Gb
            pDest[(i+1)*width + j] = pSrc[(i+1)*width*3 +j*3 + 1] << bits;
            //B
            pDest[(i+1)*width +(j+1)] = pSrc[(i+1)*width*3 + (j+1)*3] << bits;
        }
    }

    return true;
}

void ChannelController::postEvent(QEvent *event, int priority)
{
    if ((m_channelContext != nullptr) && (m_channelContext->EventReceiver != nullptr)) {
        QCoreApplication::postEvent(m_channelContext->EventReceiver, event, priority);
    }
}
