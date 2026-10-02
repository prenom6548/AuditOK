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

#include "XdgManager.h"

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
#include <QDebug>

#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
using kmre::wayland::XdgActivationTokenV1;
using kmre::wayland::XdgActivationV1;
#endif
#endif

const struct ::wl_registry_listener XdgManager::registryListener = {
    XdgManager::handleGlobal,
    XdgManager::handleGlobalRemove
};



void XdgManager::handleGlobal(void *data, ::wl_registry *registry, uint32_t id, const char *interface, uint32_t version)
{
    Q_UNUSED(registry);
    Q_UNUSED(version);
    resolve(data)->handleGlobal(registry, id, version, QByteArray(interface));
}

void XdgManager::handleGlobal(::wl_registry *registry, uint32_t id, uint32_t version, const QByteArray &interface)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    if (interface == XdgActivationV1::interface()->name) {
        mXdgActivationId = id;
        mXdgActivation.reset(new XdgActivationV1(registry, id, version));

        auto p = mXdgActivation.get();
        if (p) {
            wl_proxy_set_queue((wl_proxy*)p->object(), mWlEventQueue);
        }
    }
#endif
#endif
}

void XdgManager::handleGlobalRemove(void *data, ::wl_registry *registry, uint32_t id)
{
    Q_UNUSED(registry);
    resolve(data)->handleGlobalRemove(id);
}

void XdgManager::handleGlobalRemove(uint32_t id)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    if (mXdgActivationId == id) {
        mXdgActivationId = 0;
        mXdgActivation.reset(nullptr);
    }
#endif
#endif
}

void XdgManager::initialize()
{
    if (!qGuiApp) {
        syslog(LOG_ERR, "XdgManager: Requires initialized QGuiApplication.");
        return;
    }

    if (!mWlDisplay) {
        mWlDisplay = (wl_display*) qGuiApp->platformNativeInterface()->nativeResourceForIntegration(QByteArrayLiteral("wl_display"));
    }
    if (!mWlDisplay) {
        syslog(LOG_ERR, "XdgManager: Requires wayland display.");
        return;
    }

    if (!mWlEventQueue) {
        mWlEventQueue = wl_display_create_queue(mWlDisplay);
    }
    if (!mWlEventQueue) {
        syslog(LOG_ERR, "XdgManager: Failed to create event queue.");
        goto error_out;
    }

    if (!mWlRegistry) {
        mWlRegistry = wl_display_get_registry(mWlDisplay);
    }
    if (!mWlRegistry) {
        syslog(LOG_ERR, "XdgManager: Failed to get registry.");
        goto error_out;
    }

    wl_proxy_set_queue((wl_proxy*) mWlRegistry, mWlEventQueue);

    wl_registry_add_listener(mWlRegistry, &XdgManager::registryListener, this);

    wl_display_roundtrip_queue(mWlDisplay, mWlEventQueue);

    return;

error_out:
    destroyWaylandResources();
}

XdgManager::XdgManager()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    mXdgActivation.reset(nullptr);
#endif
#endif
    initialize();
}

void XdgManager::destroyWaylandResources()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    mXdgActivation.reset(nullptr);
#endif
#endif

    if (mWlRegistry) {
        wl_registry_destroy(mWlRegistry);
        mWlRegistry = nullptr;
    }

    if (mWlEventQueue) {
        wl_event_queue_destroy(mWlEventQueue);
        mWlEventQueue = nullptr;
    }
}

XdgManager::~XdgManager()
{
    destroyWaylandResources();
}

void XdgManager::activateWindow(QWindow* window)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    QtWaylandClient::QWaylandWindow* wlWindow = nullptr;
    QString appName;
    uint32_t serial = 0;

    if (!window) {
        return;
    }

    if (!qApp) {
        return;
    }

    appName = qApp->applicationDisplayName();

    wlWindow = dynamic_cast<QtWaylandClient::QWaylandWindow*>(window->handle());
    if (!wlWindow) {
        return;
    }

    if (!mXdgActivation) {
        initialize();
    }

    if (!mXdgActivation) {
        syslog(LOG_ERR, "XdgManager: Failed to initialize XdgActivationV1.");
        return;
    }

    const auto seat = wlWindow->display()->lastInputDevice();
    if (seat) {
        serial = seat->serial();
    }

    const auto tokenProvider = mXdgActivation->requestXdgActivationToken(wlWindow->display(), wlWindow->wlSurface(), serial, appName);
    connect(tokenProvider, &XdgActivationTokenV1::done, this,
        [this, tokenProvider, wlWindow](const QString &token) {
            this->mXdgActivation->activate(token, wlWindow->wlSurface());
            wl_display_roundtrip_queue(this->mWlDisplay, this->mWlEventQueue);
            delete tokenProvider;
    });
    wl_display_roundtrip_queue(mWlDisplay, mWlEventQueue);
#endif
#else
    QtWaylandClient::QWaylandWindow* wlWindow = nullptr;

    if (!window) {
        return;
    }

    wlWindow = dynamic_cast<QtWaylandClient::QWaylandWindow*>(window->handle());
    if (!wlWindow) {
        return;
    }

    wlWindow->requestActivateWindow();
#endif
}

void XdgManager::moveWindow(QWindow* window)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    QtWaylandClient::QWaylandWindow* wlWindow = nullptr;
    QtWaylandClient::QWaylandDisplay* display = nullptr;
    QtWaylandClient::QWaylandShellSurface* shellSurface = nullptr;

    if (!window) {
        return;
    }

    wlWindow = dynamic_cast<QtWaylandClient::QWaylandWindow*>(window->handle());
    if (!wlWindow) {
        return;
    }

    shellSurface = wlWindow->shellSurface();
    if (!shellSurface) {
        return;
    }

    display = wlWindow->display();
    if (!display) {
        return;
    }

    shellSurface->move(display->currentInputDevice());
#endif
}
