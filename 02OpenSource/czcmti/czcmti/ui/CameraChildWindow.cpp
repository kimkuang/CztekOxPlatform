#include "CameraChildWindow.h"
#include "ui_CameraChildWindow.h"
#include <QSettings>
#include <QMessageBox>
#include <QFileInfo>
#include <QtConcurrent>
#include <QTimer>
#include <QCryptographicHash> 
#include "conf/MachineSettings.h"
#include "service/MachineService.h"
#include "SelectSensorDialog.h"
#include "HardwareTestDialog.h"
#include "DecoderDialog.h"
#include "TestItemManagementDialog.h"
#include "TestItemSettingDialog.h"
#include "TestItemButton.h"
#include "UiDefs.h"
#include "I2cDebugDialog.h"
#include "RawOptionDlg.h"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
using namespace cv;

CameraChildWindow::CameraChildWindow(uint windowIdx, uint chnIdx, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CameraChildWindow)
{
    ui->setupUi(this);
    ui->comboBoxShowMode->addItems(QStringList() << "-- 无 --" << "--九宫格--" << "--米字格--" << "--田字格--" << "--平行线--");
    ui->comboBoxShowMode->setCurrentIndex(0);
    m_windowIdx = windowIdx;
    readSettings();
    connectActions();
    initTextPanelContextMenu();

    m_chnIdx = 0xff;
    m_sysSettings = SystemSettings::GetInstance();
    loadModuleSettings();
    m_channelContext.ChnIdx = chnIdx;
    m_channelContext.RenderedEvent = new SyncEvent(true, false);
    ui->m_openGLWidget->SetSemaphoreEvent(m_channelContext.RenderedEvent);
    m_channelContext.EventReceiver = this;
    m_channelContext.MachineSettings = MachineSettings::GetInstance();
    m_channelContext.SystemSettings = SystemSettings::GetInstance();
    m_channelContext.ChannelControllerVector.resize(m_sysSettings->CameraNumber);
    for (uint idx = 0; idx < (uint)m_sysSettings->CameraNumber; idx++) {
        m_channelContext.ChannelControllerVector[idx] = ChannelController::GetInstance(idx);
    }
    m_channelState = ChannelStateEvent::State_PowerOff;
    m_testLastCategoryCode = ReservedCategory_NoError;
    m_isDiskFull = false;
    m_isLoadTestItemOver = true;
    bindChannelIndex(chnIdx);
}

CameraChildWindow::~CameraChildWindow()
{
    writeSettings();
    m_moduleSettings->WriteSettings();
    m_moduleSettings = nullptr;
    m_sysSettings = nullptr;

    delete m_channelContext.RenderedEvent;
    delete m_textPanelContextMenu;
    m_channelController = nullptr;
    delete ui;
}

void CameraChildWindow::bindChannelIndex(uint chnIdx)
{
    if (chnIdx != m_chnIdx) {
        m_chnIdx = chnIdx;
        ui->m_openGLWidget->RenderText(((m_chnIdx == 0xff) ? "" : QString(QChar('A' + m_chnIdx))), qRgb(0, 0, 0x80), QColor(Qt::darkGray).rgb());

        m_channelController = ChannelController::GetInstance(m_chnIdx);
        m_channelController->BindChannelContext(&m_channelContext);
        loadSensorParam();
        m_testItemService = TestItemService::GetInstance(m_chnIdx);
        qDebug() << "m_chnIdx:" << m_chnIdx << (m_testItemService != nullptr);
        loadHardwareTestItems();
        loadImageTestItems();
        loadMachineService();

        DecoderService::GetInstance(m_chnIdx)->SetCurrentDecoderName(m_moduleSettings->ImageDecoder);
        updateDisplayStatus();
    }
}

void CameraChildWindow::OpenCameraAsync()
{
    if (m_isDiskFull) {
        ui->m_openGLWidget->RenderText(tr("Disk Full"), qRgb(0, 0, 0), qRgb(255, 0, 0));
        return;
    }

    if (false == m_isLoadTestItemOver) {
        ui->m_openGLWidget->RenderText(tr("TestItem Plugin Missed"), qRgb(0, 0, 0), qRgb(255, 0, 0));
        return;
    }

    // clear ui
//    ui->m_listWdtLog->clear();
    m_channelContext.RenderedEvent->Set();
    ui->m_openGLWidget->RenderText(((m_chnIdx == 0xff) ? "" : QString(QChar('A' + m_chnIdx))), qRgb(0, 0, 0x80), QColor(Qt::darkGray).rgb());
    refreshTestButtons();
    qDebug().noquote() << QString("====== %1[%2] ======").arg(m_moduleSettings->SensorDb.SchemeName).arg(m_chnIdx);

    // non ui
    clearLastTestResult();
    QtConcurrent::run(m_channelController, &ChannelController::OpenCamera);
}

int CameraChildWindow::StartGroupTest(int groupIdx, bool &isAsync)
{
    isAsync = false;
    // check the history result
    int idxInGroup = 0;
    bool hasError = false;
    bool hasMesUpdate = false;
    int mesUpdateGroupIdx = 0;
    QMapIterator<int, QVector<T_ImageTestItem>> itGroup(m_mapGroup2ImageTestItem);
    while (itGroup.hasNext()) {
        itGroup.next();
        if (!hasError && (groupIdx == itGroup.key())) // the right index
            break;
        QVectorIterator<T_ImageTestItem> itImageTestItem(itGroup.value());
        while (itImageTestItem.hasNext()) {
            const T_ImageTestItem &imgTestItem = itImageTestItem.next();
            idxInGroup = imgTestItem.IdxInGroup;
//            qDebug()<<imgTestItem.FullInstanceName<<imgTestItem.GroupIdx<<imgTestItem.IdxInGroup<<imgTestItem.ErrorCode;
            if (imgTestItem.ErrorCode != ERR_NoError) {
                qDebug()<<imgTestItem.FullInstanceName<<imgTestItem.GroupIdx<<imgTestItem.IdxInGroup<<imgTestItem.ErrorCode;
                hasError = true;
            }
            if (imgTestItem.IsMesUpdate) {
                hasMesUpdate = true;
                mesUpdateGroupIdx = imgTestItem.GroupIdx;
                break;
            }
        }
        if (itGroup.key() >= groupIdx) // exceeds the command's group index
            break;
    }

    qDebug()<<hasError<<hasMesUpdate<<groupIdx<<idxInGroup;
    bool isEmptyGroup = (m_mapGroup2ImageTestItem[groupIdx].size() == 0);
    if (hasError) // has error
    {
        if (isEmptyGroup) {
            WriteLog(tr("Pre-group has error, this group[%1] is empty.").arg(groupIdx), qRgb(255, 0, 0));
            return ERR_Failed;
        }
        if (hasMesUpdate && (groupIdx == mesUpdateGroupIdx)) {
            int ec = m_channelController->StartTest(groupIdx, idxInGroup);
            isAsync = (ec == ERR_NoError);
            return ec;
        }
        else {
            WriteLog(tr("Pre-group has error."), qRgb(255, 0, 0));
            return ERR_Failed;
        }
    }
    else
    {
        if (isEmptyGroup)
            return ERR_NoError;
        else {
            if (m_mapGroup2ImageTestItem[groupIdx][0].ErrorCode != ERR_NotFinished) { // 不能重复测试
                WriteLog(tr("Handler command repeat. Ignore...groupIdx: %1").arg(groupIdx), qRgb(255, 0, 0));
                return ERR_Failed;
            }
            else {
                int ec = m_channelController->StartTest(groupIdx, 0);
                isAsync = (ec == ERR_NoError);
                return ec;
            }
        }
    }
}

