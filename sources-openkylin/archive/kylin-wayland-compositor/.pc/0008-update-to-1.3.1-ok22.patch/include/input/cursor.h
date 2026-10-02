// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _CURSOR_H_
#define _CURSOR_H_

#include "gesture.h"
#include "input.h"

/* make same as enum wp_cursor_shape_device_v1_shape */
enum cursor_name {
    CURSOR_NONE = 0,
    CURSOR_DEFAULT = 1,
    CURSOR_CONTEXT_MENU = 2,
    CURSOR_HELP = 3,
    CHRSOR_POINTER = 4,
    CURSOR_PROGRESS = 5,
    CURSOR_WAIT = 6,
    CURSOR_CELL = 7,
    CURSOR_CROSSHAIR = 8,
    CURSOR_TEXT = 9,
    CURSOR_VERTICAL_TEXT = 10,
    CURSOR_ALIAS = 11,
    CURSOR_COPY = 12,
    CURSOR_MOVE = 13,
    CURSOR_NO_DROP = 14,
    CURSOR_NOT_ALLOWED = 15,
    CURSOR_GRAB = 16,
    CURSOR_GRABBING = 17,
    CURSOR_E_RESIZE = 18,
    CURSOR_N_RESIZE = 19,
    CURSOR_NE_RESIZE = 20,
    CURSOR_NW_RESIZE = 21,
    CURSOR_S_RESIZE = 22,
    CURSOR_SE_RESIZE = 23,
    CURSOR_SW_RESIZE = 24,
    CURSOR_W_RESIZE = 25,
    CURSOR_EW_RESIZE = 26,
    CURSOR_NS_RESIZE = 27,
    CURSOR_NESW_RESIZE = 28,
    CURSOR_NWSE_RESIZE = 29,
    CURSOR_COL_RESIZE = 30,
    CURSOR_ROW_RESIZE = 31,
    CURSOR_ALL_SCROLL = 32,
    CURSOR_ZOOM_IN = 33,
    CURSOR_ZOOM_OUT = 34,
    /* rename for old names */
    CURSOR_RESIZE_TOP_LEFT = 21,
    CURSOR_RESIZE_TOP = 19,
    CURSOR_RESIZE_TOP_RIGHT = 20,
    CURSOR_RESIZE_RIGHT = 18,
    CURSOR_RESIZE_BOTTOM_RIGHT = 23,
    CURSOR_RESIZE_BOTTOM = 22,
    CURSOR_RESIZE_BOTTOM_LEFT = 24,
    CURSOR_RESIZE_LEFT = 25,
};

struct cursor_node {
    /* node at the cursor position */
    struct ky_scene_node *node;
    /* do something if node destroy */
    struct wl_listener destroy;
};

struct cursor {
    struct wlr_cursor *wlr_cursor;
    struct wlr_xcursor_manager *xcursor_manager;
    struct seat *seat;

    struct wl_listener motion;
    struct wl_listener motion_absolute;
    struct wl_listener button;
    struct wl_listener axis;
    struct wl_listener frame;

    struct wl_listener swipe_begin;
    struct wl_listener swipe_update;
    struct wl_listener swipe_end;
    struct wl_listener pinch_begin;
    struct wl_listener pinch_update;
    struct wl_listener pinch_end;
    struct wl_listener hold_begin;
    struct wl_listener hold_end;

    struct wl_listener touch_up;
    struct wl_listener touch_down;
    struct wl_listener touch_motion;
    struct wl_listener touch_cancel;
    struct wl_listener touch_frame;

    struct wl_listener tablet_tool_axis;
    struct wl_listener tablet_tool_proximity;
    struct wl_listener tablet_tool_tip;
    struct wl_listener tablet_tool_button;

    struct wl_listener request_set_cursor;
    /* client cursor surface */
    struct wlr_surface *surface;
    struct wl_listener surface_client_commit;
    struct wl_listener surface_destroy;

    struct cursor_constraint *active_constraint;
    struct cursor_constraint *pending_constraint;

    struct gesture_state gestures;

    bool touch_simulation_pointer;
    bool pointer_touch_up;
    int32_t pointer_touch_id;

    bool tablet_tool_tip_simulation_pointer;
    bool tablet_tool_button_simulation_pointer;
    bool tablet_tool_tip_down;
    uint32_t tablet_tool_buttons;

    enum cursor_name name;
    /* current cursor position in layout coord */
    double lx, ly;
    int image_locks;
    bool client_requested, hidden;

    /* cached button clicked info */
    uint32_t last_click_time;
    uint32_t last_click_button;
    bool last_click_pressed;

    /* node below the cursor */
    struct cursor_node hover;
    /* node if button clicked */
    struct cursor_node focus;
    /* current curosr position in node coord */
    double sx, sy;

    /* special: hold a pressed button and leave surface */
    bool hold_mode;
};

struct cursor_constraint {
    struct cursor *cursor;
    struct wlr_pointer_constraint_v1 *constraint;
    struct wl_listener set_region;
    struct wl_listener destroy;
    struct wl_listener surface_unmap;
};

struct cursor *cursor_create(struct seat *seat);

void curosr_add_input(struct seat *seat, struct input *input);

void cursor_remove_input(struct input *input);

void cursor_destroy(struct cursor *cursor);

void cursor_set_image(struct cursor *cursor, enum cursor_name name);

void cursor_set_resize_image(struct cursor *cursor, uint32_t edges);

void cursor_rebase(struct cursor *cursor);

void cursor_rebase_all(bool force);

/* only move the cursor, no event process */
void cursor_move(struct cursor *cursor, struct wlr_input_device *dev, double x, double y,
                 bool delta, bool absolute);

void cursor_set_hidden(struct cursor *cursor, bool hidden);

void cursor_lock_image(struct cursor *cursor, bool lock);

#endif /* _CURSOR_H_ */
