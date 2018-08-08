#ifndef DIALOGSENSORCONFIG_H
#define DIALOGSENSORCONFIG_H

#include <QDialog>
#include <QString>
#include <QDataWidgetMapper>
#include <QStandardItemModel>
#include <QSqlQueryModel>
//#include <QSortFilterProxyModel>
#include "bll/sensorbll.h"
#include "entities/sensor.h"
#include "SortFilterProxyModelPlus.h"

namespace Ui {
class SensorDetailDialog;
}

class QSettings;

class SensorDetailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SensorDetailDialog(const QString &sensorName, int interfaceType, QWidget *parent = 0);
    ~SensorDetailDialog();
    void GetSelectSensor(Sensor &sensor);

private slots:
    void btnEdit_clicked();
    void btnDelete_clicked();
    void btnImport_clicked();
    void btnExport_clicked();
    void btnExportTemplate_clicked();
    void btnOK_clicked();
    void btnCancel_clicked();
    void treeView_clicked(const QModelIndex &index);
    void treeView_doubleClicked(const QModelIndex &index);
    void applyTreeViewFilter(const QString &text);

private:
    Ui::SensorDetailDialog *ui;
    SensorBLL *m_sensorBll;
    QStandardItemModel *m_treeViewModel;
    QStandardItemModel *m_tableViewModel;
//    QSortFilterProxyModel *m_treeViewFilterModel;
    SortFilterProxyModelPlus *m_treeViewFilterModel;
    Sensor m_currentSensor;
    int m_interfaceType;

    void connectSignalSlots();
    void updateTreeView();
    void setCurrentTreeViewItem(const QModelIndex &index);
    void locateTreeViewSchemeNode(const QString &text, int level);
    void updateTableView(int id);
    void readSettings();
    void writeSettings();
};

#endif // DIALOGSENSORCONFIG_H
