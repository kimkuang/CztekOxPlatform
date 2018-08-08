#include "ShareDirWidget.h"
#include "ui_ShareDirWidget.h"
#include <QFileDialog>

ShareDirWidget::ShareDirWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShareDirWidget)
{
    ui->setupUi(this);
    m_sysSettings = SystemSettings::GetInstance();

    connect(ui->m_btnFind, &QPushButton::clicked, this, &ShareDirWidget::btnFind_clicked);
    connect(ui->m_btnBrowse, &QPushButton::clicked, this, &ShareDirWidget::btnBrowse_clicked);
}

ShareDirWidget::~ShareDirWidget()
{
    delete ui;
    m_sysSettings = nullptr;
}

void ShareDirWidget::LoadSettings()
{
    m_sysSettings->ReadSettings();
    ui->m_edtServerIp->setText(m_sysSettings->ShareDir.ServerIP);
    ui->m_edtUserName->setText(m_sysSettings->ShareDir.UserName);
    ui->m_edtPassword->setText(m_sysSettings->ShareDir.Password);
    ui->m_cbxShareDirList->setCurrentText(m_sysSettings->ShareDir.SourceDir);
    ui->m_edtMountPath->setText(m_sysSettings->ShareDir.MountPath);
}

bool ShareDirWidget::SaveSettings()
{
    m_sysSettings->ShareDir.ServerIP = ui->m_edtServerIp->text();
    m_sysSettings->ShareDir.UserName = ui->m_edtUserName->text();
    m_sysSettings->ShareDir.Password = ui->m_edtPassword->text();
    m_sysSettings->ShareDir.SourceDir = ui->m_cbxShareDirList->currentText();
//    m_sysSettings->ShareDir.oldMountPath = m_sysSettings->Middleware.MountPath;
    m_sysSettings->ShareDir.MountPath = ui->m_edtMountPath->text();
    m_sysSettings->WriteSettings();
    return true;
}

void ShareDirWidget::btnFind_clicked()
{
//    QString strCmd = "";
//    QString serverIP = ui->m_edtShareDirServerIp->text();
//    QString userName = ui->lineEditUserName->text();
//    QString password = ui->lineEditPassword->text();
//    strCmd.sprintf("%s smbclient %s %s %s",
//                            GlobalVars::EXEC_SH_FILE_NAME.toStdString().c_str(),
//                            serverIP.toStdString().c_str(),
//                            userName.toStdString().c_str(),
//                            password.toStdString().c_str());
//    qDebug() << strCmd;
//    if (0 != QProcess::execute(strCmd))
//    {
//        QMessageBox::information(NULL, "Result", "exec process failed.", QMessageBox::Ok);
//        return;
//    }

//    QFile resultFile("result");
//    if (false == resultFile.open(QIODevice::ReadOnly))
//    {
//        QMessageBox::information(NULL, "Result", "No result file", QMessageBox::Ok);
//        return;
//    }

//    int flag = false;
//    ui->comboBoxShareList->clear();
//    while (false == resultFile.atEnd())
//    {
//        QByteArray lineInfo =  resultFile.readLine();
//        if ('\t' == lineInfo.at(0))
//        {
//            QString strLine(lineInfo.replace('\t', ' '));
//            QStringList ltSplit = strLine.split(QChar(' '), QString::SkipEmptyParts);
//            if ((ltSplit[0] != "Sharename") && (-1 == ltSplit[0].indexOf("--")))
//            {
//                ui->comboBoxShareList->addItem(ltSplit[0]);
//            }

//            flag = true;
//        }
//        else
//        {
//            if (true == flag)
//            {
//                break;
//            }
//        }
//    }

//    if (false == flag)
//    {
//        ui->comboBoxShareList->clear();
//        ui->comboBoxShareList->addItem("No Find Share.");
//        ui->comboBoxShareList->setCurrentIndex(0);
//    }

//    resultFile.close();
//    QFile::remove("result");
}

void ShareDirWidget::btnBrowse_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Mount Directory"),
        QDir::homePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->m_edtMountPath->setText(dir);
}
