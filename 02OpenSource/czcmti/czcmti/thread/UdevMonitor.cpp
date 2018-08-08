#include "UdevMonitor.h"
#include <QDebug>
#include <QCoreApplication>
#include <poll.h>

// *******************************************************************
// libudev资料：
// http://www.signal11.us/oss/udev/
// https://www.kernel.org/pub/linux/utils/kernel/hotplug/libudev/ch01.html
// *******************************************************************

#define SUBSYSTEM_BLOCK_FILTER       "block"
#define SUBSYSTEM_VIDEO4LINUX_FILTER "video4linux"
#define DEVTYPE_PARTITION_FILTER "partition"

UdevMonitor::UdevMonitor(UdevTypes::DeviceTypes deviceTypes)
{
    m_deviceTypes = deviceTypes;
    m_udev = udev_new();
    m_abort = false;

    qRegisterMetaType<UdevTypes::E_DeviceType>("UdevTypes::E_DeviceType");
    qRegisterMetaType<UdevTypes::E_HotplugEvent>("UdevTypes::E_HotplugEvent");
}

UdevMonitor::~UdevMonitor()
{
    udev_unref(m_udev);
}

void UdevMonitor::Start()
{
    QMutexLocker locker(&m_mutex);
    m_abort = false;
    start();
}

void UdevMonitor::Stop()
{
    QMutexLocker locker(&m_mutex);
    m_abort = true;
}

void UdevMonitor::run()
{
    // 枚举现有的设备
    enumerateExistedDevice();

    // 启动热插拔检测
    struct udev_monitor *monitor = udev_monitor_new_from_netlink(m_udev, "udev");
    if (m_deviceTypes.testFlag(UdevTypes::DT_UsbDisk)) {
        udev_monitor_filter_add_match_subsystem_devtype(monitor, SUBSYSTEM_BLOCK_FILTER, DEVTYPE_PARTITION_FILTER);
    }
    if (m_deviceTypes.testFlag(UdevTypes::DT_V4L2)) {
        udev_monitor_filter_add_match_subsystem_devtype(monitor, SUBSYSTEM_VIDEO4LINUX_FILTER, NULL);
    }
    udev_monitor_enable_receiving(monitor);

    struct pollfd pfd;
    pfd.fd = udev_monitor_get_fd(monitor);
    pfd.events = POLLIN;
    while (!m_abort) {
        if (poll(&pfd, 1, 500) < 1)
            continue;

        if (pfd.revents & POLLIN) {
            struct udev_device *device = udev_monitor_receive_device(monitor);
            handleDevice(device);
            udev_device_unref(device);
        }
    }
    udev_monitor_unref(monitor);
}

void UdevMonitor::enumerateExistedDevice()
{
    struct udev_enumerate *enumerate = udev_enumerate_new(m_udev);
    if (m_deviceTypes.testFlag(UdevTypes::DT_UsbDisk)) {
        udev_enumerate_add_match_subsystem(enumerate, SUBSYSTEM_BLOCK_FILTER);
    }
    if (m_deviceTypes.testFlag(UdevTypes::DT_V4L2)) {
        udev_enumerate_add_match_subsystem(enumerate, SUBSYSTEM_VIDEO4LINUX_FILTER);
    }
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry *list_entries = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry *list_entry;
    udev_list_entry_foreach(list_entry, list_entries) {
        const char* dev_path = udev_list_entry_get_name(list_entry);
        struct udev_device *device = udev_device_new_from_syspath(m_udev, dev_path);
        handleDevice(device, true);
        udev_device_unref(device);
    }
    udev_enumerate_unref(enumerate);
}

void UdevMonitor::handleDevice(struct udev_device *device, bool checkExisted, bool debugOn)
{
    if (device == NULL) {
        qCritical()<<"Function udev_monitor_receive_device() returns NULL.";
        return;
    }
    UdevTypes::E_DeviceType deviceType = UdevTypes::DT_Unkown;
    const char *subsystem = udev_device_get_subsystem(device);
    if (strcmp(subsystem, SUBSYSTEM_BLOCK_FILTER) == 0) {
        // 检测Block设备
        QString devType = udev_device_get_devtype(device);
        if (devType.compare(DEVTYPE_PARTITION_FILTER, Qt::CaseInsensitive) != 0) {
            if (debugOn)
                qDebug()<<QString("Not partition device type[%1], SKIP it.").arg(devType);
            return;
        }
        QString idBusValue = udev_device_get_property_value(device, "ID_BUS");
        if (idBusValue.compare("usb", Qt::CaseInsensitive) != 0) {
            if (debugOn)
                qDebug()<<QString("Not usb device[%1], SKIP it.").arg(idBusValue);
            return;
        }
        deviceType = UdevTypes::DT_UsbDisk;
    }
    else if (strcmp(subsystem, SUBSYSTEM_VIDEO4LINUX_FILTER) == 0) {
        // 检测V4L2
        const char *cap = udev_device_get_property_value(device, "ID_V4L_CAPABILITIES");
        if (cap == NULL || strstr(":capture:", cap) == NULL) {
            if (debugOn)
                qDebug()<<"Not capture UVC device.";
            return;
        }
        deviceType = UdevTypes::DT_V4L2;
    }    

    QString devNode = udev_device_get_devnode(device);
    QString action = udev_device_get_action(device);
    if (action.compare("add", Qt::CaseInsensitive) == 0) {
        emit eventChanged(deviceType, UdevTypes::HE_DeviceAdded, devNode);
    }
    else if (action.compare("remove", Qt::CaseInsensitive) == 0) {
        emit eventChanged(deviceType, UdevTypes::HE_DeviceRemoved, devNode);
    }
    else if (checkExisted && action.isEmpty()) {
        emit eventChanged(deviceType, UdevTypes::HE_DeviceExisted, devNode);
    }
    else {
        qDebug()<<devNode<<action;
    }
}
