#include "ConfWidget.h"
#include "ui_ConfWidget.h"
#include "czcmtidefs.h"
#include <QDebug>

ConfWidget::ConfWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfWidget)
{
    ui->setupUi(this);

    auto initComboBox = [](QComboBox *cbx) {
        cbx->clear();
        cbx->addItem("nA", (int)CurrentRange_nA);
        cbx->addItem("uA", (int)CurrentRange_uA);
//        cbx->addItem("mA", (int)CurrentRange_mA);
    };
    initComboBox(ui->m_cbxRange_DVDD);
    initComboBox(ui->m_cbxRange_AVDD);
    initComboBox(ui->m_cbxRange_DOVDD);
    initComboBox(ui->m_cbxRange_AFVCC);
    initComboBox(ui->m_cbxRange_VPP);
    initComboBox(ui->m_cbxRange_Total);

    m_option = Option::GetInstance();
    Cache2Ui();
}

ConfWidget::~ConfWidget()
{
    m_option = nullptr;
    delete ui;
}

int ConfWidget::RestoreDefaults()
{
    m_option->RestoreDefaults();
    Cache2Ui();
    return ERR_NoError;
}

int ConfWidget::Cache2Ui()
{
    ui->m_chkbDVDDEnabled->setChecked(m_option->DvddEnabled);
    ui->m_chkbAVDDEnabled->setChecked(m_option->AvddEnabled);
    ui->m_chkbDOVDDEnabled->setChecked(m_option->DovddEnabled);
    ui->m_chkbAFVCCEnabled->setChecked(m_option->AfvccEnabled);
    ui->m_chkbVPPEnabled->setChecked(m_option->VppEnabled);
    ui->m_chkbTotalEnabled->setChecked(m_option->TotalEnabled);

    int index = 0;
    index = ui->m_cbxRange_DVDD->findData(m_option->DvddRange);
    ui->m_cbxRange_DVDD->setCurrentIndex(index);
    qDebug() << index;
    ui->m_cbxRange_AVDD->setCurrentIndex(ui->m_cbxRange_AVDD->findData(m_option->AvddRange));
    ui->m_cbxRange_DOVDD->setCurrentIndex(ui->m_cbxRange_DOVDD->findData(m_option->DovddRange));
    ui->m_cbxRange_AFVCC->setCurrentIndex(ui->m_cbxRange_AFVCC->findData(m_option->AfvccRange));
    ui->m_cbxRange_VPP->setCurrentIndex(ui->m_cbxRange_VPP->findData(m_option->VppRange));
    ui->m_cbxRange_Total->setCurrentIndex(ui->m_cbxRange_Total->findData(m_option->TotalRange));

    ui->m_spinUpperLimit_DVDD->setValue(m_option->DvddUpperLimit);
    ui->m_spinUpperLimit_AVDD->setValue(m_option->AvddUpperLimit);
    ui->m_spinUpperLimit_DOVDD->setValue(m_option->DovddUpperLimit);
    ui->m_spinUpperLimit_AFVCC->setValue(m_option->AfvccUpperLimit);
    ui->m_spinUpperLimit_VPP->setValue(m_option->VppUpperLimit);
    ui->m_spinUpperLimit_Total->setValue(m_option->TotalUpperLimit);

    ui->m_chkbContinueWhenFailed->setChecked(m_option->ContinueWhenFailed);

    return ERR_NoError;
}

int ConfWidget::Ui2Cache()
{
    m_option->DvddEnabled = ui->m_chkbDVDDEnabled->isChecked();
    m_option->AvddEnabled = ui->m_chkbAVDDEnabled->isChecked();
    m_option->DovddEnabled = ui->m_chkbDOVDDEnabled->isChecked();
    m_option->AfvccEnabled = ui->m_chkbAFVCCEnabled->isChecked();
    m_option->VppEnabled = ui->m_chkbVPPEnabled->isChecked();
    m_option->TotalEnabled = ui->m_chkbTotalEnabled->isChecked();

    m_option->DvddRange = ui->m_cbxRange_DVDD->currentData().toInt();
    m_option->AvddRange = ui->m_cbxRange_AVDD->currentData().toInt();
    m_option->DovddRange = ui->m_cbxRange_DOVDD->currentData().toInt();
    m_option->AfvccRange = ui->m_cbxRange_AFVCC->currentData().toInt();
    m_option->VppRange = ui->m_cbxRange_VPP->currentData().toInt();
    m_option->TotalRange = ui->m_cbxRange_Total->currentData().toInt();

    m_option->DvddUpperLimit = ui->m_spinUpperLimit_DVDD->value();
    m_option->AvddUpperLimit = ui->m_spinUpperLimit_AVDD->value();
    m_option->DovddUpperLimit = ui->m_spinUpperLimit_DOVDD->value();
    m_option->AfvccUpperLimit = ui->m_spinUpperLimit_AFVCC->value();
    m_option->VppUpperLimit = ui->m_spinUpperLimit_VPP->value();
    m_option->TotalUpperLimit = ui->m_spinUpperLimit_Total->value();

    m_option->ContinueWhenFailed = ui->m_chkbContinueWhenFailed->isChecked();

    return ERR_NoError;
}
