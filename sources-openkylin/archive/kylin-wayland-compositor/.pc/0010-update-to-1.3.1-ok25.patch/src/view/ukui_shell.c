// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>

#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xdg_shell.h>

#include "input/cursor.h"
#include "input/keyboard.h"
#include "input/seat.h"
#include "output.h"
#include "scene/surface.h"
#include "ukui-shell-v1-protocol.h"
#include "util/macros.h"
#include "util/wayland.h"
#include "view_p.h"

#define UKUI_SHELL_V1_VERSION 4

struct ukui_shell {
    struct wl_global *global;
    struct wl_list ukui_surfaces;

    struct wl_listener display_destroy;
    struct wl_listener server_destroy;
};

struct ukui_surface {
    struct wl_list link;
    struct wl_resource *resource;

    struct wlr_surface *wlr_surface;
    struct wl_listener surface_map;
    struct wl_listener surface_commit;
    struct wl_listener surface_destroy;

    /* for popup position */
    struct scene_buffer *buffer;

    bool minimizable, maximizable, fullscreenable;
    bool closeable, movable, resizable;
    bool focusable, activatable;

    /* get in map listener */
    struct view *view;
    struct wl_listener view_map;
    struct wl_listener view_unmap;
    struct wl_listener view_position;
    struct wl_listener view_update_capabilities;
    struct wl_listener view_destroy;
    struct wl_listener view_activate;
    struct wl_listener view_minimize;
    struct wl_listener view_maximize;
    struct wl_listener view_fullscreen;
    struct wl_listener view_above;
    struct wl_listener view_below;
    struct wl_listener view_tile;

    struct seat_keyboard_grab keyboard_grab;
    struct wl_list ukui_keyboard_grabs;
    char *icon_name;

    /* "surface_state" is the status return to the application */
    uint32_t surface_state;
    /* "state" is the status sent by the application */
    uint32_t state, flags;
    struct kywc_box startup;
    struct kywc_box restore;
    int32_t x, y, offset_x, offset_y;
    enum ukui_surface_v1_role role;
    int32_t skip_taskbar, skip_switcher;
    bool panel_auto_hide;
};

struct ukui_keyboard_grab {
    struct seat_keyboard_grab keyboard_grab;
    struct wl_list link;
};

struct ukui_decoration {
    struct wl_resource *resource;

    struct wlr_surface *wlr_surface;
    struct wl_listener surface_map;

    struct wlr_addon addon;

    uint32_t flags;
    int32_t no_titlebar;
};

enum surface_state_flag {
    STATE_FLAG_MINIMIZABLE = 0,
    STATE_FLAG_MAXIMIZABLE,
    STATE_FLAG_CLOSEABLE,
    STATE_FLAG_FULLSCREENABLE,
    STATE_FLAG_MOVABLE,
    STATE_FLAG_RESIZABLE,
    STATE_FLAG_FOCUSABLE,
    STATE_FLAG_ACTIVATABLE,
    STATE_FLAG_KEEPABOVE,
    STATE_FLAG_KEEPBELOW,
    STATE_FLAG_LAST,
};

static void handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static void position_apply_immediately(struct ukui_surface *surface)
{
    view_do_move(surface->view, surface->x, surface->y);
    if (surface->view->base.mapped) {
        cursor_rebase_all(false);
    }
    surface->x = surface->y = INT32_MAX;
}

static void ukui_surface_apply_position(struct ukui_surface *surface, bool need_commit)
{
    if (surface->view) {
        if (!need_commit) {
            position_apply_immediately(surface);
        } else if (!surface->view->base.mapped) {
            view_do_move(surface->view, surface->x, surface->y);
        }
    } else if (surface->buffer) {
        struct wlr_xdg_popup *xdg_popup = wlr_xdg_popup_try_from_wlr_surface(surface->wlr_surface);
        if (xdg_popup) {
            int lx, ly;
            scene_node_coords(&surface->buffer->node, &lx, &ly);
            xdg_popup->pending.geometry.x = xdg_popup->current.geometry.x + surface->x - lx;
            xdg_popup->pending.geometry.y = xdg_popup->current.geometry.y + surface->y - ly;
        }
    }
}

static void handle_set_position(struct wl_client *client, struct wl_resource *resource, int32_t x,
                                int32_t y)
{
    struct ukui_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface) {
        return;
    }

    surface->x = x;
    surface->y = y;
    ukui_surface_apply_position(surface, false);
}

