#include "I2cDebugDialog.h"
#include "ui_I2cDebugDialog.h"
#include <QString>
#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include <QProcess>
#include "IImageSensor.h"
#include "IOtpSensor.h"
#include "IVcmDriver.h"
#include "conf/SystemSettings.h"

I2cDebugDialog::I2cDebugDialog(uint windowIdx, uint chnIdx, T_ChannelContext *context, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::I2cDebugDialog)
{
    ui->setupUi(this);
    setWindowTitle(windowTitle() + QString(" - %1").arg(windowIdx));

    m_chnIdx = chnIdx;
    m_pContext = context;
    m_pModuleSettings = static_cast<ModuleSettings *>(m_pContext->ModuleSettings);
    UpdateUI();
    Connect();
}

I2cDebugDialog::~I2cDebugDialog()
{
}

void I2cDebugDialog::OnBtnReadI2c(void)
{
    Q_ASSERT(m_pContext != nullptr);
    uint i2cAddr = (uint)ui->comboBoxI2cAddr->currentText().toInt(NULL, 16);
    ushort regAddr = (ushort)ui->lineEditRegAddr->text().toInt(NULL, 16);

    ushort regData = 0;
#if 0
    int mode = ui->comboBoxI2cMode->currentIndex();
    int iRet = m_pContext->ChannelController->ReadDiscreteI2c(i2cAddr, 100, mode, &regAddr, &regData, 1);
#else
    uint addrSize = 1;
    uint dataSize = 1;
    int mode = ui->comboBoxI2cMode->currentIndex();
    if ((REG8DATA16 == mode) || (REG16DATA16 == mode))
    {
        dataSize = 2;
    }

    if ((REG16DATA8 == mode) || (REG16DATA16 == mode))
    {
        addrSize = 2;
    }

    int iRet = m_pContext->ChannelController->ReadContinuousI2c(i2cAddr, 100, regAddr, addrSize, (unsigned char*)(&regData), dataSize);
#endif
    if (ERR_NoError != iRet)
    {
        ui->labelErrorInfo->setStyleSheet(QString("color:red"));
        ui->labelErrorInfo->setText(QString("Read I2c Fail"));
        return;
    }
    else
    {
        ui->labelErrorInfo->setStyleSheet(QString("color:green"));
        ui->labelErrorInfo->setText(QString("Read I2c Success"));
    }

    QString strVal = "";
    if ((REG8DATA16 == mode) || (REG16DATA16 == mode))
    {
        strVal.sprintf("0x%04x", regData);
    }
    else
    {
        strVal.sprintf("0x%02x", regData);
    }

    ui->lineEditRegData->setText(strVal);
}

void I2cDebugDialog::OnBtnWriteI2c(void)
{
    Q_ASSERT(m_pContext != nullptr);
    uint i2cAddr = (uint)ui->comboBoxI2cAddr->currentText().toInt(NULL, 16);
    ushort regAddr = (ushort)ui->lineEditRegAddr->text().toInt(NULL, 16);
    ushort regData = (ushort)ui->lineEditRegData->text().toInt(NULL, 16);
    uint mode = (uint)ui->comboBoxI2cMode->currentIndex();
    int iRet = m_pContext->ChannelController->WriteDiscreteI2c(i2cAddr, 100, mode, &regAddr, &regData, 1);
    if (ERR_NoError != iRet)
    {
        ui->labelErrorInfo->setStyleSheet(QString("color:red"));
        ui->labelErrorInfo->setText(QString("Writ I2c Fail"));
    }
    else
    {
        ui->labelErrorInfo->setStyleSheet(QString("color:green"));
        ui->labelErrorInfo->setText(QString("Write I2c Success"));
    }
}

void I2cDebugDialog::OnBtnSaveAEAndGain(void)
{

}

void I2cDebugDialog::OnBtnSetCode(void)
{
    Q_ASSERT(m_pContext != nullptr);
    uint valCode = (uint)ui->spinBoxVcmCode->value();
    if ((m_pContext->VcmDriver != nullptr) && (ERR_NoError == m_pContext->VcmDriver->VcmWriteCode(valCode)))
    {
        ui->labelCurrentCode->setText(QString("Current Code: %1").arg(valCode));
    }
}

