// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include <wlr/types/wlr_compositor.h>

#include "input/cursor.h"
#include "input/seat.h"
#include "plasma-shell-protocol.h"
#include "scene/surface.h"
#include "util/wayland.h"
#include "view_p.h"

#define PLASMA_SHELL_VERSION 8

struct kde_plasma_shell {
    struct wl_global *global;

    struct wl_listener display_destroy;
    struct wl_listener server_destroy;
};

struct kde_plasma_surface {
    struct wlr_surface *wlr_surface;
    struct wl_listener surface_map;
    struct wl_listener surface_destroy;

    /* for popup position */
    struct scene_buffer *buffer;

    /* get in map listener */
    struct view *view;
    bool takes_focus;
    struct wl_listener view_map;
    struct wl_listener view_unmap;
    struct wl_listener view_destroy;
    struct wl_listener view_update_capabilities;

    int32_t x, y;
    enum org_kde_plasma_surface_role role;
    int32_t skip_taskbar, skip_switcher;
    bool open_under_cursor;
};

static void handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static void handle_set_output(struct wl_client *client, struct wl_resource *resource,
                              struct wl_resource *output)
{
    // Not implemented yet
}

static void kde_plasma_surface_apply_position(struct kde_plasma_surface *surface)
{
    if (surface->view) {
        view_do_move(surface->view, surface->x, surface->y);
    } else if (surface->buffer) {
        struct scene_node *node = &surface->buffer->node;
        int lx, ly;
        scene_node_coords(node, &lx, &ly);
        scene_node_set_position(&surface->buffer->node, node->x + surface->x - lx,
                                node->y + surface->y - ly);
    }
}

static void handle_set_position(struct wl_client *client, struct wl_resource *resource, int32_t x,
                                int32_t y)
{
    struct kde_plasma_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface) {
        return;
    }

    surface->x = x;
    surface->y = y;

    kde_plasma_surface_apply_position(surface);
}

static enum kywc_view_role role_from_plasma_surface(struct kde_plasma_surface *surface)
{
    switch (surface->role) {
    default:
        return KYWC_VIEW_ROLE_NORMAL;
    case ORG_KDE_PLASMA_SURFACE_ROLE_DESKTOP:
        return KYWC_VIEW_ROLE_DESKTOP;
    case ORG_KDE_PLASMA_SURFACE_ROLE_PANEL:
        return KYWC_VIEW_ROLE_PANEL;
    case ORG_KDE_PLASMA_SURFACE_ROLE_TOOLTIP:
        return KYWC_VIEW_ROLE_TOOLTIP;
    case ORG_KDE_PLASMA_SURFACE_ROLE_ONSCREENDISPLAY:
        return KYWC_VIEW_ROLE_ONSCREENDISPLAY;
    case ORG_KDE_PLASMA_SURFACE_ROLE_NOTIFICATION:
        return KYWC_VIEW_ROLE_NOTIFICATION;
    case ORG_KDE_PLASMA_SURFACE_ROLE_CRITICALNOTIFICATION:
        return KYWC_VIEW_ROLE_CRITICALNOTIFICATION;
    case ORG_KDE_PLASMA_SURFACE_ROLE_APPLETPOPUP:
        return KYWC_VIEW_ROLE_APPLETPOPUP;
    }
}

static void handle_set_role(struct wl_client *client, struct wl_resource *resource, uint32_t role)
{
    struct kde_plasma_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface) {
        return;
    }

    if (surface->view) {
        kywc_log(KYWC_WARN, "Plasma-shell: reject set_role(%d) request after mapped from %s", role,
                 surface->view->base.app_id);
        return;
    }

    surface->role = role;
}

static void handle_set_panel_behavior(struct wl_client *client, struct wl_resource *resource,
                                      uint32_t flag)
{
    // Not implemented yet
}

static void handle_set_skip_taskbar(struct wl_client *client, struct wl_resource *resource,
                                    uint32_t skip)
{
    struct kde_plasma_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface) {
        return;
    }

    surface->skip_taskbar = skip;

    if (surface->view) {
        kywc_view_set_skip_taskbar(&surface->view->base, skip);
    }
}

static void handle_panel_auto_hide_hide(struct wl_client *client, struct wl_resource *resource)
{
    // Not implemented yet
}

static void handle_panel_auto_hide_show(struct wl_client *client, struct wl_resource *resource)
{
    // Not implemented yet
}

static void handle_set_panel_takes_focus(struct wl_client *client, struct wl_resource *resource,
                                         uint32_t takes_focus)
{
    struct kde_plasma_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface || !surface->view) {
        return;
    }

    struct kywc_view *kywc_view = &surface->view->base;
    if (kywc_view->role == KYWC_VIEW_ROLE_PANEL && surface->takes_focus != takes_focus) {
        surface->takes_focus = takes_focus;
        view_update_capabilities(surface->view, KYWC_VIEW_FOCUSABLE | KYWC_VIEW_ACTIVATABLE);
    }
}

