// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <float.h>
#include <linux/input-event-codes.h>

#include <wlr/types/wlr_compositor.h>

#include "xwayland_p.h"

struct xwayland_grab {
    struct wlr_seat *wlr_seat;
    struct wl_listener seat_destroy;

    struct wlr_surface *wlr_surface;
    struct wl_listener surface_map;
    struct wl_listener surface_unmap;
    struct wl_listener surface_destroy;

    struct wlr_seat_pointer_grab pointer_grab;
    struct wlr_seat_touch_grab touch_grab;
};

static void xwayland_pointer_grab_enter(struct wlr_seat_pointer_grab *grab,
                                        struct wlr_surface *surface, double sx, double sy)
{
    if (xwayland_surface_try_from_wlr_surface(surface)) {
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
    struct wlr_surface *surface = xwayland_grab->wlr_surface;
    wlr_seat_pointer_enter(grab->seat, surface, FLT_MAX, FLT_MAX);
    serial = wlr_seat_pointer_send_button(grab->seat, time, button, state);
    /* clear focus to eat the release button event */
    wlr_seat_pointer_clear_focus(grab->seat);

    return serial;
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
    xwayland_grab->wlr_surface = NULL;
    xwayland_grab->pointer_grab.seat = NULL;
    xwayland_grab->touch_grab.seat = NULL;
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

static uint32_t xwayland_touch_grab_down(struct wlr_seat_touch_grab *grab, uint32_t time,
                                         struct wlr_touch_point *point)
{
    struct xwayland_grab *xwayland_grab = grab->data;
    xwayland_pointer_grab_enter(&xwayland_grab->pointer_grab, point->surface, point->sx, point->sy);
    return xwayland_pointer_grab_button(&xwayland_grab->pointer_grab, time, BTN_LEFT, 1);
}

static void xwayland_touch_grab_up(struct wlr_seat_touch_grab *grab, uint32_t time,
                                   struct wlr_touch_point *point)
{
    struct xwayland_grab *xwayland_grab = grab->data;
    xwayland_pointer_grab_button(&xwayland_grab->pointer_grab, time, BTN_LEFT, 0);
}

static void xwayland_touch_grab_enter(struct wlr_seat_touch_grab *grab, uint32_t time,
                                      struct wlr_touch_point *point)
{
}

static void xwayland_touch_grab_motion(struct wlr_seat_touch_grab *grab, uint32_t time,
                                       struct wlr_touch_point *point)
{
}

static void xwayland_touch_grab_cancel(struct wlr_seat_touch_grab *grab)
{
    xwayland_grab_clear(grab->data);
}

static const struct wlr_touch_grab_interface xwayland_touch_grab_impl = {
    .down = xwayland_touch_grab_down,
    .up = xwayland_touch_grab_up,
    .enter = xwayland_touch_grab_enter,
    .motion = xwayland_touch_grab_motion,
    .cancel = xwayland_touch_grab_cancel,
};

static void handle_surface_map(struct wl_listener *listener, void *data)
{
    struct xwayland_grab *xwayland_grab = wl_container_of(listener, xwayland_grab, surface_map);
    wlr_seat_pointer_start_grab(xwayland_grab->wlr_seat, &xwayland_grab->pointer_grab);
    wlr_seat_touch_start_grab(xwayland_grab->wlr_seat, &xwayland_grab->touch_grab);
}

static void handle_surface_unmap(struct wl_listener *listener, void *data)
{
    struct xwayland_grab *xwayland_grab = wl_container_of(listener, xwayland_grab, surface_unmap);
    struct wlr_seat *wlr_seat = xwayland_grab->wlr_seat;
    wlr_seat_pointer_end_grab(wlr_seat);
    wlr_seat_touch_end_grab(wlr_seat);
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
    .touch_grab = {
        .interface = &xwayland_touch_grab_impl,
        .data = &xwayland_grab,
    },
};

void xwayland_grab_pointer(struct wlr_surface *wlr_surface, struct wlr_seat *wlr_seat, bool grab)
{
    if (!wlr_surface || !wlr_seat) {
        return;
    }

    struct wlr_seat_pointer_grab *pointer_grab = wlr_seat->pointer_state.grab;
    if (grab && pointer_grab != &xwayland_grab.pointer_grab &&
        xwayland_grab.wlr_surface != wlr_surface) {
        xwayland_grab.wlr_seat = wlr_seat;
        wl_signal_add(&wlr_seat->events.destroy, &xwayland_grab.seat_destroy);

        xwayland_grab.wlr_surface = wlr_surface;
        wl_signal_add(&wlr_surface->events.map, &xwayland_grab.surface_map);
        wl_signal_add(&wlr_surface->events.unmap, &xwayland_grab.surface_unmap);
        wl_signal_add(&wlr_surface->events.destroy, &xwayland_grab.surface_destroy);

        if (wlr_surface->mapped) {
            handle_surface_map(&xwayland_grab.surface_map, NULL);
        }
    } else if (!grab && pointer_grab == &xwayland_grab.pointer_grab &&
               xwayland_grab.wlr_surface == wlr_surface && xwayland_grab.pointer_grab.seat) {
        wlr_seat_pointer_end_grab(wlr_seat);
        wlr_seat_touch_end_grab(wlr_seat);
    }
}
