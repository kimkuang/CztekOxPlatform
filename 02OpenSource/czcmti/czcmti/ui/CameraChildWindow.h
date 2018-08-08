#ifndef CAMERACHILDWINDOW_H
#define CAMERACHILDWINDOW_H

#include <QMainWindow>
#include "OpenGLWidget.h"
#include <atomic>
#include "conf/SystemSettings.h"
#include "conf/ModuleSettings.h"
#include "bll/sensorbll.h"
#include "controller/ChannelController.h"
#include "CustomEvent.h"
#include "UiUtils.h"
#include "UiDefs.h"

namespace Ui {
class CameraChildWindow;
}

class CameraChildWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum DisplayStatusInfo {
        DSI_Title = 0x01,
        DSI_Action = 0x02,
        DSI_START = 0x04,
        DSI_STOP = 0x06,
        DSI_EnterOffline = 0x08,
        DSI_ExitOffline = 0x0a,
        DSI_All = DSI_Title | DSI_Action,
    };
    Q_DECLARE_FLAGS(DisplayStatusInfos, DisplayStatusInfo)
    Q_FLAGS(DisplayStatusInfos)

public:
    explicit CameraChildWindow(uint windowIdx, uint chnIdx, QWidget *parent = 0);
    ~CameraChildWindow();

    void OpenCameraAsync();
    int StartGroupTest(int groupIdx, bool &isAsync);
    void CloseCameraAsync();
    int RefreshUi();
    void WriteLog(const QString &message, QRgb rgb = QColor(Qt::black).rgb());
    void QueryOverCurrent();

protected:
    void customEvent(QEvent *event);

    // custom event handlers
    void renderFrameEvent(RenderFrameEvent *event);
    void fpsChangedEvent(FpsEvent *event);
    void loggerEvent(LoggerEvent *event);
    void channelStateEvent(ChannelStateEvent *event);
    void _actionEvent(ActionEvent *event);
    void testResultEvent(TestResultEvent *event);
    void machineRequestEvent(MachineRequestEvent *event);
    void machineResponseEvent(MachineResponseEvent *event);
    void machineAntiShakeEvent(MachineAntiShakeEvent *event);
    void testItemStateChangedEvent(TestItemStateChangedEvent *event);
    void exceptionEvent(ExceptionEvent *event);
    void asyncInvokeEvent(AsyncInvokeEvent *event);
    void diskSizeChangedEvent(DiskSizeChangedEvent *event);

private slots:
    void actionSelectSensor_triggered();
    void actionHardwareTestSetting_triggered();
    void actionDecoder_triggered();
    void actionTestItemManagement_triggered();
    void actionTestItemSetting_triggered();
    void actionStart_triggered();
    void actionStop_triggered();
    void actionDebug_triggered();
    void actionSaveLog_triggered();
    void actionClearLog_triggered();
    void actionShowMode_triggered(int index);
    void listWdtLog_customContextMenuRequested(const QPoint &pos);

private:
    Ui::CameraChildWindow *ui;
    uint m_windowIdx; // from left to right, 0...n-1
    uint m_chnIdx;    // from 0...n-1, 0xff stands for invalid
    T_ChannelContext m_channelContext;
    ChannelController *m_channelController;
    TestItemService *m_testItemService;
    SystemSettings *m_sysSettings;
    ModuleSettings *m_moduleSettings;
    ChannelStateEvent::E_ChannelState m_channelState;
    QMap<QString, GeneralRoTableModel *> m_testItem2ResultTableModelMap; // <caption, model>
    QMap<int, QVector<T_ImageTestItem>> m_mapGroup2ImageTestItem; // <groupIdx, QVector<T_ImageTestItem>>
    QMenu *m_textPanelContextMenu;
    QMap<uint, uint> m_mapCmd2CategoryCode;
    QMap<uint, QString> m_mapCategory2Message;
    uint m_testLastCategoryCode; // save the test items' total category code
    QMutex m_mutex;
    MachineRequestEvent m_currMachineReqEvent;
    std::atomic<bool> m_isDiskFull;
    std::atomic<bool> m_isLoadTestItemOver;

    void bindChannelIndex(uint chnIdx);
    QString getCategoryMessage(uint categoryCode);
    void connectActions();
    void loadModuleSettings();
    void readSettings();
    void writeSettings();
    void initTextPanelContextMenu();
    bool loadSensorParam();
    int loadHardwareTestItems();
    int loadImageTestItems();
    int loadMachineService();
    void clearLastTestResult();
    void refreshTestButtons();
    void autoTest();
    int enterOffline();
    int exitOffline();
    void saveCurVideo();
    void updateDisplayStatus(DisplayStatusInfos infos = DSI_All);
    void updateFps(FpsEvent::E_FpsType type, float fps);
    void machineResponse(uint command, uint subStation, uint chnIdx, uint categoryCode);
	void enableTestButton(bool flag);

    friend class MainWindow;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(CameraChildWindow::DisplayStatusInfos)

#endif // CAMERACHILDWINDOW_H
