#ifndef SHAREDIRWIDGET_H
#define SHAREDIRWIDGET_H

#include <QWidget>
#include "conf/SystemSettings.h"
#include "ISettingsWidget.h"

namespace Ui {
class ShareDirWidget;
}

class ShareDirWidget : public QWidget, public ISettingsWidget
{
    Q_OBJECT

public:
    explicit ShareDirWidget(QWidget *parent = 0);
    ~ShareDirWidget();

    void LoadSettings();
    bool SaveSettings();

private slots:
    void btnFind_clicked();
    void btnBrowse_clicked();

private:
    Ui::ShareDirWidget *ui;
    SystemSettings *m_sysSettings;
};

#endif // SHAREDIRWIDGET_H
