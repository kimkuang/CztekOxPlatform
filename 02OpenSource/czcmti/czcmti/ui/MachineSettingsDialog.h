#ifndef MACHINESETTINGSDIALOG_H
#define MACHINESETTINGSDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include <QAbstractButton>
#include "conf/SystemSettings.h"
#include "service/MachineService.h"

namespace Ui {
class MachineSettingsDialog;
}

class MachineSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MachineSettingsDialog(SystemSettings *systemSettings, QWidget *parent = 0);
    ~MachineSettingsDialog();

private slots:
    void buttonBox_clicked(QAbstractButton *button);
    void machineRadioChange();

private:
    Ui::MachineSettingsDialog *ui;
    SystemSettings *m_systemSettings;
    MachineService *m_machineService;
    QMap<QString, QWidget *> m_name2WidgetMap;

    void initListWidget();
};

#endif // MACHINESETTINGSDIALOG_H
