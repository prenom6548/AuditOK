// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include <kywc/log.h>

#include "input/data_device.h"
#include "input/primary_selection.h"
#include "input_p.h"
#include "scene/surface.h"
#include "server.h"
#include "view/view.h"

struct selection_manager {
    struct wl_listener new_seat;
    struct wl_listener server_destroy;
};

/* selection per seat */
struct selection {
    struct selection_manager *manager;
    struct seat *seat;
    struct wl_listener seat_destroy;

    struct wl_listener request_start_drag;
    struct wl_listener start_drag;
    struct wl_listener destroy_drag;

    struct wl_listener request_set_selection;
    struct wl_listener request_set_primary_selection;

    struct drag *drag;
    /* only one drag_icon in seat at the same time */
    struct drag_icon *drag_icon;
    struct scene_node *icon_node;
    struct scene_node *surface_node;

    struct wl_listener drag_icon_map;
    struct wl_listener drag_icon_unmap;
    struct wl_listener drag_icon_commit;
    struct wl_listener drag_icon_destroy;

    struct wl_listener source_accepted;
    struct wl_listener source_dnd_action;

    struct wl_listener pointer_grab_begin;
    struct wl_listener focus_change;
};

static void handle_drag_icon_map(struct wl_listener *listener, void *data)
{
    struct selection *selection = wl_container_of(listener, selection, drag_icon_map);
    scene_node_set_enabled(selection->icon_node, true);
}

static void handle_drag_icon_unmap(struct wl_listener *listener, void *data)
{
    struct selection *selection = wl_container_of(listener, selection, drag_icon_unmap);
    scene_node_set_enabled(selection->icon_node, false);
}

static void handle_drag_icon_commit(struct wl_listener *listener, void *data)
{
    struct selection *selection = wl_container_of(listener, selection, drag_icon_commit);
    struct wlr_surface *surface = selection->drag_icon->surface;

    scene_node_set_position(selection->surface_node,
                            selection->surface_node->x + surface->current.dx,
                            selection->surface_node->y + surface->current.dy);
}

static void handle_drag_icon_destroy(struct wl_listener *listener, void *data)
{
    struct selection *selection = wl_container_of(listener, selection, drag_icon_destroy);

    wl_list_remove(&selection->drag_icon_map.link);
    wl_list_remove(&selection->drag_icon_unmap.link);
    wl_list_remove(&selection->drag_icon_commit.link);
    wl_list_remove(&selection->drag_icon_destroy.link);

    scene_node_destroy(selection->icon_node);
    selection->icon_node = NULL;
}

static void handle_request_set_selection(struct wl_listener *listener, void *data)
{
    struct selection *selection = wl_container_of(listener, selection, request_set_selection);
    struct seat_request_set_selection_event *event = data;
    seat_set_selection(selection->seat, event->source, event->serial);
}

static void handle_request_start_drag(struct wl_listener *listener, void *data)
{
    struct selection *selection = wl_container_of(listener, selection, request_start_drag);
    struct wlr_seat *wlr_seat = selection->seat->wlr_seat;
    struct seat_request_start_drag_event *event = data;

    if (wlr_seat_validate_pointer_grab_serial(wlr_seat, event->origin, event->serial)) {
        seat_start_pointer_drag(selection->seat, event->drag, event->serial);
        return;
    }

    struct wlr_touch_point *point;
    if (wlr_seat_validate_touch_grab_serial(wlr_seat, event->origin, event->serial, &point)) {
        seat_start_touch_drag(selection->seat, event->drag, event->serial, point);
        return;
    }

    /* workaround to simulate pointer drag */
    if (tablet_has_implicit_grab(selection->seat)) {
        wlr_seat->pointer_state.grab_button = BTN_LEFT;
        selection->seat->cursor->tablet_tool_tip_simulation_pointer = true;
        seat_start_pointer_drag(selection->seat, event->drag, event->serial);
        return;
    }

    data_source_destroy(event->drag->source);
}

