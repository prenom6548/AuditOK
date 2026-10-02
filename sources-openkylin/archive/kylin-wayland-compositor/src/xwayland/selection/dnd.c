// SPDX-FileCopyrightText: 2024 The wlroots contributors
// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <xcb/xfixes.h>

#include <kywc/log.h>

#include "input/seat.h"
#include "output.h"
#include "view/view.h"
#include "xwayland/xwayland.h"
#include "xwayland/xwm.h"

static void xwm_dnd_send_event(struct xwm *xwm, xcb_atom_t type, xcb_client_message_data_t *data)
{
    struct xwayland_surface *dest = xwm->drag_focus;
    assert(dest != NULL);

    xcb_client_message_event_t event = {
        .response_type = XCB_CLIENT_MESSAGE,
        .format = 32,
        .sequence = 0,
        .window = dest->window_id,
        .type = type,
        .data = *data,
    };

    xwm_send_event_with_size(xwm->xcb_conn, 0,
                             dest->proxy_window ? dest->proxy_window : dest->window_id,
                             XCB_EVENT_MASK_NO_EVENT, &event, sizeof(event));
    xcb_flush(xwm->xcb_conn);
}

static void xwm_dnd_send_enter(struct xwm *xwm)
{
    struct drag *drag = xwm->drag;
    assert(drag != NULL);
    struct wl_array *mime_types = &drag->source->mime_types;

    xcb_client_message_data_t data = { 0 };
    data.data32[0] = xwm->dnd_selection.window;
    data.data32[1] = XDND_VERSION << 24;

    // If we have 3 MIME types or less, we can send them directly in the
    // DND_ENTER message
    size_t n = mime_types->size / sizeof(char *);
    if (n <= 3) {
        size_t i = 0;
        char **mime_type_ptr;
        wl_array_for_each(mime_type_ptr, mime_types) {
            char *mime_type = *mime_type_ptr;
            data.data32[2 + i] = xwm_mime_type_to_atom(xwm, mime_type);
            ++i;
        }
    } else {
        // Let the client know that targets are not contained in the message
        // data and must be retrieved with the DND_TYPE_LIST property
        data.data32[1] |= 1;

        xcb_atom_t targets[n];
        size_t i = 0;
        char **mime_type_ptr;
        wl_array_for_each(mime_type_ptr, mime_types) {
            char *mime_type = *mime_type_ptr;
            targets[i] = xwm_mime_type_to_atom(xwm, mime_type);
            ++i;
        }

        xcb_change_property(xwm->xcb_conn, XCB_PROP_MODE_REPLACE, xwm->dnd_selection.window,
                            xwm->atoms[DND_TYPE_LIST].atom, XCB_ATOM_ATOM,
                            32, // format
                            n, targets);
    }

    xwm_dnd_send_event(xwm, xwm->atoms[DND_ENTER].atom, &data);
}

static void xwm_dnd_send_position(struct xwm *xwm, uint32_t time, int16_t x, int16_t y)
{
    struct drag *drag = xwm->drag;
    assert(drag != NULL);

    xcb_client_message_data_t data = { 0 };
    data.data32[0] = xwm->dnd_selection.window;
    data.data32[2] = (x << 16) | y;
    data.data32[3] = time;
    data.data32[4] = data_device_manager_dnd_action_to_atom(xwm, drag->source->actions);

    xwm_dnd_send_event(xwm, xwm->atoms[DND_POSITION].atom, &data);
}

static void xwm_dnd_send_drop(struct xwm *xwm, uint32_t time)
{
    struct drag *drag = xwm->drag;
    assert(drag != NULL);
    struct xwayland_surface *dest = xwm->drag_focus;
    assert(dest != NULL);

    xcb_client_message_data_t data = { 0 };
    data.data32[0] = xwm->dnd_selection.window;
    data.data32[2] = time;

    xwm_dnd_send_event(xwm, xwm->atoms[DND_DROP].atom, &data);
}

static void xwm_dnd_send_leave(struct xwm *xwm)
{
    struct drag *drag = xwm->drag;
    assert(drag != NULL);
    struct xwayland_surface *dest = xwm->drag_focus;
    assert(dest != NULL);

    xcb_client_message_data_t data = { 0 };
    data.data32[0] = xwm->dnd_selection.window;

    xwm_dnd_send_event(xwm, xwm->atoms[DND_LEAVE].atom, &data);
}

