#include "ui/SensorDetailDialog.h"
#include "ui_SensorDetailDialog.h"
#include <QSqlQuery>
#include <QList>
#include <QMap>
#include <QDebug>
#include <QSettings>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include "dal/sqldatabaseutil.h"
#include "conf/sensorini.h"
#include "EditSensorDialog.h"
#include "common/Global.h"
#ifdef __linux__
#include "common/UdevManager.h"
#endif

#define QTROLE_TREE_VIEW_LEVEL Qt::UserRole + 1
#define QTROLE_TREE_VIEW_ID    Qt::UserRole + 2

SensorDetailDialog::SensorDetailDialog(const QString &sensorName, int interfaceType, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SensorDetailDialog)
{
    ui->setupUi(this);
    m_interfaceType = interfaceType;
    readSettings();
    ui->m_treeView->setHeaderHidden(true);

    ui->m_tableView->horizontalHeader()->setStretchLastSection(true);
    ui->m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->m_tableView->setAlternatingRowColors(true);
    ui->m_tableView->setStyleSheet("alternate-background-color: rgb(220, 220, 220);");

    connectSignalSlots();
    m_sensorBll = new SensorBLL();
    m_treeViewModel = new QStandardItemModel(ui->m_treeView);
    m_tableViewModel = new QStandardItemModel(ui->m_tableView);
    m_treeViewFilterModel = new SortFilterProxyModelPlus(ui->m_treeView);
    updateTreeView();
    locateTreeViewSchemeNode(sensorName, 1);
}

SensorDetailDialog::~SensorDetailDialog()
{
    delete ui;
    delete m_sensorBll;
}

void SensorDetailDialog::GetSelectSensor(Sensor &sensor)
{
    sensor = m_currentSensor;
}

void SensorDetailDialog::btnEdit_clicked()
{
    QModelIndex index = ui->m_treeView->currentIndex();
    if (!index.isValid()) {
        qCritical()<<"index is invalid!"<<index.row()<<index.column();
        return;
    }
    int id = index.data(QTROLE_TREE_VIEW_ID).toInt();
    DialogEditSensor dlg(this);
    dlg.setWindowTitle(tr("Edit Sensor Profile"));
    dlg.SetId(id);
    if (dlg.exec() == QDialog::Accepted) {
        updateTreeView();
        QString schemeName;
        dlg.GetSchemeName(schemeName);
        locateTreeViewSchemeNode(schemeName, 1);
    }
}