static enum kywc_view_role role_from_ukui_surface(struct ukui_surface *surface)
{
    switch (surface->role) {
    default:
        return KYWC_VIEW_ROLE_NORMAL;
    case UKUI_SURFACE_V1_ROLE_DESKTOP:
        return KYWC_VIEW_ROLE_DESKTOP;
    case UKUI_SURFACE_V1_ROLE_PANEL:
        return KYWC_VIEW_ROLE_PANEL;
    case UKUI_SURFACE_V1_ROLE_ONSCREENDISPLAY:
        return KYWC_VIEW_ROLE_ONSCREENDISPLAY;
    case UKUI_SURFACE_V1_ROLE_NOTIFICATION:
        return KYWC_VIEW_ROLE_NOTIFICATION;
    case UKUI_SURFACE_V1_ROLE_TOOLTIP:
        return KYWC_VIEW_ROLE_TOOLTIP;
    case UKUI_SURFACE_V1_ROLE_CRITICALNOTIFICATION:
        return KYWC_VIEW_ROLE_CRITICALNOTIFICATION;
    case UKUI_SURFACE_V1_ROLE_APPLETPOPUP:
        return KYWC_VIEW_ROLE_APPLETPOPUP;
    case UKUI_SURFACE_V1_ROLE_SCREENLOCK:
        return KYWC_VIEW_ROLE_SCREENLOCK;
    case UKUI_SURFACE_V1_ROLE_WATERMARK:
        return KYWC_VIEW_ROLE_WATERMARK;
    case UKUI_SURFACE_V1_ROLE_SYSTEMWINDOW:
        return KYWC_VIEW_ROLE_SYSTEMWINDOW;
    case UKUI_SURFACE_V1_ROLE_INPUTPANEL:
        return KYWC_VIEW_ROLE_INPUTPANEL;
    case UKUI_SURFACE_V1_ROLE_LOGOUT:
        return KYWC_VIEW_ROLE_LOGOUT;
    case UKUI_SURFACE_V1_ROLE_SCREENLOCKNOTIFICATION:
        return KYWC_VIEW_ROLE_SCREENLOCKNOTIFICATION;
    case UKUI_SURFACE_V1_ROLE_SWITCHER:
        return KYWC_VIEW_ROLE_SWITCHER;
    case UKUI_SURFACE_V1_ROLE_AUTHENTICATION:
        return KYWC_VIEW_ROLE_AUTHENTICATION;
    }
}

static void handle_set_role(struct wl_client *client, struct wl_resource *resource, uint32_t role)
{
    struct ukui_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface) {
        return;
    }

    if (surface->wlr_surface->mapped) {
        wl_resource_post_error(resource, UKUI_SURFACE_V1_ERROR_MAPPED,
                               "wl_surface object already mapped");
        return;
    }

    surface->role = role;
}

static void handle_set_skip_taskbar(struct wl_client *client, struct wl_resource *resource,
                                    uint32_t skip)
{
    struct ukui_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface) {
        return;
    }

    if (surface->wlr_surface->mapped) {
        wl_resource_post_error(resource, UKUI_SURFACE_V1_ERROR_MAPPED,
                               "wl_surface object already mapped");
        return;
    }

    surface->skip_taskbar = skip;
}

static void handle_set_skip_switcher(struct wl_client *client, struct wl_resource *resource,
                                     uint32_t skip)
{
    struct ukui_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface) {
        return;
    }

    if (surface->wlr_surface->mapped) {
        wl_resource_post_error(resource, UKUI_SURFACE_V1_ERROR_MAPPED,
                               "wl_surface object already mapped");
        return;
    }

    surface->skip_switcher = skip;
}

#define set_state(states, prop, state)                                                             \
    if (prop) {                                                                                    \
        states |= UKUI_SURFACE_V1_SURFACE_STATE_##state;                                           \
    } else {                                                                                       \
        states &= ~UKUI_SURFACE_V1_SURFACE_STATE_##state;                                          \
    }

static void surface_send_surface_state(struct ukui_surface *surface, uint32_t mask)
{
    if (wl_resource_get_version(surface->resource) < UKUI_SURFACE_V1_SURFACE_STATE_SINCE_VERSION) {
        return;
    }

    uint32_t states = surface->surface_state;
    if (mask & UKUI_SURFACE_V1_SURFACE_STATE_MINIMIZED) {
        set_state(surface->surface_state, surface->view->base.minimized, MINIMIZED);
    }
    if (mask & UKUI_SURFACE_V1_SURFACE_STATE_MAXIMIZED) {
        set_state(surface->surface_state, surface->view->base.maximized, MAXIMIZED);
    }
    if (mask & UKUI_SURFACE_V1_SURFACE_STATE_FULLSCREEN) {
        set_state(surface->surface_state, surface->view->base.fullscreen, FULLSCREEN);
    }
    if (mask & UKUI_SURFACE_V1_SURFACE_STATE_ACTIVE) {
        set_state(surface->surface_state, surface->view->base.activated, ACTIVE);
    }
    if (mask & UKUI_SURFACE_V1_SURFACE_STATE_ABOVE) {
        set_state(surface->surface_state, surface->view->base.kept_above, ABOVE);
    }
    if (mask & UKUI_SURFACE_V1_SURFACE_STATE_BELOW) {
        set_state(surface->surface_state, surface->view->base.kept_below, BELOW);
    }
    if (mask & UKUI_SURFACE_V1_SURFACE_STATE_TILED) {
        set_state(surface->surface_state, !!surface->view->base.tiled, TILED);
    }

    if (states == surface->surface_state) {
        return;
    }

    ukui_surface_v1_send_surface_state(surface->resource, surface->surface_state);
}

static void surface_handle_view_activate(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, view_activate);
    surface_send_surface_state(surface, UKUI_SURFACE_V1_SURFACE_STATE_ACTIVE);
}

static void surface_handle_view_minimize(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, view_minimize);
    surface_send_surface_state(surface, UKUI_SURFACE_V1_SURFACE_STATE_MINIMIZED);
}

static void surface_handle_view_maximize(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, view_maximize);
    surface_send_surface_state(surface, UKUI_SURFACE_V1_SURFACE_STATE_MAXIMIZED);
}

static void surface_handle_view_fullscreen(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, view_fullscreen);
    surface_send_surface_state(surface, UKUI_SURFACE_V1_SURFACE_STATE_FULLSCREEN);
}

