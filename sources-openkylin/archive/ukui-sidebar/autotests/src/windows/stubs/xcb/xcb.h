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

#ifndef AUTOTESTS_WINDOWS_STUB_XCB_H
#define AUTOTESTS_WINDOWS_STUB_XCB_H

#include <cstddef>
#include <cstdint>

typedef struct xcb_connection_t {
    int dummy;
} xcb_connection_t;

typedef uint32_t xcb_window_t;
typedef uint32_t xcb_atom_t;

typedef struct xcb_intern_atom_cookie_t {
    unsigned int sequence;
} xcb_intern_atom_cookie_t;

typedef struct xcb_intern_atom_reply_t {
    uint8_t response_type;
    uint8_t pad0;
    uint16_t sequence;
    uint32_t length;
    xcb_atom_t atom;
} xcb_intern_atom_reply_t;

enum {
    XCB_PROP_MODE_REPLACE = 0,
    XCB_ATOM_CARDINAL = 6
};

struct XcbStubState
{
    bool connectionAvailable = false;
    bool atomReplyAvailable = true;
    int internCalls = 0;
    int changeCalls = 0;
    int deleteCalls = 0;
    int flushCalls = 0;
    uint32_t lastWindow = 0;
    uint32_t lastAtom = 0;
    uint32_t lastFormat = 0;
    uint32_t lastDataLength = 0;
    uint32_t lastRadius = 0;
};

XcbStubState &xcbStubState();
void resetXcbStubState();

xcb_intern_atom_cookie_t xcb_intern_atom_unchecked(xcb_connection_t *c,
                                                   uint8_t only_if_exists,
                                                   uint16_t name_len,
                                                   const char *name);
xcb_intern_atom_reply_t *xcb_intern_atom_reply(xcb_connection_t *c,
                                               xcb_intern_atom_cookie_t cookie,
                                               void *e);
void xcb_change_property(xcb_connection_t *c,
                         uint8_t mode,
                         xcb_window_t window,
                         xcb_atom_t property,
                         xcb_atom_t type,
                         uint8_t format,
                         uint32_t data_len,
                         const void *data);
void xcb_delete_property(xcb_connection_t *c, xcb_window_t window, xcb_atom_t property);
int xcb_flush(xcb_connection_t *c);

#endif // AUTOTESTS_WINDOWS_STUB_XCB_H
