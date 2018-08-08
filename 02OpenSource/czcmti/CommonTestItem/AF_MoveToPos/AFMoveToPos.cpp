#include "AFMoveToPos.h"
#include <QDebug>
#include "IOtpSensor.h"
#include "IVcmDriver.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

CZPLUGIN_API ICzPlugin *CreatePlugin(void *arg)
{
    return new AFMoveToPos((const char *)arg);
}

CZPLUGIN_API void DestroyPlugin(ICzPlugin **plugin)
{
    AFMoveToPos *pTestItem = (AFMoveToPos *)(*plugin);
    if (pTestItem != nullptr) {
        delete pTestItem; pTestItem = nullptr;
        *plugin = nullptr;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

AFMoveToPos::AFMoveToPos(const char *instanceName)
{
    m_instanceName = QString::fromLatin1(instanceName);
    m_initialized = false;
    m_option = new Option();
    m_wdtConf = new ConfWidget(m_option);
    m_context = nullptr;
    m_u64VcmCodeTimeStamp = 0;
    m_u16InfinityCode = 0;
    m_u16MacroCode = 0;
    m_u16MiddleCode = 0;
    m_u16MoveDistanceCode = 0;
}

AFMoveToPos::~AFMoveToPos()
{
    delete m_wdtConf;
    delete m_option;
}

int AFMoveToPos::GetPluginInfo(T_PluginInfo &pluginInfo)
{
    pluginInfo.Version = 0x09000004;
    strcpy(pluginInfo.Description, "AFMoveToPos(20180802)");
    strcpy(pluginInfo.FriendlyName, "AF_MoveToPos");
    strcpy(pluginInfo.InstanceName, m_instanceName.toLatin1().constData());
    strcpy(pluginInfo.VendorName, "CZTEK");
    pluginInfo.OptionDlgHandle = (uint64)m_wdtConf;

    return 0;
}

int AFMoveToPos::LoadOption()
{
    if ((m_context == nullptr) || (m_context->ChannelController == nullptr))
        return ERR_Failed;
    QMap<QString, QString> configurations;
    m_context->ModuleSettings->ReadSection(m_instanceName, configurations);
    if (configurations.contains("bSaveAFCode")) {
        m_option->m_bSaveAfCode = configurations["bSaveAFCode"].toInt();
    }
    if (configurations.contains("MoveToPosMethod")) {
        m_option->m_MoveToPosMethod = configurations["MoveToPosMethod"].toInt();
    }
    if (configurations.contains("ModuleEFL")) { // Effective Focal Length
        m_option->m_fEFL = configurations["ModuleEFL"].toFloat();
    }
    if (configurations.contains("InfinityDistance")) {
        m_option->m_fInifiDistance = configurations["InfinityDistance"].toFloat();
    }
    if (configurations.contains("InfinityAddrH")) {
        m_option->m_uInifiAddrH = configurations["InfinityAddrH"].toInt();
    }
    if (configurations.contains("InfinityAddrL")) {
        m_option->m_uInifiAddrL = configurations["InfinityAddrL"].toInt();
    }
    if (configurations.contains("MacroDistance")) {
        m_option->m_fMacroDistance = configurations["MacroDistance"].toFloat();
    }
    if (configurations.contains("MacroAddrH")) {
        m_option->m_uMacroAddrH = configurations["MacroAddrH"].toUShort();
    }
    if (configurations.contains("MacroAddrL")) {
        m_option->m_uMacroAddrL = configurations["MacroAddrL"].toUShort();
    }
    if (configurations.contains("OtherCode")) {
        m_option->m_uOtherCode = configurations["OtherCode"].toUShort();
    }
    if (configurations.contains("MoveDistance")) {
        m_option->m_fMoveDistance = configurations["MoveDistance"].toFloat();
    }
    if (configurations.contains("VCMDelay")) {
        m_option->m_uVcmDelay = configurations["VCMDelay"].toUInt();
    }

    return m_wdtConf->Cache2Ui();
}

int AFMoveToPos::SaveOption()
{
    if ((m_context == nullptr) || (m_context->ChannelController == nullptr))
        return ERR_Failed;

    int ec = m_wdtConf->Ui2Cache();
    if (ERR_NoError == ec)
    {
        QMap<QString, QString> configurations;
        configurations["bSaveAFCode"] = QString::number(m_option->m_bSaveAfCode);
        configurations["MoveToPosMethod"] = QString::number(m_option->m_MoveToPosMethod);
        configurations["ModuleEFL"] = QString::number(m_option->m_fEFL);
        configurations["InfinityDistance"] = QString::number(m_option->m_fInifiDistance);
        configurations["InfinityAddrH"] = QString::number(m_option->m_uInifiAddrH);
        configurations["InfinityAddrL"] = QString::number(m_option->m_uInifiAddrL);
        configurations["MacroDistance"] = QString::number(m_option->m_fMacroDistance);
        configurations["MacroAddrH"] = QString::number(m_option->m_uMacroAddrH);
        configurations["MacroAddrL"] = QString::number(m_option->m_uMacroAddrL);
        configurations["OtherCode"] = QString::number(m_option->m_uOtherCode);
        configurations["MoveDistance"] = QString::number(m_option->m_fMoveDistance);
        configurations["VCMDelay"] = QString::number(m_option->m_uVcmDelay);

        // save to file
        ec = m_context->ModuleSettings->WriteSection(m_instanceName, configurations);
    }

    return ec;
}

int AFMoveToPos::RestoreDefaults()
{
    return m_wdtConf->RestoreDefaults();
}

int AFMoveToPos::BindChannelContext(T_ChannelContext *context)
{
    m_context = context;
    LoadOption();
    return ERR_NoError;
}

ITestItem::E_ItemType AFMoveToPos::GetItemType() const
{
    return ITestItem::ItemType_ImageEvaluation;
}

bool AFMoveToPos::GetIsSynchronous() const
{
    return true;
}

QString AFMoveToPos::GetReportHeader() const
{
    QString strHeader = "";
    if (m_option->m_bSaveAfCode) {
        strHeader = "SaveAFCode";
        return strHeader;
    }

    return strHeader;
}

QString AFMoveToPos::GetReportContent() const
{
    QString strContent = "";
    if (m_option->m_bSaveAfCode) {
        strContent.sprintf("%d", m_u16SaveCode);
        return strContent;
    }

    return strContent;
}

bool AFMoveToPos::GetIsEngineeringMode() const
{
    return false;
}

int AFMoveToPos::Initialize(const T_FrameParam &frameParam)
{
    (void)frameParam;
    if (m_initialized) {
        return ERR_NoError;
    }
    m_u16SaveCode = 0;
    QString strLogInfo = "";
    strLogInfo.sprintf("Module EFL: %0.2f", m_option->m_fEFL);
    m_context->ChannelController->LogToWindow(strLogInfo, qRgb(0, 0, 255));

    // Infinity
    strLogInfo.sprintf("Infinity Distance: %0.2fm", m_option->m_fInifiDistance);
    m_context->ChannelController->LogToWindow(strLogInfo, qRgb(0, 0, 255));    
    uchar uCodeH = 0, uCodeL = 0;
    int iRet = m_context->OtpSensor->OtpRead(m_option->m_uInifiAddrH, m_option->m_uInifiAddrH, &uCodeH);
    iRet += m_context->OtpSensor->OtpRead(m_option->m_uInifiAddrL, m_option->m_uInifiAddrL, &uCodeL);
    if (ERR_NoError != iRet) {
        m_context->ChannelController->LogToWindow("Read infinity code failed.", qRgb(255, 0, 0));
        return ERR_Failed;
    }
    m_u16InfinityCode = (uCodeH << 8) + uCodeL;
    strLogInfo.sprintf("Infinity Code: %d", m_u16InfinityCode);
    m_context->ChannelController->LogToWindow(strLogInfo, qRgb(0, 0, 255));

    // Macro
    strLogInfo.sprintf("Macro Distance: %0.2fm", m_option->m_fMacroDistance);
    m_context->ChannelController->LogToWindow(strLogInfo, qRgb(0, 0, 255));
    iRet = m_context->OtpSensor->OtpRead(m_option->m_uMacroAddrH, m_option->m_uMacroAddrH, &uCodeH);
    iRet += m_context->OtpSensor->OtpRead(m_option->m_uMacroAddrL, m_option->m_uMacroAddrL, &uCodeL);
    if (ERR_NoError != iRet) {
        m_context->ChannelController->LogToWindow("Read macro code failed.", qRgb(255, 0, 0));
        return ERR_Failed;
    }
    m_u16MacroCode = (uCodeH << 8) + uCodeL;
    strLogInfo.sprintf("Macro Code: %d", m_u16MacroCode);
    m_context->ChannelController->LogToWindow(strLogInfo, qRgb(0, 0, 255));

    m_u16MiddleCode = (m_u16InfinityCode + m_u16MacroCode) / 2;
    strLogInfo.sprintf("Middle Code: %d", m_u16MiddleCode);
    m_context->ChannelController->LogToWindow(strLogInfo, qRgb(0, 0, 255));
    if (Option::Move_OtherPos == m_option->m_MoveToPosMethod) {
        m_u16MoveDistanceCode = calCodeByDistance();
        strLogInfo.sprintf("Code Of Distance: %d", m_u16MoveDistanceCode);
        m_context->ChannelController->LogToWindow(strLogInfo, qRgb(0, 0, 255));
    }
    else if (Option::Move_OtherCode == m_option->m_MoveToPosMethod) {
        strLogInfo.sprintf("Other Code: %d", m_option->m_uOtherCode);
        m_context->ChannelController->LogToWindow(strLogInfo, qRgb(0, 0, 255));
    }

    m_initialized = true;
    return ERR_NoError;
}

int AFMoveToPos::Evaluate(const uchar *image, uint64 &timestamp, std::vector<T_RoiInfo> &roiInfos)
{
    (void)image;
    roiInfos.clear();
    QString strLogInfo = "";
    uint Codes[] = {m_u16MoveDistanceCode, m_u16InfinityCode, m_u16MacroCode, m_u16MiddleCode, m_option->m_uOtherCode};
    if (ERR_NoError != m_context->VcmDriver->VcmWriteCode(Codes[m_option->m_MoveToPosMethod])) {
        strLogInfo.sprintf("Write Code: %d Failed.", Codes[m_option->m_MoveToPosMethod]);
        m_context->ChannelController->LogToWindow(strLogInfo, qRgb(255, 0, 0));
        m_u64VcmCodeTimeStamp = 0;
        m_initialized = true;
        return ERR_Failed;
    }

    uint64 u64Diff = 0;
    if (0 == m_u64VcmCodeTimeStamp) {
        m_u64VcmCodeTimeStamp = m_context->ChannelController->GetCurrentTimestamp();
    }
    if (timestamp > m_u64VcmCodeTimeStamp) {
        u64Diff = (timestamp - m_u64VcmCodeTimeStamp) / 1000;
    }
    if (m_option->m_uVcmDelay > u64Diff) {
        return ERR_Continue;
    }

    m_u16SaveCode = Codes[m_option->m_MoveToPosMethod];
    strLogInfo.sprintf("Write Code: %d Success", Codes[m_option->m_MoveToPosMethod]);
    m_context->ChannelController->LogToWindow(strLogInfo, qRgb(0, 0x80, 0));
    m_u64VcmCodeTimeStamp = 0;
    m_initialized = true;

    return ERR_NoError;
}

ushort AFMoveToPos::calCodeByDistance()
{
    float eflPow = m_option->m_fEFL * m_option->m_fEFL;
    float movePosLensShift = eflPow / (m_option->m_fEFL - m_option->m_fMoveDistance*1000);
    float infLensShift = eflPow / (m_option->m_fEFL - m_option->m_fInifiDistance*1000);
    float macLensShift = eflPow / (m_option->m_fEFL - m_option->m_fMacroDistance*1000);
    QString strLogInfo = "";
    strLogInfo.sprintf("InfinitLensShift: %f, MacroLensShift: %f, MoveLensShift: %f", infLensShift, macLensShift, movePosLensShift);
    m_context->ChannelController->LogToWindow(strLogInfo, qRgb(0, 200, 0));
    return 1.0 * (m_u16InfinityCode * (movePosLensShift - macLensShift) + m_u16MacroCode * (infLensShift - movePosLensShift)) / (infLensShift - macLensShift);
}