static bool xwm_add_atom_to_mime_types(struct xwm *xwm, struct wl_array *mime_types,
                                       struct wl_array *mime_types_atoms, xcb_atom_t atom)
{
    char *mime_type = xwm_mime_type_from_atom(xwm, atom);
    if (mime_type == NULL) {
        return false;
    }

    char **mime_type_ptr = wl_array_add(mime_types, sizeof(*mime_type_ptr));
    if (mime_type_ptr == NULL) {
        return false;
    }
    *mime_type_ptr = mime_type;

    xcb_atom_t *mime_type_atom_ptr = wl_array_add(mime_types_atoms, sizeof(*mime_type_atom_ptr));
    if (mime_type_atom_ptr == NULL) {
        return false;
    }
    *mime_type_atom_ptr = atom;
    kywc_log(KYWC_DEBUG, "Mime_type %s", mime_type);
    return true;
}

static bool xwm_dnd_get_mime_types(struct xwm *xwm, struct wl_array *mime_types,
                                   struct wl_array *mime_types_atoms, xcb_window_t source)
{
    xcb_get_property_cookie_t cookie =
        xcb_get_property(xwm->xcb_conn,
                         0, // delete
                         source, xwm->atoms[DND_TYPE_LIST].atom, XCB_GET_PROPERTY_TYPE_ANY,
                         0,         // offset
                         0x1fffffff // length
        );

    xcb_get_property_reply_t *reply = xcb_get_property_reply(xwm->xcb_conn, cookie, NULL);
    if (reply == NULL) {
        return false;
    }
    if (reply->type != XCB_ATOM_ATOM || reply->value_len == 0) {
        kywc_log(KYWC_ERROR, "Invalid XdndTypeList property");
        goto error;
    }

    xcb_atom_t *atoms = xcb_get_property_value(reply);
    for (uint32_t i = 0; i < reply->value_len; ++i) {
        if (!xwm_add_atom_to_mime_types(xwm, mime_types, mime_types_atoms, atoms[i])) {
            kywc_log(KYWC_ERROR, "Failed to add MIME type atom to list");
            goto error;
        }
    }

    free(reply);
    return true;

error:
    free(reply);
    return false;
}

static void xwm_handle_dnd_enter(struct xwm *xwm, xcb_client_message_data_t *data)
{
    if (!xwm->drag || !xwm->drag->drag_x11 || !xwm->drag->source) {
        kywc_log(KYWC_ERROR, "Ignoring XdndEnter client message because "
                             "no xwayland drag is being performed");
        return;
    }

    struct x11_data_source *source = x11_data_source_from_data_source(xwm->drag->source);
    // already has data source
    if (source->mime_types_atoms.size > 0) {
        return;
    }

    xcb_window_t source_window = data->data32[0];
    if (source_window != xwm->dnd_selection.owner) {
        kywc_log(KYWC_ERROR, "Ignoring XdndEnter client message because the source window "
                             "hasn't set the drag-and-drop selection");
        return;
    }

    if ((data->data32[1] & 1) == 0) {
        // Less than 3 MIME types, those are in the message data
        for (size_t i = 0; i < 3; ++i) {
            xcb_atom_t atom = data->data32[2 + i];
            if (atom == XCB_ATOM_NONE) {
                break;
            }
            if (!xwm_add_atom_to_mime_types(xwm, &source->base.mime_types,
                                            &source->mime_types_atoms, atom)) {
                kywc_log(KYWC_ERROR, "Failed to add MIME type atom to list");
                break;
            }
        }
    } else {
        if (!xwm_dnd_get_mime_types(xwm, &source->base.mime_types, &source->mime_types_atoms,
                                    source_window)) {
            kywc_log(KYWC_ERROR, "Failed to add MIME type atom to list");
        }
    }

    // unmap the dnd window catcher, mapping it when enter wayland window
    xwm_map_dnd_selection_window(xwm, false);
}

