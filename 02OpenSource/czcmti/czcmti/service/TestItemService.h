#ifndef TESTITEMSERVICE_H
#define TESTITEMSERVICE_H
#include <map>
#include <thread>
#include <atomic>
#include <QVector>
#include <QBitArray>
#include <future>
#include "czcmtidefs.h"
#include "utils/LibraryLoader.h"
#include "ICzPlugin.h"
#include "CustomEvent.h"
#include "CzUtilsDefs.h"
#include "UiDefs.h"

class TestItemButton;
struct T_ImageTestItem {
    int GroupIdx;
    int IdxInGroup;
    QString FullInstanceName;
    QString InstanceName;
    QString LibraryName;
    uint CategoryCode;
    TestItemButton *PButton;
    int ErrorCode;
    int64_t StartTimestamp;
    int64_t EndTimestamp;
    int Elapsed;
    bool IsMesUpdate;
    T_ImageTestItem() {
        GroupIdx = IdxInGroup = 0;
        FullInstanceName = InstanceName = LibraryName = "";
        CategoryCode = 0;
        PButton = nullptr;
        ErrorCode = ERR_Failed;
        StartTimestamp = EndTimestamp = Elapsed = 0;
        IsMesUpdate = false;
    }
};

class IChannelController;
class TestItemService
{
public:
    static TestItemService *GetInstance(uint chnIdx);
    static void FreeInstance(uint chnIdx);
    static void FreeAllInstances();
private:
    static std::mutex m_mutex;
    static QMap<uint, TestItemService *> m_mapChnIdx2TestItemService;
    TestItemService(uint chnIdx);
    virtual ~TestItemService();
    TestItemService() = delete;
    TestItemService(const TestItemService &) = delete;
    TestItemService &operator=(const TestItemService &) = delete;

public:
    void BindChannelContext(T_ChannelContext *context);
    int GetPluginInfo(const QString &fullInstanceName, T_PluginInfo &pluginInfo);
    ITestItem *GetPluginInstance(const QString &fullInstanceName);
    void FreePluginInstance(const QString &fullInstanceName);
    void FreeAllPluginInstances();
    void SetHardwareTestItems(const QVector<QString> &hardwareTestItems);
    void SetImageTestGroupedItems(const QMap<int, QVector<T_ImageTestItem>> &mapGroup2GroupedImgTestItem,
                                  UiDefs::E_WorkMode workMode);
    void RunImageTest(int groupIdx, int idxInGroup);
    int StartTestThread();
    int StopTestThread();
    IHardwareTest *GetHardwareTest(IHardwareTest::E_HardwareTestType hwTestType);
    int RunHardwareTestOneTime(IHardwareTest *hardwareTest);
    int EnterOfflineTestThread(const uchar *pimge, const T_FrameParam &frameParam);
    int ExitOfflineTestThread();
    int GetItemType(const QString &fullInstanceName, ITestItem::E_ItemType &itemType);
    int RestoreDefaults(const QString &fullInstanceName);
    int LoadSettings(const QString &fullInstanceName);
    int SaveSettings(const QString &fullInstanceName);

private:
    enum {
        MAX_SUPPORTED_ASYNC_ITEMS = 10,
    };
    uint m_chnIdx;
    T_ChannelContext *m_channelContext;
    QMap<QString, ITestItem*> m_mapName2Instance; // <Instance Name@Library File Name, Plugin Instance>
    IHardwareTest *m_openShortTest;
    IHardwareTest *m_standbyCurrentTest;
    std::atomic<bool> m_hardwareTestThreadRunning;
    std::thread *m_hardwareTestThread;
    QVector<IHardwareTest *> m_hardwareTestItems;
    ISensorImageDecoder *m_sensorImageDecoder;
    T_FrameParam m_frameParam;
    std::atomic<bool> m_imageTestThreadRunning;
    std::thread *m_imageTestThread;

    std::atomic<bool> m_imageOfflineTestThreadRunning;
    std::thread *m_imageOfflineTestThread;
    QMap<int, QVector<T_ImageTestItem>> m_mapGroup2GroupedImgTestItem;
    std::atomic<bool> m_imageEvalTestEnabled;
    struct T_ImageTestGroupContext {
        std::atomic<int> GroupIdx;
        std::atomic<int> IdxInGroup;
        int MesUpdateIdx;
        int LastErrorCode;
        T_ImageTestGroupContext() {
            GroupIdx = 0;
            IdxInGroup = 0;
            MesUpdateIdx = -1;
            LastErrorCode = 0;
        }
    } m_imgTestGroupContext;
    UiDefs::E_WorkMode m_workMode;   
    uchar* m_offlineTestRaw;

    void hardwareTestThread();
    void imageTestThread();
    void imageOfflineTestThread();
    void executeOneImageTest(const uchar *pSensorImgBuffer, uint64 &timestamp, std::vector<T_RoiInfo> &roiInfos,
        bool &itemInitialized, QVector<std::shared_future<T_ImageTestItem>> &vtFutureAsync);
    void postEvent(QEvent *event, int priority = Qt::NormalEventPriority);  
};

#endif // TESTITEMSERVICE_H
