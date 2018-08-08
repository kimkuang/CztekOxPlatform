#ifndef TESTITEMMANAGEMENTDIALOG_H
#define TESTITEMMANAGEMENTDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QAbstractButton>
#include "conf/ModuleSettings.h"
#include "service/TestItemService.h"

namespace Ui {
class TestItemManagementDialog;
}

class TestItemManagementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TestItemManagementDialog(uint windowIdx, uint chnIdx, QWidget *parent = 0);
    ~TestItemManagementDialog();

private slots:
    void btnSelect_clicked();
    void btnDisselect_clicked();
    void btnOK_clicked();
    void btnCancel_clicked();
    void btnUp_clicked();
    void btnDown_clicked();
    void btnAddGroup_clicked();
    void btnDelGroup_clicked();
    void btnClear_clicked();
    void lineEditTextChanged(const QString& strText);

private:
    Ui::TestItemManagementDialog *ui;
    uint m_windowIdx;
    ModuleSettings *m_moduleSettings;
    TestItemService *m_testItemService;
    struct T_TestItemConf {
        int Group;
        QString InstanceName;
        QString LibraryName;
        int ErrorCode;
    };
    QMap<int, QVector<T_TestItemConf>> m_groupedTestItems;

    void initTableWidgetSource();
    void initTreeWidgetDestination();
};

#endif // TESTITEMMANAGEMENTDIALOG_H
