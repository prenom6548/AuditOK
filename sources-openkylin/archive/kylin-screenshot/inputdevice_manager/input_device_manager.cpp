/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This file is part of Kylin-Screenshot.
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
#include "input_device_manager.h"

#include <libudev.h>
#include <libinput.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <iostream>
#include <QtCore/QSocketNotifier>
#include <QtCore/QEvent>

static int open_restricted(const char *path, int flags, void *user_data)
{
    Q_UNUSED(user_data);
    int fd = open(path, flags);
    return fd < 0 ? -errno : fd;
}

static void close_restricted(int fd, void *user_data)
{
    Q_UNUSED(user_data);
    close(fd);
}

const static struct libinput_interface liInterface = {
        .open_restricted = open_restricted,
        .close_restricted = close_restricted,
};

InputDeviceManager::InputDeviceManager(QObject *parent) : QObject(parent)
{
#ifndef UNUI
    m_udev = udev_new();
    if (Q_UNLIKELY(!m_udev))
        qFatal("Failed to get udev context for libinput");

    m_li = libinput_udev_create_context(&liInterface, nullptr, m_udev);
    if (Q_UNLIKELY(!m_li))
        qFatal("Failed to get libinput context");

    if (Q_UNLIKELY(libinput_udev_assign_seat(m_li, "seat0")))
        qFatal("Failed to assign seat");

    m_liFd = libinput_get_fd(m_li);
    m_notifier.reset(new QSocketNotifier(m_liFd, QSocketNotifier::Read));

    connect(m_notifier.data(), &QSocketNotifier::activated, this, &InputDeviceManager::onReadyRead);

    onReadyRead();
#endif
}

void InputDeviceManager::onReadyRead()
{
#ifndef UNUI
    if (libinput_dispatch(m_li)) {
        qWarning("libinput_dispatch failed");
        return;
    }

    libinput_event *ev;
    while ((ev = libinput_get_event(m_li)) != nullptr) {
        processEvent(ev);
        libinput_event_destroy(ev);
    }

    return;
#endif
}

void InputDeviceManager::processEvent(libinput_event *ev)
{
#ifndef UNUI
    libinput_event_type type = libinput_event_get_type(ev);

    switch (type) {
    case LIBINPUT_EVENT_POINTER_BUTTON:
        break;
    case LIBINPUT_EVENT_POINTER_MOTION:
        break;
    case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
        break;
    case LIBINPUT_EVENT_POINTER_AXIS:
        processAxis(libinput_event_get_pointer_event(ev));
        break;
    case LIBINPUT_EVENT_KEYBOARD_KEY:
        break;
    case LIBINPUT_EVENT_TOUCH_DOWN:
        break;
    case LIBINPUT_EVENT_TOUCH_MOTION:
        break;
    case LIBINPUT_EVENT_TOUCH_UP:
        break;
    case LIBINPUT_EVENT_TOUCH_CANCEL:
        break;
    case LIBINPUT_EVENT_TOUCH_FRAME:
        break;
    default:
        break;
    }

    return;
#endif
}

void InputDeviceManager::processAxis(libinput_event_pointer *e)
{
#ifndef UNUI
    double value; // default axis value is 15 degrees per wheel click
    QPoint angleDelta;

    if (libinput_event_pointer_has_axis(e, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL)) {
        value = libinput_event_pointer_get_axis_value(e, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);
        angleDelta.setY(qRound(value));
    }

    if (libinput_event_pointer_has_axis(e, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL)) {
        value = libinput_event_pointer_get_axis_value(e, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);
        angleDelta.setX(qRound(value));
    }

    const int factor = 8;
    angleDelta *= -factor;

    emit onMouseWheel(angleDelta);

    return;
#endif
}

InputDeviceManager::~InputDeviceManager()
{
#ifndef UNUI
    if (m_li)
        libinput_unref(m_li);

    if (m_udev)
        udev_unref(m_udev);
#endif
}
