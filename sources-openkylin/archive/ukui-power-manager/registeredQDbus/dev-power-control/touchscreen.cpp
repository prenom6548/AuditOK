/*
 * Copyright 2023 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QString>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>

#include "touchscreen.h"

TouchScreenDevice::TouchScreenDevice(QObject *parent) : QObject(parent)
{
    m_powerConfigMap = UpmCustomHardwareConfig::self()->getConfig(UPM_GROUP_TOUCHSCREEN);
//    for (const auto& pair : m_powerConfigMap) {
//        qDebug() << pair.first << ":" << pair.second;
//    }
    printConfigMap();
    //m_controlFile = KEYBOARD_BACKLIGHT_FILE;
    getDevicePowerInfo();
}
int TouchScreenDevice::open_restricted(const char *path, int flags, void *user_data)
{
    return open(path, flags);
}

void TouchScreenDevice::close_restricted(int fd, void *user_data)
{
    close(fd);
}

void TouchScreenDevice::getDevicePowerInfo()
{

    m_controlPath.clear();
    m_deviceId.clear();

    struct udev *udev = udev_new();
    if (!udev) {
        qCritical() << "创建 udev 失败";
        return;
    }

    static const struct libinput_interface interface = {
        .open_restricted = open_restricted,
        .close_restricted = close_restricted
    };

    struct libinput *li = libinput_udev_create_context(&interface, nullptr, udev);
    if (!li) {
        qCritical() << "创建 libinput 失败";
        udev_unref(udev);
        return;
    }

    libinput_udev_assign_seat(li, "seat0");
    libinput_dispatch(li);

    struct libinput_event *event;
    while ((event = libinput_get_event(li)) != nullptr) {
        if (libinput_event_get_type(event) == LIBINPUT_EVENT_DEVICE_ADDED) {
            struct libinput_device *dev = libinput_event_get_device(event);
            if (libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_TOUCH)) {
                struct udev_device *udev_dev = libinput_device_get_udev_device(dev);
                if (udev_dev) {
                    const char *devnode = udev_device_get_devnode(udev_dev);
                    if (devnode) {
                        //qDebug() << "✅ 找到触摸屏：" << devnode;
                        touchScreenPathid(QString::fromLocal8Bit(devnode));
                    }
                }
            }
        }
        libinput_event_destroy(event);
    }

    libinput_unref(li);
    udev_unref(udev);

    if (!m_controlPath.isEmpty()) {
        qInfo() << "TouchScreenDevice -- m_controlPath = " << m_controlPath;
        qInfo() << "TouchScreenDevice -- m_deviceId = " << m_deviceId;
    } else {
        qInfo() << "TouchScreenDevice === 未找到触摸屏驱动 ===";
        return;
    }

    const QString tempDevicePath = m_controlPath;
    m_controlPath = m_controlPath + "/";
    QFileInfo checkFile(tempDevicePath);
    if(checkFile.exists()) {
        qDebug() << "TouchScreenDevice::getTouchScreenDevicePowerInfo(): " << "control path is exists: " << tempDevicePath;
        m_devicePowerInfo.push_back(new DevicePowerInfo("touch_screen", m_controlPath, m_deviceId, "", this));
    }
}
bool TouchScreenDevice::touchScreenPathid(const QString &eventPath)
{
    char sys_path[PATH_MAX];
    char real_path[PATH_MAX];
    char driver_link[PATH_MAX];
    char driver_abs_path[PATH_MAX] = {0};
    char device_part[PATH_MAX];
    char dev_id[64] = {0};

    QByteArray eventBytes = eventPath.toLocal8Bit();
    const char *eventName = basename(const_cast<char*>(eventBytes.constData()));
    snprintf(sys_path, sizeof(sys_path), "/sys/class/input/%s/device", eventName);
    //qDebug() << "sys路径：" << sys_path;

    if (!realpath(sys_path, real_path)) {
        perror("realpath failed");
        return false;
    }
    //qDebug() << "真实路径：" << real_path;

    const char *i2c_pos = strstr(real_path, "/i2c-");
    const char *input_pos = strstr(real_path, "/input/");
    if (!i2c_pos || !input_pos) {
        //qWarning() << "不是 I2C 触摸屏";
        return false;
    }

    size_t part_len = input_pos - i2c_pos;
    strncpy(device_part, i2c_pos, part_len);
    device_part[part_len] = '\0';
    //qDebug() << "设备子路径：" << device_part;

    const char *last_slash = strrchr(device_part, '/');
    if (!last_slash) {
        //qWarning() << "无法提取设备ID";
        return false;
    }
    strcpy(dev_id, last_slash + 1);
    qInfo() << "TouchScreenDevice-设备ID：" << dev_id;

    snprintf(driver_link, sizeof(driver_link), "/sys/bus/i2c/devices%s/driver", device_part);
    //qDebug() << "驱动链接：" << driver_link;

    if (realpath(driver_link, driver_abs_path) == nullptr) {
        perror("realpath driver failed");
        return false;
    }
    qInfo() << "驱动绝对路径：" << driver_abs_path;

    m_controlPath = QString::fromLocal8Bit(driver_abs_path);
    m_deviceId = QString::fromLocal8Bit(dev_id);
    return true;
}

bool TouchScreenDevice::setPowerStat(int deviceIndex, const QString &stat)
{

    // if (stat == "default" || stat == "performance" || stat == "powersave" || stat == "powersupersave") {
    //     if (deviceIndex < m_devicePowerInfo.count()) {
    //         return m_devicePowerInfo[deviceIndex]->setDevicePowerStat(stat);
    //     }
    // }
    // return false;
}

bool TouchScreenDevice::setAllDevicePowerStat(const QString &stat)
{
    qDebug() << "TouchScreenDevice::setAllDevicePowerStat: " << stat;
    if (stat == "bind" || stat == "unbind") {
        for (int i = 0; i < m_devicePowerInfo.count(); ++i) {
            m_devicePowerInfo[i]->setDevicePowerStat(stat);
        }
        return true;
    }
    return false;
}
