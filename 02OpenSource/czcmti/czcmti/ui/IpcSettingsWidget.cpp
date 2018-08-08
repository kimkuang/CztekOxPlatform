#include "IpcSettingsWidget.h"
#include "ui_IpcSettingsWidget.h"
#include <QDebug>
#include <QProcess>
#include <QMessageBox>
#include <QFile>

IpcSettingsWidget::IpcSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IpcSettingsWidget)
{
    ui->setupUi(this);
    m_sysSettings = SystemSettings::GetInstance();
}

IpcSettingsWidget::~IpcSettingsWidget()
{
    m_sysSettings = nullptr;
    delete ui;
}

void IpcSettingsWidget::LoadSettings()
{
    m_sysSettings->ReadSettings();
    ui->m_grpIpcEnabled->setChecked(m_sysSettings->Ipc.Enabled);
    ui->m_spinListenPort->setValue(m_sysSettings->Ipc.ListenPort);
    ui->m_rdoDevice0->setChecked(m_sysSettings->Ipc.CurrentDeviceIdx == 0);
    ui->m_rdoDevice1->setChecked(m_sysSettings->Ipc.CurrentDeviceIdx == 1);
    ui->m_edtDev0Ip->setText(m_sysSettings->Ipc.Dev0Ip);
    ui->m_spinDev0ChnCnt->setValue(m_sysSettings->Ipc.Dev0ChnCnt);
    ui->m_edtDev1Ip->setText(m_sysSettings->Ipc.Dev1Ip);
    ui->m_spinDev1ChnCnt->setValue(m_sysSettings->Ipc.Dev1ChnCnt);
}

bool IpcSettingsWidget::SaveSettings()
{
    m_sysSettings->Ipc.Enabled = ui->m_grpIpcEnabled->isChecked();
    m_sysSettings->Ipc.ListenPort = ui->m_spinListenPort->value();
    if (ui->m_rdoDevice0->isChecked())
        m_sysSettings->Ipc.CurrentDeviceIdx = 0;
    else
        m_sysSettings->Ipc.CurrentDeviceIdx = 1;
    m_sysSettings->Ipc.Dev0Ip = ui->m_edtDev0Ip->text();
    m_sysSettings->Ipc.Dev0ChnCnt = ui->m_spinDev0ChnCnt->value();
    m_sysSettings->Ipc.Dev1Ip = ui->m_edtDev1Ip->text();
    m_sysSettings->Ipc.Dev1ChnCnt = ui->m_spinDev1ChnCnt->value();
    m_sysSettings->WriteSettings();
    return true;
}