void SensorDetailDialog::btnDelete_clicked()
{
    QModelIndex index = ui->m_treeView->currentIndex();
    if (!index.isValid()) {
        qCritical()<<"index is invalid!"<<index.row()<<index.column();
        return;
    }
    int level = index.data(QTROLE_TREE_VIEW_LEVEL).toInt();
    if (level == 0) {
        QMessageBox::critical(this, tr("Error"), tr("Cannot remove a vendor!"), QMessageBox::Ok);
        return;
    }
    QString str = ui->m_treeView->model()->data(index).toString();
    if (QMessageBox::question(this, tr("Question"), tr("Sure to delete the record [%1]?").arg(str),
                              QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Cancel) {
        return;
    }
    int id = index.data(QTROLE_TREE_VIEW_ID).toInt();
    if (m_sensorBll->Delete(id)) { //delete successfully
        QString vendorName = index.parent().data(Qt::DisplayRole).toString();
        qDebug()<<"vendorname:"<<vendorName;
        updateTreeView();
        locateTreeViewSchemeNode(vendorName, 0);
        QMessageBox::information(this, tr("Information"), tr("Record [%1] has been deleted.").arg(str), QMessageBox::Ok);
    }
}

void SensorDetailDialog::btnImport_clicked()
{
    QString initDir = GlobalVars::APP_PATH;
#ifdef __linux__
    QList<UdevTypes::T_UdevDevice> devList;
    UdevManager::GetUdevList(UdevTypes::DT_UsbDisk, devList);
    if (devList.count() > 0) {
        initDir = devList[0].ExtraInfo;
    }
#endif
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select sensor profile file..."), initDir, tr("Sensor profile files(*.ini)"));
    if (fileName.isEmpty())
        return;
    SensorIni iniParser;
    Sensor sensor;
    if (iniParser.LoadFromFile(sensor, fileName)) {
        if (m_sensorBll->Insert(sensor)) {
            m_currentSensor = sensor;
            updateTreeView();
            locateTreeViewSchemeNode(m_currentSensor.SchemeName, 1);
            QMessageBox::information(this, tr("Information"), tr("Importing data succeed."), QMessageBox::Ok);
        }
        else {
            QMessageBox::critical(this, tr("Error"), tr("Import data failed."), QMessageBox::Ok);
        }
    }
    else {
        QMessageBox::critical(this, tr("Error"), tr("Unkown file format!"), QMessageBox::Ok);
    }
}

void SensorDetailDialog::btnExport_clicked()
{
    QModelIndex index = ui->m_treeView->currentIndex();
    if (!index.isValid()) {
        qCritical()<<"index is invalid!"<<index.row()<<index.column();
        return;
    }
    int id = index.data(QTROLE_TREE_VIEW_ID).toInt();

    QString initDir = GlobalVars::APP_PATH;
#ifdef __linux__
    QList<UdevTypes::T_UdevDevice> devList;
    UdevManager::GetUdevList(UdevTypes::DT_UsbDisk, devList);
    if (devList.count() > 0) {
        initDir = devList[0].ExtraInfo;
    }
#endif
    QFileDialog dlg(this, tr("Save sensor profile file..."), initDir, tr("Sensor profile files(*.ini)"));
    QString fileName = ui->m_treeView->model()->data(index).toString();
    dlg.selectFile(fileName);
    dlg.setDefaultSuffix("ini");
    QStringList fileList;
    if (dlg.exec() == QDialog::Accepted)
        fileList = dlg.selectedFiles();
    if (fileList.count() < 1 || fileList[0].isEmpty())
        return;
    fileName = fileList[0];
//    qDebug()<<fileName;
    Sensor sensor;
    sensor.Id = id;
    if (m_sensorBll->SelectById(sensor)) {
        SensorIni iniParser;
        if (iniParser.SaveToFile(sensor, fileName))
            QMessageBox::information(this, tr("Information"), tr("Exporting data succeed."), QMessageBox::Ok);
    }
}

void SensorDetailDialog::btnExportTemplate_clicked()
{
    QString initDir = GlobalVars::APP_PATH;
#ifdef __linux__
    QList<UdevTypes::T_UdevDevice> devList;
    UdevManager::GetUdevList(UdevTypes::DT_UsbDisk, devList);
    if (devList.count() > 0) {
        initDir = devList[0].ExtraInfo;
    }
#endif
    QFileDialog dlg(this, tr("Save sensor profile file..."), initDir, tr("Sensor profile files(*.ini)"));
    QString fileName = "SensorTemplate";
    dlg.selectFile(fileName);
    dlg.setDefaultSuffix("ini");
    QStringList fileList;
    if (dlg.exec() == QDialog::Accepted)
        fileList = dlg.selectedFiles();
    if (fileList.count() < 1 || fileList[0].isEmpty())
        return;
    fileName = fileList[0];
    SensorIni iniParser;
    if (iniParser.SaveTemplateToFile(fileName))
        QMessageBox::information(this, tr("Information"), tr("Exporting data succeed."), QMessageBox::Ok);
}

void SensorDetailDialog::btnOK_clicked()
{
    writeSettings();
    accept();
}

void SensorDetailDialog::btnCancel_clicked()
{
    reject();
}

void SensorDetailDialog::treeView_clicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        qCritical()<<"index is invalid!"<<index.row()<<index.column();
        return;
    }
    int level = index.data(QTROLE_TREE_VIEW_LEVEL).toInt();
    ui->m_btnExport->setEnabled(level == 1);
    int id = index.data(QTROLE_TREE_VIEW_ID).toInt();
    updateTableView(id);
}

