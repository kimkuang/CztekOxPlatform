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
    QString strValue = "";
    strValue.sprintf("%d", m_option->m_stShading.u32RoiH);
    ui->lineEditRoiH->setText(strValue);
    strValue.sprintf("%d", m_option->m_stShading.u32RoiW);
    ui->lineEditRoiW->setText(strValue);
    strValue.sprintf("%d", m_option->m_stShading.u32Blc);
    ui->lineEditBlc->setText(strValue);

    strValue.sprintf("%d", m_option->m_u32VcmDelay);
    ui->lineEditDelay->setText(strValue);
    strValue.sprintf("%d", m_option->m_u32Y1Min);
    ui->lineEditY1Min->setText(strValue);
    strValue.sprintf("%d", m_option->m_u32Y1Max);
    ui->lineEditY1Max->setText(strValue);
    strValue.sprintf("%d", m_option->m_u32Y2Min);
    ui->lineEditY2Min->setText(strValue);
    strValue.sprintf("%d", m_option->m_u32Y2Max);
    ui->lineEditY2Max->setText(strValue);
    strValue.sprintf("%d", m_option->m_u32Y3Min);
    ui->lineEditY3Min->setText(strValue);
    strValue.sprintf("%d", m_option->m_u32Y3Max);
    ui->lineEditY3Max->setText(strValue);
    strValue.sprintf("%d", m_option->m_u32Y4Min);
    ui->lineEditY4Min->setText(strValue);
    strValue.sprintf("%d", m_option->m_u32Y4Max);
    ui->lineEditY4Max->setText(strValue);

    strValue.sprintf("%d", m_option->m_stShading.Roi[0].cx);
    ui->lineEditRoiCenterX->setText(strValue);
    strValue.sprintf("%d", m_option->m_stShading.Roi[0].cy);
    ui->lineEditRoiCenterY->setText(strValue);

    strValue.sprintf("%d", m_option->m_stShading.Roi[1].cx);
    ui->lineEditRoiLeftTopX->setText(strValue);
    strValue.sprintf("%d", m_option->m_stShading.Roi[1].cy);
    ui->lineEditRoiLeftTopY->setText(strValue);

    strValue.sprintf("%d", m_option->m_stShading.Roi[2].cx);
    ui->lineEditRoiRightTopX->setText(strValue);
    strValue.sprintf("%d", m_option->m_stShading.Roi[2].cy);
    ui->lineEditRoiRightTopY->setText(strValue);

    strValue.sprintf("%d", m_option->m_stShading.Roi[3].cx);
    ui->lineEditRoiLeftBottomX->setText(strValue);
    strValue.sprintf("%d", m_option->m_stShading.Roi[3].cy);
    ui->lineEditRoiLeftBottomY->setText(strValue);

    strValue.sprintf("%d", m_option->m_stShading.Roi[4].cx);
    ui->lineEditRoiRightBottomX->setText(strValue);
    strValue.sprintf("%d", m_option->m_stShading.Roi[4].cy);
    ui->lineEditRoiRightBottomY->setText(strValue);

    return ERR_NoError;
}

int ConfWidget::Ui2Cache()
{
    m_option->m_stShading.u32RoiH = ui->lineEditRoiH->text().toUInt();
    m_option->m_stShading.u32RoiW = ui->lineEditRoiW->text().toUInt();
    m_option->m_stShading.u32Blc = ui->lineEditBlc->text().toUInt();
    m_option->m_u32VcmDelay = ui->lineEditDelay->text().toInt();
    m_option->m_u32Y1Min = ui->lineEditY1Min->text().toInt();
    m_option->m_u32Y1Max = ui->lineEditY1Max->text().toInt();
    m_option->m_u32Y2Min = ui->lineEditY2Min->text().toInt();
    m_option->m_u32Y2Max = ui->lineEditY2Max->text().toInt();
    m_option->m_u32Y3Min = ui->lineEditY3Min->text().toInt();
    m_option->m_u32Y3Max = ui->lineEditY3Max->text().toInt();
    m_option->m_u32Y4Min = ui->lineEditY4Min->text().toInt();
    m_option->m_u32Y4Max = ui->lineEditY4Max->text().toInt();

    m_option->m_stShading.Roi[0].cx = ui->lineEditRoiCenterX->text().toUInt();
    m_option->m_stShading.Roi[0].cy = ui->lineEditRoiCenterY->text().toUInt();
    m_option->m_stShading.Roi[1].cx = ui->lineEditRoiLeftTopX->text().toUInt();
    m_option->m_stShading.Roi[1].cy = ui->lineEditRoiLeftTopY->text().toUInt();
    m_option->m_stShading.Roi[2].cx = ui->lineEditRoiRightTopX->text().toUInt();
    m_option->m_stShading.Roi[2].cy = ui->lineEditRoiRightTopY->text().toUInt();
    m_option->m_stShading.Roi[3].cx = ui->lineEditRoiLeftBottomX->text().toUInt();
    m_option->m_stShading.Roi[3].cy = ui->lineEditRoiLeftBottomY->text().toUInt();
    m_option->m_stShading.Roi[4].cx = ui->lineEditRoiRightBottomX->text().toUInt();
    m_option->m_stShading.Roi[3].cy = ui->lineEditRoiRightBottomY->text().toUInt();

    return ERR_NoError;
}
