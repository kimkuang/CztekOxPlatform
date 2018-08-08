#include "GeneralSettingsWidget.h"
#include "ui_GeneralSettingsWidget.h"
#include <QMessageBox>
#include "UNumberLabel.h"

GeneralSettingsWidget::GeneralSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GeneralSettingsWidget)
{
    ui->setupUi(this);
    m_sysSettings = SystemSettings::GetInstance();
    ui->m_spinCameraNumber->setMaximum(m_sysSettings->HardwareChannelCount);
    ui->m_spinDispCntPerLine->setMaximum(m_sysSettings->HardwareChannelCount);

    connect(ui->m_spinCameraNumber, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &GeneralSettingsWidget::spinDispParam_valueChanged);
    connect(ui->m_spinDispCntPerLine, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &GeneralSettingsWidget::spinDispParam_valueChanged);
}

GeneralSettingsWidget::~GeneralSettingsWidget()
{
    m_sysSettings = nullptr;
    delete ui;
}

void GeneralSettingsWidget::LoadSettings()
{
    m_sysSettings->ReadSettings();
    ui->m_spinCameraNumber->setValue(m_sysSettings->CameraNumber);
    ui->m_spinDispCntPerLine->setValue(m_sysSettings->DispCntPerRow);
    ui->m_spinAntiShakeDelay->setValue(m_sysSettings->AntiShakeDelay);
    ui->m_spinAlarmMsgThreshold->setValue(m_sysSettings->AlarmMsgThreshold);
    ui->m_spinStopTestingThreshold->setValue(m_sysSettings->StopTestingThreshold);
}

bool GeneralSettingsWidget::SaveSettings()
{
    m_sysSettings->CameraNumber = ui->m_spinCameraNumber->value();
    m_sysSettings->DispCntPerRow = ui->m_spinDispCntPerLine->value();
    m_sysSettings->DispChnIdxList.clear();
    QGridLayout *gridLayout = qobject_cast<QGridLayout *>(ui->m_frameIndicator->layout());
    if (nullptr != gridLayout) {
        for (int r = 0; r < gridLayout->rowCount(); r++) {
            for (int c = 0; c < gridLayout->columnCount(); c++) {
                QLayoutItem *layoutItem = gridLayout->itemAtPosition(r, c);
                if (layoutItem != nullptr) {
                    UNumberLabel *pLabel = qobject_cast<UNumberLabel *>(layoutItem->widget());
                    if (pLabel != nullptr) {
                        int number = pLabel->GetNumber();
                        if (!m_sysSettings->DispChnIdxList.contains(number)) {
                            m_sysSettings->DispChnIdxList.push_back(number);
                        }
                        else {
                            QMessageBox::critical(this, tr("Error"), tr("Cannot choose the same channel!"));
                            return false;
                        }
                    }
                }
            }
        }
    }
    qDebug()<<"CameraNumber:"<<m_sysSettings->CameraNumber<<"DispCntPerRow:"<<m_sysSettings->DispCntPerRow
           <<"DispCntIdxList:"<<m_sysSettings->DispChnIdxList;
    m_sysSettings->AntiShakeDelay = ui->m_spinAntiShakeDelay->value();
    m_sysSettings->AlarmMsgThreshold = ui->m_spinAlarmMsgThreshold->value();
    m_sysSettings->StopTestingThreshold = ui->m_spinStopTestingThreshold->value();
    m_sysSettings->WriteSettings();
    return true;
}

void GeneralSettingsWidget::showEvent(QShowEvent *event)
{
    (void)event;
    relayoutIndicator(m_sysSettings->HardwareChannelCount, m_sysSettings->CameraNumber, m_sysSettings->DispCntPerRow);
}

void GeneralSettingsWidget::spinDispParam_valueChanged(int n)
{
    (void)n;
    QSpinBox *spinBox = qobject_cast<QSpinBox *>(sender());
    if (spinBox == ui->m_spinCameraNumber) {
        m_sysSettings->CameraNumber = ui->m_spinCameraNumber->value();
        ui->m_spinDispCntPerLine->setMaximum(m_sysSettings->CameraNumber);
    }
    else {
        m_sysSettings->DispCntPerRow = ui->m_spinDispCntPerLine->value();
    }
    qDebug()<<"CameraNumber:"<<m_sysSettings->CameraNumber<<"DispCntPerRow:"<<m_sysSettings->DispCntPerRow;
    relayoutIndicator(m_sysSettings->HardwareChannelCount, m_sysSettings->CameraNumber, m_sysSettings->DispCntPerRow);
}

void GeneralSettingsWidget::relayoutIndicator(int hardwareChannelCnt, int dispCameraCnt, int dispCntPerRow)
{
    int size = ui->m_grpbx1->height();
    ui->m_frameIndicator->setMinimumSize(size, size);
    ui->m_frameIndicator->setMaximumSize(size, size);
    QGridLayout *gridLayout = qobject_cast<QGridLayout *>(ui->m_frameIndicator->layout());
    if (nullptr == gridLayout)
        return;

    for (int i = gridLayout->count() - 1; i >= 0; i--) {
        QLayoutItem *layoutItem = gridLayout->itemAt(i);
        if (layoutItem != nullptr) {
            UNumberLabel *pLabel = qobject_cast<UNumberLabel *>(layoutItem->widget());
            if (pLabel != nullptr) {
                gridLayout->removeItem(layoutItem);
                delete pLabel;
            }
        }
    }

    int rowCnt = Utils::PageCount(dispCameraCnt, dispCntPerRow);
    int idx = 0;
    for (int rowIdx = 0; rowIdx < rowCnt; rowIdx++) {
        for (int colIdx = 0; colIdx < dispCntPerRow; colIdx++) {
            qDebug("rowIdx: %u, colIdx: %u, idx: %u", rowIdx, colIdx, idx);
            int chnIdx = m_sysSettings->DispChnIdxList.size() > idx ? m_sysSettings->DispChnIdxList[idx] : idx;
            UNumberLabel *pLabel = new UNumberLabel(chnIdx, hardwareChannelCnt, this);
            gridLayout->addWidget(pLabel, rowIdx, colIdx, 1, 1);
            idx++;
            if (idx >= dispCameraCnt)
                break;
        }
    }
}
