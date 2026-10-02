#ifndef HARDWAREINFO_H
#define HARDWAREINFO_H

#include <QList>
#include <QString>
#include <QStringList>
#include "../driverinfo/driverinfo.h"

enum class DeviceType : int {
    NONE = 0,
    PRINTER,
    SCANNER,
    GraphicCard,
    SoundCard,
    NetworkCard,
    Bluetooth,
    InputDev,
    VideoOrAudio,
    Other,
};

class HardWareInfo
{
public:
    HardWareInfo();

    DeviceType type = DeviceType::NONE;
    QString vendor{QString()}; // 供应商
    QString model{QString()}; // 型号
    QString serial{QString()}; // 序列号
    QString name{QString()};   // 设备名称
    QList<DriverInfo> driverNameList;
    QString deviceId{QString()};
    QString others{QString()};
    QString driver_now{QString()};
    QString driver_version_now{QString()};

    QString sysPath{QString()}; // sys下的目录
    QString devicePath{QString()}; // dev下的目录 绝对路径
    QString busNumber{QString()};
    QString deviceNumber{QString()};
    QString VID{QString()}; // usb vid
    QString PID{QString()}; // usb pid
    bool upgradeable = false;
};
extern QString hardWareInfoPrint(const HardWareInfo&);
extern QDebug operator<<(QDebug debug, const HardWareInfo&);


#endif // HARDWAREINFO_H
