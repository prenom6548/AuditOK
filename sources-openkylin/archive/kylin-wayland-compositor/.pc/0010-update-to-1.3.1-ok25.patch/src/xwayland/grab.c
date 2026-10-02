// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <float.h>

#include <wlr/types/wlr_compositor.h>

#include "xwayland_p.h"

struct xwayland_grab {
    struct wlr_seat *wlr_seat;
    struct wl_listener seat_destroy;

    struct wlr_xwayland_surface *wlr_xwayland_surface;
    struct wl_listener surface_map;
    struct wl_listener surface_unmap;
    struct wl_listener surface_destroy;

    struct wlr_seat_pointer_grab pointer_grab;
};

static void xwayland_pointer_grab_enter(struct wlr_seat_pointer_grab *grab,
                                        struct wlr_surface *surface, double sx, double sy)
{
    if (wlr_xwayland_surface_try_from_wlr_surface(surface)) {
        wlr_seat_pointer_enter(grab->seat, surface, sx, sy);
    } else {
        wlr_seat_pointer_clear_focus(grab->seat);
    }
}

static void xwayland_pointer_grab_clear_focus(struct wlr_seat_pointer_grab *grab)
{
    wlr_seat_pointer_clear_focus(grab->seat);
}

static void xwayland_pointer_grab_motion(struct wlr_seat_pointer_grab *grab, uint32_t time,
                                         double sx, double sy)
{
    wlr_seat_pointer_send_motion(grab->seat, time, sx, sy);
}

static uint32_t xwayland_pointer_grab_button(struct wlr_seat_pointer_grab *grab, uint32_t time,
                                             uint32_t button, uint32_t state)
{
    uint32_t serial = wlr_seat_pointer_send_button(grab->seat, time, button, state);
    if (serial) {
        return serial;
    }

    struct xwayland_grab *xwayland_grab = grab->data;
    struct wlr_surface *surface = xwayland_grab->wlr_xwayland_surface->surface;
    wlr_seat_pointer_enter(grab->seat, surface, FLT_MAX, FLT_MAX);
    wlr_seat_pointer_send_button(grab->seat, time, button, state);
    /* clear focus to eat the release button event */
    wlr_seat_pointer_clear_focus(grab->seat);

    return 0;
}

static void xwayland_pointer_grab_axis(struct wlr_seat_pointer_grab *grab, uint32_t time,
                                       enum wlr_axis_orientation orientation, double value,
                                       int32_t value_discrete, enum wlr_axis_source source)
{
    wlr_seat_pointer_send_axis(grab->seat, time, orientation, value, value_discrete, source);
}

static void xwayland_pointer_grab_frame(struct wlr_seat_pointer_grab *grab)
{
    wlr_seat_pointer_send_frame(grab->seat);
}

static void xwayland_grab_clear(struct xwayland_grab *xwayland_grab)
{
    wl_list_remove(&xwayland_grab->seat_destroy.link);
    wl_list_init(&xwayland_grab->seat_destroy.link);
    wl_list_remove(&xwayland_grab->surface_map.link);
    wl_list_init(&xwayland_grab->surface_map.link);
    wl_list_remove(&xwayland_grab->surface_unmap.link);
    wl_list_init(&xwayland_grab->surface_unmap.link);
    wl_list_remove(&xwayland_grab->surface_destroy.link);
    wl_list_init(&xwayland_grab->surface_destroy.link);

    xwayland_grab->wlr_seat = NULL;
    xwayland_grab->wlr_xwayland_surface = NULL;
    xwayland_grab->pointer_grab.seat = NULL;
}

static void xwayland_pointer_grab_cancel(struct wlr_seat_pointer_grab *grab)
{
    kywc_log(KYWC_DEBUG, "xwayland grab cancel");
    xwayland_grab_clear(grab->data);
}

static const struct wlr_pointer_grab_interface xwayland_pointer_grab_impl = {
    .enter = xwayland_pointer_grab_enter,
    .clear_focus = xwayland_pointer_grab_clear_focus,
    .motion = xwayland_pointer_grab_motion,
    .button = xwayland_pointer_grab_button,
    .cancel = xwayland_pointer_grab_cancel,
    .axis = xwayland_pointer_grab_axis,
    .frame = xwayland_pointer_grab_frame,
};

static void handle_surface_map(struct wl_listener *listener, void *data)
{
    struct xwayland_grab *xwayland_grab = wl_container_of(listener, xwayland_grab, surface_map);
    wlr_seat_pointer_start_grab(xwayland_grab->wlr_seat, &xwayland_grab->pointer_grab);
}

static void handle_surface_unmap(struct wl_listener *listener, void *data)
{
    struct xwayland_grab *xwayland_grab = wl_container_of(listener, xwayland_grab, surface_unmap);
    wlr_seat_pointer_end_grab(xwayland_grab->wlr_seat);
}

static void handle_surface_destroy(struct wl_listener *listener, void *data)
{
    struct xwayland_grab *xwayland_grab = wl_container_of(listener, xwayland_grab, surface_destroy);
    xwayland_grab_clear(xwayland_grab);
}

static void handle_seat_destroy(struct wl_listener *listener, void *data)
{
    struct xwayland_grab *xwayland_grab = wl_container_of(listener, xwayland_grab, seat_destroy);
    xwayland_grab_clear(xwayland_grab);
}

static struct xwayland_grab xwayland_grab = {
    .seat_destroy.notify = handle_seat_destroy,
    .surface_map.notify = handle_surface_map,
    .surface_unmap.notify = handle_surface_unmap,
    .surface_destroy.notify = handle_surface_destroy,
    .pointer_grab = {
        .interface = &xwayland_pointer_grab_impl,
        .data = &xwayland_grab,
    },
};

static void xwayland_grab_pointer(struct wlr_xwayland_surface *wlr_xwayland_surface,
                                  struct wlr_seat *wlr_seat, bool grab)
{
    struct wlr_seat_pointer_grab *pointer_grab = wlr_seat->pointer_state.grab;

    if (grab && pointer_grab != &xwayland_grab.pointer_grab &&
        xwayland_grab.wlr_xwayland_surface != wlr_xwayland_surface) {
        xwayland_grab.wlr_seat = wlr_seat;
        wl_signal_add(&wlr_seat->events.destroy, &xwayland_grab.seat_destroy);

        xwayland_grab.wlr_xwayland_surface = wlr_xwayland_surface;
        wl_signal_add(&wlr_xwayland_surface->surface->events.map, &xwayland_grab.surface_map);
        wl_signal_add(&wlr_xwayland_surface->surface->events.unmap, &xwayland_grab.surface_unmap);
        wl_signal_add(&wlr_xwayland_surface->surface->events.destroy,
                      &xwayland_grab.surface_destroy);

        if (wlr_xwayland_surface->surface->mapped) {
            handle_surface_map(&xwayland_grab.surface_map, NULL);
        }
    } else if (!grab && pointer_grab == &xwayland_grab.pointer_grab &&
               xwayland_grab.wlr_xwayland_surface == wlr_xwayland_surface &&
               xwayland_grab.pointer_grab.seat) {
        wlr_seat_pointer_end_grab(wlr_seat);
    }
}

void xwayland_surface_set_grab_pointer(struct wlr_xwayland_surface *wlr_xwayland_surface,
                                       struct wlr_seat *wlr_seat, bool grab)
{
    if (!wlr_xwayland_surface || !wlr_xwayland_surface->surface || !wlr_seat) {
        return;
    }

    xwayland_grab_pointer(wlr_xwayland_surface, wlr_seat, grab);
}