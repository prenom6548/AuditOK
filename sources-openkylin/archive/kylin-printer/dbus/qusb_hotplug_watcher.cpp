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

#include "qusb_hotplug_watcher.h"
#include <QDebug>
#include <poll.h>

QUsbHotplugWatcher::QUsbHotplugWatcher(QObject *parent)
    : QObject(parent)
{
    if (libusb_init(&m_ctx) != 0) {
        qWarning() << "Failed to initialize libusb";
        return;
    }

    if (!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) {
        qWarning() << "libusb hotplug not supported on this system";
        libusb_exit(m_ctx);
        m_ctx = nullptr;
        return;
    }

    int rc = libusb_hotplug_register_callback(
        m_ctx,
        static_cast<libusb_hotplug_event>(
            LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED |
            LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT),
        LIBUSB_HOTPLUG_NO_FLAGS,
        LIBUSB_HOTPLUG_MATCH_ANY,
        LIBUSB_HOTPLUG_MATCH_ANY,
        LIBUSB_HOTPLUG_MATCH_ANY,
        hotplugCallback,
        this,
        &m_cbHandle);

    if (rc != LIBUSB_SUCCESS) {
        qWarning() << "Failed to register hotplug callback:" << libusb_error_name(rc);
        libusb_exit(m_ctx);
        m_ctx = nullptr;
        return;
    }

    setupNotifiers();
}

QUsbHotplugWatcher::~QUsbHotplugWatcher()
{
    cleanup();
}

void QUsbHotplugWatcher::cleanup()
{
    if (m_ctx) {
        libusb_hotplug_deregister_callback(m_ctx, m_cbHandle);

        for (auto notifier : m_notifiers) {
            delete notifier;
        }
        m_notifiers.clear();

        libusb_exit(m_ctx);
        m_ctx = nullptr;
    }
}

int QUsbHotplugWatcher::hotplugCallback(libusb_context *ctx,
                                        libusb_device *dev,
                                        libusb_hotplug_event event,
                                        void *user_data)
{
    QUsbHotplugWatcher *self = static_cast<QUsbHotplugWatcher*>(user_data);
    if (!self) return 0;

    libusb_device_descriptor desc;
    if (libusb_get_device_descriptor(dev, &desc) != 0) {
        return 0;
    }

    if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) {
        emit self->deviceAdded(desc.idVendor, desc.idProduct);
    } else if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
        emit self->deviceRemoved(desc.idVendor, desc.idProduct);
    }
    return 0;
}

void QUsbHotplugWatcher::setupNotifiers()
{
    const struct libusb_pollfd **pollfds = libusb_get_pollfds(m_ctx);
    if (!pollfds) {
        qWarning() << "Failed to get libusb pollfds";
        return;
    }

    for (int i = 0; pollfds[i] != nullptr; i++) {
        int fd = pollfds[i]->fd;
        short events = pollfds[i]->events;

        if (events & POLLIN) {
            QSocketNotifier *notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
            connect(notifier, &QSocketNotifier::activated, this, [=](int){
                timeval tv{0,0};
                libusb_handle_events_timeout(m_ctx, &tv);
            });
            m_notifiers.append(notifier);
        }
        if (events & POLLOUT) {
            QSocketNotifier *notifier = new QSocketNotifier(fd, QSocketNotifier::Write, this);
            connect(notifier, &QSocketNotifier::activated, this, [=](int){
                timeval tv{0,0};
                libusb_handle_events_timeout(m_ctx, &tv);
            });
            m_notifiers.append(notifier);
        }
    }

    libusb_free_pollfds(pollfds);
}
