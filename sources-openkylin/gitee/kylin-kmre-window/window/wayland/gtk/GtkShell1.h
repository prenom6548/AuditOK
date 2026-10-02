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

#ifndef KMRE_WAYLAND_GTK_SHELL1
#define KMRE_WAYLAND_GTK_SHELL1

#include <QObject>

#include "qwayland-gtk-shell.h"

#include <QtWaylandClient/qtwaylandclientglobal.h>

namespace kmre {
namespace wayland {

class GtkSurface1 : public QObject, public QtWayland::gtk_surface1
{
    Q_OBJECT
public:
    ~GtkSurface1() override;
};

class GtkShell1 : public QtWayland::gtk_shell1
{
public:
    GtkShell1(struct ::wl_registry *registry, uint32_t id, uint32_t availableVersion);
    ~GtkShell1() override;

    GtkSurface1* getGtkSurface1(struct ::wl_surface *surface, struct ::wl_event_queue *queue);
};

}
}

#endif
