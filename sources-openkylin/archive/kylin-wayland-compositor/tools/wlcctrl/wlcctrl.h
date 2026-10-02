// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef WLCCTRL_H
#define WLCCTRL_H

#include <stdint.h>

#include <libkywc.h>

#include "seat.h"

typedef struct _keyboard keyboard;
typedef struct _cursor cursor;

/* wayland resources a command may depend on */
#define NEED_SEAT (1 << 0)
#define NEED_KEYBOARD (1 << 1) /* zwp_virtual_keyboard_manager_v1 */
#define NEED_POINTER (1 << 2)  /* zwlr_virtual_pointer_manager_v1 */
#define NEED_OUTPUTS (1 << 3)  /* wl_output tracking */

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
    uint32_t needs;
};

struct wlcom_ctrl_manager *wayland_init(uint32_t needs);

void wayland_deinit(struct wlcom_ctrl_manager *manager);

cursor *wlcctrl_cursor_init(struct wlcom_ctrl_manager *manager);

keyboard *wlcctrl_keyboard_init(struct wlcom_ctrl_manager *manager);

#endif