void CameraChildWindow::CloseCameraAsync()
{
    QtConcurrent::run(m_channelController, &ChannelController::CloseCamera, false);
}

int CameraChildWindow::RefreshUi()
{
    auto fnRefreshUi = [this](bool flag, const QString &errMsg) {
        QString text = "PASS";
        QRgb background = QColor(Qt::green).rgb();
        if (!flag) {
            background = QColor(Qt::red).rgb();
            text = errMsg;
        }
        QString highlightMessage = m_channelController->m_hightlightMessage;
        ui->m_openGLWidget->RenderText(QString("%1\n%2").arg(text).arg(highlightMessage), qRgb(0, 0, 0), background);
    };

    uint categoryCode = 0;
    // Open camera
    if (m_mapCmd2CategoryCode.contains(MachineCommand_OpenCamera)) {
        categoryCode = m_mapCmd2CategoryCode[MachineCommand_OpenCamera];
    }
    else {
        categoryCode = ReservedCategory_OpenCamera;
        WriteLog(tr("Miss open camera!"), qRgb(255, 0, 0));
    }
    if (categoryCode != ReservedCategory_NoError) {
        fnRefreshUi(false, getCategoryMessage(categoryCode));
        return ERR_NoError;
    }

    // travel all test items
    int elapsed = 0;
    bool flag = true;
    if (m_mapGroup2ImageTestItem.size() > 0) {
        QMapIterator<int, QVector<T_ImageTestItem>> itGroup(m_mapGroup2ImageTestItem);
        while (itGroup.hasNext() && flag) {
            itGroup.next();
            QVectorIterator<T_ImageTestItem> itImageTestItem(itGroup.value());
            while (itImageTestItem.hasNext()) {
                const T_ImageTestItem &imgTestItem = itImageTestItem.next();
//                qDebug()<<imgTestItem.FullInstanceName<<imgTestItem.ErrorCode<<imgTestItem.CategoryCode;
                elapsed += imgTestItem.Elapsed;
                if (imgTestItem.ErrorCode != ERR_NoError) {
                    if (imgTestItem.ErrorCode == ERR_NotFinished)
                        categoryCode = ReservedCategory_NotFinished;
                    else
                        categoryCode = imgTestItem.CategoryCode;
                    flag = false;
                    break;
                }
            }
        }
    }
    else {
        categoryCode = ReservedCategory_NotFinished;
    }
    ui->m_lblTotalElapsed->setText(tr("%L1ms").arg(elapsed));
    if (categoryCode != ReservedCategory_NoError) {
        fnRefreshUi(false, getCategoryMessage(categoryCode));
        return ERR_NoError;
    }

    // Close camera
    if (m_mapCmd2CategoryCode.contains(MachineCommand_CloseCamera)) {
        categoryCode = m_mapCmd2CategoryCode[MachineCommand_CloseCamera];
    }
    else {
        categoryCode = ReservedCategory_CloseCamera;
        WriteLog(tr("Miss close camera!"), qRgb(255, 0, 0));
    }
    fnRefreshUi((categoryCode == ReservedCategory_NoError), getCategoryMessage(categoryCode));

    return ERR_NoError;
}

void CameraChildWindow::WriteLog(const QString &message, QRgb rgb)
{
    LoggerEvent evt(message, rgb);
    loggerEvent(&evt);
}

void CameraChildWindow::QueryOverCurrent()
{
    uint count;
    QString powerNames;
    if ((ERR_NoError == m_channelController->QueryOverCurrent(count, powerNames)) && (count > 0)) {
        WriteLog(tr("Overcurrent!!![%1]").arg(powerNames), qRgb(255, 0, 0));
    }
}

void CameraChildWindow::customEvent(QEvent *event)
{
    if (event->type() == RenderFrameEvent::type()) {        
        renderFrameEvent(static_cast<RenderFrameEvent *>(event));
    }
    else if (event->type() == ExceptionEvent::type()) {
        exceptionEvent(static_cast<ExceptionEvent *>(event));
    }
    else if (event->type() == FpsEvent::type()) {
        fpsChangedEvent(static_cast<FpsEvent *>(event));
    }
    else if (event->type() == LoggerEvent::type()) {
        loggerEvent(static_cast<LoggerEvent *>(event));
    }
    else if (event->type() == ChannelStateEvent::type()) {
        channelStateEvent(static_cast<ChannelStateEvent *>(event));
    }
    else if (event->type() == ActionEvent::type()) {
        _actionEvent(static_cast<ActionEvent *>(event));
    }
    else if (event->type() == TestResultEvent::type()) {
        testResultEvent(static_cast<TestResultEvent *>(event));
    }
    else if (event->type() == TestItemStateChangedEvent::type()) {
        testItemStateChangedEvent(static_cast<TestItemStateChangedEvent *>(event));
    }
    else if (event->type() == MachineRequestEvent::type()) {
        machineRequestEvent(static_cast<MachineRequestEvent *>(event));
    }
    else if (event->type() == MachineResponseEvent::type()) {
        machineResponseEvent(static_cast<MachineResponseEvent *>(event));
    }
    else if (event->type() == MachineAntiShakeEvent::type())
    {
        machineAntiShakeEvent(static_cast<MachineAntiShakeEvent *>(event));
    }
    else if (event->type() == AsyncInvokeEvent::type()) {
        asyncInvokeEvent(static_cast<AsyncInvokeEvent *>(event));
    }
    else if (event->type() == DiskSizeChangedEvent::type()) {
        diskSizeChangedEvent(static_cast<DiskSizeChangedEvent *>(event));
    }
    else
        QMainWindow::customEvent(event);
}

void CameraChildWindow::renderFrameEvent(RenderFrameEvent *event)
{
    if (m_channelState != ChannelStateEvent::State_Playing)
        return;
    ui->m_openGLWidget->RenderFrame(event->RgbBuffer, event->FrameParam);
}

void CameraChildWindow::fpsChangedEvent(FpsEvent *event)
{
    updateFps(event->FpsType, event->Fps);
}

void CameraChildWindow::loggerEvent(LoggerEvent *event)
{
//#ifdef QT_NO_DEBUG
//    if (event->LogType == LoggerEvent::Log_Debug)
//        return;
//#endif
    qInfo().noquote() << event->Text;
    QString formatedMsg = QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + " " + event->Text;
    QListWidgetItem *listItem = new QListWidgetItem(formatedMsg);
    listItem->setSizeHint(QSize(60, 25));  // use to change the height
    listItem->setForeground(QBrush(QColor(event->Rgb)));
    listItem->setBackground(QBrush(QColor(Qt::lightGray)));
    ui->m_listWdtLog->insertItem(0, listItem);
}

