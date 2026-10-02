/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#pragma once

#include <QDebug>

#include <xcb/xcb.h>

#include <cstring>

class GeEventMemMover
{
public:
    GeEventMemMover(xcb_generic_event_t *event)
        : m_event(reinterpret_cast<xcb_ge_generic_event_t *>(event))
    {
        // xcb event structs contain stuff that wasn't on the wire, the full_sequence field
        // adds an extra 4 bytes and generic events cookie data is on the wire right after the standard 32 bytes.
        // Move this data back to have the same layout in memory as it was on the wire
        // and allow casting, overwriting the full_sequence field.
        memmove((char*) m_event + 32, (char*) m_event + 36, m_event->length * 4);
    }
    ~GeEventMemMover()
    {
        // move memory layout back, so that Qt can do the same without breaking
        memmove((char*) m_event + 36, (char *) m_event + 32, m_event->length * 4);
    }

    xcb_ge_generic_event_t *operator->() const {
        return m_event;
    }

private:
    xcb_ge_generic_event_t *m_event;
};
