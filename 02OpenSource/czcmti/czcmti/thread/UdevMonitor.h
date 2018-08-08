#ifndef UDEVMONITOR_H
#define UDEVMONITOR_H

#include <QThread>
#include <QMutex>
#include <QHash>
#include "libudev.h"
#include "common/UdevTypes.h"

class UdevMonitor : public QThread
{
    Q_OBJECT
public:
    UdevMonitor(UdevTypes::DeviceTypes deviceTypes = UdevTypes::DT_All);
    ~UdevMonitor();

    void Start();
    void Stop();

signals:
    void eventChanged(UdevTypes::E_DeviceType deviceType, UdevTypes::E_HotplugEvent event, const QString &devNode);

protected:
    void run();

private:
    QMutex m_mutex;
    volatile bool m_abort;
    UdevTypes::DeviceTypes m_deviceTypes;
    struct udev *m_udev;

    void enumerateExistedDevice();
    void handleDevice(struct udev_device *device, bool checkExisted = false, bool debugOn = false);
};

#endif // UDEVMONITOR_H