static void xwm_handle_dnd_position(struct xwm *xwm, xcb_client_message_data_t *data)
{
    if (!xwm->drag || !xwm->drag->drag_x11) {
        kywc_log(KYWC_DEBUG, "Ignoring XdndPosition client message because "
                             "no xwayland drag is being performed");
        return;
    }

    xcb_window_t source_window = data->data32[0];
    xcb_timestamp_t timestamp = data->data32[3];
    xcb_atom_t action_atom = data->data32[4];

    if (source_window != xwm->dnd_selection.owner) {
        kywc_log(KYWC_DEBUG, "Ignoring XdndPosition client message because "
                             "the source window hasn't set the drag-and-drop selection");
        return;
    }

    enum wl_data_device_manager_dnd_action action =
        data_device_manager_dnd_action_from_atom(xwm, action_atom);
    xwm->drag->source->actions = action;

    kywc_log(KYWC_DEBUG, "DND_POSITION window=%d timestamp=%u action=%d", source_window, timestamp,
             action);
}

static void xwm_handle_dnd_drop(struct xwm *xwm, xcb_client_message_data_t *data)
{
    if (xwm->drag && xwm->drag->drag_x11) {
        kywc_log(KYWC_ERROR, "The drag should be cancelled in the grab pointer button");
    }
}

int xwm_handle_selection_client_message(struct xwm *xwm, xcb_client_message_event_t *ev)
{
    if (ev->type == xwm->atoms[DND_STATUS].atom) {
        if (xwm->drag == NULL) {
            kywc_log(KYWC_DEBUG, "ignoring XdndStatus client message because "
                                 "there's no drag");
            return 1;
        }

        xcb_client_message_data_t *data = &ev->data;
        xcb_window_t target_window = data->data32[0];
        bool accepted = data->data32[1] & 1;
        xcb_atom_t action_atom = data->data32[4];

        if (xwm->drag_focus == NULL || target_window != xwm->drag_focus->window_id) {
            kywc_log(KYWC_DEBUG, "ignoring XdndStatus client message because "
                                 "it doesn't match the current drag focus window ID");
            return 1;
        }

        enum wl_data_device_manager_dnd_action action =
            data_device_manager_dnd_action_from_atom(xwm, action_atom);

        struct drag *drag = xwm->drag;
        assert(drag != NULL);

        drag->source->accepted = accepted;
        wl_signal_emit_mutable(&drag->source->events.accepted, NULL);

        data_source_dnd_action(drag->source, action);

        drag->pos.waiting = false;
        if (drag->pos.cached) {
            drag->pos.cached = false;
            xwm_dnd_send_position(xwm, XCB_CURRENT_TIME, drag->pos.cache_x, drag->pos.cache_y);
        }

        kywc_log(KYWC_DEBUG, "DND_STATUS window=%" PRIu32 " accepted=%d action=%d", target_window,
                 accepted, action);
        return 1;
    } else if (ev->type == xwm->atoms[DND_FINISHED].atom) {
        // This should only happen after the drag has ended, but before the drag
        // source is destroyed
        if (xwm->seat == NULL || xwm->seat->drag_source == NULL || xwm->drag != NULL) {
            kywc_log(KYWC_DEBUG, "ignoring XdndFinished client message because "
                                 "there's no finished drag");
            return 1;
        }

        struct data_source *source = xwm->seat->drag_source;

        xcb_client_message_data_t *data = &ev->data;
        xcb_window_t target_window = data->data32[0];
        bool performed = data->data32[1] & 1;
        xcb_atom_t action_atom = data->data32[2];

        if (xwm->drop_focus == NULL || target_window != xwm->drop_focus->window_id) {
            kywc_log(KYWC_DEBUG, "ignoring XdndFinished client message because "
                                 "it doesn't match the finished drop focus window ID");
            return 1;
        }

        enum wl_data_device_manager_dnd_action action =
            data_device_manager_dnd_action_from_atom(xwm, action_atom);

        data_source_dnd_finish(source);
        kywc_log(KYWC_DEBUG, "DND_FINISH window=%" PRIu32 " performed=%d action=%d", target_window,
                 performed, action);
        return 1;
    } else if (ev->type == xwm->atoms[DND_ENTER].atom) {
        kywc_log(KYWC_DEBUG, "DND enter");
        xwm_handle_dnd_enter(xwm, &ev->data);
        return 1;
    } else if (ev->type == xwm->atoms[DND_POSITION].atom) {
        kywc_log(KYWC_DEBUG, "DND position");
        xwm_handle_dnd_position(xwm, &ev->data);
        return 1;
    } else if (ev->type == xwm->atoms[DND_DROP].atom) {
        kywc_log(KYWC_DEBUG, "DND drop");
        xwm_handle_dnd_drop(xwm, &ev->data);
        return 1;
    } else if (ev->type == xwm->atoms[DND_LEAVE].atom) {
        kywc_log(KYWC_DEBUG, "DND leave");
        return 1;
    } else {
        return 0;
    }
}

