// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later
#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700

#include <cairo/cairo.h>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

#include "alpha-modifier-v1-client-protocol.h"
#include "xdg-decoration-unstable-v1-client-protocol.h"
#include "xdg-shell-client-protocol.h"

struct state {
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_surface *surface;
    struct wl_shm *shm;
    struct wl_seat *seat;
    struct wl_pointer *pointer;

    struct xdg_wm_base *xdg_wm_base;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;

    struct zxdg_decoration_manager_v1 *decoration_manager;
    struct zxdg_toplevel_decoration_v1 *toplevel_decoration;
    struct wp_alpha_modifier_v1 *alpha_modifier;
    struct wp_alpha_modifier_surface_v1 *alpha_modifier_surface;

    struct wl_buffer *buffer;
    void *shm_data;

    int width, height;
    bool running;
};

static void draw_frame(struct state *state)
{
    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, state->width);
    int size = stride * state->height;

    memset(state->shm_data, 0, size);

    cairo_surface_t *cairo_surface = cairo_image_surface_create_for_data(
        state->shm_data, CAIRO_FORMAT_ARGB32, state->width, state->height, stride);
    cairo_t *cr = cairo_create(cairo_surface);

    cairo_set_source_rgba(cr, 0.0, 0.5, 1.0, 0.5);
    cairo_paint(cr);

    cairo_set_source_rgba(cr, 0.0, 0.5, 1.0, 1.0);
    cairo_rectangle(cr, 50, 50, 100, 100);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
    cairo_select_font_face(cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 40);

    cairo_text_extents_t extents;
    cairo_text_extents(cr, "Hello Alpha!", &extents);
    cairo_move_to(cr, state->width / 2.0 - extents.width / 2.0, state->height / 2.0);
    cairo_show_text(cr, "Hello Alpha!");

    cairo_destroy(cr);
    cairo_surface_destroy(cairo_surface);

    wl_surface_attach(state->surface, state->buffer, 0, 0);
    wl_surface_damage_buffer(state->surface, 0, 0, state->width, state->height);
    wl_surface_commit(state->surface);
}

static void xdg_toplevel_handle_configure(void *data, struct xdg_toplevel *xdg_toplevel,
                                          int32_t width, int32_t height, struct wl_array *states)
{
    struct state *state = data;
    if (width > 0 && height > 0) {
        state->width = width;
        state->height = height;
    }
}

static void xdg_toplevel_handle_close(void *data, struct xdg_toplevel *xdg_toplevel)
{
    struct state *state = data;
    state->running = false;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_handle_configure,
    .close = xdg_toplevel_handle_close,
};

static void xdg_surface_handle_configure(void *data, struct xdg_surface *xdg_surface,
                                         uint32_t serial)
{
    struct state *state = data;
    xdg_surface_ack_configure(xdg_surface, serial);
    // commit once after ack_configure
    static bool flag = true;
    if (flag) {
        draw_frame(state);
        flag = false;
    }
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_handle_configure,
};

static void xdg_wm_base_handle_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_handle_ping,
};

static void pointer_handle_enter(void *data, struct wl_pointer *pointer, uint32_t serial,
                                 struct wl_surface *surface, wl_fixed_t sx, wl_fixed_t sy)
{
}

static void pointer_handle_leave(void *data, struct wl_pointer *pointer, uint32_t serial,
                                 struct wl_surface *surface)
{
}

static void pointer_handle_motion(void *data, struct wl_pointer *pointer, uint32_t time,
                                  wl_fixed_t sx, wl_fixed_t sy)
{
}

static void pointer_handle_button(void *data, struct wl_pointer *pointer, uint32_t serial,
                                  uint32_t time, uint32_t button, uint32_t states)
{
    struct state *state = data;
    if ((button != BTN_LEFT && button != BTN_RIGHT) || states != WL_POINTER_BUTTON_STATE_PRESSED) {
        return;
    }
    if (state->alpha_modifier_surface) {
        static int32_t factor = 5;
        if (button == BTN_LEFT) {
            factor = factor + 1;
            if (factor > 10) {
                factor = 0;
            }
        } else {
            factor = factor - 1;
            if (factor < 0) {
                factor = 10;
            }
        }
        wp_alpha_modifier_surface_v1_set_multiplier(state->alpha_modifier_surface,
                                                    factor / 10.0 * UINT32_MAX);
        printf("✅ Alpha modifier enabled, factor %f\n", factor / 10.0);
        draw_frame(state);
    }
}

static void pointer_handle_axis(void *data, struct wl_pointer *pointer, uint32_t time,
                                uint32_t axis, wl_fixed_t value)
{
}

static void pointer_handle_axis_discrete(void *data, struct wl_pointer *wl_pointer, uint32_t axis,
                                         int32_t discrete)
{
}

static void pointer_handle_axis_relative_direction(void *data, struct wl_pointer *wl_pointer,
                                                   uint32_t axis, uint32_t direction)
{
}

static void pointer_handle_axis_source(void *data, struct wl_pointer *wl_pointer,
                                       uint32_t axis_source)
{
}

static void pointer_handle_axis_stop(void *data, struct wl_pointer *wl_pointer, uint32_t time,
                                     uint32_t axis)
{
}

static void pointer_handle_axis_value120(void *data, struct wl_pointer *wl_pointer, uint32_t axis,
                                         int32_t value120)
{
}

static void pointer_handle_frame(void *data, struct wl_pointer *pointer) {}