static void surface_handle_view_above(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, view_above);
    surface_send_surface_state(surface, UKUI_SURFACE_V1_SURFACE_STATE_ABOVE);
}

static void surface_handle_view_below(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, view_below);
    surface_send_surface_state(surface, UKUI_SURFACE_V1_SURFACE_STATE_BELOW);
}

static void surface_handle_view_tile(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, view_tile);
    surface_send_surface_state(surface, UKUI_SURFACE_V1_SURFACE_STATE_TILED);
}

static void ukui_surface_set_state(struct ukui_surface *surface, enum surface_state_flag flag,
                                   bool state)
{
    uint32_t mask = 0;
    switch (flag) {
    case STATE_FLAG_MINIMIZABLE:
        surface->minimizable = state;
        mask |= KYWC_VIEW_MINIMIZABLE | KYWC_VIEW_MINIMIZE_BUTTON;
        break;
    case STATE_FLAG_MAXIMIZABLE:
        surface->maximizable = state;
        mask |= KYWC_VIEW_MAXIMIZABLE | KYWC_VIEW_MAXIMIZE_BUTTON;
        break;
    case STATE_FLAG_CLOSEABLE:
        surface->closeable = state;
        mask |= KYWC_VIEW_CLOSEABLE;
        break;
    case STATE_FLAG_FULLSCREENABLE:
        surface->fullscreenable = state;
        mask |= KYWC_VIEW_FULLSCREENABLE;
        break;
    case STATE_FLAG_MOVABLE:
        surface->movable = state;
        mask |= KYWC_VIEW_MOVABLE;
        if (!surface->movable && surface->view->base.tiled) {
            kywc_view_set_tiled(&surface->view->base, KYWC_TILE_NONE, NULL);
        }
        break;
    case STATE_FLAG_RESIZABLE:
        surface->resizable = state;
        mask |= KYWC_VIEW_RESIZABLE;
        if (!surface->resizable && surface->view->base.tiled) {
            kywc_view_set_tiled(&surface->view->base, KYWC_TILE_NONE, NULL);
        }
        break;
    case STATE_FLAG_FOCUSABLE:
        surface->focusable = state;
        mask |= KYWC_VIEW_FOCUSABLE;
        break;
    case STATE_FLAG_ACTIVATABLE:
        surface->activatable = state;
        mask |= KYWC_VIEW_ACTIVATABLE;
        break;
    case STATE_FLAG_KEEPABOVE:
        if (surface->view->current_proxy) {
            kywc_view_set_kept_above(&surface->view->base, state);
        }
        break;
    case STATE_FLAG_KEEPBELOW:
        if (surface->view->current_proxy) {
            kywc_view_set_kept_below(&surface->view->base, state);
        }
        break;
    case STATE_FLAG_LAST:
        break;
    }

    if (mask != 0) {
        view_update_capabilities(surface->view, mask);
    }
}

static void handle_set_state(struct wl_client *client, struct wl_resource *resource, uint32_t flags,
                             uint32_t state)
{
    struct ukui_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface) {
        return;
    }

    for (int i = 0; i < STATE_FLAG_LAST; i++) {
        if (((flags >> i) & 0x1) == 0) {
            continue;
        }
        if (surface->view) {
            ukui_surface_set_state(surface, i, (state >> i) & 0x1);
        } else {
            surface->flags |= 1 << i;
            if ((state >> i) & 0x1) {
                surface->state |= 1 << i;
            } else {
                surface->state &= ~(1 << i);
            }
        }
    }
}

static void panel_surface_change_layer(struct ukui_surface *surface)
{
    if (surface->view->base.role != KYWC_VIEW_ROLE_PANEL) {
        return;
    }
    // when ukui panel hide, will show a transparent window, it need above other window
    enum layer layer = surface->panel_auto_hide ? LAYER_ABOVE : LAYER_DOCK;
    struct view_layer *view_layer = view_manager_get_layer(layer, false);
    scene_node_reparent(&surface->view->tree->node, view_layer->tree);
}

static void ukui_surface_set_usable_area(struct ukui_surface *surface, bool enabled);

static void handle_set_panel_auto_hide(struct wl_client *client, struct wl_resource *resource,
                                       uint32_t hide)
{
    struct ukui_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface || surface->panel_auto_hide == hide) {
        return;
    }

    surface->panel_auto_hide = hide;

    if (surface->view) {
        ukui_surface_set_usable_area(surface, true);
        panel_surface_change_layer(surface);
    }
}

static void handle_open_under_cursor(struct wl_client *client, struct wl_resource *resource,
                                     int32_t x, int32_t y)
{
    struct ukui_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface) {
        return;
    }

    if (surface->wlr_surface->mapped) {
        wl_resource_post_error(resource, UKUI_SURFACE_V1_ERROR_MAPPED,
                               "wl_surface object already mapped");
        return;
    }

    surface->offset_x = x;
    surface->offset_y = y;
}

static struct ukui_keyboard_grab *get_ukui_keyboard_grab_from_seat(struct ukui_surface *surface,
                                                                   struct seat *seat)
{
    struct ukui_keyboard_grab *grab;
    wl_list_for_each(grab, &surface->ukui_keyboard_grabs, link) {
        if (grab->keyboard_grab.seat == seat) {
            return grab;
        }
    }
    return NULL;
}

