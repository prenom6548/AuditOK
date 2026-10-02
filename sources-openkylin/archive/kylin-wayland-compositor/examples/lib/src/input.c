// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <unistd.h>

#include "wlu_private.h"

static void keyboard_keymap(void *data, struct wl_keyboard *keyboard, uint32_t format, int fd,
                            uint32_t size)
{
    close(fd);
}

static void keyboard_enter(void *data, struct wl_keyboard *keyboard, uint32_t serial,
                           struct wl_surface *surface, struct wl_array *keys)
{
}

static void keyboard_leave(void *data, struct wl_keyboard *keyboard, uint32_t serial,
                           struct wl_surface *surface)
{
}

static void keyboard_key(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time,
                         uint32_t key, uint32_t state)
{
    struct wlu_display *display = data;
    struct wlu_window *window = display->activated_window;
    if (window && window->event_callbacks.keyboard) {
        window->event_callbacks.keyboard(window, key, state);
    }
}

static void keyboard_modifiers(void *data, struct wl_keyboard *keyboard, uint32_t serial,
                               uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked,
                               uint32_t group)
{
}

static const struct wl_keyboard_listener keyboard_listener = {
    .keymap = keyboard_keymap,
    .enter = keyboard_enter,
    .leave = keyboard_leave,
    .key = keyboard_key,
    .modifiers = keyboard_modifiers,
};

const struct wl_keyboard_listener *wlu_input_keyboard_listener(void)
{
    return &keyboard_listener;
}

static void pointer_enter(void *data, struct wl_pointer *pointer, uint32_t serial,
                          struct wl_surface *surface, wl_fixed_t sx, wl_fixed_t sy)
{
}

static void pointer_leave(void *data, struct wl_pointer *pointer, uint32_t serial,
                          struct wl_surface *surface)
{
}

static void pointer_motion(void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t sx,
                           wl_fixed_t sy)
{
    double x = wl_fixed_to_double(sx);
    double y = wl_fixed_to_double(sy);

    struct wlu_display *display = data;
    display->pointer_position.x = x;
    display->pointer_position.y = y;

    struct wlu_window *window = display->activated_window;
    if (window && window->event_callbacks.pointer_motion) {
        window->event_callbacks.pointer_motion(window, x, y);
    }
}

static void pointer_button(void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time,
                           uint32_t button, uint32_t state)
{
    struct wlu_display *display = data;
    struct wlu_window *window = display->activated_window;
    if (window && window->event_callbacks.pointer_button) {
        window->event_callbacks.pointer_button(window, button, state, display->pointer_position.x,
                                               display->pointer_position.y);
    }
}

static void pointer_axis(void *data, struct wl_pointer *pointer, uint32_t time, uint32_t axis,
                         wl_fixed_t value)
{
}

static const struct wl_pointer_listener pointer_listener = {
    .enter = pointer_enter,
    .leave = pointer_leave,
    .motion = pointer_motion,
    .button = pointer_button,
    .axis = pointer_axis,
};

const struct wl_pointer_listener *wlu_input_pointer_listener(void)
{
    return &pointer_listener;
}
