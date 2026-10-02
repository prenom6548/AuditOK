// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#pragma once

#include "ukui-ukui-shell.h"

#include "xdgdecorationv1.h"

#include <QRegion>

#include <memory>

namespace QtWaylandClient
{
class QWaylandDisplay;
class QWaylandWindow;
}

namespace UKUI
{

class Popup;
class Toplevel;
class UkuiShellSurface;
class XdgDialogWmV1;

class UkuiShell : public ukui_shell, public xdg_wm_base
{
public:
    UkuiShell(QtWaylandClient::QWaylandDisplay *display, uint32_t id, uint32_t availableVersion);
    ~UkuiShell() override;

    XdgDecorationManagerV1 *decorationManager();
    XdgDialogWmV1 *xdgDialogWm();

    Popup *topmostGrabbingPopup();
    void setTopmostGrabbingPopup(Popup *popup);

    UkuiShellSurface *getUkuiShellSurface(QtWaylandClient::QWaylandWindow *window);

protected:
    void xdg_wm_base_ping(uint32_t serial) override;

private:
    static void handleRegistryGlobal(void *data, ::wl_registry *registry, uint id,
                                     const QString &interface, uint version);

    QtWaylandClient::QWaylandDisplay *m_display = nullptr;

    std::unique_ptr<XdgDecorationManagerV1> m_xdgDecorationManager;
    std::unique_ptr<XdgDialogWmV1> m_xdgDialogWm;

    Popup *m_topmostGrabbingPopup = nullptr;
};
}