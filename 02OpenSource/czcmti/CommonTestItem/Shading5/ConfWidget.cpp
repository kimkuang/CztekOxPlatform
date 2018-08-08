#include "ConfWidget.h"
#include "ui_ConfWidget.h"
#include <QMessageBox>
#include <QDebug>

ConfWidget::ConfWidget(Option *option, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfWidget),
    m_option(option)
{
    ui->setupUi(this);
}

ConfWidget::~ConfWidget()
{
    delete ui;
    m_option = nullptr;
}

int ConfWidget::RestoreDefaults()
{
    m_option->RestoreDefaults();
    Cache2Ui();
    return ERR_NoError;
}

int ConfWidget::Cache2Ui()
{
    ui->m_edtBLC->setText(QString::number(m_option->m_BLC));
    ui->m_boxContinue->setChecked(m_option->m_Continue);
    ui->m_spinBoxYDiff->setValue(m_option->m_YDiff);
    ui->m_spinBoxYMax->setValue(m_option->m_YMax);
    ui->m_spinBoxYMin->setValue(m_option->m_YMin);
    ui->m_spinBoxBGRatioMin->setValue(m_option->m_BGMin);
    ui->m_spinBoxBGRatioMax->setValue(m_option->m_BGMax);
    ui->m_spinBoxRGRatioMax->setValue(m_option->m_RGMax);
    ui->m_spinBoxRGRatioMin->setValue(m_option->m_RGMin);

    ui->m_spinBoxShading0X->setValue(m_option->m_RoiConfs[0].XPercent);
    ui->m_spinBoxShading0Y->setValue(m_option->m_RoiConfs[0].YPercent);
    ui->m_spinBoxShading1X->setValue(m_option->m_RoiConfs[1].XPercent);
    ui->m_spinBoxShading1Y->setValue(m_option->m_RoiConfs[1].YPercent);
    ui->m_spinBoxShading2X->setValue(m_option->m_RoiConfs[2].XPercent);
    ui->m_spinBoxShading2Y->setValue(m_option->m_RoiConfs[2].YPercent);
    ui->m_spinBoxShading3X->setValue(m_option->m_RoiConfs[3].XPercent);
    ui->m_spinBoxShading3Y->setValue(m_option->m_RoiConfs[3].YPercent);
    ui->m_spinBoxShading4X->setValue(m_option->m_RoiConfs[4].XPercent);
    ui->m_spinBoxShading4Y->setValue(m_option->m_RoiConfs[4].YPercent);
    ui->m_edtROIWidth->setText(QString::number(m_option->m_RoiWidthPercent));
    ui->m_edtROIHeight->setText(QString::number(m_option->m_RoiHeightPercent));

    return ERR_NoError;
}

int ConfWidget::Ui2Cache()
{
    QString value;
    value = ui->m_edtBLC->text();
    bool ok = false;
    if ((value.toInt(&ok) < 0) || !ok) {
        QMessageBox::critical(this, tr("Error"), tr("BLC 设置错误.").arg(value));
        return ERR_Failed;
    }
    m_option->m_BLC = value.toInt();
    m_option->m_Continue = ui->m_boxContinue->isChecked();
    m_option->m_YDiff = ui->m_spinBoxYDiff->value();

    m_option->m_YMax = ui->m_spinBoxYMax->value();
    m_option->m_YMin = ui->m_spinBoxYMin->value();
    m_option->m_BGMin = ui->m_spinBoxBGRatioMin->value();
    m_option->m_BGMax = ui->m_spinBoxBGRatioMax->value();
    m_option->m_RGMax = ui->m_spinBoxRGRatioMax->value();
    m_option->m_RGMin = ui->m_spinBoxRGRatioMin->value();

    m_option->m_RoiConfs[0].XPercent = ui->m_spinBoxShading0X->value();
    m_option->m_RoiConfs[0].YPercent = ui->m_spinBoxShading0Y->value();
    m_option->m_RoiConfs[1].XPercent = ui->m_spinBoxShading1X->value();
    m_option->m_RoiConfs[1].YPercent = ui->m_spinBoxShading1Y->value();
    m_option->m_RoiConfs[2].XPercent = ui->m_spinBoxShading2X->value();
    m_option->m_RoiConfs[2].YPercent = ui->m_spinBoxShading2Y->value();
    m_option->m_RoiConfs[3].XPercent = ui->m_spinBoxShading3X->value();
    m_option->m_RoiConfs[3].YPercent = ui->m_spinBoxShading3Y->value();
    m_option->m_RoiConfs[4].XPercent = ui->m_spinBoxShading4X->value();
    m_option->m_RoiConfs[4].YPercent = ui->m_spinBoxShading4Y->value();

    value = ui->m_edtROIWidth->text();
    if ((value.toInt(&ok) < 0) || !ok) {
        QMessageBox::critical(this, tr("Error"), tr("ROI长 设置错误.").arg(value));
        return ERR_Failed;
    }
    m_option->m_RoiWidthPercent = value.toInt();
    value = ui->m_edtROIHeight->text();
    if ((value.toInt(&ok) < 0) || !ok) {
        QMessageBox::critical(this, tr("Error"), tr("ROI宽 设置错误.").arg(value));
        return ERR_Failed;
    }
    m_option->m_RoiHeightPercent = value.toInt();

    return ERR_NoError;
}


