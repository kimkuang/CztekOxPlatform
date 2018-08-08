#ifndef CHANNEL_H
#define CHANNEL_H
#include "hal/IHalOx.h"
#include "service/FileTransferService.h"
#include "service/TestItemService.h"
#include "utils/LibraryLoader.h"
#include "bll/sensorbll.h"
#include "thread/syncevent.h"
#include "thread/CaptureThread.h"
#include "CustomEvent.h"
#include "IChannelController.h"
#include "macrodefs.h"
#include "IImageSensor.h"
#include "IVcmDriver.h"
#include "IOtpSensor.h"
#include "conf/SystemSettings.h"
#include "conf/ModuleSettings.h"
#include <future>

class ChannelController : public QObject, public IChannelController
{
public:
    static ChannelController *GetInstance(uint chnIdx);
    static void FreeInstance(uint chnIdx);
    static void FreeAllInstances();

private:
    static std::mutex m_singleInstanceMutex;
    static QMap<uint, ChannelController *> m_mapChnIdx2ChannelController;

    ChannelController(uint chnIdx);
    virtual ~ChannelController();
    ChannelController() = delete;
    ChannelController(const ChannelController &) = delete;
    ChannelController &operator=(const ChannelController &) = delete;

public:
    void BindChannelContext(T_ChannelContext *context);
    int LoadSensorParam(const QString &sensorName);
    int OpenCamera();
    int StartTest(int groupIdx, int idxInGroup);
    int CloseCamera(bool isException = false);
    int UnloadSensorParam();

    int PowerOn();
    int PowerOff();
    IHalOx *GetHalService();

private:
    int openCameraWithoutLock();
    int closeCameraWithoutLock();

    int StartVideo(bool loadRegisters);
    int StopVideo();

public:
    // IChannelController
    QString GetBarcode();
    int SetBeepOn(uint ms);
    QString GetDeviceVersion();
    QString GetHalVersion();
    int WriteRegisterList(const T_I2CCommParam &i2cParam, const QList<T_RegConf> &regConfs);
    int WriteDiscreteI2c(uint slaveAddr, uint speedkHz, uint mode, const ushort regAddrs[],
                         const ushort regVals[], uint regNum);
    int ReadDiscreteI2c(uint slaveAddr, uint speedkHz, uint mode, const ushort regAddrs[],
                        ushort regVals[], uint regNum);
    int WriteContinuousI2c(uint slaveAddr, uint speedkHz, uint regAddr, uint regAddrSize,
                           const uchar *data, uint dataSize);
    int ReadContinuousI2c(uint slaveAddr, uint speedkHz, uint regAddr, uint regAddrSize,
                          uchar *data, uint dataSize);
    int GetFrameParam(T_FrameParam &frameParam);
    int VideoControl(uint ctrl);
    int GrabFrame(uchar *pbuffer, int bufferLen, T_FrameParam &frameParam);
    int DequeueFrameBuffer(int &index, uchar* &pbuffer, uint &size, uint64 &timestamp);
    int EnqueueFrameBuffer(int index);

    int SetOsTestConfig(uint supplyVol_uV, uint supplyCurrent_uA, const uint pinsId[],
                        const uint openStdVols_uV[], const uint shortStdVols_uV[], uint pinCount);
    int ReadOsTestResult(const uint pinsId[], uint openVols_uV[], uint shortVol_uV[],
                         uint results[], uint pinCount);
    int GetCurrent(const uint powerIds[], const uint currentRange[], float current_nA[],
                   uint count);
    int QueryOverCurrent(uint &count, QString &powerNames);

    QString GetProjectDir();
    uint64 GetCurrentTimestamp();
    int SetCacheData(const QString &key, const QByteArray &data);
    int GetCacheData(const QString &key, QByteArray &data, bool removeIt = true);

    QString GetSensorFuseId();
    int LogToWindow(const QString &text, QRgb rgb = qRgb(0, 0x80, 0));
    void SaveReport(const QString& strItemInstName, int itemType, const QString& strHeader, const QString& strContent, bool result);
    int SetBindCode(const QString& bindCode);
    QString GetBindCode(void);
    int GetMesData(int type, QStringList &mesData);
    int SetUseMesFlag(bool flag);
    int RemoteProcedureCall(const QVector<T_TagLenVal> &inParams, QVector<T_TagLenVal> &outParams, int timeoutMs = 30000);

    //image mode
    int EnterOfflineTest(const QString &strImgPath, const T_FrameParam &frameParam, uint showType);
    int ExitOfflineTest();
    int SaveImage(const QString& imgFullPath, int imgType, const uchar* imgBuf = nullptr, uint imgSize=0, bool bDiskOrCloud=false/*default:disk*/);

