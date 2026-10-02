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
#include "view_p.h"

#define UKUI_SHELL_V1_VERSION 1

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
    struct wl_listener surface_destroy;

    /* for popup position */
    struct ky_scene_buffer *buffer;

    /* get in map listener */
    struct view *view;
    struct wl_listener view_map;
    struct wl_listener view_unmap;
    struct wl_listener view_minimize;
    struct wl_listener view_size;
    struct wl_listener view_position;
    struct wl_listener view_output;
    struct wl_listener view_destroy;

    struct wl_listener output_update_usable_area;

    struct {
        struct kywc_box geometry;
        bool use_startup_geometry;
    } startup;

    int x, y;
    enum ukui_surface_v1_role role;
    bool role_changed;
    bool skip_taskbar, skip_switcher;
    bool open_under_cursor;

    bool panel_auto_hide;
    bool hide_changed;

    uint32_t surface_state;
    uint32_t surface_flags;
    bool state_changed;

    struct seat_keyboard_grab keyboard_grab;

    struct wl_list ukui_keyboard_grabs;
    bool grab_all_keyboard;

    char *icon_name;
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

    bool pending_no_titlebar;
    bool no_titlebar;
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

static void ukui_surface_apply_position(struct ukui_surface *surface)
{
    if (surface->view) {
        view_do_move(surface->view, surface->x, surface->y);
        if (surface->view->base.mapped) {
            input_rebase_all_cursor();
        }
    } else if (surface->buffer) {
        struct wlr_xdg_popup *xdg_popup = wlr_xdg_popup_try_from_wlr_surface(surface->wlr_surface);
        if (xdg_popup) {
            int lx, ly;
            ky_scene_node_coords(&surface->buffer->node, &lx, &ly);
            xdg_popup->pending.geometry.x = xdg_popup->current.geometry.x + surface->x - lx;
            xdg_popup->pending.geometry.y = xdg_popup->current.geometry.x + surface->y - ly;
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
    if (surface->x == x && surface->y == y) {
        return;
    }

    surface->x = x;
    surface->y = y;
    ukui_surface_apply_position(surface);
}

static void ukui_surface_apply_role(struct ukui_surface *surface)
{
    struct kywc_view *kywc_view = &surface->view->base;

    switch (surface->role) {
    case UKUI_SURFACE_V1_ROLE_NORMAL:
        kywc_view->role = KYWC_VIEW_ROLE_NORMAL;
        break;
    case UKUI_SURFACE_V1_ROLE_DESKTOP:
        kywc_view->role = KYWC_VIEW_ROLE_DESKTOP;
        break;
    case UKUI_SURFACE_V1_ROLE_PANEL:
        kywc_view->role = KYWC_VIEW_ROLE_PANEL;
        break;
    case UKUI_SURFACE_V1_ROLE_ONSCREENDISPLAY:
        kywc_view->role = KYWC_VIEW_ROLE_ONSCREENDISPLAY;
        break;
    case UKUI_SURFACE_V1_ROLE_NOTIFICATION:
        kywc_view->role = KYWC_VIEW_ROLE_NOTIFICATION;
        break;
    case UKUI_SURFACE_V1_ROLE_TOOLTIP:
        kywc_view->role = KYWC_VIEW_ROLE_TOOLTIP;
        break;
    case UKUI_SURFACE_V1_ROLE_CRITICALNOTIFICATION:
        kywc_view->role = KYWC_VIEW_ROLE_CRITICALNOTIFICATION;
        break;
    case UKUI_SURFACE_V1_ROLE_APPLETPOPUP:
        kywc_view->role = KYWC_VIEW_ROLE_APPLETPOPUP;
        break;
    case UKUI_SURFACE_V1_ROLE_SCREENLOCK:
        kywc_view->role = KYWC_VIEW_ROLE_SCREENLOCK;
        break;
    case UKUI_SURFACE_V1_ROLE_WATERMARK:
        kywc_view->role = KYWC_VIEW_ROLE_WATERMARK;
        break;
    case UKUI_SURFACE_V1_ROLE_SYSTEMWINDOW:
        kywc_view->role = KYWC_VIEW_ROLE_SYSTEMWINDOW;
        break;
    case UKUI_SURFACE_V1_ROLE_INPUTPANEL:
        kywc_view->role = KYWC_VIEW_ROLE_INPUTPANEL;
        break;
    case UKUI_SURFACE_V1_ROLE_LOGOUT:
        kywc_view->role = KYWC_VIEW_ROLE_LOGOUT;
        break;
    case UKUI_SURFACE_V1_ROLE_SCREENLOCKNOTIFICATION:
        kywc_view->role = KYWC_VIEW_ROLE_SCREENLOCKNOTIFICATION;
        break;
    case UKUI_SURFACE_V1_ROLE_SWITCHER:
        kywc_view->role = KYWC_VIEW_ROLE_SWITCHER;
        break;
    case UKUI_SURFACE_V1_ROLE_AUTHENTICATION:
        kywc_view->role = KYWC_VIEW_ROLE_AUTHENTICATION;
        break;
    }

    view_apply_role(view_from_kywc_view(kywc_view));
    surface->role_changed = false;
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

    if (surface->role != role) {
        surface->role = role;
        surface->role_changed = true;
    }
}

static void handle_set_skip_taskbar(struct wl_client *client, struct wl_resource *resource,
                                    uint32_t skip)
{
    struct ukui_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface || surface->skip_taskbar == skip) {
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

static void ukui_surface_set_state(struct ukui_surface *surface, enum surface_state_flag flag,
                                   bool state)
{
    switch (flag) {
    case STATE_FLAG_MINIMIZABLE:
        surface->view->base.minimizable = state;
        break;
    case STATE_FLAG_MAXIMIZABLE:
        surface->view->base.maximizable = state;
        break;
    case STATE_FLAG_CLOSEABLE:
        surface->view->base.closeable = state;
        break;
    case STATE_FLAG_FULLSCREENABLE:
        surface->view->base.fullscreenable = state;
        break;
    case STATE_FLAG_MOVABLE:
        surface->view->base.movable = state;
        break;
    case STATE_FLAG_RESIZABLE:
        surface->view->base.resizable = state;
        break;
    case STATE_FLAG_FOCUSABLE:
        surface->view->base.focusable = state;
        break;
    case STATE_FLAG_ACTIVATABLE:
        surface->view->base.activatable = state;
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
}

static void handle_set_state(struct wl_client *client, struct wl_resource *resource, uint32_t flags,
                             uint32_t state)
{
    struct ukui_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface) {
        return;
    }

    if (surface->view) {
        for (int i = 0; i < STATE_FLAG_LAST; i++) {
            if ((flags >> i) & 0x1) {
                ukui_surface_set_state(surface, i, (state >> i) & 0x1);
            }
        }

        /* need to redraw ssd when minimizable/maximizable/closeable changed */
        if (((flags >> STATE_FLAG_MINIMIZABLE) & 0x1) ||
            ((flags >> STATE_FLAG_MAXIMIZABLE) & 0x1) || ((flags >> STATE_FLAG_CLOSEABLE) & 0x1)) {
            view_set_decoration(surface->view, surface->view->base.ssd);
        }

        return;
    }

    surface->surface_flags = flags;
    surface->surface_state = state;
    surface->state_changed = true;
}

static void panel_change_layer(struct view *view, bool hide)
{
    // For ukui panel
    // when ukui panel hide, will show a transparent window, it need above other window

    enum layer layer = hide ? LAYER_ABOVE : LAYER_DOCK;
    struct view_layer *view_layer = view_manager_get_layer(layer, false);
    ky_scene_node_reparent(&view->tree->node, view_layer->tree);
}

static void ukui_surface_set_usable_area(struct ukui_surface *surface, bool enabled);

static void handle_set_panel_auto_hide(struct wl_client *client, struct wl_resource *resource,
                                       uint32_t hide)
{
    struct ukui_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface || surface->role != UKUI_SURFACE_V1_ROLE_PANEL ||
        surface->panel_auto_hide == hide) {
        return;
    }

    if (surface->view) {
        ukui_surface_set_usable_area(surface, true);
        panel_change_layer(surface->view, hide);
        return;
    }

    surface->panel_auto_hide = hide;
    surface->hide_changed = true;
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

    surface->open_under_cursor = true;
}

static struct ukui_keyboard_grab *get_ukui_keyboard_grab_from_seat(struct ukui_surface *surface,
                                                                   struct seat *seat)
{
    struct ukui_keyboard_grab *grab, *tmp;
    wl_list_for_each_safe(grab, tmp, &surface->ukui_keyboard_grabs, link) {
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
    wlr_seat_set_keyboard(keyboard_grab->seat->wlr_seat, keyboard->wlr_keyboard);
    wlr_seat_keyboard_enter(keyboard_grab->seat->wlr_seat, surface->wlr_surface,
                            wlr_keyboard->keycodes, wlr_keyboard->num_keycodes,
                            &wlr_keyboard->modifiers);
    wlr_seat_keyboard_send_key(keyboard_grab->seat->wlr_seat, time, key, pressed);
    return true;
}

static void keyboard_grab_cancel(struct seat_keyboard_grab *keyboard_grab)
{
    // For seat destroy
    struct ukui_surface *surface = keyboard_grab->data;
    struct ukui_keyboard_grab *grab =
        get_ukui_keyboard_grab_from_seat(surface, keyboard_grab->seat);
    seat_end_keyboard_grab(keyboard_grab->seat, keyboard_grab);
    wl_list_remove(&grab->link);
    free(grab);
}

static const struct seat_keyboard_grab_interface keyboard_grab_impl = {
    .key = keyboard_grab_key,
    .cancel = keyboard_grab_cancel,
};

static bool end_remain_grab(struct seat *seat, int index, void *data)
{
    wlr_seat_keyboard_end_grab(seat->wlr_seat);
    wlr_seat_keyboard_clear_focus(seat->wlr_seat);
    return false;
}

static struct ukui_keyboard_grab *get_or_create_keyboard_grab(struct ukui_surface *surface,
                                                              struct seat *seat)
{
    struct ukui_keyboard_grab *grab = get_ukui_keyboard_grab_from_seat(surface, seat);
    if (grab) {
        return grab;
    }

    grab = calloc(1, sizeof(struct ukui_keyboard_grab));
    if (!grab) {
        return NULL;
    }

    grab->keyboard_grab.data = surface;
    grab->keyboard_grab.interface = &keyboard_grab_impl;
    seat_start_keyboard_grab(seat, &grab->keyboard_grab);
    wl_list_insert(&surface->ukui_keyboard_grabs, &grab->link);

    return grab;
}

static bool start_grab_keyboard_foreach(struct seat *seat, int index, void *data)
{
    struct ukui_surface *surface = data;
    get_or_create_keyboard_grab(surface, seat);
    return false;
}

static void end_ukui_grab_keyboard(struct ukui_keyboard_grab *ukui_keyboard_grab)
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

static void handle_grab_keyboard(struct wl_client *client, struct wl_resource *resource,
                                 struct wl_resource *wl_seat)
{
    struct ukui_surface *surface = wl_resource_get_user_data(resource);
    if (!surface->wlr_surface || surface->grab_all_keyboard) {
        return;
    }

    if (surface->view && surface->view->base.role == KYWC_VIEW_ROLE_SWITCHER) {
        view_manager_set_switcher_shown(true);
    }

    if (wl_seat) {
        struct seat *seat = seat_from_resource(wl_seat);
        if (seat) {
            end_remain_grab(seat, 0, NULL);
            get_or_create_keyboard_grab(surface, seat);
        }
    } else {
        surface->grab_all_keyboard = true;
        input_manager_for_each_seat(end_remain_grab, NULL);
        input_manager_for_each_seat(start_grab_keyboard_foreach, surface);
    }
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
        return;
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

    surface->startup.use_startup_geometry = true;
    surface->startup.geometry = (struct kywc_box){ x, y, width, height };
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
        kywc_log(KYWC_DEBUG, "surface is not a view");
        return;
    }

    struct seat *seat = wl_seat ? seat_from_resource(wl_seat) : input_manager_get_default_seat();
    if (!seat) {
        return;
    }

    struct kywc_box box = (struct kywc_box){ x, y, width, height };
    if (!view_show_tile_flyout(view, seat, &box)) {
        kywc_log(KYWC_ERROR, "surface show tile flyout failed");
    }
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
};

static void surface_handle_output_update_usable_area(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, output_update_usable_area);
    struct kywc_box *usable_area = data;

    struct kywc_box geo;
    kywc_output_effective_geometry(surface->view->output, &geo);

    struct kywc_box *view_geo = &surface->view->base.geometry;
    int mid_x = view_geo->x + view_geo->width / 2;
    int mid_y = view_geo->y + view_geo->height / 2;

    if (view_geo->width > view_geo->height) {
        if (mid_y - geo.y < geo.y + geo.height - mid_y) {
            // position is top
            int y = view_geo->y + view_geo->height;
            geo.height -= y - geo.y;
            geo.y = y;
        } else {
            // position is bottom
            geo.height = view_geo->y - geo.y;
        }
    } else {
        if (mid_x - geo.x < geo.x + geo.width - mid_x) {
            // position is left
            int x = view_geo->width + view_geo->x;
            geo.width -= x - geo.x;
            geo.x = x;
        } else {
            // position is right
            geo.width = view_geo->x - geo.x;
        }
    }

    /* intersect usable_area and geo */
    usable_area->x = geo.x > usable_area->x ? geo.x : usable_area->x;
    usable_area->y = geo.y > usable_area->y ? geo.y : usable_area->y;
    usable_area->width = geo.width < usable_area->width ? geo.width : usable_area->width;
    usable_area->height = geo.height < usable_area->height ? geo.height : usable_area->height;
}

static void ukui_surface_set_usable_area(struct ukui_surface *surface, bool enabled)
{
    bool had_area = !wl_list_empty(&surface->output_update_usable_area.link);
    bool has_area = enabled && surface->view->base.mapped && !surface->view->base.minimized &&
                    surface->role == UKUI_SURFACE_V1_ROLE_PANEL && !surface->panel_auto_hide;

    if (!has_area) {
        if (had_area) {
            wl_list_remove(&surface->output_update_usable_area.link);
            wl_list_init(&surface->output_update_usable_area.link);
            output_update_usable_area(surface->view->output);
        }
        return;
    }

    if (!had_area) {
        surface->output_update_usable_area.notify = surface_handle_output_update_usable_area;
        output_add_update_usable_area_listener(surface->view->output,
                                               &surface->output_update_usable_area, false);
    }

    output_update_usable_area(surface->view->output);
}

static void surface_handle_view_minimize(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, view_minimize);
    ukui_surface_set_usable_area(surface, true);
}