static void update_drag_cursor(struct selection *selection)
{
    struct data_source *source = selection->drag->source;
    struct cursor *cursor = selection->seat->cursor;

    cursor_lock_image(cursor, false);
    if (source && source->accepted) {
        switch (source->current_dnd_action) {
        case WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE:
            cursor_set_image(cursor, CURSOR_GRABBING);
            break;
        case WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY:
            cursor_set_image(cursor, CURSOR_COPY);
            break;
        case WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE:
            cursor_set_image(cursor, CURSOR_MOVE);
            break;
        case WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK:
            cursor_set_image(cursor, CURSOR_GRABBING);
            break;
        }
    } else {
        cursor_set_image(cursor, CURSOR_NOT_ALLOWED);
    }
    cursor_lock_image(cursor, true);
}

static void handle_source_accepted(struct wl_listener *listener, void *data)
{
    struct selection *selection = wl_container_of(listener, selection, source_accepted);
    update_drag_cursor(selection);
}

static void handle_source_dnd_action(struct wl_listener *listener, void *data)
{
    struct selection *selection = wl_container_of(listener, selection, source_dnd_action);
    update_drag_cursor(selection);
}

static void handle_pointer_grab_begin(struct wl_listener *listener, void *data)
{
    struct selection *selection = wl_container_of(listener, selection, pointer_grab_begin);
    wl_list_remove(&selection->pointer_grab_begin.link);
    wl_list_init(&selection->pointer_grab_begin.link);
    data_source_destroy(selection->drag->seat->drag_source);
}

static void handle_start_drag(struct wl_listener *listener, void *data)
{
    struct selection *selection = wl_container_of(listener, selection, start_drag);
    struct drag *drag = data;
    struct drag_icon *drag_icon = drag->icon;

    selection->drag = drag;
    selection->drag_icon = drag_icon;
    wl_signal_add(&drag->events.destroy, &selection->destroy_drag);

    selection->source_accepted.notify = handle_source_accepted;
    selection->source_dnd_action.notify = handle_source_dnd_action;
    wl_list_init(&selection->source_accepted.link);
    wl_list_init(&selection->source_dnd_action.link);
    /* the cursor will be set by x11 window if drag x11 window */
    if (!drag->drag_x11) {
        cursor_set_image(selection->seat->cursor, CURSOR_DEFAULT);
        cursor_lock_image(selection->seat->cursor, true);
        if (drag->source) {
            wl_signal_add(&drag->source->events.accepted, &selection->source_accepted);
            wl_signal_add(&drag->source->events.dnd_action, &selection->source_dnd_action);
        }
    }

    if (drag->grab_type == DRAG_GRAB_KEYBOARD_TOUCH) {
        selection->pointer_grab_begin.notify = handle_pointer_grab_begin;
        wl_signal_add(&drag->seat->wlr_seat->events.pointer_grab_begin,
                      &selection->pointer_grab_begin);
    } else {
        wl_list_init(&selection->pointer_grab_begin.link);
    }

    /* drag icon may be NULL */
    if (!drag_icon) {
        kywc_log(KYWC_DEBUG, "Started drag but not set a drag icon");
        return;
    }

    struct view_layer *layer = view_manager_get_layer(LAYER_ON_SCREEN_DISPLAY, false);
    struct scene_tree *tree = scene_tree_create(layer->tree);
    struct scene_tree *surface_tree = scene_subsurface_tree_create(tree, drag_icon->surface);
    selection->icon_node = &tree->node;
    selection->surface_node = &surface_tree->node;

    scene_node_set_position(selection->icon_node, selection->seat->cursor->lx,
                            selection->seat->cursor->ly);
    scene_node_set_enabled(selection->icon_node, drag_icon->surface->mapped);

    selection->drag_icon_map.notify = handle_drag_icon_map;
    wl_signal_add(&drag_icon->surface->events.map, &selection->drag_icon_map);
    selection->drag_icon_unmap.notify = handle_drag_icon_unmap;
    wl_signal_add(&drag_icon->surface->events.unmap, &selection->drag_icon_unmap);
    selection->drag_icon_commit.notify = handle_drag_icon_commit;
    wl_signal_add(&drag_icon->surface->events.commit, &selection->drag_icon_commit);
    selection->drag_icon_destroy.notify = handle_drag_icon_destroy;
    wl_signal_add(&drag_icon->events.destroy, &selection->drag_icon_destroy);
}