    void SetAntiShakeMode(bool mode);
    bool GetAnitShakeMode(void);
    int HighlightMessage(const QString &message);
    int GetSensorSettingNameList(QStringList &settingNameList);
    int SwitchSensorSetting(const QString &schemeName);
    int PauseCapture();
    int ResumeCapture();
    IOtpSensor *GetOtpSensor();

private:
    int SetFrameParam(uint imageFormat, uint imageMode, uint width, uint height,
                      uint size, uint cropLeft, uint cropTop, uint cropWidth, uint cropHeight);
    int GetFrameParam(uint &imageFormat, uint &imageMode, uint &width, uint &height,
                      uint &size, uint &cropLeft, uint &cropTop, uint &cropWidth, uint &cropHeight);
    int GetFrameParam(uint &imageFormat, uint &imageMode, uint &width, uint &height, uint &size);
    int writeSensorMultiRegs(uint slaveAddr, uint speedkHz, uint mode, const uint *pRegParam, int length);
    int writeSensorMultiRegsWithDelay(uint slaveAddr, uint speedkHz, uint mode, const ushort regAddr[],
                                      const ushort regData[], const ushort regDelay[], int regNum);
    void saveReportToFile(const QString &strItemInstName, const QString& strHeader, const QString& strContent);
    void setMesUpdateData(const QString &strItemInstName,  int itemType,const QString& strHeader, const QString& strContent);
    int saveRGB(const QString &imgFullPath, int imgType);
    int saveRaw(const QString &imgFullPath, int imgType);
    int saveRaw(const QString &imgFullPath, const uchar *imgBuf, uint imgSize);
    void RawToRaw8(const uchar* pSrc, uchar* pDest, int width, int height, uchar bits);
    void Raw8ToRaw(const uchar* pSrc, uchar* pDest, int width, int height, uchar bits);
    int checkFlagRegisters(const T_I2CCommParam &i2cParam);

private:
    struct T_ReportData {
        QString Caption;
        QString Header;
        QString Content;
        int ErrorCode;
        T_ReportData() {}
        T_ReportData(const QString &caption, const QString &header, const QString &content, int ec)
        {
            Caption = caption;
            Header = header;
            Content = content;
            ErrorCode = ec;
        }
    };
    uint m_chnIdx;
    T_ChannelContext *m_channelContext;
    ChannelStateEvent::E_ChannelState m_channelState;
    IHalOx *m_halService;
    SensorBLL *m_sensorBll;
    T_SensorSetting m_sensorSetting;
    TestItemService *m_testItemService;
    FileTransferService *m_fileTransferService;
    PingPongBuffer *m_capTransBuffer;
    CaptureThread *m_captureThread;
    SystemSettings *m_systemSettings;
    ModuleSettings *m_moduleSettings;
    QStringList m_imageTestMesData;
    QStringList m_hardwareTestData;
//    QVector<T_ReportData> m_reportDataList;
    bool m_mesResult;
    bool m_hardTestResult;
    bool m_bUseMes;          //1、MesCheck、MesBinding和MesUnbinding失败置false，MesUpdate失败 //2、No MesCheck TestItem置false，MesUpdate失败
    bool m_bAntiShakeMode;   //false or true
    std::recursive_timed_mutex m_mutex;
    char *m_pszRpcResult;
    QString m_fuseId;
    QString m_bindCode;
    QMap<QString, QByteArray> m_tempCacheData;
    bool m_bNeedReloadSensorParam;
    std::future<int> m_future;
    QString m_hightlightMessage;

    void setChannelState(ChannelStateEvent::E_ChannelState state);
    void sensorBo2sensorSetting(const T_SENSOR_CFG &sensorBo, T_SensorSetting &sensorSetting);
    void freeSensorSetting(T_SensorSetting &sensorSetting);
    bool RGB24toRawForGBRG(const uchar *pSrc, ushort *pDest, int width, int height, int outformat);
    bool RGB24toRawForRGGB(const uchar *pSrc, ushort *pDest, int width, int height, int outformat);
    bool RGB24toRawForGRBG(const uchar *pSrc, ushort *pDest, int width, int height, int outformat);
    bool RGB24toRawForBGGR(const uchar *pSrc, ushort *pDest, int width, int height, int outformat);
    void postEvent(QEvent *event, int priority = Qt::NormalEventPriority);

    friend class CameraChildWindow;
};

#endif // CHANNEL_H
