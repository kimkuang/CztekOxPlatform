#ifndef GLOBALSETTINGSDIALOG_H
#define GLOBALSETTINGSDIALOG_H

#include <QDialog>
#include <QAbstractButton>
#include "ISettingsWidget.h"

namespace Ui {
class GlobalSettingsDialog;
}

class GlobalSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GlobalSettingsDialog(QWidget *parent = 0);
    ~GlobalSettingsDialog();

private slots:
    void btnGeneral_clicked();
    void btnMiddleware_clicked();
    void btnIpc_clicked();
    void btnShareDir_clicked();
    void btnOK_clicked();
    void btnCancel_clicked();

private:
    Ui::GlobalSettingsDialog *ui;
    QList<ISettingsWidget *> m_settingsWidgetList;

    void connectSignalSlots();
};

#endif // GLOBALSETTINGSDIALOG_H
