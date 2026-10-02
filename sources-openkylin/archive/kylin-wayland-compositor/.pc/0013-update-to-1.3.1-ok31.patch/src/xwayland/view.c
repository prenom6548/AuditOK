// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <float.h>
#include <stdlib.h>

#include <wlr/types/wlr_compositor.h>
#include <wlr/util/region.h>

#include "effect/scale.h"
#include "input/event.h"
#include "input/seat.h"
#include "output.h"
#include "painter.h"
#include "scene/surface.h"
#include "theme.h"
#include "util/client.h"
#include "util/macros.h"
#include "view/action.h"
#include "view/workspace.h"
#include "xwayland_p.h"

struct xwayland_view {
    struct view view;
    struct xwayland_surface *xsurface;
    struct wl_listener surface_tree_destroy;

    bool minimizable, maximizable;
    bool focusable, activatable, movable, resizable;
    struct wl_listener view_update_capabilities;

    /* view state need sync with xwayland */
    struct wl_listener view_stack;
    struct wl_listener view_above;
    struct wl_listener view_below;
    struct wl_listener view_sticky;
    struct wl_listener view_skip_taskbar;
    struct wl_listener view_skip_switcher;
    struct wl_listener view_demands_attention;
};

static bool xwayland_view_hover(struct seat *seat, struct scene_node *node, double x, double y,
                                uint32_t time, bool first, bool hold, void *data)
{
    struct wlr_surface *surface = wlr_surface_try_from_node(node);
    struct xwayland_view *xwayland_view = data;

    if (first) {
        view_hover(seat, &xwayland_view->view);
    }

    xwayland_update_seat(seat);

    if (!hold) {
        seat_notify_motion(seat, surface, time, xwayland_scale(x), xwayland_scale(y), first);
        return false;
    }

    double sx = x - xwayland_view->view.base.geometry.x;
    double sy = y - xwayland_view->view.base.geometry.y;
    seat_notify_motion(seat, surface, time, xwayland_scale(sx), xwayland_scale(sy), first);
    return true;
}

static void xwayland_view_click(struct seat *seat, struct scene_node *node, uint32_t button,
                                bool pressed, uint32_t time, enum click_state state, void *data)
{
    xwayland_update_seat(seat);

    seat_notify_button(seat, time, button, pressed);

    /* only do activated when button pressed */
    if (!pressed) {
        return;
    }

    struct xwayland_view *xwayland_view = data;
    view_click(seat, &xwayland_view->view, button, pressed, state);
}

static void xwayland_view_leave(struct seat *seat, struct scene_node *node, bool last, void *data)
{
    /* so surface will call set_cursor when enter again */
    struct wlr_surface *surface = wlr_surface_try_from_node(node);
    seat_notify_leave(seat, surface);
}

static struct scene_node *xwayland_view_get_root(void *data)
{
    struct xwayland_view *xwayland_view = data;
    return &xwayland_view->view.tree->node;
}

static struct wlr_surface *xwayland_view_get_toplevel(void *data)
{
    struct xwayland_view *xwayland_view = data;
    return xwayland_view->view.surface;
}

static const struct input_event_node_impl xwayland_view_event_node_impl = {
    .hover = xwayland_view_hover,
    .click = xwayland_view_click,
    .leave = xwayland_view_leave,
};

static struct xwayland_view *xwayland_view_from_view(struct view *view)
{
    struct xwayland_view *xwayland_view = wl_container_of(view, xwayland_view, view);
    return xwayland_view;
}

struct xwayland_surface *xwayland_surface_from_view(struct view *view)
{
    if (!view || !xwayland_check_view(view)) {
        return NULL;
    }
    return xwayland_view_from_view(view)->xsurface;
}

static void xwayland_view_close(struct view *view)
{
    struct xwayland_view *xwayland_view = xwayland_view_from_view(view);
    xwayland_surface_close(xwayland_view->xsurface);
}

static void xwayland_view_destroy(struct view *view)
{
    struct xwayland_view *xwayland_view = xwayland_view_from_view(view);
    free(xwayland_view);
}

static void xwayland_view_move(struct xwayland_view *xwayland_view, int x, int y)
{
    struct xwayland_surface *xsurface = xwayland_view->xsurface;
    struct view *view = &xwayland_view->view;

    /* xwayland views need sync position except in moving */
    if (!view->interactive_moving) {
        xwayland_surface_configure(xsurface, xwayland_scale(x), xwayland_scale(y), xsurface->width,
                                   xsurface->height);
    }
    view_helper_move(view, x, y);
}

static void xwayland_view_ping(struct view *view)
{
    struct xwayland_view *xwayland_view = xwayland_view_from_view(view);
    struct xwayland_surface *xsurface = xwayland_view->xsurface;
    xwayland_surface_ping(xsurface);
}

static bool xwayland_view_find_sibling_iterator(struct kywc_view *kywc_view, void *data)
{
    struct view **found = (struct view **)data;
    struct view *view = view_from_kywc_view(kywc_view);
    if (xwayland_check_view(view) && !view->base.minimized) {
        *found = view;
        return true;
    }
    return false;
}

