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

#include "libinputeventgather.h"

#include <QThread>
#include <QSettings>
#include <QDebug>
#include <QTime>

#include <array>

#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <libinput.h>
#include <libudev.h>
#include <libevdev/libevdev.h>

LibinputEventGather::LibinputEventGather(InputEventGather *parent)
    :InputEventGather(parent)
{
    init();
}

LibinputEventGather::~LibinputEventGather()
{
    if (this->libinputContext != nullptr) {
        libinput_unref(this->libinputContext);
    }

    if (this->udevContext != nullptr) {
        udev_unref(this->udevContext);
    }
}

int LibinputEventGather::openRestricted(const char *path, int flags, void * userData) {
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

void LibinputEventGather::closeRestricted(int fd, void * /*userData*/) {
    close(fd);
}

void LibinputEventGather::init()
{
    // init os-release info
    QSettings osinfo("/etc/os-release", QSettings::IniFormat);
    if (osinfo.contains(QString("PROJECT_CODENAME"))) {
        m_project = osinfo.value(QString("PROJECT_CODENAME")).toString();
    }

    this->udevContext = udev_new();
    if (this->udevContext == nullptr) {
        throw std::runtime_error{"Error initialising UKUI-Input-Gather-Server: udev"};
    }

    this->libinputContext = libinput_udev_create_context(
                            &this->libinputInterface, nullptr, this->udevContext);
    if (this->libinputContext == nullptr) {
        throw std::runtime_error{"Error initialising UKUI-Input-Gather-Server: libinput"};
    }

    int seat = libinput_udev_assign_seat(this->libinputContext, "seat0");
    if (seat != 0) {
        throw std::runtime_error{"Error initialising UKUI-Input-Gather-Server: libinput seat"};
    }
}

void LibinputEventGather::run()
{
    qDebug() << QThread::currentThread();
    int fd = libinput_get_fd(this->libinputContext);
    if (fd == -1) {
        throw std::runtime_error{"Error initialising UKUI-Input-Gather-Server: libinput_get_fd"};
    }

    // Create poll to wait until libinput's file descriptor has data
    // https://man7.org/linux/man-pages/man2/poll.2.html
    int pollTimeout = -1;
    std::array<struct pollfd, 1> pollFds{{fd, POLLIN, 0}};

    while (poll(pollFds.data(), pollFds.size(), pollTimeout) >= 0) {
        // Once the data is in the file descriptor, read and process every event
        bool hasEvents = true;
        do {
            libinput_dispatch(this->libinputContext);
            struct libinput_event *event = libinput_get_event(this->libinputContext);
            if (event != nullptr) {
                handleEvent(event);
                libinput_event_destroy(event);
            } else {
                hasEvents = false;
            }
        } while (hasEvents);
    }
}

void LibinputEventGather::handleEvent(struct libinput_event *e)
{
    bool isSupported = true;
    Event event;
    libinput_event_type eventType = libinput_event_get_type(e);
    event.type = eventType;

    struct libinput_device *libinputDevice = libinput_event_get_device(e);
    const char *deviceName = libinput_device_get_name(libinputDevice);
    /*!
     * \brief strncpy 不要用strcpy！！！！在arm下会报指针越界的错误
     */
    int deviceNameLen = strlen(deviceName);
    int copyLen = qMin(255, deviceNameLen);
    strncpy(event.device.name, deviceName, copyLen);
    event.device.name[copyLen] = '\0';

    switch (eventType) {
    case LIBINPUT_EVENT_DEVICE_ADDED:
        handleDeviceAdded(e, event);
        break;
    case LIBINPUT_EVENT_DEVICE_REMOVED:
        break;
    case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
    case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE:
    case LIBINPUT_EVENT_GESTURE_SWIPE_END:{
        handleSwipeEvent(e, event);
        break;
    }
    case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
    case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:
    case LIBINPUT_EVENT_GESTURE_PINCH_END:{
        handlePinchEvent(e, event);
        break;
    }
    case LIBINPUT_EVENT_SWITCH_TOGGLE: {
        handleSwitchEvent(e, event);
        break;
    }
    case LIBINPUT_EVENT_RAW_TAP: {
        handleRawTapEvent(e, event);
        break;
    }
    default:{
        isSupported = false;
        break;
    }
    }
    if(isSupported)
    {
        emit sendEvent(event);
    }
}

void LibinputEventGather::handleSwipeEvent(libinput_event *e, Event &event)
{
    struct libinput_event_gesture *gestureEvent =
            libinput_event_get_gesture_event(e);
    event.event.swipeEventData.deltaX =
            libinput_event_gesture_get_dx(gestureEvent);
    event.event.swipeEventData.deltaY =
            libinput_event_gesture_get_dy(gestureEvent);
    event.event.swipeEventData.fingerCount =
            libinput_event_gesture_get_finger_count(gestureEvent);
    event.event.swipeEventData.time =
            libinput_event_gesture_get_time(gestureEvent);
    event.event.swipeEventData.cancelled =
            event.type == LIBINPUT_EVENT_GESTURE_SWIPE_END ?
                libinput_event_gesture_get_cancelled(gestureEvent) : 0;
    event.event.swipeEventData.unacceleratedDeltaX =
            libinput_event_gesture_get_dx_unaccelerated(gestureEvent);
    event.event.swipeEventData.unacceleratedDeltaY =
            libinput_event_gesture_get_dy_unaccelerated(gestureEvent);
}

void LibinputEventGather::handlePinchEvent(libinput_event *e, Event &event)
{
    struct libinput_event_gesture *gestureEvent =
            libinput_event_get_gesture_event(e);
    event.event.pinchEventData.angleDelta =
            libinput_event_gesture_get_angle_delta(gestureEvent);
    event.event.pinchEventData.cancelled =
            event.type == LIBINPUT_EVENT_GESTURE_PINCH_END ?
                libinput_event_gesture_get_cancelled(gestureEvent) : 0;
    event.event.pinchEventData.deltaX =
            libinput_event_gesture_get_dx(gestureEvent);
    event.event.pinchEventData.deltaY =
            libinput_event_gesture_get_dy(gestureEvent);
    event.event.pinchEventData.fingerCount =
            libinput_event_gesture_get_finger_count(gestureEvent);
    event.event.pinchEventData.scale =
            libinput_event_gesture_get_scale(gestureEvent);
    event.event.pinchEventData.time =
            libinput_event_gesture_get_time(gestureEvent);
    event.event.pinchEventData.unacceleratedDeltaX =
            libinput_event_gesture_get_dx_unaccelerated(gestureEvent);
    event.event.pinchEventData.unacceleratedDeltaY =
            libinput_event_gesture_get_dy_unaccelerated(gestureEvent);
}

void LibinputEventGather::handleSwitchEvent(libinput_event *e, Event &event)
{
    auto switch_event = libinput_event_get_switch_event(e);
    event.event.switchEventDate.switchType = libinput_event_switch_get_switch(switch_event);
    event.event.switchEventDate.switchState = libinput_event_switch_get_switch_state(switch_event);
}

void LibinputEventGather::handleRawTapEvent(libinput_event *e, Event &event)
{
    struct libinput_event_raw_tap *rawTapEvent =
            libinput_event_get_raw_tap_event(e);
    event.event.tapEventData.deltaX = 0;
    event.event.tapEventData.deltaY = 0;
    event.event.tapEventData.fingerCount =
            libinput_event_raw_tap_get_finger_count(rawTapEvent);
    event.event.tapEventData.time =
            libinput_event_raw_tap_get_time(rawTapEvent);
    event.event.tapEventData.cancelled = false;
}

void LibinputEventGather::handleDeviceAdded(libinput_event * e, Event & event)
{
    struct libinput_device *device = libinput_event_get_device(e);

    // 触控板设备
    if (libinput_device_config_tap_get_finger_count(device) != 0) {
        initTapToClickState(device);
    }
}

void LibinputEventGather::initTapToClickState(libinput_device *device)
{
    /* 打开触控板设备的 tap-to-click 功能，并设置 button map 为 LRN，
       这样可以接收 LIBINPUT_EVENT_RAW_TAP 事件 */
    libinput_device_config_tap_set_enabled(device, LIBINPUT_CONFIG_TAP_ENABLED);
    libinput_device_config_tap_set_button_map(device, LIBINPUT_CONFIG_TAP_MAP_LRN);
}
