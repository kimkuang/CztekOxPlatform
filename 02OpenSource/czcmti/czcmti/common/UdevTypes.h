#ifndef UDEVTYPES_H
#define UDEVTYPES_H

#include <QMetaType>
#include <QObject>

namespace UdevTypes {
    enum E_HotplugEvent {
        HE_DeviceExisted,
        HE_DeviceAdded,
        HE_DeviceRemoved,
    };

    enum E_DeviceType {
        DT_Unkown = -1,
        DT_UsbDisk = 0x01, // u disk
        DT_V4L2 = 0x02,    // uvc
        DT_All = DT_UsbDisk | DT_V4L2
    };

    struct T_UdevDevice {
        E_DeviceType DeviceType;
        QString DevNode;
        QString ExtraInfo;
    };

    Q_DECLARE_FLAGS(DeviceTypes, E_DeviceType)
    Q_FLAGS(DeviceTypes) // todo
}
Q_DECLARE_OPERATORS_FOR_FLAGS(UdevTypes::DeviceTypes)
Q_DECLARE_METATYPE(UdevTypes::E_DeviceType)
Q_DECLARE_METATYPE(UdevTypes::E_HotplugEvent)

#endif // UDEVTYPES_H

