#include "OpenShort.h"
#include "ConfWidget.h"
#include "EnumNameMap.h"
#include <sstream>
#include <iostream>
#include "StdStringUtils.h"
#include <QDebug>

#define OS_TEST_SHORT_PIN_MASK  0x3FF

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

CZPLUGIN_API ICzPlugin *CreatePlugin(void *arg)
{
    return new OpenShort((const char *)arg);
}

CZPLUGIN_API void DestroyPlugin(ICzPlugin **plugin)
{
    OpenShort *pTestItem = (OpenShort *)(*plugin);
    if (pTestItem != nullptr) {
        delete pTestItem; pTestItem = nullptr;
        *plugin = nullptr;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

OpenShort::OpenShort(const char *instanceName)
{
    m_instanceName = QString::fromLatin1(instanceName);
    m_wdtConf = new ConfWidget();
    m_option = Option::GetInstance();
    m_context = nullptr;
    m_wdtConf->BindOpenShort(this);
    if (m_instanceName.isEmpty())
        m_instanceName = "Open Short";

    m_readOpenVol_uV = nullptr;
    m_readShortVol_uV = nullptr;
    m_readResult = nullptr;
    m_size = 0;
}

OpenShort::~OpenShort()
{
    delete m_wdtConf;
    Option::FreeInstance();
    m_context = nullptr;
    freeMemory();
}

int OpenShort::GetPluginInfo(T_PluginInfo &pluginInfo)
{
    pluginInfo.Version = 0x09000004;
    strcpy(pluginInfo.Description, "Open short.(20180319)");
    strcpy(pluginInfo.FriendlyName, "Open Short");
    strcpy(pluginInfo.InstanceName, m_instanceName.toLatin1().constData());
    strcpy(pluginInfo.VendorName, "CZTEK");
    pluginInfo.OptionDlgHandle = (long)m_wdtConf;

    return ERR_NoError;
}

int OpenShort::LoadOption()
{
    QMap<QString, QString> configurations;
    m_context->ModuleSettings->ReadSection(m_instanceName, configurations);
    if (configurations.contains("supplyVolt")) {
        m_option->SupplyVol_mV = configurations["supplyVolt"].toInt();
    }
    if (configurations.contains("supplyCurr")) {
        m_option->SupplyCur_uA = configurations["supplyCurr"].toInt();
    }
    if (configurations.contains("openThreshold")) {
        m_option->OpenThreshold_mV = configurations["openThreshold"].toInt();
    }
    if (configurations.contains("shortThreshold")) {
        m_option->ShortThreshold_mV = configurations["shortThreshold"].toInt();
    }
    if (configurations.contains("testPins")) {
        std::vector<std::string> testPinVector = StdStringUtils::split(configurations["testPins"].toStdString(), ",", true);
        m_option->TestPinVector.clear();
        for (auto it = testPinVector.begin(); it != testPinVector.end(); ++it) {
            m_option->TestPinVector.push_back(std::stoi(*it));
        }
    }
#ifdef DISABLE_CONTINUE_WHEN_FAILED
    m_option->ContinueWhenFailed = false;
#else
    if (configurations.contains("continueWhenFailed")) {
        m_option->ContinueWhenFailed = (configurations["continueWhenFailed"] == "true");
    }
#endif

    return m_wdtConf->Cache2Ui();
}

int OpenShort::SaveOption()
{
    int ec = m_wdtConf->Ui2Cache();
    if (ec == 0) {
        QMap<QString, QString> configurations;
        configurations["supplyVolt"] = QString::number(m_option->SupplyVol_mV);
        configurations["supplyCurr"] = QString::number(m_option->SupplyCur_uA);
        configurations["openThreshold"] = QString::number(m_option->OpenThreshold_mV);
        configurations["shortThreshold"] = QString::number(m_option->ShortThreshold_mV);
        std::ostringstream oss;
        for (auto it = m_option->TestPinVector.begin(); it != m_option->TestPinVector.end(); ++it) {
            if (it == m_option->TestPinVector.end() - 1)
                oss << *it;
            else
                oss << *it << ",";
        }
        configurations["testPins"] = QString::fromStdString(oss.str());
#ifdef DISABLE_CONTINUE_WHEN_FAILED
        configurations["continueWhenFailed"] = "false";
#else
        configurations["continueWhenFailed"] = m_option->ContinueWhenFailed ? "true" : "false";
#endif
        // save to file
        ec = m_context->ModuleSettings->WriteSection(m_instanceName, configurations);
    }
    return ec;
}

int OpenShort::RestoreDefaults()
{
    return m_wdtConf->RestoreDefaults();
}

int OpenShort::BindChannelContext(T_ChannelContext *context)
{
    m_context = context;
    return ERR_NoError;
}

ITestItem::E_ItemType OpenShort::GetItemType() const
{
    return ItemType_Hardware;
}

bool OpenShort::GetIsSynchronous() const
{
    return true;
}

QString OpenShort::GetReportHeader() const
{
    QString strHeader = "";
    EnumNameMap enumNameMap;
    if (m_readResult!=nullptr
            &&m_readOpenVol_uV!=nullptr
            &&m_readShortVol_uV!=nullptr)
    {
        for (size_t idx=0; idx<m_size; ++idx)
        {
            std::string pinName = enumNameMap.FindPinName(m_option->TestPinVector[idx]);
            strHeader.sprintf("%s%s(result),open volt(mv),short volt(mv),", strHeader.toStdString().c_str(), pinName.c_str());
        }
    }

    return strHeader;
}

QString OpenShort::GetReportContent() const
{
    QString strContent = "";
    if (m_readResult!=nullptr
            &&m_readOpenVol_uV!=nullptr
            &&m_readShortVol_uV!=nullptr)
    {
        for (size_t idx=0; idx<m_size; ++idx)
        {
            std::string osResult = (m_readResult[idx] == OS_Result_Pass) ? "OK" : "NG";
            strContent.sprintf("%s%s,%d,%d,", strContent.toStdString().c_str(),
                                            osResult.c_str(),
                                            m_readOpenVol_uV[idx]/1000,
                                            m_readShortVol_uV[idx]/1000);
        }
    }

    return strContent;
}

bool OpenShort::GetIsEngineeringMode() const
{
    return false;
}

IHardwareTest::E_HardwareTestType OpenShort::GetHardwareType() const
{
    return HardwareTest_OpenShort;
}

bool OpenShort::GetContinueWhenFailed() const
{
    return m_option->ContinueWhenFailed;
}

int OpenShort::Initialize()
{
    return ERR_NoError;
}

int OpenShort::RunTest(std::vector<std::string> &resultTable)
{
    if ((m_context == nullptr) || (m_context->ChannelController == nullptr)) {
        qCritical("Device controller instance is null!");
        return ERR_Failed;
    }
    resultTable.clear();
    resultTable.push_back("PinId,PinName,Result,Open Detail(mV),Short Detail(mV)");
    m_size = m_option->TestPinVector.size();
    if (false == allocaMemory())
    {
        m_context->ChannelController->LogToWindow("alloca memory failed.", qRgb(255, 0, 0));
        return ERR_Failed;
    }

    qDebug() << "m_size: " << m_size;
    uint *openStdVol_uv = new uint[m_size];
    uint *shortStdVol_uv = new uint[m_size];
    uint *testPins = new uint[m_size];
    for (uint i = 0; i < m_size; i++) {
        openStdVol_uv[i] = m_option->OpenThreshold_mV * 1000;
        shortStdVol_uv[i] = m_option->ShortThreshold_mV * 1000;
        testPins[i] = m_option->TestPinVector[i];
//        qDebug() << i;
    }

    int ec = m_context->ChannelController->SetOsTestConfig(m_option->SupplyVol_mV * 1000, m_option->SupplyCur_uA,
                                                  testPins, openStdVol_uv, shortStdVol_uv, m_size);
    if (ec < 0) {
        delete[] openStdVol_uv;
        delete[] shortStdVol_uv;
        delete[] testPins;
        return ec;
    }

    ec = m_context->ChannelController->ReadOsTestResult(testPins, m_readOpenVol_uV, m_readShortVol_uV, m_readResult, m_size);
    if (ec < 0) {
        qCritical("call ReadOsTestResult() falied");
    }
    else {
        EnumNameMap enumNameMap;
        for (uint i = 0; i < m_size; i++) {
            std::string openData = "";
            if (m_readResult[i] & OS_Result_Open) {
                openData = "Open: ";
                ec = -1;
            }
            openData += std::to_string(m_readOpenVol_uV[i] / 1000);

            std::string shortData = "";
            if (m_readResult[i] & OS_Result_Short) {
                uint shortPin = m_readResult[i] & OS_TEST_SHORT_PIN_MASK;
                std::string shortPinName = enumNameMap.FindPinName(shortPin);
                shortData = "Short Pin[" + shortPinName + "]: " + std::to_string(m_readShortVol_uV[i] / 1000);
                ec = -1;
            }
            std::string osResult = (m_readResult[i] == OS_Result_Pass) ? "OK" : "NG";

            std::ostringstream ossRow;
            std::string testPinName = enumNameMap.FindPinName(testPins[i]);
            ossRow << testPins[i] << "," << testPinName << "," << osResult << "," << openData << "," << shortData;
//            std::cout << ossRow.str() << std::endl;
            resultTable.push_back(ossRow.str());
        }
    }

    delete[] openStdVol_uv;
    delete[] shortStdVol_uv;
    delete[] testPins;
    //freeMemory(); //在GetReportContent中使用，不能在RunTest里释放
    return ec;
}

bool OpenShort::allocaMemory()
{
    freeMemory();
    m_readOpenVol_uV = new uint[m_size];
    m_readShortVol_uV = new uint[m_size];
    m_readResult = new uint[m_size];
    if ((nullptr == m_readOpenVol_uV)
                || (nullptr == m_readShortVol_uV)
                || (nullptr == m_readResult))
    {
        freeMemory();
        return false;
    }

    return true;
}

void OpenShort::freeMemory()
{
    if (nullptr != m_readOpenVol_uV)
    {
        delete[] m_readOpenVol_uV;
        m_readOpenVol_uV = nullptr;
    }

    if (nullptr != m_readShortVol_uV)
    {
        delete[] m_readShortVol_uV;
        m_readShortVol_uV = nullptr;
    }

    if (nullptr != m_readResult)
    {
        delete[] m_readResult;
        m_readResult = nullptr;
    }
}

