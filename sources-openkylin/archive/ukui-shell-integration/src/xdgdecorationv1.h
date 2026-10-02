// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#pragma once

#include "ukui-xdg-decoration-unstable-v1.h"
#include "ukui-xdg-shell.h"

#include <QtWaylandClient/qtwaylandclientglobal.h>

class QWindow;

namespace UKUI
{

class XdgToplevel;
class XdgToplevelDecorationV1;

class XdgDecorationManagerV1 : public zxdg_decoration_manager_v1
{
public:
    XdgDecorationManagerV1(struct ::wl_registry *registry, uint32_t id, uint32_t availableVersion);
    ~XdgDecorationManagerV1() override;
    XdgToplevelDecorationV1 *createToplevelDecoration(::xdg_toplevel *toplevel);
};

class XdgToplevelDecorationV1 : public zxdg_toplevel_decoration_v1
{
public:
    XdgToplevelDecorationV1(::zxdg_toplevel_decoration_v1 *decoration);
    ~XdgToplevelDecorationV1() override;
    void requestMode(mode mode);
    void unsetMode();
    mode pending() const;
    bool isConfigured() const;

protected:
    void zxdg_toplevel_decoration_v1_configure(uint32_t mode) override;

private:
    mode m_pending = mode_client_side;
    mode m_requested = mode_client_side;
    bool m_modeSet = false;
    bool m_configured = false;
};

} // namespace UKUI
