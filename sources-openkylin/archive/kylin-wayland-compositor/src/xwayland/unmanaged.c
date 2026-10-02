// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include "scene/surface.h"
#include "view/view.h"
#include "xwayland_p.h"

struct xwayland_unmanaged {
    struct xwayland_surface *xsurface;
    struct wl_listener node_destroy;
};

static bool xwayland_unmanaged_hover(struct seat *seat, struct scene_node *node, double x, double y,
                                     uint32_t time, bool first, bool hold, void *data)
{
    struct wlr_surface *surface = wlr_surface_try_from_node(node);

    xwayland_update_seat(seat);

    if (!hold) {
        seat_notify_motion(seat, surface, time, xwayland_scale(x), xwayland_scale(y), first);
        return false;
    }

    struct xwayland_unmanaged *unmanaged = data;
    int lx, ly;
    scene_node_coords(unmanaged->xsurface->surface_node, &lx, &ly);
    seat_notify_motion(seat, surface, time, xwayland_scale(x - lx), xwayland_scale(y - ly), first);
    return true;
}

static bool xwayland_unmanaged_is_focusable(struct xwayland_unmanaged *unmanaged)
{
    struct xwayland_surface *xsurface = unmanaged->xsurface;

    if (!xwayland_surface_wants_focus(xsurface)) {
        return false;
    }

    /* No Input and Globally Active clients set the input field to False,
     * which requests that the window manager not set the input focus to their top-level window.
     */
    if (xsurface->hints && !xsurface->hints->input) {
        return false;
    }

    return xwayland_surface_has_input(xsurface, INPUT_MASK_KEYBOARD);
}

static void xwayland_unmanaged_focus(struct xwayland_unmanaged *unmanaged)
{
    if (!xwayland_unmanaged_is_focusable(unmanaged)) {
        return;
    }

    struct seat *seat = unmanaged->xsurface->xwm->seat;
    if (!seat) {
        return;
    }

    seat_focus_surface(seat, unmanaged->xsurface->surface);
}

static void xwayland_unmanaged_click(struct seat *seat, struct scene_node *node, uint32_t button,
                                     bool pressed, uint32_t time, enum click_state state,
                                     void *data)
{
    xwayland_update_seat(seat);

    seat_notify_button(seat, time, button, pressed);

    /* only do activated when button pressed */
    if (!pressed) {
        return;
    }

    /* only activate and focus top surface */
    struct xwayland_unmanaged *unmanaged = data;
    xwayland_unmanaged_focus(unmanaged);
}

static void xwayland_unmanaged_leave(struct seat *seat, struct scene_node *node, bool last,
                                     void *data)
{
    /* so surface will call set_cursor when enter again */
    struct wlr_surface *surface = wlr_surface_try_from_node(node);
    seat_notify_leave(seat, surface);
}

static struct scene_node *xwayland_unmanaged_get_root(void *data)
{
    struct xwayland_unmanaged *unmanaged = data;
    return unmanaged->xsurface->surface_node;
}

static struct wlr_surface *xwayland_unmanaged_get_toplevel(void *data)
{
    struct xwayland_unmanaged *unmanaged = data;
    /* only return surface if focusable */
    if (!xwayland_unmanaged_is_focusable(unmanaged)) {
        return NULL;
    }
    return unmanaged->xsurface->surface;
}

static const struct input_event_node_impl xwayland_unmanaged_event_node_impl = {
    .hover = xwayland_unmanaged_hover,
    .click = xwayland_unmanaged_click,
    .leave = xwayland_unmanaged_leave,
};

static void xwayland_unmanaged_try_grab_pointer(struct xwayland_unmanaged *unmanaged)
{
    struct xwayland_surface *xsurface = unmanaged->xsurface;

    if (xsurface->parent &&
        (xwayland_surface_has_type(xsurface, NET_WM_WINDOW_TYPE_POPUP_MENU) ||
         xwayland_surface_has_type(xsurface, NET_WM_WINDOW_TYPE_DROPDOWN_MENU) ||
         xwayland_surface_has_type(xsurface, NET_WM_WINDOW_TYPE_COMBO))) {
        struct wlr_seat *wlr_seat =
            unmanaged->xsurface->xwm->seat ? unmanaged->xsurface->xwm->seat->wlr_seat : NULL;
        if (wlr_seat) {
            xwayland_grab_pointer(xsurface->surface, wlr_seat, true);
            kywc_log(KYWC_DEBUG, "Unmanaged popup menu start grab pointer");
        }
    }
}

