#include "UpgradeDialog.h"
#include "ui_UpgradeDialog.h"
#include "ShareDirWidget.h"
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include "CzUtilsDefs.h"
#include "UiUtils.h"

UpgradeDialog::UpgradeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpgradeDialog)
{
    ui->setupUi(this);
    m_sysSettings = SystemSettings::GetInstance();
    m_upgraderConfFile = "upgrade.ini";
    initUi();

    connect(ui->m_btnCheckVersion, &QPushButton::clicked, this, &UpgradeDialog::btnCheckVersion_clicked);
    connect(ui->m_btnUpgrade, &QPushButton::clicked, this, &UpgradeDialog::btnUpgrade_clicked);
    connect(ui->m_btnRevert, &QPushButton::clicked, this, &UpgradeDialog::btnRevert_clicked);
    connect(ui->m_btnClose, &QPushButton::clicked, this, &UpgradeDialog::btnClose_clicked);
}

UpgradeDialog::~UpgradeDialog()
{
    delete ui;
    m_sysSettings = nullptr;
}

void UpgradeDialog::btnCheckVersion_clicked()
{
    QString srcIniFileName = m_sysSettings->ShareDir.MountPath + "/" + m_upgraderConfFile;
    QString destIniFileName = GlobalVars::UPGRADE_PATH + m_upgraderConfFile;
    QFile srcFile(srcIniFileName);
    if (srcFile.exists() && !srcFile.copy(destIniFileName)) {
        ui->m_lblVersionHint->setText(tr("Copying %1 file to local failed.").arg(m_upgraderConfFile));
        return;
    }

    QFile destFile(destIniFileName);
    if (!destFile.exists()) {
        ui->m_lblVersionHint->setText(tr("Upgrade config file [%1] has not found!").arg(m_upgraderConfFile));
        return;
    }

    IniFile upgradeIniFile;
    upgradeIniFile.LoadFile(destIniFileName.toStdString());
//    m_upgradeParam.PacketType = (T_UpgradeParam::E_PacketType)upgradeIniFile.ReadInteger("common", "packetType", 1);
//    m_upgradeParam.PartialUpdate = upgradeIniFile.ReadBool("common", "partialUpdate", false);

    // fileName=version,check version flag,relative path
    std::vector<IniFile::T_LineConf> sectionLines;
    upgradeIniFile.ReadSection("files", sectionLines);
    QString fileName, strTemp, version, relativePath, md5sum;
    bool checkVersionFlag = false;
    int row = 0;
    for (auto it = sectionLines.begin(); it != sectionLines.end(); ++it) {
        fileName = QString::fromStdString(it->Key).trimmed();
        strTemp = QString::fromStdString(it->Value).trimmed();
        if (fileName.isEmpty() || strTemp.isEmpty())
            continue;
        QStringList slTemp = strTemp.split(QChar(','), QString::KeepEmptyParts);
        qDebug()<<fileName<<strTemp<<slTemp;
        if (slTemp.length() != 4) {
            qCritical().noquote()<<tr("Error config %1=%2").arg(fileName).arg(strTemp);
            continue;
        }
        version = slTemp[0].trimmed();
        checkVersionFlag = (slTemp[1].trimmed().compare("true", Qt::CaseInsensitive) == 0);
        relativePath = slTemp[2].trimmed();
        md5sum = slTemp[3].trimmed();

        ui->m_tabwdtVersionDetail->setRowCount(row + 1);
        ui->m_tabwdtVersionDetail->setItem(row, 0, new QTableWidgetItem(fileName));
        ui->m_tabwdtVersionDetail->setItem(row, 1, new QTableWidgetItem(version));
        ui->m_tabwdtVersionDetail->setItem(row, 2, new QTableWidgetItem(""));
        row++;

        if (checkVersionFlag) {
            // TODO
        }
    }
    ui->m_tabwdtVersionDetail->resizeColumnsToContents();

    ui->m_btnUpgrade->setEnabled(row > 0);
}

void UpgradeDialog::btnUpgrade_clicked()
{
    bool bYes = QMessageBox::question(this, tr("Question"), tr("Are you sure to upgrade?"),
                                      QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes;
    if (bYes) {
        qDebug().noquote() << tr("Starting upgrader...");
//        QProcess::startDetached("")
    }
}

void UpgradeDialog::btnRevert_clicked()
{
    bool bYes = QMessageBox::question(this, tr("Question"), tr("Are you sure to revert?"),
                                      QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
    if (bYes) {
        // Revert
    }
}

void UpgradeDialog::btnClose_clicked()
{
    m_sysSettings->WriteSettings();
    close();
}

void UpgradeDialog::initUi()
{
    ui->m_btnCheckVersion->setEnabled(false);
    ui->m_btnUpgrade->setEnabled(false);

    ui->m_tabwdtVersionDetail->setHorizontalHeaderLabels(QStringList()<<tr("File Name")<<tr("Upgrade Version")<<tr("Local Version"));
    QDir dir(m_sysSettings->ShareDir.MountPath);
    if (!dir.exists()) {
        ui->m_lblVersionHint->setText(tr("Share directory[%1] has not configured!")
                                      .arg(m_sysSettings->ShareDir.MountPath));
        return;
    }
    ui->m_btnCheckVersion->setEnabled(true);

//    QMapIterator<QString, QString> itTestItem(m_moduleSettings->ImageTestItemMap);
//    int row = 0;
//    while (itTestItem.hasNext()) {
//        itTestItem.next();
//        QStringList slSubItem = itTestItem.value().split(QChar(','), QString::KeepEmptyParts);
//        if (slSubItem.size() < 1)
//            continue;
//        int errorCode = 1;
//        if (slSubItem.size() > 1) {
//            bool ok = false;
//            errorCode = slSubItem[1].toInt(&ok, 10);
//        }
//        QStringList slTemp = slSubItem[0].split(QChar('@'), QString::KeepEmptyParts);
//        if (slTemp.size() == 2) {
//            ui->m_tabwdtDestination->setRowCount(row + 1);
//            ui->m_tabwdtDestination->setItem(row, 0, new QTableWidgetItem(slTemp[0])); // Instance Name
//            ui->m_tabwdtDestination->setItem(row, 1, new QTableWidgetItem(slTemp[1])); // File Name
//            ui->m_tabwdtDestination->setItem(row, 2, new QTableWidgetItem(QString::asprintf("%d", errorCode)));

//            QString fullInstanceName = slTemp[0] + "@" + slTemp[1];
//            T_PluginInfo pluginInfo;
//            if (m_testItemService->GetPluginInfo(fullInstanceName, pluginInfo) == ERR_NoError) {
//                ui->m_tabwdtDestination->item(row, 0)->setBackground(QBrush(Qt::white));
//                ui->m_tabwdtDestination->item(row, 1)->setBackground(QBrush(Qt::white));
//            }
//            else { // high light
//                ui->m_tabwdtDestination->item(row, 0)->setBackground(QBrush(Qt::red));
//                ui->m_tabwdtDestination->item(row, 1)->setBackground(QBrush(Qt::red));
//            }
//            m_testItemService->FreePluginInstance(fullInstanceName);
//            row++;
//        }
//    }

//    ui->m_tabwdtDestination->selectRow(ui->m_tabwdtDestination->rowCount()-1);
//    ui->m_tabwdtDestination->resizeColumnsToContents();
}
