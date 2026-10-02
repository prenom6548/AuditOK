/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <QThread>

#include "network_device_detector.h"
#include "network_device_monitor.h"
#include "device_monitor.h"
#include "lpinfo_browser.h"
#include "snmpwalk_browser.h"
#include "common.h"
#include "connect.h"
#include "udev_devices.h"
#include "information_collector.h"

QString getValueBySnmpwalk(const QString &name, const QString &key)
{
    std::string sname = name.toStdString();
    std::string skey = key.toStdString();

    const char * cname = sname.c_str();
    const char * ckey = skey.c_str();

    std::vector<OidNode> oids = {
        { ckey },
    };


    std::vector<HostNode> hosts = {
        { cname, "public" },
    };

    std::vector<std::string> buf;

    if (::snmpwalkBrowser_initialize(oids) != STAT_SUCCESS) {
        return QString();
    }
    int ans = ::snmpwalkBrowser_synchronous(buf, hosts, oids);
    if (ans != STAT_SUCCESS) {
        return QString();
    }
    if (!buf.size()) {
        return QString();
    }
    QString res =  QString(buf.at(0).c_str());

    if (res.contains("IpAddress")
     || res.contains("STRING")) {

        res = res.split(":").at(1);
        res.remove(0,1);
        res.remove("\"");
        return res;
    }
    return "";
}

NetworkDeviceDetector *NetworkDeviceDetector::getInstance()
{
    static NetworkDeviceDetector *instance = nullptr;
    if (nullptr == instance) {
        instance = new NetworkDeviceDetector();
    }
    return instance;
}

NetworkDeviceDetector::~NetworkDeviceDetector()
{

}

QList<DeviceInformation> NetworkDeviceDetector::getDeviceInfoFromHost(const QString &host, const int port, ProtocolType protocolType)
{
    QString hhost;
    int pport;
    if (host.contains(":")) {
        hhost = host.split(":").at(0);
        pport = host.split(":").at(1).toInt();
    }
    else {
        hhost = host;
        pport = port;
    }
    QList<DeviceInformation> deviceInfoList;
    DeviceInformation deviceInfo;
    // 先检测host能否成功连接，如果无法成功连接则不用继续

    bool connect = false;
    if (::try_connect(QTC(hhost), pport) != 0) {
        qDebug() << "try to connect 631...";
        if (::try_connect(QTC(hhost), 631) != 0) {
            qDebug() << "port 631 failed!";
        }
        else {
            connect = true;
            pport = 631;
        }

        qDebug() << "try to connect 9100...";
        if (::try_connect(QTC(hhost), 9100) != 0) {
            qDebug() << "port 9100 failed!";
        }
        else {
            connect = true;
            pport = 9100;
        }
    }
    else {
        connect = true;
    }

    // lpd
    // if (!::try_connect(QTC(hhost), 515)) {
    //     qDebug("Cannot connect to %s:%d!\n", hhost, 515);
    //     return deviceInfo;
    // }

    if (connect == false) {
        return deviceInfoList;
    }
    NetworkDeviceDetector *p = NetworkDeviceDetector::getInstance();

#ifdef REMOTE_PRINTER
    if (p->probeIpp(deviceInfoList, hhost, pport)) {
        return deviceInfoList;
    }
#endif
    if (p->probe(deviceInfo, hhost, pport, protocolType)) {
        deviceInfoList.append(deviceInfo);
        return deviceInfoList;
    }
    deviceInfoList.append(deviceInfo);
    return deviceInfoList;
}

bool NetworkDeviceDetector::probe(DeviceInformation &deviceInfo, const QString host, const int port, ProtocolType protocolType)
{
    if (host.isEmpty()) {
        return false;
    }

    if (port == 631) {
        return false;
    }

    deviceInfo.host = host;
    setUri(deviceInfo, protocolType);

    this->detectNetWorkNode    (deviceInfo);
    this->detectVendorAndModel (deviceInfo);
    this->detectUUID           (deviceInfo);
    this->detectSerial         (deviceInfo);

    if (deviceInfo.uri.isEmpty()
     || deviceInfo.vendor.isEmpty()
     || deviceInfo.model.isEmpty()  ) {

        return false;
    }
    return true;
}

