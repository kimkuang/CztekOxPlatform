#include "ConfWidget.h"
#include "ui_ConfWidget.h"
#include <QMessageBox>
#include <QDebug>
#include "IImageSensor.h"

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
    ui->spinBox_DelayTime->setValue(m_option->m_DelayTime);
    ui->checkBox_Enable->setChecked(m_option->m_Enable);
    return ERR_NoError;
}

int ConfWidget::Ui2Cache()
{
    m_option->m_DelayTime = ui->spinBox_DelayTime->value();
    m_option->m_Enable = ui->checkBox_Enable->isChecked();
    return ERR_NoError;
}


