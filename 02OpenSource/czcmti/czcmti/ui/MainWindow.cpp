#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QMessageBox>
#include <QTranslator>
#include <QApplication>
#include <QtSolutions/QtSingleApplication>
#include <QDebug>
#include <signal.h>
#include <QStringList>
#include <QUrl>
#include <QFileInfo>
#include <QFile>
#include <QStandardPaths>
#include "ui/LoginDialog.h"
#include "ui/MachineSettingsDialog.h"
#include "ui/GlobalSettingsDialog.h"
#include "ui/AboutDialog.h"
#include "ui/PasswordDialog.h"
#include "ui/UpgradeDialog.h"
#include "common/Global.h"
#include "utils/utils.h"
#include "thread/patchexecutor.h"
#include "thread/ShareDirTask.h"
#include "service/DecoderService.h"
#ifdef TRIAL_VERSION
#include "common/license.h"
#endif
#include "CustomEvent.h"
#include "send_recv_api.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    readSettings();
    setupUi();
    runPatchExecutor();
    checkProductBarcode();

    registerMetaTypes();
    connectActions();
    m_sysSettings = SystemSettings::GetInstance();
    if (m_sysSettings == nullptr) {
        qCritical("Getting SysSetting instance failed.");
        exit(-1);
    }
    ui->m_actionMachineSettings->setVisible(m_sysSettings->WorkMode == UiDefs::WorkMode_Machine);

    relayoutCentralWidget();
    updateActions();

    createThreads();

    qDebug("==== FUNC: %s Thread: %p(id: %p) ====", __FUNCTION__, QThread::currentThread(), QThread::currentThreadId());

    AboutDialog::GetDbVersion(m_version);
    qInfo().noquote()<<"------------------Db Version: "<<m_version<<"------------------";
    AboutDialog::GetVersion(m_version);
    qInfo().noquote()<<"------------------Bin Version: "<<m_version<<"------------------";
    updateWindowTitle();
//    mountShareDisk();
    ::InitClient(LOG_ERROR); // network transfer module
}

MainWindow::~MainWindow()
{
    writeSettings();

    qDebug()<<"Destroying MainWindow...";
    tearDownUI();
    qDebug("Destroying threads...");
    destroyThreads();
//    umountShareDisk(m_sysSettings->ShareDir.MountPath);
    delete ui;

    m_sysSettings->vtMd5MoudleInI.clear();
    for (int i = 0; i < m_sysSettings->CameraNumber; ++i)
    {
        if ((m_pChildWindows.size() > i) && m_pChildWindows[i] != nullptr)
            delete m_pChildWindows[i];

        QString strModuleInI = GlobalVars::PROJECT_PATH + m_sysSettings->ProjectName + "/" + QString("module%1.ini").arg(i);
        QFile file(strModuleInI);
        file.open(QIODevice::ReadOnly);
        QByteArray btMd5 = QCryptographicHash::hash(file.readAll() + "OfTester", QCryptographicHash::Md5).toHex();
        m_sysSettings->vtMd5MoudleInI.push_back(QString::fromStdString(btMd5.toStdString()));
        file.close();
    }

    QString systemSection = "";
    systemSection.sprintf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", m_sysSettings->WorkMode, m_sysSettings->Operator,
        m_sysSettings->ShowLoginDialog, m_sysSettings->CameraNumber, m_sysSettings->AntiShakeDelay,
        m_sysSettings->AlarmMsgThreshold, m_sysSettings->StopTestingThreshold, m_sysSettings->SensorSelected,
        m_sysSettings->ShowStatusBar, m_sysSettings->ShowToolBar, m_sysSettings->ShowTextPanel,
        m_sysSettings->ShowTestItemPanel, m_sysSettings->ShowTestResultPanel);
    for (int i = 0; i < m_sysSettings->CameraNumber; ++i)
    {
        systemSection += m_sysSettings->vtMd5MoudleInI[i];
    }
    QByteArray btSysSection = QCryptographicHash::hash(systemSection.toLatin1(), QCryptographicHash::Md5).toHex();
    m_sysSettings->md5SystemSection = QString::fromStdString(btSysSection.toStdString());
    m_sysSettings->WriteSettings();
    m_sysSettings = nullptr;

    // Free Service
    DecoderService::FreeAllInstances();
    MachineService::FreeInstance();
    TestItemService::FreeAllInstances();
    ::Destory(); // network transfer module
}

