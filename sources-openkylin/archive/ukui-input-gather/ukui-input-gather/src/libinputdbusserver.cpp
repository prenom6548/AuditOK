/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 *
 */

#include "libinputdbusserver.h"

#include <fcntl.h>
#include <unistd.h>
#include <libudev.h>
#include <libinput.h>
#include <libevdev/libevdev.h>

#include <QtDBus/QDBusConnection>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

static int openRestricted(const char *path, int flags, void *userData);
static void closeRestricted(int fd, void *userData);

struct libinput_interface libinputInterface {
    openRestricted,
    closeRestricted
};


LibinputDBusServer::LibinputDBusServer(QObject* parent) : QObject(parent)
{
    if (!QDBusConnection::systemBus().registerService("org.ukui.libinput.proxy"))
        qWarning() << "register dbus service error!";

    if (!QDBusConnection::systemBus().registerObject("/org/ukui/libinput/proxy",
                                                     "org.ukui.libinput.proxy", this,
                                                     QDBusConnection :: ExportAllSlots |
                                                     QDBusConnection :: ExportAllSignals))
        qWarning() << "register object error;";
}


int LibinputDBusServer::libinputTabletSwitchState()
{
    struct libinput *li = nullptr;
    struct libinput_event *ev = nullptr;
    struct udev* udev = nullptr;

    udev = udev_new();
    li = libinput_udev_create_context(
                &libinputInterface, nullptr, udev);
    if (!li) {
        throw std::runtime_error{"Error initialising UKUI-Input-Gather-Server: libinput"};
    }

    int seat = libinput_udev_assign_seat(li, "seat0");
    if (seat != 0) {
        throw std::runtime_error{"Error initialising UKUI-Input-Gather-Server: libinput seat"};
    }

    libinput_dispatch(li);
    while ((ev = libinput_get_event(li))) {

        if (libinput_event_get_type(ev) == LIBINPUT_EVENT_DEVICE_ADDED) {
            int state = deviceTabletModeState(ev);
            if (state >= 0) {
                libinput_ref(li);
                udev_unref(udev);
                libinput_event_destroy(ev);
                return state;
            }
        }

        libinput_event_destroy(ev);
        libinput_dispatch(li);
    }

    libinput_ref(li);
    udev_unref(udev);
    return -1;
}

QByteArray LibinputDBusServer::libinputDeviceList()
{
    struct libinput *li = nullptr;
    struct libinput_event *ev = nullptr;
    struct udev* udev = nullptr;

    udev = udev_new();
    li = libinput_udev_create_context(&libinputInterface, nullptr, udev);
    if (!li) {
        qWarning() << "UKUI-INPUT-GATHER: failed to create libinput context.";
        return QByteArray();
    }

    int seat = libinput_udev_assign_seat(li, "seat0");
    if (seat != 0) {
        qWarning() << "UKUI-INPUT-GATHER: failed to assign udev seat.";
        return QByteArray();
    }

    QJsonArray libinputDevices;
    libinput_dispatch(li);
    while ((ev = libinput_get_event(li))) {

        if (libinput_event_get_type(ev) == LIBINPUT_EVENT_DEVICE_ADDED) {
            libinput_device *libinputDevice = libinput_event_get_device(ev);
            QString deviceName = libinput_device_get_name(libinputDevice);

            DeviceCapability caps = checkDeviceCapability(libinputDevice);

            QJsonObject obj;
            obj["Name"] = deviceName;
            obj["Caps"] = static_cast<int>(caps);
            libinputDevices.append(obj);
        }

        libinput_event_destroy(ev);
        libinput_dispatch(li);
    }

    libinput_unref(li);
    udev_unref(udev);
    QJsonDocument doc(libinputDevices);
    QByteArray arr = doc.toJson();
    return arr;
}

int LibinputDBusServer::deviceTabletModeState(libinput_event *e)
{
    struct libinput_device *device = libinput_event_get_device(e);

    int hasSwitch = libinput_device_switch_has_switch(device, LIBINPUT_SWITCH_TABLET_MODE);

    if (hasSwitch > 0) {
        struct libevdev *evdev = NULL;
        const char* sysname   = libinput_device_get_sysname(device);
        std::string eventPath = std::string("/dev/input/") + sysname;

        int fd = open(eventPath.c_str(), O_RDONLY | O_NONBLOCK);
        int rc = libevdev_new_from_fd(fd, &evdev);
        if (rc < 0) {
            close(fd);
            qCritical() << "Failed to init libevdev" << strerror(-rc);
            return -1;
        }

        int state = libevdev_get_event_value(evdev, EV_SW, SW_TABLET_MODE);

        libevdev_free(evdev);
        return state;
    }

    return -1;
}

DeviceCapability LibinputDBusServer::checkDeviceCapability(libinput_device *device)
{
    DeviceCapability caps = static_cast<DeviceCapability>(0);
    if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_KEYBOARD)) {
        caps = static_cast<DeviceCapability>(caps | DEVICE_KEYBOARD);
    }
    if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_POINTER)) {
        caps = static_cast<DeviceCapability>(caps | DEVICE_POINTER);
    }
    if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_TOUCH)) {
        caps = static_cast<DeviceCapability>(caps | DEVICE_TOUCH);
    }
    if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_TABLET_TOOL)) {
        caps = static_cast<DeviceCapability>(caps | DEVICE_TABLET);
    }
    if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_TABLET_PAD)) {
        caps = static_cast<DeviceCapability>(caps | DEVICE_TABLET_PAD);
    }
    if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_GESTURE)) {
        caps = static_cast<DeviceCapability>(caps | DEVICE_GESTURE);
    }
    if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_SWITCH)) {
        caps = static_cast<DeviceCapability>(caps | DEVICE_SWITCH);
    }

    return caps;
}

int openRestricted(const char *path, int flags, void *userData)
{
    bool *grab = static_cast<bool*>(userData);
    int fd = open(path, flags);

    if (fd < 0)
        fprintf(stderr, "Failed to open %s (%s)\n",
            path, strerror(errno));
    else if (grab && *grab && ioctl(fd, EVIOCGRAB, (void*)1) == -1)
        fprintf(stderr, "Grab requested, but failed for %s (%s)\n",
            path, strerror(errno));

    return fd < 0 ? -errno : fd;
}
void closeRestricted(int fd, void *userData)
{
    Q_UNUSED(userData)
    close(fd);
}