void CameraChildWindow::channelStateEvent(ChannelStateEvent *event)
{
    m_channelState = event->ChannelState;
    updateDisplayStatus();
}

void CameraChildWindow::_actionEvent(ActionEvent *event)
{
    switch (event->ActionId) {
    case Action_DispTextPanel:
        ui->m_dockTextPanel->setVisible(event->Checked);
        break;
    case Action_DispTestItemPanel:
        ui->m_dockTestItemPanel->setVisible(event->Checked);
        break;
    case Action_DispTestResultPanel:
        ui->m_dockTestResultPanel->setVisible(event->Checked);
        break;
    default:
        break;
    }
}

void CameraChildWindow::testResultEvent(TestResultEvent *event)
{
    if (!ui->m_dockTestResultPanel->isVisible())
        return;
    if (!m_testItem2ResultTableModelMap.contains(event->Caption))
        m_testItem2ResultTableModelMap[event->Caption] = new GeneralRoTableModel(this);
    m_testItem2ResultTableModelMap[event->Caption]->RefreshTable(event->ResultTable);

    int idx = -1;
    for (int i = 0; i < ui->m_tabWidgetResult->count(); i++) {
        if (ui->m_tabWidgetResult->tabText(i) == event->Caption) {
            idx = i;
            break;
        }
    }
    QTableView *tableView = nullptr;
    if (idx == -1) {
        tableView = new QTableView(ui->m_tabWidgetResult);
        MiscUiUtils::InitTableView(tableView, m_testItem2ResultTableModelMap[event->Caption]);
        idx = ui->m_tabWidgetResult->addTab(tableView, event->Caption);
        ui->m_tabWidgetResult->setCurrentIndex(idx); // switch current to the new page
    }
    else {
        tableView = qobject_cast<QTableView *>(ui->m_tabWidgetResult->widget(idx));
    }
    if (tableView != nullptr) {
        tableView->resizeColumnsToContents();
        if (event->ErrorCode == ERR_NoError) {
            ui->m_tabWidgetResult->tabBar()->setTabTextColor(idx, QColor(Qt::darkGreen));
//            ui->m_tabWidgetResult->tabBar()->setStyleSheet("::tab { background-color: green }");
            tableView->horizontalHeader()->setStyleSheet("background-color: green");
        }
        else {
            ui->m_tabWidgetResult->tabBar()->setTabTextColor(idx, QColor(Qt::red));
//            ui->m_tabWidgetResult->tabBar()->setStyleSheet("::tab { background-color: red }");
            tableView->horizontalHeader()->setStyleSheet("background-color: red");
        }
    }
}

void CameraChildWindow::machineRequestEvent(MachineRequestEvent *event)
{
//    if (event->ChnIdx != m_chnIdx) {
//        WriteLog(tr("Invalid channel index in MachineRequestEvent!"), qRgb(255, 0, 0));
//        return;
//    }
    if (m_sysSettings->WorkMode != UiDefs::WorkMode_Machine) {
        WriteLog(tr("Not in machine work mode!"), qRgb(0xff, 0, 0));
        return;
    }
    if (MachineCommand_OpenCamera == event->Command) {
        ui->m_listWdtLog->clear();
        m_testLastCategoryCode = ReservedCategory_NoError;
    }
    WriteLog(tr("[REQ]Cmd: %1, SubStation: %2, ChnIdx: %3").arg(event->Command).arg(event->SubStation).arg(event->ChnIdx));

    bool isAsync = true;
    uint categoryCode = ReservedCategory_NoError;
    int ec = 0;
    int groupIdx = 0;
    {
        QMutexLocker locker(&m_mutex);
        m_currMachineReqEvent = *event;
    }
    switch ((E_MachineCommand)event->Command)
    {
    case MachineCommand_OpenCamera:
        OpenCameraAsync();
        break;
    case MachineCommand_Testing:
        groupIdx = event->SubStation - 1;
        if (m_mapGroup2ImageTestItem.contains(groupIdx)) {
            ec = StartGroupTest(groupIdx, isAsync);
        }
        else {
            WriteLog(tr("Group index is out of range!"), qRgb(0, 0, 255));
            ec = ERR_Failed;
            isAsync = false;
        }
        if (ec < 0) {
            categoryCode = (ReservedCategory_NoError == m_testLastCategoryCode) ? (uint)ReservedCategory_NotFinished : m_testLastCategoryCode;
        }
        break;
    case MachineCommand_CloseCamera:
        CloseCameraAsync();
        break;
    case MachineCommand_RefreshUi:
        ec = RefreshUi();
        isAsync = false;
        categoryCode = ReservedCategory_NoError;
        break;
    case MachineCommand_BindCode:
        isAsync = false;
        ec = m_channelController->SetBindCode(event->Parameter);
        break;
    default:
        WriteLog(tr("Unsupported command %1!").arg(event->Command), QColor(Qt::red).rgb());
        break;
    }
    if (!isAsync) {
        machineResponse(event->Command, event->SubStation, event->ChnIdx, categoryCode);
    }
}

void CameraChildWindow::machineResponseEvent(MachineResponseEvent *event)
{
//    if (event->ChnIdx != m_chnIdx) {
//        WriteLog(tr("Invalid channel index in machineResponseEvent![%1!=%2]").arg(event->ChnIdx).arg(m_chnIdx), qRgb(255, 0, 0));
//        return;
//    }
    if (m_sysSettings->WorkMode != UiDefs::WorkMode_Machine) {
        WriteLog(tr("Not in machine work mode!"), qRgb(0xff, 0, 0));
        return;
    }

    WriteLog(tr("[RESP]Cmd: %1, SubStation: %2, ChnIdx: %3, CategoryCode: %4")
             .arg(event->Command).arg(event->SubStation).arg(event->ChnIdx).arg(event->CategoryCode));
    QString libFileName = m_sysSettings->MachineName;
    MachineService::GetInstance()->ResponseToHandler(libFileName, event->Command, event->SubStation,
             m_chnIdx, event->CategoryCode);
}

void CameraChildWindow::machineAntiShakeEvent(MachineAntiShakeEvent *event)
{
    if (0 == event->m_state) { // open S0100 (01-fixture no 00-open)
        WriteLog(tr("Fixture[%1] has opened, enter anti-shake mode.").arg(event->m_id), qRgb(0, 0, 0xff));
        m_channelController->SetAntiShakeMode(true);
    }
    else { // close
        int delay = m_sysSettings->AntiShakeDelay;
        WriteLog(tr("Fixture[%1] has closed, delay %2 milliseconds, then leave anti-shake mode.").arg(event->m_id).arg(delay), qRgb(0, 0, 0xff));
        QTimer::singleShot(delay, [&]() {
            m_channelController->SetAntiShakeMode(false);
        });
    }
}

