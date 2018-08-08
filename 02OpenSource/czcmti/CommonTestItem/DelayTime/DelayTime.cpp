#include "DelayTime.h"
#include <QDebug>
#include <QThread>
#include <QCoreApplication>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

CZPLUGIN_API ICzPlugin *CreatePlugin(void *arg)
{
    return new DelayTime((const char *)arg);
}

CZPLUGIN_API void DestroyPlugin(ICzPlugin **plugin)
{
    DelayTime *pTestItem = (DelayTime *)(*plugin);
    if (pTestItem != nullptr) {
        delete pTestItem; pTestItem = nullptr;
        *plugin = nullptr;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

DelayTime::DelayTime(const char *instanceName)
{
    m_instanceName = QString::fromLatin1(instanceName);
    m_option = new Option();
    m_wdtConf = new ConfWidget(m_option);
    m_context = nullptr;
}

DelayTime::~DelayTime()
{
    delete m_wdtConf;
    delete m_option;
}

int DelayTime::GetPluginInfo(T_PluginInfo &pluginInfo)
{
    pluginInfo.Version = 0x09000001;
    strcpy(pluginInfo.Description, "Delay Time.");
    strcpy(pluginInfo.FriendlyName, "DelayTime");
    strcpy(pluginInfo.VendorName, "CZTEK");
    strcpy(pluginInfo.InstanceName, m_instanceName.toLatin1().constData());
    pluginInfo.OptionDlgHandle = (uint64)m_wdtConf;

    return 0;
}

int DelayTime::LoadOption()
{
    if ((m_context == nullptr) || (m_context->ChannelController == nullptr))
        return ERR_Failed;
    QMap<QString, QString> configurations;
    m_context->ModuleSettings->ReadSection(m_instanceName, configurations);
    if (configurations.contains("m_DelayTime"))
        m_option->m_DelayTime = configurations["m_DelayTime"].toInt();
    if (configurations.contains("m_Enable"))
        m_option->m_Enable = (configurations.contains("m_Enable") == 1);

    return m_wdtConf->Cache2Ui();
}

int DelayTime::SaveOption()
{
    if ((m_context == nullptr) || (m_context->ChannelController == nullptr))
        return ERR_Failed;
    int ec = m_wdtConf->Ui2Cache();
    if (ec == 0) {
        QMap<QString, QString> configurations;
        configurations["m_Enable"] =  QString::number(m_option->m_Enable ? 1 : 0);
        configurations["m_DelayTime"] = QString::number(m_option->m_DelayTime);

        // save to file
        ec = m_context->ModuleSettings->WriteSection(m_instanceName, configurations);
    }
    return ec;
}

int DelayTime::RestoreDefaults()
{
    return m_wdtConf->RestoreDefaults();
}

int DelayTime::BindChannelContext(T_ChannelContext *context)
{
    m_context = context;
    LoadOption();
    return ERR_NoError;
}

ITestItem::E_ItemType DelayTime::GetItemType() const
{
    return ITestItem::ItemType_ImageEvaluation;
}

bool DelayTime::GetIsSynchronous() const
{
    return true;
}

QString DelayTime::GetReportHeader() const
{
    QString strHeader = "";
    return strHeader;
}

QString DelayTime::GetReportContent() const
{
    QString strContent = "";
    return strContent;
}

bool DelayTime::GetIsEngineeringMode() const
{
    return false;
}

int DelayTime::Initialize(const T_FrameParam &frameParam)
{
    (void)frameParam;
    return ERR_NoError;
}

int DelayTime::Evaluate(const uchar *image, uint64 &timestamp, std::vector<T_RoiInfo> &roiInfos)
{
    (void)(timestamp);
    (void)(image);
    (void)(roiInfos);

    if(m_option->m_Enable)
    {
        QThread::msleep(m_option->m_DelayTime);
    }

    return ERR_NoError;
}