static void xwayland_view_handle_view_stack(struct wl_listener *listener, void *data)
{
    struct xwayland_view *xwayland_view = wl_container_of(listener, xwayland_view, view_stack);
    struct view *view = &xwayland_view->view;
    struct xwayland_surface *xsurface = xwayland_view->xsurface;
    /* move to invisible workspace */
    if (view->current_proxy && view->current_proxy->workspace != workspace_manager_get_current()) {
        xwayland_surface_restack(xsurface, NULL, XCB_STACK_MODE_BELOW);
        return;
    }

    struct view *sibling_view = NULL;
    view_manager_for_each_stack_view(xwayland_view_find_sibling_iterator, &xwayland_view->view,
                                     true, &sibling_view);
    enum xcb_stack_mode_t mode = sibling_view ? XCB_STACK_MODE_BELOW : XCB_STACK_MODE_ABOVE;
    xwayland_surface_restack(xsurface, xwayland_surface_from_view(sibling_view), mode);
}

static void xwayland_view_configure(struct view *view)
{
    struct xwayland_view *xwayland_view = xwayland_view_from_view(view);
    struct xwayland_surface *xsurface = xwayland_view->xsurface;
    struct kywc_view *kywc_view = &xwayland_view->view.base;

    if (view->pending.action & VIEW_ACTION_MINIMIZE) {
        view->pending.action &= ~VIEW_ACTION_MINIMIZE;
        xwayland_surface_set_minimized(xsurface, kywc_view->minimized);
        if (kywc_view->minimized) {
            xwayland_surface_restack(xsurface, NULL, XCB_STACK_MODE_BELOW);
        } else {
            xwayland_view_handle_view_stack(&xwayland_view->view_stack, NULL);
        }
    }

    if (view->pending.action & VIEW_ACTION_ACTIVATE) {
        view->pending.action &= ~VIEW_ACTION_ACTIVATE;
        if (kywc_view->activated && xsurface->minimized) {
            xwayland_surface_set_minimized(xsurface, false);
            xwayland_view_handle_view_stack(&xwayland_view->view_stack, NULL);
        }
        xwayland_surface_activate(xsurface, kywc_view->activated);
    }

    /* direct move when not changed size */
    if (view->pending.action & VIEW_ACTION_MOVE) {
        view->pending.action &= ~VIEW_ACTION_MOVE;
        if (!view_action_change_size(view->pending.configure_action)) {
            xwayland_view_move(xwayland_view, view->pending.geometry.x, view->pending.geometry.y);
            /* Set pending.configure_geometry when maximizing restore or restore tiling */
        } else if (view->pending.configure_action & (VIEW_ACTION_MAXIMIZE | VIEW_ACTION_TILE) &&
                   !view->base.maximized && !view->base.tiled) {
            view->pending.configure_geometry.x = view->pending.geometry.x;
            view->pending.configure_geometry.y = view->pending.geometry.y;
        } else {

            kywc_log(KYWC_DEBUG, "Skip move when pending configure action 0x%x",
                     view->pending.configure_action);
        }
    }

    if (view->pending.action == VIEW_ACTION_NOP) {
        return;
    }

    /* now, only changed size action left */
    assert(view_action_change_size(view->pending.action));

    if (view->pending.action & VIEW_ACTION_FULLSCREEN) {
        view_add_scale_effect_pre(view);
        xwayland_surface_set_fullscreen(xsurface, kywc_view->fullscreen);
    }

    if (view->pending.action & VIEW_ACTION_MAXIMIZE) {
        view_add_scale_effect_pre(view);
        xwayland_surface_set_maximized(xsurface, kywc_view->maximized);
    }

    struct kywc_box *pending = &view->pending.geometry;

    /* If no need to resizing, process the move immediately */
    if (!view_action_change_size(view->pending.configure_action) &&
        xsurface->width == xwayland_scale(pending->width) &&
        xsurface->height == xwayland_scale(pending->height)) {
        view->pending.action &= ~VIEW_ACTION_RESIZE;
        xwayland_view_move(xwayland_view, pending->x, pending->y);
        view_configure(&xwayland_view->view, 0);
        view_configured(&xwayland_view->view, true);
        return;
    }

    /* there is no commit after map */
    if (view->base.has_initial_position) {
        view_helper_move(view, pending->x, pending->y);
    }

    if (xwayland_view->xsurface->sync.counter) {
        /* skip the configure if prev one is not acked when resizing */
        if (xwayland_view->xsurface->sync.blocked && view->current_resize_edges != KYWC_EDGE_NONE) {
            view_configure(&xwayland_view->view, xwayland_view->xsurface->sync.counter);
            return;
        }
        if (!xwayland_view->xsurface->sync.blocked) {
            xwayland_surface_set_allow_commits(xsurface, false);
        }
        xwayland_surface_send_sync_request(xsurface, &xsurface->sync.value);
    }

    xwayland_surface_configure(xsurface, xwayland_scale(pending->x), xwayland_scale(pending->y),
                               xwayland_scale(pending->width), xwayland_scale(pending->height));
    /* configure add timeout handle */
    view_configure(&xwayland_view->view, 1);
}

static void xwayland_view_configure_timeout(struct view *view)
{
    struct xwayland_view *xwayland_view = xwayland_view_from_view(view);
    struct xwayland_surface *xsurface = xwayland_view->xsurface;
    /* ack_synced but not committed */
    if (!xwayland_view->xsurface->sync.counter || !xwayland_view->xsurface->sync.blocked) {
        return;
    }
    xwayland_surface_set_allow_commits(xsurface, true);
}

static struct wlr_buffer *xwayland_view_get_wm_icon_buffer(struct view *view, int size, float scale)
{
    struct xwayland_view *xwayland_view = xwayland_view_from_view(view);

    return xwayland_surface_get_icon_buffer(xwayland_view->xsurface, size, scale);
}