bool NetworkDeviceDetector::probeIpp(QList<DeviceInformation> &deviceInfoList, const QString host, const int port)
{
    if (port == 9100) {
        return false;
    }
    deviceInfoList.clear();
    QStringList list = LpinfoBrowser::findRemotePrinter(host, port);
    for (auto str : list) {
        DeviceInformation deviceInfo;
        deviceInfo.uri = str;
        deviceInfo.ppdName =  "raw";
        deviceInfoList.append(deviceInfo);
    }
    if (deviceInfoList.size()) {
        return true;
    }
    return false;
}

NetworkDeviceDetector::NetworkDeviceDetector(/* args */)
{

}

DetectResult NetworkDeviceDetector::detectNetWorkNode(DeviceInformation &deviceInfo)
{
    deviceInfo.networkNode = ::getValueBySnmpwalk(deviceInfo.host, "1.3.6.1.2.1.1.5");
    if (deviceInfo.networkNode.isEmpty()) {
        return DetectResult::DETECT_ERROR;
    }
    return DetectResult::DETECT_OK;
}

DetectResult NetworkDeviceDetector::detectVendorAndModel(DeviceInformation &deviceInfo)
{
    // 直接在1.3.6.1.2.1.25.3.2.1.3有问题，暂时使用cups的方法获取
    // QString res = ::getValueBySnmpwalk(deviceInfo.host, "1.3.6.1.2.1.25.3.2.1.3");
#if 1
    QString res = ::getRetFromCommand(QStringList{"/usr/lib/cups/backend/snmp", deviceInfo.host});
#else
    // TODO： 通过1.3.6.1.4.1.x 获取打印机信息

#endif
    if (res.isEmpty()) {
        // qDebug () << deviceInfo.host << " not found!";
        return DetectResult::DETECT_ERROR;
    }
    QStringList resList;
    resList = res.split("\"");
    if (resList.size() >= 6) {
        res = resList.at(5);
    }
    else {
        return DetectResult::DETECT_ERROR;
    }
    device_id_s id;
    parse_device_id(QTC(res), &id);
    if (id.mfg.isEmpty() || id.mdl.isEmpty()) {
        return DetectResult::DETECT_ERROR;
    }
    deviceInfo.vendor = id.mfg;
    deviceInfo.model = id.mdl;
    return DetectResult::DETECT_OK;
}

DetectResult NetworkDeviceDetector::detectUUID(DeviceInformation &deviceInfo)
{
    QString uuid;
    switch (deviceInfo.protocolType)
    {
    // dnssd://AABBCCDD._ipps._tcp.local/?uuid=83298d6b-197a-49eb-92a3-bd7bc23bcabe
    case (ProtocolType::DNSSD):
        if (deviceInfo.uri.contains("=")) {
            uuid = deviceInfo.uri.split("=").at(1);
        }
        if (!uuid.isEmpty()) {
            return DetectResult::DETECT_OK;
        }
    default:
        break;
    }
    return DetectResult::DETECT_ERROR;
}

DetectResult NetworkDeviceDetector::detectSerial(DeviceInformation &deviceInfo)
{
    deviceInfo.serial = ::getValueBySnmpwalk(deviceInfo.host, "1.3.6.1.2.1.43.5.1.1.17");
    if (deviceInfo.serial.isEmpty()) {
        return DetectResult::DETECT_ERROR;
    }
    return DetectResult::DETECT_OK;
}

SetResult NetworkDeviceDetector::setUri(DeviceInformation &deviceInfo, const ProtocolType &protocolType)
{
    switch (protocolType)
    {
    case (ProtocolType::SOCKET) :
        deviceInfo.uri = QString("socket://") + deviceInfo.host + (":9100");
        deviceInfo.deviceType = DeviceType::SOCKET;
        break;
    case (ProtocolType::HTTP) :
        deviceInfo.uri = QString("http://") + deviceInfo.host;
        deviceInfo.deviceType = DeviceType::HTTP;
        break;
    case (ProtocolType::IPP) :
        deviceInfo.uri = QString("ipp://") + deviceInfo.host +":631/ipp";
        deviceInfo.deviceType = DeviceType::IPP;
        break;
    default:
        return SetResult::SET_ERROR;
    }
    deviceInfo.protocolType = protocolType;
    return SetResult::SET_OK;
}