static void handle_set_skip_switcher(struct wl_client *client, struct wl_resource *resource,
                                     uint32_t skip)
{
    struct kde_plasma_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface) {
        return;
    }

    surface->skip_switcher = skip;

    if (surface->view) {
        kywc_view_set_skip_switcher(&surface->view->base, skip);
    }
}

static void handle_open_under_cursor(struct wl_client *client, struct wl_resource *resource)
{
    struct kde_plasma_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface) {
        return;
    }

    surface->open_under_cursor = true;

    if (surface->view) {
        struct seat *seat = surface->view->focused_seat;
        view_do_move(surface->view, seat->cursor->lx, seat->cursor->ly);
    }
}

static const struct org_kde_plasma_surface_interface kde_plasma_surface_impl = {
    .destroy = handle_destroy,
    .set_output = handle_set_output,
    .set_position = handle_set_position,
    .set_role = handle_set_role,
    .set_panel_behavior = handle_set_panel_behavior,
    .set_skip_taskbar = handle_set_skip_taskbar,
    .panel_auto_hide_hide = handle_panel_auto_hide_hide,
    .panel_auto_hide_show = handle_panel_auto_hide_show,
    .set_panel_takes_focus = handle_set_panel_takes_focus,
    .set_skip_switcher = handle_set_skip_switcher,
    .open_under_cursor = handle_open_under_cursor,
};

static void kde_plasma_surface_set_usable_area(struct kde_plasma_surface *surface, bool enabled)
{
    bool has_area =
        enabled && surface->view->base.mapped && surface->view->base.role == KYWC_VIEW_ROLE_PANEL;
    view_set_exclusive(surface->view, has_area);
}

static void surface_handle_view_map(struct wl_listener *listener, void *data)
{
    struct kde_plasma_surface *surface = wl_container_of(listener, surface, view_map);
    kde_plasma_surface_set_usable_area(surface, true);
}

static void surface_handle_view_unmap(struct wl_listener *listener, void *data)
{
    struct kde_plasma_surface *surface = wl_container_of(listener, surface, view_unmap);
    kde_plasma_surface_set_usable_area(surface, false);
}

static void surface_handle_view_destroy(struct wl_listener *listener, void *data)
{
    struct kde_plasma_surface *surface = wl_container_of(listener, surface, view_destroy);

    wl_list_remove(&surface->view_destroy.link);
    wl_list_remove(&surface->view_map.link);
    wl_list_remove(&surface->view_unmap.link);
    wl_list_remove(&surface->view_update_capabilities.link);

    surface->view = NULL;
}

static void surface_handle_view_update_capabilities(struct wl_listener *listener, void *data)
{
    struct kde_plasma_surface *surface =
        wl_container_of(listener, surface, view_update_capabilities);
    struct view_update_capabilities_event *event = data;

    if (event->mask & KYWC_VIEW_FOCUSABLE) {
        if (!surface->takes_focus) {
            event->state &= ~KYWC_VIEW_FOCUSABLE;
        }
    }
    if (event->mask & KYWC_VIEW_ACTIVATABLE) {
        if (!surface->takes_focus) {
            event->state &= ~KYWC_VIEW_ACTIVATABLE;
        }
    }
}

static void surface_handle_map(struct wl_listener *listener, void *data)
{
    struct kde_plasma_surface *surface = wl_container_of(listener, surface, surface_map);

    /* useless once we connect surface and view */
    wl_list_remove(&surface->surface_map.link);
    wl_list_init(&surface->surface_map.link);

    /* get view from surface */
    if (!surface->view) {
        surface->view = view_try_from_wlr_surface(surface->wlr_surface);
        if (!surface->view) {
            kywc_log(KYWC_DEBUG, "Surface is not a toplevel");
            surface->buffer = scene_buffer_try_from_surface(surface->wlr_surface);
            return;
        }
        wl_signal_add(&surface->view->base.events.destroy, &surface->view_destroy);
        view_add_update_capabilities_listener(surface->view, &surface->view_update_capabilities);
    }

    if (surface->skip_taskbar != -1) {
        kywc_view_set_skip_taskbar(&surface->view->base, surface->skip_taskbar);
    }
    if (surface->skip_switcher != -1) {
        kywc_view_set_skip_switcher(&surface->view->base, surface->skip_switcher);
    }

    if (surface->role != UINT32_MAX) {
        view_set_role(surface->view, role_from_plasma_surface(surface));
    }

    if (surface->open_under_cursor) {
        struct seat *seat = surface->view->focused_seat;
        surface->view->base.has_initial_position = true;
        view_do_move(surface->view, seat->cursor->lx, seat->cursor->ly);
    }

    /* apply set_position called before map */
    if (surface->x != INT32_MAX || surface->y != INT32_MAX) {
        surface->view->base.has_initial_position = true;
        kde_plasma_surface_apply_position(surface);
    }

    surface->view_map.notify = surface_handle_view_map;
    wl_signal_add(&surface->view->base.events.map, &surface->view_map);
    surface->view_unmap.notify = surface_handle_view_unmap;
    wl_signal_add(&surface->view->base.events.unmap, &surface->view_unmap);

    /* workaround to fix this listener is behind view map */
    if (surface->view->base.mapped) {
        surface_handle_view_map(&surface->view_map, NULL);
    }
}