static void xwayland_view_update_usable_area(struct view *view, struct kywc_output *output,
                                             struct kywc_box *usable_area, enum kywc_edges edge)
{
    struct xwayland_view *xwayland_view = wl_container_of(view, xwayland_view, view);
    struct xwayland_surface *xsurface = xwayland_view->xsurface;

    struct kywc_box geo;
    kywc_output_effective_geometry(xwayland_view->view.output, &geo);

    xcb_ewmh_wm_strut_partial_t *strut = xsurface->strut_partial;
    if (strut) {
        if (strut->left_start_y != strut->left_end_y) {
            int scaled_left = xwayland_unscale(strut->left);
            geo.x += scaled_left;
            geo.width -= scaled_left;
        }
        if (strut->right_start_y != strut->right_end_y) {
            geo.width -= xwayland_unscale(strut->right);
        }
        if (strut->top_start_x != strut->top_end_x) {
            int scaled_top = xwayland_unscale(strut->top);
            geo.y += scaled_top;
            geo.height -= scaled_top;
        }
        if (strut->bottom_start_x != strut->bottom_end_x) {
            geo.height -= xwayland_unscale(strut->bottom);
        }
    }

    if (kywc_box_empty(&geo)) {
        return;
    }

    /* intersect usable_area and geo */
    usable_area->x = MAX(geo.x, usable_area->x);
    usable_area->y = MAX(geo.y, usable_area->y);
    usable_area->width = MIN(geo.width, usable_area->width);
    usable_area->height = MIN(geo.height, usable_area->height);
}

static const struct view_impl xwl_surface_impl = {
    .ping = xwayland_view_ping,
    .configure = xwayland_view_configure,
    .close = xwayland_view_close,
    .destroy = xwayland_view_destroy,
    .configure_timeout = xwayland_view_configure_timeout,
    .update_usable_area = xwayland_view_update_usable_area,
    .get_icon_buffer = xwayland_view_get_wm_icon_buffer,
};

static const struct xwayland_surface_impl view_surface_impl;

static bool xwayland_surface_is_view(struct xwayland_surface *xsurface)
{
    return xsurface && xsurface->impl == &view_surface_impl;
}

static bool xwayland_surface_is_window(struct xwayland_surface *xsurface)
{
    return xwayland_surface_is_view(xsurface) && xsurface->surface && xsurface->class &&
           *xsurface->class &&
           (xwayland_surface_has_type(xsurface, NET_WM_WINDOW_TYPE_NORMAL) ||
            xwayland_surface_has_type(xsurface, NET_WM_WINDOW_TYPE_DIALOG));
}

static bool xwayland_view_fixup_parent(struct xwayland_view *xwayland_view)
{
    if (xwayland_surface_is_window(xwayland_view->xsurface->parent)) {
        return true;
    }

    struct xwm *xwm = xwayland_view->xsurface->xwm;
    struct view *parent = NULL;
    struct xwayland_surface *xsurface;
    wl_list_for_each(xsurface, &xwm->surfaces, link) {
        if (!xsurface->surface || !xsurface->surface->mapped ||
            xsurface == xwayland_view->xsurface || xsurface->pid != xwayland_view->xsurface->pid) {
            continue;
        }
        if (xwayland_surface_is_window(xsurface)) {
            parent = &xsurface->xwayland_view->view;
        }
    }
    view_set_parent(&xwayland_view->view, parent);
    return !!parent;
}

static void xwayland_view_handle_view_above(struct wl_listener *listener, void *data)
{
    struct xwayland_view *xwayland_view = wl_container_of(listener, xwayland_view, view_above);
    struct xwayland_surface *xsurface = xwayland_view->xsurface;
    struct kywc_view *kywc_view = &xwayland_view->view.base;

    if (xsurface->above != kywc_view->kept_above) {
        xwayland_surface_set_above(xsurface, kywc_view->kept_above);
    }
}

static void xwayland_view_handle_view_below(struct wl_listener *listener, void *data)
{
    struct xwayland_view *xwayland_view = wl_container_of(listener, xwayland_view, view_below);
    struct xwayland_surface *xsurface = xwayland_view->xsurface;
    struct kywc_view *kywc_view = &xwayland_view->view.base;

    if (xsurface->below != kywc_view->kept_below) {
        xwayland_surface_set_below(xsurface, kywc_view->kept_below);
    }
}

static void xwayland_view_handle_view_sticky(struct wl_listener *listener, void *data)
{
    struct xwayland_view *xwayland_view = wl_container_of(listener, xwayland_view, view_sticky);
    struct xwayland_surface *xsurface = xwayland_view->xsurface;
    struct kywc_view *kywc_view = &xwayland_view->view.base;

    if (xsurface->sticky != kywc_view->sticky) {
        xwayland_surface_set_sticky(xsurface, kywc_view->sticky);
    }
}

static void xwayland_view_handle_view_skip_taskbar(struct wl_listener *listener, void *data)
{
    struct xwayland_view *xwayland_view =
        wl_container_of(listener, xwayland_view, view_skip_taskbar);
    struct xwayland_surface *xsurface = xwayland_view->xsurface;
    struct kywc_view *kywc_view = &xwayland_view->view.base;

    if (xsurface->skip_taskbar != kywc_view->skip_taskbar) {
        xwayland_surface_set_skip_taskbar(xsurface, kywc_view->skip_taskbar);
    }
}