static void xwm_set_drag_focus(struct xwm *xwm, struct xwayland_surface *focus);

static void drag_focus_handle_destroy(struct wl_listener *listener, void *data)
{
    struct xwm *xwm = wl_container_of(listener, xwm, drag_focus_destroy);
    xwm_set_drag_focus(xwm, NULL);
}

static void drop_focus_handle_destroy(struct wl_listener *listener, void *data)
{
    struct xwm *xwm = wl_container_of(listener, xwm, drop_focus_destroy);
    wl_list_remove(&xwm->drop_focus_destroy.link);
    wl_list_init(&xwm->drop_focus_destroy.link);
    xwm->drop_focus = NULL;
}

static bool property_reply_is_valid(xcb_get_property_reply_t *reply)
{
    return reply && reply->type == XCB_ATOM_WINDOW && reply->format == 32 &&
           xcb_get_property_value_length(reply) == sizeof(xcb_window_t);
}

static xcb_window_t get_proxy_window(struct xwm *xwm, xcb_window_t window)
{
    xcb_window_t target_window = window;
    xcb_get_property_cookie_t proxy_cookie = xcb_get_property(
        xwm->xcb_conn, 0, window, xwm->atoms[DND_PROXY].atom, XCB_ATOM_WINDOW, 0, 1);
    xcb_get_property_reply_t *proxy_reply =
        xcb_get_property_reply(xwm->xcb_conn, proxy_cookie, NULL);

    if (property_reply_is_valid(proxy_reply)) {
        xcb_window_t proxy_window = *(xcb_window_t *)xcb_get_property_value(proxy_reply);

        xcb_get_property_cookie_t proxy_verify_cookie = xcb_get_property(
            xwm->xcb_conn, 0, proxy_window, xwm->atoms[DND_PROXY].atom, XCB_ATOM_WINDOW, 0, 1);
        xcb_get_property_reply_t *proxy_verify_reply =
            xcb_get_property_reply(xwm->xcb_conn, proxy_verify_cookie, NULL);

        if (property_reply_is_valid(proxy_verify_reply)) {
            xcb_window_t verifyWindow = *(xcb_window_t *)xcb_get_property_value(proxy_verify_reply);
            if (verifyWindow == proxy_window) {
                target_window = proxy_window;
            }
        }
        free(proxy_verify_reply);
    }

    free(proxy_reply);
    return target_window;
}

static void xwm_set_drag_focus(struct xwm *xwm, struct xwayland_surface *focus)
{
    if (focus == xwm->drag_focus) {
        return;
    }

    /**
     * The drag has already ended (drag == NULL) but drag_focus was kept alive
     * for selection transfer. If this function is called during that window
     * (e.g. the drag_focus surface is destroyed), clean up the lingering
     * listener and pointer.
     */
    if (xwm->drag == NULL) {
        wl_list_remove(&xwm->drag_focus_destroy.link);
        wl_list_init(&xwm->drag_focus_destroy.link);
        xwm->drag_focus = NULL;
        return;
    }

    // clear pos flags
    xwm->drag->pos.waiting = false;
    xwm->drag->pos.cached = false;

    if (xwm->drag_focus != NULL) {
        data_source_dnd_action(xwm->drag->source, WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE);
        xwm_dnd_send_leave(xwm);
    }

    wl_list_remove(&xwm->drag_focus_destroy.link);
    wl_list_init(&xwm->drag_focus_destroy.link);

    xwm->drag_focus = focus;

    if (xwm->drag_focus != NULL) {
        xwm->drag_focus_destroy.notify = drag_focus_handle_destroy;
        wl_signal_add(&xwm->drag_focus->events.destroy, &xwm->drag_focus_destroy);

        if (!xwm->drag_focus->proxy_window) {
            xwm->drag_focus->proxy_window = get_proxy_window(xwm, focus->window_id);
        }

        xwm_dnd_send_enter(xwm);
    }
}