void CameraChildWindow::testItemStateChangedEvent(TestItemStateChangedEvent *event)
{
    QVector<T_ImageTestItem> &groupedTestItem = m_mapGroup2ImageTestItem[event->GroupIdx];
    groupedTestItem[event->IdxInGroup].Elapsed = event->Elapsed;
    groupedTestItem[event->IdxInGroup].ErrorCode = event->ErrorCode;
    if (TestItemStateChangedEvent::State_GroupFinished == event->State) { // group finished
        if (UiDefs::WorkMode_Machine == m_sysSettings->WorkMode) {
            for (int idx = 0; idx < groupedTestItem.size(); idx++) {
                if ((groupedTestItem[idx].ErrorCode < 0) && (ReservedCategory_NoError == m_testLastCategoryCode)) { // found the first error item, when maybe have mes update
                    m_testLastCategoryCode = groupedTestItem[idx].CategoryCode;
                    break;
                }
            }
            // report the first category code
            machineResponse((uint)MachineCommand_Testing, event->GroupIdx + 1, event->ChnIdx, m_testLastCategoryCode);
        }
    }
    else {
        switch (event->State) {
        case TestItemStateChangedEvent::State_Initialized:
            groupedTestItem[event->IdxInGroup].PButton->setStyleSheet("background-color:yellow; color: rgb(0, 0, 0)");
            break;
        case TestItemStateChangedEvent::State_Finished:
        {
            QString text = groupedTestItem[event->IdxInGroup].InstanceName + "[" + QString::number(event->Elapsed) + "]";
            if (event->ErrorCode == ERR_NoError)
                groupedTestItem[event->IdxInGroup].PButton->setStyleSheet("background-color:green; color: rgb(0, 0, 0)");
            else if (event->ErrorCode < 0)
                groupedTestItem[event->IdxInGroup].PButton->setStyleSheet("background-color:red; color: rgb(0, 0, 0)");
            groupedTestItem[event->IdxInGroup].PButton->setText(text);
            break;
        }
        default:
            break;
        }
    }
}

void CameraChildWindow::exceptionEvent(ExceptionEvent *event)
{
    if (ExceptionEvent::Exception_ApplicationError == event->ExType) {
        if (event->CategoryCode == ReservedCategory_VideoInterrupt) {
            WriteLog(event->Message, qRgb(255, 0, 0));
            ui->m_openGLWidget->RenderText(getCategoryMessage(event->CategoryCode), qRgb(0, 0, 0), qRgb(255, 0, 0));
            if (UiDefs::WorkMode_Machine == m_sysSettings->WorkMode) {
                QMutexLocker locker(&m_mutex);
                machineResponse(m_currMachineReqEvent.Command, m_currMachineReqEvent.SubStation, m_chnIdx, ReservedCategory_VideoInterrupt);
            }
        }
    }
}

void CameraChildWindow::asyncInvokeEvent(AsyncInvokeEvent *event)
{
    uint machineCmd;
    int cmd = (event->SessionId & 0xffff);
    switch (cmd) {
    case AsyncInvokeEvent::AsyncCommand_OpenCamera:
    {
        machineCmd = MachineCommand_OpenCamera;
        if (event->ErrorCode != ERR_NoError) {
            m_mapCmd2CategoryCode[machineCmd] = ReservedCategory_OpenCamera;
            RefreshUi();
        }
        else {
            m_mapCmd2CategoryCode[machineCmd] = ReservedCategory_NoError;
            updateDisplayStatus(DSI_START);
            m_sysSettings->WriteSettings();
        }
        if (UiDefs::WorkMode_Machine == m_sysSettings->WorkMode)
            machineResponse(machineCmd, 0x01, m_chnIdx, m_mapCmd2CategoryCode[machineCmd]);
        break;
    }
    case AsyncInvokeEvent::AsyncCommand_CloseCamera:
    {
        machineCmd = MachineCommand_CloseCamera;
        if (event->ErrorCode != ERR_NoError)
            m_mapCmd2CategoryCode[machineCmd] = ReservedCategory_CloseCamera;
        else
            m_mapCmd2CategoryCode[machineCmd] = ReservedCategory_NoError;
        ui->m_openGLWidget->RenderText((m_chnIdx == 0xff) ? "" : QString(QChar(('A' + m_chnIdx))), qRgb(0, 0, 0x80), qRgb(0x80, 0x80, 0x80));
        updateDisplayStatus(DSI_STOP);
        bool isException = ((event->SessionId & 0xffff0000) == AsyncInvokeEvent::AsyncCommand_Exception);
//        qDebug()<<event->SessionId<<isException;
        if ((UiDefs::WorkMode_Machine == m_sysSettings->WorkMode) && !isException)
            machineResponse(machineCmd, 0x01, m_chnIdx, m_mapCmd2CategoryCode[machineCmd]);
        break;
    }
    default:
        break;
    }
}

void CameraChildWindow::diskSizeChangedEvent(DiskSizeChangedEvent *event)
{
    const uint64_t G = 1024 * 1024 * 1024ull;
    double available = event->Available / (1.0 * G);
    if (event->Available < m_sysSettings->AlarmMsgThreshold * 1.0 * G) {
        WriteLog(tr("Disk available space ONLY %1G!").arg(available, 0, 'g', 2), qRgb(0xff, 0, 0));
    }
    if (event->Available < m_sysSettings->StopTestingThreshold * 1.0 * G) {
        WriteLog(tr("Disk available space ONLY %1G! Stop testing now!!").arg(available, 0, 'g', 2), qRgb(0xff, 0, 0));
        m_isDiskFull = true;
    }
}

void CameraChildWindow::actionSelectSensor_triggered()
{
    SelectSensorDialog selectSensorDlg(m_windowIdx, m_chnIdx, this);
    if (selectSensorDlg.exec() == QDialog::Accepted) {
        loadSensorParam();
    }
}

void CameraChildWindow::actionHardwareTestSetting_triggered()
{
    HardwareTestDialog hardwareTestDlg(m_windowIdx, m_chnIdx, this);
    if (hardwareTestDlg.exec() == QDialog::Accepted) {
        if (loadHardwareTestItems() != ERR_NoError) {
            WriteLog(tr("Loading hardware test items failed."), QColor(Qt::red).rgb());
            return;
        }
    }
}

void CameraChildWindow::actionDecoder_triggered()
{
    DecoderDialog decoderDlg(m_windowIdx, m_chnIdx, this);
    if (decoderDlg.exec() == QDialog::Accepted) {
        DecoderService::GetInstance(m_chnIdx)->SetCurrentDecoderName(m_moduleSettings->ImageDecoder);
    }
}

void CameraChildWindow::actionTestItemManagement_triggered()
{
    TestItemManagementDialog testItemManagementDlg(m_windowIdx, m_chnIdx, this);
    if (testItemManagementDlg.exec() == QDialog::Accepted) {
        if (loadImageTestItems() != ERR_NoError) {
            WriteLog(tr("Loading image test items failed."), QColor(Qt::red).rgb());
            return;
        }
    }
}

void CameraChildWindow::actionTestItemSetting_triggered()
{
    TestItemSettingDialog testItemSettingDlg(m_windowIdx, m_chnIdx, this);
    if (testItemSettingDlg.exec() == QDialog::Accepted) {
        //
    }
}

void CameraChildWindow::actionStart_triggered()
{
    OpenCameraAsync();
}

void CameraChildWindow::actionStop_triggered()
{
    CloseCameraAsync();
}

