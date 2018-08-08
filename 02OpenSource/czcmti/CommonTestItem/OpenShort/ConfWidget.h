#ifndef CONFWIDGET_H
#define CONFWIDGET_H

#include <QWidget>
#include <QTableView>
#include "Option.h"
#include "GeneralRoTableModel.h"

class OpenShort;

namespace Ui {
class ConfWidget;
}

class ConfWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConfWidget(QWidget *parent = 0);
    ~ConfWidget();

    int RestoreDefaults();
    int Cache2Ui();
    int Ui2Cache();
    void BindOpenShort(OpenShort *openShrot);

private slots:
    void btnDebugOS_clicked();

private:
    Ui::ConfWidget *ui;
    Option *m_option;
    GeneralRoTableModel *m_tabmodelOsTestResult;
    QList<QStringList> m_osTestResultRecordList;

    void initTableView(QTableView *tabView, QAbstractTableModel *tabModel);
    void initListWidget(const std::map<int, std::string> &pinNameMap);
    std::vector<int> getSelectedPins();
    void setSelectedPins(const std::vector<int> &testPins);

    OpenShort *m_openShortObj;
};

#endif // CONFWIDGET_H
