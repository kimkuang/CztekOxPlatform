#include "TestItemManagementDialog.h"
#include "ui_TestItemManagementDialog.h"
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QDebug>
#include "common/Global.h"
#include "conf/SystemSettings.h"

TestItemManagementDialog::TestItemManagementDialog(uint windowIdx, uint chnIdx, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TestItemManagementDialog)
{
    ui->setupUi(this);
    setWindowTitle(windowTitle() + QString(" - %1").arg(windowIdx));

    m_moduleSettings = ModuleSettings::GetInstance(SystemSettings::GetInstance()->ProjectName, windowIdx);
    m_testItemService = TestItemService::GetInstance(chnIdx);
    initTableWidgetSource();
    initTreeWidgetDestination();
    ui->m_splitter->setStretchFactor(0, 1);
    ui->m_splitter->setStretchFactor(1, 1);

    if (UiDefs::Operator_Operator == SystemSettings::GetInstance()->Operator)
    {
        ui->m_btnOK->setDisabled(true);
    }
    else
    {
        ui->m_btnOK->setEnabled(true);
    }

    connect(ui->lineEditFilterTestIItem, &QLineEdit::textChanged, this, &TestItemManagementDialog::lineEditTextChanged);
    connect(ui->m_btnSelect, &QPushButton::clicked, this, &TestItemManagementDialog::btnSelect_clicked);
    connect(ui->m_btnDisselect, &QPushButton::clicked, this, &TestItemManagementDialog::btnDisselect_clicked);
    connect(ui->m_btnOK, &QPushButton::clicked, this, &TestItemManagementDialog::btnOK_clicked);
    connect(ui->m_btnCancel, &QPushButton::clicked, this, &TestItemManagementDialog::btnCancel_clicked);
    connect(ui->pushButtonUp, &QPushButton::clicked, this, &TestItemManagementDialog::btnUp_clicked);
    connect(ui->pushButtonDown, &QPushButton::clicked, this, &TestItemManagementDialog::btnDown_clicked);
    connect(ui->m_btnAddGroup, &QPushButton::clicked, this, &TestItemManagementDialog::btnAddGroup_clicked);
    connect(ui->m_btnDelGroup, &QPushButton::clicked, this, &TestItemManagementDialog::btnDelGroup_clicked);
    connect(ui->pushButtonClear, &QPushButton::clicked, this, &TestItemManagementDialog::btnClear_clicked);
}

TestItemManagementDialog::~TestItemManagementDialog()
{
    delete ui;
    m_moduleSettings = nullptr;
    m_testItemService = nullptr;
}

void TestItemManagementDialog::btnSelect_clicked()
{
    QList<QTableWidgetItem*> items = ui->m_tabwdtSource->selectedItems();
    if (items.size() == ui->m_tabwdtSource->columnCount()) { // select a row
        QString instanceName = items[0]->text();
        QString fileName = items[1]->text();
        QList<QTreeWidgetItem *> lstTreeItem = ui->m_treewdtDestination->findItems(fileName, Qt::MatchExactly | Qt::MatchRecursive, 1);
        if (!lstTreeItem.isEmpty())
            instanceName += QString::number(lstTreeItem.size() + 1);

        QTreeWidgetItem *currDestItem = ui->m_treewdtDestination->currentItem();
        if (currDestItem == nullptr) {
            QMessageBox::critical(this, tr("Error"), tr("Select destination line!"));
            return;
        }
        QTreeWidgetItem *newDestItem = new QTreeWidgetItem(QStringList()<<instanceName<<fileName<<"5");
        newDestItem->setFlags(newDestItem->flags() | Qt::ItemIsEditable);
        newDestItem->setData(0, Qt::UserRole, 1);

        int level = currDestItem->data(0, Qt::UserRole).toInt();
        if (0 == level) { // Group
            currDestItem->addChild(newDestItem);
        }
        else { // Test Item
            QTreeWidgetItem *parent = currDestItem->parent();
            if (parent == nullptr)
                return;
            int index = parent->indexOfChild(currDestItem);
            parent->insertChild(index + 1, newDestItem);
        }
        ui->m_treewdtDestination->setCurrentItem(newDestItem);
    }
    ui->m_treewdtDestination->resizeColumnToContents(0);
}