void CameraChildWindow::actionDebug_triggered()
{
    if (m_channelState <= ChannelStateEvent::State_PowerOff)
    {
        m_channelController->PowerOn();
        I2cDebugDialog i2cDebugDlg(m_windowIdx, m_chnIdx, &m_channelContext, this);
        i2cDebugDlg.exec();
        m_channelController->PowerOff();
    }
    else
    {
        I2cDebugDialog i2cDebugDlg(m_windowIdx, m_chnIdx, &m_channelContext, this);
        i2cDebugDlg.exec();
    }
}

void CameraChildWindow::actionSaveLog_triggered()
{
    QString dir = QDir::homePath(); // GlobalVars::APP_PATH;
    QString filter = tr("Text files (*.txt)");
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save log file..."), dir, filter, &selectedFilter);
    qDebug()<<fileName<<selectedFilter;
    if (!fileName.isEmpty()) {
        if (QFileInfo(fileName).completeSuffix().isEmpty())
            fileName = fileName + ".txt";
        QSaveFile saveFile(fileName);
        if (!saveFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, tr("Error"), tr("Open file opened!"));
            return;
        }
        for (int row = ui->m_listWdtLog->count() - 1; row > 0; row--) {
            QByteArray oneLine = ui->m_listWdtLog->item(row)->text().toLocal8Bit();
            oneLine.append("\r\n");
            saveFile.write(oneLine);
        }
        if (saveFile.commit())
            QMessageBox::information(this, tr("Information"), tr("Save file OK!"));
    }
}

void CameraChildWindow::actionClearLog_triggered()
{
    bool bYes = QMessageBox::question(this, tr("Question"), tr("Clear all the log?"),
                                      QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
    if (bYes)
        ui->m_listWdtLog->clear();
}

void CameraChildWindow::actionShowMode_triggered(int index)
{
    m_channelController->SetCacheData(DECODESHOWMODE, QByteArray::number(index));  // DecodeToRgb24::decode->GetCacheData()
}

void CameraChildWindow::listWdtLog_customContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos);
    m_textPanelContextMenu->popup(QCursor::pos());
}

QString CameraChildWindow::getCategoryMessage(uint categoryCode)
{
    switch (categoryCode) {
    case ReservedCategory_OpenCamera:
        return tr("Open");
    case ReservedCategory_CloseCamera:
        return tr("Close");
    case ReservedCategory_NotFinished:
        return tr("Not Finished");
    case ReservedCategory_VideoInterrupt:
        return tr("Video Interrupt");
    default:
        return m_mapCategory2Message[categoryCode];
    }
}

void CameraChildWindow::connectActions()
{
    //System
//    connect(ui->m_actionSystemConf, &QAction::triggered, this, &MainWindow::actionSystemConf_triggered);
    connect(ui->m_actionSelectSensor, &QAction::triggered, this, &CameraChildWindow::actionSelectSensor_triggered);
//    connect(ui->m_actionSubStation, &QAction::triggered, this, &CameraChildWindow::actionSubStation_triggered);

    connect(ui->m_actionHardwareTestSetting, &QAction::triggered, this, &CameraChildWindow::actionHardwareTestSetting_triggered);
    connect(ui->m_actionDecoder, &QAction::triggered, this, &CameraChildWindow::actionDecoder_triggered);
    connect(ui->m_actionTestItemManagement, &QAction::triggered, this, &CameraChildWindow::actionTestItemManagement_triggered);
    connect(ui->m_actionTestItemSetting, &QAction::triggered, this, &CameraChildWindow::actionTestItemSetting_triggered);
//    connect(ui->m_actionExportFiles, &QAction::triggered, this, &MainWindow::actionExportFiles_triggered);
//    connect(ui->m_actionImportFiles, &QAction::triggered, this, &MainWindow::actionImportFiles_triggered);

    // Video
    connect(ui->m_actionStart, &QAction::triggered, this, &CameraChildWindow::actionStart_triggered);
    connect(ui->m_actionStop, &QAction::triggered, this, &CameraChildWindow::actionStop_triggered);
    QActionGroup *sensorGroup = new QActionGroup(this);
    sensorGroup->addAction(ui->m_actionStart);
    sensorGroup->addAction(ui->m_actionStop);
    sensorGroup->setExclusive(true);

    connect(ui->comboBoxShowMode, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CameraChildWindow::actionShowMode_triggered);
    // Debug
    connect(ui->m_actionDebug, &QAction::triggered, this, &CameraChildWindow::actionDebug_triggered);
}

void CameraChildWindow::loadModuleSettings()
{
    qDebug().noquote() << "projectName:" << m_sysSettings->ProjectName << "windowIdx:" << m_windowIdx;
    m_moduleSettings = ModuleSettings::GetInstance(m_sysSettings->ProjectName, m_windowIdx);
    m_channelContext.ModuleSettings = m_moduleSettings;
}

void CameraChildWindow::readSettings()
{
    QSettings settings("CZTEK", "czcmti");
    settings.beginGroup(QString("cameraChildWindow_%1").arg(m_chnIdx));
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    settings.endGroup();
}

void CameraChildWindow::writeSettings()
{
    QSettings settings("CZTEK", "czcmti");
    settings.beginGroup(QString("cameraChildWindow_%1").arg(m_chnIdx));
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.endGroup();
}

void CameraChildWindow::initTextPanelContextMenu()
{
    m_textPanelContextMenu = new QMenu(this);
    m_textPanelContextMenu->addAction(ui->m_actionSaveLog);
    m_textPanelContextMenu->addAction(ui->m_actionClearLog);
    connect(ui->m_actionSaveLog, &QAction::triggered, this, &CameraChildWindow::actionSaveLog_triggered);
    connect(ui->m_actionClearLog, &QAction::triggered, this, &CameraChildWindow::actionClearLog_triggered);

    connect(ui->m_listWdtLog, &QListWidget::customContextMenuRequested,
            this, &CameraChildWindow::listWdtLog_customContextMenuRequested);
}

bool CameraChildWindow::loadSensorParam()
{
    QString sensorName = m_moduleSettings->SensorDb.SchemeName;
    if ((sensorName != UiDefs::NC_SENSOR_NAME) && !sensorName.isEmpty()) {
        if (m_channelController->LoadSensorParam(sensorName.toLatin1().data()) == ERR_NoError) {
            ui->m_lblCameraName->setText(sensorName);
            return true;
        }
    }
    return false;
}

