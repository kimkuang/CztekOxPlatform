#include "GlobalSettingsDialog.h"
#include "ui_GlobalSettingsDialog.h"
#include <QGridLayout>
#include "GeneralSettingsWidget.h"
#include "MiddlewareSettingsWidget.h"
#include "IpcSettingsWidget.h"
#include "ShareDirWidget.h"

GlobalSettingsDialog::GlobalSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GlobalSettingsDialog)
{
    ui->setupUi(this);
    QGridLayout *pageGeneralLayout = new QGridLayout(ui->m_pageGeneral);
    GeneralSettingsWidget *wdtGeneralSettigns = new GeneralSettingsWidget(ui->m_pageGeneral);
    pageGeneralLayout->setMargin(0);
    pageGeneralLayout->addWidget(wdtGeneralSettigns);
    m_settingsWidgetList.push_back(wdtGeneralSettigns);

    QGridLayout *pageMiddlewareLayout = new QGridLayout(ui->m_pageMiddleware);
    MiddlewareSettingsWidget *wdtMiddlewareSettings = new MiddlewareSettingsWidget(ui->m_pageMiddleware);
    pageMiddlewareLayout->setMargin(0);
    pageMiddlewareLayout->addWidget(wdtMiddlewareSettings);
    m_settingsWidgetList.push_back(wdtMiddlewareSettings);

    QGridLayout *pageIpcLayout = new QGridLayout(ui->m_pageIpc);
    IpcSettingsWidget *wdtIpcSettings = new IpcSettingsWidget(ui->m_pageIpc);
    pageIpcLayout->setMargin(0);
    pageIpcLayout->addWidget(wdtIpcSettings);
    m_settingsWidgetList.push_back(wdtIpcSettings);

    QGridLayout *pageShareDirLayout = new QGridLayout(ui->m_pageShareDir);
    ShareDirWidget *shareDirWidget = new ShareDirWidget(ui->m_pageShareDir);
    pageShareDirLayout->setMargin(0);
    pageShareDirLayout->addWidget(shareDirWidget);
    m_settingsWidgetList.push_back(shareDirWidget);

#if 0
    QGridLayout *pageDisplayLayout = new QGridLayout(ui->m_pageDisplay);
    DisplaySettingsWidget *wdtDisplaySettings = new DisplaySettingsWidget(ui->m_pageDisplay);
    pageDisplayLayout->setMargin(0);
    pageDisplayLayout->addWidget(wdtDisplaySettings);
    m_settingsWidgetList.push_back(wdtDisplaySettings);
#endif
    foreach (ISettingsWidget *w, m_settingsWidgetList) {
        w->LoadSettings();
    }
    ui->m_stackedWidget->setCurrentIndex(0);

    connectSignalSlots();
}

GlobalSettingsDialog::~GlobalSettingsDialog()
{
    delete ui;
}

void GlobalSettingsDialog::btnGeneral_clicked()
{
    ui->m_stackedWidget->setCurrentIndex(0);
}

void GlobalSettingsDialog::btnMiddleware_clicked()
{
    ui->m_stackedWidget->setCurrentIndex(1);
}

void GlobalSettingsDialog::btnIpc_clicked()
{
    ui->m_stackedWidget->setCurrentIndex(2);
}

void GlobalSettingsDialog::btnShareDir_clicked()
{
    ui->m_stackedWidget->setCurrentIndex(3);
}

void GlobalSettingsDialog::btnOK_clicked()
{
    foreach (ISettingsWidget *w, m_settingsWidgetList) {
        if (!w->SaveSettings()) {
            return;
        }
    }
    accept();
}

void GlobalSettingsDialog::btnCancel_clicked()
{
    reject();
}

void GlobalSettingsDialog::connectSignalSlots()
{
    ui->m_btnGeneral->setAutoFillBackground(true);
    ui->m_btnMiddleware->setAutoFillBackground(true);
    connect(ui->m_btnGeneral, &QPushButton::clicked, this, &GlobalSettingsDialog::btnGeneral_clicked);
    connect(ui->m_btnMiddleware, &QPushButton::clicked, this, &GlobalSettingsDialog::btnMiddleware_clicked);
    connect(ui->m_btnIpc, &QPushButton::clicked, this, &GlobalSettingsDialog::btnIpc_clicked);
    connect(ui->m_btnShareDir, &QPushButton::clicked, this, &GlobalSettingsDialog::btnShareDir_clicked);
    connect(ui->m_btnOK, &QPushButton::clicked, this, &GlobalSettingsDialog::btnOK_clicked);
    connect(ui->m_btnCancel, &QPushButton::clicked, this, &GlobalSettingsDialog::btnCancel_clicked);
}
