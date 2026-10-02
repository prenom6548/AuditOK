// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _XWAYLAND_P_H_
#define _XWAYLAND_P_H_

#include <kywc/log.h>

#include "kywc_config.h"
#include "xwayland/shell.h"
#include "xwayland/xwayland.h"
#include "xwayland/xwm.h"

/* xwayland server */
struct xwayland_server_options {
    bool lazy;
    bool enable_wm;
    bool no_touch_pointer_emulation;
    bool force_xrandr_emulation;
    int terminate_delay; // in seconds, 0 to terminate immediately
};

struct xwayland_server {
    pid_t pid;
    struct wl_client *client;
    struct wl_event_source *pipe_source;
    int wm_fd[2], wl_fd[2];
    bool ready;

    time_t server_start;

    /* Anything above display is reset on Xwayland restart, rest is conserved */

    int display;
    char display_name[16];
    int x_fd[2];
    struct wl_event_source *x_fd_read_event[2];
    struct xwayland_server_options options;
    char *xauthority_file;

    struct wl_display *wl_display;
    struct wl_event_source *idle_source;

    struct {
        struct wl_signal start;
        struct wl_signal ready;
        struct wl_signal destroy;
    } events;

    struct wl_listener client_destroy;
    struct wl_listener display_destroy;

    void *data;
};

struct xwayland_server_ready_event {
    struct xwayland_server *server;
    int wm_fd;
};

struct xwayland_server *xwayland_server_create(struct server *server,
                                               struct xwayland_server_options *options);
void xwayland_server_destroy(struct xwayland_server *server);

/* sockets */
bool set_cloexec(int fd, bool cloexec);
void unlink_display_sockets(int display);
int open_display_sockets(int socks[2]);

/* view */
void xwayland_view_create(struct xwayland_surface *xsurface);

struct xwayland_surface *xwayland_surface_from_view(struct view *view);

/* unmanaged */
void xwayland_unmanaged_create(struct xwayland_surface *xsurface);

void xwayland_grab_pointer(struct wlr_surface *wlr_surface, struct wlr_seat *wlr_seat, bool grab);

/* xauth */
char *generate_xauthority_file(int display);

struct xwayland_impl {
    void (*send_touch_size)(struct input *input, int touch_id, double touch_major,
                            double touch_minor);
    void (*send_touch_info)(struct input *input, int major_minimum, int major_maximum,
                            int major_resolution, int minor_minimum, int minor_maximum,
                            int minor_resolution);
};

/* xwayland manager */
struct xwayland_manager {
    struct server *server;
    struct xwm *xwm;

    struct xwayland_server *xwayland_server;
    struct wl_listener xwayland_server_start;
    struct wl_listener xwayland_server_ready;
    struct wl_listener xwayland_server_destroy;

    struct xwayland_shell_v1 *shell;
    struct wl_listener shell_destroy;

    struct xwayland_impl impl;
};

void xwayland_update_seat(struct seat *seat);

#endif /* _XWAYLAND_P_H_ */
