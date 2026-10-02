#ifndef BLUETOOTHINFO_H
#define BLUETOOTHINFO_H

#include "hardwareinfo.h"
#include "deviceinfo.h"
#include "../driverinfo/drivermodeinfo.h"

class BluetoothInfo : public HardWareInfo, public DeviceInfo {
public:
    BluetoothInfo();

    QString address{QString()};
    QString alcmtu{QString()};
    QString bus{QString()};
    QString dev_version{QString()};
    QString features{QString()};
    QString link_mode{QString()};
    QString link_policy{QString()};
    QString dev_name{QString()};
    QString packettype{QString()};
    QString scomtu{QString()};
    QString speed{QString()};
    DriverModeInfo modeinfo;

    QString getValue(QString value);

};

#endif // BLUETOOTHINFO_H