static void seat_handle_drag_focus(struct wl_listener *listener, void *data)
{
    struct drag *drag = data;
    struct xwm *xwm = wl_container_of(listener, xwm, seat_drag_focus);

    // dnd_leave will be sent when drag destroy if necessary
    if (drag->cancelling) {
        return;
    }

    /* xwayland source */
    if (xwm->drag && xwm->drag->drag_x11) {
        if (drag->focus && xwayland_surface_try_from_wlr_surface(drag->focus)) {
            xwm_map_dnd_selection_window(xwm, false);
        } else {
            xwm_map_dnd_selection_window(xwm, true);
        }
        return;
    }

    struct xwayland_surface *focus = NULL;
    if (drag->focus != NULL) {
        focus = xwayland_surface_try_from_wlr_surface(drag->focus);
    }

    xwm_set_drag_focus(xwm, focus);
}

static void seat_handle_drag_motion(struct wl_listener *listener, void *data)
{
    struct xwm *xwm = wl_container_of(listener, xwm, seat_drag_motion);
    struct drag_motion_event *event = data;
    struct xwayland_surface *surface = xwm->drag_focus;
    struct drag *drag = xwm->drag;

    if (!surface) {
        return; // No xwayland surface focused
    }

    if (drag->pos.waiting) {
        drag->pos.cache_x = (int16_t)event->sx;
        drag->pos.cache_y = (int16_t)event->sy;
        drag->pos.cached = true;
        return;
    }

    drag->pos.waiting = true;

    xwm_dnd_send_position(xwm, event->time, surface->x + (int16_t)event->sx,
                          surface->y + (int16_t)event->sy);
}

static void seat_handle_drag_drop(struct wl_listener *listener, void *data)
{
    struct xwm *xwm = wl_container_of(listener, xwm, seat_drag_drop);
    struct drag_drop_event *event = data;

    if (xwm->drag_focus == NULL) {
        return; // No xwayland surface focused
    }

    kywc_log(KYWC_DEBUG, "Wayland drag dropped over an Xwayland window");

    xwm->drop_focus = xwm->drag_focus;
    xwm->drop_focus_destroy.notify = drop_focus_handle_destroy;
    wl_list_remove(&xwm->drop_focus_destroy.link);
    wl_signal_add(&xwm->drop_focus->events.destroy, &xwm->drop_focus_destroy);

    xwm_dnd_send_drop(xwm, event->time);
}

static void seat_handle_drag_destroy(struct wl_listener *listener, void *data)
{
    struct xwm *xwm = wl_container_of(listener, xwm, seat_drag_destroy);
    struct drag *drag = xwm->drag;

    if (drag->drag_x11) {
        xwm_map_dnd_selection_window(xwm, false);
    }

    // Don't reset drag focus yet because the target will read the drag source
    // right after
    if (xwm->drag_focus != NULL && xwm->drag->source && !xwm->drag->source->accepted) {
        kywc_log(KYWC_DEBUG, "Wayland drag cancelled over an Xwayland window");
        xwm_dnd_send_leave(xwm);
    }

    wl_list_remove(&xwm->seat_drag_focus.link);
    wl_list_remove(&xwm->seat_drag_motion.link);
    wl_list_remove(&xwm->seat_drag_drop.link);
    wl_list_remove(&xwm->seat_drag_destroy.link);
    wl_list_remove(&xwm->cursor_motion.link);
    wl_list_remove(&xwm->cursor_button.link);
    xwm->drag = NULL;
}

static void seat_handle_drag_source_destroy(struct wl_listener *listener, void *data)
{
    struct xwm *xwm = wl_container_of(listener, xwm, seat_drag_source_destroy);

    wl_list_remove(&xwm->seat_drag_source_destroy.link);
    wl_list_init(&xwm->seat_drag_source_destroy.link);
    wl_list_remove(&xwm->drag_focus_destroy.link);
    wl_list_init(&xwm->drag_focus_destroy.link);
    xwm->drag_focus = NULL;

    wl_list_remove(&xwm->drop_focus_destroy.link);
    wl_list_init(&xwm->drop_focus_destroy.link);
    xwm->drop_focus = NULL;
}

