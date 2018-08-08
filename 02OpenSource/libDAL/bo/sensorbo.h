#ifndef SENSORBO_H
#define SENSORBO_H
#include <string>
#include <vector>
#include <map>
#include "czcmtidefs.h"

struct T_PWDN_CFG {
#define DEF_PWDN_INIT_IS_VALID    0
#define DEF_PWDN_KEEP_DELAY_MS    2
#define DEF_PWDN_RELEASE_DELAY_MS 10
    int val;
    int initIsValid;
    int keepDelayMs;
    int releaseDelayMs;

    inline T_PWDN_CFG() {
        Default();
    }

    inline void Default() {
        val = 0;
        initIsValid = DEF_PWDN_INIT_IS_VALID;
        keepDelayMs = DEF_PWDN_KEEP_DELAY_MS;
        releaseDelayMs = DEF_PWDN_RELEASE_DELAY_MS;
    }
};

struct T_RESET_CFG {
#define DEF_RESET_KEEP_DELAY_MS    10
#define DEF_RESET_RELEASE_DELAY_MS 30
    int val;
    int keepDelayMs;
    int releaseDelayMs;

    inline T_RESET_CFG() {
        Default();
    }

    inline void Default() {
        val = 0;
        keepDelayMs = DEF_RESET_KEEP_DELAY_MS;
        releaseDelayMs = DEF_RESET_RELEASE_DELAY_MS;
    }
};

enum E_AF_MODE {
    AF_MODE_INVALID = -1,
    AF_MODE_SENSOR,
    AF_MODE_DRIVER_IC,
    AF_MODE_IAF
};

struct T_FocusParam {
    int Type; // Ref E_AF_MODE
    int SubType;
//    int FarPosition;
//    int NearPosition;
};

struct T_Size
{
    int Width;
    int Height;
    T_Size()
    {
        Width = Height = 0;
    }
    T_Size(int w, int h)
    {
        Width = w;
        Height = h;
    }
};
struct T_SpiCommParam {
    E_RegBitsMode RegBitsMode;
    uint ChipId;  // 0x00 stands for NO ChipId
    uint Speed;   // unit:100kHz, 0 stands for max(400k)
    ushort Mode;  // 16bit, from linux kernel
    uchar DataBits;
    bool IsBigEndian;
};

struct T_CommIntfConf {
    enum E_IntfType { IT_I2C = 0, IT_SPI };
    E_IntfType IntfType;
    union {
        T_I2CCommParam I2C;
        T_SpiCommParam Spi;
    };
};

struct T_LvdsParam {
    std::string SeaFileName;
    std::string SedFileName;
    uint Version;
    uint Mode;
    uint V_Total;
    uint H_Total;
    uint V_H_Blank;
    uint Crop_Top;
    T_LvdsParam() {
        Reset();
    }
    void Reset() {
        SeaFileName = SedFileName = "";
        Version = Mode = V_Total = H_Total = V_H_Blank = 0;
        Crop_Top = 0;
    }
};

struct T_REG_CFG {
    uint addr;
    uint value;
    uint delay_ms;
    uint mask;
    T_REG_CFG() {
        new (this)T_REG_CFG(0, 0, 0, 0xff);
    }
    T_REG_CFG(uint addr, uint value) {
        new (this)T_REG_CFG(addr, value, 0, 0xff);
    }
    T_REG_CFG(uint addr, uint value, uint delayMs) {
        new (this)T_REG_CFG(addr, value, delayMs, 0xff);
    }
    T_REG_CFG(uint _addr, uint _value, uint _delayMs, uint _mask) {
        addr = _addr;
        value = _value;
        delay_ms = _delayMs;
        mask = _mask;
    }
};
typedef std::vector<T_REG_CFG> T_REG_CFG_LIST;

struct T_HiLoRegCfg {
    std::vector<T_RegConf> Registers;
    int MinVal;
    int MaxVal;
    int ValRegCnt;
};

struct T_OtpExposureParam {
    uint StartStep;
    uint EndStep;
    uint StepValue;
    T_OtpExposureParam() {
        StartStep = EndStep = StepValue = 0;
    }
};

struct T_OtpWbParam {
    T_Size WbRoi; // width(%), height(%)
    uint GCodeRef;
    uint GCodeOffset;
    T_OtpWbParam() {
        WbRoi = T_Size(0, 0);
        GCodeRef = GCodeOffset = 0;
    }
};

struct T_OtpLscParam {
    T_Size LscRoi; // width(%), height(%)
    uint DestRatioLower;
    uint DestRatioUpper;
    uint RatioR;
    uint RatioGr;
    uint RatioGb;
    uint RatioB;
    T_OtpLscParam() {
        LscRoi = T_Size(0, 0);
        DestRatioLower = DestRatioUpper = 0;
        RatioR = RatioGr = RatioGb = RatioB = 0;
    }
};

struct T_OtpAlgParams {
    T_OtpExposureParam ExposureParam;
    T_Size CenterRoi; // width(%), height(%)
    T_OtpWbParam WbParam;
    T_OtpLscParam LscParam;
    T_OtpAlgParams() {
        CenterRoi = T_Size(0, 0);
    }
};

struct T_DOMAIN {
    int id;//string
    int value;
    int delay_ms;
    int sequence;
    T_DOMAIN() {
        new (this)T_DOMAIN(0, 0, 0, 0);
    }
    T_DOMAIN(int _id, int _value, int _delayMs, int _sequence) {
        id = _id;
        value = _value;
        delay_ms = _delayMs;
        sequence = _sequence;
    }
};

struct T_SENSOR_CFG {
    int id = 0;                 // db's ID, used for comparision(<1 is invalid)
    std::string SchemeName;
    std::string ChipName;
    int Mclk_kHz;
    E_InterfaceType InterfaceType;
    int MipiFreq;
    int Lanes;
    int DataWidth;
    int PclkPol;
    int DataPol;
    int HsyncPol;
    int VsyncPol;
    T_PWDN_CFG pwdn;
    T_RESET_CFG reset;
    T_FocusParam FocusParam;

    E_ImageFormat ImageFormat;
    E_ImageMode ImageMode;
    int PixelWidth;
    int PixelHeight;
    int QuickWidth;
    int QuickHeight;
    T_Rect CropParam;

    T_CommIntfConf CommIntfConf;
    T_LvdsParam LvdsParam;

    std::vector<T_RegConf> FullModeParams;
    std::vector<T_RegConf> OtpInitParams;
    std::vector<T_RegConf> QuickModeParams;
    std::vector<T_RegConf> SleepParams;
    std::vector<T_RegConf> AfInitParams;
    std::vector<T_RegConf> AfAutoParams;
    std::vector<T_RegConf> AfFarParams;
    std::vector<T_RegConf> AfNearParams;
    T_HiLoRegCfg ExposureParam;
    T_HiLoRegCfg GainParam;
    T_OtpAlgParams OtpAlgDefParams;
    std::vector<T_RegConf> FlagRegisters;
    std::vector<T_Power> Domains;
};

//inline void DomainListToMap(const std::vector<T_DOMAIN> &domainList, std::map<int, T_DOMAIN> &domainMap)
//{
//    domainMap.clear();
//    for (uint i = 0; i < domainList.size(); i++) {
//        domainMap.insert(std::make_pair(domainList[i].id, domainList[i]));
//    }
//}

#ifdef QT_CORE_LIB // qt extension
//#include "sensorbo_qt.h"
#endif

#endif // SENSORBO_H