static bool keyboard_grab_key(struct seat_keyboard_grab *keyboard_grab, struct keyboard *keyboard,
                              uint32_t time, uint32_t key, bool pressed, uint32_t modifiers)
{
    struct ukui_surface *surface = keyboard_grab->data;
    struct wlr_keyboard *wlr_keyboard = keyboard->wlr_keyboard;
    /* don't use notify functions to skip the wlr_seat grab */
    wlr_seat_set_keyboard(keyboard_grab->seat->wlr_seat, keyboard->wlr_keyboard);
    wlr_seat_keyboard_enter(keyboard_grab->seat->wlr_seat, surface->wlr_surface,
                            wlr_keyboard->keycodes, wlr_keyboard->num_keycodes,
                            &wlr_keyboard->modifiers);
    wlr_seat_keyboard_send_key(keyboard_grab->seat->wlr_seat, time, key, pressed);
    return true;
}

static void keyboard_grab_cancel(struct seat_keyboard_grab *keyboard_grab)
{
    struct ukui_surface *surface = keyboard_grab->data;
    struct ukui_keyboard_grab *grab =
        get_ukui_keyboard_grab_from_seat(surface, keyboard_grab->seat);
    wl_list_remove(&grab->link);
    free(grab);
}

static const struct seat_keyboard_grab_interface keyboard_grab_impl = {
    .key = keyboard_grab_key,
    .cancel = keyboard_grab_cancel,
};

static struct ukui_keyboard_grab *ukui_surface_create_keyboard_grab(struct ukui_surface *surface,
                                                                    struct seat *seat)
{
    struct ukui_keyboard_grab *grab = calloc(1, sizeof(*grab));
    if (!grab) {
        return NULL;
    }

    grab->keyboard_grab.data = surface;
    grab->keyboard_grab.interface = &keyboard_grab_impl;
    wl_list_insert(&surface->ukui_keyboard_grabs, &grab->link);

    wlr_seat_keyboard_end_grab(seat->wlr_seat);
    wlr_seat_keyboard_clear_focus(seat->wlr_seat);
    seat_start_keyboard_grab(seat, &grab->keyboard_grab);

    return grab;
}

static void ukui_surface_end_keyboard_grab(struct ukui_keyboard_grab *ukui_keyboard_grab)
{
    struct ukui_surface *surface = ukui_keyboard_grab->keyboard_grab.data;
    if (surface->view && surface->view->base.role == KYWC_VIEW_ROLE_SWITCHER) {
        view_manager_set_switcher_shown(false);
    }

    seat_end_keyboard_grab(ukui_keyboard_grab->keyboard_grab.seat,
                           &ukui_keyboard_grab->keyboard_grab);
    wl_list_remove(&ukui_keyboard_grab->link);
    free(ukui_keyboard_grab);
}

struct ukui_surface_grab_data {
    struct ukui_surface *surface;
    struct seat *seat; // NULL means all
};

static bool ukui_surface_start_keyboard_grab(struct seat *seat, int index, void *data)
{
    struct ukui_surface_grab_data *grab_data = data;
    /* skip seat that is not matched */
    if (grab_data->seat && grab_data->seat != seat) {
        return false;
    }

    struct ukui_surface *surface = grab_data->surface;
    if (!get_ukui_keyboard_grab_from_seat(surface, seat)) {
        ukui_surface_create_keyboard_grab(surface, seat);
    }
    /* return true if seat is specified */
    return grab_data->seat;
}

static void handle_grab_keyboard(struct wl_client *client, struct wl_resource *resource,
                                 struct wl_resource *wl_seat)
{
    struct ukui_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface) {
        return;
    }

    struct seat *seat = NULL;
    if (wl_seat) {
        seat = seat_from_resource(wl_seat);
        if (!seat) {
            return;
        }
    }

    if (surface->view && surface->view->base.role == KYWC_VIEW_ROLE_SWITCHER) {
        view_manager_set_switcher_shown(true);
    }

    struct ukui_surface_grab_data grab_data = { surface, seat };
    input_manager_for_each_seat(ukui_surface_start_keyboard_grab, &grab_data);
}

static void handle_set_icon(struct wl_client *client, struct wl_resource *resource,
                            const char *icon_name)
{
    struct ukui_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface) {
        return;
    }

    if (surface->wlr_surface->mapped) {
        wl_resource_post_error(resource, UKUI_SURFACE_V1_ERROR_MAPPED,
                               "wl_surface object already mapped");
        return;
    }

    free(surface->icon_name);
    surface->icon_name = icon_name ? strdup(icon_name) : NULL;
}

static void handle_activate(struct wl_client *client, struct wl_resource *resource)
{
    struct ukui_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface) {
        return;
    }

    if (surface->view) {
        /* activation request */
        kywc_view_activate(&surface->view->base);
        view_set_focus(surface->view, input_manager_get_default_seat());
    }
}

static void handle_set_startup_geometry(struct wl_client *client, struct wl_resource *resource,
                                        int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    struct ukui_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface) {
        return;
    }

    if (surface->wlr_surface->mapped) {
        wl_resource_post_error(resource, UKUI_SURFACE_V1_ERROR_MAPPED,
                               "wl_surface object already mapped");
        return;
    }

    surface->startup = (struct kywc_box){ x, y, width, height };
}

static void handle_set_restore_geometry(struct wl_client *client, struct wl_resource *resource,
                                        int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    struct ukui_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface) {
        return;
    }

    if (surface->view) {
        surface->view->restore_geometry = (struct kywc_box){ x, y, width, height };
        return;
    }

    surface->restore = (struct kywc_box){ x, y, width, height };
}

