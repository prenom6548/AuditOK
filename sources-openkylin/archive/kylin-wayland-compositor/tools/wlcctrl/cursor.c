// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include "cursor.h"
#include "util/time.h"

void cursor_axis_set(cursor *cursor, int32_t dx, int32_t dy)
{
    if (!cursor->zwlr_virtual_pointer_v1) {
        return;
    }

    uint32_t time = current_time_msec();
    zwlr_virtual_pointer_v1_axis_source(cursor->zwlr_virtual_pointer_v1,
                                        WL_POINTER_AXIS_SOURCE_FINGER);
    if (dx) {
        zwlr_virtual_pointer_v1_axis(cursor->zwlr_virtual_pointer_v1, time,
                                     WL_POINTER_AXIS_HORIZONTAL_SCROLL, wl_fixed_from_int(dx));
    }
    if (dy) {
        zwlr_virtual_pointer_v1_axis(cursor->zwlr_virtual_pointer_v1, time,
                                     WL_POINTER_AXIS_VERTICAL_SCROLL, wl_fixed_from_int(dy));
    }
    zwlr_virtual_pointer_v1_frame(cursor->zwlr_virtual_pointer_v1);

    uint32_t time_stop = current_time_msec();
    zwlr_virtual_pointer_v1_axis_source(cursor->zwlr_virtual_pointer_v1,
                                        WL_POINTER_AXIS_SOURCE_FINGER);
    if (dx) {
        zwlr_virtual_pointer_v1_axis_stop(cursor->zwlr_virtual_pointer_v1, time_stop,
                                          WL_POINTER_AXIS_HORIZONTAL_SCROLL);
    }
    if (dy) {
        zwlr_virtual_pointer_v1_axis_stop(cursor->zwlr_virtual_pointer_v1, time_stop,
                                          WL_POINTER_AXIS_VERTICAL_SCROLL);
    }
    zwlr_virtual_pointer_v1_frame(cursor->zwlr_virtual_pointer_v1);
    wl_display_flush(cursor->display);
}

void cursor_button_click(cursor *cursor, uint32_t button)
{
    if (!cursor->zwlr_virtual_pointer_v1) {
        return;
    }

    uint32_t time = current_time_msec();
    zwlr_virtual_pointer_v1_button(cursor->zwlr_virtual_pointer_v1, time, button,
                                   WL_POINTER_BUTTON_STATE_PRESSED);
    zwlr_virtual_pointer_v1_frame(cursor->zwlr_virtual_pointer_v1);
    zwlr_virtual_pointer_v1_button(cursor->zwlr_virtual_pointer_v1, time, button,
                                   WL_POINTER_BUTTON_STATE_RELEASED);
    zwlr_virtual_pointer_v1_frame(cursor->zwlr_virtual_pointer_v1);
    wl_display_flush(cursor->display);
}

void cursor_button_press(cursor *cursor, uint32_t button, bool press)
{
    if (!cursor->zwlr_virtual_pointer_v1) {
        return;
    }

    uint32_t time = current_time_msec();
    zwlr_virtual_pointer_v1_button(cursor->zwlr_virtual_pointer_v1, time, button,
                                   press ? WL_POINTER_BUTTON_STATE_PRESSED
                                         : WL_POINTER_BUTTON_STATE_RELEASED);
    zwlr_virtual_pointer_v1_frame(cursor->zwlr_virtual_pointer_v1);
    wl_display_flush(cursor->display);
}

void cursor_set_motion_absolute(cursor *cursor, uint32_t pos_x, uint32_t pos_y)
{
    if (!cursor->zwlr_virtual_pointer_v1) {
        return;
    }

    uint32_t time = current_time_msec();
    zwlr_virtual_pointer_v1_motion_absolute(cursor->zwlr_virtual_pointer_v1, time, pos_x, pos_y,
                                            cursor->output->width, cursor->output->height);
    zwlr_virtual_pointer_v1_frame(cursor->zwlr_virtual_pointer_v1);
    wl_display_flush(cursor->display);
}

void cursor_destroy(cursor *cursor)
{
    if (cursor->zwlr_virtual_pointer_v1)
        zwlr_virtual_pointer_v1_destroy(cursor->zwlr_virtual_pointer_v1);
    free(cursor);
}
