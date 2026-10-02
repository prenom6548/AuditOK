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

#include "GtkShell1.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandinputdevice_p.h>
#endif

namespace kmre {
namespace wayland {
GtkSurface1::~GtkSurface1()
{
    if (isInitialized()) {
        release();
    }
}

GtkShell1::GtkShell1(wl_registry *registry,
        uint32_t id,
        uint32_t availableVersion)
    : QtWayland::gtk_shell1(registry, id, availableVersion)
{
}

GtkShell1::~GtkShell1()
{
    if (isInitialized()) {
        destroy();
    }
}

GtkSurface1* GtkShell1::getGtkSurface1(struct ::wl_surface *surface, struct ::wl_event_queue *queue)
{
    auto wl = get_gtk_surface(surface);

    if (queue) {
        wl_proxy_set_queue((wl_proxy*)wl, queue);
    }

    auto provider = new GtkSurface1;
    provider->init(wl);

    return provider;
}

}
}