static void surface_handle_view_size(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, view_size);
    ukui_surface_set_usable_area(surface, true);
}

static void surface_handle_view_position(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, view_position);
    ukui_surface_set_usable_area(surface, true);

    ukui_surface_v1_send_position(surface->resource, surface->view->base.geometry.x,
                                  surface->view->base.geometry.y);
}

static void surface_handle_view_output(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, view_output);
    struct kywc_output *old_output = data;

    if (!wl_list_empty(&surface->output_update_usable_area.link)) {
        wl_list_remove(&surface->output_update_usable_area.link);
        wl_list_init(&surface->output_update_usable_area.link);
        output_update_usable_area(old_output);
    }

    ukui_surface_set_usable_area(surface, true);
}

static void surface_handle_view_destroy(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, view_destroy);

    wl_list_remove(&surface->view_destroy.link);
    wl_list_remove(&surface->view_map.link);
    wl_list_remove(&surface->view_unmap.link);

    surface->view = NULL;
}

static void surface_handle_view_map(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, view_map);
    ukui_surface_set_usable_area(surface, true);

    surface->view_minimize.notify = surface_handle_view_minimize;
    wl_signal_add(&surface->view->base.events.minimize, &surface->view_minimize);
    surface->view_size.notify = surface_handle_view_size;
    wl_signal_add(&surface->view->base.events.size, &surface->view_size);
    surface->view_position.notify = surface_handle_view_position;
    wl_signal_add(&surface->view->base.events.position, &surface->view_position);
    surface->view_output.notify = surface_handle_view_output;
    wl_signal_add(&surface->view->events.output, &surface->view_output);

    ukui_surface_v1_send_position(surface->resource, surface->view->base.geometry.x,
                                  surface->view->base.geometry.y);
}