static void handle_cursor_motion(struct wl_listener *listener, void *data)
{
    struct xwm *xwm = wl_container_of(listener, xwm, cursor_motion);
    struct seat_cursor_motion_event *event = data;
    struct drag *drag = xwm->drag;
    struct seat *seat = drag->seat;
    struct x11_data_source *x11_data_source = x11_data_source_from_data_source(drag->source);
    assert(x11_data_source->view);

    /* it is always necessary to send the position to xwayland */
    // TODO: may be touch drag
    double sx = event->lx - x11_data_source->view->base.geometry.x;
    double sy = event->ly - x11_data_source->view->base.geometry.y;
    wlr_seat_pointer_enter(seat->wlr_seat, x11_data_source->view->surface, xwayland_scale(sx),
                           xwayland_scale(sy));
    wlr_seat_pointer_send_motion(seat->wlr_seat, event->time_msec, xwayland_scale(sx),
                                 xwayland_scale(sy));
    wlr_seat_pointer_send_frame(seat->wlr_seat);

    if (!drag->focus) {
        // send no action to source window when hover NULL
        xwm_send_dnd_status(xwm, xwm->dnd_selection.window, xwm->dnd_selection.owner, 0);
    }
}

static void handle_cursor_button(struct wl_listener *listener, void *data)
{
    struct xwm *xwm = wl_container_of(listener, xwm, cursor_button);
    struct drag *drag = xwm->drag;
    struct seat *seat = drag->seat;
    struct x11_data_source *x11_data_source = x11_data_source_from_data_source(drag->source);
    assert(x11_data_source->view);

    // default left button drag, TODO: other button drag
    struct seat_cursor_button_event *event = data;
    if (event->button != BTN_LEFT || event->pressed) {
        return;
    }

    /* it is always necessary to send the button to xwayland */
    // TODO: may be touch drag
    struct wl_client *wl_client = wl_resource_get_client(x11_data_source->view->surface->resource);
    struct wlr_seat_client *client = wlr_seat_client_for_wl_client(seat->wlr_seat, wl_client);
    uint32_t serial = wlr_seat_client_next_serial(client);
    struct wl_resource *resource;
    wl_resource_for_each(resource, &client->pointers) {
        wl_pointer_send_button(resource, serial, event->time_msec, BTN_LEFT, WLR_BUTTON_RELEASED);
    }
}

void xwm_seat_handle_start_drag(struct xwm *xwm, struct drag *drag)
{
    wl_list_remove(&xwm->drag_focus_destroy.link);
    wl_list_init(&xwm->drag_focus_destroy.link);
    wl_list_remove(&xwm->drop_focus_destroy.link);
    wl_list_init(&xwm->drop_focus_destroy.link);

    xwm->drag = drag;
    xwm->drag_focus = NULL;
    xwm->drop_focus = NULL;

    if (drag != NULL) {
        wl_signal_add(&drag->events.focus, &xwm->seat_drag_focus);
        xwm->seat_drag_focus.notify = seat_handle_drag_focus;
        wl_signal_add(&drag->events.motion, &xwm->seat_drag_motion);
        xwm->seat_drag_motion.notify = seat_handle_drag_motion;
        wl_signal_add(&drag->events.drop, &xwm->seat_drag_drop);
        xwm->seat_drag_drop.notify = seat_handle_drag_drop;
        wl_signal_add(&drag->events.destroy, &xwm->seat_drag_destroy);
        xwm->seat_drag_destroy.notify = seat_handle_drag_destroy;

        wl_signal_add(&drag->source->events.destroy, &xwm->seat_drag_source_destroy);
        xwm->seat_drag_source_destroy.notify = seat_handle_drag_source_destroy;

        xwm->cursor_motion.notify = handle_cursor_motion;
        xwm->cursor_button.notify = handle_cursor_button;
        if (drag->drag_x11) {
            wl_signal_add(&drag->seat->events.cursor_motion, &xwm->cursor_motion);
            wl_signal_add(&drag->seat->events.cursor_button, &xwm->cursor_button);
        } else {
            wl_list_init(&xwm->cursor_motion.link);
            wl_list_init(&xwm->cursor_button.link);
        }
    }
}

void xwm_seat_unlink_drag_handlers(struct xwm *xwm)
{
    wl_list_remove(&xwm->seat_drag_source_destroy.link);
    wl_list_remove(&xwm->drag_focus_destroy.link);
    wl_list_remove(&xwm->drop_focus_destroy.link);

    if (!xwm->drag) {
        return;
    }
    wl_list_remove(&xwm->seat_drag_focus.link);
    wl_list_remove(&xwm->seat_drag_motion.link);
    wl_list_remove(&xwm->seat_drag_drop.link);
    wl_list_remove(&xwm->seat_drag_destroy.link);
    wl_list_remove(&xwm->cursor_motion.link);
    wl_list_remove(&xwm->cursor_button.link);
}

