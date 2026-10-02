// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include "xdgdecorationv1.h"
#include "ukuishellsurface.h"

namespace UKUI
{

XdgDecorationManagerV1::XdgDecorationManagerV1(wl_registry *registry, uint32_t id, uint32_t availableVersion)
    : zxdg_decoration_manager_v1(registry, id, qMin(availableVersion, 1u))
{
}

XdgDecorationManagerV1::~XdgDecorationManagerV1()
{
    Q_ASSERT(isInitialized());
    destroy();
}

XdgToplevelDecorationV1 *XdgDecorationManagerV1::createToplevelDecoration(::xdg_toplevel *toplevel)
{
    Q_ASSERT(toplevel);
    return new XdgToplevelDecorationV1(get_toplevel_decoration(toplevel));
}

XdgToplevelDecorationV1::XdgToplevelDecorationV1(::zxdg_toplevel_decoration_v1 *decoration)
    : zxdg_toplevel_decoration_v1(decoration)
{
}

XdgToplevelDecorationV1::~XdgToplevelDecorationV1()
{
    Q_ASSERT(isInitialized());
    destroy();
}

void XdgToplevelDecorationV1::requestMode(zxdg_toplevel_decoration_v1::mode mode)
{
    // According to the spec the client is responsible for not requesting a mode repeatedly.
    if (m_modeSet && m_requested == mode)
        return;

    set_mode(mode);
    m_requested = mode;
    m_modeSet = true;
}

void XdgToplevelDecorationV1::unsetMode()
{
    unset_mode();
    m_modeSet = false;
    m_requested = mode_client_side;
}

XdgToplevelDecorationV1::mode XdgToplevelDecorationV1::pending() const
{
    return m_pending;
}

bool XdgToplevelDecorationV1::isConfigured() const
{
    return m_configured;
}

void XdgToplevelDecorationV1::zxdg_toplevel_decoration_v1_configure(uint32_t mode)
{
    m_pending = zxdg_toplevel_decoration_v1::mode(mode);
    m_configured = true;
}

} // namespace UKUI
