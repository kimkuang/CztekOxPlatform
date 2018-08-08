#include "SelectSensorDialog.h"
#include "ui_SelectSensorDialog.h"
#include <QMessageBox>
#include <QDebug>
#include "SensorDetailDialog.h"
#include "IVcmDriver.h"
#include "IOtpSensor.h"

SelectSensorDialog::SelectSensorDialog(uint windowIdx, uint chnIdx, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectSensorDialog)
{
    (void)chnIdx;
    ui->setupUi(this);
    setWindowTitle(windowTitle() + QString(" - %1").arg(windowIdx));
//    ui->m_chkUseMasterCamFuseId->setVisible(windowIdx != 0);
//    m_sensorPropertyBrower = new SensorPropertyBrowserWidget(ui->m_framePropertyBrowser);
//    QVBoxLayout *layout = new QVBoxLayout(ui->m_framePropertyBrowser);
//    layout->setMargin(0);
//    layout->addWidget(m_sensorPropertyBrower, 1);
    m_sysSetting = SystemSettings::GetInstance();
    m_moduleSettings = ModuleSettings::GetInstance(m_sysSetting->ProjectName, windowIdx);
    initComboBoxVcmDriverIc();
    intiComboBoxOtpSensor();
    connectSignalSlots();

    m_sysSetting = SystemSettings::GetInstance();
    if (1 == m_sysSetting->CameraNumber)
    {
        ui->m_chkUseMasterCamFuseId->setChecked(false);
    }

    ui->m_chkUseMasterCamFuseId->setEnabled(m_sysSetting->CameraNumber > 1);
    if (UiDefs::Operator_Operator == m_sysSetting->Operator)
    {
        ui->m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
    else
    {
        ui->m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }

    loadLastUsedSensors();
    option2Ui();
    rdoDVP_clicked();
}

SelectSensorDialog::~SelectSensorDialog()
{
    delete ui;
    clearSensors();
    m_moduleSettings = nullptr;
    m_sysSetting = nullptr;
}

void SelectSensorDialog::rdoDVP_clicked()
{
    int interfaceType = getInterfaceType();
    initComboBoxSchemeName(interfaceType);
    QString sensorName = m_moduleSettings->SensorDb.SchemeName;
    qDebug()<<sensorName;
    locateSensor(sensorName, interfaceType);
}

void SelectSensorDialog::cbxSchemeName_currentIndexChanged(const QString &text)
{
    if (text == "")
        return;
//    qDebug()<<text;

    int interfaceType = getInterfaceType();
    locateSensor(text, interfaceType);
}

void SelectSensorDialog::btnAdvancedSearch_clicked()
{
    int interfaceType = getInterfaceType();
    SensorDetailDialog dlg(ui->m_cbxSchemeName->currentText(), interfaceType, this);
    if (dlg.exec() == QDialog::Accepted) {
        Sensor sensor;
        dlg.GetSelectSensor(sensor);
        if (m_sensorCache.contains(sensor.SchemeName)) { // update cache
            delete m_sensorCache[sensor.SchemeName];
            m_sensorCache[sensor.SchemeName] = new Sensor(sensor);
        }
        locateSensor(sensor);
    }
}

void SelectSensorDialog::buttonBox_clicked(QAbstractButton *button)
{
    QDialogButtonBox::StandardButton stdBtn = ui->m_buttonBox->standardButton(button);
    switch (stdBtn) {
    case QDialogButtonBox::Ok: {
        ui2Option();
        QString &schemeName = m_moduleSettings->SensorDb.SchemeName;
        if (schemeName.isEmpty())
            schemeName = UiDefs::NC_SENSOR_NAME;
    //    qDebug()<<schemeName;
        Sensor *sensor = NULL;
        if (schemeName != UiDefs::NC_SENSOR_NAME) {
            sensor = m_sensorCache[schemeName];
            if (sensor == NULL) {
                QMessageBox::critical(this, tr("Error"), tr("Cannot find sensor: %1").arg(schemeName), QMessageBox::Ok);
                return;
            }
//            m_sensorPropertyBrower->GetSensor(*sensor);
    //        sensor->LastUsedTime = QDateTime::currentDateTime().toTime_t();
            if (!m_sensorBll.Update(*sensor)) {
                qCritical()<<"Updating last used time failed.";
                return;
            }
        }
        qDebug()<<schemeName;
        m_moduleSettings->WriteSettings();
        accept();
        break;
    }
    case QDialogButtonBox::Cancel:
        reject();
        break;
    default:
        break;
    }
}

void SelectSensorDialog::initComboBoxVcmDriverIc()
{
    ui->m_cbxVcmDriverName->clear();
    std::vector<std::string> driverIcList;
    ::GetVcmDriverIcList(driverIcList);
    for (uint i = 0; i < driverIcList.size(); i++) {
        ui->m_cbxVcmDriverName->addItem(QString::fromStdString(driverIcList[i]));
    }
}

void SelectSensorDialog::intiComboBoxOtpSensor()
{
    ui->m_cbxOtpSensorName->clear();
    std::list<std::string> otpSensorList;
    ::GetOtpSensorList(otpSensorList);
    for (auto it = otpSensorList.begin(); it != otpSensorList.end(); ++it) {
        ui->m_cbxOtpSensorName->addItem(QString::fromStdString(*it));
    }
}

void SelectSensorDialog::initComboBoxSchemeName(int interfaceType)
{
    ui->m_cbxSchemeName->clear();
    ui->m_cbxSchemeName->addItem(UiDefs::NC_SENSOR_NAME);
    QMutableMapIterator<QString, Sensor *> it(m_sensorCache);
    while (it.hasNext()) {
        it.next();
        Sensor *pSensor = it.value();
        if (pSensor == NULL)
            continue;
//        qDebug()<<it.key()<<pSensor->InterfaceType<<pSensor->SchemeName<<interfaceType;
        if (interfaceType == (int)IT_DVP) {
            if ((pSensor->InterfaceType != interfaceType) && (pSensor->InterfaceType != IT_MTK))
                continue;
        }
        else if (interfaceType == (int)IT_MIPI) {
            if (pSensor->InterfaceType != IT_MIPI)
                continue;
        }
        ui->m_cbxSchemeName->addItem(it.key());
    }
}

void SelectSensorDialog::loadLastUsedSensors()
{
    QList<Sensor> sensors;
    if (!m_sensorBll.SelectListOrderByLastUsedNr(sensors, 10)) {
        qCritical()<<"Loading sensor list failed.";
        QMessageBox::critical(this, tr("Error"), tr("Loading sensor list failed."), QMessageBox::Ok);
        return;
    }
    clearSensors();
    for (int i = 0; i < sensors.count(); i++) {
        qDebug()<<sensors[i].SchemeName<<sensors[i].LastUsedTime<<sensors[i].Description;
        Sensor *pSensor = new Sensor(sensors[i]);
        m_sensorCache.insert(sensors[i].SchemeName, pSensor);
    }
}

void SelectSensorDialog::clearSensors()
{
    QMutableMapIterator<QString, Sensor *> it(m_sensorCache);
    while (it.hasNext()) {
        it.next();
        delete it.value();
//        it.remove();
    }
    m_sensorCache.clear();
}

int SelectSensorDialog::getInterfaceType()
{
    if (ui->m_rdoMipi->isChecked())
        return (int)IT_MIPI;
    else if (ui->m_rdoDVP->isChecked())
        return (int)IT_DVP;
    else
        return -1;
}

void SelectSensorDialog::option2Ui()
{
    ui->m_rdoMipi->setChecked(m_sysSetting->SensorParam.InterfaceType == IT_MIPI);
    ui->m_rdoDVP->setChecked(m_sysSetting->SensorParam.InterfaceType == IT_DVP);

    ui->m_chkUseMasterCamFuseId->setChecked(m_moduleSettings->SensorDb.UseMasterI2c);

    ui->m_grpVcmDriver->setChecked(m_moduleSettings->VcmDriverDb.Enabled);
    ui->m_cbxVcmDriverName->setCurrentText(m_moduleSettings->VcmDriverDb.DriverIcName);
    ui->m_spinVcmDriverI2cAddr->setValue(m_moduleSettings->VcmDriverDb.I2cAddr);

    ui->m_grpOtpSensor->setChecked(m_moduleSettings->OtpSensorDb.Enabled);
    ui->m_cbxOtpSensorName->setCurrentText(m_moduleSettings->OtpSensorDb.OtpSensorName);
    ui->m_spinOtpSensorI2cAddr->setValue(m_moduleSettings->OtpSensorDb.I2cAddr);
}

void SelectSensorDialog::ui2Option()
{
//    if (ui->m_rdoDVP->isChecked())
//        m_sysSetting->SensorParam.InterfaceType = IT_DVP;
//    else
        m_sysSetting->SensorParam.InterfaceType = IT_MIPI;

    m_moduleSettings->SensorDb.SchemeName = ui->m_cbxSchemeName->currentText();
    m_moduleSettings->SensorDb.UseMasterI2c = ui->m_chkUseMasterCamFuseId->isChecked();

    m_moduleSettings->VcmDriverDb.Enabled = ui->m_grpVcmDriver->isChecked();
    m_moduleSettings->VcmDriverDb.DriverIcName = ui->m_cbxVcmDriverName->currentText();
    m_moduleSettings->VcmDriverDb.I2cAddr = ui->m_spinVcmDriverI2cAddr->value();

    m_moduleSettings->OtpSensorDb.Enabled = ui->m_grpOtpSensor->isChecked();
    m_moduleSettings->OtpSensorDb.OtpSensorName = ui->m_cbxOtpSensorName->currentText();
    m_moduleSettings->OtpSensorDb.I2cAddr = ui->m_spinOtpSensorI2cAddr->value();
}

bool SelectSensorDialog::locateSensor(const QString &schemeName, int interfaceType)
{
    if (schemeName == UiDefs::NC_SENSOR_NAME) {
//        qDebug()<<"Scheme name is: "<<UiDefs::NC_SENSOR_NAME;
        return false;
    }

    Sensor *pSensor = NULL;
    if (m_sensorCache.contains(schemeName)) {
        pSensor = m_sensorCache[schemeName];
    }
    else {
        pSensor = new Sensor();
        pSensor->SchemeName = schemeName;
        if (!m_sensorBll.SelectBySchemeName(*pSensor)) {
            qCritical()<<QString("Executing SelectBySchemeName[%1] failed.").arg(schemeName);
            return false;
        }
        m_sensorCache.insert(schemeName, pSensor);
    }

    if (pSensor == NULL) {
        qDebug()<<"Sensor is null.";
        return false;
    }
    if (interfaceType == (int)IT_DVP) {
        if ((pSensor->InterfaceType != interfaceType) && (pSensor->InterfaceType != IT_MTK))
            return false;
    }
    else if (interfaceType == (int)IT_MIPI) {
        if (pSensor->InterfaceType != (int)IT_MIPI)
            return false;
    }
    locateSensor(*pSensor);
    return true;
}

void SelectSensorDialog::locateSensor(const Sensor &sensor)
{
//    qDebug()<<sensor.SchemeName;
    if (!m_sensorCache.contains(sensor.SchemeName))
        m_sensorCache.insert(sensor.SchemeName, new Sensor(sensor));
    int idx = ui->m_cbxSchemeName->findText(sensor.SchemeName);
    if (idx == -1)
        ui->m_cbxSchemeName->addItem(sensor.SchemeName);
    ui->m_cbxSchemeName->setCurrentText(sensor.SchemeName);
//    m_sensorPropertyBrower->SetSensor(sensor, false);
}

void SelectSensorDialog::connectSignalSlots()
{
    connect(ui->m_rdoMipi, &QRadioButton::clicked, this, &SelectSensorDialog::rdoDVP_clicked);
    connect(ui->m_rdoDVP, &QRadioButton::clicked, this, &SelectSensorDialog::rdoDVP_clicked);
    connect(ui->m_cbxSchemeName, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &SelectSensorDialog::cbxSchemeName_currentIndexChanged);
    connect(ui->m_btnAdvancedSearch, &QPushButton::clicked, this, &SelectSensorDialog::btnAdvancedSearch_clicked);
    connect(ui->m_buttonBox, &QDialogButtonBox::clicked, this, &SelectSensorDialog::buttonBox_clicked);
}
