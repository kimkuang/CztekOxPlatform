#ifndef __ICHANNELCONTROLLER_H__
#define __ICHANNELCONTROLLER_H__
#include <map>
#include "TlvDefs.h"
#include "ICzPlugin.h"
#include <QRgb>
#include <QStringList>

class IChannelController
{
public:
    virtual QString GetBarcode() = 0;
    virtual int GetFrameParam(T_FrameParam &frameParam) = 0;
    virtual int VideoControl(uint ctrl) = 0; // ctrl=0: stop; ctrl=1: start; ctrl=2: resume
    virtual int GrabFrame(uchar *pbuffer, int bufferLen, T_FrameParam &frameParam) = 0;
    virtual int DequeueFrameBuffer(int &bufIdx, uchar* &pbuffer, uint &size, uint64 &timestamp) = 0;
    virtual int EnqueueFrameBuffer(int bufIdx) = 0;
    virtual int SetOsTestConfig(uint supplyVol_uV, uint supplyCurrent_uA, const uint pinIds[],
                                const uint openStdVols_uV[], const uint shortStdVols_uV[], uint pinCount) = 0;
    virtual int ReadOsTestResult(const uint pinIds[], uint openVols_uV[], uint shortVol_uV[],
                                 uint results[], uint pinCount) = 0;
    virtual int GetCurrent(const uint powersId[], const uint currentRange[], float current_nA[], uint count) = 0;

    virtual int WriteRegisterList(const T_I2CCommParam &i2cParam, const QList<T_RegConf> &regConfs) = 0;
    virtual int WriteDiscreteI2c(uint slaveAddr, uint speedkHz, uint mode, const ushort regAddrs[],
                         const ushort regVals[], uint regNum) = 0;
    virtual int ReadDiscreteI2c(uint slaveAddr, uint speedkHz, uint mode, const ushort regAddrs[],
                        ushort regVals[], uint regNum) = 0;
    virtual int WriteContinuousI2c(uint slaveAddr, uint speedkHz, uint regAddr, uint regAddrSize,
                           const uchar *data, uint dataSize) = 0;
    virtual int ReadContinuousI2c(uint slaveAddr, uint speedkHz, uint regAddr, uint regAddrSize,
                          uchar *data, uint dataSize) = 0;
    virtual uint64 GetCurrentTimestamp() = 0;
    virtual int SetCacheData(const QString &key, const QByteArray &data) = 0;
    virtual int GetCacheData(const QString &key, QByteArray &data, bool removeIt = true) = 0;

    virtual QString GetSensorFuseId() = 0;
    virtual int LogToWindow(const QString &text, QRgb rgb = qRgb(0, 0x80, 0)) = 0;
    virtual void SaveReport(const QString &strItemName, int itemType, const QString& strHeader, const QString& strContent, bool result) = 0;
    virtual int GetMesData(int type, QStringList &mesData) = 0;
    virtual int SetUseMesFlag(bool flag) = 0;
    virtual int SaveImage(const QString& imgFullPath, int imgType, const uchar* imgBuf = nullptr, uint imgSize=0, bool bDiskOrClound=false) = 0;
    virtual void SetAntiShakeMode(bool mode) = 0;
    virtual bool GetAnitShakeMode() = 0;
    virtual QString GetProjectDir() = 0;
    virtual int RemoteProcedureCall(const QVector<T_TagLenVal> &inParams, QVector<T_TagLenVal> &outParams, int timeoutMs = 30000) = 0;
    virtual int HighlightMessage(const QString &message) = 0;
    virtual int GetSensorSettingNameList(QStringList &settingNameList) = 0;
    virtual int SwitchSensorSetting(const QString &schemeName) = 0;
    virtual int PauseCapture() = 0;
    virtual int ResumeCapture() = 0;
    virtual IOtpSensor *GetOtpSensor() = 0;
};

#endif /* __ICHANNELCONTROLLER_H__ */
