#ifndef __CZPLUGIN_H__
#define __CZPLUGIN_H__
#include <string>
#include <vector>
#include <QMap>
#include <QString>
#include <QVector>
#include "czcmtidefs.h"
#include "IIniFile.h"

#ifndef CZPLUGIN_API
#ifdef CZPLUGIN_EXPORTS
    #ifdef __GNUC__
        #define CZPLUGIN_API __attribute__((visibility("default")))
    #else
        #define CZPLUGIN_API __declspec(dllexport)
    #endif
#else
    #ifdef __GNUC__
        #define CZPLUGIN_API __attribute__((visibility("default")))
    #else
        #define CZPLUGIN_API __declspec(dllimport)
    #endif
#endif /* TESTITEM_EXPORTS */
#endif /* CZPLUGIN_API */

struct T_PluginInfo
{
    uint32 FileType; // E_FileType
    char LibBaseName[32];
    char FriendlyName[32];
    char InstanceName[32];
    char Description[128];
    char VendorName[64];
    uint32 Version;
    uint64 OptionDlgHandle;
};

class ICzPlugin
{
public:
    virtual int GetPluginInfo(T_PluginInfo &pluginInfo) = 0;
    virtual int LoadOption() = 0;
    virtual int SaveOption() = 0;
    virtual int RestoreDefaults() = 0;
};
typedef ICzPlugin* (*PFuncCreatePlugin)(void *arg);
typedef void(*PFuncDestroyPlugin)(ICzPlugin** plugin);

class QObject;
class SyncEvent;
class IChannelController;
class IImageSensor;
class IOtpSensor;
class IVcmDriver;
struct T_ChannelContext {
    uint ChnIdx; // current channel index
    IIniFile *MachineSettings; // machine.ini
    IIniFile *SystemSettings;  // czcmti.ini
    IIniFile *ModuleSettings;  // moduleX.ini
    QObject *EventReceiver;
    SyncEvent *RenderedEvent;
    QVector<IChannelController *> ChannelControllerVector; // all channel controllers
    IChannelController *ChannelController;                 // current channel controller
    IImageSensor *ImageSensor;
    IOtpSensor *OtpSensor;                        // current otp sensor
    IVcmDriver *VcmDriver;
    T_ChannelContext() {
        ChnIdx = 0;
        MachineSettings = nullptr;
        SystemSettings = nullptr;
        ModuleSettings = nullptr;
        EventReceiver = nullptr;
        RenderedEvent = nullptr;
        ChannelController = nullptr;
        ImageSensor = nullptr;
        OtpSensor = nullptr;
        VcmDriver = nullptr;
    }
};

class ISensorImageDecoder : public ICzPlugin
{
public:
    virtual int BindChannelContext(T_ChannelContext *context) = 0;
    virtual int Decode(uchar *inImage, const T_FrameParam &inFrameParam, const std::vector<T_RoiInfo> &roiInfos,
                       uchar *&outImage, T_FrameParam &outFrameParam) = 0;
};

enum E_MachineCommand {
    MachineCommand_OpenCamera = 0,
    MachineCommand_Testing = 1,
    MachineCommand_CloseCamera = 2,
    MachineCommand_RefreshUi = 3,
    MachineCommand_BindCode = 4,
};
class IProtocolHandler : public ICzPlugin
{
public:
    virtual int BindChannelContext(T_ChannelContext *context) = 0;
    virtual int Start() = 0;
    virtual int Stop() = 0;
    virtual int ResponseToHandler(int command, int substation, int chnIdx, int ec) = 0;
};

class ITestItem : public ICzPlugin
{
public:
    enum E_ItemType {
        ItemType_ImageEvaluation = 0,
        ItemType_Hardware,
    };
public:
    virtual int BindChannelContext(T_ChannelContext *context) = 0;
    virtual E_ItemType GetItemType() const = 0;
    virtual bool GetIsSynchronous() const = 0;
    virtual QString GetReportHeader() const = 0;
    virtual QString GetReportContent() const = 0;
    /// If open the engineering mode, much data has saved
    virtual bool GetIsEngineeringMode() const = 0;
};

class IImageEvaluation : public ITestItem
{
public:
    virtual int Initialize(const T_FrameParam &frameParam) = 0;
    // if timestamp==0, the caller will discard this image to display.
    virtual int Evaluate(const uchar *image, uint64 &timestamp, std::vector<T_RoiInfo> &roiInfos) = 0;
};

class IHardwareTest : public ITestItem
{
public:
    enum E_HardwareTestType {
        HardwareTest_OpenShort,
        HardwareTest_StandbyCurrent,
        HardwareTest_Normal,
    };
public:
    virtual E_HardwareTestType GetHardwareType() const = 0;
    virtual bool GetContinueWhenFailed() const = 0;
    virtual int Initialize() = 0;
    /* Result table consists of many rows, each column of row splitted by ','. */
    virtual int RunTest(std::vector<std::string> &resultTable) = 0;
};

#endif // __CZPLUGIN_H__
