#include "devicemonitorthread.h"
#include <libudev.h>
#include <iostream>

DeviceMonitorThread::DeviceMonitorThread(QObject *parent)
        : QThread(parent) {
}

void DeviceMonitorThread::run() {
    struct udev *udev = udev_new();
    if (!udev) {
        std::cerr << "Failed to create udev context\n";
        return;
    }

    struct udev_monitor *mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "block", NULL); // 监视块设备的变化
    udev_monitor_filter_add_match_subsystem_devtype(mon, "input", NULL); // 监视输入设备的变化
    udev_monitor_filter_add_match_subsystem_devtype(mon, "drm", "drm_minor"); // 监视显示器设备的变化
    udev_monitor_filter_add_match_subsystem_devtype(mon, "bluetooth", NULL); // 监视蓝牙设备的变化
    udev_monitor_filter_add_match_subsystem_devtype(mon, "net", NULL); // 监视网络设备的变化
    udev_monitor_filter_add_match_subsystem_devtype(mon, "scsi", "scsi_device"); // 监视SCSI设备的变化

    udev_monitor_enable_receiving(mon);

    int fd = udev_monitor_get_fd(mon);
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    while (m_running) {
        if (select(fd + 1, &fds, NULL, NULL, NULL) > 0) {
            if (FD_ISSET(fd, &fds)) {
                struct udev_device *dev = udev_monitor_receive_device(mon);
                if (dev) {
                    const QString action = udev_device_get_action(dev);
                    const char *subsystem = udev_device_get_subsystem(dev);
                    const char *devtype = udev_device_get_devtype(dev);
                    const char *devpath = udev_device_get_devpath(dev);

                    if (!(action == "add" || action == "remove" || (action == "change") && strcmp(devtype, "drm_minor") == 0 && strcmp(subsystem, "drm") == 0)) {
                        continue;
                    }
                    if (subsystem && devpath) {
                        if (strcmp(subsystem, "block") == 0) {
                            // 块设备，可能是U盘
                            if (strstr(devtype, "disk")) {
                                if (action == "change") {
                                    continue;
                                }
                                emit deviceInsRem(action, "USB storage device");
                            }
                        } else if (strcmp(subsystem, "input") == 0) {
                            if (action == "change") {
                                continue;
                            }
                            // 输入设备，可能是键盘或鼠标
                            if (!QString(devpath).contains(m_devPath) || m_devPath == "" || m_currentAction != action) {
                                m_currentAction = action;
                                emit deviceInsRem(action, "Keyboard or mouse");
                                QStringList m_devPathList = QString(devpath).split("/");
                                for (int i = m_devPathList.length(); i > 6; --i) {
                                    m_devPathList.removeLast();
                                }
                                m_devPath = m_devPathList.join("/");
                            }
                        } else if (strcmp(subsystem, "drm") == 0 && strcmp(devtype, "drm_minor") == 0) {
                            // 显示器设备
                            emit deviceInsRem(action, "Display");
                        } else if (strcmp(subsystem, "bluetooth") == 0) {
                            // 蓝牙设备
                            emit deviceInsRem(action, "Bluetooth device");
                        } else if (strcmp(subsystem, "net") == 0) {
                            // 网卡设备
                            emit deviceInsRem(action, "Network card");
                        }else if (strcmp(subsystem, "scsi") == 0) {
                            // SCSI设备，可能是光驱
                            emit deviceInsRem(action, "Optical drive");
                        }
                    }
                    udev_device_unref(dev);
                }
            }
        }
    }
    udev_monitor_unref(mon);
    udev_unref(udev);
}

void DeviceMonitorThread::stop() {
    m_running = false;
}