//void MainWindow::saveSnappedPicture(const CmtiDefs::T_ImageCache &snapshotImgCache)
//{
//    Q_UNUSED(snapshotImgCache);
    /*QString initDir = GlobalVars::PIC_PATH;
#ifdef __linux__
    QList<UdevTypes::T_UdevDevice> devList;
    UdevManager::GetUdevList(UdevTypes::DT_UsbDisk, devList);
    if (devList.count() > 0) {
        initDir = devList[0].ExtraInfo;
    }
#endif
    QFileDialog savePicDlg(this);
    savePicDlg.setAcceptMode(QFileDialog::AcceptSave);
    savePicDlg.setWindowTitle(tr("Save snapped picture..."));
    savePicDlg.setDirectory(initDir);
    QStringList filters;
    filters << tr("BMP files(*.bmp)");
    QString suffix = "bmp";
    if ((snapshotImgCache.FrameInfo.ImageFormat >= IMAGE_FMT_RAW8) && (snapshotImgCache.FrameInfo.ImageFormat <= IMAGE_FMT_RAW14)) {
        filters << tr("Bayer raw files(*.raw)");
        suffix = "raw";
    }
    else if (snapshotImgCache.FrameInfo.ImageFormat == IMAGE_FMT_YUV422) {
        filters << tr("YUV422 files(*.yuv422)");
        suffix = "yuv422";
    }
    savePicDlg.setNameFilters(filters);
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/media");
//         << QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    savePicDlg.setSidebarUrls(urls);
    savePicDlg.selectFile("pic" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss")); // 默认文件名

    MediaFile::E_PictureType picType;
    QString fileName;
    if (savePicDlg.exec() == QDialog::Accepted) {
        QString selectedFilter = savePicDlg.selectedNameFilter();
        QStringList fileNames = savePicDlg.selectedFiles();
        if (fileNames.count() < 1)
            return;
        fileName = fileNames[0];

        QFileInfo fileInfo(fileName);
        suffix = fileInfo.suffix();
        if (suffix.isEmpty()) {
            int  idx = selectedFilter.indexOf(QChar('.'));
            suffix = selectedFilter.mid(idx);
            suffix.chop(1);
            fileName = fileName + suffix;
        }
        qDebug()<<fileName<<suffix;
        if ((suffix == ".raw") || (suffix == ".yuv422"))
            picType = MediaFile::PT_Original;
        else
            picType = MediaFile::PT_BMP;
        UiDefs::T_DispEffectParam picEffectParam;
        picEffectParam.VideoEffectParam = m_sysSettings->VideoEffect;
        picEffectParam.RgbGainParam = m_gainParam;
//        m_originalImageProcessor->SavePicture(snapshotImgCache.ImageBuffer, snapshotImgCache.ImageParam, picType, fileName, picEffectParam); // todo
    }*/
//}

