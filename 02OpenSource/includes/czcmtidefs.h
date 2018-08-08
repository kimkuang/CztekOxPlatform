#ifndef __CZCMTIDEF_H__
#define __CZCMTIDEF_H__

typedef unsigned short uint16;
typedef unsigned int uint32;
#if defined _MSC_VER || defined __BORLANDC__
    typedef __int64 int64;
    typedef unsigned __int64 uint64;
#else
    #include <inttypes.h>
/***
 * REF:
 *     https://github.com/goldendict/goldendict/issues/714
 *     http://blog.csdn.net/shiwei408/article/details/7463476
 *
 * NOTICE:
 * qendian.h have qbswap template for quint64 (defined as "unsigned long long" in qglobal.h) parameter
 * but haven't it for uint64_t (defined as "unsigned long" on UNIX-like systems).
*/
//    typedef /*int64_t*/long long int64;   // conflicting declaration with opencv!!!
//    typedef /*uint64_t*/unsigned long long uint64;
    typedef int64_t/*long long*/ int64;   // conflicting declaration with opencv!!!
    typedef uint64_t/*unsigned long long*/ uint64;
#endif

typedef unsigned char uchar;
typedef unsigned char byte;
typedef unsigned char uint8;
typedef char int8;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

#ifdef _WIN32
    #include <windows.h>
#else
typedef void *LPVOID;
typedef unsigned long       DWORD;
typedef bool                BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef unsigned int        UINT;
typedef char            CHAR, TCHAR;
typedef unsigned short  USHORT;
typedef unsigned int    UINT32;
typedef int             INT32;
#endif


#ifdef TRUE
#undef TRUE
#endif
#define TRUE true

#ifdef FALSE
#undef FALSE
#endif
#define FALSE false


#define LogRed      qRgb(0xFF, 0, 0)
#define LogGreen    qRgb(0, 0x80, 0)
#define LogBlue     qRgb(0, 0, 0xFF)

//CacheData Key String
#define DECODESHOWMODE      "DecodeShowMode"   // 0:none 1:jiugongge 2:mizige 3:tianzige 4:pingxingxian
#define MESLINKSTATUS       "MesLinkStatus"    // 0:On  1:Off

struct T_Rect
{
    int X;
    int Y;
    int Width;
    int Height;
};

enum E_PowerId {
    PI_DVDD = 0,
    PI_AVDD,
    PI_DOVDD,
    PI_AFVCC,
    PI_VPP,
    PowerId_MaxPowerCount,
};

struct T_Power {
    int Id;  // E_PowerId
    int Value;
    int Delay_ms;
};

#define GPIO_DIR_INPUT      1
#define GPIO_DIR_OUTPUT     0

enum E_Gpio {
    IO_Reset = 1 << 0,
    IO_Pwdn1 = 1 << 1,
    IO_Pwdn2 = 1 << 2,
    IO_PO1 = 1 << 3,
    IO_PO2 = 1 << 4,
    IO_PO3 = 1 << 5,
    IO_PO4 = 1 << 6,
};

enum E_ExtGpio {
    ExtGpio1 = 1 << 0,
    ExtGpio2 = 1 << 1,
    ExtGpio3 = 1 << 2,
};

enum E_InterfaceType {
    IT_MIPI = 0,
    IT_DVP,
    IT_MTK,
    IT_SPI,
    IT_HIPI,
    IT_SPREADTRUM,
    IT_TV,
    IT_UVC,
    IT_LVDS,
    IT_MAX_VAL = IT_LVDS,
    IT_MIPI_DVP = (IT_DVP << 8) | IT_MIPI, // NOTICE: non-db value, only for ui
};

#define ESC_MODE 0xBADA0001 // Escape character for IIC mode
#define ESC_ADDR 0xBADA0002 // Escape character for IIC address

