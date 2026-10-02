// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _TEXT_INPUT_V3_H_
#define _TEXT_INPUT_V3_H_

#include <wayland-server-core.h>
#include <wlr/util/box.h>

enum text_input_v3_features {
    TEXT_INPUT_V3_FEATURE_SURROUNDING_TEXT = 1 << 0,
    TEXT_INPUT_V3_FEATURE_CONTENT_TYPE = 1 << 1,
    TEXT_INPUT_V3_FEATURE_CURSOR_RECTANGLE = 1 << 2,
};

struct text_input_v3_state {
    struct {
        char *text;
        uint32_t cursor;
        uint32_t anchor;
    } surrounding;

    uint32_t text_change_cause;

    struct {
        uint32_t hint;
        uint32_t purpose;
    } content_type;

    struct wlr_box cursor_rectangle;

    uint32_t features; // bitfield of enum wlr_text_input_v3_features
    bool enabled;
};

struct text_input_v3 {
    struct wl_list link;
    struct wl_resource *resource;

    struct wlr_seat *seat;
    struct wl_listener seat_destroy;

    struct wlr_surface *focused_surface;
    struct wl_listener surface_destroy;

    struct text_input_v3_state pending;
    struct text_input_v3_state current;
    uint32_t active_features; // bitfield of enum wlr_text_input_v3_features

    uint32_t serial;

    struct {
        struct wl_signal enable;
        struct wl_signal disable;
        struct wl_signal commit;
        struct wl_signal destroy;
    } events;
};

struct text_input_manager_v3 {
    struct wl_global *global;
    struct wl_list text_inputs;

    struct wl_listener display_destroy;

    struct {
        struct wl_signal new_text_input;
        struct wl_signal destroy;
    } events;
};

struct text_input_manager_v3 *text_input_manager_v3_create(struct wl_display *display);

void text_input_v3_send_enter(struct text_input_v3 *text_input, struct wlr_surface *surface);

void text_input_v3_send_leave(struct text_input_v3 *text_input);

void text_input_v3_send_preedit_string(struct text_input_v3 *text_input, const char *text,
                                       int32_t cursor_begin, int32_t cursor_end);

void text_input_v3_send_commit_string(struct text_input_v3 *text_input, const char *text);

void text_input_v3_send_delete_surrounding_text(struct text_input_v3 *text_input,
                                                uint32_t before_length, uint32_t after_length);

void text_input_v3_send_done(struct text_input_v3 *text_input);

#endif /* _TEXT_INPUT_V3_H_ */