static void handle_show_tile_flyout(struct wl_client *client, struct wl_resource *resource,
                                    struct wl_resource *wl_seat, int32_t x, int32_t y,
                                    uint32_t width, uint32_t height)
{
    struct ukui_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface || !surface->wlr_surface->mapped) {
        return;
    }

    struct view *view = view_try_from_wlr_surface(surface->wlr_surface);
    if (!view) {
        kywc_log(KYWC_DEBUG, "Surface is not a view");
        return;
    }

    struct seat *seat = wl_seat ? seat_from_resource(wl_seat) : input_manager_get_default_seat();
    if (!seat) {
        return;
    }

    struct kywc_box box = { x, y, width, height };
    if (!view_show_tile_flyout(view, seat, &box)) {
        kywc_log(KYWC_ERROR, "Surface show tile flyout failed");
    }
}

static void handle_set_pending_position(struct wl_client *client, struct wl_resource *resource,
                                        int32_t x, int32_t y)
{
    struct ukui_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface) {
        return;
    }

    surface->x = x;
    surface->y = y;
    ukui_surface_apply_position(surface, true);
}

static const struct ukui_surface_v1_interface ukui_surface_impl = {
    .destroy = handle_destroy,
    .set_position = handle_set_position,
    .set_skip_taskbar = handle_set_skip_taskbar,
    .set_skip_switcher = handle_set_skip_switcher,
    .set_role = handle_set_role,
    .set_state = handle_set_state,
    .set_panel_auto_hide = handle_set_panel_auto_hide,
    .open_under_cursor = handle_open_under_cursor,
    .grab_keyboard = handle_grab_keyboard,
    .set_icon = handle_set_icon,
    .activate = handle_activate,
    .set_startup_geometry = handle_set_startup_geometry,
    .show_tile_flyout = handle_show_tile_flyout,
    .set_restore_geometry = handle_set_restore_geometry,
    .set_pending_position = handle_set_pending_position,
};

static void ukui_surface_set_usable_area(struct ukui_surface *surface, bool enabled)
{
    bool has_area = enabled && surface->view->base.mapped &&
                    surface->view->base.role == KYWC_VIEW_ROLE_PANEL && !surface->panel_auto_hide;
    view_set_exclusive(surface->view, has_area);
}

static void surface_handle_view_update_capabilities(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, view_update_capabilities);
    struct view_update_capabilities_event *event = data;

    if (event->mask & KYWC_VIEW_MINIMIZABLE) {
        if (!surface->minimizable) {
            event->state &= ~KYWC_VIEW_MINIMIZABLE;
        }
    }
    if (event->mask & KYWC_VIEW_MAXIMIZABLE) {
        if (!surface->maximizable) {
            event->state &= ~KYWC_VIEW_MAXIMIZABLE;
        }
    }
    if (event->mask & KYWC_VIEW_CLOSEABLE) {
        if (!surface->closeable) {
            event->state &= ~KYWC_VIEW_CLOSEABLE;
        }
    }
    if (event->mask & KYWC_VIEW_FULLSCREENABLE) {
        if (!surface->fullscreenable) {
            event->state &= ~KYWC_VIEW_FULLSCREENABLE;
        }
    }
    if (event->mask & KYWC_VIEW_MOVABLE) {
        if (!surface->movable) {
            event->state &= ~KYWC_VIEW_MOVABLE;
        }
    }
    if (event->mask & KYWC_VIEW_RESIZABLE) {
        if (!surface->resizable) {
            event->state &= ~KYWC_VIEW_RESIZABLE;
        }
    }
    if (event->mask & KYWC_VIEW_FOCUSABLE) {
        if (!surface->focusable) {
            event->state &= ~KYWC_VIEW_FOCUSABLE;
        }
    }
    if (event->mask & KYWC_VIEW_ACTIVATABLE) {
        if (!surface->activatable) {
            event->state &= ~KYWC_VIEW_ACTIVATABLE;
        }
    }
    if (event->mask & KYWC_VIEW_MINIMIZE_BUTTON) {
        if (!surface->minimizable) {
            event->state &= ~KYWC_VIEW_MINIMIZE_BUTTON;
        }
    }
    if (event->mask & KYWC_VIEW_MAXIMIZE_BUTTON) {
        if (!surface->maximizable) {
            event->state &= ~KYWC_VIEW_MAXIMIZE_BUTTON;
        }
    }
}

static void surface_handle_view_position(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, view_position);
    ukui_surface_v1_send_position(surface->resource, surface->view->base.geometry.x,
                                  surface->view->base.geometry.y);
}

static void surface_handle_view_destroy(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, view_destroy);

    wl_list_remove(&surface->view_destroy.link);
    wl_list_remove(&surface->view_map.link);
    wl_list_remove(&surface->view_unmap.link);
    wl_list_remove(&surface->view_update_capabilities.link);
    wl_list_remove(&surface->view_activate.link);
    wl_list_remove(&surface->view_minimize.link);
    wl_list_remove(&surface->view_maximize.link);
    wl_list_remove(&surface->view_fullscreen.link);
    wl_list_remove(&surface->view_above.link);
    wl_list_remove(&surface->view_below.link);
    wl_list_remove(&surface->view_tile.link);

    surface->view = NULL;
}

static void surface_handle_view_map(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, view_map);
    ukui_surface_set_usable_area(surface, true);

    surface->view_position.notify = surface_handle_view_position;
    wl_signal_add(&surface->view->base.events.position, &surface->view_position);

    ukui_surface_v1_send_position(surface->resource, surface->view->base.geometry.x,
                                  surface->view->base.geometry.y);
}