enum E_RegBitsMode {
    RB_NORMAL = 0,        // 8Addr,8Data
    RB_ADDR8_DATA8,       // 8Addr,8Data
    RB_ADDR8_DATA16,      // 8Addr,16Data
    RB_ADDR16_DATA8,      // 16Addr,8Data
    RB_ADDR16_DATA16,     // 16Addr,16Data
};

struct T_I2CCommParam {
    E_RegBitsMode RegBitsMode;
    uint Addr;
    uint Speed; // unit:100kHz, 0 stands for max(400k)
};

struct T_RegConf {
    uint Addr;
    uint Value;
    uint Delay_ms;
    uint Mask;
};

typedef enum {
    IMAGE_FMT_INVALID = -1,
    IMAGE_FMT_RAW8 = 0,
    IMAGE_FMT_RAW10 = 1, // unpacked, 2 bytes
    IMAGE_FMT_RAW12 = 2, // unpacked, 2 bytes
    IMAGE_FMT_RAW14 = 3, // unpacked, 2 bytes
    IMAGE_FMT_RAW16 = 4,
    IMAGE_FMT_RGB16 = 5,
    IMAGE_FMT_RGB24 = 6,
    IMAGE_FMT_RGB32 = 7,
    IMAGE_FMT_YUV422 = 8,
    IMAGE_FMT_YUV420 = 9,
    IMAGE_FMT_LUMINANCE = 10,
    IMAGE_FMT_PackedRaw10 = 11, // packed
    IMAGE_FMT_PackedRaw12 = 12, // packed
    IMAGE_FMT_PackedRaw14 = 13, // packed
} E_ImageFormat;

enum E_ImageMode {
    IMAGE_MODE_INVALID = -1,
    IMAGE_MODE_YCbYCr_RG_GB = 0,
    IMAGE_MODE_YCrYCb_GR_BG,
    IMAGE_MODE_CbYCrY_GB_RG,
    IMAGE_MODE_CrYCbY_BG_GR,
};

struct T_FrameParam {
    E_ImageFormat ImageFormat;
    E_ImageMode   ImageMode;
    uint Width;
    uint Height;
    uint Size;      // bytes
    T_FrameParam() {
        Reset();
    }
    T_FrameParam(E_ImageFormat imgFormat, E_ImageMode imgMode, int width, int height) {
        ImageFormat = imgFormat;
        ImageMode = imgMode;
        Width = width;
        Height = height;
        Size = 0;
    }
    T_FrameParam(E_ImageFormat imgFormat, E_ImageMode imgMode, int width, int height, uint size) {
        ImageFormat = imgFormat;
        ImageMode = imgMode;
        Width = width;
        Height = height;
        Size = size;
    }
    void Reset() {
        ImageFormat = IMAGE_FMT_INVALID;
        ImageMode = IMAGE_MODE_INVALID;
        Width = Height = Size = 0;
    }
};

//struct T_Image {
//    unsigned char *Image;
//    E_ImageFormat Format;
//    E_ImageMode Mode;
//    uint Width;
//    uint Height;
//    uint Size;
//    uint64 Timestamp;
//    T_Image() {
//        Image = nullptr;
//        Format = IMAGE_FMT_INVALID;
//        Mode = IMAGE_MODE_INVALID;
//        Width = Height = Size = 0;
//        Timestamp = 0;
//    }
//};

struct T_SensorSetting {
    int Id;                 // greater than 0
    char *SchemeName;
    char *ChipName;
    int Mclk_kHz;
    E_InterfaceType InterfaceType;
    int MipiFreq;
    int Lanes;
    int DataWidth;
    int PclkPol;
    int DataPol;
    int HsyncPol;
    int VsyncPol;
    int Pwdn;
    int Reset;

    E_ImageFormat ImageFormat;
    E_ImageMode ImageMode;
    int PixelWidth;
    int PixelHeight;
    int QuickWidth;
    int QuickHeight;
    T_Rect CropParam;