static void surface_handle_view_unmap(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, view_unmap);

    wl_list_remove(&surface->view_minimize.link);
    wl_list_remove(&surface->view_size.link);
    wl_list_remove(&surface->view_position.link);
    wl_list_remove(&surface->view_output.link);

    ukui_surface_set_usable_area(surface, false);

    struct ukui_keyboard_grab *grab, *tmp;
    wl_list_for_each_safe(grab, tmp, &surface->ukui_keyboard_grabs, link) {
        end_ukui_grab_keyboard(grab);
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
        kywc_log(KYWC_DEBUG, "surface is not a toplevel");
        surface->buffer = ky_scene_buffer_try_from_surface(surface->wlr_surface);
        return;
    }

    kywc_view_set_skip_taskbar(&surface->view->base, surface->skip_taskbar);
    kywc_view_set_skip_switcher(&surface->view->base, surface->skip_switcher);

    surface->view->startup_geometry.geometry = surface->startup.geometry;
    surface->view->startup_geometry.use_startup_geometry = surface->startup.use_startup_geometry;

    if (surface->role_changed) {
        ukui_surface_apply_role(surface);
    }

    if (surface->state_changed) {
        for (int i = 0; i < STATE_FLAG_LAST; i++) {
            if ((surface->surface_flags >> i) & 0x1) {
                ukui_surface_set_state(surface, i, (surface->surface_state >> i) & 0x1);
            }
        }
        surface->state_changed = false;
    }

    if (surface->icon_name) {
        view_set_icon_name(surface->view, surface->icon_name);
    }

    if (surface->open_under_cursor) {
        struct seat *seat = surface->view->base.focused_seat;
        surface->view->base.has_initial_position = true;
        view_do_move(surface->view, seat->cursor->lx, seat->cursor->ly);
    }

    if (surface->hide_changed) {
        panel_change_layer(surface->view, surface->panel_auto_hide);
    }

    /* apply set_position called before map */
    if (surface->x != INT32_MAX || surface->y != INT32_MAX) {
        surface->view->base.has_initial_position = true;
        ukui_surface_apply_position(surface);
    }

    surface->view_map.notify = surface_handle_view_map;
    wl_signal_add(&surface->view->base.events.map, &surface->view_map);
    surface->view_unmap.notify = surface_handle_view_unmap;
    wl_signal_add(&surface->view->base.events.unmap, &surface->view_unmap);
    surface->view_destroy.notify = surface_handle_view_destroy;
    wl_signal_add(&surface->view->base.events.destroy, &surface->view_destroy);
}

