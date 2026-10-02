/*
 * Copyright (C) 2013 ~ 2020 KylinSoft Ltd.
 *
 * Authors:
 *  MaChao    machao@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KMRE_WAYLAND_XDG_MANAGER
#define KMRE_WAYLAND_XDG_MANAGER

#include <QByteArray>

#include <QtGui/QWindow>
#include <QtCore/QScopedPointer>

#include <wayland-client.h>

#include "XdgActivationV1.h"
#include "singleton.h"

#include <QtWaylandClient/qtwaylandclientglobal.h>

class XdgManager : public QObject, public kmre::SingletonP<XdgManager>
{
    Q_OBJECT
public:
    void activateWindow(QWindow* window);
    void moveWindow(QWindow* window);

private:
    XdgManager();
    ~XdgManager() override;

private:
    static XdgManager* resolve(void* data) { return static_cast<XdgManager*>(data); }
    static const struct ::wl_registry_listener registryListener;
    static void handleGlobal(void *data, struct ::wl_registry *registry, uint32_t id, const char *interface, uint32_t version);
    static void handleGlobalRemove(void *data, struct ::wl_registry *registry, uint32_t id);
    void handleGlobal(::wl_registry *registry, uint32_t id, uint32_t version, const QByteArray &interface);
    void handleGlobalRemove(uint32_t id);
    void initialize();
    void destroyWaylandResources();

private:
    ::wl_display * mWlDisplay = nullptr;
    ::wl_registry *mWlRegistry = nullptr;
    ::wl_event_queue* mWlEventQueue = nullptr;
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    uint32_t mXdgActivationId = 0;
    QScopedPointer<kmre::wayland::XdgActivationV1> mXdgActivation;
#endif
#endif

    friend SingletonP<XdgManager>;
};

#endif