    T_I2CCommParam I2cParam;

    int FullModeParamCount;
    T_RegConf *FullModeParams;
    int OtpInitParamCount;
    T_RegConf *OtpInitParams;
    int SleepParamCount;
    T_RegConf *SleepParams;
    int AfInitParamCount;
    T_RegConf *AfInitParams;
    int AfAutoParamCount;
    T_RegConf *AfAutoParams;
    int AfFarParamCount;
    T_RegConf *AfFarParams;
    int AfNearParamCount;
    T_RegConf *AfNearParams;
    int ExposureParamCount;
    T_RegConf *ExposureParams;
    int GainParamCount;
    T_RegConf *GainParams;
    int FlagRegisterCount;
    T_RegConf *FlagRegisters;
    int PowerCount;
    T_Power *Powers;

    T_SensorSetting(){
        CropParam.Height = CropParam.Width = CropParam.X = CropParam.Y = 0;
    }
};

enum E_OSM_PIN_TYPE {
    OSM_PIN_INVALID = -1,
    OSM_PIN_AVDD = 0,
    OSM_PIN_DOVDD,
    OSM_PIN_DVDD,
    OSM_PIN_AFVCC,
    OSM_PIN_VPP,
    OSM_PIN_MCLK,
    OSM_PIN_SCL,
    OSM_PIN_SDA,
    OSM_PIN_PWDN,
    OSM_PIN_RST,
    OSM_PIN_PO1,
    OSM_PIN_PO2,
    OSM_PIN_PO3,
    OSM_PIN_PO4,
    OSM_PIN_MIPI_D3P,
    OSM_PIN_MIPI_D3N,
    OSM_PIN_MIPI_D2P,
    OSM_PIN_MIPI_D2N,
    OSM_PIN_MIPI_D1P,
    OSM_PIN_MIPI_D1N,
    OSM_PIN_MIPI_D0P,
    OSM_PIN_MIPI_D0N,
    OSM_PIN_MIPI_CLKP,
    OSM_PIN_MIPI_CLKN,
    OSM_PIN_SGND1,
    OSM_PIN_SGND2,
    OSM_PIN_SGND3,
    OSM_PIN_SGND4,
};
#define OS_TEST_SHORT_PIN_MASK  0x3FF

enum E_OsTest_Result {
    OS_Result_Pass = 0,
    OS_Result_Open = (1 << 15),
    OS_Result_Short = (1 << 14),
};

enum E_CurrentRange {
    CurrentRange_mA = 0,
    CurrentRange_uA,
    CurrentRange_nA
};

#define CURRENT_RESULT_UPOVERFLOW   (-1.0)

enum E_ErrorCode {
    ERR_NoError = 0,
    ERR_Continue = 1,
//    ERR_RestartVideoSuccess = 2,
    ERR_SensorPowerOn = -1,
    ERR_SetMclk = -2,
    ERR_SetGpio = -3,
    ERR_NotImplemented = -4,
    ERR_VideoDequeuTimeout = -5,
    ERR_VideoNoDevice = -6,
    ERR_VideoCritical = -7,
    ERR_CheckFlagRegister = -8,
    ERR_InitExtCtrlParam = -9,
    ERR_OpenShortTest = -10,
    ERR_SetMipiParam = -11,
    ERR_GetGpio = -12,
    ERR_SetVideoParam = -13,
    ERR_StartVideo = -14,
    ERR_I2cWrite = -15,
    ERR_I2cRead = -16,
    ERR_Beep = -17,
    ERR_I2cWriteBlock = -18,
    ERR_I2cReadBlock = -19,
    ERR_GetMclk = -20,
    ERR_SetVideoExtendParam = -21,
    ERR_GetMipiParam = -22,
    ERR_SetGpioDir = -23,
    ERR_GetGpioDir = -24,
    ERR_SetFramParam = -25,
    ERR_GetFramParam = -26,
    ERR_SetSensorPower = -27,
    ERR_GetSensorPower = -28,
    ERR_Failed = -29,
    ERR_InvalidParameter = -30,
    ERR_DismatchedFlags = -31,
    ERR_EmptyBufferPool = -32,
    ERR_ReadCurrent = -33,
    ERR_DeviceOffline = -34,
    ERR_LoadLibrary = -35,
    ERR_NO_MEMEORY = -36,
    ERR_FileNotFound = -37,
    ERR_NotFinished = -38,
//    ERR_RestartVideoFail = -39,
    ERR_ConnectFailed = -40,