void MainWindow::closeEvent(QCloseEvent *event)
{
    bool canClose = QMessageBox::question(this, tr("Query"), tr("Are you sure to exit?"),
                                          QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
    if (canClose) {
        m_sysSettings->WriteSettings();
#ifdef __linux__
        sync(); // unistd.h
#endif
        qDebug("Accept close event...");
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    } else
        QMainWindow::changeEvent(event);
}

void MainWindow::customEvent(QEvent *event)
{
    if (event->type() == ExceptionEvent::type()) {
        exceptionEvent(static_cast<ExceptionEvent *>(event));
    }
    else
        QMainWindow::customEvent(event);
}

void MainWindow::exceptionEvent(ExceptionEvent *event)
{
    QString strLog;
    if (event->ExType == ExceptionEvent::Exception_SystemSignal) {
        if (event->Code == SIGIO) { // 过流
            stop();
            for (int i = 0; i < m_pChildWindows.size(); i++) {
                if (m_pChildWindows[i] != nullptr)
                    m_pChildWindows[i]->QueryOverCurrent();
            }
        }
        else if (event->Code == SIGSEGV) {
            qCritical()<<"SIGSEGV catched!!! Nothing to do here...";
            stop();
            exit(-1);
        }
        else {
            stop();
            strLog = tr("SignalNo: %1, exit normally...").arg(event->Code);
            writeLog(-1, strLog, qRgb(255, 0, 0)); // which channel
            exit(0);
        }
    }
}

void MainWindow::app_messageReceived(const QString &message)
{
    qDebug()<<message;
}

#ifdef __linux__
void MainWindow::udevMonitor_eventChanged(UdevTypes::E_DeviceType deviceType, UdevTypes::E_HotplugEvent event, const QString &devNode)
{
    switch (event) {
    case UdevTypes::HE_DeviceExisted:
    case UdevTypes::HE_DeviceAdded: {
        UdevTypes::T_UdevDevice device;
        device.DeviceType = deviceType;
        device.DevNode = devNode;
        if ((deviceType == UdevTypes::DT_UsbDisk) && Utils::GetMountPointFromDevNode(device.ExtraInfo, devNode)) {
            qDebug()<<devNode<<device.ExtraInfo;
        }
        if (deviceType == UdevTypes::DT_V4L2) {
//            m_uvcDevVideo = devNode;
            qDebug()<<QString("Added %1").arg(devNode);
        }
        UdevManager::RegisterDevice(devNode, device);
        break;
    }
    case UdevTypes::HE_DeviceRemoved: {
        UdevManager::UnregisterDevice(devNode);
        if (deviceType == UdevTypes::DT_V4L2) {
//            m_uvcDevVideo = "";
            qDebug()<<QString("Removed %1. Stop...").arg(devNode);
            stop();
        }
        break;
    }
    default:
        break;
    }
}
#endif

void MainWindow::actionLogin_triggered()
{
    LoginDialog *loginDlg = LoginDialog::GetInstance();
    if (loginDlg->exec() == QDialog::Accepted) {
        updateWindowTitle();
        ui->m_actionMachineSettings->setVisible(m_sysSettings->WorkMode == UiDefs::WorkMode_Machine);

        for (int i = 0; i < m_sysSettings->CameraNumber; i++) {
            bool selected = (((m_sysSettings->SensorSelected >> i) & 0x01) == 0x01);
            if (selected) {
                m_pChildWindows[i]->loadMachineService();
                m_pChildWindows[i]->loadModuleSettings();
                m_pChildWindows[i]->loadImageTestItems();
                m_pChildWindows[i]->updateDisplayStatus();
            }
        }
    }
}

void MainWindow::actionMachineSettings_triggered()
{
    MachineSettingsDialog machineSettingsDlg(m_sysSettings, this);
    if (machineSettingsDlg.exec() == QDialog::Accepted) {
        for (int i = 0; i < m_sysSettings->CameraNumber; i++) {
            bool selected = (((m_sysSettings->SensorSelected >> i) & 0x01) == 0x01);
            if (selected) {
                m_pChildWindows[i]->loadMachineService();
            }
        }
    }
}

void MainWindow::actionGlobalSettings_triggered()
{
    // close camera firstly
    actionStop_triggered();

    GlobalSettingsDialog globalSettingsDlg(this);
    QString oldMountPath = m_sysSettings->ShareDir.MountPath;
    int oldCameraNumber = m_sysSettings->CameraNumber;
    int oldDispCntPerRow = m_sysSettings->DispCntPerRow;
    QList<int> oldDispChnIdxList = m_sysSettings->DispChnIdxList;
    if (globalSettingsDlg.exec() == QDialog::Accepted) {
        if (oldMountPath != m_sysSettings->ShareDir.MountPath) {
//            umountShareDisk(oldMountPath);
//            mountShareDisk();
        }
        if ((oldCameraNumber != m_sysSettings->CameraNumber) ||
            (oldDispCntPerRow != m_sysSettings->DispCntPerRow) ||
            (oldDispChnIdxList != m_sysSettings->DispChnIdxList))
        {
            relayoutCentralWidget();
            updateActions();
        }

        if ((m_sysSettings->Ipc.Enabled) && (m_ipcService != nullptr)) {
            m_ipcService->StopListen();
            m_ipcService->StartListen(m_sysSettings->Ipc.ListenPort);
        }
    }
}

void MainWindow::actionExportFiles_triggered()
{
    QProcess::startDetached(GlobalVars::APP_PATH + "filemanager", QStringList()<<"1");
    GlobalVars::InitApplicationDirectory(); // 导出操作可能删除不用的空目录，重新初始化一级子目录
}

void MainWindow::actionImportFiles_triggered()
{
    QProcess::startDetached(GlobalVars::APP_PATH + "filemanager", QStringList()<<"2");
}

void MainWindow::actionChannelA_triggered()
{
    m_sysSettings->SensorSelected = 0x01;
    for (int i = 0; i < m_sysSettings->CameraNumber; i++) {
        m_pChildWindows[i]->updateDisplayStatus();
    }
}

void MainWindow::actionChannelB_triggered()
{
    m_sysSettings->SensorSelected = 0x02;
    for (int i = 0; i < m_sysSettings->CameraNumber; i++) {
        m_pChildWindows[i]->updateDisplayStatus();
    }
}

void MainWindow::actionChannelC_triggered()
{
    m_sysSettings->SensorSelected = 0x04;
    for (int i = 0; i < m_sysSettings->CameraNumber; i++) {
        m_pChildWindows[i]->updateDisplayStatus();
    }
}

void MainWindow::actionAllChannels_triggered()
{
    for (int i = 0; i < m_sysSettings->CameraNumber; i++) {
        m_sysSettings->SensorSelected |= (0x01 << i);
        m_pChildWindows[i]->updateDisplayStatus();
    }
}

void MainWindow::actionStart_triggered()
{
    for (int i = 0; i < m_sysSettings->CameraNumber; i++) {
        bool selected = (((m_sysSettings->SensorSelected >> i) & 0x01) == 0x01);
        if (selected)
            m_pChildWindows[i]->OpenCameraAsync();
    }
}

void MainWindow::actionStop_triggered()
{
    for (int i = 0; i < m_sysSettings->CameraNumber; i++) {
        if (m_pChildWindows[i] != nullptr) {
            m_pChildWindows[i]->CloseCameraAsync();
        }
    }
}

void MainWindow::actionShowStatusBar_triggered(bool checked)
{
    m_sysSettings->ShowStatusBar = checked;
    ui->m_statusBar->setVisible(checked);
}

void MainWindow::actionShowToolBar_triggered(bool checked)
{
    m_sysSettings->ShowToolBar = checked;
    ui->m_toolBar->setVisible(checked);
}

void MainWindow::actionShowTextPanel_triggered(bool checked)
{
    m_sysSettings->ShowTextPanel = checked;
    for (int i = 0; i < m_pChildWindows.size(); i++)
        QCoreApplication::postEvent(m_pChildWindows[i], new ActionEvent(Action_DispTextPanel, checked));
}

void MainWindow::actionShowTestItemPanel_triggered(bool checked)
{
    m_sysSettings->ShowTestItemPanel = checked;
    for (int i = 0; i < m_pChildWindows.size(); i++)
        QCoreApplication::postEvent(m_pChildWindows[i], new ActionEvent(Action_DispTestItemPanel, checked));
}

void MainWindow::actionShowTestResultPanel_triggered(bool checked)
{
    m_sysSettings->ShowTestResultPanel = checked;
    for (int i = 0; i < m_pChildWindows.size(); i++)
        QCoreApplication::postEvent(m_pChildWindows[i], new ActionEvent(Action_DispTestResultPanel, checked));
}

void MainWindow::actionZoomOut_triggered()
{
//    QImage image = ui->m_glwidgetVideo->grabFrameBuffer();
//    image.save("test.png", "PNG");
//    m_sysSettings->VideoDispParam.ScaleFactor *= (1.0f - 0.1f);
//    ui->m_glwidgetVideo->SetVideoDispParam(m_sysSettings->VideoDispParam);
}

void MainWindow::actionZoomIn_triggered()
{
//    m_sysSettings->VideoDispParam.ScaleFactor *= (1.0f + 0.1f);
//    ui->m_glwidgetVideo->SetVideoDispParam(m_sysSettings->VideoDispParam);
}

void MainWindow::actionAbout_triggered()
{
    AboutDialog dlg(this);
    dlg.exec();
}

void MainWindow::actionSampleCheckON_triggered()
{
    for(int i = 0; i < m_sysSettings->CameraNumber; i++)
    {
        ModuleSettings *moduleSettings = ModuleSettings::GetInstance(m_sysSettings->ProjectName, i);
        moduleSettings->WriteInteger("WBSetting","GlobalTime",0);
        moduleSettings->WriteSettings();
    }
    //AF Sample Check
    QString strCurrentPath = "/home/ubuntu/Documents";
    QString strFileName = strCurrentPath + ("/SFRSampleCheck.ini");
    FILE* fp = NULL;
    fp = fopen(strFileName.toStdString().c_str(),"a");
    if (fp == NULL)
    {
        qCritical("SFRSampleCheck.ini Open Failed.");
        return;
    }
    fclose(fp);

    QSettings *configIniWrite = new QSettings(strFileName, QSettings::IniFormat);
    configIniWrite->beginGroup("SFRSampleCheck");
    configIniWrite->setValue("TimeTag",0);
    configIniWrite->endGroup();
    delete configIniWrite;

}

void MainWindow::actionSampleCheckOFF_triggered()
{
    for(int i = 0; i < m_sysSettings->CameraNumber; i++)
    {
        ModuleSettings *moduleSettings = ModuleSettings::GetInstance(m_sysSettings->ProjectName, i);
        moduleSettings->WriteInteger("WBSetting","GlobalTime",12);
        moduleSettings->WriteSettings();
    }
}

void MainWindow::actionUpgrade_triggered()
{
#ifdef TRIAL_VERSION
//    QMessageBox::critical(this, tr("Error"), tr("Not supported in trial version!"), QMessageBox::Ok);
    bool ok;
    QString text = QInputDialog::getText(this, tr("Please Input Password"),
                                         tr("The feature is protected by password in trial version!"), QLineEdit::Password, "", &ok);
    if (!ok || text != "ftjS1osu") {
        QMessageBox::critical(this, tr("Error"), tr("Password is error!"), QMessageBox::Ok);
        return;
    }
    QFile::remove(GlobalVars::APP_PATH + License::LICENSE_FILE_NAME);
#endif
    writeLog(-1, tr("Stopping service before upgrading..."), qRgb(0, 0, 255));
    stop();

    UpgradeDialog upgradeDialog(this);
    upgradeDialog.exec();
}

void MainWindow::actionEnterTest_triggered()
{
    bool flag = false;
    for (int i = 0; i < m_sysSettings->CameraNumber; i++) {
        bool selected = (((m_sysSettings->SensorSelected >> i) & 0x01) == 0x01);
        if (selected)
        {
            if (ERR_NoError != m_pChildWindows[i]->enterOffline())
            {
                for (int j=0; j<i; ++j)
                {
                    m_pChildWindows[j]->exitOffline();
                }

                return;
            }

            flag = true;
        }
    }

    if (true == flag)
    {
        ui->actionEnter_Test->setChecked(true);
        ui->actionEnter_Test->setDisabled(true);
        ui->actionExit_Test->setEnabled(true);
        ui->actionExit_Test->setChecked(false);
        ui->actionSave_Current_Image->setDisabled(true);
        ui->actionSave_Left_Image->setDisabled(true);
        ui->actionSave_Right_Image->setDisabled(true);
    }
    else
    {
        for (int i = 0; i < m_sysSettings->CameraNumber; i++) {
            writeLog(i, QString("please select camrea for enter image test."), qRgb(255, 0, 0));
        }
    }
}

void MainWindow::actionExitTest_triggered()
{
    for (int i = 0; i < m_sysSettings->CameraNumber; i++)
    {
        //if (m_sysSettings->SensorSelected.testBit(i))
        //{
            if (ERR_NoError != m_pChildWindows[i]->exitOffline())
            {
                return;
            }
        //}
    }

    ui->actionEnter_Test->setEnabled(true);
    ui->actionEnter_Test->setChecked(false);
    ui->actionExit_Test->setDisabled(true);
    ui->actionExit_Test->setChecked(true);
    ui->actionSave_Current_Image->setEnabled(true);
    ui->actionSave_Left_Image->setEnabled(true);
    ui->actionSave_Right_Image->setEnabled(true);
}

void MainWindow::actionSaveCurImage_triggered()
{
    m_pChildWindows[0]->saveCurVideo();
}

void MainWindow::actionSaveLeftImage_triggered()
{
    m_pChildWindows[0]->saveCurVideo();
}

void MainWindow::actionSaveRightImage_triggered()
{
    m_pChildWindows[1]->saveCurVideo();
}

void MainWindow::readSettings()
{
    QSettings settings("CZTEK", "czcmti");
    settings.beginGroup(this->objectName());
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    settings.endGroup();
}

void MainWindow::writeSettings()
{
    QSettings settings("CZTEK", "czcmti");
    settings.beginGroup(this->objectName());
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.endGroup();
}

void MainWindow::runPatchExecutor()
{
    PatchExecutor *patchExecutor = new PatchExecutor();
    // QThreadPool takes ownership and deletes 'patchExecutor' automatically
    QThreadPool::globalInstance()->start(patchExecutor);
}

void MainWindow::createThreads()
{
#ifdef __linux__
    m_udevMonitor = new UdevMonitor(UdevTypes::DT_All);
    connect(m_udevMonitor, &UdevMonitor::eventChanged, this, &MainWindow::udevMonitor_eventChanged, Qt::QueuedConnection);
    m_udevMonitor->Start();
#endif
    m_ipcService = nullptr;
    if (m_sysSettings->Ipc.Enabled) {
        m_ipcService = new IpcService(m_sysSettings);
        m_ipcService->StartListen(m_sysSettings->Ipc.ListenPort);
    }
}

void MainWindow::registerMetaTypes()
{
    qRegisterMetaType<T_FrameParam>("T_FrameParam");
}

void MainWindow::updateWindowTitle()
{
    QString strWorkMode;
    switch ((UiDefs::E_WorkMode)m_sysSettings->WorkMode) {
    case UiDefs::WorkMode_Manual:
        strWorkMode = tr("Manual");
        break;
    case UiDefs::WorkMode_Machine:
        strWorkMode = tr("Machine");
        break;
    default:
        strWorkMode = tr("Unkown");
        break;
    }
#ifdef TRIAL_VERSION
    setWindowTitle(tr("Camera Module Test Instrument - %1 [%2][%3] %4 == %5 days left").arg(m_version)
                   .arg(strWorkMode).arg(m_sysSettings->ProjectName).arg(m_leftDays));
#else
    setWindowTitle(tr("Camera Module Test Instrument - %1 [%2][%3]").arg(m_version).arg(strWorkMode)
                   .arg(m_sysSettings->ProjectName));
#endif
    m_lblTitleInfo->setText(tr("Version: %1 [%2] <font color='#0000ff'>[%3]</font> ").arg(m_version).arg(strWorkMode).arg(m_sysSettings->ProjectName));
}

void MainWindow::relayoutCentralWidget()
{
    for (int i = ui->m_centralGridLayout->count() - 1; i >= 0; i--) {
        QLayoutItem *layoutItem = ui->m_centralGridLayout->itemAt(i);
        if (layoutItem != nullptr) {
            CameraChildWindow *pChild = qobject_cast<CameraChildWindow *>(layoutItem->widget());
            if (pChild != nullptr) {
                ui->m_centralGridLayout->removeItem(layoutItem);
                delete pChild;
            }
        }
    }

    m_pChildWindows.clear();
    int rowCnt = Utils::PageCount(m_sysSettings->CameraNumber, m_sysSettings->DispCntPerRow);
    Q_ASSERT(m_sysSettings->DispChnIdxList.size() >= m_sysSettings->CameraNumber);
    int windowIdx = 0;
    for (int rowIdx = 0; rowIdx < rowCnt; rowIdx++) {
        for (int colIdx = 0; colIdx < (int)m_sysSettings->DispCntPerRow; colIdx++) {
            uint chnIdx = m_sysSettings->DispChnIdxList[windowIdx];
            qDebug("rowIdx: %u, colIdx: %u, windowIdx: %u, chnIdx: %u", rowIdx, colIdx, windowIdx, chnIdx);
            CameraChildWindow *pChild = new CameraChildWindow(windowIdx, chnIdx, this);
            pChild->setFocusPolicy(Qt::NoFocus);
            ui->m_centralGridLayout->addWidget(pChild, rowIdx, colIdx, 1, 1);
            m_pChildWindows.push_back(pChild);
            if (++windowIdx >= m_sysSettings->CameraNumber)
                break;
        }
    }
}

void MainWindow::setupUi()
{
    ui->m_toolBar->clear();
    m_lblTitleInfo = new QLabel(this);
    m_lblTitleInfo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_lblTitleInfo->setMinimumWidth(50);
//    ui->m_toolBar->addWidget(m_lblTitleInfo);
    m_lblTitleInfo->setFixedHeight(25);

    ui->m_menuBar->setCornerWidget(m_lblTitleInfo, Qt::TopRightCorner);
}

void MainWindow::tearDownUI()
{
    delete m_lblTitleInfo;
}

void MainWindow::connectActions()
{
    //System
    connect(ui->m_actionLogin, &QAction::triggered, this, &MainWindow::actionLogin_triggered);
    connect(ui->m_actionMachineSettings, &QAction::triggered, this, &MainWindow::actionMachineSettings_triggered);
    connect(ui->m_actionGlobalSettings, &QAction::triggered, this, &MainWindow::actionGlobalSettings_triggered);
    connect(ui->m_actionExportFiles, &QAction::triggered, this, &MainWindow::actionExportFiles_triggered);
    connect(ui->m_actionImportFiles, &QAction::triggered, this, &MainWindow::actionImportFiles_triggered);
    connect(ui->m_actionExit, &QAction::triggered, this, &QMainWindow::close);

    // Camera Module
    connect(ui->m_actionChannelA, &QAction::triggered, this, &MainWindow::actionChannelA_triggered);
    connect(ui->m_actionChannelB, &QAction::triggered, this, &MainWindow::actionChannelB_triggered);
    connect(ui->m_actionChannelC, &QAction::triggered, this, &MainWindow::actionChannelC_triggered);
    connect(ui->m_actionAllChannels, &QAction::triggered, this, &MainWindow::actionAllChannels_triggered);
    QActionGroup *channelGroup = new QActionGroup(this);
    channelGroup->addAction(ui->m_actionChannelA);
    channelGroup->addAction(ui->m_actionChannelB);
    channelGroup->addAction(ui->m_actionChannelC);
    channelGroup->addAction(ui->m_actionAllChannels);
    channelGroup->setExclusive(true);

    // Control
    connect(ui->m_actionStart, &QAction::triggered, this, &MainWindow::actionStart_triggered);
    connect(ui->m_actionStop, &QAction::triggered, this, &MainWindow::actionStop_triggered);
    QActionGroup *sensorGroup = new QActionGroup(this);
    sensorGroup->addAction(ui->m_actionStart);
    sensorGroup->addAction(ui->m_actionStop);
    sensorGroup->setExclusive(true);

    // Image Mode
    connect(ui->actionEnter_Test, &QAction::triggered, this, &MainWindow::actionEnterTest_triggered);
    connect(ui->actionExit_Test, &QAction::triggered, this, &MainWindow::actionExitTest_triggered);
    connect(ui->actionSave_Current_Image, &QAction::triggered, this, &MainWindow::actionSaveCurImage_triggered);
    connect(ui->actionSave_Left_Image, &QAction::triggered, this, &MainWindow::actionSaveLeftImage_triggered);
    connect(ui->actionSave_Right_Image, &QAction::triggered, this, &MainWindow::actionSaveRightImage_triggered);

    // View
    connect(ui->m_actionShowStatusBar, &QAction::triggered, this, &MainWindow::actionShowStatusBar_triggered);
//    connect(ui->m_actionShowToolBar, &QAction::triggered, this, &MainWindow::actionShowToolBar_triggered);
    connect(ui->m_actionShowTextPanel, &QAction::triggered, this, &MainWindow::actionShowTextPanel_triggered);
    connect(ui->m_actionShowTestItemPanel, &QAction::triggered, this, &MainWindow::actionShowTestItemPanel_triggered);
    connect(ui->m_actionShowTestResultPanel, &QAction::triggered, this, &MainWindow::actionShowTestResultPanel_triggered);

    // Help
    connect(ui->m_actionAbout, &QAction::triggered, this, &MainWindow::actionAbout_triggered);
    connect(ui->m_actionUpgrade, &QAction::triggered, this, &MainWindow::actionUpgrade_triggered);
    connect(ui->m_actionGetDNPIndex_ON, &QAction::triggered, this, &MainWindow::actionSampleCheckON_triggered);
    connect(ui->m_actionGetDNPIndex_OFF, &QAction::triggered, this, &MainWindow::actionSampleCheckOFF_triggered);
}

void MainWindow::updateActions()
{
    bool allSelected = true;
    if ((m_sysSettings->SensorSelected & 0x01) == 0x01)
        ui->m_actionChannelA->setChecked(true);
    else
        allSelected = false;

    if (m_sysSettings->CameraNumber > 1) {
        if (((m_sysSettings->SensorSelected >> 1) & 0x01) == 0x01)
            ui->m_actionChannelB->setChecked(true);
        else
            allSelected = false;
    }
    ui->m_actionChannelB->setEnabled(m_sysSettings->CameraNumber > 1);

    if (m_sysSettings->CameraNumber > 2) {
        if (((m_sysSettings->SensorSelected >> 2) & 0x01) == 0x01)
            ui->m_actionChannelC->setChecked(true);
        else
            allSelected = false;
    }
    ui->m_actionChannelC->setEnabled(m_sysSettings->CameraNumber > 2);
    ui->m_actionAllChannels->setChecked(allSelected);

    ui->m_actionShowStatusBar->setChecked(m_sysSettings->ShowStatusBar);
    actionShowStatusBar_triggered(m_sysSettings->ShowStatusBar);

    ui->m_toolBar->setVisible(false);
//    ui->m_actionShowToolBar->setChecked(m_sysSettings->ShowToolBar);
//    actionShowToolBar_triggered(m_sysSettings->ShowToolBar);

    ui->m_actionShowTextPanel->setChecked(m_sysSettings->ShowTextPanel);
    actionShowTextPanel_triggered(m_sysSettings->ShowTextPanel);

    ui->m_actionShowTestItemPanel->setChecked(m_sysSettings->ShowTestItemPanel);
    actionShowTestItemPanel_triggered(m_sysSettings->ShowTestItemPanel);

    ui->m_actionShowTestResultPanel->setChecked(m_sysSettings->ShowTestResultPanel);
    actionShowTestResultPanel_triggered(m_sysSettings->ShowTestResultPanel);

    //image mode
    ui->actionEnter_Test->setEnabled(true);
    ui->actionEnter_Test->setChecked(false);
    ui->actionExit_Test->setDisabled(true);
    ui->actionExit_Test->setChecked(true);
    ui->actionSave_Current_Image->setVisible(1 == m_sysSettings->CameraNumber);
    ui->actionSave_Left_Image->setVisible(2 == m_sysSettings->CameraNumber);
    ui->actionSave_Right_Image->setVisible(2 == m_sysSettings->CameraNumber);
}

void MainWindow::stop()
{
    ui->m_actionStop->activate(QAction::Trigger);
    ui->actionExit_Test->activate(QAction::Trigger);
}

void MainWindow::destroyThreads()
{
    if (m_ipcService != nullptr)
        delete m_ipcService;
#ifdef __linux__
    qDebug("Destroying udev monitor thread ...");
    if (m_udevMonitor) {
        m_udevMonitor->Stop();
        if (!m_udevMonitor->wait(1000))
            qCritical("Udev monitor thread timeout exited.");
        delete m_udevMonitor;
    }
    qDebug("Udev monitor exited.");
#endif
}

void MainWindow::checkLicenseFile()
{
#ifdef TRIAL_VERSION
    License license(GlobalVars::ThisProductBarcode.OriginalString);
    QString errorMessage;
    m_leftDays = license.CheckLicense(errorMessage);
    if (-1 == m_leftDays) {
        bool flag = QMessageBox::question(NULL, tr("Query"), tr("Are you about to experience?"),
                                          QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes;
        if (flag) {
            if (!license.InitLicense()) {
                qFatal("Licence initilization failed.");
                exit(-1);
            }
            else {
                m_leftDays = License::EXPIRED_DAYS;
                ui->m_stackedWidget->setCurrentIndex(0);
            }
        }
        else {
            exit(0);
        }
    }
    else if (0 == m_leftDays) {
        ui->m_stackedWidget->setCurrentIndex(1);
        ui->m_trialVersionWidget->SetErrorMessage(errorMessage);
    }
    else {
        ui->m_stackedWidget->setCurrentIndex(0);
    }

    if (m_leftDays > 0) {
        license.UpdateLicense();
    }
#else
#endif
}

void MainWindow::checkProductBarcode()
{
#if 0
    m_leftDays = 0;
    if (GlobalVars::ThisProductBarcode.HardwareVersion == ProductBarcode::HV_Invalid) {
        QString upgraderName = "/usr/bin/upgrader";
        if (QFile::exists(upgraderName)) {
            qDebug("Removing upgrader due to invalid serial number...");
            QFile::remove(upgraderName);
        }
        ui->m_trialVersionWidget->SetErrorMessage(tr("Invalid serial number."));
        ui->m_stackedWidget->setCurrentIndex(1);
    }
    else {
        ui->m_stackedWidget->setCurrentIndex(0);
        checkLicenseFile();
    }
#endif
}

void MainWindow::mountShareDisk()
{
    if (m_sysSettings->ShareDir.SourceDir.isEmpty())
    {
        qCritical().noquote() << tr("Share directory[%1] is empty!").arg(m_sysSettings->ShareDir.SourceDir);
        return;
    }

    QString sourceUrl = QString("//%1/%2").arg(m_sysSettings->ShareDir.ServerIP)
            .arg(m_sysSettings->ShareDir.SourceDir);
    ShareDirTask::T_ShellParam shellParam;
    shellParam.CommandType = ShareDirTask::T_ShellParam::Command_Mount;
    shellParam.UserName = m_sysSettings->ShareDir.UserName;
    shellParam.Password = m_sysSettings->ShareDir.Password;
    shellParam.SourceUrl = sourceUrl;
    shellParam.MountedUrl = m_sysSettings->ShareDir.MountPath;
    ShareDirTask *shareDirTask = new ShareDirTask(shellParam);
    QThreadPool::globalInstance()->start(shareDirTask);

    //    if (0 != QProcess::execute(strCmd))
    //    {
    //        m_sysSettings->Middleware.ShareName = "No Mount";
    //    }
}

void MainWindow::umountShareDisk(const QString& mountPath)
{
    QDir dir(mountPath);
    if (mountPath.isEmpty() || !dir.exists(mountPath)) {
        qCritical().noquote() << tr("Invalid mount directory: %1").arg(mountPath);
        return;
    }

    ShareDirTask::T_ShellParam shellParam;
    shellParam.CommandType = ShareDirTask::T_ShellParam::Command_Umount;
    shellParam.MountedUrl = mountPath;
    ShareDirTask *shareDirTask = new ShareDirTask(shellParam);
    QThreadPool::globalInstance()->start(shareDirTask);
}

void MainWindow::writeLog(int chnIdx, const QString &message, QRgb rgb)
{
    for (int i = 0; i < m_pChildWindows.size(); i++) {
        if ((chnIdx == i) || (chnIdx == -1)) {
            CameraChildWindow *childWindow = m_pChildWindows[i];
            if (childWindow != nullptr)
                childWindow->WriteLog(message, rgb);
        }
    }
}
