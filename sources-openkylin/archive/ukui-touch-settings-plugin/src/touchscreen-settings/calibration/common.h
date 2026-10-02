#ifndef COMMON_H
#define COMMON_H

#include <QString>

enum class DeviceType {
    InputDevice,
    OutputDevice
};

struct Device
{
    explicit Device(QString devicename = QString(),
                    DeviceType devicetype = DeviceType::InputDevice, int id = 0)
        : name(devicename)
        , type(devicetype)
        , deviceId(id){}

    QString name;
    DeviceType type;
    int deviceId;
};

namespace env {

bool isWayland();
bool isTabletMode();

} // namespace env

#endif // COMMON_H