void TestItemManagementDialog::btnDisselect_clicked()
{
    QTreeWidgetItem *currDestItem = ui->m_treewdtDestination->currentItem();
    if (currDestItem == nullptr)
        return;
    int level = currDestItem->data(0, Qt::UserRole).toInt();
    if (level != 0)
        delete currDestItem->parent()->takeChild(ui->m_treewdtDestination->currentIndex().row());
}

void TestItemManagementDialog::btnOK_clicked()
{
    m_moduleSettings->ImageTestItemMap.clear();
    int row = 1;
    for (int groupIdx = 0; groupIdx < ui->m_treewdtDestination->topLevelItemCount(); groupIdx++) {
        QTreeWidgetItem *itemGroup = ui->m_treewdtDestination->topLevelItem(groupIdx);
        int group = itemGroup->data(0, Qt::UserRole + 1).toInt();
        for (int itemIdx = 0; itemIdx < itemGroup->childCount(); itemIdx++) {
            QTreeWidgetItem *itemTestItem = itemGroup->child(itemIdx);
            QString key = QString::asprintf("testItem%02d", row);
            QString val = QString::number(group) + "," +
                    itemTestItem->text(0) + "@" + itemTestItem->text(1) + "," +
                    itemTestItem->text(2);
            m_moduleSettings->ImageTestItemMap.insert(key, val);
            row++;
        }
    }
    m_moduleSettings->WriteSettings();
    accept();
}

void TestItemManagementDialog::btnCancel_clicked()
{
    reject();
}

void TestItemManagementDialog::btnUp_clicked()
{
    QTreeWidgetItem *currItem = ui->m_treewdtDestination->currentItem();
    if (currItem == nullptr)
        return;
    int currLevel = currItem->data(0, Qt::UserRole).toInt();
    if (1 != currLevel)
        return;
    QTreeWidgetItem *currParent = currItem->parent();
    if (currParent == nullptr)
        return;
    int currIndex = currParent->indexOfChild(currItem);

    QTreeWidgetItem *aboveItem = ui->m_treewdtDestination->itemAbove(currItem);
    if (aboveItem != nullptr) {
        int aboveLevel = aboveItem->data(0, Qt::UserRole).toInt();
        if (0 == aboveLevel) {
            int index = ui->m_treewdtDestination->indexOfTopLevelItem(aboveItem);
            if (index > 0) {
                QTreeWidgetItem *aboveTopItem = ui->m_treewdtDestination->topLevelItem(index - 1);
                if (aboveTopItem != nullptr) {
                    QTreeWidgetItem *theItem = currParent->takeChild(currIndex);
                    aboveTopItem->addChild(theItem);
                    ui->m_treewdtDestination->setCurrentItem(theItem);
                    aboveTopItem->setExpanded(true);
                }
            }
        }
        else {
            QTreeWidgetItem *aboveParent = aboveItem->parent();
            if (aboveParent == nullptr)
                return;
            int aboveIndex = aboveParent->indexOfChild(aboveItem);
            QTreeWidgetItem *theItem = currParent->takeChild(currIndex);
            aboveParent->insertChild(aboveIndex, theItem);
            ui->m_treewdtDestination->setCurrentItem(theItem);
        }
    }
}

void TestItemManagementDialog::btnDown_clicked()
{
    QTreeWidgetItem *currItem = ui->m_treewdtDestination->currentItem();
    if (currItem == nullptr)
        return;
    int currLevel = currItem->data(0, Qt::UserRole).toInt();
    if (1 != currLevel)
        return;
    QTreeWidgetItem *currParent = currItem->parent();
    if (currParent == nullptr)
        return;
    int currIndex = currParent->indexOfChild(currItem);

    QTreeWidgetItem *belowItem = ui->m_treewdtDestination->itemBelow(currItem);
    if (belowItem != nullptr) {
        int belowLevel = belowItem->data(0, Qt::UserRole).toInt();
        if (0 == belowLevel) {
            QTreeWidgetItem *theItem = currParent->takeChild(currIndex);
            belowItem->insertChild(0, theItem);
            ui->m_treewdtDestination->setCurrentItem(theItem);
            belowItem->setExpanded(true);
        }
        else {
            QTreeWidgetItem *belowParent = belowItem->parent();
            if (belowParent == nullptr)
                return;
            int belowIndex = belowParent->indexOfChild(belowItem);
            QTreeWidgetItem *theItem = currParent->takeChild(currIndex);
            belowParent->insertChild(belowIndex, theItem);
            ui->m_treewdtDestination->setCurrentItem(theItem);
        }
    }
}

