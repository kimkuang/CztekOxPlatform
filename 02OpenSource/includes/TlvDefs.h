#ifndef __TLVDEFS_H__
#define __TLVDEFS_H__
#include "czcmtidefs.h"
#include <QString>
#include <QMap>
#include <QMutex>

enum E_RpcReservedTag {
    Tag_RpcSessionId = 0xA0,
    Tag_RequesterId = 0xA1,

    Tag_Parameter_TestResult = 0x83,
    Tag_Parameter_ErrorInfo = 0x84,
};

enum E_RpcVendor {
    Vendor_ArcSoft = 0x01,
    Vendor_Qualcomm = 0x02,
    Vendor_Sony = 0x03,
    Vendor_Youth = 0x04,
    Vendor_Histar = 0x05,
    Vendor_Ofilm = 0x06,
};

// ArcSoft
enum E_RpcArcSoftApplication {
    ArcSoftApplication_HDC = 0x01,
};

// Youth
enum E_RpcYouthApplication {
    YouthApplication_DNP = 0x04,
};

// Histar
enum E_RpcHistarApplication {
    HistarApplication_DepthCalcCheck = 0x01,
    HistarApplication_CalibrationVerify = 0x03,
    HistarApplication_WDC = 0x04,
    HistarApplication_HwBlemish = 0x05,
    HistarApplication_HwMMI = 0x06, // 同HistarApplication_DepthCalcCheck，使用bat批处理，保持与r3一致
    HistarApplication_AFCTesterLaser = 0x07,
};

// Sony
enum E_RpcSonyApplication {
    SonyApplication_PdafCheck = 0x01,
};

// Ofilm
enum E_RpcOfilmApplication {
    OfilmApplication_OTPCheck = 0x01,
    OfilmApplication_PdafPdafCheck = 0x02,
    OfilmApplication_SaveImage = 0x03,
};

struct T_TagLenVal {
    uint8 Tag;
    uint32 Length;
    union {
        uint8 u8Value;
        uint16 u16Value;
        uint32 u32Value;
        uchar *szValue;  // Caller manage memory, DO NOT free buffer while using
    };
};

enum E_InvokeType {
    Invoke_Invalid = -1,
    Invoke_Sync = 0,
    Invoke_BeginAsync = 1,
    Invoke_EndAsync = 2,
    Invoke_OnlyBeginAsync = 3,
};

inline uint32 MakeRpcSessionId(uint8 vendor, uint8 application, E_InvokeType invokeType, uint8 seqNo)
{
    return ((vendor << 24) | (application << 16) | ((uint8)invokeType << 8) | seqNo);
}

inline uint32 GetIdFromRpcSessionId(uint32 sessionId)
{
    return (sessionId & 0xffff00ff);
}

inline uint8 GetVendorFromRpcSessionId(uint32 sessionId)
{
    return (sessionId >> 24) & 0xff;
}

inline uint8 GetApplicationFromRpcSessionId(uint32 sessionId)
{
    return (sessionId >> 16) & 0xff;
}

inline E_InvokeType GetInvokeTypeFromRpcSessionId(uint sessionId)
{
    return (E_InvokeType)((sessionId >> 8) & 0xff);
}

inline uint8 GetSeqNoFromRpcSessionId(uint32 sessionId)
{
    return sessionId & 0xff;
}

// 推荐使用此函数，保证全局唯一的seqNo
inline uint8 GetNextSeqNo()
{
    static uint8 g_seqNo = 0;
    QMutex mutext;
    mutext.lock();
    if (g_seqNo + 1 == 256)
        g_seqNo = 0;
    else
        g_seqNo++;
    uint8 seqNo = g_seqNo;
    mutext.unlock();
    return seqNo;
}

// 多通道模式下不再推荐使用此函数
inline uint8 GetNextSeqNo(uint8 &seqNo)
{
    if (seqNo + 1 == 256)
        seqNo = 0;
    else
        seqNo++;
    return seqNo;
}

inline QString GetVendorApplicationName(uint vendorId, uint applicationId, int isVendor)
{
    struct T_Application {
        uint VendorId;
        QString VendorName;
        uint ApplicationId;
        QString ApplicationName;
        T_Application(uint vendorId, const QString &vendorName, uint applicationId, const QString &applicationName)
        {
            VendorId = vendorId;
            VendorName = vendorName;
            ApplicationId = applicationId;
            ApplicationName = applicationName;
        }
    };
    QList<T_Application> applicationTable;
    applicationTable << T_Application(Vendor_ArcSoft, "ArcSoft", ArcSoftApplication_HDC, "HDC");
    applicationTable << T_Application(Vendor_Histar, "Histar", HistarApplication_DepthCalcCheck, "DepthCalcCheck");
    applicationTable << T_Application(Vendor_Histar, "Histar", HistarApplication_CalibrationVerify, "CalibrationVerify");
    applicationTable << T_Application(Vendor_Histar, "Histar", HistarApplication_WDC, "WDC");
    applicationTable << T_Application(Vendor_Histar, "Histar", HistarApplication_HwBlemish, "HwBlemish");
    applicationTable << T_Application(Vendor_Histar, "Histar", HistarApplication_HwMMI, "HwMMI");
    applicationTable << T_Application(Vendor_Histar, "Histar", HistarApplication_AFCTesterLaser, "AFCTesterLaser");
    applicationTable << T_Application(Vendor_Sony, "Sony", SonyApplication_PdafCheck, "PDAF");
    applicationTable << T_Application(Vendor_Youth, "Youth", YouthApplication_DNP, "DNP");
    applicationTable << T_Application(Vendor_Ofilm, "Ofilm", OfilmApplication_OTPCheck, "OTPCheck");
    applicationTable << T_Application(Vendor_Ofilm, "Ofilm", OfilmApplication_PdafPdafCheck, "PdafPdafcheck");
    applicationTable << T_Application(Vendor_Ofilm, "Ofilm", OfilmApplication_SaveImage, "Save Image");

    for (int i = 0; i < applicationTable.length(); i++) {
        if (vendorId == applicationTable[i].VendorId) {
            if (isVendor) {
                return applicationTable[i].VendorName;
            }
            else {
                if (applicationId == applicationTable[i].ApplicationId) {
                    return applicationTable[i].ApplicationName;
                }
            }
        }
    }
    return "Unknow";
}

#endif /* __TLVDEFS_H__ */
