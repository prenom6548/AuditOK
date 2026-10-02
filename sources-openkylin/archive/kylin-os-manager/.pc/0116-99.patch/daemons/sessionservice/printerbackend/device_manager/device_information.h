#ifndef DEVICE_INFORMATION_H
#define DEVICE_INFORMATION_H

#include <QDebug>
#include <QObject>
#include <QString>
#include <QStringList>
#include <map>
#include "uri.h"

class DeviceInformation {
public:
    // 公有部分
    ProtocolType protocolType{ProtocolType::NONE};
    DeviceType deviceType{DeviceType::NONE};
    QString devClass;
    QString ieee1284ID;
    QString location;
    QString info;
    QString name; // 打印机的名字
    QString displayName;
    QString vendor; // 供应商
    QString model; // 型号
    QString serial; // 序列号
    QString uri; // 设备uri
    QStringList debNameList{QStringList()};
    QStringList packageNameList{QStringList()}; // 包名
    QString makeAndModel; // make-and-model
    QString uuid;
    QString ppdName;
    QString packageLevel{QString()};
    QString suggestPpdName{QString()};
    QString userChoosePpdName{QString()};
    QString driverlessDomain{QString()};
    int suggestPpdScore{0};

    // usb 部分
    QString sysPath; // sys下的目录
    QString devicePath; // dev下的目录 绝对路径
    QString usbClass; // 设备种类 打印机为07
    QString busNumber;
    QString deviceNumber{QString()};
    QString VID; // usb vid
    QString PID; // usb pid

    // 网络部分
    QString networkNode;
    QString host;

    QString serialize() const;
    void unserialize(const QString &str);
    void importUri(const Uri &uri);
};

extern DeviceInformation unserializeDeviceInformation(const QString &str);

extern QString devInfoPrint(const DeviceInformation&);

extern QDebug operator<<(QDebug debug, const DeviceInformation&);

extern QStringList getPackagesNameFromHttp(DeviceInformation&);

extern QString getNameFromDeviceInformation(const DeviceInformation&);

extern void postDeviceInfo(const DeviceInformation &device, int ppdRank);

Q_DECLARE_METATYPE(DeviceInformation)

#endif // DEVICE_INFORMATION_H
