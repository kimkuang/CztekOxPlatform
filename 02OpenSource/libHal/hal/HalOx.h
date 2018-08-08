#ifndef __HALOX_H__
#define __HALOX_H__

#include "library_global.h"
#include "IHalOx.h"

class HalOx_Cztek; // adaptee
class LIBRARY_API HalOx : public IHalOx
{
public:
    HalOx(uint channelIdx);
    virtual ~HalOx();
public:
    // Sensor control
    int WriteDiscreteI2c(uint slaveAddr, uint speedkHz, uint mode, const ushort *regs,
                            const ushort *values, uint regNum);
    int ReadDiscreteI2c(uint slaveAddr, uint speedkHz, uint mode, const ushort *regs,
                            ushort *values, uint regNum);
    int WriteContinuousI2c(uint slaveAddr, uint speedkHz, uint regAddr, uint regAddrSize,
                           const uchar *data, uint dataSize);
    int ReadContinuousI2c(uint slaveAddr, uint speedkHz, uint regAddr, uint regAddrSize,
                          uchar *data, uint dataSize);
    int SetSensorClock(uint clk100kHz);
    int GetSensorClock(uint &clk100kHz);
    int SetMipiParam(uint laneNum, uint freqMHz, uint virtualChannel);
    int GetMipiParam(uint &laneNum, uint &freqMHz, uint &virtualChannel);
    int SetSensorGpioPinLevel(ushort pin, ushort level);
    int GetSensorGpioPinLevel(ushort pin, ushort &level);
    int SetSensorGpioPinDir(ushort pin, ushort dir) ; //1:input,0:output
    int GetSensorGpioPinDir(ushort pin, ushort &dir) ; //1:input,0:output
    int SetSensorPower(const uint *powerIds, const uint *voltagemV, const uint *delayms, uint count);
    int GetSensorPower(const uint *powerIds, uint *voltagemV, uint *delayms, uint count);

    // Video Management
    int SetFrameParam(uint imageFormat, uint imageMode, uint width, uint height, uint outImageFormat,
                      uint cropLeft, uint cropTop, uint cropWidth, uint cropHeight);
    int GetFrameParam(uint &imageFormat, uint &imageMode, uint &width, uint &height, uint &size,
                      uint &cropLeft, uint &cropTop, uint &cropWidth, uint &cropHeight);
    int SetVideoControl(uint ctrl);
    int SetSensorPort(uint port);
    int GetSensorPort(uint &port);
    int DequeueBuffer(int &index, uchar **data, timeval &timeVal);
    int EnqueueBuffer(int index);
//    int EnqueueBuffer(int index, const timeval &timestamp);

    // Measure
    int SetOsTestConfig(uint supplyVol_uV, uint supplyCurrent_uA, const uint *pinId,
                        const uint *openStdVol_uV, const uint *shortStdVol_uV, uint count);
    int ReadOsTestResult(const uint *reqPins, uint *openVol_uV, uint *shortVol_uV, uint *result, uint count);
    int GetCurrent(const uint *powerId, const uint *currentRange, float *current_nA, uint count);

    // others
    int BeepOn(uint ms);
    int LedFlash(bool on);

    // extend gpio
    int SetExtendGpioPinLevel(ushort pin, ushort level);
    int GetExtendGpioPinLevel(ushort pin, ushort &level);
    int SetExtendGpioPinDir(ushort pin, ushort dir); //1:input,0:output
    int GetExtendGpioPinDir(ushort pin, ushort &dir); //1:input,0:output

    int GetHalVersion(char *buffer, int len);
    int GetDriverVersion(char *version, int len);
    int GetBarcode(char *barcode, int len);

    int QueryOverCurrent(uint &count, char *domainNames);
    int SetCaptureTimeout(uint timeout_ms);
    int SetInterface(uint _interface);

private:
    HalOx_Cztek *m_halOxCztek;
};

#endif // __HALOX_H__