static void surface_handle_view_unmap(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, view_unmap);
    wl_list_remove(&surface->view_position.link);
    ukui_surface_set_usable_area(surface, false);

    struct ukui_keyboard_grab *grab, *tmp;
    wl_list_for_each_safe(grab, tmp, &surface->ukui_keyboard_grabs, link) {
        ukui_surface_end_keyboard_grab(grab);
    }
}

static void surface_handle_map(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, surface_map);

    /* useless once we connect surface and view */
    wl_list_remove(&surface->surface_map.link);
    wl_list_init(&surface->surface_map.link);

    /* get view from surface */
    surface->view = view_try_from_wlr_surface(surface->wlr_surface);
    if (!surface->view) {
        kywc_log(KYWC_DEBUG, "Surface is not a toplevel");
        surface->buffer = scene_buffer_try_from_surface(surface->wlr_surface);
        return;
    }
    surface->view_update_capabilities.notify = surface_handle_view_update_capabilities;
    view_add_update_capabilities_listener(surface->view, &surface->view_update_capabilities);
    struct kywc_view *kywc_view = &surface->view->base;
    surface->view_activate.notify = surface_handle_view_activate;
    wl_signal_add(&kywc_view->events.activate, &surface->view_activate);
    surface->view_minimize.notify = surface_handle_view_minimize;
    wl_signal_add(&kywc_view->events.minimize, &surface->view_minimize);
    surface->view_maximize.notify = surface_handle_view_maximize;
    wl_signal_add(&kywc_view->events.maximize, &surface->view_maximize);
    surface->view_fullscreen.notify = surface_handle_view_fullscreen;
    wl_signal_add(&kywc_view->events.fullscreen, &surface->view_fullscreen);
    surface->view_above.notify = surface_handle_view_above;
    wl_signal_add(&kywc_view->events.above, &surface->view_above);
    surface->view_below.notify = surface_handle_view_below;
    wl_signal_add(&kywc_view->events.below, &surface->view_below);
    surface->view_tile.notify = surface_handle_view_tile;
    wl_signal_add(&kywc_view->events.tile, &surface->view_tile);

    if (surface->skip_taskbar != -1) {
        kywc_view_set_skip_taskbar(&surface->view->base, surface->skip_taskbar);
    }
    if (surface->skip_switcher != -1) {
        kywc_view_set_skip_switcher(&surface->view->base, surface->skip_switcher);
    }

    if (kywc_box_not_empty(&surface->startup)) {
        surface->view->startup_geometry = surface->startup;
        surface->startup = (struct kywc_box){ 0 };
    }

    if (kywc_box_not_empty(&surface->restore)) {
        surface->view->restore_geometry = surface->restore;
        surface->restore = (struct kywc_box){ 0 };
    }

    if (surface->role != UINT32_MAX) {
        view_set_role(surface->view, role_from_ukui_surface(surface));
    }
    panel_surface_change_layer(surface);

    if (surface->flags != 0) {
        for (int i = 0; i < STATE_FLAG_LAST; i++) {
            if ((surface->flags >> i) & 0x1) {
                ukui_surface_set_state(surface, i, (surface->state >> i) & 0x1);
            }
        }
        surface->flags = 0;
    }

    if (surface->icon_name) {
        view_set_icon_name(surface->view, surface->icon_name);
    }

    if (surface->offset_x != INT32_MAX || surface->offset_y != INT32_MAX) {
        struct seat *seat = surface->view->base.focused_seat;
        int lx = seat->cursor->lx + surface->offset_x;
        int ly = seat->cursor->ly + surface->offset_y;

        struct output *output = input_current_output(seat);
        if (output) {
            struct kywc_box *geo = &output->geometry;
            int width = surface->wlr_surface->current.width;
            int height = surface->wlr_surface->current.height;
            int min_x = geo->x, max_x = geo->x + geo->width - width;
            int min_y = geo->y, max_y = geo->y + geo->height - height;
            lx = CLAMP(lx, min_x, max_x);
            ly = CLAMP(ly, min_y, max_y);
        }

        surface->view->base.has_initial_position = true;
        view_do_move(surface->view, lx, ly);
        surface->offset_x = surface->offset_y = INT32_MAX;
    }

    /* apply set_position called before map */
    if (surface->x != INT32_MAX || surface->y != INT32_MAX) {
        surface->view->base.has_initial_position = true;
        ukui_surface_apply_position(surface, false);
        surface->x = surface->y = INT32_MAX;
    }

    surface->view_map.notify = surface_handle_view_map;
    wl_signal_add(&surface->view->base.events.map, &surface->view_map);
    surface->view_unmap.notify = surface_handle_view_unmap;
    wl_signal_add(&surface->view->base.events.unmap, &surface->view_unmap);
    surface->view_destroy.notify = surface_handle_view_destroy;
    wl_signal_add(&surface->view->base.events.destroy, &surface->view_destroy);
}

static void surface_handle_commit(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, surface_commit);
    if (!surface->view || !surface->view->base.mapped) {
        return;
    }

    if (surface->x != INT32_MAX || surface->y != INT32_MAX) {
        position_apply_immediately(surface);
    }
}

