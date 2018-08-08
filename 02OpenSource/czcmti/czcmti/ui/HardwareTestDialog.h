#ifndef HARDWARETESTDIALOG_H
#define HARDWARETESTDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include <QAbstractButton>
#include "conf/ModuleSettings.h"
#include "service/TestItemService.h"

namespace Ui {
class HardwareTestDialog;
}

class HardwareTestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HardwareTestDialog(uint windowIdx, uint chnIdx, QWidget *parent = 0);
    ~HardwareTestDialog();

private slots:
    void lstwdtHardwareTestItems_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void buttonBox_clicked(QAbstractButton *button);

private:
    Ui::HardwareTestDialog *ui;
    uint m_windowIdx;
    ModuleSettings *m_moduleSettings;
    TestItemService *m_testItemService;
    QMap<QString, QWidget *> m_name2WidgetMap;

    void initListWidget();
};

#endif // HARDWARETESTDIALOG_H
