#include "MachineSettingsDialog.h"
#include "ui_MachineSettingsDialog.h"
#include <QDir>
#include <QMessageBox>
#include <QRadioButton>
#include "common/Global.h"

MachineSettingsDialog::MachineSettingsDialog(SystemSettings *systemSettings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MachineSettingsDialog)
{
    ui->setupUi(this);

    m_systemSettings = systemSettings;
    m_machineService = MachineService::GetInstance();
    initListWidget();
    connect(ui->m_buttonBox, &QDialogButtonBox::clicked, this, &MachineSettingsDialog::buttonBox_clicked);
}

MachineSettingsDialog::~MachineSettingsDialog()
{
    for (int i = ui->m_stackedWidget->count() - 1; i >= 0; i--) {
        QWidget *widget = ui->m_stackedWidget->widget(i);
        if (widget) {
            qDebug()<<i<<widget->objectName();
            widget->setParent(nullptr); // plugin manage itself's conf window
        }
    }

    delete ui;
    m_systemSettings = nullptr;
    m_machineService = nullptr;
}

void MachineSettingsDialog::buttonBox_clicked(QAbstractButton *button)
{
    QDialogButtonBox::StandardButton stdBtn = ui->m_buttonBox->standardButton(button);
    switch (stdBtn) {
    case QDialogButtonBox::RestoreDefaults: {
        QListWidgetItem *item = ui->m_lstwdtMachineList->currentItem();
        if (item == nullptr) {
            QMessageBox::critical(this, tr("Error"), tr("Please select the item to operate firstly."));
            break;
        }
        QString libFileName = item->data(Qt::UserRole).toString();
        m_machineService->RestoreDefaults(libFileName);
        break;
    }
    case QDialogButtonBox::Ok: {
        for (int i = 0; i < ui->m_lstwdtMachineList->count(); i++) {
            QListWidgetItem *item = ui->m_lstwdtMachineList->item(i);
            if (item == nullptr)
                continue;
            QRadioButton *radioButton = static_cast<QRadioButton *>(ui->m_lstwdtMachineList->itemWidget(item));
            if (radioButton == nullptr)
                continue;
            if (radioButton->isChecked()) {
                QString libFileName = item->data(Qt::UserRole).toString();
                qInfo()<<"switch to machine:"<<libFileName;
                m_machineService->SaveSettings(libFileName);
                if (m_systemSettings->MachineName != libFileName) { // changed
                    m_machineService->Stop(m_systemSettings->MachineName);
                    m_machineService->Start(libFileName);
                    m_systemSettings->MachineName = libFileName;
                }
                break;
            }
        }
        m_systemSettings->WriteSettings();
        accept();
        break;
    }
    case QDialogButtonBox::Cancel:
        reject();
        break;
    default:
        break;
    }
}

void MachineSettingsDialog::initListWidget()
{
    QDir dir(GlobalVars::PLUGIN_MACHINE_PATH);
    if(dir.exists()) {
        QFileInfoList fileInfoList = dir.entryInfoList(QStringList()<<"*.so", QDir::Files, QDir::Name);
        QListIterator<QFileInfo> it(fileInfoList);
        while (it.hasNext()) {
            QFileInfo fileInfo = it.next();
            QString libFileName = fileInfo.fileName();
            T_PluginInfo pluginInfo;
            if (m_machineService->GetPluginInfo(libFileName, pluginInfo) == ERR_NoError) {
                QListWidgetItem *listItem = new QListWidgetItem(ui->m_lstwdtMachineList);
                listItem->setSizeHint(QSize(60, 25));  // use to change the height
                QRadioButton *radioButton = new QRadioButton(QString::fromLocal8Bit(pluginInfo.FriendlyName), ui->m_lstwdtMachineList);
                connect(radioButton, &QRadioButton::clicked, this, &MachineSettingsDialog::machineRadioChange);

                ui->m_lstwdtMachineList->setItemWidget(listItem, radioButton);
                radioButton->setChecked(m_systemSettings->MachineName == libFileName);
                listItem->setData(Qt::UserRole, libFileName);
                if (m_systemSettings->MachineName == libFileName){
                    qDebug()<<"set using :"<<listItem->data(Qt::UserRole).toString();
                    ui->m_lstwdtMachineList->setCurrentItem(listItem);
                }

                if (pluginInfo.OptionDlgHandle != 0) { // config ui
                    QWidget *widget = (QWidget *)(pluginInfo.OptionDlgHandle);
                    ui->m_stackedWidget->addWidget(widget);
                    if (m_machineService->LoadSettings(libFileName) == ERR_NoError) {
                        m_name2WidgetMap[libFileName] = widget;
                        ui->m_stackedWidget->addWidget(widget);
                        if (m_systemSettings->MachineName == libFileName) {
                            ui->m_stackedWidget->setCurrentWidget(widget);
                            qDebug()<<"using machine:"<<libFileName;
                        }
                    } else {
                        qCritical() << "Load settings fail! machine:"<<libFileName;
                    }
                }
            }
        }
    }
}

/*
 * function: machineRadioChange
 * description: 用于切换不同的机台，实现radio的变更实时显示对应的界面
 * */
void MachineSettingsDialog::machineRadioChange()
{
    for (int i = 0; i < ui->m_lstwdtMachineList->count(); i++) {
        QListWidgetItem *item = ui->m_lstwdtMachineList->item(i);
        if (item == nullptr){
            qCritical()<<"item "<<i<<" is null";
            continue;
        }

        QRadioButton *radioButton = static_cast<QRadioButton *>(ui->m_lstwdtMachineList->itemWidget(item));
        if (radioButton == nullptr){
            qCritical()<<"radioButton "<<i<<" is null";
            continue;
        }
        if (radioButton->isChecked()) {
            QString libFileName = item->data(Qt::UserRole).toString();
            T_PluginInfo pluginInfo;
            qInfo()<<"select "<<libFileName;
            m_machineService->GetPluginInfo(libFileName, pluginInfo);
            QWidget *widget = (QWidget *)(pluginInfo.OptionDlgHandle);
            ui->m_stackedWidget->setCurrentWidget(widget);
        }
    }
}