void xwm_map_dnd_selection_window(struct xwm *xwm, bool map)
{
    if (map == xwm->dnd_selection.window_mapped) {
        return;
    }

    xwm->dnd_selection.window_mapped = map;
    int width, height;
    output_layout_get_size(&width, &height);
    if (map) {
        xcb_map_window(xwm->xcb_conn, xwm->dnd_selection.window);
        uint16_t value_mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
                              XCB_CONFIG_WINDOW_HEIGHT;
        uint32_t value_list[] = { 0, 0, xwayland_scale(width), xwayland_scale(height) };
        xcb_configure_window(xwm->xcb_conn, xwm->dnd_selection.window, value_mask, value_list);
        uint32_t values[] = { XCB_STACK_MODE_ABOVE };
        xcb_configure_window(xwm->xcb_conn, xwm->dnd_selection.window, XCB_CONFIG_WINDOW_STACK_MODE,
                             values);
    } else {
        xcb_unmap_window(xwm->xcb_conn, xwm->dnd_selection.window);
    }
    xcb_flush(xwm->xcb_conn);
}

void xwm_send_dnd_status(struct xwm *xwm, xcb_window_t requestor, xcb_window_t window,
                         uint32_t action)
{
    uint32_t flags = 0;
    if (action != WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE) {
        flags = 1 << 1;  // Opt-in for XdndPosition messages
        flags |= 1 << 0; // We accept the drop
    }

    xcb_client_message_data_t data = { 0 };
    data.data32[0] = requestor;
    data.data32[1] = flags;
    data.data32[4] = data_device_manager_dnd_action_to_atom(xwm, action);

    xcb_client_message_event_t event = {
        .response_type = XCB_CLIENT_MESSAGE,
        .format = 32,
        .sequence = 0,
        .window = window,
        .type = xwm->atoms[DND_STATUS].atom,
        .data = data,
    };

    xcb_send_event(xwm->xcb_conn, 0, window, XCB_EVENT_MASK_NO_EVENT, (char *)&event);
    xcb_flush(xwm->xcb_conn);
}

void xwm_send_dnd_finish(struct xwm *xwm, xcb_window_t requestor, xcb_window_t window, bool accept,
                         uint32_t action)
{
    xcb_client_message_data_t data = { 0 };
    data.data32[0] = requestor;
    data.data32[1] = accept;
    if (accept) {
        data.data32[2] = data_device_manager_dnd_action_to_atom(xwm, action);
    }

    xcb_client_message_event_t event = {
        .response_type = XCB_CLIENT_MESSAGE,
        .format = 32,
        .sequence = 0,
        .window = window,
        .type = xwm->atoms[DND_FINISHED].atom,
        .data = data,
    };
    xcb_send_event(xwm->xcb_conn, 0, window, XCB_EVENT_MASK_NO_EVENT, (char *)&event);
    xcb_flush(xwm->xcb_conn);
}

xcb_atom_t data_device_manager_dnd_action_to_atom(struct xwm *xwm,
                                                  enum wl_data_device_manager_dnd_action action)
{
    if (action & WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY) {
        return xwm->atoms[DND_ACTION_COPY].atom;
    } else if (action & WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE) {
        return xwm->atoms[DND_ACTION_MOVE].atom;
    } else if (action & WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK) {
        return xwm->atoms[DND_ACTION_ASK].atom;
    }
    return XCB_ATOM_NONE;
}

enum wl_data_device_manager_dnd_action data_device_manager_dnd_action_from_atom(struct xwm *xwm,
                                                                                xcb_atom_t atom)
{
    if (atom == xwm->atoms[DND_ACTION_COPY].atom || atom == xwm->atoms[DND_ACTION_PRIVATE].atom) {
        return WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
    } else if (atom == xwm->atoms[DND_ACTION_MOVE].atom) {
        return WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE;
    } else if (atom == xwm->atoms[DND_ACTION_ASK].atom) {
        return WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK;
    }
    return WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
}