void I2cDebugDialog::OnBtnFindI2cAddr()
{
    Q_ASSERT(m_pContext != nullptr);
    ui->comboBoxI2cAddr->clear();
    ushort regAddr = 0x00;
    ushort regData = 0x00;
    QString strVal = "";
    bool bFind = false;
    ui->labelErrorInfo->setStyleSheet(QString("color:blue"));
    for (uint i2cAddr=0x02; i2cAddr<0xff; i2cAddr+=2)
    {
        if (ERR_NoError == m_pContext->ChannelController->ReadDiscreteI2c(i2cAddr, 100, NORMAL, &regAddr, &regData, 1))
        {
            strVal.sprintf("0x%02x", i2cAddr);
            ui->comboBoxI2cAddr->addItem(strVal);
            ui->labelErrorInfo->setText(strVal);
            bFind = true;
        }
    }

    if (true == bFind)
    {
        ui->labelErrorInfo->setStyleSheet(QString("color:green"));
        ui->labelErrorInfo->setText(QString("Find Ok"));
    }
    else
    {
        ui->labelErrorInfo->setStyleSheet(QString("color:red"));
        ui->labelErrorInfo->setText(QString("No Find"));
    }

    ui->comboBoxI2cAddr->setCurrentIndex(0);
}

void I2cDebugDialog::OnBtnGetFuseID()
{
    QString strFuseID = m_pContext->ChannelController->GetSensorFuseId();
    if (!strFuseID.isEmpty())
    {
        ui->lineEditFuseID->setText(strFuseID);
    }
    else
    {
        ui->lineEditFuseID->setText(QString("Get Fuse ID Failed"));
    }
}

void I2cDebugDialog::OnBtnDump()
{
    uint startPage = ui->lineEditStartPage->text().toInt();
    uint endPage = ui->lineEditEndPage->text().toInt();
    uint startAddr = ui->lineEditStartAddr->text().toInt(NULL, 16);
    uint endAddr = ui->lineEditEndAddr->text().toInt(NULL, 16);
    uint uSize = endAddr - startAddr + 1;
    QString strFilePath = "";
    strFilePath.sprintf("%d_Readback_Value.ini", m_chnIdx);
    QFile MyFile(strFilePath);
    MyFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    uchar* pReadData = new uchar[uSize];
    if (NULL == pReadData)
    {
        return;
    }

    for (uint page=startPage; page<=endPage; ++page)
    {
        if ((m_pContext->OtpSensor != nullptr) && (ERR_NoError != m_pContext->OtpSensor->OtpRead(startAddr, endAddr, pReadData, page)))
        {
            delete[] pReadData;
            QMessageBox::information(this, QString("I2cDebug"), QString("Dump Error"), QMessageBox::Ok);
            return;
        }

        QString strTmpInfo = "";
        if (startPage != endPage)
        {
            strTmpInfo.sprintf("Page: %d\n", page);
            MyFile.write(strTmpInfo.toLatin1().data());
        }

        for (ushort uAddr=startAddr; uAddr<=endAddr; ++uAddr)
        {
            strTmpInfo.sprintf("0x%04X\t0x%02X\n", uAddr, pReadData[uAddr - startAddr]);
            MyFile.write(strTmpInfo.toLatin1().data());
        }
    }

    MyFile.close();
    delete[] pReadData;
    pReadData = NULL;
    QString strCmd = "";
    strCmd.sprintf("gedit \"%s\"", strFilePath.toLatin1().data());
    //QProcess::execute(strCmd);
    QProcess::startDetached(strCmd);
}

void I2cDebugDialog::OnSliderAEValChange(void)
{
    uint valAE = (uint)ui->horizontalSliderAE->value();
    if ((m_pContext->ImageSensor != nullptr) && (ERR_NoError == m_pContext->ImageSensor->SetSensorExposure(valAE)))
    {
        QString strVal = "";
        strVal.sprintf("0x%04x", valAE);
        ui->lineEditAEVal->setText(strVal);
    }
}

void I2cDebugDialog::OnSliderGainValChange(void)
{
    uint valGain = (uint)ui->horizontalSliderGain->value();
    if ((m_pContext->ImageSensor != nullptr) && (ERR_NoError == m_pContext->ImageSensor->SetSensorGain(valGain)))
    {
        QString strVal = "";
        strVal.sprintf("0x%04x", valGain);
        ui->lineEditGainVal->setText(strVal);
    }
}

void I2cDebugDialog::OnEditAEValChange(void)
{
    uint valAE = (uint)ui->lineEditAEVal->text().toInt(NULL, 16);
    if ((m_pContext->ImageSensor != nullptr) && (ERR_NoError == m_pContext->ImageSensor->SetSensorExposure(valAE)))
    {
        ui->horizontalSliderAE->setValue(valAE);
    }
}

void I2cDebugDialog::OnEditGainValChange(void)
{
    uint valGain = (uint)ui->lineEditGainVal->text().toInt(NULL, 16);
    if ((m_pContext->ImageSensor != nullptr) && (ERR_NoError == m_pContext->ImageSensor->SetSensorGain(valGain)))
    {
        ui->horizontalSliderGain->setValue(valGain);
    }
}