    ERR_IPC_MQ_GET = -1000,
    ERR_IPC_PDU = -1001,
    ERR_IPC_MQ_SEND = -1002,
    ERR_IPC_MQ_RECV = -1003,
    ERR_IPC_SM_ATTACH = -1004,
    ERR_Unkown_Ipc_Cmd = -1005,

    ERR_GEV_READ = -1006,
    ERR_GEV_WRITE = -1007,
    
    ERR_READ_REG = -2002,
    ERR_READ_MEM = -2003,
    ERR_WRITE_REG = -2004,
    ERR_WRITE_MEM = -2005,

    ERR_OtpPregroupIsEmpty = -3000,
    ERR_OtpGroupIsNotEmpty = -3001,
    ERR_OtpGroupIsNotAvailable = -3002,
    ERR_OtpCheckData = -3003,
    ERR_OtpWriteData = -3004,
    ERR_OtpTimeout = -3005,
    
    ERR_Unkown = -9999,
};

#define MAX_REG_COUNT_PER_PACKET (130)

enum E_FileType {
    FileType_Upgrade = 0,
    FileType_Otp,
    FileType_TestItem
};

enum E_OtpSegmentType {
    Segment_BaseInfo = 0,
    Segment_WB,
    Segment_WB_Golden,
    Segment_AF,
    Segment_LSC,
    Segment_PDAF,
    Segment_WaferInfo,
    Segment_DeadPixel,
    Segment_MaxSegmentType,
};

enum E_OtpStatusType
{
    OtpStatus_Ready = 0,
    OtpStatus_Pending,
    OtpStatus_Finished,
};

enum E_TransmitterType
{
    Transmitter_GigE = 0,
    Transmitter_Ipc,
    Transmitter_Usb,
};

enum E_TransferMode
{
    TransferMode_Capture = 0x01,
    TransferMode_TransferByGigE = 0x02,
    TransferMode_TransferByUsb = 0x04,
    TransferMode_TransferByIpc = 0x08
};

enum E_ImageType
{
    Image_JPG = 0x01,
    Image_BMP = 0x02,
    Image_Raw8 = 0x03,
    Image_Raw10 = 0x04,
    Image_MyRaw = 0x05
};

enum E_MesSelect
{
    Mes_Check = 0x01,
    Mes_Binding = 0x02,
    Mes_Update = 0x03,
    Mes_Unbinding = 0x04,
};

#ifdef __cplusplus
#include <string>
struct T_RoiInfo {
    enum {
        Marker_Rectangle,
        Marker_Circle,
    } MarkerType;
    uint X;
    uint Y;
    uint Width;
    uint Height;
    std::string Label;
    uint MarkerColor;
    uint LabelColor;
    uint LineWidth;
    uint FontSize;
    T_RoiInfo() {
        X = Y = Width = Height = 0;
        Label = "";
        MarkerColor = LabelColor = 0x0000ff;
        LineWidth = 2;
        FontSize = 8;
    }
};
#endif /* __cplusplus */

#define DRV_TAB_ITEM(X)\
		{\
		    #X, Get##X\
		}

#define DRV_ITEM_ALLOC(X)\
    template<typename T1, typename T2>\
    T1 *Get##X(T2 &p)\
    {\
	return new X(p);\
    }

#endif // __CZCMTIDEF_H__
