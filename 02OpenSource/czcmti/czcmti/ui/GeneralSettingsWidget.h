#ifndef GENERALSETTINGSWIDGET_H
#define GENERALSETTINGSWIDGET_H

#include <QWidget>
#include <conf/SystemSettings.h>
#include "ISettingsWidget.h"

namespace Ui {
class GeneralSettingsWidget;
}

class GeneralSettingsWidget : public QWidget, public ISettingsWidget
{
    Q_OBJECT

public:
    explicit GeneralSettingsWidget(QWidget *parent = 0);
    ~GeneralSettingsWidget();

    void LoadSettings();
    bool SaveSettings();

protected:
    void showEvent(QShowEvent *event);

private slots:
    void spinDispParam_valueChanged(int n);

private:
    Ui::GeneralSettingsWidget *ui;
    SystemSettings *m_sysSettings;

    void relayoutIndicator(int hardwareChannelCnt, int dispCameraCnt, int dispCntPerRow);
};

#endif // GENERALSETTINGSWIDGET_H
