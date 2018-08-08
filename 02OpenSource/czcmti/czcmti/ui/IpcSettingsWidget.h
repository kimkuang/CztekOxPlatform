#ifndef __IPCSETTINGSWIDGET_H__
#define __IPCSETTINGSWIDGET_H__

#include <QWidget>
#include <conf/SystemSettings.h>
#include "ISettingsWidget.h"

namespace Ui {
class IpcSettingsWidget;
}

class IpcSettingsWidget : public QWidget, public ISettingsWidget
{
    Q_OBJECT

public:
    explicit IpcSettingsWidget(QWidget *parent = 0);
    ~IpcSettingsWidget();

    void LoadSettings();
    bool SaveSettings();

private:
    Ui::IpcSettingsWidget *ui;
    SystemSettings *m_sysSettings;
};

#endif // __IPCSETTINGSWIDGET_H__
