#ifndef SERVICEDETAILDIALOG_H
#define SERVICEDETAILDIALOG_H

#include <QDialog>
#include "IAppService.h"

namespace Ui {
class ServiceDetailDialog;
}

class ServiceDetailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ServiceDetailDialog(QWidget *parent = 0);
    ~ServiceDetailDialog();

private slots:
    void btnStart_clicked();
    void btnStop_clicked();
    void btnConfig_clicked();

private:
    Ui::ServiceDetailDialog *ui;
    IAppService *m_appService;

    void populateService();
    void connectSignalSlots();
};

#endif // SERVICEDETAILDIALOG_H
