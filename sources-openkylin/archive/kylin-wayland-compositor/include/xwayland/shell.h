// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef XWAYLAND_SHELL_H_
#define XWAYLAND_SHELL_H_

#include <stdbool.h>
#include <wayland-server-core.h>

struct xwayland_shell_v1 {
    struct wl_global *global;

    struct {
        struct wl_signal destroy;
        struct wl_signal new_surface; // struct xwayland_surface_v1
    } events;

    struct wl_client *client;
    struct wl_list surfaces; // xwayland_surface_v1.link

    struct wl_listener display_destroy;
    struct wl_listener client_destroy;
};

struct xwayland_surface_v1 {
    struct wlr_surface *wlr_surface;
    uint64_t serial;

    struct wl_resource *resource;
    struct wl_list link;
    struct xwayland_shell_v1 *shell;
    bool added;
};

struct xwayland_shell_v1 *xwayland_shell_v1_create(struct wl_display *display);

void xwayland_shell_v1_destroy(struct xwayland_shell_v1 *shell);

void xwayland_shell_v1_set_client(struct xwayland_shell_v1 *shell, struct wl_client *client);

struct wlr_surface *xwayland_shell_v1_surface_from_serial(struct xwayland_shell_v1 *shell,
                                                          uint64_t serial);

#endif /* _XWAYLAND_SHELL_H_ */
