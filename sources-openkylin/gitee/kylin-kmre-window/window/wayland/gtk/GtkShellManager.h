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

#ifndef KMRE_WAYLAND_GTK_SHELL_MANAGER
#define KMRE_WAYLAND_GTK_SHELL_MANAGER

#include <QByteArray>

#include <QtGui/QWindow>
#include <QtCore/QScopedPointer>

#include <wayland-client.h>

#include "GtkShell1.h"
#include "singleton.h"

#include <QtWaylandClient/qtwaylandclientglobal.h>

class GtkShellManager : public QObject, public kmre::SingletonP<GtkShellManager>
{
    Q_OBJECT
public:
    void activateWindow(QWindow* window);

private:
    GtkShellManager();
    ~GtkShellManager() override;

private:
    static GtkShellManager* resolve(void* data) { return static_cast<GtkShellManager*>(data); }
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
    uint32_t mGtkShellId = 0;
    QScopedPointer<kmre::wayland::GtkShell1> mGtkShell;

    friend SingletonP<GtkShellManager>;
};

#endif
