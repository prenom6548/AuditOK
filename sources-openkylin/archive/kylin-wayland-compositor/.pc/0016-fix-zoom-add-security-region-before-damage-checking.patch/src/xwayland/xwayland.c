// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>

#include <wlr/types/wlr_compositor.h>

#include "input/seat.h"
#include "output.h"
#include "security/security.h"
#include "server.h"
#include "util/macros.h"
#include "xwayland_p.h"

static struct xwayland_manager *xwayland = NULL;

void xwayland_update_seat(struct seat *seat)
{
    // hover by input_rebase will run here when xwm_destroy
    if (xwayland->server->terminate || !xwayland->xwayland_server || !xwayland->xwm ||
        xwayland->xwm->seat == seat) {
        return;
    }

    xwm_set_seat(xwayland->xwm, seat);
}

static void handle_xwayland_server_destroy(struct wl_listener *listener, void *data)
{
    // Server is being destroyed so avoid destroying it once again.
    xwayland->xwayland_server = NULL;
    xwayland_manager_destroy();
}

static void handle_xwayland_server_start(struct wl_listener *listener, void *data)
{
    if (xwayland->shell != NULL) {
        xwayland_shell_v1_set_client(xwayland->shell, xwayland->xwayland_server->client);
    }
}

static void handle_xwayland_server_ready(struct wl_listener *listener, void *data)
{
    kywc_log(KYWC_INFO, "Xwayland is ready");

    assert(xwayland->xwayland_server->wm_fd[0] >= 0);
    xwayland->xwm = xwm_create(xwayland, xwayland->xwayland_server->wm_fd[0]);
    if (!xwayland->xwm) {
        return;
    }

    /* use the default seat0 */
    xwm_set_seat(xwayland->xwm, input_manager_get_default_seat());
}

static bool xwayland_filter_global(const struct wl_client *client, void *data)
{
    /* only expose this global to Xwayland clients */
    return xwayland_check_client(client);
}

static void handle_shell_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&xwayland->shell_destroy.link);
    security_remove_global_filter(xwayland->shell->global);
    xwayland->shell = NULL;
}

bool xwayland_manager_create(struct server *server)
{
    if (!server->options.enable_xwayland) {
        kywc_log(KYWC_INFO, "Xwayland is disabled by cmdline");
        return false;
    }

    xwayland = calloc(1, sizeof(*xwayland));
    if (!xwayland) {
        kywc_log(KYWC_ERROR, "Cannot create xwayland server");
        return false;
    }

    xwayland->state.eavesdrops_key_mode = ALL_KEYS_WITH_MODIFIER;
    xwayland_config_init(xwayland);
    xwayland_read_config(xwayland);

    xwayland->shell = xwayland_shell_v1_create(server->display);
    if (!xwayland->shell) {
        kywc_log(KYWC_ERROR, "Cannot create xwayland shell");
        goto error;
    }
    xwayland->shell_destroy.notify = handle_shell_destroy;
    wl_signal_add(&xwayland->shell->events.destroy, &xwayland->shell_destroy);
    security_add_global_filter(xwayland->shell->global, xwayland_filter_global, NULL);

    struct xwayland_server_options options = {
        .lazy = server->options.lazy_xwayland,
        .enable_wm = true,
        .terminate_delay = server->options.lazy_xwayland ? 10 : 0,
    };
    xwayland->xwayland_server = xwayland_server_create(server, &options);
    if (!xwayland->xwayland_server) {
        kywc_log(KYWC_ERROR, "Cannot create wlroots xwayland server");
        goto error;
    }

    xwayland->server = server;
    xwayland->xwayland_server_destroy.notify = handle_xwayland_server_destroy;
    wl_signal_add(&xwayland->xwayland_server->events.destroy, &xwayland->xwayland_server_destroy);
    xwayland->xwayland_server_start.notify = handle_xwayland_server_start;
    wl_signal_add(&xwayland->xwayland_server->events.start, &xwayland->xwayland_server_start);
    xwayland->xwayland_server_ready.notify = handle_xwayland_server_ready;
    wl_signal_add(&xwayland->xwayland_server->events.ready, &xwayland->xwayland_server_ready);

    if (xwayland->xwayland_server->ready) {
        handle_xwayland_server_ready(&xwayland->xwayland_server_ready, NULL);
    }

    setenv("DISPLAY", xwayland->xwayland_server->display_name, true);
    kywc_log(KYWC_INFO, "Xwayland is running on display %s",
             xwayland->xwayland_server->display_name);
    return true;

error:
    xwayland_shell_v1_destroy(xwayland->shell);
    free(xwayland);
    xwayland = NULL;
    return false;
}

