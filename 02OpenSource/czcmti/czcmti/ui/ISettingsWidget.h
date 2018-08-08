#ifndef ISETTINGSWIDGET_H
#define ISETTINGSWIDGET_H

class ISettingsWidget
{
public:
    virtual void LoadSettings() = 0;
    virtual bool SaveSettings() = 0;
};

#endif // ISETTINGSWIDGET_H
