#include "EditSensorDialog.h"
#include "ui_EditSensorDialog.h"
#include <QMessageBox>
#include <QDebug>

DialogEditSensor::DialogEditSensor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEditSensor)
{
    ui->setupUi(this);

    m_sensorBll = new SensorBLL();
    initComboBoxData();
    initBoolFrame();
    connect(ui->m_btnOK, &QPushButton::clicked, this, &DialogEditSensor::btnOK_clicked);
    connect(ui->m_btnCancel, &QPushButton::clicked, this, &DialogEditSensor::btnCancel_clicked);

    m_schemeName = "";
}

DialogEditSensor::~DialogEditSensor()
{
    delete m_sensorBll;
    delete ui;
}

void DialogEditSensor::btnOK_clicked()
{
    Sensor sensor;
    sensor.Id = m_id;
    if (!m_sensorBll->SelectById(sensor)) {
        QMessageBox::critical(this, tr("Error"), QString(tr("Cannot find the sensor which id is %1!")).arg(m_id), QMessageBox::Ok);
        return;
    }
    bool ok;
    sensor.SchemeName = ui->m_edtSchemeName->text();
    sensor.InterfaceType = (E_InterfaceType)ui->m_cbxInterface->currentData().toInt();
    sensor.MclkDef = sensor.Mclk = ui->m_cbxMclk->currentText().toFloat(&ok);
    if (!ok) {
        QMessageBox::critical(this, tr("Error"), tr("Mclk is error!"), QMessageBox::Ok);
        return;
    }
    sensor.Lanes = ui->m_spinMipiLanes->value();
    sensor.MipiFreqDef = sensor.MipiFreq = ui->m_cbxMipiFreq->currentText().toInt(&ok);
    if (!ok) {
        QMessageBox::critical(this, tr("Error"), tr("Mipi frequence is error!"), QMessageBox::Ok);
        return;
    }
    sensor.ImageFormat = (E_ImageFormat)(ui->m_cbxImageFormat->currentData().toInt());
    sensor.ImageModeDef = sensor.ImageMode = (E_ImageMode)ui->m_cbxImageMode->currentData().toInt();
    sensor.PixelWidth = ui->m_edtPixelWidth->text().toInt(&ok);
    if (!ok) {
        QMessageBox::critical(this, tr("Error"), tr("Pixel width is error!"), QMessageBox::Ok);
        return;
    }
    sensor.PixelHeight = ui->m_edtPixelHeight->text().toInt(&ok);
    if (!ok) {
        QMessageBox::critical(this, tr("Error"), tr("Pixel height is error!"), QMessageBox::Ok);
        return;
    }
    sensor.PclkPolDef = sensor.PclkPol = ui->m_grpPclkPol->value() ? 1 : 0;
    sensor.DataPolDef = sensor.DataPol = ui->m_grpDataPol->value() ? 1 : 0;
    sensor.HsyncPolDef = sensor.HsyncPol = ui->m_grpHsyncPol->value() ? 1 : 0;
    sensor.VsyncPolDef = sensor.VsyncPol = ui->m_grpVsyncPol->value() ? 1 : 0;
    sensor.PwdnDef = sensor.Pwdn = ui->m_grpPwdn->value() ? 1 : 0;
    sensor.ResetDef = sensor.Reset = ui->m_grpReset->value() ? 1 : 0;

    sensor.FullModeParams = ui->m_edtFullModeRegisters->toPlainText();
    sensor.OtpInitParams = ui->m_edtOtpInitRegisters->toPlainText();
    sensor.OtpAlgParams = ui->m_edtOtpAlgParams->toPlainText();
    sensor.ExposureParams = ui->m_edtExposureRegisters->toPlainText();
    sensor.GainParams = ui->m_edtGainRegisters->toPlainText();
//    qDebug()<<sensor.FullModeParams;
    if (m_sensorBll->Update(sensor)) {
        m_schemeName = sensor.SchemeName;
        accept();
//        QMessageBox::information(this, tr("Information"), tr("Submit data succeed."), QMessageBox::Ok);
    }
    else {
        QMessageBox::critical(this, tr("Error"), tr("Submit data failed."), QMessageBox::Ok);
    }
}

void DialogEditSensor::btnCancel_clicked()
{
    reject();
}