static void surface_handle_destroy(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, surface_destroy);

    struct ukui_keyboard_grab *grab, *tmp;
    wl_list_for_each_safe(grab, tmp, &surface->ukui_keyboard_grabs, link) {
        ukui_surface_end_keyboard_grab(grab);
    }

    wl_list_remove(&surface->surface_map.link);
    wl_list_remove(&surface->surface_commit.link);
    wl_list_remove(&surface->surface_destroy.link);

    surface->wlr_surface = NULL;
}

static void ukui_surface_handle_resource_destroy(struct wl_resource *resource)
{
    struct ukui_surface *surface = wl_resource_get_user_data(resource);

    if (surface->wlr_surface) {
        surface_handle_destroy(&surface->surface_destroy, NULL);
    }

    if (surface->view) {
        if (surface->view->base.mapped) {
            surface_handle_view_unmap(&surface->view_unmap, NULL);
        }
        surface_handle_view_destroy(&surface->view_destroy, NULL);
    }

    wl_list_remove(&surface->link);
    free(surface->icon_name);
    free(surface);
}

static struct ukui_surface *ukui_surface_from_wlr_surface(struct ukui_shell *shell,
                                                          struct wlr_surface *wlr_surface)
{
    struct ukui_surface *ukui_surface;
    wl_list_for_each(ukui_surface, &shell->ukui_surfaces, link) {
        if (ukui_surface->wlr_surface == wlr_surface) {
            return ukui_surface;
        }
    }
    return NULL;
}

static void handle_create_surface(struct wl_client *client, struct wl_resource *shell_resource,
                                  uint32_t id, struct wl_resource *surface_resource)
{
    struct wlr_surface *wlr_surface = wlr_surface_from_resource(surface_resource);
    struct ukui_shell *shell = wl_resource_get_user_data(shell_resource);

    if (wlr_surface->mapped) {
        wl_resource_post_error(shell_resource, UKUI_SHELL_V1_ERROR_MAPPED,
                               "surface has already mapped");
        return;
    }

    struct ukui_surface *surface = ukui_surface_from_wlr_surface(shell, wlr_surface);
    if (surface) {
        wl_resource_post_error(shell_resource, UKUI_SHELL_V1_ERROR_SURFACE_EXISTS,
                               "surface has wl_surface object associated");
        return;
    }

    /* create new ukui surface */
    surface = calloc(1, sizeof(*surface));
    if (!surface) {
        wl_client_post_no_memory(client);
        return;
    }

    int version = wl_resource_get_version(shell_resource);
    struct wl_resource *resource =
        wl_resource_create(client, &ukui_surface_v1_interface, version, id);
    if (!resource) {
        free(surface);
        wl_client_post_no_memory(client);
        return;
    }

    surface->resource = resource;
    wl_resource_set_implementation(resource, &ukui_surface_impl, surface,
                                   ukui_surface_handle_resource_destroy);

    surface->x = surface->y = INT32_MAX;
    surface->offset_x = surface->offset_y = INT32_MAX;
    surface->role = UINT32_MAX;
    surface->skip_taskbar = surface->skip_switcher = -1;
    surface->minimizable = surface->maximizable = surface->fullscreenable = surface->closeable =
        surface->movable = surface->resizable = surface->focusable = surface->activatable = true;
    wl_list_init(&surface->ukui_keyboard_grabs);

    surface->wlr_surface = wlr_surface;
    surface->surface_map.notify = surface_handle_map;
    wl_signal_add_next(&wlr_surface->events.map, &surface->surface_map);
    surface->surface_commit.notify = surface_handle_commit;
    wl_signal_add_next(&wlr_surface->events.commit, &surface->surface_commit);
    surface->surface_destroy.notify = surface_handle_destroy;
    wl_signal_add(&wlr_surface->events.destroy, &surface->surface_destroy);

    wl_list_insert(&shell->ukui_surfaces, &surface->link);
}

static void decoration_handle_surface_map(struct wl_listener *listener, void *data)
{
    struct ukui_decoration *decoration = wl_container_of(listener, decoration, surface_map);
    struct view *view = view_try_from_wlr_surface(decoration->wlr_surface);
    if (!view) {
        return;
    }

    enum kywc_ssd ssd = view->base.ssd;
    if (ssd == KYWC_SSD_NONE || decoration->no_titlebar == -1) {
        return;
    }

    if (decoration->no_titlebar) {
        ssd &= ~KYWC_SSD_TITLE;
    } else {
        ssd |= KYWC_SSD_TITLE;
    }
    view_set_decoration(view, ssd);
}

static void decoration_destroy(struct ukui_decoration *decoration)
{
    if (!decoration) {
        return;
    }

    wlr_addon_finish(&decoration->addon);
    wl_list_remove(&decoration->surface_map.link);

    wl_resource_set_user_data(decoration->resource, NULL);
    free(decoration);
}

static void ukui_decoration_handle_resource_destroy(struct wl_resource *resource)
{
    struct ukui_decoration *decoration = wl_resource_get_user_data(resource);
    decoration_destroy(decoration);
}

static void decoration_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static void handle_set_no_titlebar(struct wl_client *client, struct wl_resource *resource,
                                   uint32_t no_titlebar)
{
    struct ukui_decoration *decoration = wl_resource_get_user_data(resource);
    if (!decoration) {
        return;
    }

    if (decoration->wlr_surface->mapped) {
        wl_resource_post_error(resource, UKUI_DECORATION_V1_ERROR_MAPPED,
                               "wl_surface object already mapped");
        return;
    }

    decoration->no_titlebar = no_titlebar;
}