static void surface_handle_destroy(struct wl_listener *listener, void *data)
{
    struct kde_plasma_surface *surface = wl_container_of(listener, surface, surface_destroy);

    wl_list_remove(&surface->surface_map.link);
    wl_list_remove(&surface->surface_destroy.link);

    surface->wlr_surface = NULL;
}

static void kde_plasma_surface_handle_resource_destroy(struct wl_resource *resource)
{
    struct kde_plasma_surface *surface = wl_resource_get_user_data(resource);

    if (surface->wlr_surface) {
        surface_handle_destroy(&surface->surface_destroy, NULL);
    }

    if (surface->view) {
        if (surface->view->base.mapped) {
            surface_handle_view_unmap(&surface->view_unmap, NULL);
        }
        surface_handle_view_destroy(&surface->view_destroy, NULL);
    }

    free(surface);
}

static void handle_get_surface(struct wl_client *client, struct wl_resource *shell_resource,
                               uint32_t id, struct wl_resource *surface_resource)
{
    struct wlr_surface *wlr_surface = wlr_surface_from_resource(surface_resource);
    /* create a plasma surface */
    struct kde_plasma_surface *surface = calloc(1, sizeof(*surface));
    if (!surface) {
        wl_client_post_no_memory(client);
        return;
    }

    int version = wl_resource_get_version(shell_resource);
    struct wl_resource *resource =
        wl_resource_create(client, &org_kde_plasma_surface_interface, version, id);
    if (!resource) {
        free(surface);
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, &kde_plasma_surface_impl, surface,
                                   kde_plasma_surface_handle_resource_destroy);

    surface->x = surface->y = INT32_MAX;
    surface->role = UINT32_MAX;
    surface->skip_taskbar = surface->skip_switcher = -1;
    surface->takes_focus = true;

    surface->wlr_surface = wlr_surface;
    surface->surface_map.notify = surface_handle_map;
    wl_signal_add_next(&wlr_surface->events.map, &surface->surface_map);
    surface->surface_destroy.notify = surface_handle_destroy;
    wl_signal_add(&wlr_surface->events.destroy, &surface->surface_destroy);

    surface->view = view_try_from_wlr_surface(surface->wlr_surface);
    surface->view_destroy.notify = surface_handle_view_destroy;

    surface->view_update_capabilities.notify = surface_handle_view_update_capabilities;

    if (surface->view) {
        wl_list_init(&surface->view_map.link);
        wl_list_init(&surface->view_unmap.link);
        wl_signal_add(&surface->view->base.events.destroy, &surface->view_destroy);
        view_add_update_capabilities_listener(surface->view, &surface->view_update_capabilities);
    }

    /* workaround to fix this request is behind surface map */
    if (surface->wlr_surface->mapped) {
        surface_handle_map(&surface->surface_map, NULL);
    }
}

static const struct org_kde_plasma_shell_interface kde_plasma_shell_impl = {
    .get_surface = handle_get_surface,
};

static void kde_plasma_shell_bind(struct wl_client *client, void *data, uint32_t version,
                                  uint32_t id)
{
    struct wl_resource *resource =
        wl_resource_create(client, &org_kde_plasma_shell_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    struct kde_plasma_shell *shell = data;
    wl_resource_set_implementation(resource, &kde_plasma_shell_impl, shell, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    struct kde_plasma_shell *shell = wl_container_of(listener, shell, display_destroy);
    wl_list_remove(&shell->display_destroy.link);
    wl_global_destroy(shell->global);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    struct kde_plasma_shell *shell = wl_container_of(listener, shell, server_destroy);
    wl_list_remove(&shell->server_destroy.link);
    free(shell);
}

bool kde_plasma_shell_create(struct server *server)
{
    struct kde_plasma_shell *shell = calloc(1, sizeof(*shell));
    if (!shell) {
        return false;
    }

    shell->global = wl_global_create(server->display, &org_kde_plasma_shell_interface,
                                     PLASMA_SHELL_VERSION, shell, kde_plasma_shell_bind);
    if (!shell->global) {
        kywc_log(KYWC_WARN, "Kde plasma shell create failed");
        free(shell);
        return false;
    }

    shell->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &shell->server_destroy);
    shell->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(server->display, &shell->display_destroy);

    return true;
}