void DialogEditSensor::initComboBoxData()
{
    ui->m_cbxInterface->clear();
    ui->m_cbxInterface->addItem("MIPI", (int)IT_MIPI);
    ui->m_cbxInterface->addItem(tr("DVP"), (int)IT_DVP);
    ui->m_cbxInterface->addItem("MTK", (int)IT_MTK);
    ui->m_cbxInterface->addItem("SPI", (int)IT_SPI);
    ui->m_cbxInterface->addItem("HIPI", (int)IT_HIPI);
    ui->m_cbxInterface->addItem(tr("SPREADTRUM"), (int)IT_SPREADTRUM);
    ui->m_cbxInterface->addItem("TV", (int)IT_TV);
    ui->m_cbxInterface->addItem("UVC", (int)IT_UVC);
    ui->m_cbxInterface->addItem("LVDS", (int)IT_LVDS);

    ui->m_cbxMclk->clear();
    ui->m_cbxMclk->addItems(QStringList()<<"48.0"<<"36.0"<<"24.0"<<"18.0"<<"12.0"<<"8.0"<<"6.0");

    ui->m_cbxMipiFreq->clear();
    ui->m_cbxMipiFreq->addItems(QStringList()<<"800"<<"700"<<"600"<<"500"<<"400");

    ui->m_cbxImageFormat->clear();
    ui->m_cbxImageFormat->addItem("RAW8", (int)IMAGE_FMT_RAW8);
    ui->m_cbxImageFormat->addItem("RAW10", (int)IMAGE_FMT_RAW10);
    ui->m_cbxImageFormat->addItem("RAW12", (int)IMAGE_FMT_RAW12);
    ui->m_cbxImageFormat->addItem("RAW14", (int)IMAGE_FMT_RAW14);
    ui->m_cbxImageFormat->addItem("RGB16", (int)IMAGE_FMT_RGB16);
    ui->m_cbxImageFormat->addItem("RGB24", (int)IMAGE_FMT_RGB24);
    ui->m_cbxImageFormat->addItem("RGB32", (int)IMAGE_FMT_RGB32);
    ui->m_cbxImageFormat->addItem("YUV422", (int)IMAGE_FMT_YUV422);

    ui->m_cbxImageMode->clear();
    ui->m_cbxImageMode->addItem("YUYV/RGGB", (int)IMAGE_MODE_YCbYCr_RG_GB);
    ui->m_cbxImageMode->addItem("YVYU/GRBG", (int)IMAGE_MODE_YCrYCb_GR_BG);
    ui->m_cbxImageMode->addItem("UYVY/GBRG", (int)IMAGE_MODE_CbYCrY_GB_RG);
    ui->m_cbxImageMode->addItem("VYUY/BGGR", (int)IMAGE_MODE_CrYCbY_BG_GR);
}

void DialogEditSensor::initBoolFrame()
{
    ui->m_grpPclkPol->setTrueText(tr("Positive"));
    ui->m_grpPclkPol->setFalseText(tr("Negative"));
    ui->m_grpDataPol->setTrueText(tr("Positive"));
    ui->m_grpDataPol->setFalseText(tr("Negative"));
    ui->m_grpHsyncPol->setTrueText(tr("Positive"));
    ui->m_grpHsyncPol->setFalseText(tr("Negative"));
    ui->m_grpVsyncPol->setTrueText(tr("Positive"));
    ui->m_grpVsyncPol->setFalseText(tr("Negative"));
    ui->m_grpPwdn->setTrueText(tr("High"));
    ui->m_grpPwdn->setFalseText(tr("Low"));
    ui->m_grpReset->setTrueText(tr("High"));
    ui->m_grpReset->setFalseText(tr("Low"));
}

void DialogEditSensor::bindData(int id)
{
    Sensor sensor;
    sensor.Id = id;
    if (m_sensorBll->SelectById(sensor)) {
        ui->m_edtSchemeName->setText(sensor.SchemeName);

        int idx = ui->m_cbxInterface->findData((int)sensor.InterfaceType);
        ui->m_cbxInterface->setCurrentIndex(idx);

        ui->m_cbxMclk->setCurrentText(QString::number(sensor.Mclk, 'f', 1));
        ui->m_spinMipiLanes->setValue(sensor.Lanes);
        ui->m_cbxMipiFreq->setCurrentText(QString::number(sensor.MipiFreq));

        idx = ui->m_cbxImageFormat->findData((int)sensor.ImageFormat);
        ui->m_cbxImageFormat->setCurrentIndex(idx);

        idx = ui->m_cbxImageMode->findData((int)sensor.ImageMode);
        ui->m_cbxImageMode->setCurrentIndex(idx);

        ui->m_edtPixelWidth->setText(QString::number(sensor.PixelWidth));
        ui->m_edtPixelHeight->setText(QString::number(sensor.PixelHeight));

        ui->m_grpPclkPol->setValue(sensor.PclkPol);
        ui->m_grpDataPol->setValue(sensor.DataPol);
        ui->m_grpHsyncPol->setValue(sensor.HsyncPol);
        ui->m_grpVsyncPol->setValue(sensor.VsyncPol);
        ui->m_grpPwdn->setValue(sensor.Pwdn);
        ui->m_grpReset->setValue(sensor.Reset);

        ui->m_edtFullModeRegisters->setPlainText(sensor.FullModeParams);
        ui->m_edtOtpInitRegisters->setPlainText(sensor.OtpInitParams);
        ui->m_edtOtpAlgParams->setPlainText(sensor.OtpAlgParams);
        ui->m_edtExposureRegisters->setPlainText(sensor.ExposureParams);
        ui->m_edtGainRegisters->setPlainText(sensor.GainParams);
    }
}
