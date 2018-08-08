#include "LoginDialog.h"
#include "ui_LoginDialog.h"
#include "dal/sqldatabaseutil.h"
#include <QDir>
#include <QMessageBox>
#include <QDebug>

LoginDialog *LoginDialog::m_instance = nullptr;

LoginDialog *LoginDialog::GetInstance()
{
    if (m_instance == nullptr)
        m_instance = new LoginDialog();
    return m_instance;
}

void LoginDialog::FreeInstance()
{
    if (m_instance != nullptr) {
        delete m_instance;
        m_instance = nullptr;
    }
}

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    m_sysSettings = SystemSettings::GetInstance();
    QString pathName = GlobalVars::PROJECT_PATH + m_sysSettings->ProjectName;
    QDir dir(pathName);
    if (!dir.exists())
        dir.mkdir(pathName);
    initProjectComboBox();
    initWorkModeComboBox();
    initOperatorComboBox();
    connectSignalSlots();
    cache2Ui();
}

LoginDialog::~LoginDialog()
{
    m_sysSettings = nullptr;
    delete ui;
}

void LoginDialog::buttonBox_clicked(QAbstractButton *button)
{
    QDialogButtonBox::StandardButton stdBtn = ui->m_buttonBox->standardButton(button);
    switch (stdBtn) {
    case QDialogButtonBox::Ok: {
        ui2Cache();
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

void LoginDialog::initProjectComboBox()
{
    /*if (UiDefs::Operator_Operator == m_sysSettings->Operator)
    {
        ui->m_cbxProjectName->setDisabled(true);
    }
	else
	{
		ui->m_cbxProjectName->setEnabled(true);
	}*/

    QDir dir(GlobalVars::PROJECT_PATH);
    if(dir.exists()) {
        QFileInfoList fileInfoList = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name);
        QListIterator<QFileInfo> it(fileInfoList);
        ui->m_cbxProjectName->clear();
        while (it.hasNext()) {
            QFileInfo fileInfo = it.next();
//            qDebug()<<fileInfo.baseName()<<fileInfo.absoluteFilePath();
            ui->m_cbxProjectName->addItem(fileInfo.baseName());
        }
    }
}

void LoginDialog::initWorkModeComboBox()
{
    ui->m_cbxWorkMode->addItem(tr("Manual"), (int)UiDefs::WorkMode_Manual);
    ui->m_cbxWorkMode->addItem(tr("Machine"), (int)UiDefs::WorkMode_Machine);
}

void LoginDialog::initOperatorComboBox()
{
    /*if (UiDefs::Operator_Operator == m_sysSettings->Operator)
    {
        ui->m_cbxOperator->setDisabled(true);
    }
	else
	{
		ui->m_cbxOperator->setEnabled(true);
	}*/

    ui->m_cbxOperator->addItem(tr("Operator"), (int)UiDefs::Operator_Operator);
    ui->m_cbxOperator->addItem(tr("Engineer"), (int)UiDefs::Operator_Engineer);
}

void LoginDialog::connectSignalSlots()
{
    connect(ui->m_buttonBox, &QDialogButtonBox::clicked, this, &LoginDialog::buttonBox_clicked);
}

void LoginDialog::cache2Ui()
{
    ui->m_cbxProjectName->setCurrentText(m_sysSettings->ProjectName);
    int index = ui->m_cbxWorkMode->findData(m_sysSettings->WorkMode);
    ui->m_cbxWorkMode->setCurrentIndex(index);
    index = ui->m_cbxOperator->findData(m_sysSettings->Operator);
    ui->m_cbxOperator->setCurrentIndex(index);
    ui->m_chkbxShowLoginDialog->setChecked(m_sysSettings->ShowLoginDialog);
}

void LoginDialog::ui2Cache()
{
    if (ui->m_cbxProjectName->currentText() != m_sysSettings->ProjectName) {
        bool bYes = QMessageBox::question(this, tr("Question"),
            tr("Are you sure to change the project? This will lead to reload new configuration!"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
        if (bYes) {
            QString oldProjectPath = GlobalVars::PROJECT_PATH + m_sysSettings->ProjectName;
            m_sysSettings->ProjectName = ui->m_cbxProjectName->currentText();
            QString newProjectPath = GlobalVars::PROJECT_PATH + m_sysSettings->ProjectName;
            QDir oldPrjDir(oldProjectPath);
            QDir newPrjDir(newProjectPath);
            if (!newPrjDir.exists()) {
                newPrjDir.mkpath(newProjectPath);
                bYes = QMessageBox::question(this, tr("Question"),
                    tr("Copy current project configuration to new project?"),
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes;
                if (bYes) {
                    QStringList fileNames = oldPrjDir.entryList(QStringList() << "*.ini", QDir::Files);
                    foreach (QString fileName, fileNames) {
                        QFile::copy(oldProjectPath + "/" + fileName, newProjectPath + "/" + fileName);
                    }
                }
            }
        }
    }

    m_sysSettings->WorkMode = ui->m_cbxWorkMode->currentData().toInt();
    m_sysSettings->Operator = ui->m_cbxOperator->currentData().toInt();
    m_sysSettings->ShowLoginDialog = ui->m_chkbxShowLoginDialog->isChecked();
    m_sysSettings->WriteSettings();
}
