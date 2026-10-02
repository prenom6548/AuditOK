// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include "ukuishell.h"
#include "logging.h"
#include "ukuishellsurface.h"
#include "xdgdialogv1.h"

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

#include <QGuiApplication>

namespace UKUI
{

UkuiShell::UkuiShell(QtWaylandClient::QWaylandDisplay *display, uint32_t id, uint32_t availableVersion)
    : xdg_wm_base(display->wl_registry(), id, qMin(availableVersion, 5u))
    , m_display(display)
{
    display->addRegistryListener(&UkuiShell::handleRegistryGlobal, this);
}

UkuiShell::~UkuiShell()
{
    m_display->removeListener(&UkuiShell::handleRegistryGlobal, this);
    destroy();
}

XdgDecorationManagerV1 *UkuiShell::decorationManager()
{
    return m_xdgDecorationManager.get();
}

XdgDialogWmV1 *UkuiShell::xdgDialogWm()
{
    return m_xdgDialogWm.get();
}

Popup *UkuiShell::topmostGrabbingPopup()
{
    return m_topmostGrabbingPopup;
}

void UkuiShell::setTopmostGrabbingPopup(Popup *popup)
{
    m_topmostGrabbingPopup = popup;
}

UkuiShellSurface *UkuiShell::getUkuiShellSurface(QtWaylandClient::QWaylandWindow *window)
{
    return new UkuiShellSurface(this, get_xdg_surface(window->wlSurface()), create_surface(window->wlSurface()), window);
}

void UkuiShell::xdg_wm_base_ping(uint32_t serial)
{
    pong(serial);
}

void UkuiShell::handleRegistryGlobal(void *data, wl_registry *registry, uint id, const QString &interface, uint version)
{
    UkuiShell *xdgShell = static_cast<UkuiShell *>(data);
    if (interface == QLatin1String(ukui_shell::interface()->name)) {
        xdgShell->ukui_shell::init(registry, id, qMin(version, 1u));
        // We don't want toplevels to be placed at fixed positions
        QtWaylandClient::QWaylandWindow::fixedToplevelPositions = false;
    } else if (interface == QLatin1String(XdgDecorationManagerV1::interface()->name)) {
        xdgShell->m_xdgDecorationManager.reset(new XdgDecorationManagerV1(registry, id, version));
    } else if (interface == QLatin1String(XdgDialogWmV1::interface()->name)) {
        xdgShell->m_xdgDialogWm.reset(new XdgDialogWmV1(registry, id, version));
    }
}
}