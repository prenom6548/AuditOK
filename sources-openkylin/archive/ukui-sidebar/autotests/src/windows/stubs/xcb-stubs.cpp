/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "xcb/xcb.h"

#include <QtGlobal>

#include <cstdlib>

namespace {
XcbStubState g_state;
xcb_connection_t g_connection;
}

XcbStubState &xcbStubState()
{
    return g_state;
}

void resetXcbStubState()
{
    g_state = XcbStubState();
}

xcb_intern_atom_cookie_t xcb_intern_atom_unchecked(xcb_connection_t *c,
                                                   uint8_t only_if_exists,
                                                   uint16_t name_len,
                                                   const char *name)
{
    Q_UNUSED(c)
    Q_UNUSED(only_if_exists)
    Q_UNUSED(name_len)
    Q_UNUSED(name)
    ++g_state.internCalls;
    return {1};
}

xcb_intern_atom_reply_t *xcb_intern_atom_reply(xcb_connection_t *c,
                                               xcb_intern_atom_cookie_t cookie,
                                               void *e)
{
    Q_UNUSED(c)
    Q_UNUSED(cookie)
    Q_UNUSED(e)
    if (!g_state.atomReplyAvailable) {
        return nullptr;
    }

    auto *reply = static_cast<xcb_intern_atom_reply_t *>(std::calloc(1, sizeof(xcb_intern_atom_reply_t)));
    reply->atom = 42;
    return reply;
}

void xcb_change_property(xcb_connection_t *c,
                         uint8_t mode,
                         xcb_window_t window,
                         xcb_atom_t property,
                         xcb_atom_t type,
                         uint8_t format,
                         uint32_t data_len,
                         const void *data)
{
    Q_UNUSED(c)
    Q_UNUSED(mode)
    Q_UNUSED(type)
    ++g_state.changeCalls;
    g_state.lastWindow = window;
    g_state.lastAtom = property;
    g_state.lastFormat = format;
    g_state.lastDataLength = data_len;
    if (data && data_len > 0) {
        const auto *values = static_cast<const uint32_t *>(data);
        g_state.lastRadius = values[data_len - 1];
    }
}

void xcb_delete_property(xcb_connection_t *c, xcb_window_t window, xcb_atom_t property)
{
    Q_UNUSED(c)
    ++g_state.deleteCalls;
    g_state.lastWindow = window;
    g_state.lastAtom = property;
}

int xcb_flush(xcb_connection_t *c)
{
    Q_UNUSED(c)
    ++g_state.flushCalls;
    return 0;
}

xcb_connection_t *xcbStubConnection()
{
    return g_state.connectionAvailable ? &g_connection : nullptr;
}
