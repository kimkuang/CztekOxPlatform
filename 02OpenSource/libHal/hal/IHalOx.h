#ifndef IHAL_CZTEK_H
#define IHAL_CZTEK_H

#include "library_global.h"
#include "czcmtidefs.h"

class LIBRARY_API IHalOx
{
public:
    virtual ~IHalOx() {}

    virtual int WriteDiscreteI2c(uint slaveAddr, uint speedkHz, uint mode, const ushort regs[],
                                 const ushort values[], uint regNum)= 0;
    virtual int ReadDiscreteI2c(uint slaveAddr, uint speedkHz, uint mode, const ushort regs[],
                                ushort values[], uint regNum)= 0;
    virtual int WriteContinuousI2c(uint slaveAddr, uint speedkHz, uint regAddr, uint regAddrSize,
                                   const uchar data[], uint dataSize)= 0;
    virtual int ReadContinuousI2c(uint slaveAddr, uint speedkHz, uint regAddr, uint regAddrSize,
                                  uchar data[], uint dataSize)= 0;
    virtual int SetSensorClock(uint clk100kHz)= 0;
    virtual int SetMipiParam(uint laneNum, uint freqMHz, uint virtualChannel)= 0;
    virtual int SetSensorGpioPinLevel(ushort pin, ushort level) = 0;
    virtual int GetSensorGpioPinLevel(ushort pin, ushort &level) = 0;
    virtual int SetSensorGpioPinDir(ushort pin, ushort dir) = 0;  // 1:input, 0:output
    virtual int GetSensorGpioPinDir(ushort pin, ushort &dir) = 0; // 1:input, 0:output
    virtual int SetSensorPower(const uint powerIds[], const uint voltagemV[], const uint delayms[], uint count)= 0;

    virtual int SetFrameParam(uint imageFormat, uint imageMode, uint width, uint height,uint outImageFormat,
                              uint cropLeft, uint cropTop, uint cropWidth, uint cropHeight) = 0;
    virtual int GetFrameParam(uint &imageFormat, uint &imageMode, uint &width, uint &height, uint &size,
                              uint &cropLeft, uint &cropTop, uint &cropWidth, uint &cropHeight) = 0;
    virtual int SetVideoControl(uint ctrl) = 0;
    virtual int SetSensorPort(uint port) = 0;
    virtual int DequeueBuffer(int &index, uchar **data, timeval &timestamp) = 0;
    virtual int EnqueueBuffer(int index) = 0;

    virtual int SetOsTestConfig(uint supplyVol_uV, uint supplyCurrent_uA, const uint pinIds[],
                                const uint openStdVol_uV[], const uint shortStdVol_uV[], uint count) = 0;
    virtual int ReadOsTestResult(const uint pinIds[], uint openVol_uV[], uint shortVol_uV[],
                                 uint results[], uint count) = 0;
    virtual int GetCurrent(const uint powerIds[], const uint currentRange[], float current_nA[], uint count) = 0;
    virtual int BeepOn(uint ms) = 0;
    virtual int LedFlash(bool on) = 0;

    virtual int SetExtendGpioPinLevel(ushort pin, ushort level) = 0;
    virtual int GetExtendGpioPinLevel(ushort pin, ushort &level) = 0;
    virtual int SetExtendGpioPinDir(ushort pin, ushort dir) = 0;  // 1:input, 0:output
    virtual int GetExtendGpioPinDir(ushort pin, ushort &dir) = 0; // 1:input, 0:output

    virtual int GetHalVersion(char *buffer, int len) = 0;
    virtual int GetDriverVersion(char *version, int len) = 0;
    virtual int GetBarcode(char *barcode, int len) = 0;

    virtual int QueryOverCurrent(uint &count, char *domainNames) = 0;
    virtual int SetCaptureTimeout(uint timeout_ms) = 0;
    virtual int SetInterface(uint _interface) = 0;
};

#endif // IHAL_CZTEK_H