static void unmanaged_surface_request_activate(struct xwayland_surface *xsurface)
{
    struct xwayland_unmanaged *unmanaged = xsurface->unmanaged;
    xwayland_unmanaged_focus(unmanaged);
}

static void
unmanaged_surface_request_configure(struct xwayland_surface *xsurface,
                                    struct xwayland_surface_request_configure *configure)
{
    xwayland_surface_configure(xsurface, configure->x, configure->y, configure->width,
                               configure->height);
    if (xsurface->surface_node) {
        scene_node_set_position(xsurface->surface_node, xwayland_unscale(configure->x),
                                xwayland_unscale(configure->y));
    }
}

static void unmanaged_surface_restack(struct xwayland_surface *xsurface, bool above)
{
    uint32_t values[2];
    size_t idx = 0;
    uint32_t flags = XCB_CONFIG_WINDOW_STACK_MODE;
    if (!above) {
        if (!wl_list_empty(&xsurface->xwm->surfaces_in_stack_order)) {
            struct xwayland_surface *xwayland_view_above = wl_container_of(
                xsurface->xwm->surfaces_in_stack_order.prev, xwayland_view_above, stack_link);
            values[idx++] = xwayland_view_above->window_id;
        } else {
            values[idx++] = xsurface->xwm->guard_window;
        }

        flags |= XCB_CONFIG_WINDOW_SIBLING;
    }
    values[idx++] = XCB_STACK_MODE_ABOVE;

    xcb_configure_window(xsurface->xwm->xcb_conn, xsurface->window_id, flags, &values);
    xcb_flush(xsurface->xwm->xcb_conn);
}

static void unmanaged_surface_request_above(struct xwayland_surface *xsurface)
{
    if (xsurface->surface_node) {
        scene_node_raise_to_top(xsurface->surface_node);
    }
    unmanaged_surface_restack(xsurface, true);
}

static void unmanaged_surface_request_below(struct xwayland_surface *xsurface)
{
    if (xsurface->surface_node) {
        scene_node_lower_to_bottom(xsurface->surface_node);
    }
    unmanaged_surface_restack(xsurface, false);
}

static void unmanaged_surface_set_geometry(struct xwayland_surface *xsurface)
{
    if (xsurface->surface_node) {
        scene_node_set_position(xsurface->surface_node, xwayland_unscale(xsurface->x),
                                xwayland_unscale(xsurface->y));
    }
}

static void unmanaged_surface_map(struct xwayland_surface *xsurface)
{
    struct xwayland_unmanaged *unmanaged = xsurface->unmanaged;

    xwayland_unmanaged_focus(unmanaged);
    xwayland_unmanaged_try_grab_pointer(unmanaged);

    scene_node_set_enabled(xsurface->surface_node, true);
    scene_node_set_position(xsurface->surface_node, xwayland_unscale(xsurface->x),
                            xwayland_unscale(xsurface->y));

    /* workaround: wechat drag icon has no NET_WM_WINDOW_TYPE_DND */
    if ((seat_is_dragging(xsurface->xwm->seat) &&
         !xwayland_surface_has_input(xsurface, INPUT_MASK_POINTER) &&
         xwayland_surface_has_type(xsurface, NET_WM_WINDOW_TYPE_TOOLTIP)) ||
        xwayland_surface_has_type(xsurface, NET_WM_WINDOW_TYPE_DND)) {
        scene_node_set_input_bypassed(xsurface->surface_node, true);
    }
}

