#include "MiddlewareSettingsWidget.h"
#include "ui_MiddlewareSettingsWidget.h"
#include <QDebug>
#include <QProcess>
#include <QMessageBox>
#include <QFile>

MiddlewareSettingsWidget::MiddlewareSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MiddlewareSettingsWidget)
{
    ui->setupUi(this);
    m_sysSettings = SystemSettings::GetInstance();
}

MiddlewareSettingsWidget::~MiddlewareSettingsWidget()
{
    m_sysSettings = nullptr;
    delete ui;
}

void MiddlewareSettingsWidget::LoadSettings()
{
    m_sysSettings->ReadSettings();
    ui->m_chkbMergeMesData->setChecked(m_sysSettings->Middleware.bMergeMesData);
    ui->m_spinMergeMesChnIdx->setValue(m_sysSettings->Middleware.iMergeMesChnIdx);
    ui->m_edtRpcServerIp->setText(m_sysSettings->Middleware.RpcServerIp);
    ui->m_spinRpcServerPort->setValue(m_sysSettings->Middleware.RpcServerPort);
    ui->lineEditSaveReportPath->setText(m_sysSettings->Middleware.ReportPath);
    ui->checkBoxSaveData->setChecked(m_sysSettings->Middleware.bSaveData);
}

bool MiddlewareSettingsWidget::SaveSettings()
{
    m_sysSettings->Middleware.bMergeMesData = ui->m_chkbMergeMesData->isChecked();
    m_sysSettings->Middleware.iMergeMesChnIdx = ui->m_spinMergeMesChnIdx->value();
    m_sysSettings->Middleware.RpcServerIp = ui->m_edtRpcServerIp->text();
    m_sysSettings->Middleware.RpcServerPort = ui->m_spinRpcServerPort->value();
    m_sysSettings->Middleware.ReportPath = ui->lineEditSaveReportPath->text();
    m_sysSettings->Middleware.bSaveData = ui->checkBoxSaveData->isChecked();
    m_sysSettings->WriteSettings();
    return true;
}