static void xwayland_view_handle_view_skip_switcher(struct wl_listener *listener, void *data)
{
    struct xwayland_view *xwayland_view =
        wl_container_of(listener, xwayland_view, view_skip_switcher);
    struct xwayland_surface *xsurface = xwayland_view->xsurface;
    struct kywc_view *kywc_view = &xwayland_view->view.base;

    if (xsurface->skip_switcher != kywc_view->skip_switcher) {
        xwayland_surface_set_skip_switcher(xsurface, kywc_view->skip_switcher);
    }
}

static void xwayland_view_handle_view_demands_attention(struct wl_listener *listener, void *data)
{
    struct xwayland_view *xwayland_view =
        wl_container_of(listener, xwayland_view, view_demands_attention);
    struct xwayland_surface *xsurface = xwayland_view->xsurface;
    struct kywc_view *kywc_view = &xwayland_view->view.base;

    if (xsurface->demands_attention != kywc_view->demands_attention) {
        xwayland_surface_set_demands_attention(xsurface, kywc_view->demands_attention);
    }
}

static void xwayland_view_update_geometry(struct xwayland_view *xwayland_view)
{
    struct xwayland_surface *xsurface = xwayland_view->xsurface;
    if (!xsurface->surface) {
        return;
    }

    struct wlr_surface_state *state = &xsurface->surface->current;
    xcb_size_hints_t *size_hints = xwayland_view->xsurface->size_hints;

    if (!size_hints) {
        view_update_size(&xwayland_view->view, state->width, state->height, 0, 0, 0, 0);
    } else {
        /* convert -1 to zero followed by xdg-shell */
        view_update_size(&xwayland_view->view, state->width, state->height,
                         size_hints->min_width < 0 ? 0 : xwayland_unscale(size_hints->min_width),
                         size_hints->min_height < 0 ? 0 : xwayland_unscale(size_hints->min_height),
                         size_hints->max_width < 0 ? 0 : xwayland_unscale(size_hints->max_width),
                         size_hints->max_height < 0 ? 0 : xwayland_unscale(size_hints->max_height));
    }
}

static void xwayland_view_adjust_geometry(struct xwayland_view *xwayland_view, struct kywc_box *geo)
{
    struct kywc_view *kywc_view = &xwayland_view->view.base;
    struct kywc_output *kywc_output = kywc_output_at_point(geo->x, geo->y);

    if (kywc_view->fullscreen) {
        kywc_output_effective_geometry(kywc_output, geo);
        return;
    } else if (kywc_view->maximized) {
        view_get_tiled_geometry(&xwayland_view->view, geo, kywc_output, KYWC_TILE_ALL);
        return;
    } else if (kywc_view->tiled) {
        view_get_tiled_geometry(&xwayland_view->view, geo, kywc_output, kywc_view->tiled);
        return;
    }

    int max_width = kywc_view->max_width;
    int max_height = kywc_view->max_height;
    int width = 0, height = 0;
    output_layout_get_size(&width, &height);
    if (max_width <= 0 || max_width > width - kywc_view->margin.off_width) {
        max_width = width - kywc_view->margin.off_width;
    }
    if (max_height <= 0 || max_height > height - kywc_view->margin.off_height) {
        max_height = height - kywc_view->margin.off_height;
    }
    geo->width = CLAMP(geo->width, kywc_view->min_width, max_width);
    geo->height = CLAMP(geo->height, kywc_view->min_height, max_height);

    if (xwayland_view->view.exclusive_zone) {
        return;
    }

    struct output *output = output_from_kywc_output(kywc_output);
    if (kywc_view->has_initial_position) {
        int min_x = output->usable_area.x + kywc_view->margin.off_x;
        int min_y = output->usable_area.y + kywc_view->margin.off_y;
        int max_x = output->usable_area.x + output->usable_area.width;
        int max_y = output->usable_area.y + output->usable_area.height;
        geo->x = MAX(min_x, geo->x);
        geo->y = MAX(min_y, geo->y);

        if (geo->x > max_x) {
            geo->x = geo->width > output->usable_area.width ? min_x : max_x - geo->width;
        }
        if (geo->y > max_y) {
            geo->y = geo->height > output->usable_area.height ? min_y : max_y - geo->height;
        }
    } else {
        window_move_constraints(kywc_view, output, &geo->x, &geo->y, geo->width, geo->height);
    }
}

static void view_surface_commit(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;
    struct kywc_box geo = xwayland_view->view.base.geometry;
    struct kywc_box *current = &xwayland_view->view.base.geometry;
    uint32_t resize_edges = xwayland_view->view.current_resize_edges;

    xwayland_view_update_geometry(xwayland_view);

    enum view_action pending_action = xwayland_view->view.pending.configure_action;
    if (pending_action == VIEW_ACTION_NOP) {
        /* fix position when resizing by left or top edges */
        int x = resize_edges & KYWC_EDGE_LEFT ? geo.x + geo.width - current->width : geo.x;
        int y = resize_edges & KYWC_EDGE_TOP ? geo.y + geo.height - current->height : geo.y;
        if (x != geo.x || y != geo.y) {
            xwayland_view_move(xwayland_view, x, y);
        }
        return;
    }

    assert(view_action_change_size(pending_action));

    struct kywc_box *pending = &xwayland_view->view.pending.configure_geometry;
    int x = pending->x, y = pending->y;

    if (pending_action & VIEW_ACTION_RESIZE) {
        if (resize_edges & KYWC_EDGE_LEFT) {
            x += pending->width - current->width;
        }
        if (resize_edges & KYWC_EDGE_TOP) {
            y += pending->height - current->height;
        }
    }

    xwayland_view_move(xwayland_view, x, y);
    /* don't clear configure timer if this commit is handled after block */
    view_configured(&xwayland_view->view, !xwayland_view->xsurface->sync.blocked);
}

