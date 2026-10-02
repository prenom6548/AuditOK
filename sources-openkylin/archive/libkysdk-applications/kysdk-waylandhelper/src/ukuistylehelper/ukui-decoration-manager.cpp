/*
 * libkysdk-waylandhelper's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#include "ukui-decoration-manager.h"

#include "ukui-decoration-client.h"
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/surface.h>
#include <QApplication>
#include <QDebug>

static const u_int32_t ukui_decoration_surpported_version = 1;
static UKUIDecorationManager *global_instance = nullptr;

static wl_display *display = nullptr;
static ukui_decoration *ukui_decoration_manager = nullptr;

static void handle_global(void *data, struct wl_registry *registry,
                          uint32_t name, const char *interface, uint32_t version)
{
    if (strcmp(interface, ukui_decoration_interface.name) == 0) {
        u_int32_t surpportedVersion = qMin(ukui_decoration_surpported_version, version);
        ukui_decoration_manager = (ukui_decoration *)wl_registry_bind(registry, name, &ukui_decoration_interface, surpportedVersion);
    }
}

static void handle_global_remove(void *data, struct wl_registry *registry,
                                 uint32_t name)
{
    // Who cares
}

static const struct wl_registry_listener registry_listener = {
    .global = handle_global,
    .global_remove = handle_global_remove,
};

UKUIDecorationManager *UKUIDecorationManager::getInstance()
{
    if (!global_instance)
        global_instance = new UKUIDecorationManager;
    return global_instance;
}

bool UKUIDecorationManager::supportUKUIDecoration()
{
    return ukui_decoration_manager;
}

bool UKUIDecorationManager::moveWindow(QWindow *windowHandle)
{
    if (!supportUKUIDecoration())
        return false;

    auto kde_surface = KWayland::Client::Surface::fromWindow(windowHandle);
    if (!kde_surface)
        return false;
    wl_surface *surface = *kde_surface;
    if (!surface)
        return false;

    ukui_decoration_move_surface(ukui_decoration_manager, surface);
    wl_surface_commit(surface);
    wl_display_roundtrip(display);
    return true;
}

bool UKUIDecorationManager::removeHeaderBar(QWindow *windowHandle)
{
    if (!supportUKUIDecoration())
        return false;

    auto kde_surface = KWayland::Client::Surface::fromWindow(windowHandle);
    if (!kde_surface) {
        qDebug() << "UKUIDecorationManager::get kde_surface failed!";
        return false;
    }
    wl_surface *surface = *kde_surface;
    if (!surface) {
        qDebug() << "UKUIDecorationManager::get wl_surface failed!";
        return false;
    }
    ukui_decoration_set_ukui_decoration_mode(ukui_decoration_manager, surface, 1);
    wl_surface_commit(surface);
    return true;
}

bool UKUIDecorationManager::setCornerRadius(QWindow *windowHandle, int topleft, int topright, int bottomleft, int bottomright)
{
    if (!supportUKUIDecoration())
        return false;

    auto kde_surface = KWayland::Client::Surface::fromWindow(windowHandle);
    if (!kde_surface)
        return false;
    wl_surface *surface = *kde_surface;
    if (!surface)
        return false;

    ukui_decoration_set_unity_border_radius(ukui_decoration_manager, surface, topleft, topright, bottomleft, bottomright);
    wl_surface_commit(surface);
    wl_display_roundtrip(display);
    return true;
}

UKUIDecorationManager::UKUIDecorationManager()
{
    auto connectionThread = KWayland::Client::ConnectionThread::fromApplication(qApp);
    display = connectionThread->display();

    struct wl_registry *registry = wl_display_get_registry(display);

    // get ukui_decoration_manager
    wl_registry_add_listener(registry, &registry_listener, nullptr);
    wl_display_roundtrip(display);
}
