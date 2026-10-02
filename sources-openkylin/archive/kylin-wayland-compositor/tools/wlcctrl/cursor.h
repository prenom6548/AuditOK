// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _CURSOR_H_
#define _CURSOR_H_

#include <stdbool.h>

#include "output.h"

#include "wlr-virtual-pointer-unstable-v1-client-protocol.h"

typedef struct _cursor {
    struct wl_display *display;
    struct zwlr_virtual_pointer_v1 *zwlr_virtual_pointer_v1;
    const output *output;
} cursor;

void cursor_axis_set(cursor *pointer, int32_t dx, int32_t dy);

void cursor_button_click(cursor *pointer, uint32_t button);

void cursor_button_press(cursor *pointer, uint32_t button, bool press);

void cursor_set_motion_absolute(cursor *pointer, uint32_t pos_x, uint32_t pos_y);

void cursor_destroy(cursor *pointer);

#endif