static void unmanaged_surface_unmap(struct xwayland_surface *xsurface)
{
    if (xsurface->surface_node) {
        scene_node_set_enabled(xsurface->surface_node, false);
    }

    struct wlr_surface *wlr_surface = xsurface->surface;
    struct wlr_seat *wlr_seat = xsurface->xwm->seat ? xsurface->xwm->seat->wlr_seat : NULL;
    if (wlr_seat && wlr_seat->keyboard_state.focused_surface == wlr_surface) {
        struct wlr_surface *parent_surface = xsurface->parent ? xsurface->parent->surface : NULL;
        struct view *parent = parent_surface ? view_try_from_wlr_surface(parent_surface) : NULL;
        struct view *activated_view = view_manager_get_activated();
        struct wlr_surface *focused_surface = wlr_seat->keyboard_state.focused_surface;

        if (activated_view && activated_view != parent) {
            if (activated_view->surface != focused_surface) {
                struct xwayland_surface *xwayland_surface =
                    xwayland_surface_try_from_wlr_surface(activated_view->surface);
                if (xwayland_surface) {
                    xwayland_surface_activate(xwayland_surface, true);
                }
                view_set_focus(activated_view, seat_from_wlr_seat(wlr_seat));
            }
            return;
        }

        if (parent && KYWC_VIEW_IS_ACTIVATABLE(&parent->base) &&
            KYWC_VIEW_IS_FOCUSABLE(&parent->base)) {
            view_do_activate(parent);
            view_set_focus(parent, seat_from_wlr_seat(wlr_seat));
        } else {
            view_activate_topmost(false);
        }
    }
}

static void unmanaged_handle_node_destroy(struct wl_listener *listener, void *data)
{
    struct xwayland_unmanaged *unmanaged = wl_container_of(listener, unmanaged, node_destroy);
    struct xwayland_surface *xsurface = unmanaged->xsurface;
    wl_list_remove(&unmanaged->node_destroy.link);
    xsurface->surface_node = NULL;
}

static const char *unmanaged_handle_desc(const struct scene_node *node, void *data)
{
    struct xwayland_surface *xsurface = data;
    return xsurface->class;
}

static const struct scene_node_role_interface role_impl = {
    .desc = unmanaged_handle_desc,
};

static struct scene_node *unmanaged_surface_associate(struct xwayland_surface *xsurface)
{
    struct xwayland_unmanaged *unmanaged = xsurface->unmanaged;

    struct view_layer *layer = view_manager_get_layer(LAYER_UNMANAGED, false);
    struct scene_surface *scene_surface = scene_surface_create(layer->tree, xsurface->surface);
    struct scene_node *surface_node = &scene_surface->buffer->node;
    scene_node_set_role(surface_node, SCENE_ROLE_XWAYLAND_UNMANAGED, &role_impl, xsurface);
    scene_node_set_enabled(surface_node, false);
    input_event_node_create(surface_node, &xwayland_unmanaged_event_node_impl,
                            xwayland_unmanaged_get_root, xwayland_unmanaged_get_toplevel,
                            unmanaged);

    unmanaged->node_destroy.notify = unmanaged_handle_node_destroy;
    wl_signal_add(&surface_node->events.destroy, &unmanaged->node_destroy);

    xsurface->surface_node = surface_node;
    return surface_node;
}

static void unmanaged_surface_dissociate(struct xwayland_surface *xsurface)
{
    scene_node_destroy(xsurface->surface_node);
}

static void unmanaged_surface_destroy(struct xwayland_surface *xsurface)
{
    struct xwayland_unmanaged *unmanaged = xsurface->unmanaged;
    xsurface->unmanaged = NULL;
    free(unmanaged);
}

static const struct xwayland_surface_impl unmanaged_surface_impl = {
    .associate = unmanaged_surface_associate,
    .dissociate = unmanaged_surface_dissociate,
    .map = unmanaged_surface_map,
    .unmap = unmanaged_surface_unmap,
    .destroy = unmanaged_surface_destroy,

    .request_configure = unmanaged_surface_request_configure,
    .request_activate = unmanaged_surface_request_activate,

    .request_above = unmanaged_surface_request_above,
    .request_below = unmanaged_surface_request_below,

    .set_geometry = unmanaged_surface_set_geometry,
};

void xwayland_unmanaged_create(struct xwayland_surface *xsurface)
{
    struct xwayland_unmanaged *unmanaged = calloc(1, sizeof(*unmanaged));
    if (!unmanaged) {
        return;
    }

    xsurface->impl = &unmanaged_surface_impl;
    xsurface->unmanaged = unmanaged;
    unmanaged->xsurface = xsurface;

    if (xsurface->surface) {
        unmanaged_surface_associate(xsurface);
        if (xsurface->surface->mapped) {
            unmanaged_surface_map(xsurface);
        }
    }
}