int CameraChildWindow::loadHardwareTestItems()
{
    QVector<QString> hardwareTestItems;
    foreach (QString fullInstanceName, m_moduleSettings->HardwareTestItemMap.values()) {
        QStringList slTemp = fullInstanceName.split(QChar('@'), QString::KeepEmptyParts);
        if (slTemp.size() != 2)
            continue;
        QString fakeFullInstanceName = "@" + slTemp[1];
        T_PluginInfo pluginInfo;
        int ec = m_testItemService->GetPluginInfo(fakeFullInstanceName, pluginInfo);
        if (ec != ERR_NoError) {
            WriteLog(tr("Loading test item[%1] plugin failed.").arg(slTemp[1]), QColor(Qt::red).rgb());
            continue;
        }
        ec = m_testItemService->LoadSettings(fakeFullInstanceName);
        if (ec != ERR_NoError) {
            WriteLog(tr("Loading test item[%1] setting failed.").arg(slTemp[1]), QColor(Qt::red).rgb());
            continue;
        }
        QString strLog;
        strLog.sprintf("%s: V%d.%d.%d.%d", pluginInfo.FriendlyName,
                       (pluginInfo.Version >> 24) & 0xff, (pluginInfo.Version >> 16) & 0xff,
                       (pluginInfo.Version >> 8) & 0xff, (pluginInfo.Version) & 0xff);
        WriteLog(strLog, qRgb(0, 0x80, 0));
        hardwareTestItems.append(fakeFullInstanceName);
    }
    m_testItemService->SetHardwareTestItems(hardwareTestItems);
    return ERR_NoError;
}

int CameraChildWindow::loadImageTestItems()
{
    m_mapGroup2ImageTestItem.clear();
    QListIterator<QString> itTestItem(m_moduleSettings->ImageTestItemMap.values());
    int maxGroupIdx = 0;
    while (itTestItem.hasNext()) {
        QString strTestItem = itTestItem.next();
        QStringList slSubItem = strTestItem.split(QChar(','), QString::KeepEmptyParts);
        if ((slSubItem.size() < 3) || (slSubItem[1].indexOf(QChar('@')) < 0))
            continue;
        QString fullInstanceName = slSubItem[1];
        T_PluginInfo pluginInfo;
        int ec = m_testItemService->GetPluginInfo(fullInstanceName, pluginInfo);
        if (ec != ERR_NoError) {
            WriteLog(tr("Loading image test item plugin failed. [%1]").arg(fullInstanceName), qRgb(255, 0, 0));
            m_isLoadTestItemOver = false;
            continue;
        }
        ec = m_testItemService->LoadSettings(fullInstanceName);
        if (ec != ERR_NoError) {
            WriteLog(tr("Loading test item setting failed.[%1]").arg(fullInstanceName), qRgb(255, 0, 0));
            continue;
        }

        T_ImageTestItem testItem;
        testItem.GroupIdx = slSubItem[0].toInt() - 1;
        if (testItem.GroupIdx > maxGroupIdx)
            maxGroupIdx = testItem.GroupIdx;
        testItem.FullInstanceName = fullInstanceName;
        testItem.InstanceName = fullInstanceName.section(QChar('@'), 0, 0);
        testItem.LibraryName = fullInstanceName.section(QChar('@'), 1, 1);
        testItem.CategoryCode = slSubItem[2].toUInt();
        testItem.ErrorCode = ERR_NotFinished;
        m_mapCategory2Message[testItem.CategoryCode] = testItem.InstanceName;
        QString friendlyName = QString::fromLocal8Bit(pluginInfo.FriendlyName);
        testItem.IsMesUpdate = (friendlyName.indexOf("MES Update", 0, Qt::CaseInsensitive) >= 0);
        qDebug()<<testItem.GroupIdx<<testItem.InstanceName<<testItem.LibraryName<<testItem.CategoryCode;

        if (!m_mapGroup2ImageTestItem.contains(testItem.GroupIdx))
            m_mapGroup2ImageTestItem.insert(testItem.GroupIdx, QVector<T_ImageTestItem>());
        QVector<T_ImageTestItem> &theGroup = m_mapGroup2ImageTestItem[testItem.GroupIdx];
        testItem.IdxInGroup = theGroup.size();
        theGroup.append(testItem);
    }
    // Add the empty group
    for (int grpIdx = 0; grpIdx <= maxGroupIdx; grpIdx++) {
        if (!m_mapGroup2ImageTestItem.contains(grpIdx))
            m_mapGroup2ImageTestItem.insert(grpIdx, QVector<T_ImageTestItem>());
    }
    m_testItemService->SetImageTestGroupedItems(m_mapGroup2ImageTestItem, (UiDefs::E_WorkMode)(m_sysSettings->WorkMode));

    // 3. Relayout ui
    for (int i = ui->m_gridLayoutTestItemPanel->count() - 1; i >= 0; i--) {
        QLayoutItem *layoutItem = ui->m_gridLayoutTestItemPanel->itemAt(i);
        if (layoutItem != nullptr) {
            TestItemButton *pBtn = qobject_cast<TestItemButton *>(layoutItem->widget());
            if (pBtn != nullptr) {
                qDebug("Deleting test button[%s] ...", pBtn->text().toLocal8Bit().data());
                delete pBtn;
            }
            ui->m_gridLayoutTestItemPanel->removeItem(layoutItem);
        }
    }

    int rowIdx = 0;
    QMutableMapIterator<int, QVector<T_ImageTestItem>> itGroup(m_mapGroup2ImageTestItem);
    while (itGroup.hasNext()) {
        itGroup.next();
//        int group = itGroup.key() + 1;
        QMutableVectorIterator<T_ImageTestItem> itImageTestItem(itGroup.value());
        while (itImageTestItem.hasNext()) {
            T_ImageTestItem &imgTestItem = itImageTestItem.next();
            imgTestItem.PButton = new TestItemButton(imgTestItem, this);
            connect(imgTestItem.PButton, &TestItemButton::clicked, this, [&]() {
                if (!m_isDiskFull) {
                    TestItemButton *pBtn = qobject_cast<TestItemButton *>(sender());
                    T_ImageTestItem myTestItem = pBtn->GetImageTestItem();
                    m_testItemService->RunImageTest(myTestItem.GroupIdx, myTestItem.IdxInGroup);
                }
            });
            imgTestItem.PButton->setFocusPolicy(Qt::NoFocus);
            int flag = (m_channelState == ChannelStateEvent::State_Playing)
                            && ((int)UiDefs::WorkMode_Manual == m_sysSettings->WorkMode);
            imgTestItem.PButton->setEnabled(flag);
            ui->m_gridLayoutTestItemPanel->addWidget(imgTestItem.PButton, rowIdx++, 0, 1, 1); // one column
        }
    }
    ui->m_gridLayoutTestItemPanel->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding), rowIdx + 1, 0);

    return ERR_NoError;
}

int CameraChildWindow::loadMachineService()
{
    if (m_sysSettings->WorkMode != UiDefs::WorkMode_Machine)
        return ERR_NoError;

    QString libFileName = m_sysSettings->MachineName;
    qDebug()<<"using machine:"<<libFileName;
    IProtocolHandler *machineInstance = MachineService::GetInstance()->GetPluginInstance(libFileName);
    if (machineInstance == nullptr) {
        WriteLog(tr("Loading machine[%1] plugin failed.").arg(libFileName), qRgb(255, 0, 0));
        return ERR_Failed;
    }
    machineInstance->BindChannelContext(&m_channelContext);
    T_PluginInfo pluginInfo;
    int ec = machineInstance->GetPluginInfo(pluginInfo);
    if (ec != ERR_NoError) {
        WriteLog(tr("Getting plugin info[%1] failed.").arg(libFileName), qRgb(255, 0, 0));
        return ec;
    }
    QString strLog;
    strLog.sprintf("%s: V%d.%d.%d.%d", pluginInfo.FriendlyName,
                   (pluginInfo.Version >> 24) & 0xff, (pluginInfo.Version >> 16) & 0xff,
                   (pluginInfo.Version >> 8) & 0xff, (pluginInfo.Version) & 0xff);
    WriteLog(strLog, qRgb(0, 0x80, 0));
    ec = machineInstance->LoadOption();
    if (ec != ERR_NoError) {
        WriteLog(tr("Loading plugin option[%1] failed.").arg(libFileName), qRgb(255, 0, 0));
        return ec;
    }
    machineInstance->Stop();
    ec = machineInstance->Start();

    return ec;
}

