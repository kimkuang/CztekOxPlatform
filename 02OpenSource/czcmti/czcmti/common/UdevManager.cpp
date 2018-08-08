#include "UdevManager.h"

QHash<QString, UdevTypes::T_UdevDevice> UdevManager::m_udevQueue;

UdevManager::UdevManager()
{
}

UdevManager::~UdevManager()
{
}


void UdevManager::RegisterDevice(const QString &devNode, UdevTypes::T_UdevDevice device)
{
    if (m_udevQueue.contains(devNode))
        m_udevQueue[devNode] = device;
    else
        m_udevQueue.insert(devNode, device);
}

void UdevManager::UnregisterDevice(const QString &devNode)
{
    m_udevQueue.remove(devNode);
}

void UdevManager::GetUdevList(UdevTypes::E_DeviceType devType, QList<UdevTypes::T_UdevDevice> &devList)
{
    devList.clear();
    QHashIterator<QString, UdevTypes::T_UdevDevice> it(m_udevQueue);
    while (it.hasNext()) {
        it.next();
        UdevTypes::T_UdevDevice dev = it.value();
        if (dev.DeviceType == devType) {
            devList.append(dev);
        }
    }
}

