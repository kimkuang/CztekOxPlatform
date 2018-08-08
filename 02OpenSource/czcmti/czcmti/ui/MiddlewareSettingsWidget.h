#ifndef __MIDDLEWARESETTINGSWIDGET_H__
#define __MIDDLEWARESETTINGSWIDGET_H__

#include <QWidget>
#include <conf/SystemSettings.h>
#include "ISettingsWidget.h"

namespace Ui {
class MiddlewareSettingsWidget;
}

class MiddlewareSettingsWidget : public QWidget, public ISettingsWidget
{
    Q_OBJECT

public:
    explicit MiddlewareSettingsWidget(QWidget *parent = 0);
    ~MiddlewareSettingsWidget();

    void LoadSettings();
    bool SaveSettings();

private:
    Ui::MiddlewareSettingsWidget *ui;
    SystemSettings *m_sysSettings;
};

#endif // __MIDDLEWARESETTINGSWIDGET_H__