void TestItemManagementDialog::btnAddGroup_clicked()
{
    int group = ui->m_treewdtDestination->topLevelItemCount() + 1;
    QTreeWidgetItem *itemNewGroup = new QTreeWidgetItem(QStringList() << tr("Group%1").arg(group));
    itemNewGroup->setSizeHint(2, QSize(50, 23));
    itemNewGroup->setData(0, Qt::UserRole, 0); // Level 0
    itemNewGroup->setData(0, Qt::UserRole + 1, group); // group
    ui->m_treewdtDestination->addTopLevelItem(itemNewGroup);
}

void TestItemManagementDialog::btnDelGroup_clicked()
{
    QTreeWidgetItem *currDestItem = ui->m_treewdtDestination->currentItem();
    if (currDestItem == nullptr)
        return;
    int level = currDestItem->data(0, Qt::UserRole).toInt();
    if (level == 0) {
        if (currDestItem->childCount() > 0) {
            QMessageBox::critical(this, tr("Error"), tr("Node is not empty!"));
            return;
        }
        int index = ui->m_treewdtDestination->indexOfTopLevelItem(currDestItem);
        if (index != ui->m_treewdtDestination->topLevelItemCount() - 1) {
            QMessageBox::critical(this, tr("Error"), tr("Only the last group can be deleted!"));
            return;
        }
        delete ui->m_treewdtDestination->takeTopLevelItem(ui->m_treewdtDestination->currentIndex().row());
    }
}

void TestItemManagementDialog::btnClear_clicked()
{
    ui->m_treewdtDestination->clear();
}

void TestItemManagementDialog::initTableWidgetSource()
{
    ui->m_tabwdtSource->setHorizontalHeaderLabels(QStringList()<<tr("Plugin Name")<<tr("File Name")<<tr("Version")<<tr("Description"));

    QDir dir(GlobalVars::PLUGIN_TESTITEM_PATH);
    if(dir.exists()) {
        QFileInfoList fileInfoList = dir.entryInfoList(QStringList()<<"*.so", QDir::Files, QDir::Name);
        QListIterator<QFileInfo> it(fileInfoList);
        int row = 0;
        while (it.hasNext()) {
            QFileInfo fileInfo = it.next();
            QString libFileName = fileInfo.fileName();
            QString fullInstanceName = "tempInstance@" + libFileName;
            T_PluginInfo pluginInfo;
            ITestItem::E_ItemType itemType;
            if ((m_testItemService->GetPluginInfo(fullInstanceName, pluginInfo) == ERR_NoError) &&
                (m_testItemService->GetItemType(fullInstanceName, itemType) == ERR_NoError) &&
                (ITestItem::ItemType_ImageEvaluation == itemType))
            {
                ui->m_tabwdtSource->setRowCount(row + 1);
                ui->m_tabwdtSource->setItem(row, 0, new QTableWidgetItem(QString::fromLocal8Bit(pluginInfo.FriendlyName)));
                ui->m_tabwdtSource->setItem(row, 1, new QTableWidgetItem(libFileName));
                QString strVer = QString::asprintf("V%d.%d.%d.%d",
                                                   (pluginInfo.Version >> 24) & 0xff, (pluginInfo.Version >> 16) & 0xff,
                                                   (pluginInfo.Version >> 8) & 0xff, pluginInfo.Version & 0xff);
                ui->m_tabwdtSource->setItem(row, 2, new QTableWidgetItem(strVer));
                ui->m_tabwdtSource->setItem(row, 3, new QTableWidgetItem(QString::fromLocal8Bit(pluginInfo.Description)));
                row++;
                m_testItemService->FreePluginInstance(fullInstanceName);
            }
        }
    }
    ui->m_tabwdtSource->resizeColumnsToContents();
}

