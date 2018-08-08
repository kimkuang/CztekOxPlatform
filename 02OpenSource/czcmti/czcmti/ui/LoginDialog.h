#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QAbstractButton>
#include "conf/SystemSettings.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    static LoginDialog *GetInstance();
    static void FreeInstance();

private:
    static LoginDialog *m_instance;

    explicit LoginDialog(QWidget *parent = 0);
    ~LoginDialog();

private slots:
    void buttonBox_clicked(QAbstractButton *button);

private:
    Ui::LoginDialog *ui;
    SystemSettings *m_sysSettings;

    void initProjectComboBox();
    void initWorkModeComboBox();
    void initOperatorComboBox();
    void connectSignalSlots();
    void cache2Ui();
    void ui2Cache();
};

#endif // LOGINDIALOG_H