static void view_surface_request_move(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;
    struct seat *seat = xsurface->xwm->seat;

    if (view_validate_move_or_resize_request(&xwayland_view->view, seat)) {
        window_begin_move(&xwayland_view->view, seat, 0);
    }
}

static void view_surface_request_resize(struct xwayland_surface *xsurface, uint32_t edges)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;
    struct seat *seat = xsurface->xwm->seat;

    if (view_validate_move_or_resize_request(&xwayland_view->view, seat)) {
        window_begin_resize(&xwayland_view->view, edges, seat, 0);
    }
}

static void view_surface_request_minimize(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;
    kywc_view_set_minimized(&xwayland_view->view.base, xsurface->minimized);
}

static void view_surface_request_maximize(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;

    bool maximized = xsurface->maximized_horz && xsurface->maximized_vert;
    kywc_view_set_maximized(&xwayland_view->view.base, maximized, NULL);
}

static void view_surface_request_fullscreen(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;

    bool fullscreen = xsurface->fullscreen;
    kywc_view_set_fullscreen(&xwayland_view->view.base, fullscreen, NULL);
}

static void view_surface_request_activate(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;
    if (!xwayland_view->view.base.mapped) {
        return;
    }

    kywc_view_activate(&xwayland_view->view.base);
    if (xwayland_view->view.base.activated) {
        xwayland_surface_activate(xwayland_view->xsurface, true);
    }

    struct seat *seat =
        xsurface->xwm->seat ? xsurface->xwm->seat : input_manager_get_default_seat();
    view_set_focus(&xwayland_view->view, seat);
}

static void view_surface_request_sticky(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;

    bool sticky = xsurface->sticky;
    kywc_view_set_sticky(&xwayland_view->view.base, sticky);
}

static void view_surface_request_skip_taskbar(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;

    bool skip_taskbar = xsurface->skip_taskbar;
    kywc_view_set_skip_taskbar(&xwayland_view->view.base, skip_taskbar);
}

static void view_surface_request_skip_switcher(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;

    bool skip_switcher = xsurface->skip_switcher;
    kywc_view_set_skip_switcher(&xwayland_view->view.base, skip_switcher);
}

static void view_surface_request_above(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;

    bool above = xsurface->above;
    kywc_view_set_kept_above(&xwayland_view->view.base, above);
}

static void view_surface_request_below(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;

    bool below = xsurface->below;
    kywc_view_set_kept_below(&xwayland_view->view.base, below);
}

static void view_surface_request_demands_attention(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;

    bool demands_attention = xsurface->demands_attention;
    kywc_view_set_demands_attention(&xwayland_view->view.base, demands_attention);
}

static bool xwayland_view_fixup_parent(struct xwayland_view *xwayland_view);

static void view_surface_request_modal(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;

    bool modal = xsurface->modal && xsurface->parent && xwayland_view_fixup_parent(xwayland_view);
    kywc_view_set_modal(&xwayland_view->view.base, modal);
}

static void view_surface_request_move_resize(struct xwayland_surface *xsurface,
                                             struct kywc_box *box)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;
    struct kywc_box geo = { xwayland_unscale(box->x), xwayland_unscale(box->y),
                            xwayland_unscale(box->width), xwayland_unscale(box->height) };
    xwayland_view_adjust_geometry(xwayland_view, &geo);
    view_do_resize(&xwayland_view->view, &geo);
}

static void view_surface_set_title(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;

    view_set_title(&xwayland_view->view, xsurface->title);
}

static void view_surface_set_class(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;

    view_set_app_id(&xwayland_view->view, xsurface->class);
}

static void view_surface_set_parent(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;
    struct xwayland_surface *parent = xwayland_view->xsurface->parent;
    struct xwayland_view *parent_xwayland_view =
        xwayland_surface_is_view(parent) ? parent->xwayland_view : NULL;
    struct view *parent_view = parent_xwayland_view ? &parent_xwayland_view->view : NULL;

    view_set_parent(&xwayland_view->view, parent_view);
}

static void view_surface_set_hints(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;
    enum xwayland_icccm_input_model input_model =
        xwayland_surface_icccm_input_model(xwayland_view->xsurface);

    if (input_model == ICCCM_INPUT_MODEL_NONE) {
        xwayland_view->focusable = xwayland_view->activatable = false;
    } else if (input_model == ICCCM_INPUT_MODEL_GLOBAL) {
        xwayland_view->activatable = true;
        xwayland_view->focusable = false;
    } else {
        xwayland_view->focusable = xwayland_view->activatable = true;
    }
    view_update_capabilities(&xwayland_view->view, KYWC_VIEW_FOCUSABLE | KYWC_VIEW_ACTIVATABLE);
}

static void view_surface_set_size_hints(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;
    xwayland_view_update_geometry(xwayland_view);
}

static void view_surface_set_functions(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;

    bool new_minimizable = !(xsurface->functions & XWAYLAND_SURFACE_FUNCTIONS_NO_MINIMIZE);
    bool new_maximizable = !(xsurface->functions & XWAYLAND_SURFACE_FUNCTIONS_NO_MAXIMIZE);

    if (xwayland_view->minimizable != new_minimizable) {
        xwayland_view->minimizable = new_minimizable;
        view_update_capabilities(&xwayland_view->view,
                                 KYWC_VIEW_MINIMIZABLE | KYWC_VIEW_MINIMIZE_BUTTON);
    }
    if (xwayland_view->maximizable != new_maximizable) {
        xwayland_view->maximizable = new_maximizable;
        view_update_capabilities(&xwayland_view->view,
                                 KYWC_VIEW_MAXIMIZABLE | KYWC_VIEW_MAXIMIZE_BUTTON);
    }
}

