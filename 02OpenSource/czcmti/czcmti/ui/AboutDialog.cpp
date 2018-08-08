#include "AboutDialog.h"
#include "ui_AboutDialog.h"
#include <QDateTime>
#include <QDebug>
#include <QTimeZone>
#include <QLocale>
#include "common/Global.h"
#include "dal/versiondal.h"
#include "controller/ChannelController.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    setWindowTitle(tr("About..."));
    initAboutInformation();
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::GetVersion(QString &version, bool noBuildVer)
{
    QString strTime = QString("%1 %2").arg(__DATE__).arg(__TIME__).simplified(); // Here, LocalTime = UTC Time
    QLocale locale(QLocale::English, QLocale::UnitedStates);
    QDateTime buildTime = locale.toDateTime(strTime, "MMM d yyyy HH:mm:ss");
    buildTime = buildTime.toOffsetFromUtc(8 * 3600); // convert to +8 zone
    QString revision = buildTime.toString("Mdd");
    QString buildNumber = buildTime.toString("Hmmss");//yyMMddHHmm
    QString suffixVer = "";
#ifdef TRIAL_VERSION
    suffixVer = "(trial)";
#endif
#ifdef RDDBG
    suffixVer = "(rddbg)";
#endif
    if (noBuildVer)
        version = QString("%1.%2.%3%4").arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(revision).arg(suffixVer);
    else
        version = QString("%1.%2.%3.%4%5").arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(revision).arg(buildNumber).arg(suffixVer);
}

void AboutDialog::GetDbVersion(QString &version)
{
    VersionDAL verDal;
    Version ver;
    ver.Id = 1;
    if (verDal.SelectById(ver))
        version = ver.DbDate;
    else
        version = "(Unkown)";
}

void AboutDialog::initAboutInformation()
{
    ui->lblProductName->setText(tr("Camera Module Test Instrument"));
    QString version;
    GetVersion(version);
    QString dbVersion;
    GetDbVersion(dbVersion);
    ui->lblVersion->setText(QString("%1(%2)").arg(version).arg(dbVersion));
    version = ChannelController::GetInstance(0)->GetDeviceVersion();
    ui->m_lblDeviceVersion->setText(version);
    version = ChannelController::GetInstance(0)->GetHalVersion();
    ui->m_lblHalVersion->setText(QString("HAL: %1").arg(version));

    QStringList features;
    if (GlobalVars::ThisProductFeatures.testFlag(ProductBarcode::PF_StandbyCurrent))
        features.append(tr("Standby Current"));
    if (GlobalVars::ThisProductFeatures.testFlag(ProductBarcode::PF_WorkingCurrent))
        features.append(tr("Working Current"));
    if (GlobalVars::ThisProductFeatures.testFlag(ProductBarcode::PF_OpenShort))
        features.append(tr("O/S"));
    QString barcode = ChannelController::GetInstance(0)->GetBarcode();
    if (features.count() < 1)
        ui->m_lblBarcode->setText(tr("Barcode: %1").arg(barcode));
    else
        ui->m_lblBarcode->setText(tr("Barcode: %1(%2)").arg(barcode).arg(features.join(QChar(','))));
    ui->lblCompany->setText(tr("(c) 2013-%1 CZTEK Co., Ltd.").arg(QDate::currentDate().year()));
    ui->lblCopyright->setText(tr("All rights reserved."));
    QString rootfsVer;
    bool flag = ProductBarcode::GetRootfsVersion(rootfsVer);
    ui->m_lblRootfsVer->setVisible(flag);
    if (flag)
        ui->m_lblRootfsVer->setText(tr("Rootfs Version: %1").arg(rootfsVer));
}
