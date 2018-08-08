#ifndef TI_DPARTICLE_H
#define TI_DPARTICLE_H

#include "ICzPlugin.h"
#include "ConfWidget.h"
#include "Option.h"
#include "IChannelController.h"

class Dparticle : public IImageEvaluation
{
public:
    Dparticle(const char* instanceName);
    virtual ~Dparticle();

    // ICzPlugin
    int GetPluginInfo(T_PluginInfo &pluginInfo);
    int LoadOption();
    int SaveOption();
    int RestoreDefaults();
    // ITestItem
    int BindChannelContext(T_ChannelContext *context);
    E_ItemType GetItemType() const;
    bool GetIsSynchronous() const;
    QString GetReportHeader() const;
    QString GetReportContent() const;
    bool GetIsEngineeringMode() const;
    // IImageEvaluation
    int Initialize(const T_FrameParam &frameParam);
    int Evaluate(const uchar *image, uint64 &timestamp, std::vector<T_RoiInfo> &roiInfos);

private:
    enum {E_MaxDeadCount = 1000};

    enum {
        CHANNEL_R0C0 = 0,
        CHANNEL_R0C1 = 1,
        CHANNEL_R1C0 = 2,
        CHANNEL_R1C1 = 3,
        CHANNEL_COUNT = 4
    };

    struct T_WPixInfo {
        ushort xIndex;
        ushort yIndex;
        ushort Y_Value;
        ushort Average;
    };

    struct T_WDefectStack {
        ushort DefectStackCnt;
        ushort Podtype; //1为Bright_Dead 2为Bright_Wound 3为Dark_Dead 4为Dark_Wound
        ushort DeadNum;
        T_WPixInfo *DfcStk; //所有坏点信息存在在DfcStk中。
    };

    struct T_WPixInfoStack {
        T_WPixInfo *pBrightDeadStk;
        int DeadCnt;
    };

    bool m_hasSetExpGain;
    ConfWidget *m_wdtConf;
    Option *m_option;
    QString m_instanceName;
    T_FrameParam m_frameParam;
    T_ChannelContext *m_context;

    uchar *m_raw8ChannlesBuf[CHANNEL_COUNT]; //0:r0c0, 1:r0c1, 2:r1c0, 3:r1c1
    T_WPixInfoStack m_deadStack;
    T_WDefectStack  m_whiteSpotResult;
    uint64 m_setExpGainTimeStamp_us;

    uint m_lastExp;
    uint m_lastGain;
    bool m_bTestResult;

    int splitRaw10ToRaw8Channels(const uchar *raw10, uint width, uint height, uchar *raw8[4]);
    float getSurroundingAreaAve(const uchar *pChannel, int width, int height, int X, int Y,
                                int KenalW, int KenalH, uint LastSum, uint &NowSum, uint &lastCnt);
    void pushDeadIntoStack(T_WPixInfo *stack, const T_WPixInfo &deadPix, int count,
                           uint imgWidth, uint imgHeight, int stackMaxSize);
    void getClusterFromStack(T_WPixInfo *SingleDeadStack, int StackSize, int &nParticleNum,
                             int nType, int nImageWidth, T_WDefectStack *DfcStk);
    void getDeadPixelFromPannel(const uchar *pChannel, uint channelWidth, uint channelHeight,
                                T_WPixInfoStack *InfoStack);
    int getDeadPixelPositionInFullImage(T_WDefectStack *DfcStk, int chnIdx);
    int whiteSpotTestInner(const uchar *image, const T_FrameParam &frameParam, T_WDefectStack *DfcStk);

    int allocateMemory();
    void freeMemory();
    void showDParticlePositon(const T_WDefectStack &whiteSpotResult, std::vector<T_RoiInfo> &roiInfos);
};

#endif // TI_DPARTICLE_H
