#ifndef DIALOGPASSWORD_H
#define DIALOGPASSWORD_H

#include <QDialog>
#include "dal/sqldatabaseutil.h"

namespace Ui {
class PasswordDialog;
}

class PasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PasswordDialog(QWidget *parent = 0);
    ~PasswordDialog();

private slots:
    void btnEditPwd_clicked();
    void btnOK_clicked();
    void btnCancel_clicked();

private:
    Ui::PasswordDialog *ui;
    bool m_isEditPwdMode;
    QSqlDatabase m_systemDb;

    void connectSignalSlots();
    bool checkInputPwd();
    void editPasswordMode(bool isEditMode);
};

#endif // DIALOGPASSWORD_H