void I2cDebugDialog::OnComboIndexChange(int index)
{
    int mode = index;
    ushort regAddr = (ushort)ui->lineEditRegAddr->text().toInt(NULL, 16);
    ushort regData = (ushort)ui->lineEditRegData->text().toInt(NULL, 16);
    QString strVal = "";
    if ((NORMAL == mode) || (REG8DATA8 == mode) || (REG8DATA16 == mode))
    {
        strVal.sprintf("0x%02x", regAddr&0xFF);
    }
    else
    {
        strVal.sprintf("0x%04x", regAddr);
    }

    ui->lineEditRegAddr->setText(strVal);
    if ((NORMAL == mode) || (REG8DATA8 == mode) || (REG16DATA8 == mode))
    {
        strVal.sprintf("0x%02x", regData&0xFF);
    }
    else
    {
        strVal.sprintf("0x%04x", regData);
    }

    ui->lineEditRegData->setText(strVal);
}

void I2cDebugDialog::UpdateUI()
{
    Q_ASSERT(m_pContext);
    OnBtnFindI2cAddr();
    ui->lineEditRegAddr->setText(QString("0x00"));
    ui->lineEditRegData->setText(QString("0x00"));

    ui->comboBoxI2cMode->addItem(QString("0:Normal"));
    ui->comboBoxI2cMode->addItem(QString("1:Reg8Data8"));
    ui->comboBoxI2cMode->addItem(QString("2:Reg8Data16"));
    ui->comboBoxI2cMode->addItem(QString("3:Reg16Data8"));
    ui->comboBoxI2cMode->addItem(QString("4:Reg16Data16"));
    ui->comboBoxI2cMode->setCurrentIndex(0);
    ui->labelErrorInfo->setText(QString(""));
    ui->labelErrorInfo->setStyleSheet(QString("color::red"));

    uint valAE = 0, valGain = 0;
    if (m_pContext->ImageSensor != nullptr) {
        m_pContext->ImageSensor->GetSensorExposure(valAE);
        m_pContext->ImageSensor->GetSensorGain(valGain);
    }
    ui->horizontalSliderAE->setValue(valAE);
    ui->horizontalSliderGain->setValue(valGain);
    QString strVal = "";
    strVal.sprintf("0x%04x", valAE);
    ui->lineEditAEVal->setText(strVal);
    strVal.sprintf("0x%04x", valGain);
    ui->lineEditGainVal->setText(strVal);
    ui->lineEditBrigth->setText(QString("0"));

    ui->spinBoxVcmCode->setValue(0);
    uint valCode = 0;
    if (m_pContext->VcmDriver != nullptr)
        m_pContext->VcmDriver->VcmReadCode(valCode);
    ui->labelCurrentCode->setText(QString("Current Code:%1").arg(valCode));
    ui->lineEditStartPage->setText(QString("0"));
    ui->lineEditEndPage->setText(QString("0"));
    ui->lineEditStartAddr->setText(QString("0x0000"));
    ui->lineEditEndAddr->setText(QString("0x1FFF"));
}

void I2cDebugDialog::Connect()
{
    connect(ui->pushButtonRead, &QPushButton::clicked, this, &I2cDebugDialog::OnBtnReadI2c);
    connect(ui->pushButtonWrite, &QPushButton::clicked, this, &I2cDebugDialog::OnBtnWriteI2c);
    connect(ui->pushButtonSaveAEAndGain, &QPushButton::clicked, this, &I2cDebugDialog::OnBtnSaveAEAndGain);
    connect(ui->pushButtonSetCode, &QPushButton::clicked, this, &I2cDebugDialog::OnBtnSetCode);
    connect(ui->pushButtonFindI2cAddr, &QPushButton::clicked, this, &I2cDebugDialog::OnBtnFindI2cAddr);
    connect(ui->pushButtonGetFuseID, &QPushButton::clicked, this, &I2cDebugDialog::OnBtnGetFuseID);
    connect(ui->pushButtonDump, &QPushButton::clicked, this, &I2cDebugDialog::OnBtnDump);
    connect(ui->lineEditAEVal, &QLineEdit::textChanged, this, &I2cDebugDialog::OnEditAEValChange);
    connect(ui->lineEditGainVal, &QLineEdit::textChanged, this, &I2cDebugDialog::OnEditGainValChange);
    connect(ui->horizontalSliderAE, &QSlider::valueChanged, this, &I2cDebugDialog::OnSliderAEValChange);
    connect(ui->horizontalSliderGain, &QSlider::valueChanged, this, &I2cDebugDialog::OnSliderGainValChange);
    connect(ui->comboBoxI2cMode, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &I2cDebugDialog::OnComboIndexChange);
}