void TestItemManagementDialog::initTreeWidgetDestination()
{
    m_groupedTestItems.clear();
    QListIterator<QString> itTestItem(m_moduleSettings->ImageTestItemMap.values());
    while (itTestItem.hasNext()) {
        QString strTestItem = itTestItem.next();
        QStringList slSubItem = strTestItem.split(QChar(','), QString::KeepEmptyParts);
        if ((slSubItem.size() < 3) || (slSubItem[1].indexOf(QChar('@')) < 0))
            continue;

        T_TestItemConf testItemConf;
        testItemConf.Group = slSubItem[0].toInt();
        testItemConf.InstanceName = slSubItem[1].section(QChar('@'), 0, 0);
        testItemConf.LibraryName = slSubItem[1].section(QChar('@'), 1, 1);
        testItemConf.ErrorCode = slSubItem[2].toInt();
//        qDebug()<<testItemConf.Group<<testItemConf.InstanceName<<testItemConf.LibraryName<<testItemConf.ErrorCode;

        if (!m_groupedTestItems.contains(testItemConf.Group))
            m_groupedTestItems.insert(testItemConf.Group, QVector<T_TestItemConf>());
        QVector<T_TestItemConf> &theGroup = m_groupedTestItems[testItemConf.Group];
        theGroup.append(testItemConf);
    }
    // add the empty group
    int maxGroup = m_groupedTestItems.isEmpty() ? 0 : m_groupedTestItems.lastKey();
    for (int group = 1; group <= maxGroup; group++) {
        if (!m_groupedTestItems.contains(group))
            m_groupedTestItems.insert(group, QVector<T_TestItemConf>());
    }

    // display to ui
    ui->m_treewdtDestination->setHeaderLabels(QStringList()<<tr("Item Name")<<tr("File Name")<<tr("Error Code"));
    ui->m_treewdtDestination->setUniformRowHeights(true);
    QMapIterator<int, QVector<T_TestItemConf>> itGroup(m_groupedTestItems);
    while (itGroup.hasNext())
    {
        itGroup.next();
        int group = itGroup.key();
        const QVector<T_TestItemConf> &theGroup = itGroup.value();
        QTreeWidgetItem *itemGroup = new QTreeWidgetItem(QStringList() << tr("Group%1").arg(group));
        itemGroup->setSizeHint(2, QSize(50, 23));
//        itemGroup->setBackgroundColor(0, QColor(Qt::darkYellow));
        itemGroup->setData(0, Qt::UserRole, 0); // Level 0
        itemGroup->setData(0, Qt::UserRole + 1, group); // group
        ui->m_treewdtDestination->addTopLevelItem(itemGroup);

        QVectorIterator<T_TestItemConf> itItem(theGroup);
        while (itItem.hasNext()) {
            const T_TestItemConf &testItemConf = itItem.next();
            QTreeWidgetItem *itemTestItem = new QTreeWidgetItem(QStringList() << testItemConf.InstanceName << testItemConf.LibraryName << QString::number(testItemConf.ErrorCode));
            itemTestItem->setFlags(itemTestItem->flags() | Qt::ItemIsEditable);
            itemTestItem->setData(0, Qt::UserRole, 1);  // Level 1
            QString fullInstanceName = testItemConf.InstanceName + "@" + testItemConf.LibraryName;
            T_PluginInfo pluginInfo;
            if (m_testItemService->GetPluginInfo(fullInstanceName, pluginInfo) == ERR_NoError) {
                for (int col = 0; col < ui->m_treewdtDestination->columnCount(); col++)
                    itemTestItem->setForeground(col, QBrush(Qt::black));
            }
            else { // highlight
                for (int col = 0; col < ui->m_treewdtDestination->columnCount(); col++)
                    itemTestItem->setForeground(col, QBrush(Qt::red));
            }
//            m_testItemService->FreePluginInstance(fullInstanceName);
            itemGroup->addChild(itemTestItem);
        }
    }
    ui->m_treewdtDestination->expandAll();
    ui->m_treewdtDestination->resizeColumnToContents(0);
}

void TestItemManagementDialog::lineEditTextChanged(const QString& strText)
{
    qDebug() << strText;
    QList<QTableWidgetItem*> ltItem = ui->m_tabwdtSource->findItems(strText, Qt::MatchContains);
    QListIterator<QTableWidgetItem*> itItem(ltItem);
    while (itItem.hasNext())
    {
        auto it = itItem.next();
        if (0 != it->column())
        {
            continue;
        }

        ui->m_tabwdtSource->selectRow(it->row());
        break;
    }
}
