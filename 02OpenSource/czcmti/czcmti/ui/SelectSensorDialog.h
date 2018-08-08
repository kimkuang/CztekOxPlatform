#ifndef SELECTSENSORDIALOG_H
#define SELECTSENSORDIALOG_H

#include <QDialog>
#include <QAbstractButton>
//#include "SensorPropertyBrowserWidget.h"
#include "conf/SystemSettings.h"
#include "conf/ModuleSettings.h"
#include "bll/sensorbll.h"

namespace Ui {
class SelectSensorDialog;
}

class SelectSensorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectSensorDialog(uint windowIdx, uint chnIdx, QWidget *parent = 0);
    ~SelectSensorDialog();

private slots:
    void rdoDVP_clicked();
    void cbxSchemeName_currentIndexChanged(const QString &text);
    void btnAdvancedSearch_clicked();
    void buttonBox_clicked(QAbstractButton *button);

private:
    Ui::SelectSensorDialog *ui;
//    SensorPropertyBrowserWidget *m_sensorPropertyBrower;
    SystemSettings *m_sysSetting;
    ModuleSettings *m_moduleSettings;
    SensorBLL m_sensorBll;
    QMap<QString, Sensor *> m_sensorCache;

    void initComboBoxVcmDriverIc();
    void intiComboBoxOtpSensor();
    void initComboBoxSchemeName(int interfaceType);
    void loadLastUsedSensors();
    void clearSensors();
    int getInterfaceType();
    void option2Ui();
    void ui2Option();
    bool locateSensor(const QString &schemeName, int interfaceType);
    void locateSensor(const Sensor &sensor);
    void connectSignalSlots();
};

#endif // SELECTSENSORDIALOG_H
