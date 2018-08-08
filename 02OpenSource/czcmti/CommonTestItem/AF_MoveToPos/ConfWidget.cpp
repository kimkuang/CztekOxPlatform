#include "ConfWidget.h"
#include "ui_ConfWidget.h"
#include <QMessageBox>
#include <QDebug>
#include <QPushButton>

ConfWidget::ConfWidget(Option *option, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfWidget),
    m_option(option)
{
    ui->setupUi(this);
    m_btnGroup1.addButton(ui->checkBoxMoveOtherPos);
    m_btnGroup1.addButton(ui->checkBoxMoveInfiPos);
    m_btnGroup1.addButton(ui->checkBoxMoveMacroPos);
    m_btnGroup1.addButton(ui->checkBoxMoveMiddlePos);
    m_btnGroup1.addButton(ui->checkBoxMoveOtherCode);
}

ConfWidget::~ConfWidget()
{
    delete ui;
    m_option = nullptr;
}

int ConfWidget::RestoreDefaults()
{
    m_option->RestoreDefaults();
    Cache2Ui();
    return ERR_NoError;
}

int ConfWidget::Cache2Ui()
{
    m_option->m_bSaveAfCode = ui->checkBoxSaveAFCode->isChecked();
    if (Option::Move_OtherPos == m_option->m_MoveToPosMethod)
        ui->checkBoxMoveOtherPos->setChecked(true);
    else if (Option::Move_InfinityPos == m_option->m_MoveToPosMethod)
        ui->checkBoxMoveInfiPos->setChecked(true);
    else if (Option::Move_MacroPos == m_option->m_MoveToPosMethod)
        ui->checkBoxMoveMacroPos->setChecked(true);
    else if (Option::Move_MiddlePos == m_option->m_MoveToPosMethod)
        ui->checkBoxMoveMiddlePos->setChecked(true);
    else if (Option::Move_OtherCode == m_option->m_MoveToPosMethod)
        ui->checkBoxMoveOtherCode->setChecked(true);

    QString strValue = "";
    strValue.sprintf("%0.2f", m_option->m_fEFL);
    ui->lineEditModuleEFL->setText(strValue);
    strValue.sprintf("%0.2f", m_option->m_fInifiDistance);
    ui->lineEditInfinityDistance->setText(strValue);
    strValue.sprintf("0x%04x", m_option->m_uInifiAddrH);
    ui->lineEditInfiAddrH->setText(strValue);
    strValue.sprintf("0x%04x", m_option->m_uInifiAddrL);
    ui->lineEditInfiAddrL->setText(strValue);
    strValue.sprintf("%0.2f", m_option->m_fMacroDistance);
    ui->lineEditMacroDistance->setText(strValue);
    strValue.sprintf("0x%04x", m_option->m_uMacroAddrH);
    ui->lineEditMacroAddrH->setText(strValue);
    strValue.sprintf("0x%04x", m_option->m_uMacroAddrL);
    ui->lineEditMacroAddrL->setText(strValue);
    strValue.sprintf("%d", m_option->m_uOtherCode);
    ui->lineEditOtherCode->setText(strValue);
    strValue.sprintf("%d", m_option->m_uVcmDelay);
    ui->lineEditVCMDelay->setText(strValue);
    strValue.sprintf("%0.2F", m_option->m_fMoveDistance);
    ui->lineEditMovePos->setText(strValue);

    return ERR_NoError;
}

int ConfWidget::Ui2Cache()
{
    m_option->m_bSaveAfCode = ui->checkBoxSaveAFCode->isChecked();
    if (ui->checkBoxMoveOtherPos->isChecked())
        m_option->m_MoveToPosMethod = Option::Move_OtherPos;
    else if (ui->checkBoxMoveInfiPos->isChecked())
        m_option->m_MoveToPosMethod = Option::Move_InfinityPos;
    else if (ui->checkBoxMoveMiddlePos->isChecked())
        m_option->m_MoveToPosMethod = Option::Move_MiddlePos;
    else if (ui->checkBoxMoveMacroPos->isChecked())
        m_option->m_MoveToPosMethod = Option::Move_MacroPos;
    else if (ui->checkBoxMoveOtherCode->isChecked())
        m_option->m_MoveToPosMethod = Option::Move_OtherCode;

    m_option->m_fEFL = ui->lineEditModuleEFL->text().toFloat();
    m_option->m_fInifiDistance = ui->lineEditInfinityDistance->text().toFloat();
    m_option->m_uInifiAddrH = ui->lineEditInfiAddrH->text().toInt(NULL, 16);
    m_option->m_uInifiAddrL = ui->lineEditInfiAddrL->text().toInt(NULL, 16);
    m_option->m_fMacroDistance = ui->lineEditMacroDistance->text().toFloat();
    m_option->m_uMacroAddrH = ui->lineEditMacroAddrH->text().toInt(NULL, 16);
    m_option->m_uMacroAddrL = ui->lineEditMacroAddrL->text().toInt(NULL, 16);
    m_option->m_uOtherCode = ui->lineEditOtherCode->text().toInt();
    m_option->m_uVcmDelay = ui->lineEditVCMDelay->text().toInt();
    m_option->m_fMoveDistance = ui->lineEditMovePos->text().toFloat();

    return ERR_NoError;
}

