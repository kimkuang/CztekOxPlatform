#include "DatabaseSettingWidget.h"
#include "ui_DatabaseSettingWidget.h"

DatabaseSettingWidget::DatabaseSettingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DatabaseSettingWidget)
{
    ui->setupUi(this);
}

DatabaseSettingWidget::~DatabaseSettingWidget()
{
    delete ui;
}
