// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include "ukuishellintegration.h"
#include "logging.h"
#include "ukuishellsurface.h"
#include "xdgdecorationv1.h"

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

namespace UKUI
{

bool UkuiShellIntegration::initialize(QtWaylandClient::QWaylandDisplay *display)
{
    for (auto global : display->globals()) {
        if (global.interface == QLatin1String("xdg_wm_base")) {
            m_ukuiShell.reset(new UkuiShell(display, global.id, global.version));
            break;
        }
    }

    if (!m_ukuiShell) {
        qCDebug(UKUI_SHELL) << "Couldn't find global xdg_wm_base for xdg-shell stable";
        return false;
    }

    return QWaylandShellIntegration::initialize(display);
}

QtWaylandClient::QWaylandShellSurface *UkuiShellIntegration::createShellSurface(QtWaylandClient::QWaylandWindow *window)
{
    return m_ukuiShell->getUkuiShellSurface(window);
}

}