static void view_surface_set_decorations(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;

    if (xwayland_surface_has_type(xsurface, NET_WM_WINDOW_TYPE_DOCK) ||
        xwayland_surface_has_type(xsurface, NET_WM_WINDOW_TYPE_SPLASH) ||
        xwayland_surface_has_type(xsurface, KDE_NET_WM_WINDOW_TYPE_OVERRIDE)) {
        view_set_decoration(&xwayland_view->view, KYWC_SSD_NONE);
        return;
    }

    /* don't add shadow if surface decoration is 0 */
    if (xsurface->decorations == KYWC_SSD_RESIZE) {
        xsurface->decorations = KYWC_SSD_NONE;
    }
    view_set_decoration(&xwayland_view->view, xsurface->decorations);
}

static void view_surface_set_strut_partial(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;
    xcb_ewmh_wm_strut_partial_t *strut = xsurface->strut_partial;
    bool has_area =
        strut &&
        (strut->left_start_y != strut->left_end_y || strut->right_start_y != strut->right_end_y ||
         strut->top_start_x != strut->top_end_x || strut->bottom_start_x != strut->bottom_end_x);

    xwayland_view->movable = !has_area;
    view_update_capabilities(&xwayland_view->view, KYWC_VIEW_MOVABLE);
    xwayland_view->view.base.unconstrained = has_area;
    view_set_exclusive(&xwayland_view->view, has_area);
}

static void view_surface_set_icon(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;

    /* update icon */
    view_set_icon(&xwayland_view->view, true);
}

static void view_surface_ping_timeout(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;
    view_show_hang_window(&xwayland_view->view);
}

static void view_surface_try_reconfigure(struct xwayland_surface *xsurface,
                                         struct kywc_output *output)
{
    struct view *view = &xsurface->xwayland_view->view;
    if (!view->base.mapped || view->output == output) {
        return;
    }

    int lx = view->base.geometry.x - view->base.margin.off_x;
    int ly = view->base.geometry.y - view->base.margin.off_y;
    if (!kywc_output_contains_point(view->output, lx, ly)) {
        return;
    }

    view_move_to_output(view, NULL, NULL, view->output);
}

static void xwayland_view_apply_type(struct xwayland_view *xwayland_view)
{
    struct xwayland_surface *surface = xwayland_view->xsurface;
    struct view_layer *layer = NULL;
    bool removed_from_workspace = false;

    if (xwayland_surface_has_type(surface, NET_WM_WINDOW_TYPE_DESKTOP)) {
        layer = view_manager_get_layer(LAYER_DESKTOP, false);
        xwayland_view->resizable = false;
        view_update_capabilities(&xwayland_view->view, KYWC_VIEW_RESIZABLE);
        xwayland_view->view.base.role = KYWC_VIEW_ROLE_DESKTOP;
        removed_from_workspace = true;
    } else if (xwayland_surface_has_type(surface, NET_WM_WINDOW_TYPE_DOCK)) {
        layer = view_manager_get_layer(LAYER_DOCK, false);
        xwayland_view->focusable = xwayland_view->resizable = false;
        view_update_capabilities(&xwayland_view->view,
                                 KYWC_VIEW_FOCUSABLE | KYWC_VIEW_ACTIVATABLE | KYWC_VIEW_RESIZABLE);
        xwayland_view->view.base.role = KYWC_VIEW_ROLE_PANEL;
        removed_from_workspace = true;
    } else if (xwayland_surface_has_type(surface, NET_WM_WINDOW_TYPE_UTILITY)) {
        xwayland_view->view.base.skip_taskbar = true;
        if (!xwayland_surface_has_input(surface, INPUT_MASK_KEYBOARD)) {
            xwayland_view->focusable = xwayland_view->activatable = false;
            view_update_capabilities(&xwayland_view->view,
                                     KYWC_VIEW_FOCUSABLE | KYWC_VIEW_ACTIVATABLE);
        }
    } else if (xwayland_surface_has_type(surface, KDE_NET_WM_WINDOW_TYPE_ON_SCREEN_DISPLAY)) {
        layer = view_manager_get_layer(LAYER_ON_SCREEN_DISPLAY, false);
        xwayland_view->view.base.role = KYWC_VIEW_ROLE_ONSCREENDISPLAY;
        removed_from_workspace = true;
    }

    if (removed_from_workspace) {
        view_unset_workspace(&xwayland_view->view, layer);
    }

    xwayland_view->view.base.has_round_corner =
        xwayland_surface_has_type(surface, NET_WM_WINDOW_TYPE_NORMAL) ||
        xwayland_surface_has_type(surface, NET_WM_WINDOW_TYPE_DIALOG);
}