void SensorDetailDialog::treeView_doubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        qCritical()<<"index is invalid!"<<index.row()<<index.column();
        return;
    }
    btnOK_clicked();
}

void SensorDetailDialog::applyTreeViewFilter(const QString & text)
{
    m_treeViewFilterModel->setFilterKeyColumn(-1);
    QRegExp regExp(text, Qt::CaseInsensitive);
    m_treeViewFilterModel->setFilterRegExp(regExp);

    setCurrentTreeViewItem(m_treeViewFilterModel->index(0, 0));
}

void SensorDetailDialog::connectSignalSlots()
{    
    connect(ui->m_btnEdit, SIGNAL(clicked()), this, SLOT(btnEdit_clicked()));
    connect(ui->m_btnDelete, SIGNAL(clicked()), this, SLOT(btnDelete_clicked()));
    connect(ui->m_btnImport, SIGNAL(clicked()), this, SLOT(btnImport_clicked()));
    connect(ui->m_btnExport, SIGNAL(clicked()), this, SLOT(btnExport_clicked()));
    connect(ui->m_btnExportTemplate, &QPushButton::clicked, this, &SensorDetailDialog::btnExportTemplate_clicked);
    connect(ui->m_btnOK, SIGNAL(clicked()), this, SLOT(btnOK_clicked()));
    connect(ui->m_btnCancel, SIGNAL(clicked()), this, SLOT(btnCancel_clicked()));
    // NOTICE: 换上一行编译后找不到信号。原因不明。
//    connect(ui->m_treeView, &QAbstractItemView::clicked, this, &SensorDetailDialog::treeView_clicked);
    connect(ui->m_treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(treeView_clicked(QModelIndex)));
    connect(ui->m_treeView, &QTreeView::doubleClicked, this, &SensorDetailDialog::treeView_doubleClicked);
    connect(ui->m_edtTreeViewFilter, &QLineEdit::textChanged, this, &SensorDetailDialog::applyTreeViewFilter);
}

void SensorDetailDialog::updateTreeView()
{
    m_treeViewModel->clear();
    QMap<QString, QMap<int, QString> > treeCache;
    QList<Sensor> sensors;
    if (!m_sensorBll->SelectList_IdVendorSchemeNameByInterfaceType(sensors, m_interfaceType))
        return;
    Sensor sensor;
    for (int i = 0; i < sensors.count(); i++) {
        sensor = sensors[i];
        int id = sensor.Id;
        QString vendorName = sensor.VendorName;
        QString schemeName = sensor.SchemeName;
        if (treeCache.contains(vendorName)) {
            treeCache[vendorName].insert(id, schemeName);
        }
        else {
            QMap<int, QString> idSchemeMap;
            idSchemeMap.insert(id, schemeName);
            treeCache.insert(vendorName, idSchemeMap);
        }
    }

    /**
     * 树分为两级，第一级表示VendorName，第二级表示SchemeName，每个节点有两个自定义数据，第一个是树的层级，
     * 第二个是数据库记录的ID，方便后面记录定位。如果是第一层节点，数据库记录的ID是第一个孩子节点的ID。
     **/
    QMapIterator<QString, QMap<int, QString> > it(treeCache);
    while (it.hasNext()) {
        it.next();
        QStandardItem *item = new QStandardItem(it.key()); // Vendor Name
        //item->setData(it.key(), Qt::DisplayRole);
        item->setData(0, QTROLE_TREE_VIEW_LEVEL);
        bool flag = false;
        QMapIterator<int, QString> it2(it.value());
        while (it2.hasNext()) {
            it2.next();
            QStandardItem *subItem = new QStandardItem(it2.value()); // Scheme Name
            //subItem->setData(it2.value(), Qt::DisplayRole);
            subItem->setData(1, QTROLE_TREE_VIEW_LEVEL);
            subItem->setData(it2.key(), QTROLE_TREE_VIEW_ID);
            item->appendRow(subItem);
            if (!flag) {
                item->setData(it2.key(), QTROLE_TREE_VIEW_ID);
                flag = true;
            }
        }
        if (!flag)
            item->setData(0, QTROLE_TREE_VIEW_ID);
        m_treeViewModel->appendRow(item);
    }

    m_treeViewFilterModel->setSourceModel(m_treeViewModel);
    ui->m_treeView->setModel(m_treeViewFilterModel);
    ui->m_treeView->setSortingEnabled(true);
}