void CameraChildWindow::clearLastTestResult()
{
    m_mapCmd2CategoryCode.clear();
    // travell all items
    QMutableMapIterator<int, QVector<T_ImageTestItem>> itGroup(m_mapGroup2ImageTestItem);
    while (itGroup.hasNext()) {
        itGroup.next();
        QMutableVectorIterator<T_ImageTestItem> itImageTestItem(itGroup.value());
        while (itImageTestItem.hasNext()) {
            T_ImageTestItem &imgTestItem = itImageTestItem.next();
            imgTestItem.ErrorCode = ERR_NotFinished;
        }
    }
}

void CameraChildWindow::refreshTestButtons()
{
    for (int i = 0; i < ui->m_gridLayoutTestItemPanel->count(); i++) {
        QLayoutItem *layoutItem = ui->m_gridLayoutTestItemPanel->itemAt(i);
        if (layoutItem != nullptr) {
            TestItemButton *pBtn = qobject_cast<TestItemButton *>(layoutItem->widget());
            if (pBtn != nullptr) {
                pBtn->ResetButton();
            }
        }
    }
}

void CameraChildWindow::autoTest()
{
//    QMapIterator<int, QVector<T_ImageTestItem>> itGroup(m_mapGroup2ImageTestItem);
//    while (itGroup.hasNext()) {
//        itGroup.next();
//        if (ERR_NoError != m_channelController->StartTest(itGroup.value(), 0))
//        {
//            break;
//        }

//        QVectorIterator<T_ImageTestItem> itImageTestItem(itGroup.value());
//        while (itImageTestItem.hasNext()) {
//            const T_ImageTestItem &imgTestItem = itImageTestItem.next();
//            if (imgTestItem.ErrorCode == ERR_NotFinished) {
//                if (imgTestItem.GroupIdx != groupIdx) {
//                    WriteLog(QString("Has TestItem(group:%1,item:%1) Not Finished.").arg(imgTestItem.GroupIdx, imgTestItem.IdxInGroup), qRgb(255, 0, 0));
//                    return ERR_NoError;
//                }
//            }
//            else if (imgTestItem.ErrorCode != ERR_NoError) {
//                hasError = true;
//            }

//            if (imgTestItem.IsMesUpdate) {
//                hasMesUpdate = true;
//                idxMesUpdateInGroup = imgTestItem.GroupIdx;
//                break;
//            }

//            idxInGroup++;
//        }
//    }
}

int CameraChildWindow::enterOffline()
{
    if (m_channelState > ChannelStateEvent::State_PowerOff)
    {
        WriteLog(QString("please close camera!"), qRgb(255, 0, 0));
        return ERR_Failed;
    }

    if (m_sysSettings->WorkMode == (int)UiDefs::WorkMode_Machine)
    {
        WriteLog(QString("please select (manual) work mode!"), qRgb(255, 0, 0));
        return ERR_Failed;
    }

    QFileDialog *fileDlg = new QFileDialog(this, "Load Image", QDir::homePath() + "/Downloads/", tr("JPEG (*.jpg);;BMP (*.bmp);;Raw (*.raw)"));
    fileDlg->setAcceptMode(QFileDialog::AcceptOpen);
    if (QDialog::Accepted == fileDlg->exec())
    {
        T_FrameParam frameParam;
        uint showType = 0;
        QStringList selectedFile = fileDlg->selectedFiles();
        QString selectedFilter = fileDlg->selectedNameFilter();
        QFileInfo fileInfo(selectedFile[0]);
        RawOptionDlg rawOptDlg(fileInfo.size(), this);
        if (selectedFilter == "Raw (*.raw)")
        {
            rawOptDlg.m_rawWidth = 0;
            rawOptDlg.m_rawHeight = 0;
        }
        else
        {
            Mat imgMat = imread(selectedFile[0].toStdString(), IMREAD_COLOR);
            rawOptDlg.m_rawWidth = imgMat.cols;
            rawOptDlg.m_rawHeight = imgMat.rows;
        }

        rawOptDlg.Cache2Ui();
        if (QDialog::Accepted == rawOptDlg.exec())
        {
            QString strLogInfo = "";
            strLogInfo.sprintf("W:%d, H:%d, bit: %d", rawOptDlg.m_rawWidth, rawOptDlg.m_rawHeight, rawOptDlg.m_rawBits);
            WriteLog(strLogInfo);
            frameParam.Size = (rawOptDlg.m_rawWidth*rawOptDlg.m_rawHeight) * ((rawOptDlg.m_rawBits > 0) ? 2 : 1);
            if (rawOptDlg.m_rawSize != frameParam.Size)
            {
                if (selectedFilter == "Raw (*.raw)")
                {
                    WriteLog(QString("width&height set error."), qRgb(255, 0, 0));
                    return ERR_Failed;
                }
            }

            frameParam.Width = rawOptDlg.m_rawWidth;
            frameParam.Height = rawOptDlg.m_rawHeight;
            frameParam.ImageFormat = (E_ImageFormat)rawOptDlg.m_rawBits; //(0|1)
            frameParam.ImageMode = (E_ImageMode)rawOptDlg.m_rawOutFormat;
            showType = rawOptDlg.m_rawShowType;
        }
        else
        {
            WriteLog(QString("Can not load %1").arg(selectedFile[0]), qRgb(255, 0, 0));
            return ERR_Failed;
        }

        int ec = m_channelController->EnterOfflineTest(selectedFile[0], frameParam, showType);
        if (ERR_NoError != ec)
        {
            updateDisplayStatus(DSI_ExitOffline);
            WriteLog("Enter Image Offline Test Failed.", qRgb(255, 0, 0));
        }
        else
        {
            ui->m_listWdtLog->clear();
            clearLastTestResult();
            m_channelContext.RenderedEvent->Set();
            ui->m_openGLWidget->RenderText((m_chnIdx == 0xff) ? "" : QString(QChar('A' + m_chnIdx)), qRgb(0, 0, 0x80), qRgb(128, 128, 128));
            refreshTestButtons();
            updateDisplayStatus(DSI_EnterOffline);
            WriteLog("Enter Image Offline Test Succeed.", qRgb(0, 0x84, 0));
        }

        return ec;
    }

    return ERR_Failed;
}

int CameraChildWindow::exitOffline()
{
    int ec = m_channelController->ExitOfflineTest();
    if (ERR_NoError == ec)
    {
        RefreshUi();
        updateDisplayStatus(DSI_ExitOffline);
        WriteLog("Exit Image Offline Test Suceed.", qRgb(0, 0x84, 0));
    }
    else
    {
        WriteLog("Exit Image Offline Test Failed.", qRgb(0, 0x84, 0));
    }

    return ec;
}

