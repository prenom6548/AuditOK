//
// Created by sqp on 2021/11/15.
//

#ifndef KYLIN_CONNECTIVITY_UTIL_H
#define KYLIN_CONNECTIVITY_UTIL_H

#include <QUuid>

namespace KCommon {

enum DeviceType {
    UNKNOWN = 0,
    PC = 1,
    Phone = 2,
    TV = 3,
};

struct DiscoveryDeviceInfo {
    QString uuid;
    QString ip;
    QString port;
    QString deviceName;
    quint16 deviceType;
};

QString getUserName();
QString getHostName();

}  // namespace KCommon

#endif  // KYLIN_CONNECTIVITY_UTIL_H