static void handle_set_decoration_components(struct wl_client *client, struct wl_resource *resource,
                                             uint32_t flags)
{
    struct ukui_decoration *decoration = wl_resource_get_user_data(resource);
    if (!decoration) {
        return;
    }

    if (decoration->wlr_surface->mapped) {
        wl_resource_post_error(resource, UKUI_DECORATION_V1_ERROR_MAPPED,
                               "wl_surface object already mapped");
        return;
    }

    decoration->flags = flags;
}

static const struct ukui_decoration_v1_interface ukui_decoration_impl = {
    .set_no_titlebar = handle_set_no_titlebar,
    .set_decoration_components = handle_set_decoration_components,
    .destroy = decoration_handle_destroy,
};

static void surface_decoration_addon_destroy(struct wlr_addon *addon)
{
    struct ukui_decoration *decoration = wl_container_of(addon, decoration, addon);
    decoration_destroy(decoration);
}

static const struct wlr_addon_interface surface_decoration_impl = {
    .name = "surface_decoration",
    .destroy = surface_decoration_addon_destroy,
};

static struct ukui_decoration *ukui_decoration_from_wlr_surface(struct wlr_surface *surface)
{
    struct ukui_decoration *decoration = NULL;
    struct wlr_addon *decoration_addon =
        wlr_addon_find(&surface->addons, surface, &surface_decoration_impl);
    if (decoration_addon) {
        decoration = wl_container_of(decoration_addon, decoration, addon);
    }
    return decoration;
}

static void handle_create_decoration(struct wl_client *client, struct wl_resource *shell_resource,
                                     uint32_t id, struct wl_resource *surface_resource)
{
    struct wlr_surface *wlr_surface = wlr_surface_from_resource(surface_resource);

    if (wlr_surface->mapped) {
        wl_resource_post_error(shell_resource, UKUI_SHELL_V1_ERROR_MAPPED,
                               "surface has already mapped");
        return;
    }

    struct ukui_decoration *decoration = ukui_decoration_from_wlr_surface(wlr_surface);
    if (decoration) {
        wl_resource_post_error(shell_resource, UKUI_SHELL_V1_ERROR_DECORATION_EXISTS,
                               "surface has wl_surface object associated");
        return;
    }

    /* create new ukui surface */
    decoration = calloc(1, sizeof(*decoration));
    if (!decoration) {
        wl_client_post_no_memory(client);
        return;
    }

    int version = wl_resource_get_version(shell_resource);
    struct wl_resource *resource =
        wl_resource_create(client, &ukui_decoration_v1_interface, version, id);
    if (!resource) {
        free(decoration);
        wl_client_post_no_memory(client);
        return;
    }

    decoration->resource = resource;
    wl_resource_set_implementation(resource, &ukui_decoration_impl, decoration,
                                   ukui_decoration_handle_resource_destroy);

    decoration->no_titlebar = -1;
    decoration->wlr_surface = wlr_surface;
    decoration->surface_map.notify = decoration_handle_surface_map;
    wl_signal_add(&wlr_surface->events.map, &decoration->surface_map);
    /* use addon destroy for surface destroy */
    wlr_addon_init(&decoration->addon, &wlr_surface->addons, wlr_surface, &surface_decoration_impl);
}

static bool send_seat_output(struct seat *seat, int index, void *data)
{
    struct wl_resource *resource = data;
    struct kywc_output *kywc_output = kywc_output_at_point(seat->cursor->lx, seat->cursor->ly);
    ukui_shell_v1_send_current_output(resource, kywc_output->name, seat->name);
    return false;
}

static void handle_get_current_output(struct wl_client *client, struct wl_resource *shell_resource)
{
    input_manager_for_each_seat(send_seat_output, shell_resource);
    ukui_shell_v1_send_done(shell_resource);
}

static const struct ukui_shell_v1_interface ukui_shell_impl = {
    .create_surface = handle_create_surface,
    .create_decoration = handle_create_decoration,
    .get_current_output = handle_get_current_output,
};

static void ukui_shell_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
    struct wl_resource *resource =
        wl_resource_create(client, &ukui_shell_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    struct ukui_shell *shell = data;
    wl_resource_set_implementation(resource, &ukui_shell_impl, shell, NULL);

    input_manager_for_each_seat(send_seat_output, resource);
    ukui_shell_v1_send_done(resource);
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    struct ukui_shell *shell = wl_container_of(listener, shell, display_destroy);
    wl_list_remove(&shell->display_destroy.link);
    wl_global_destroy(shell->global);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    struct ukui_shell *shell = wl_container_of(listener, shell, server_destroy);
    wl_list_remove(&shell->server_destroy.link);
    free(shell);
}

bool ukui_shell_create(struct server *server)
{
    struct ukui_shell *shell = calloc(1, sizeof(*shell));
    if (!shell) {
        return false;
    }

    shell->global = wl_global_create(server->display, &ukui_shell_v1_interface,
                                     UKUI_SHELL_V1_VERSION, shell, ukui_shell_bind);
    if (!shell->global) {
        kywc_log(KYWC_WARN, "Ukui-shell-v1 create failed");
        free(shell);
        return false;
    }

    wl_list_init(&shell->ukui_surfaces);

    shell->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &shell->server_destroy);
    shell->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(server->display, &shell->display_destroy);

    return true;
}

uint32_t ukui_shell_get_surface_decoration_flags(struct wlr_surface *surface)
{
    struct ukui_decoration *decoration = ukui_decoration_from_wlr_surface(surface);
    return decoration ? decoration->flags : 0;
}
