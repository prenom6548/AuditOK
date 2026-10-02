/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#include "ukui-decoration-manager.h"

#include "ukui-decoration-client.h"

#include <QApplication>
#include <qpa/qplatformnativeinterface.h>
#include <KWayland/Client/connection_thread.h>

static UKUIDecorationManager *global_instance = nullptr;

static wl_display *display = nullptr;
static ukui_decoration *ukui_decoration_manager = nullptr;

static void handle_global(void *data, struct wl_registry *registry,
                          uint32_t name, const char *interface, uint32_t version) {
    if (strcmp(interface, ukui_decoration_interface.name) == 0) {
        ukui_decoration_manager = (ukui_decoration *) wl_registry_bind(registry, name, &ukui_decoration_interface, version);
    }
}

static void handle_global_remove(void *data, struct wl_registry *registry,
                                 uint32_t name) {
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

    auto nativeInterface = qApp->platformNativeInterface();
    wl_surface *surface = reinterpret_cast<wl_surface *>(nativeInterface->nativeResourceForWindow(QByteArrayLiteral("surface"), windowHandle));
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

    auto nativeInterface = qApp->platformNativeInterface();
    wl_surface *surface = reinterpret_cast<wl_surface *>(nativeInterface->nativeResourceForWindow(QByteArrayLiteral("surface"), windowHandle));
    if (!surface)
        return false;

    ukui_decoration_set_ukui_decoration_mode(ukui_decoration_manager, surface, 1);
    wl_surface_commit(surface);
    wl_display_roundtrip(display);
    return true;
}

bool UKUIDecorationManager::setCornerRadius(QWindow *windowHandle, int topleft, int topright, int bottomleft, int bottomright)
{
    if (!supportUKUIDecoration())
        return false;

    auto nativeInterface = qApp->platformNativeInterface();
    wl_surface *surface = reinterpret_cast<wl_surface *>(nativeInterface->nativeResourceForWindow(QByteArrayLiteral("surface"), windowHandle));
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