static void surface_handle_destroy(struct wl_listener *listener, void *data)
{
    struct ukui_surface *surface = wl_container_of(listener, surface, surface_destroy);

    struct ukui_keyboard_grab *grab, *tmp;
    wl_list_for_each_safe(grab, tmp, &surface->ukui_keyboard_grabs, link) {
        end_ukui_grab_keyboard(grab);
    }

    wl_list_remove(&surface->surface_map.link);
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
    if (!wlr_surface || !shell) {
        return;
    }
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
    surface = calloc(1, sizeof(struct ukui_surface));
    if (!surface) {
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
    surface->role = UKUI_SURFACE_V1_ROLE_NORMAL;
    wl_list_init(&surface->output_update_usable_area.link);
    wl_list_init(&surface->ukui_keyboard_grabs);

    surface->wlr_surface = wlr_surface;
    surface->surface_map.notify = surface_handle_map;
    wl_signal_add(&wlr_surface->events.map, &surface->surface_map);
    surface->surface_destroy.notify = surface_handle_destroy;
    wl_signal_add(&wlr_surface->events.destroy, &surface->surface_destroy);

    wl_list_insert(&shell->ukui_surfaces, &surface->link);
}

static void ukui_decoration_apply_titlebar(struct ukui_decoration *decoration)
{
    struct view *view = view_try_from_wlr_surface(decoration->wlr_surface);
    if (!view) {
        return;
    }

    struct kywc_view *kywc_view = &view->base;
    enum kywc_ssd ssd = kywc_view->ssd;

    if (decoration->pending_no_titlebar != decoration->no_titlebar) {
        if (decoration->pending_no_titlebar) {
            ssd &= ~KYWC_SSD_TITLE;
        } else {
            ssd |= KYWC_SSD_TITLE;
        }
    }

    decoration->no_titlebar = decoration->pending_no_titlebar;
    view_set_decoration(view, ssd);
}

static void decoration_handle_surface_map(struct wl_listener *listener, void *data)
{
    struct ukui_decoration *decoration = wl_container_of(listener, decoration, surface_map);
    ukui_decoration_apply_titlebar(decoration);
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
    if (!decoration || !decoration->wlr_surface || decoration->pending_no_titlebar == no_titlebar) {
        return;
    }

    if (decoration->wlr_surface->mapped) {
        wl_resource_post_error(resource, UKUI_SURFACE_V1_ERROR_MAPPED,
                               "wl_surface object already mapped");
        return;
    }

    decoration->pending_no_titlebar = no_titlebar;
}

static void handle_set_decoration_components(struct wl_client *client, struct wl_resource *resource,
                                             uint32_t flags)
{
    struct ukui_decoration *decoration = wl_resource_get_user_data(resource);
    if (!decoration || !decoration->wlr_surface || decoration->flags == flags) {
        return;
    }

    if (decoration->wlr_surface->mapped) {
        wl_resource_post_error(resource, UKUI_SURFACE_V1_ERROR_MAPPED,
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
    struct ukui_shell *shell = wl_resource_get_user_data(shell_resource);
    if (!wlr_surface || !shell) {
        return;
    }

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
    decoration = calloc(1, sizeof(struct ukui_decoration));
    if (!decoration) {
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

    decoration->wlr_surface = wlr_surface;
    decoration->surface_map.notify = decoration_handle_surface_map;
    wl_signal_add(&wlr_surface->events.map, &decoration->surface_map);

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
    struct ukui_shell *shell = calloc(1, sizeof(struct ukui_shell));
    if (!shell) {
        return false;
    }

    shell->global = wl_global_create(server->display, &ukui_shell_v1_interface,
                                     UKUI_SHELL_V1_VERSION, shell, ukui_shell_bind);
    if (!shell->global) {
        kywc_log(KYWC_WARN, "ukui-shell-v1 create failed");
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
