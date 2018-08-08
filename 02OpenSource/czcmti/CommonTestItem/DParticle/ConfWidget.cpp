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
    ui->m_spinBoxLeftSL->setValue(m_option->m_LeftSL);
    ui->m_spinBoxRightSL->setValue(m_option->m_RightSL);
    ui->m_spinBoxTopSL->setValue(m_option->m_TopSL);
    ui->m_spinBoxBottomSL->setValue(m_option->m_BottomSL);
    ui->m_spinBoxAreaWidth->setValue(m_option->m_AreaWidth);
    ui->m_spinBoxAreaHeight->setValue(m_option->m_AreaHeight);
    ui->m_spinBoxDeadUnit->setValue(m_option->m_DeadUnit);
    ui->m_spinBoxDeadSpec->setValue(m_option->m_DeadSpec);
    ui->m_spinBoxDeadType->setValue(m_option->m_DeadType);
    ui->m_spinBoxExposure->setValue(m_option->m_DefultExposure);
    ui->m_spinBoxGain->setValue(m_option->m_DefultGain);
    ui->m_checkBoxSaveImage->setChecked(m_option->m_SaveImage);

    return ERR_NoError;
}

int ConfWidget::Ui2Cache()
{
    m_option->m_LeftSL = ui->m_spinBoxLeftSL->value();
    m_option->m_RightSL = ui->m_spinBoxRightSL->value();
    m_option->m_TopSL = ui->m_spinBoxTopSL->value();
    m_option->m_BottomSL = ui->m_spinBoxBottomSL->value();
    m_option->m_AreaWidth = ui->m_spinBoxAreaWidth->value();
    m_option->m_AreaHeight = ui->m_spinBoxAreaHeight->value();
    m_option->m_DeadUnit = ui->m_spinBoxDeadUnit->value();
    m_option->m_DeadSpec = ui->m_spinBoxDeadSpec->value();
    m_option->m_DefultExposure = ui->m_spinBoxExposure->value();
    m_option->m_DefultGain = ui->m_spinBoxGain->value();
    m_option->m_DeadType = ui->m_spinBoxDeadType->value();
    m_option->m_SaveImage = ui->m_checkBoxSaveImage->isChecked();

    return ERR_NoError;
}


