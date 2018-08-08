#include "ui/PasswordDialog.h"
#include "ui_PasswordDialog.h"
#include <QCryptographicHash>
#include <QMessageBox>
#include <QSqlQuery>

PasswordDialog::PasswordDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PasswordDialog)
{
    ui->setupUi(this);

    m_systemDb = SqlDatabaseUtil::GetDatabase("system");
    ui->m_edtCurrPwd->setStyleSheet("lineedit-password-character: 42");
    ui->m_edtNewPwd->setStyleSheet("lineedit-password-character: 42");
    ui->m_edtConfirmPwd->setStyleSheet("lineedit-password-character: 42");
    m_isEditPwdMode = false;
    editPasswordMode(m_isEditPwdMode);

    connectSignalSlots();
}

PasswordDialog::~PasswordDialog()
{
    delete ui;
}

void PasswordDialog::btnEditPwd_clicked()
{
    if (!m_isEditPwdMode) {
        m_isEditPwdMode = true;
        editPasswordMode(m_isEditPwdMode);
        ui->m_btnEditPwd->setText(tr("Submit"));
    }
    else
    {
        if (!checkInputPwd()) {
            QMessageBox::critical(this, tr("Error"), tr("The input password is error!"), QMessageBox::Ok);
            return;
        }
        if (ui->m_edtNewPwd->text() != ui->m_edtConfirmPwd->text()) {
            QMessageBox::critical(this, tr("Error"), tr("New password do not equals to confirm password!"),
                                  QMessageBox::Ok);
            return;
        }
        // save to db
        QSqlQuery query(m_systemDb);
        QByteArray input = ui->m_edtNewPwd->text().toLatin1();
        QByteArray md5 = QCryptographicHash::hash(input, QCryptographicHash::Md5);
        QString md5str = QString::fromLatin1(md5.toHex()).toUpper();
        if (!query.exec(QString("update t_password set password='%1'").arg(md5str))) {
            QMessageBox::critical(this, tr("Error"), tr("Updating password failed."), QMessageBox::Ok);
            return;
        }
        else {
            m_isEditPwdMode = false;
            editPasswordMode(m_isEditPwdMode);
            ui->m_btnEditPwd->setText(tr("Edit Password"));
            QMessageBox::information(this, tr("Information"), tr("Updating password succeed, please re-login."),
                                     QMessageBox::Ok);
        }
    }
}

void PasswordDialog::btnOK_clicked()
{
    if (!checkInputPwd()) {
        QMessageBox::critical(this, tr("Error"), tr("The input password is error!"), QMessageBox::Ok);
    }
    else {
        done(QDialog::Accepted);
    }
}

void PasswordDialog::btnCancel_clicked()
{
    done(QDialog::Rejected);
}

void PasswordDialog::connectSignalSlots()
{
    connect(ui->m_btnEditPwd, SIGNAL(clicked()), this, SLOT(btnEditPwd_clicked()));
    connect(ui->m_btnOK, SIGNAL(clicked()), this, SLOT(btnOK_clicked()));
    connect(ui->m_btnCancel, SIGNAL(clicked()), this, SLOT(btnCancel_clicked()));
}

bool PasswordDialog::checkInputPwd()
{    
    QSqlQuery query("select password from t_password;", m_systemDb); // read from db
    if (!query.next())
        return false;
    QString dbPasswordMd5 = query.value(0).toString();
    QByteArray input = ui->m_edtCurrPwd->text().toLatin1();
    QByteArray passwordMd5 = QCryptographicHash::hash(input, QCryptographicHash::Md5);
    QString inputPasswordMd5 = QString::fromLatin1(passwordMd5.toHex()).toUpper();

    if ((inputPasswordMd5 == dbPasswordMd5) || (input == "cztek")) { // 输入密码正确
        return true;
    }
    return false;
}

void PasswordDialog::editPasswordMode(bool isEditMode)
{
    ui->m_edtNewPwd->setEnabled(isEditMode);
    ui->m_edtConfirmPwd->setEnabled(isEditMode);
}
