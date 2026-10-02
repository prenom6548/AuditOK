#include "networkcardinfo.h"
#include <QProcess>
#include <QDebug>

NetworkCardInfo::NetworkCardInfo() {

}

QString NetworkCardInfo::getValue(QString value) {
    if (value == "vendor") {
        return vendor;
    } else if (value == "model") {
        return model;
    } else if (value == "serial") {
        return serial;
    } else if (value == "name") {
        return name;
    } else if (value == "deviceId") {
        return deviceId;
    } else if (value == "driver_now") {
        return driver_now;
    } else if (value == "driver_version_now") {
        return driver_version_now;
    } else if (value == "MAC_addr") {
        return MAC_addr;
    } else if (value == "bus") {
        return bus;
    } else if (value == "ip_addr") {
        return ip_addr;
    } else if (value == "MTU") {
        return MTU;
    } else if (value == "net_musk") {
        return net_musk;
    } else if (value == "send_bytes") {
        return getNetWorkStatus()[1];
    } else if (value == "recv_bytes") {
        return getNetWorkStatus()[0];
    } else if (value == "isWireless") {
        if (isWireless) {
            return QObject::tr("Wireless");
        } else {
            return QObject::tr("Wired");
        }
    } else {
        return ""; // 如果传入的 v

    }
}

QStringList NetworkCardInfo::getNetWorkStatus() {
    QProcess process;
    QString command = "ifconfig " + name;
    process.start(command);
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split("\n");

    QString recv = "0 kb";
    QString send = "0 kb";
    foreach (const QString &line, lines) {
        if (line.contains("RX packets")){
            recv = line.split("(")[1].split(")")[0];
            continue;
        }
        if (line.contains("TX packets")){
            send = line.split("(")[1].split(")")[0];
            continue;
        }
    }
    QStringList list;
    list.append(recv);
    list.append(send);
    return list;
}