void xwayland_manager_destroy(void)
{
    if (!xwayland) {
        return;
    }

    xwayland_write_config(xwayland);

    wl_list_remove(&xwayland->xwayland_server_start.link);
    wl_list_remove(&xwayland->xwayland_server_ready.link);
    wl_list_remove(&xwayland->xwayland_server_destroy.link);

    xwayland_server_destroy(xwayland->xwayland_server);
    xwayland->xwayland_server = NULL;
    xwayland_shell_v1_destroy(xwayland->shell);
    xwm_set_seat(xwayland->xwm, NULL);
    xwm_destroy(xwayland->xwm);
    free(xwayland);
    xwayland = NULL;
}

bool xwayland_check_client(const struct wl_client *client)
{
    return xwayland && xwayland->xwayland_server && xwayland->xwayland_server->client == client;
}

int xwayland_unscale(int value)
{
    if (!xwayland || !xwayland->xwm) {
        return value;
    }

    int32_t K = (int32_t)(xwayland->xwm->scale * 256.0f);
    int64_t n = (int64_t)value * 256;
    int64_t b = n >= 0 ? (n + K / 2) / K : (n - K / 2) / K;
    return CLAMP(b, INT32_MIN, INT32_MAX);
}

int xwayland_scale(int value)
{
    if (!xwayland || !xwayland->xwm) {
        return value;
    }

    int32_t K = (int32_t)(xwayland->xwm->scale * 256.0f);
    int64_t p = (int64_t)value * K;
    int64_t a = (p >= 0) ? (p + 128) / 256 : (p - 128) / 256;
    return CLAMP(a, INT32_MIN, INT32_MAX);
}

float xwayland_get_scale(void)
{
    if (!xwayland || !xwayland->xwm) {
        return 1.0;
    }
    return xwayland->xwm->scale;
}

void xwayland_set_cursor(struct seat *seat)
{
    if (!xwayland || !xwayland->xwayland_server || !xwayland->xwm || xwayland->xwm->seat != seat) {
        return;
    }

    xwm_update_cursor(xwayland->xwm);
}

void xwayland_update_workarea(void)
{
    if (!xwayland || !xwayland->xwayland_server || !xwayland->xwm) {
        return;
    }

    struct wlr_box box;
    output_layout_get_workarea(&box);
    struct kywc_box kywc_box;
    kywc_box.x = xwayland_scale(box.x);
    kywc_box.y = xwayland_scale(box.y);
    kywc_box.width = xwayland_scale(box.width);
    kywc_box.height = xwayland_scale(box.height);
    xwm_set_workareas(xwayland->xwm, &kywc_box, 1);
}

bool xwayland_is_dragging_x11(struct seat *seat)
{
    if (!xwayland || !xwayland->xwayland_server || !xwayland->xwm || !xwayland->xwm->seat) {
        return false;
    }

    struct seat *xwayland_seat = xwayland->xwm->seat;
    return (!seat || xwayland_seat == seat) && xwayland->xwm->drag_x11;
}

pid_t xwayland_get_pid_from_wlr_surface(struct wlr_surface *wlr_surface)
{
    if (!wlr_surface) {
        return -1;
    }
    struct xwayland_surface *xsurface = xwayland_surface_try_from_wlr_surface(wlr_surface);
    if (!xsurface) {
        return -1;
    }

    return xsurface->pid;
}

void xwayland_send_touch_size(struct input *input, int touch_id, double touch_major,
                              double touch_minor)
{
    if (xwayland->impl.send_touch_size) {
        xwayland->impl.send_touch_size(input, touch_id, touch_major, touch_minor);
    }
}

void xwayland_send_touch_info(struct input *input, int major_minimum, int major_maximum,
                              int major_resolution, int minor_minimum, int minor_maximum,
                              int minor_resolution)
{
    if (xwayland->impl.send_touch_info) {
        xwayland->impl.send_touch_info(input, major_minimum, major_maximum, major_resolution,
                                       minor_minimum, minor_maximum, minor_resolution);
    }
}
