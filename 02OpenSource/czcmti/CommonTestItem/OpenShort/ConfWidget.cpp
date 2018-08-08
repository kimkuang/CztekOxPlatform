#include "ConfWidget.h"
#include "ui_ConfWidget.h"
#include "EnumNameMap.h"
#include <QDebug>
#include "OpenShort.h"

ConfWidget::ConfWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfWidget)
{
    ui->setupUi(this);
    connect(ui->m_btnDebugOS, &QPushButton::clicked, this, &ConfWidget::btnDebugOS_clicked);
    m_option = Option::GetInstance();

    EnumNameMap enumNameMap;
    initListWidget(enumNameMap.GetPinNameMap());
    m_tabmodelOsTestResult = new GeneralRoTableModel(this);
    initTableView(ui->m_tabvwOsResult, m_tabmodelOsTestResult);
#ifdef DISABLE_CONTINUE_WHEN_FAILED // Disable this function
    ui->m_chkbContinueWhenFailed->setEnabled(false);
#endif

    m_openShortObj = nullptr;
}

ConfWidget::~ConfWidget()
{
    m_option = nullptr;
    delete ui;
}

int ConfWidget::RestoreDefaults()
{
    m_option->RestoreDefaults();
    Cache2Ui();

    m_osTestResultRecordList.clear();
    m_tabmodelOsTestResult->RefreshTable(m_osTestResultRecordList);
    return ERR_NoError;
}

int ConfWidget::Cache2Ui()
{
    ui->m_spinSupplyVolt->setValue(m_option->SupplyVol_mV);
    ui->m_spinSupplyCurr->setValue(m_option->SupplyCur_uA);
    ui->m_spinOpenThreshold->setValue(m_option->OpenThreshold_mV);
    ui->m_spinShortThreshold->setValue(m_option->ShortThreshold_mV);
    setSelectedPins(m_option->TestPinVector);
#ifdef DISABLE_CONTINUE_WHEN_FAILED
    ui->m_chkbContinueWhenFailed->setChecked(false);
#else
    ui->m_chkbContinueWhenFailed->setChecked(m_option->ContinueWhenFailed);
#endif
    return 0;
}

int ConfWidget::Ui2Cache()
{
    m_option->SupplyVol_mV = ui->m_spinSupplyVolt->value();
    m_option->SupplyCur_uA = ui->m_spinSupplyCurr->value();
    m_option->OpenThreshold_mV = ui->m_spinOpenThreshold->value();
    m_option->ShortThreshold_mV = ui->m_spinShortThreshold->value();
    m_option->TestPinVector = getSelectedPins();
#ifdef DISABLE_CONTINUE_WHEN_FAILED
    m_option->ContinueWhenFailed = false;
#else
    m_option->ContinueWhenFailed = ui->m_chkbContinueWhenFailed->isChecked();
#endif
    return 0;
}

void ConfWidget::btnDebugOS_clicked()
{
    if (m_openShortObj == nullptr) {
        qCritical("Test item is null.");
        return;
    }

    Ui2Cache();
    std::vector<std::string> resultTable;
    /*int ec = */m_openShortObj->RunTest(resultTable);
    m_osTestResultRecordList.clear();
    for (auto it = resultTable.begin(); it != resultTable.end(); ++it) {
        QString strTemp = QString::fromStdString(*it);
        QStringList oneRecord = strTemp.split(QChar(','), QString::KeepEmptyParts);
        m_osTestResultRecordList.push_back(oneRecord);
    }
    m_tabmodelOsTestResult->RefreshTable(m_osTestResultRecordList);
    ui->m_tabvwOsResult->resizeColumnsToContents();
}

void ConfWidget::initTableView(QTableView *tabView, QAbstractTableModel *tabModel)
{
    tabView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tabView->setAlternatingRowColors(true);
    tabView->setStyleSheet("alternate-background-color: rgb(220, 220, 220);");
    tabView->horizontalHeader()->setStretchLastSection(true);
    tabView->horizontalHeader()->setHighlightSections(false);
    tabView->horizontalHeader()->setVisible(true);
    tabView->verticalHeader()->setVisible(true);
    tabView->verticalHeader()->setDefaultSectionSize(25);
    tabView->setShowGrid(true);
    tabView->setSelectionMode(QAbstractItemView::SingleSelection);
    tabView->setModel(tabModel);
#if 0
    tabView->setDragEnabled(true);
    tabView->viewport()->setAcceptDrops(true);
    tabView->setDropIndicatorShown(true);
    tabView->setDragDropMode(QAbstractItemView::InternalMove);
#endif
    tabView->resizeColumnsToContents();
}

void ConfWidget::initListWidget(const std::map<int, std::string> &pinNameMap)
{
    ui->m_lstwdtPinNames->clear();
//    ui->m_lstwdtPinNames->setModelColumn(2); // FIXME
    for (auto it = pinNameMap.begin(); it != pinNameMap.end(); ++it) {
        QListWidgetItem *item = new QListWidgetItem(ui->m_lstwdtPinNames);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        item->setCheckState(Qt::Unchecked);
        item->setText(QString::fromStdString(it->second));
        item->setData(Qt::UserRole, (int)it->first);
    }
}

std::vector<int> ConfWidget::getSelectedPins()
{
    std::vector<int> testPins;
    for (int i = 0; i < ui->m_lstwdtPinNames->count(); i++) {
        QListWidgetItem *item = ui->m_lstwdtPinNames->item(i);
        if (item->checkState() == Qt::Checked) {
            testPins.push_back(item->data(Qt::UserRole).toInt());
        }
    }

    return testPins;
}

void ConfWidget::setSelectedPins(const std::vector<int> &testPins)
{
    for (int i = 0; i < ui->m_lstwdtPinNames->count(); i++) {
        QListWidgetItem *item = ui->m_lstwdtPinNames->item(i);
        auto it = std::find(testPins.begin(), testPins.end(), item->data(Qt::UserRole).toInt());
        if (it != testPins.end())
            item->setCheckState(Qt::Checked);
        else
            item->setCheckState(Qt::Unchecked);
    }
}

void ConfWidget::BindOpenShort(OpenShort *openShrot)
{
    m_openShortObj = openShrot;
}
