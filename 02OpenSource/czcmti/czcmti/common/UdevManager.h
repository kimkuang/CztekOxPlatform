#ifndef UDEVMANAGER_H
#define UDEVMANAGER_H
#include <QObject>
#include <QHash>
#include <common/UdevTypes.h>

class UdevManager : public QObject
{
public:
    static void RegisterDevice(const QString &devNode, UdevTypes::T_UdevDevice device);
    static void UnregisterDevice(const QString &devNode);

    static void GetUdevList(UdevTypes::E_DeviceType devType, QList<UdevTypes::T_UdevDevice> &devList);
private:
    static QHash<QString, UdevTypes::T_UdevDevice> m_udevQueue;
private:
    UdevManager();
    ~UdevManager();
    Q_DISABLE_COPY(UdevManager)
};

#endif // UDEVMANAGER_H
