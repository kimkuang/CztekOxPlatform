#include "TestItemSettingDialog.h"
#include "ui_TestItemSettingDialog.h"
#include "conf/SystemSettings.h"
#include <QPushButton>

TestItemSettingDialog::TestItemSettingDialog(uint windowIdx, uint chnIdx, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TestItemSettingDialog)
{
    ui->setupUi(this);
    setWindowTitle(windowTitle() + QString(" - %1").arg(windowIdx));

    m_moduleSettings = ModuleSettings::GetInstance(SystemSettings::GetInstance()->ProjectName, windowIdx);
    m_testItemService = TestItemService::GetInstance(chnIdx);
    initTabWidget();
    connect(ui->m_tabTestItemConf, &QTabWidget::currentChanged, this, &TestItemSettingDialog::tabTestItemConf_currentChanged);
    connect(ui->m_buttonBox, &QDialogButtonBox::clicked, this, &TestItemSettingDialog::buttonBox_clicked);
}

TestItemSettingDialog::~TestItemSettingDialog()
{
    for (int i = ui->m_tabTestItemConf->count() - 1; i >= 0; i--) {
        QWidget *widget = ui->m_tabTestItemConf->widget(i);
        if (widget) {
            qDebug()<<i<<widget->objectName();
            widget->setParent(nullptr); // plugin manage itself's conf window
        }
    }

    delete ui;
    m_moduleSettings = nullptr;
    m_testItemService = nullptr;
}

void TestItemSettingDialog::tabTestItemConf_currentChanged(int index)
{
    QString instanceName = ui->m_tabTestItemConf->tabText(index), fullInstanceName = "";
    if (m_mapInstName2InstFullName.contains(instanceName))
        fullInstanceName = m_mapInstName2InstFullName[instanceName];
    if (UiDefs::Operator_Operator == SystemSettings::GetInstance()->Operator)
    {
        T_PluginInfo pluginInfo;
        m_testItemService->GetPluginInfo(fullInstanceName, pluginInfo);
        QString friendlyName(pluginInfo.FriendlyName);
        if ((0 == friendlyName.compare("MES Check"))
                        || (0 == friendlyName.compare("MES Update")))
        {
            ui->m_buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
            ui->m_buttonBox->button(QDialogButtonBox::RestoreDefaults)->setEnabled(true);
            ui->m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        }
        else
        {
            ui->m_buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
            ui->m_buttonBox->button(QDialogButtonBox::RestoreDefaults)->setEnabled(false);
            ui->m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        }
    }

    buttonBox_clicked(ui->m_buttonBox->button(QDialogButtonBox::Apply));
}

void TestItemSettingDialog::buttonBox_clicked(QAbstractButton *button)
{
    QDialogButtonBox::StandardButton stdBtn = ui->m_buttonBox->standardButton(button);
    switch (stdBtn) {
    case QDialogButtonBox::RestoreDefaults: {
        for (int index = 0; index < ui->m_tabTestItemConf->count(); index++) {
            QString instanceName = ui->m_tabTestItemConf->tabText(index), fullInstanceName = "";
            if (m_mapInstName2InstFullName.contains(instanceName))
                fullInstanceName = m_mapInstName2InstFullName[instanceName];
            m_testItemService->RestoreDefaults(fullInstanceName);
        }
        break;
    }
    case QDialogButtonBox::Ok: {
        for (int index = 0; index < ui->m_tabTestItemConf->count(); index++) {
            QString instanceName = ui->m_tabTestItemConf->tabText(index), fullInstanceName = "";
            if (m_mapInstName2InstFullName.contains(instanceName))
                fullInstanceName = m_mapInstName2InstFullName[instanceName];
            if (m_testItemService->SaveSettings(fullInstanceName) != ERR_NoError)
                qCritical("Save settings failed.");
        }
        accept();
        break;
    }
    case QDialogButtonBox::Apply: {
        int index = ui->m_tabTestItemConf->currentIndex();
        QString instanceName = ui->m_tabTestItemConf->tabText(index), fullInstanceName = "";
        if (m_mapInstName2InstFullName.contains(instanceName))
            fullInstanceName = m_mapInstName2InstFullName[instanceName];
        if (m_testItemService->SaveSettings(fullInstanceName) != ERR_NoError)
            qCritical("Save settings failed.");
        break;
    }
    case QDialogButtonBox::Cancel:
        reject();
        break;
    default:
        break;
    }
}

void TestItemSettingDialog::initTabWidget()
{
    setUpdatesEnabled(false);
    QListIterator<QString> it(m_moduleSettings->ImageTestItemMap.values());
    while (it.hasNext()) {
        QString strTestItem = it.next();
        QStringList slSubItem = strTestItem.split(QChar(','), QString::KeepEmptyParts);
        if (slSubItem.size() < 3)
            continue;
        T_PluginInfo pluginInfo;
        QString fullInstanceName = slSubItem[1];
        if (m_testItemService->GetPluginInfo(fullInstanceName, pluginInfo) == ERR_NoError) {
            if (pluginInfo.OptionDlgHandle != 0) { // config ui
//                qDebug("%s", pluginInfo.InstanceName);
                QString instanceName = QString::fromLocal8Bit(pluginInfo.InstanceName);
//                qDebug()<<instanceName;
                ui->m_tabTestItemConf->addTab((QWidget *)(pluginInfo.OptionDlgHandle), instanceName);
                m_mapInstName2InstFullName[instanceName] = fullInstanceName;
                m_testItemService->LoadSettings(fullInstanceName);
            }
        }
    }

    tabTestItemConf_currentChanged(0);
    setUpdatesEnabled(true);
}
