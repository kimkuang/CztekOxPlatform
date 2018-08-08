#include "DecoderDialog.h"
#include "ui_DecoderDialog.h"
#include <QDir>
#include <QMessageBox>
#include <QRadioButton>
#include "common/Global.h"
#include "conf/SystemSettings.h"

DecoderDialog::DecoderDialog(uint windowIdx, uint chnIdx, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DecoderDialog)
{
    ui->setupUi(this);
    setWindowTitle(windowTitle() + QString(" - %1").arg(windowIdx));

    m_moduleSettings = ModuleSettings::GetInstance(SystemSettings::GetInstance()->ProjectName, windowIdx);
    m_decoderService = DecoderService::GetInstance(chnIdx);
    initListWidget();
    connect(ui->m_lstwdtDecoderList, &QListWidget::currentItemChanged,
            this, &DecoderDialog::lstwdtDecoderList_currentItemChanged);
    connect(ui->m_buttonBox, &QDialogButtonBox::clicked, this, &DecoderDialog::buttonBox_clicked);
}

DecoderDialog::~DecoderDialog()
{
    for (int i = ui->m_stackedWidget->count() - 1; i >= 0; i--) {
        QWidget *widget = ui->m_stackedWidget->widget(i);
        if (widget) {
            qDebug()<<i<<widget->objectName();
            widget->setParent(nullptr); // plugin manage itself's conf window
        }
    }

    delete ui;
    m_moduleSettings = nullptr;
    m_decoderService = nullptr;
}

void DecoderDialog::lstwdtDecoderList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    (void)previous;
    QString libFileName = current->data(Qt::UserRole).toString();
    if (m_name2WidgetMap.contains(libFileName)) {
        ui->m_stackedWidget->setCurrentWidget(m_name2WidgetMap[libFileName]);
    }
}

void DecoderDialog::buttonBox_clicked(QAbstractButton *button)
{
    QDialogButtonBox::StandardButton stdBtn = ui->m_buttonBox->standardButton(button);
    switch (stdBtn) {
    case QDialogButtonBox::RestoreDefaults: {
        QListWidgetItem *item = ui->m_lstwdtDecoderList->currentItem();
        if (item == nullptr) {
            QMessageBox::critical(this, tr("Error"), tr("Please select the item to operate firstly."));
            break;
        }
        QString libFileName = item->data(Qt::UserRole).toString();
        m_decoderService->RestoreDefaults(libFileName);
        break;
    }
    case QDialogButtonBox::Ok: {
        m_moduleSettings->ImageDecoder = "";
        for (int i = 0; i < ui->m_lstwdtDecoderList->count(); i++) {
            QListWidgetItem *item = ui->m_lstwdtDecoderList->item(i);
            if (item == nullptr)
                continue;
            QRadioButton *radioButton = static_cast<QRadioButton *>(ui->m_lstwdtDecoderList->itemWidget(item));
            if (radioButton == nullptr)
                continue;
            if (radioButton->isChecked()) {
                QString libFileName = item->data(Qt::UserRole).toString();
//                qDebug()<<libFileName;
                m_decoderService->SaveSettings(libFileName);
                m_moduleSettings->ImageDecoder = libFileName;
                break;
            }
        }
        m_moduleSettings->WriteSettings();
        accept();
        break;
    }
    case QDialogButtonBox::Cancel:
        reject();
        break;
    default:
        break;
    }
}

void DecoderDialog::initListWidget()
{
    QString imageDecoderFileName = m_moduleSettings->ImageDecoder;
    QDir dir(GlobalVars::PLUGIN_IMAGEDECODER_PATH);
    if(dir.exists()) {
        QFileInfoList fileInfoList = dir.entryInfoList(QStringList()<<"*.so", QDir::Files, QDir::Name);
        QListIterator<QFileInfo> it(fileInfoList);
        while (it.hasNext()) {
            QFileInfo fileInfo = it.next();
            QString libFileName = fileInfo.fileName();
            T_PluginInfo pluginInfo;
            if (m_decoderService->GetPluginInfo(libFileName, pluginInfo) == ERR_NoError) {
                QListWidgetItem *listItem = new QListWidgetItem(ui->m_lstwdtDecoderList);
                listItem->setSizeHint(QSize(60, 25));  // use to change the height
                QRadioButton *radioButton = new QRadioButton(QString::fromLocal8Bit(pluginInfo.FriendlyName), ui->m_lstwdtDecoderList);
                ui->m_lstwdtDecoderList->setItemWidget(listItem, radioButton);
                radioButton->setChecked(imageDecoderFileName == libFileName);
                listItem->setData(Qt::UserRole, libFileName);
                if (pluginInfo.OptionDlgHandle != 0) { // config ui
                    QWidget *widget = (QWidget *)(pluginInfo.OptionDlgHandle);
                    ui->m_stackedWidget->addWidget(widget);
                    if (m_decoderService->LoadSettings(libFileName) == ERR_NoError)
                        m_name2WidgetMap[libFileName] = widget;
                }
            }
        }
    }
}
