#include "HardwareTestDialog.h"
#include "ui_HardwareTestDialog.h"
#include <QDir>
#include <QMessageBox>
#include "common/Global.h"
#include "conf/SystemSettings.h"

HardwareTestDialog::HardwareTestDialog(uint windowIdx, uint chnIdx, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HardwareTestDialog)
{
    ui->setupUi(this);
    setWindowTitle(windowTitle() + QString(" - %1").arg(windowIdx));

    m_moduleSettings = ModuleSettings::GetInstance(SystemSettings::GetInstance()->ProjectName, windowIdx);
    m_testItemService = TestItemService::GetInstance(chnIdx);
    initListWidget();
    connect(ui->m_lstwdtHardwareTestItems, &QListWidget::currentItemChanged,
            this, &HardwareTestDialog::lstwdtHardwareTestItems_currentItemChanged);
    connect(ui->m_buttonBox, &QDialogButtonBox::clicked, this, &HardwareTestDialog::buttonBox_clicked);
}

HardwareTestDialog::~HardwareTestDialog()
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
    m_testItemService = nullptr;
}

void HardwareTestDialog::lstwdtHardwareTestItems_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    (void)previous;
    QStringList slTemp = current->data(Qt::UserRole).toString().split(QChar('@'), QString::KeepEmptyParts);
    if (slTemp.size() == 2) {
        QString fakeFullInstanceName = "@" + slTemp[1];
        if (m_name2WidgetMap.contains(fakeFullInstanceName)) {
            ui->m_stackedWidget->setCurrentWidget(m_name2WidgetMap[fakeFullInstanceName]);
        }
    }
}

void HardwareTestDialog::buttonBox_clicked(QAbstractButton *button)
{
    QDialogButtonBox::StandardButton stdBtn = ui->m_buttonBox->standardButton(button);
    switch (stdBtn) {
    case QDialogButtonBox::RestoreDefaults: {
        QListWidgetItem *item = ui->m_lstwdtHardwareTestItems->currentItem();
        if (item == nullptr) {
            QMessageBox::critical(this, tr("Error"), tr("Please select the item to operate firstly."));
            break;
        }
        QStringList slTemp = item->data(Qt::UserRole).toString().split(QChar('@'), QString::KeepEmptyParts);
        if (slTemp.size() == 2) {
            QString fakeFullInstanceName = "@" + slTemp[1];
            m_testItemService->RestoreDefaults(fakeFullInstanceName);
        }
        break;
    }
    case QDialogButtonBox::Ok: {
        m_moduleSettings->HardwareTestItemMap.clear();
        int row = 0;
        for (int i = 0; i < ui->m_lstwdtHardwareTestItems->count(); i++) {
            QListWidgetItem *item = ui->m_lstwdtHardwareTestItems->item(i);
            if (item == nullptr)
                continue;
            QStringList slTemp = item->data(Qt::UserRole).toString().split(QChar('@'), QString::KeepEmptyParts);
            if ((slTemp.size() == 2) && (item->checkState() == Qt::Checked)) {
                QString fakeFullInstanceName = "@" + slTemp[1];
                m_testItemService->SaveSettings(fakeFullInstanceName);

                QString key = QString::asprintf("testItem%02d", row + 1);
                QString val = item->text() + "@" + slTemp[1];
                m_moduleSettings->HardwareTestItemMap.insert(key, val);
                row++;
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

void HardwareTestDialog::initListWidget()
{
    QList<QString> selectedHardwareItems;
    QMapIterator<QString, QString> itTestItem(m_moduleSettings->HardwareTestItemMap);
    while (itTestItem.hasNext()) {
        itTestItem.next();
        QStringList slTemp = itTestItem.value().split(QChar('@'), QString::KeepEmptyParts);
        if (slTemp.size() == 2)
            selectedHardwareItems.push_back(slTemp[1]);
    }

    QDir dir(GlobalVars::PLUGIN_TESTITEM_PATH);
    if(dir.exists()) {
        QFileInfoList fileInfoList = dir.entryInfoList(QStringList()<<"*.so", QDir::Files, QDir::Name);
        QListIterator<QFileInfo> it(fileInfoList);
        while (it.hasNext()) {
            QFileInfo fileInfo = it.next();
            QString libFileName = fileInfo.fileName();
            QString fakeFullInstanceName = "@" + libFileName;
            T_PluginInfo pluginInfo;
            ITestItem::E_ItemType itemType;
            if ((m_testItemService->GetPluginInfo(fakeFullInstanceName, pluginInfo) == ERR_NoError) &&
                (m_testItemService->GetItemType(fakeFullInstanceName, itemType) == ERR_NoError) &&
                (ITestItem::ItemType_Hardware == itemType))
            {
                QListWidgetItem *listItem = new QListWidgetItem(QString::fromLocal8Bit(pluginInfo.FriendlyName));
                listItem->setSizeHint(QSize(60, 25));  // use to change the height
                listItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
                if (selectedHardwareItems.indexOf(libFileName) != -1)
                    listItem->setCheckState(Qt::Checked);
                else
                    listItem->setCheckState(Qt::Unchecked);
                QString fullInstanceName = QString::fromLocal8Bit(pluginInfo.FriendlyName) + "@" + libFileName;
                listItem->setData(Qt::UserRole, fullInstanceName);
                ui->m_lstwdtHardwareTestItems->addItem(listItem);
                if (pluginInfo.OptionDlgHandle != 0) { // config ui
                    QWidget *widget = (QWidget *)(pluginInfo.OptionDlgHandle);
                    ui->m_stackedWidget->addWidget(widget);
                    if (m_testItemService->LoadSettings(fakeFullInstanceName) == ERR_NoError)
                        m_name2WidgetMap[fakeFullInstanceName] = widget;
                }
            }
        }
    }
}
