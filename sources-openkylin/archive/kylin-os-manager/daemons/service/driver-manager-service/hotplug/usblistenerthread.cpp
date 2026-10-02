#include "usblistenerthread.h"
#include <QDebug>
#include <QProcess>

USBListenerThread::USBListenerThread(QObject *parent) : QThread(parent) {
    m_udev = udev_new();
    if (!m_udev) {
        qDebug() << "Failed to create m_udev context";
        QCoreApplication::exit(1);
    }

    m_monitor = udev_monitor_new_from_netlink(m_udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(m_monitor, "usb", NULL);
    udev_monitor_enable_receiving(m_monitor);

    fd = udev_monitor_get_fd(m_monitor);

    connect(this, &USBListenerThread::started, this, &USBListenerThread::initializeConnectedDevices);

    qDebug() << "USB Listener thread initialized.";
}

void USBListenerThread::initializeConnectedDevices() {
    initDevPaths();

    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;

    enumerate = udev_enumerate_new(m_udev);
    udev_enumerate_add_match_subsystem(enumerate, "usb");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path = udev_list_entry_get_name(dev_list_entry);
        struct udev_device *dev = udev_device_new_from_syspath(m_udev, path);

        if (dev) {
            collectDeviceInfo(dev);
            udev_device_unref(dev);
        }
    }

    udev_enumerate_unref(enumerate);
}

USBListenerThread::~USBListenerThread() {
    udev_unref(m_udev);
    qDebug() << "USB Listener thread terminated.";
}

void USBListenerThread::run() {
    qDebug() << "USB Listener thread is running...";

    while (!isInterruptionRequested()) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        int ret = select(fd + 1, &fds, NULL, NULL, NULL);
        if (ret > 0 && FD_ISSET(fd, &fds)) {
            struct udev_device *dev = udev_monitor_receive_device(m_monitor);
            if (dev) {
                processUsbDevice(dev);
                udev_device_unref(dev);
            }
        }
    }
}

void USBListenerThread::processUsbDevice(struct udev_device *dev) {
    const QString action = udev_device_get_action(dev);
    const QString devpath = udev_device_get_devpath(dev);
    const QString devnode = udev_device_get_devnode(dev);
    const QString devtype = udev_device_get_devtype(dev);

    QString VID = udev_device_get_sysattr_value(dev, "idVendor");
    QString PID = udev_device_get_sysattr_value(dev, "idProduct");
    QString vendor = udev_device_get_sysattr_value(dev, "manufacturer");
    QString model = udev_device_get_sysattr_value(dev, "product");

    QString devID = getDeviceID(dev);
    if (action == "add"){
        if (devInfos.contains(devID)){
            qDebug() << "Ignored duplicate device: " << devpath;
            return;
        } else{
            qDebug() << "Added device: " << devpath;
            devInfos.append(devID);
        }
    }

    if (action == "remove"){
        if (devInfos.contains(devID)){
            qDebug() << "Removed device: " << devpath;
            devInfos.removeOne(devID);
        }
    }

    // 对HP特殊处理
    if (model.contains("HP")){
        model = model.remove("HP").trimmed();
    }

    // 用于判断路径
    QString checkpath = "";
    if (devpath.split("/").last().contains(":")){
        QStringList devlists = devpath.split("/");
        for (int i = 0; i < devlists.length()-1; ++i) {
            if (devlists.at(i) == ""){
                continue;
            }
            checkpath += "/";
            checkpath += devlists.at(i);
        }
    }

    QString deviceType = "";
    if (action == "add") {
        QString path = QString("/sys") + devpath;
        deviceType = getDeviceTypeFromPath(path);
        if (!(deviceType == "06" || deviceType == "07" || deviceType == "ff")){
            return;
        }
    } else {
        for (auto dev: m_devicesInfo) {
            if (checkpath == dev.devPath || devpath == dev.devPath) {
                deviceType = dev.devType;
                break;
            }
        }
    }
    qDebug() << "Action: " << action << ", Device Path: " << devpath << ", Device Type: " << deviceType;

    if (action == "add" && !VID.isEmpty() && !PID.isEmpty()) {
        DeviceInfo deviceInfo;
        deviceInfo.devPath = devpath;
        deviceInfo.VID = VID;
        deviceInfo.PID = PID;
        deviceInfo.vendor = vendor;
        deviceInfo.model = model;
        deviceInfo.devType = deviceType;
        m_devicesInfo.append(deviceInfo);
    }
    if (action == "remove") {
        for (int i = 0; i < m_devicesInfo.length(); i++) {
            if (checkpath == m_devicesInfo.at(i).devPath || devpath ==m_devicesInfo.at(i).devPath) {
                VID = m_devicesInfo.at(i).VID;
                PID = m_devicesInfo.at(i).PID;
                vendor = m_devicesInfo.at(i).vendor;
                model = m_devicesInfo.at(i).model;
                m_devicesInfo.removeAt(i);
                break;
            }
        }
    }

    if (!VID.isEmpty() && !PID.isEmpty()) {
        qDebug() << action << "  VID ID: " << VID << ", PID ID: " << PID;

        if (devtype != "08" && devtype != "09" && devtype != "10") {
            if (action == "add" || action == "remove") {
                emit deviceChange(action, VID, PID, vendor, model, deviceType);
            }
        }
    }
}

