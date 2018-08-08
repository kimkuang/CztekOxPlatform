#include "ConfWidget.h"
#include "ui_ConfWidget.h"
#include <QMessageBox>
#include <QDebug>
#include <QPushButton>

ConfWidget::ConfWidget(Option *option, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfWidget),
    m_option(option)
{
    ui->setupUi(this);
    m_btnGroup1.addButton(ui->checkBox2PD);
    m_btnGroup1.addButton(ui->checkBox4Cell);
    m_btnGroup1.addButton(ui->checkBoxNormal);
    m_btnGroup2.addButton(ui->checkBoxAvgPixel);
    m_btnGroup2.addButton(ui->checkBox128Pixel);
    m_btnGroup2.addButton(ui->checkBoxChlPixel);
    ui->comboBoxAEMode->addItem("R_Target");
    ui->comboBoxAEMode->addItem("Gr_Target");
    ui->comboBoxAEMode->addItem("Gb_Target");
    ui->comboBoxAEMode->addItem("B_Target");
    ui->comboBoxAEMode->addItem("Y_Target");
    ui->comboBoxAEMode->addItem("GG_Target");

    for (int ii=1; ii<=16; ii*=2)
    {
        ui->comboBoxAnalogGain->addItem(QString("%1x").arg(ii));
    }

    ui->checkBoxRememberAE->setChecked(false);
    ui->lineEditAEInfinitiVal->setEnabled(true);

    connect(ui->checkBox128Pixel, &QPushButton::clicked, this, &ConfWidget::CheckBox128PixelClick);
    connect(ui->checkBoxAvgPixel, &QPushButton::clicked, this, &ConfWidget::CheckBoxAvgPixelClick);
    connect(ui->checkBoxChlPixel, &QPushButton::clicked, this, &ConfWidget::CheckBoxChlPixelClick);
    connect(ui->checkBoxRememberAE, &QPushButton::clicked, this, &ConfWidget::CheckBoxRememberAEClick);
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
    ui->checkBox10Bit->setChecked(m_option->m_b10BitCal);
    if (Option::TWOPD == m_option->m_iSensorType)
    {
        ui->checkBox2PD->setChecked(true);
    }
    else if (Option::NORMAL == m_option->m_iSensorType)
    {
        ui->checkBoxNormal->setChecked(true);
    }
    else
    {
        ui->checkBox4Cell->setChecked(true);
    }

    if (Option::AVGPIXEL == m_option->m_iRefMode)
    {
        ui->checkBoxAvgPixel->setChecked(true);
    }
    else if (Option::ONE28PIXEL == m_option->m_iRefMode)
    {
        ui->checkBox128Pixel->setChecked(true);
    }
    else
    {
        ui->checkBoxChlPixel->setChecked(true);
    }

    ui->checkBoxRememberAE->setChecked(m_option->m_bRememberAE);
    ui->comboBoxAEMode->setCurrentIndex(m_option->m_iAECalMode);
    ui->comboBoxAnalogGain->setCurrentIndex(m_option->m_iAnalogGain);
    QString strVal = "";
    strVal.sprintf("%d", m_option->m_iAEMinVal);
    ui->lineEditAEMinVal->setText(strVal);
    strVal.sprintf("%d", m_option->m_iAEMaxVal);
    ui->lineEditAEMaxVal->setText(strVal);
    strVal.sprintf("%d", m_option->m_iRoiH);
    ui->lineEditROIRatioH->setText(strVal);
    strVal.sprintf("%d", m_option->m_iRoiW);
    ui->lineEditROIRatioW->setText(strVal);
    strVal.sprintf("%d", m_option->m_iStartX);
    ui->lineEditROIStartX->setText(strVal);
    strVal.sprintf("%d", m_option->m_iStartY);
    ui->lineEditROIStartY->setText(strVal);
    strVal.sprintf("%d", m_option->m_iBlc);
    ui->lineEditBLC->setText(strVal);
    strVal.sprintf("%d", m_option->m_iAETimes);
    ui->lineEditAETimes->setText(strVal);
    strVal.sprintf("%d", m_option->m_iAEInfiVal);
    ui->lineEditAEInfinitiVal->setText(strVal);
    strVal.sprintf("%d", m_option->m_iWaitTime);
    ui->lineEditErrCode->setText(strVal);
    strVal.sprintf("%d", m_option->m_iAEFrame);
    ui->lineEditFrame->setText(strVal);

    return ERR_NoError;
}

int ConfWidget::Ui2Cache()
{
    m_option->m_b10BitCal = ui->checkBox10Bit->isChecked();
    if (ui->checkBox2PD->isChecked())
    {
        m_option->m_iSensorType = Option::TWOPD;
    }
    else if (ui->checkBox4Cell->isChecked())
    {
        m_option->m_iSensorType = Option::FOURCELL;
    }
    else if (ui->checkBoxNormal->isChecked())
    {
        m_option->m_iSensorType = Option::NORMAL;
    }

    if (ui->checkBoxAvgPixel->isChecked())
    {
        m_option->m_iRefMode = Option::AVGPIXEL;
    }
    else if (ui->checkBox128Pixel->isChecked())
    {
        m_option->m_iRefMode = Option::ONE28PIXEL;
    }
    else if (ui->checkBoxChlPixel->isChecked())
    {
        m_option->m_iRefMode = Option::CHLPIXEL;
    }

    m_option->m_bRememberAE = ui->checkBoxRememberAE->isChecked();
    m_option->m_iAECalMode = ui->comboBoxAEMode->currentIndex();
    m_option->m_iAnalogGain = ui->comboBoxAnalogGain->currentIndex();
    m_option->m_iAEMinVal = ui->lineEditAEMinVal->text().toInt();
    m_option->m_iAEMaxVal = ui->lineEditAEMaxVal->text().toInt();
    m_option->m_iRoiH = ui->lineEditROIRatioH->text().toInt();
    m_option->m_iRoiW = ui->lineEditROIRatioW->text().toInt();
    m_option->m_iStartX = ui->lineEditROIStartX->text().toInt();
    m_option->m_iStartY = ui->lineEditROIStartY->text().toInt();
    m_option->m_iBlc = ui->lineEditBLC->text().toInt();
    m_option->m_iAETimes = ui->lineEditAETimes->text().toInt();
    m_option->m_iAEInfiVal = ui->lineEditAEInfinitiVal->text().toInt(Q_NULLPTR, 0);
    m_option->m_iAEFrame = ui->lineEditFrame->text().toInt();
    m_option->m_iWaitTime = ui->lineEditErrCode->text().toInt();

    return ERR_NoError;
}

void ConfWidget::CheckBox128PixelClick(void)
{
    if (ui->checkBox128Pixel->isChecked())
    {
        ui->comboBoxAEMode->setEnabled(false);
    }
}

void ConfWidget::CheckBoxAvgPixelClick(void)
{
    if (ui->checkBoxAvgPixel->isChecked())
    {
        ui->comboBoxAEMode->setEnabled(true);
    }
}

void ConfWidget::CheckBoxChlPixelClick(void)
{
    if (ui->checkBoxChlPixel->isChecked())
    {
        ui->comboBoxAEMode->setEnabled(true);
    }
}

void ConfWidget::CheckBoxRememberAEClick(void)
{
    if (ui->checkBoxRememberAE->isChecked())
    {
        ui->lineEditAEInfinitiVal->setDisabled(true);
    }
    else
    {
        ui->lineEditAEInfinitiVal->setEnabled(true);
    }
}
