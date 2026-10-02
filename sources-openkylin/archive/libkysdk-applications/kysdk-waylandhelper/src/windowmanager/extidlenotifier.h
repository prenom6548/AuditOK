/*
 * libkysdk-waylandhelper's Library
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef EXTIDLENOTIFIER_H
#define EXTIDLENOTIFIER_H

#include <KWayland/Client/seat.h>
#include <KWayland/Client/surface.h>
#include <QObject>

#include "wayland-ext-idle-notify-v1-client-protocol.h"

using namespace KWayland::Client;

/**
 * @since 2.5.1.0
 */

class ExtIdleNotification;

class ExtIdleNotifier : public QObject
{
    Q_OBJECT
public:
    explicit ExtIdleNotifier(QObject *parent = nullptr);
    ~ExtIdleNotifier();

    bool isValid() const;

    void release();

    void destroy();

    void setup(ext_idle_notifier_v1 *notifier);

    operator ext_idle_notifier_v1 *();
    operator ext_idle_notifier_v1 *() const;

    // static inline struct ext_idle_notification_v1 *
    // ext_idle_notifier_v1_get_idle_notification(struct ext_idle_notifier_v1 *ext_idle_notifier_v1, uint32_t timeout, struct wl_seat *seat)

    ExtIdleNotification *requestExtIdleNotification(wl_seat *seat, int msec);

private:
    class Private;
    QScopedPointer<Private> d;
};

class ExtIdleNotification : public QObject
{
    Q_OBJECT
public:
    explicit ExtIdleNotification(QObject *parent = nullptr);
    ~ExtIdleNotification();

    bool isValid() const;

    void release();

    void destroy();

    void setup(ext_idle_notification_v1 *notification);

    operator ext_idle_notification_v1 *();
    operator ext_idle_notification_v1 *() const;

Q_SIGNALS:
    void idled();
    void resumed();

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // EXTIDLENOTIFIER_H