void SensorDetailDialog::updateTableView(int id)
{
    QList<QPair<QString, QString> > tableViewCache;
    m_currentSensor.Id = id;
    if (m_sensorBll->SelectById(m_currentSensor)) {
        tableViewCache.append(QPair<QString, QString>(tr("Scheme Name"), m_currentSensor.SchemeName));
        tableViewCache.append(QPair<QString, QString>(tr("Vendor Name"), m_currentSensor.VendorName));
        tableViewCache.append(QPair<QString, QString>(tr("Description"), m_currentSensor.Description));
        tableViewCache.append(QPair<QString, QString>(tr("Chip Name"), m_currentSensor.ChipName));
        tableViewCache.append(QPair<QString, QString>(tr("Comm Address"), "0x"+QString::number(m_currentSensor.CommAddr, 16)));
        tableViewCache.append(QPair<QString, QString>(tr("Interface Type"), QString::number(m_currentSensor.InterfaceType)));
        tableViewCache.append(QPair<QString, QString>(tr("Lanes"), QString::number(m_currentSensor.Lanes) + "L"));
//        tableViewCache.append(QPair<QString, QString>(tr("Data Width"), QString::number(m_currentSensor.DataWidth)));
        tableViewCache.append(QPair<QString, QString>(tr("Pixel Width"), QString::number(m_currentSensor.PixelWidth)));
        tableViewCache.append(QPair<QString, QString>(tr("Pixel Height"), QString::number(m_currentSensor.PixelHeight)));
//        tableViewCache.append(QPair<QString, QString>(tr("PCLK Pol"), QString::number(m_currentSensor.PclkPol)));
//        tableViewCache.append(QPair<QString, QString>(tr("DATA Pol"), QString::number(m_currentSensor.DataPol)));
//        tableViewCache.append(QPair<QString, QString>(tr("HSYNC Pol"), QString::number(m_currentSensor.HsyncPol)));
//        tableViewCache.append(QPair<QString, QString>(tr("VSYNC Pol"), QString::number(m_currentSensor.VsyncPol)));
        tableViewCache.append(QPair<QString, QString>(tr("PWDN"), QString::number(m_currentSensor.Pwdn)));
        tableViewCache.append(QPair<QString, QString>(tr("RESET"), QString::number(m_currentSensor.Reset)));
        QStringList strList = m_currentSensor.FlagRegister.split(QChar('\n'), QString::SkipEmptyParts);
        if (strList.count() > 0)
            tableViewCache.append(QPair<QString, QString>(tr("Flag Register1"), strList[0]));
        if (strList.count() > 1)
            tableViewCache.append(QPair<QString, QString>(tr("Flag Register2"), strList[1]));
        tableViewCache.append(QPair<QString, QString>(tr("Image Format"), QString::number(m_currentSensor.ImageFormat)));
        tableViewCache.append(QPair<QString, QString>(tr("Image Mode"), QString::number(m_currentSensor.ImageMode)));
        tableViewCache.append(QPair<QString, QString>(tr("Mipi Frequency"), QString::number(m_currentSensor.MipiFreq)));
        tableViewCache.append(QPair<QString, QString>(tr("Mclk"), QString::number(m_currentSensor.Mclk)));

        ui->m_btnEdit->setEnabled(m_currentSensor.SystemRecord != 1);
        ui->m_btnDelete->setEnabled(m_currentSensor.SystemRecord != 1);
        ui->m_btnExport->setEnabled(m_currentSensor.SystemRecord != 1);

        ui->m_lblSensorName->setText(m_currentSensor.SchemeName);
    }
    m_tableViewModel->setColumnCount(2);
    m_tableViewModel->setRowCount(tableViewCache.count());
    m_tableViewModel->setHeaderData(0, Qt::Horizontal, tr("Property"));
    m_tableViewModel->setHeaderData(1, Qt::Horizontal, tr("Value"));
    QListIterator<QPair<QString, QString> > it(tableViewCache);
    int row = 0;
    while (it.hasNext()) {
        QPair<QString, QString> pair = it.next();
        QModelIndex index = m_tableViewModel->index(row, 0, QModelIndex());
        m_tableViewModel->setData(index, pair.first, Qt::DisplayRole);
        index = m_tableViewModel->index(row, 1, QModelIndex());
        m_tableViewModel->setData(index, pair.second, Qt::DisplayRole);
        row++;
//        qDebug()<<pair.first<<pair.second;
    }
    ui->m_tableView->setModel(m_tableViewModel);
}

