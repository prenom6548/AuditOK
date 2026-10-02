/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#ifndef DEVICE_INFORMATION_H
#define DEVICE_INFORMATION_H

#include <QDebug>
#include <QObject>
#include <QString>
#include <QStringList>
#include <map>

enum class ConnectType : int {
    InfoFrom_Invalid = 0,
    InfoFrom_USB,
    InfoFrom_NETWORK_DETECT,
    InfoFrom_PRINTER_IP,
    InfoFrom_REMOTE_IP
};

enum class DeviceType : int {
    NONE = 0,
    PRINTER,
    SCANNER
};

enum class ProtocolType : int {
    NONE = 0,
    LPD, // lpd://${打印机网络节点}/${打印机服务}
    IPP, // ipp://${IP}:${端口}/ipp/print
    HTTP, // http://${IP}
    SOCKET, // socket://${IP}:${端口}
    DNSSD, // dnssd://${打印机名称}._ipp._tcp.local/?uuid=${UUID}
    HTTPS,
};

const QMap<QString, ProtocolType> protocolMap = {
    std::map<QString, ProtocolType>::value_type("none", ProtocolType::NONE),
    std::map<QString, ProtocolType>::value_type("lpd", ProtocolType::LPD),
    std::map<QString, ProtocolType>::value_type("ipp", ProtocolType::IPP),
    std::map<QString, ProtocolType>::value_type("http", ProtocolType::HTTP),
    std::map<QString, ProtocolType>::value_type("socket", ProtocolType::SOCKET),
    std::map<QString, ProtocolType>::value_type("dnssd", ProtocolType::DNSSD),
    std::map<QString, ProtocolType>::value_type("https", ProtocolType::HTTPS)
};

class DeviceInformation {
public:
    DeviceInformation();
    // 公有部分
    DeviceType type{DeviceType::NONE};
    ConnectType connectType{ConnectType::InfoFrom_Invalid};
    ProtocolType protocolType{ProtocolType::NONE};
    QString name{QString()}; // 打印机的名字
    QString vendor{QString()}; // 供应商
    QString model{QString()}; // 型号
    QString serial{QString()}; // 序列号
    QString uri{QString()}; // 设备uri
    QStringList debNameList{QStringList()};
    QStringList packageNameList{QStringList()}; // 包名
    QStringList packageVersion{QStringList{}};  // 包版本
    bool installable = true;
    QString digitId;
    int packageLevel{int(-1)};
    QString makeAndModel{QString()}; // make-and-model
    QString uuid{QString()};
    QString deviceId{QString()};
    QString others{QString()};

    // usb 部分
    QString sysPath{QString()}; // sys下的目录
    QString devicePath{QString()}; // dev下的目录 绝对路径
    QString deviceType{QString()}; // 设备种类 打印机为07
    QString busNumber{QString()};
    QString deviceNumber{QString()};
    QString VID{QString()}; // usb vid
    QString PID{QString()}; // usb pid

    // 网络部分
    QString networkNode;
    QString host;
};

extern QString devInfoPrint(const DeviceInformation&);

extern QDebug operator<<(QDebug debug, const DeviceInformation&);

extern QStringList getPackagesNameFromHttp(DeviceInformation&);

void buriedNoDriverDevice(DeviceInformation device);

Q_DECLARE_METATYPE(DeviceInformation)

#endif // DEVICE_INFORMATION_H
