#include "ConfWidget.h"
#include "ui_ConfWidget.h"

ConfWidget::ConfWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfWidget)
{
    ui->setupUi(this);
    m_option = Option::GetInstance();
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
    return ERR_NoError;
}

int ConfWidget::Ui2Cache()
{
    return ERR_NoError;
}