static void handle_destroy_drag(struct wl_listener *listener, void *data)
{
    struct selection *selection = wl_container_of(listener, selection, destroy_drag);
    wl_list_remove(&selection->destroy_drag.link);
    wl_list_remove(&selection->source_accepted.link);
    wl_list_remove(&selection->source_dnd_action.link);
    wl_list_remove(&selection->pointer_grab_begin.link);

    if (!selection->drag->drag_x11) {
        cursor_lock_image(selection->seat->cursor, false);
    }
    selection->drag = NULL;
    cursor_set_image(selection->seat->cursor, CURSOR_DEFAULT);
}

static void handle_request_set_primary_selection(struct wl_listener *listener, void *data)
{
    struct selection *selection =
        wl_container_of(listener, selection, request_set_primary_selection);
    struct seat_request_set_primary_selection_event *event = data;
    seat_set_primary_selection(selection->seat, event->source, event->serial);
}

static void handle_seat_destroy(struct wl_listener *listener, void *data)
{
    struct selection *selection = wl_container_of(listener, selection, seat_destroy);

    wl_list_remove(&selection->request_start_drag.link);
    wl_list_remove(&selection->start_drag.link);
    wl_list_remove(&selection->request_set_selection.link);
    wl_list_remove(&selection->request_set_primary_selection.link);
    wl_list_remove(&selection->seat_destroy.link);
    wl_list_remove(&selection->focus_change.link);

    free(selection);
}

static void handle_focus_change(struct wl_listener *listener, void *data)
{
    struct wlr_seat_keyboard_focus_change_event *event = data;
    struct wlr_seat_client *client = event->seat->keyboard_state.focused_client;
    if (client) {
        wlr_seat_client_send_selection(client);
    }
}

static void handle_new_seat(struct wl_listener *listener, void *data)
{
    struct selection *selection = calloc(1, sizeof(*selection));
    if (!selection) {
        return;
    }

    struct selection_manager *manager = wl_container_of(listener, manager, new_seat);
    selection->manager = manager;
    struct seat *seat = data;
    selection->seat = seat;
    seat->selection = selection;

    selection->seat_destroy.notify = handle_seat_destroy;
    wl_signal_add(&seat->events.destroy, &selection->seat_destroy);

    selection->request_start_drag.notify = handle_request_start_drag;
    wl_signal_add(&seat->events.request_start_drag, &selection->request_start_drag);
    selection->start_drag.notify = handle_start_drag;
    wl_signal_add(&seat->events.start_drag, &selection->start_drag);
    selection->destroy_drag.notify = handle_destroy_drag;
    wl_list_init(&selection->destroy_drag.link);

    selection->request_set_selection.notify = handle_request_set_selection;
    wl_signal_add(&seat->events.request_set_selection, &selection->request_set_selection);
    selection->request_set_primary_selection.notify = handle_request_set_primary_selection;
    wl_signal_add(&seat->events.request_set_primary_selection,
                  &selection->request_set_primary_selection);

    // fix seat_client_send_selection in wlr_seat_keyboard_enter
    selection->focus_change.notify = handle_focus_change;
    wl_signal_add(&seat->wlr_seat->keyboard_state.events.focus_change, &selection->focus_change);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    struct selection_manager *manager = wl_container_of(listener, manager, server_destroy);
    wl_list_remove(&manager->server_destroy.link);
    wl_list_remove(&manager->new_seat.link);
    free(manager);
}

bool selection_manager_create(struct input_manager *input_manager)
{
    struct selection_manager *manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    data_device_manager_create(input_manager->server->display);
    primary_selection_device_manager_create(input_manager->server->display);
    wlr_data_control_manager_create(input_manager->server->display);
    toplevel_drag_manager_create(input_manager->server);

    manager->new_seat.notify = handle_new_seat;
    wl_signal_add(&input_manager->events.new_seat, &manager->new_seat);

    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(input_manager->server, &manager->server_destroy);

    return true;
}

void selection_handle_cursor_move(struct seat *seat, int lx, int ly)
{
    if (!seat->selection || !seat->selection->drag) {
        return;
    }
    if (toplevel_drag_move(seat->selection->drag->source, lx, ly)) {
        return;
    }
    /* update dnd icon if support */
    if (seat->selection->icon_node) {
        scene_node_set_position(seat->selection->icon_node, lx, ly);
    }
}

bool seat_is_dragging(struct seat *seat)
{
    return seat->selection && seat->selection->drag;
}