void USBListenerThread::collectDeviceInfo(struct udev_device *dev) {
    const QString devpath = udev_device_get_devpath(dev);
    const QString devnode = udev_device_get_devnode(dev);
    const QString VID = udev_device_get_sysattr_value(dev, "idVendor");
    const QString PID = udev_device_get_sysattr_value(dev, "idProduct");
    const QString vendor = udev_device_get_sysattr_value(dev, "manufacturer");
    const QString model = udev_device_get_sysattr_value(dev, "product");
    const QString devtype = udev_device_get_devtype(dev);

    if (!VID.isEmpty() && !PID.isEmpty()) {
        DeviceInfo deviceInfo;
        deviceInfo.devPath = devpath;
        deviceInfo.devNode = devnode;
        deviceInfo.VID = VID;
        deviceInfo.PID = PID;
        deviceInfo.vendor = vendor;
        deviceInfo.model = model;

        QString path = "/sys" + devpath;
        QString deviceType = getDeviceTypeFromPath(path);
        if (deviceType != "") {
            deviceInfo.devType = deviceType;
        } else {
            deviceInfo.devType = devtype;
        }

        m_devicesInfo.append(deviceInfo);
    }
}

QString USBListenerThread::getDeviceTypeFromPath(QString path) {
    QString res;
    QString bInterfaceClass;
    QStringList bInterfaceClassPathList = getResultFromCommand(
            QStringList{"find", path, "-name", "bInterfaceClass"}).split("\n");
    for (int i = 0; i < bInterfaceClassPathList.size(); i++) {
        bInterfaceClass = getResultFromCommand(QStringList{"cat", bInterfaceClassPathList.at(i)});
        res = bInterfaceClass;
    }
    return res;

}

QString USBListenerThread::getResultFromCommand(QStringList command) {
    QProcess proc;
    QStringList options;
    options << "-c" << command.join(" ");
    proc.closeWriteChannel();
    proc.start("bash", options);
    if (!proc.waitForFinished(300)) {
        return "";
    }
    QString res = QString(proc.readAll());
    proc.close();
    if (res.right(1) == "\n")
        res.chop(1);
    return res;

}

void USBListenerThread::initDevPaths() {
    struct udev *udev = udev_new();
    if (!udev) {
        qDebug() << "Failed to create udev context\n";
        return;
    }

    struct udev_enumerate *enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_add_match_subsystem(enumerate, "drm");
    udev_enumerate_add_match_subsystem(enumerate, "bluetooth");
    udev_enumerate_add_match_subsystem(enumerate, "net");
    udev_enumerate_add_match_subsystem(enumerate, "scsi");
    udev_enumerate_add_match_subsystem(enumerate, "usb");
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry *entry;
    udev_list_entry_foreach(entry, devices) {
        const char *path = udev_list_entry_get_name(entry);
        struct udev_device *dev = udev_device_new_from_syspath(udev, path);
        QString devID = getDeviceID(dev);
        if (devID != "unknown"){
            devInfos.append(devID);
        }
        udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);
}

QString USBListenerThread::getDeviceID(struct udev_device *dev) {
    const char *token = udev_device_get_property_value(dev, "USEC_INITIALIZED");
    if (token) {
        return QString(token);
    }
    return QString("unknown");
}