void CameraChildWindow::saveCurVideo()
{
    if (m_channelState != ChannelStateEvent::State_Playing)
    {
        WriteLog(QString("please open camera!"), qRgb(255, 0, 0));
        return;
    }

    if (m_sysSettings->WorkMode == (int)UiDefs::WorkMode_Machine)
    {
        WriteLog(QString("please select (manual) work mode!"), qRgb(255, 0, 0));
        return;
    }

    T_FrameParam frameParam;
    int ec = m_channelController->GetFrameParam(frameParam);
    if (ec < 0) {
        WriteLog(QString("Get Frame Parameter failed."), qRgb(255, 0, 0));
        return;
    }

    QString selectFile = "";
    QDateTime curTime = QDateTime::currentDateTime();
    const char* ImgInfo[] = {"RGGB", "GRBG", "GBRG", "BGGR"};
    selectFile.sprintf("%d_%04d%02d%02d%02d%02d%02d_%dx%d_%s", m_chnIdx, curTime.date().year(),
                            curTime.date().month(), curTime.date().day(),
                            curTime.time().hour(), curTime.time().minute(),
                            curTime.time().second(), frameParam.Width, frameParam.Height, ImgInfo[frameParam.ImageMode]);
    QFileDialog *fileDlg = new QFileDialog(this, "Save Image", "~/Downloads/", tr("JPEG (*.jpg);;BMP (*.bmp);;Raw10 (*.raw);;Raw8 (*.raw)"));
    fileDlg->setAcceptMode(QFileDialog::AcceptSave);
    fileDlg->selectFile(selectFile+".jpg");
    connect(fileDlg, &QFileDialog::filterSelected, [=] (const QString &filter) {
        if (filter == "JPEG (*.jpg)")
        {
            fileDlg->selectFile(selectFile+".jpg");
        }
        else if (filter == "BMP (*.bmp)")
        {
            fileDlg->selectFile(selectFile+".bmp");
        }
        else if (filter == "Raw10 (*.raw)")
        {
            fileDlg->selectFile(selectFile+"_raw10"+".raw");
        }
        else if (filter == "Raw8 (*.raw)")
        {
            fileDlg->selectFile(selectFile+"_raw8"+".raw");
        }
        else
        {

        }
    });
    if (QDialog::Accepted == fileDlg->exec())
    {
        QStringList selectFiles = fileDlg->selectedFiles();
        WriteLog(selectFiles[0], qRgb(0, 0, 0x80));
        int imgType = Image_JPG;
        QString filter = fileDlg->selectedNameFilter();
        if (filter == "JPEG (*.jpg)")
        {
            imgType = Image_JPG;
        }
        else if (filter == "BMP (*.bmp)")
        {
            imgType = Image_BMP;
        }
        else if (filter == "Raw10 (*.raw)")
        {
            imgType = Image_Raw10;
        }
        else if (filter == "Raw8 (*.raw)")
        {
            imgType = Image_Raw8;
        }
        else
        {

        }

        int ec = m_channelController->SaveImage(selectFiles[0], imgType);
        if (ERR_NoError != ec)
        {
            WriteLog(QString("Save Current Image Failed(%1).").arg(ec), qRgb(255, 0, 0));
        }
    }

    delete fileDlg;
    fileDlg = nullptr;
}

void CameraChildWindow::updateDisplayStatus(DisplayStatusInfos infos)
{
    if (DSI_START == infos)
    {
        enableTestButton(m_sysSettings->WorkMode != (UiDefs::E_WorkMode)UiDefs::WorkMode_Machine);
    }
    else if (DSI_STOP == infos)
    {
        enableTestButton(false);
    }
    else if (DSI_EnterOffline == infos)
    {
        enableTestButton(true);
        ui->m_actionDebug->setDisabled(true);
    }
    else if (DSI_ExitOffline == infos)
    {
        enableTestButton(false);
        ui->m_actionDebug->setEnabled(true);
    }
	
    bool flag = m_channelState <= ChannelStateEvent::State_PowerOnStreamOff;//(((m_sysSettings->SensorSelected >> m_chnIdx) & 0x01) == 0x01);
    ui->m_lblCameraName->setStyleSheet(flag ? "background-color: rgb(240, 0, 0)" : "background-color: rgb(0, 200, 0)");
    QByteArray mesLinkStatus;
    ui->labelMesStatus->setText("MesLink: OFF");
    ui->labelMesStatus->setStyleSheet("background-color: rgb(240, 0, 0)");
    if (ERR_NoError == m_channelController->GetCacheData(MESLINKSTATUS, mesLinkStatus, false))  //mes check and mes update set cache data
    {
        if (0 == mesLinkStatus.toInt())
        {
            ui->labelMesStatus->setText("MesLink: ON");
            ui->labelMesStatus->setStyleSheet("background-color: rgb(0, 200, 0)");
        }
    }

    flag = /*(m_channelState <= ChannelStateEvent::State_PowerOff) &&*/
            (m_sysSettings->WorkMode != (UiDefs::E_WorkMode)UiDefs::WorkMode_Machine);
    ui->m_actionSelectSensor->setEnabled(flag);
    ui->m_actionHardwareTestSetting->setEnabled(flag);
    ui->m_actionDecoder->setEnabled(flag);
    ui->m_actionTestItemManagement->setEnabled(flag);
    ui->m_actionTestItemSetting->setEnabled(flag);
    ui->m_actionDebug->setEnabled(flag);
    ui->m_actionStart->setEnabled(flag);
}

void CameraChildWindow::updateFps(FpsEvent::E_FpsType type, float fps)
{
    switch (type) {
    case FpsEvent::Fps_Capture:
        ui->m_lblCaptureFps->setText(QString("%1").arg((double)fps, 5, 'f', 2));
        break;
    case FpsEvent::Fps_Test:   // local
    case FpsEvent::Fps_Transfer: // remote
        ui->m_lblTransmitFps->setText(QString("%1").arg((double)fps, 5, 'f', 2));
        break;
    case FpsEvent::Fps_Display:
        ui->m_lblDisplayFps->setText(QString("%1").arg((double)fps, 5, 'f', 2));
        break;
    default:
        break;
    }
}

void CameraChildWindow::machineResponse(uint command, uint subStation, uint chnIdx, uint categoryCode)
{
    MachineResponseEvent respEvent(command, subStation, chnIdx, categoryCode);
    machineResponseEvent(&respEvent); // Sync call
}

void CameraChildWindow::enableTestButton(bool flag)
{
    QMutableMapIterator<int, QVector<T_ImageTestItem>> itGroup(m_mapGroup2ImageTestItem);
    while (itGroup.hasNext()) {
        itGroup.next();//        int group = itGroup.key() + 1;
        QMutableVectorIterator<T_ImageTestItem> itImageTestItem(itGroup.value());
        while (itImageTestItem.hasNext()) {
            T_ImageTestItem &imgTestItem = itImageTestItem.next();
            imgTestItem.PButton->setEnabled(flag);
        }
    }
}
