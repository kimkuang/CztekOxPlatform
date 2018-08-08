#ifndef SENSORPROPERTYBROWSERWIDGET_H
#define SENSORPROPERTYBROWSERWIDGET_H

#include <QtSolutions/QtTreePropertyBrowser>
#include <QtSolutions/qtpropertymanager.h>
#include <QtSolutions/qteditorfactory.h>
#include "entities/sensor.h"

class SensorPropertyBrowserWidget : public QtTreePropertyBrowser
{
public:
    SensorPropertyBrowserWidget(QWidget *parent = 0);
    ~SensorPropertyBrowserWidget();

    void SetSensor(const Sensor &sensor, bool isDefault);
    void GetSensor(Sensor &sensor);
    void ShowAdvanced(bool show);

private slots:
    void boolManager_valueChanged(QtProperty *property, bool val);

private:
    Sensor m_sensor;
    QtProperty *m_rootProperty;
    QMap<QtProperty *, QString> m_mapProperty2Name;

    QtIntPropertyManager *m_intPropManager;
    QtProperty *m_voltDovdd;
    QtProperty *m_voltDvdd;
    QtProperty *m_voltAvdd;
    QtProperty *m_voltAfvcc;
    QtProperty *m_voltVpp;

    void clearProperties();
    void addProperties();
};

#endif // SENSORPROPERTYBROWSERWIDGET_H
