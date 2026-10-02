#include "networkcardinfo.h"
#include <QProcess>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>
#include "../include/common.h"

NetworkCardInfo::NetworkCardInfo() {

}

NetworkCardInfo::NetworkCardInfo(NetworkCardInfo *others) {
    bus = others->bus;
    MAC_addr = others->MAC_addr;
    isWireless = others->isWireless;
    ip_addr = others->ip_addr;
    net_musk = others->net_musk;
    send_bytes = others->send_bytes;
    recv_bytes = others->recv_bytes;
    MTU = others->MTU;
    modeinfo = others->modeinfo;
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
    }else if (value == "size") {
        return size;
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
    process.close();
    QStringList lines = output.split("\n");

    QString recv = "0 kb";
    QString send = "0 kb";
            foreach (const QString &line, lines) {
            if (line.contains("RX packets")) {
                recv = line.split("(")[1].split(")")[0];
                continue;
            }
            if (line.contains("TX packets")) {
                send = line.split("(")[1].split(")")[0];
                continue;
            }
        }
    QStringList list;
    list.append(recv);
    list.append(send);
    return list;
}

bool NetworkCardInfo::checkStatus() {
    if (isWireless == 1) {
        QDBusConnection bus = QDBusConnection::systemBus();
        QDBusInterface manager(DBUS_SERVICENAME, DBUS_DRIVER_INFO_TOOL, DBUS_DRIVER_INFO_INTERFACE_TOOL, bus);
        QDBusMessage response = manager.call("getWirelessNetworkCardStatus");
        QString result = response.arguments().at(0).toString();
        if (result == "disabled") {
            return false;
        }else{
            return true;
        }
    } else {
        QDBusConnection bus = QDBusConnection::systemBus();
        QDBusInterface manager(DBUS_SERVICENAME, DBUS_DRIVER_INFO_TOOL, DBUS_DRIVER_INFO_INTERFACE_TOOL, bus);
        QDBusMessage response = manager.call("getWiredNetworkCardStatus");
        QString result = response.arguments().at(0).toString();
        if (result == "disabled") {
            return false;
        }else{
            return true;
        }
    }
}

void NetworkCardInfo::setValue(QString key, QString value) {
    if (key == "vendor") {
        vendor = value;
    } else if (key == "model") {
        model = value;
    } else if (key == "serial") {
        serial = value;
    } else if (key == "name") {
        name = value;
    } else if (key == "deviceId") {
        deviceId = value;
    } else if (key == "driver_now") {
        driver_now = value;
    } else if (key == "driver_version_now") {
        driver_version_now = value;
    } else if (key == "MAC_addr") {
        MAC_addr = value;
    } else if (key == "bus") {
        bus = value;
    }else if (key == "ip_addr") {
        ip_addr = value;
    }else if (key == "MTU") {
        MTU = value;
    }else if (key == "net_musk") {
        net_musk = value;
    }else if (key == "send_bytes") {
        send_bytes = value;
    }else if (key == "recv_bytes") {
        recv_bytes = value;
    }else if (key == "size") {
        size = value;
    }
}
