#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <QString>
#include <QProcess>
#include <QTime>
#include <QMutex>
#include <QTimer>
#ifdef __linux__
#include "thread/UdevMonitor.h"
#include "common/UdevManager.h"
#endif
#include "conf/SystemSettings.h"
#include "czcmtidefs.h"
#include "CzUtilsDefs.h"
#include "CameraChildWindow.h"
#include "service/MachineService.h"
#include "service/IpcService.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);
    void customEvent(QEvent *event);
    // custom event handlers
    void exceptionEvent(ExceptionEvent *event);

public slots:
    void app_messageReceived(const QString &message);

private slots:
#ifdef __linux__
    void udevMonitor_eventChanged(UdevTypes::E_DeviceType deviceType, UdevTypes::E_HotplugEvent event, const QString &devNode);
#endif
    void actionLogin_triggered();
    void actionMachineSettings_triggered();
    void actionGlobalSettings_triggered();
    void actionExportFiles_triggered();
    void actionImportFiles_triggered();
    void actionChannelA_triggered();
    void actionChannelB_triggered();
    void actionChannelC_triggered();
    void actionAllChannels_triggered();
    void actionStart_triggered();
    void actionStop_triggered();
    void actionShowStatusBar_triggered(bool checked = false);
    void actionShowToolBar_triggered(bool checked = false);
    void actionShowTextPanel_triggered(bool checked = false);
    void actionShowTestItemPanel_triggered(bool checked = false);
    void actionShowTestResultPanel_triggered(bool checked = false);
    void actionZoomOut_triggered();
    void actionZoomIn_triggered();
    void actionAbout_triggered();
    void actionUpgrade_triggered();
    void actionSampleCheckON_triggered();
    void actionSampleCheckOFF_triggered();
	
    //image mode
    void actionEnterTest_triggered();
    void actionExitTest_triggered();
    void actionSaveCurImage_triggered();
    void actionSaveLeftImage_triggered();
    void actionSaveRightImage_triggered();

private:
#ifdef __linux__
    UdevMonitor *m_udevMonitor;
#endif
    IpcService *m_ipcService;
    SystemSettings *m_sysSettings;  // 系统配置项
    QString m_version;
    QTime m_hotPlugTime;
    int m_leftDays;
    QList<CameraChildWindow *> m_pChildWindows;

    void readSettings();
    void writeSettings();
    void registerMetaTypes();
    void updateWindowTitle();
    void relayoutCentralWidget();
    void setupUi();
    void tearDownUI();
    void connectActions();
    void updateActions();
    void runPatchExecutor();
    void createThreads();
    void stop();
    void destroyThreads();
    void checkLicenseFile();
    void checkProductBarcode();
    void writeLog(int chnIdx, const QString &message, QRgb rgb = QColor(Qt::black).rgb());
    void mountShareDisk();
    void umountShareDisk(const QString& mountPath);

private:
    Ui::MainWindow *ui;
    QLabel *m_lblTitleInfo;
};

#endif // MAINWINDOW_H
