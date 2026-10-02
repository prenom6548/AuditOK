// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <libkywc.h>

#include "seat.h"

typedef struct _keyboard keyboard;
typedef struct _cursor cursor;

struct wlcom_ctrl_manager {
    struct wl_display *wl_display;
    kywc_context *kywc_ctx;
    struct wl_registry *registry;
    struct zwp_virtual_keyboard_manager_v1 *k_manager;
    struct zwlr_virtual_pointer_manager_v1 *p_manager;

    keyboard *keyboard;
    cursor *cursor;
    seat *seat;
    char *primary_output;
    struct wl_list outputs;
};

cursor *wlcctrl_cursor_init(void);

keyboard *wlcctrl_keyboard_init(void);

struct wlcom_ctrl_manager *wayland_init(void);

void wayland_deinit(struct wlcom_ctrl_manager *manager);
