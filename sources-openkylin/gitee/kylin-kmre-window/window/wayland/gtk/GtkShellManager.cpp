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

#include "GtkShellManager.h"

#include <QtGui/QGuiApplication>
#include <QtGui/private/qguiapplication_p.h>
#include <qpa/qplatformnativeinterface.h>
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylandintegration_p.h>
#include <QtWaylandClient/private/qwaylandshellsurface_p.h>
#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandinputdevice_p.h>
#endif

#include <sys/syslog.h>

using kmre::wayland::GtkSurface1;
using kmre::wayland::GtkShell1;

const struct ::wl_registry_listener GtkShellManager::registryListener = {
    GtkShellManager::handleGlobal,
    GtkShellManager::handleGlobalRemove
};



void GtkShellManager::handleGlobal(void *data, ::wl_registry *registry, uint32_t id, const char *interface, uint32_t version)
{
    Q_UNUSED(registry);
    Q_UNUSED(version);
    resolve(data)->handleGlobal(registry, id, version, QByteArray(interface));
}

void GtkShellManager::handleGlobal(::wl_registry *registry, uint32_t id, uint32_t version, const QByteArray &interface)
{
    if (interface == GtkShell1::interface()->name) {
        mGtkShellId = id;
        mGtkShell.reset(new GtkShell1(registry, id, version));

        auto p = mGtkShell.get();
        if (p) {
            wl_proxy_set_queue((wl_proxy*)p->object(), mWlEventQueue);
        }
    }
}

void GtkShellManager::handleGlobalRemove(void *data, ::wl_registry *registry, uint32_t id)
{
    Q_UNUSED(registry);
    resolve(data)->handleGlobalRemove(id);
}

void GtkShellManager::handleGlobalRemove(uint32_t id)
{
    if (mGtkShellId == id) {
        mGtkShellId = 0;
        mGtkShell.reset(nullptr);
    }
}

void GtkShellManager::initialize()
{
    if (!qGuiApp) {
        syslog(LOG_ERR, "GtkShellManager: Requires initialized QGuiApplication.");
        return;
    }

    if (!mWlDisplay) {
        mWlDisplay = (wl_display*) qGuiApp->platformNativeInterface()->nativeResourceForIntegration(QByteArrayLiteral("wl_display"));
    }
    if (!mWlDisplay) {
        syslog(LOG_ERR, "GtkShellManager: Requires wayland display.");
        return;
    }

    if (!mWlEventQueue) {
        mWlEventQueue = wl_display_create_queue(mWlDisplay);
    }
    if (!mWlEventQueue) {
        syslog(LOG_ERR, "GtkShellManager: Failed to create event queue.");
        goto error_out;
    }

    if (!mWlRegistry) {
        mWlRegistry = wl_display_get_registry(mWlDisplay);
    }
    if (!mWlRegistry) {
        syslog(LOG_ERR, "GtkShellManager: Failed to get registry.");
        goto error_out;
    }

    wl_proxy_set_queue((wl_proxy*) mWlRegistry, mWlEventQueue);

    wl_registry_add_listener(mWlRegistry, &GtkShellManager::registryListener, this);

    wl_display_roundtrip_queue(mWlDisplay, mWlEventQueue);

    return;

error_out:
    destroyWaylandResources();
}

GtkShellManager::GtkShellManager()
{
    mGtkShell.reset(nullptr);
    initialize();
}

void GtkShellManager::destroyWaylandResources()
{
    mGtkShell.reset(nullptr);

    if (mWlRegistry) {
        wl_registry_destroy(mWlRegistry);
        mWlRegistry = nullptr;
    }

    if (mWlEventQueue) {
        wl_event_queue_destroy(mWlEventQueue);
        mWlEventQueue = nullptr;
    }
}

GtkShellManager::~GtkShellManager()
{
    destroyWaylandResources();
}

void GtkShellManager::activateWindow(QWindow* window)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    QtWaylandClient::QWaylandWindow* wlWindow = nullptr;

    if (!window) {
        return;
    }

    wlWindow = dynamic_cast<QtWaylandClient::QWaylandWindow*>(window->handle());
    if (!wlWindow) {
        return;
    }

    if (!mGtkShell) {
        initialize();
    }

    if (!mGtkShell) {
        syslog(LOG_ERR, "GtkShellManager: Failed to activate window.");
        return;
    }

    const auto surface = mGtkShell->getGtkSurface1(wlWindow->wlSurface(), mWlEventQueue);
    surface->present(0);
    wl_display_roundtrip_queue(mWlDisplay, mWlEventQueue);
    delete surface;
#endif
}
