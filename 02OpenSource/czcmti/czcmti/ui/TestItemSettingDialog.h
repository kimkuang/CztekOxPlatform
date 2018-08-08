#ifndef TESTITEMSETTINGDIALOG_H
#define TESTITEMSETTINGDIALOG_H

#include <QDialog>
#include <QAbstractButton>
#include "conf/ModuleSettings.h"
#include "service/TestItemService.h"

namespace Ui {
class TestItemSettingDialog;
}

class TestItemSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TestItemSettingDialog(uint windowIdx, uint chnIdx, QWidget *parent = 0);
    ~TestItemSettingDialog();

private slots:
    void tabTestItemConf_currentChanged(int index);
    void buttonBox_clicked(QAbstractButton *button);

private:
    Ui::TestItemSettingDialog *ui;
    uint m_windowIdx;
    ModuleSettings *m_moduleSettings;
    TestItemService *m_testItemService;
    QMap<QString, QString> m_mapInstName2InstFullName;

    void initTabWidget();
};

#endif // TESTITEMSETTINGDIALOG_H