void SensorDetailDialog::setCurrentTreeViewItem(const QModelIndex &index)
{
    if (!index.isValid()) {
        qCritical()<<"index is invalid!"<<index.row()<<index.column();
        return;
    }
    int level = index.data(QTROLE_TREE_VIEW_LEVEL).toInt();
    if (level == 0) {
        ui->m_treeView->expand(index);
        if (m_treeViewFilterModel->rowCount(index) > 0) {
            ui->m_treeView->setCurrentIndex(index.child(0, 0));
            ui->m_treeView->clicked(index.child(0, 0));
        }
    }
    else {
        ui->m_treeView->expand(index.parent());
        ui->m_treeView->setCurrentIndex(index);
        ui->m_treeView->clicked(index);
    }
}

void SensorDetailDialog::locateTreeViewSchemeNode(const QString &text, int level)
{
    bool hasFound = false;
    int vendorCount = m_treeViewFilterModel->rowCount();
    for (int r = 0; r < vendorCount; r++)
    {
        QModelIndex vendorIdx = m_treeViewFilterModel->index(r, 0);
        if (level == 0) // 查询供应商节点
        {
            if (vendorIdx.data(Qt::DisplayRole).toString() == text)
            {
                setCurrentTreeViewItem(vendorIdx);
                hasFound = true;
                break;
            }
        }
        else // 查询方案节点
        {
            int schemeCount = m_treeViewFilterModel->rowCount(vendorIdx);
            for (int r2 = 0; r2 < schemeCount; r2++)
            {
                QModelIndex schemeIdx = vendorIdx.child(r2, 0);
                if (schemeIdx.data(Qt::DisplayRole).toString() == text)
                {
                    setCurrentTreeViewItem(schemeIdx);
                    hasFound = true;
                    break;
                }
            }
            if (hasFound)
                break;
        }
    }
    if (!hasFound) {
        setCurrentTreeViewItem(m_treeViewFilterModel->index(0, 0));
    }
}

void SensorDetailDialog::readSettings()
{
//    QSettings settings(GlobalVars::APP_PATH + GlobalVars::UI_INI_FILE_NAME, QSettings::IniFormat);
//    ui->splitter->restoreState(settings.value("sensorConfig/splitter").toByteArray());
}

void SensorDetailDialog::writeSettings()
{
//    QSettings settings(GlobalVars::APP_PATH + GlobalVars::UI_INI_FILE_NAME, QSettings::IniFormat);
//    settings.beginGroup("sensorConfig");
//    settings.setValue("splitter", ui->splitter->saveState());
//    settings.endGroup();
//    settings.sync();
}
