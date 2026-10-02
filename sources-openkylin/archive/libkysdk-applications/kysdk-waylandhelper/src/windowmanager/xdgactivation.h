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

#ifndef XDGACTIVATION_H
#define XDGACTIVATION_H

#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/seat.h>
#include <KWayland/Client/surface.h>
#include <QObject>

#include "wayland-xdg-activation-v1-client-protocol.h"

/**
 * @since 2.5.1.0
 */

using namespace KWayland::Client;

class XdgActivationToken;

class XdgActivation : public QObject
{
    Q_OBJECT
public:
    explicit XdgActivation(QObject *parent = nullptr);
    ~XdgActivation();

    bool isValid() const;

    void release();

    void destroy();

    void setup(xdg_activation_v1 *activation);

    void setEventQueue(EventQueue *queue);

    EventQueue *eventQueue();

    operator xdg_activation_v1 *();
    operator xdg_activation_v1 *() const;

    XdgActivationToken *getXdgActivationToken();
    XdgActivationToken *requestXdgActivationToken(wl_seat *seat, Surface *surface, uint32_t serial, const QString &app_id);
    void activateWindow(const QString &token, Surface *surface);

private:
    class Private;
    QScopedPointer<Private> d;
};

class XdgActivationToken : public QObject
{
    Q_OBJECT
public:
    explicit XdgActivationToken(QObject *parent = nullptr);
    ~XdgActivationToken();

    bool isValid() const;

    void release();

    void destroy();

    void setup(xdg_activation_token_v1 *token);

    void set_serial(uint32_t serial, wl_seat *seat);

    void set_app_id(const QString &app_id);

    void set_surface(Surface *surface);

    void commit();

    operator xdg_activation_token_v1 *();
    operator xdg_activation_token_v1 *() const;

Q_SIGNALS:
    void done(const QString &token);

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // XDGACTIVATION_H