static void xwayland_view_fixup_geometry(struct xwayland_view *xwayland_view)
{
    struct xwayland_surface *xsurface = xwayland_view->xsurface;
    struct kywc_view *kywc_view = &xwayland_view->view.base;
    struct kywc_box geo = kywc_view->geometry;

    if (xsurface->x != 0 || xsurface->y != 0) {
        geo.x = xwayland_unscale(xsurface->x);
        geo.y = xwayland_unscale(xsurface->y);
        kywc_view->has_initial_position = true;
    } else {
        /* apply the position in size_hints */
        xcb_size_hints_t *size_hints = xsurface->size_hints;
        if (size_hints && size_hints->flags &
                              (XCB_ICCCM_SIZE_HINT_US_POSITION | XCB_ICCCM_SIZE_HINT_P_POSITION)) {
            geo.x = xwayland_unscale(size_hints->x);
            geo.y = xwayland_unscale(size_hints->y);
            kywc_view->has_initial_position = true;
        }
    }

    xwayland_view_adjust_geometry(xwayland_view, &geo);
    view_do_resize(&xwayland_view->view, &geo);
}

static void view_surface_map(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;

    xwayland_view_update_geometry(xwayland_view);
    xwayland_view_apply_type(xwayland_view);
    view_surface_set_decorations(xsurface);
    /* after set decorations to get view margin */
    xwayland_view_fixup_geometry(xwayland_view);

    assert(xsurface->surface == xwayland_view->view.surface);
    xwayland_view->view_stack.notify = xwayland_view_handle_view_stack;
    wl_signal_add(&xwayland_view->view.events.stack, &xwayland_view->view_stack);
    xwayland_view->view_above.notify = xwayland_view_handle_view_above;
    wl_signal_add(&xwayland_view->view.base.events.above, &xwayland_view->view_above);
    xwayland_view->view_below.notify = xwayland_view_handle_view_below;
    wl_signal_add(&xwayland_view->view.base.events.below, &xwayland_view->view_below);
    xwayland_view->view_sticky.notify = xwayland_view_handle_view_sticky;
    wl_signal_add(&xwayland_view->view.base.events.sticky, &xwayland_view->view_sticky);
    xwayland_view->view_skip_taskbar.notify = xwayland_view_handle_view_skip_taskbar;
    wl_signal_add(&xwayland_view->view.base.events.skip_taskbar, &xwayland_view->view_skip_taskbar);
    xwayland_view->view_skip_switcher.notify = xwayland_view_handle_view_skip_switcher;
    wl_signal_add(&xwayland_view->view.base.events.skip_switcher,
                  &xwayland_view->view_skip_switcher);
    xwayland_view->view_demands_attention.notify = xwayland_view_handle_view_demands_attention;
    wl_signal_add(&xwayland_view->view.base.events.demands_attention,
                  &xwayland_view->view_demands_attention);

    view_map(&xwayland_view->view);
}

static void view_surface_unmap(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;

    wl_list_remove(&xwayland_view->view_stack.link);
    wl_list_remove(&xwayland_view->view_above.link);
    wl_list_remove(&xwayland_view->view_below.link);
    wl_list_remove(&xwayland_view->view_sticky.link);
    wl_list_remove(&xwayland_view->view_skip_taskbar.link);
    wl_list_remove(&xwayland_view->view_skip_switcher.link);
    wl_list_remove(&xwayland_view->view_demands_attention.link);

    /* surface_tree is destroyed by scene subsurface */
    view_unmap(&xwayland_view->view);
}

static void xwayland_view_handle_surface_tree_destroy(struct wl_listener *listener, void *data)
{
    struct xwayland_view *xwayland_view =
        wl_container_of(listener, xwayland_view, surface_tree_destroy);
    wl_list_remove(&xwayland_view->surface_tree_destroy.link);
    xwayland_view->view.surface_tree = NULL;
    xwayland_view->xsurface->surface_node = NULL;
}

static struct scene_node *view_surface_associate(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;

    xsurface->surface->data = &xwayland_view->view;
    xwayland_view->view.surface = xsurface->surface;
    /* create scene tree here as we get surface here */
    xwayland_view->view.surface_tree =
        scene_subsurface_tree_create(xwayland_view->view.tree, xsurface->surface);
    scene_node_set_role(&xwayland_view->view.surface_tree->node, SCENE_ROLE_XWAYLAND_WINDOW, NULL,
                        xsurface);
    struct scene_node *surface_node = &scene_buffer_try_from_surface(xsurface->surface)->node;

    /* event node will be destroyed when surface_node destroy */
    input_event_node_create(&xwayland_view->view.surface_tree->node, &xwayland_view_event_node_impl,
                            xwayland_view_get_root, xwayland_view_get_toplevel, xwayland_view);

    xwayland_view->surface_tree_destroy.notify = xwayland_view_handle_surface_tree_destroy;
    wl_signal_add(&xwayland_view->view.surface_tree->node.events.destroy,
                  &xwayland_view->surface_tree_destroy);

    xsurface->surface_node = surface_node;
    return surface_node;
}

static void view_surface_dissociate(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;

    xsurface->surface->data = NULL;
    xwayland_view->view.surface = NULL;
    if (xwayland_view->view.surface_tree) {
        scene_node_destroy(&xwayland_view->view.surface_tree->node);
    }
}

static void view_surface_destroy(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;
    xsurface->xwayland_view = NULL;
    wl_list_remove(&xwayland_view->view_update_capabilities.link);
    view_destroy(&xwayland_view->view);
}

static void view_surface_request_configure(struct xwayland_surface *xsurface,
                                           struct xwayland_surface_request_configure *event)
{
    struct xwayland_view *xwayland_view = xsurface->xwayland_view;
    struct kywc_view *kywc_view = &xwayland_view->view.base;