static const struct wl_pointer_listener wl_pointer_listener = {
    .enter = pointer_handle_enter,
    .leave = pointer_handle_leave,
    .motion = pointer_handle_motion,
    .button = pointer_handle_button,
    .axis = pointer_handle_axis,
    .frame = pointer_handle_frame,
    .axis_discrete = pointer_handle_axis_discrete,
    .axis_relative_direction = pointer_handle_axis_relative_direction,
    .axis_source = pointer_handle_axis_source,
    .axis_stop = pointer_handle_axis_stop,
    .axis_value120 = pointer_handle_axis_value120,
};

static void decoration_handle_configure(void *data, struct zxdg_toplevel_decoration_v1 *decoration,
                                        uint32_t mode)
{
    printf("==> Decoration mode: ");
    switch (mode) {
    case ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE:
        printf("Client-Side\n");
        break;
    case ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE:
        printf("Server-Side\n");
        break;
    default:
        printf("Unknown\n");
        break;
    }
}

static const struct zxdg_toplevel_decoration_v1_listener decoration_listener = {
    .configure = decoration_handle_configure,
};

static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name,
                                   const char *interface, uint32_t version)
{
    struct state *state = data;
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        state->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
        state->shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        state->seat = wl_registry_bind(registry, name, &wl_seat_interface, 8);
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        state->xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(state->xdg_wm_base, &xdg_wm_base_listener, state);
    } else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
        state->decoration_manager =
            wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1);
    } else if (strcmp(interface, wp_alpha_modifier_v1_interface.name) == 0) {
        state->alpha_modifier =
            wl_registry_bind(registry, name, &wp_alpha_modifier_v1_interface, 1);
        printf("✅ Bound to wp_alpha_modifier_v1\n");
    }
}

static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
    // This space is for rent
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

static int create_shm_buffer(struct state *state)
{
    char tmp_name[] = "/tmp/wayland-shm-XXXXXX";
    int fd = mkstemp(tmp_name);
    if (fd < 0) {
        fprintf(stderr, "mkstemp failed\n");
        return -1;
    }
    unlink(tmp_name);

    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, state->width);
    int size = stride * state->height;

    if (ftruncate(fd, size) < 0) {
        close(fd);
        fprintf(stderr, "ftruncate failed\n");
        return -1;
    }

    state->shm_data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (state->shm_data == MAP_FAILED) {
        close(fd);
        fprintf(stderr, "mmap failed\n");
        return -1;
    }

    struct wl_shm_pool *pool = wl_shm_create_pool(state->shm, fd, size);
    state->buffer = wl_shm_pool_create_buffer(pool, 0, state->width, state->height, stride,
                                              WL_SHM_FORMAT_ARGB8888);

    wl_shm_pool_destroy(pool);
    close(fd);
    return 0;
}

int main(int argc, char **argv)
{
    struct state state = { 0 };
    state.width = 640;
    state.height = 480;
    state.running = true;

    state.display = wl_display_connect(NULL);
    if (state.display == NULL) {
        fprintf(stderr, "Can't connect to a Wayland display\n");
        return 1;
    }

    state.registry = wl_display_get_registry(state.display);
    wl_registry_add_listener(state.registry, &registry_listener, &state);

    wl_display_dispatch(state.display);
    wl_display_roundtrip(state.display);

    if (state.compositor == NULL || state.shm == NULL || state.xdg_wm_base == NULL ||
        state.alpha_modifier == NULL) {
        fprintf(stderr, "Can't find compositor, shm , xdg_wm_base or alpha_modifier\n");
        return 1;
    }

    state.surface = wl_compositor_create_surface(state.compositor);

    state.pointer = wl_seat_get_pointer(state.seat);
    wl_pointer_add_listener(state.pointer, &wl_pointer_listener, &state);

    state.xdg_surface = xdg_wm_base_get_xdg_surface(state.xdg_wm_base, state.surface);
    xdg_surface_add_listener(state.xdg_surface, &xdg_surface_listener, &state);
    state.xdg_toplevel = xdg_surface_get_toplevel(state.xdg_surface);
    xdg_toplevel_add_listener(state.xdg_toplevel, &xdg_toplevel_listener, &state);

    state.alpha_modifier_surface =
        wp_alpha_modifier_v1_get_surface(state.alpha_modifier, state.surface);

    if (state.decoration_manager) {
        state.toplevel_decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(
            state.decoration_manager, state.xdg_toplevel);
        zxdg_toplevel_decoration_v1_add_listener(state.toplevel_decoration, &decoration_listener,
                                                 &state);
        zxdg_toplevel_decoration_v1_set_mode(state.toplevel_decoration,
                                             ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }

    xdg_toplevel_set_title(state.xdg_toplevel, "Wayland Hello World");

    wl_surface_commit(state.surface);

    if (create_shm_buffer(&state) < 0) {
        fprintf(stderr, "Failed to create shm buffer\n");
        return 1;
    }

    while (1) {
        if (!state.running || wl_display_dispatch(state.display) == -1) {
            break;
        }
    }

    if (state.toplevel_decoration) {
        zxdg_toplevel_decoration_v1_destroy(state.toplevel_decoration);
    }
    if (state.decoration_manager) {
        zxdg_decoration_manager_v1_destroy(state.decoration_manager);
    }
    wp_alpha_modifier_surface_v1_destroy(state.alpha_modifier_surface);
    wp_alpha_modifier_v1_destroy(state.alpha_modifier);
    wl_buffer_destroy(state.buffer);
    xdg_toplevel_destroy(state.xdg_toplevel);
    xdg_surface_destroy(state.xdg_surface);
    wl_surface_destroy(state.surface);
    xdg_wm_base_destroy(state.xdg_wm_base);
    wl_shm_destroy(state.shm);
    wl_compositor_destroy(state.compositor);
    wl_registry_destroy(state.registry);
    wl_display_disconnect(state.display);

    return 0;
}
