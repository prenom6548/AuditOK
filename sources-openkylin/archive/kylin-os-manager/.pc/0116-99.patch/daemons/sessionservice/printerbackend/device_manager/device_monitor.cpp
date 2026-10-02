#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>
#include <unistd.h>
#include <regex>
#include <iostream>

#include <sys/inotify.h>

#include <cups/cups.h>

#include <QDebug>
#include <QString>
#include <QProcess>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QThread>
#include <QObject>
#include <QMetaType>
#include <QUrl>
#include <QDataStream>

#include "device_monitor.h"
#include "common.h"
#include "lpinfo_browser.h"
#include "udev_devices.h"
#include "information_collector.h"

#define UEVENT_BUFFER_SIZE 2048

static int init_hotplug_sock(void)
{
    struct sockaddr_nl snl;
    const int buffersize = 16 * 1024 * 1024;
    int retval;
    memset(&snl, 0x00, sizeof(struct sockaddr_nl));
    snl.nl_family = AF_NETLINK;
    snl.nl_pid = getpid();
    snl.nl_groups = 1;
    int hotplug_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (hotplug_sock == -1)
    {
        qDebug("error getting socket: %s", strerror(errno));
        return -1;
    }
    /* set receive buffersize */
    setsockopt(hotplug_sock, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize));
    retval = bind(hotplug_sock, (struct sockaddr *) &snl, sizeof(struct sockaddr_nl));
    if (retval < 0) {
        qDebug("bind failed: %s", strerror(errno));
        close(hotplug_sock);
        hotplug_sock = -1;
        return -1;
    }
    return hotplug_sock;
}

QString getUsbClassFromPath(const QString &path)
{
    QString res;
    QString bInterfaceClass;
    QStringList bInterfaceClassPathList = getRetFromCommand(QStringList{"find", path ,"-name", "bInterfaceClass"}).split("\n");
    for (int i = 0; i < bInterfaceClassPathList.size(); i++) {
        bInterfaceClass = getRetFromCommand(QStringList{"cat", bInterfaceClassPathList.at(i)});
        if (bInterfaceClass == "00" || bInterfaceClass == "ff"){
            continue;
        }
        res = bInterfaceClass;
    }
    return res;
}


// DeviceMonitorBackend start

DeviceMonitorBackend *DeviceMonitorBackend::getInstance()
{
    static DeviceMonitorBackend *instance = nullptr;
    static QThread *thread;
    if (nullptr == instance) {
        instance = new DeviceMonitorBackend();
        thread = new QThread();
        thread->setObjectName("DeviceMonitorBackend");
        instance->moveToThread(thread);
        connect(instance, &DeviceMonitorBackend::start,
                instance, &DeviceMonitorBackend::detectStart);
        connect(instance, &DeviceMonitorBackend::usbDeviceAdd,
                instance, &DeviceMonitorBackend::doDeviceAdd);
        connect(instance, &DeviceMonitorBackend::usbDeviceRemove,
                instance, &DeviceMonitorBackend::doDeviceRemove);
        thread->start();
    }
    return instance;
}


DeviceMonitorBackend::DeviceMonitorBackend()
{
}

DeviceMonitorBackend::~DeviceMonitorBackend()
{
}

void DeviceMonitorBackend::detectStart()
{
    // qDebug() << "DeviceMonitor::detectStart";
    // qDebug() << QThread::currentThreadId();
    // usbDeviceAdd("bind@/devices/pci0000:00/0000:00:16.0/0000:0b:00.0/usb3/3-2");
    int hotplug_sock = init_hotplug_sock();
    while(1)
    {
        char buf[UEVENT_BUFFER_SIZE*2] = {0};
        // recv 会等待usb信号连入
        recv(hotplug_sock, &buf, sizeof(buf), 0);
        this->usbDeviceIdentify(QString(buf));
    }
}

bool DeviceMonitorBackend::usbDeviceIdentify(const QString &qstr)
{
    // qDebug() << qstr;
    // qDebug() << "DeviceMonitor::usbDeviceIdentify";
    // bind@/devices/pci0000:00/0000:00:11.0/0000:02:01.0/usb1/1-1
    if (  qstr.indexOf("bind") == 0
      &&  (qstr.contains("pci") || qstr.contains("hisi_usb"))
      && !qstr.right( qstr.size() -1 - qstr.lastIndexOf('/') ).contains(":")
    ) {
        emit this->usbDeviceAdd(devicePathCheck(qstr));
        return true;
    }
    // unbind@/devices/pci0000:00/0000:00:11.0/0000:02:01.0/usb1/1-1
    if (  qstr.contains("unbind")
      &&  (qstr.contains("pci") || qstr.contains("hisi_usb"))
      && !qstr.right( qstr.size() -1 - qstr.lastIndexOf('/') ).contains(":")
    ) {
        emit this->usbDeviceRemove(devicePathCheck(qstr));
        return true;
    }
    return false;
}


bool DeviceMonitorBackend::doDeviceAdd(const QString &qstr)
{
    QString deviceId = get_device_id_from_path(qstr);
    qDebug() << "DeviceMonitorBackend::doDeviceAdd is: " << deviceId;
    if (deviceId.isEmpty()) {
        return false;
    }

    auto newDevice = LpinfoBrowser::getUsbPrinterDeviceFromDeviceId(deviceId);
    if (newDevice.deviceType != DeviceType::USB) {
        return false;
    }
    // 判断是否重复
    if (this->m_mpDeviceInformationMap.contains(qstr)) {
        return false;
    }

    newDevice.sysPath      = QString("/sys") + qstr;
    newDevice.devicePath   = "/dev/" + getRetFromCommand(QStringList{"cat", newDevice.sysPath + "/uevent", "|" , "grep", "DEVNAME"}).remove("DEVNAME=");
    newDevice.usbClass     = getUsbClassFromPath(newDevice.sysPath);
    newDevice.busNumber    = getRetFromCommand(QStringList{"cat", newDevice.sysPath + "/busnum"});
    newDevice.deviceNumber = getRetFromCommand(QStringList{"cat", newDevice.sysPath + "/devnum"});
    newDevice.VID          = getRetFromCommand(QStringList{"cat", newDevice.sysPath + "/idVendor"});
    newDevice.PID          = getRetFromCommand(QStringList{"cat", newDevice.sysPath + "/idProduct"});


    this->m_mpDeviceInformationMap.insert(qstr, newDevice);
    qDebug() << "find a new painter!";
    // qDebug() << newDevice;
    emit DeviceMonitorBackend::getInstance()->findUsbConnect(newDevice);
    return true;
}

bool DeviceMonitorBackend::doDeviceRemove(const QString &qstr)
{
    if (this->m_mpDeviceInformationMap.contains(qstr)) {
        this->m_mpDeviceInformationMap.remove(qstr);
    }
    // qDebug() << "remove a painter!";
    QString path = QString("/sys") + qstr;
    // qDebug() << path;
    emit this->findUsbDisconnect(path);
    return true;
}