    /* skip configure when moving or resizing by left or top edges */
    if (kywc_view->mapped &&
        (xwayland_view->view.interactive_moving ||
         (xwayland_view->view.current_resize_edges & (KYWC_EDGE_LEFT | KYWC_EDGE_TOP)))) {
        xwayland_surface_configure(xsurface, xsurface->x, xsurface->y, xsurface->width,
                                   xsurface->height);
        return;
    }

    struct kywc_box geo = { xwayland_unscale(event->x), xwayland_unscale(event->y),
                            xwayland_unscale(event->width), xwayland_unscale(event->height) };

    if (!kywc_view->mapped) {
        xwayland_surface_configure(xsurface, event->x, event->y, event->width, event->height);
    } else {
        xwayland_view_adjust_geometry(xwayland_view, &geo);
    }

    view_do_resize(&xwayland_view->view, &geo);
}

static const struct xwayland_surface_impl view_surface_impl = {
    .associate = view_surface_associate,
    .dissociate = view_surface_dissociate,
    .commit = view_surface_commit,
    .map = view_surface_map,
    .unmap = view_surface_unmap,
    .destroy = view_surface_destroy,

    .request_configure = view_surface_request_configure,
    .request_move = view_surface_request_move,
    .request_resize = view_surface_request_resize,
    .request_minimize = view_surface_request_minimize,
    .request_maximize = view_surface_request_maximize,
    .request_fullscreen = view_surface_request_fullscreen,
    .request_activate = view_surface_request_activate,
    .request_sticky = view_surface_request_sticky,
    .request_skip_taskbar = view_surface_request_skip_taskbar,
    .request_skip_switcher = view_surface_request_skip_switcher,
    .request_above = view_surface_request_above,
    .request_below = view_surface_request_below,
    .request_demands_attention = view_surface_request_demands_attention,
    .request_modal = view_surface_request_modal,
    .request_move_resize = view_surface_request_move_resize,

    .set_title = view_surface_set_title,
    .set_class = view_surface_set_class,
    .set_parent = view_surface_set_parent,
    .set_hints = view_surface_set_hints,
    .set_size_hints = view_surface_set_size_hints,
    .set_functions = view_surface_set_functions,
    .set_decorations = view_surface_set_decorations,
    .set_strut_partial = view_surface_set_strut_partial,
    .set_icon = view_surface_set_icon,

    .ping_timeout = view_surface_ping_timeout,
    .try_reconfigure = view_surface_try_reconfigure,
};

static void xwayland_view_handle_update_capabilities(struct wl_listener *listener, void *data)
{
    struct xwayland_view *xwayland_view =
        wl_container_of(listener, xwayland_view, view_update_capabilities);
    struct view_update_capabilities_event *event = data;

    if (event->mask & KYWC_VIEW_MINIMIZABLE) {
        if (!xwayland_view->minimizable) {
            event->state &= ~KYWC_VIEW_MINIMIZABLE;
        }
    }
    if (event->mask & KYWC_VIEW_MAXIMIZABLE) {
        if (!xwayland_view->maximizable) {
            event->state &= ~KYWC_VIEW_MAXIMIZABLE;
        }
    }
    if (event->mask & KYWC_VIEW_FOCUSABLE) {
        if (!xwayland_view->focusable) {
            event->state &= ~KYWC_VIEW_FOCUSABLE;
        }
    }
    if (event->mask & KYWC_VIEW_ACTIVATABLE) {
        if (!xwayland_view->activatable) {
            event->state &= ~KYWC_VIEW_ACTIVATABLE;
        }
    }
    if (event->mask & KYWC_VIEW_MOVABLE) {
        if (!xwayland_view->movable) {
            event->state &= ~KYWC_VIEW_MOVABLE;
        }
    }
    if (event->mask & KYWC_VIEW_RESIZABLE) {
        if (!xwayland_view->resizable) {
            event->state &= ~KYWC_VIEW_RESIZABLE;
        }
    }
    if (event->mask & KYWC_VIEW_MINIMIZE_BUTTON) {
        if (!xwayland_view->minimizable) {
            event->state &= ~KYWC_VIEW_MINIMIZE_BUTTON;
        }
    }
    if (event->mask & KYWC_VIEW_MAXIMIZE_BUTTON) {
        if (!xwayland_view->maximizable) {
            event->state &= ~KYWC_VIEW_MAXIMIZE_BUTTON;
        }
    }
}

void xwayland_view_create(struct xwayland_surface *xsurface)
{
    struct xwayland_view *xwayland_view = calloc(1, sizeof(*xwayland_view));
    if (!xwayland_view) {
        return;
    }

    xsurface->impl = &view_surface_impl;
    xwayland_view->xsurface = xsurface;
    xsurface->xwayland_view = xwayland_view;
    xwayland_view->view.base.pid = xsurface->pid;
    xwayland_view->view.base.exec = client_get_comm(xwayland_view->view.base.pid);

    view_init(&xwayland_view->view, &xwl_surface_impl, xwayland_view);

    xwayland_view->minimizable = xwayland_view->maximizable = xwayland_view->focusable =
        xwayland_view->activatable = xwayland_view->movable = xwayland_view->resizable = true;
    xwayland_view->view_update_capabilities.notify = xwayland_view_handle_update_capabilities;
    view_add_update_capabilities_listener(&xwayland_view->view,
                                          &xwayland_view->view_update_capabilities);

    if (xsurface->surface) {
        view_surface_associate(xsurface);
        if (xsurface->surface->mapped) {
            view_surface_map(xsurface);
        }
    }
}

bool xwayland_check_view(struct view *view)
{
    return view->impl == &xwl_surface_impl;
}
