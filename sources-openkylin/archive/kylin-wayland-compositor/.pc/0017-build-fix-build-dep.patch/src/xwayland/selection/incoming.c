// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <xcb/xfixes.h>

#include <kywc/log.h>

#include "input/cursor.h"
#include "input/seat.h"
#include "scene/surface.h"
#include "security/security.h"
#include "util/client.h"
#include "view/view.h"
#include "xwayland/xwayland.h"
#include "xwayland/xwm.h"

static struct xwm_selection_transfer *
xwm_selection_transfer_create_incoming(struct xwm_selection *selection)
{
    struct xwm_selection_transfer *transfer = calloc(1, sizeof(*transfer));
    if (!transfer) {
        return NULL;
    }

    xwm_selection_transfer_init(transfer, selection);

    wl_list_insert(&selection->incoming, &transfer->link);

    struct xwm *xwm = selection->xwm;
    if (selection == &xwm->dnd_selection) {
        transfer->incoming_window = xwm->dnd_selection.window;
        return transfer;
    }

    transfer->incoming_window = xcb_generate_id(xwm->xcb_conn);
    xcb_create_window(
        xwm->xcb_conn, XCB_COPY_FROM_PARENT, transfer->incoming_window, xwm->screen->root, 0, 0, 10,
        10, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, xwm->screen->root_visual, XCB_CW_EVENT_MASK,
        (uint32_t[]){ XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_PROPERTY_CHANGE });
    xcb_flush(xwm->xcb_conn);

    return transfer;
}

struct xwm_selection_transfer *
xwm_selection_find_incoming_transfer_by_window(struct xwm_selection *selection, xcb_window_t window)
{
    struct xwm_selection_transfer *transfer;
    wl_list_for_each(transfer, &selection->incoming, link) {
        if (transfer->incoming_window == window) {
            return transfer;
        }
    }

    return NULL;
}

static bool
xwm_selection_transfer_get_incoming_selection_property(struct xwm_selection_transfer *transfer,
                                                       bool delete)
{
    struct xwm *xwm = transfer->selection->xwm;

    xcb_get_property_cookie_t cookie =
        xcb_get_property(xwm->xcb_conn, delete, transfer->incoming_window,
                         xwm->atoms[WL_SELECTION].atom, XCB_GET_PROPERTY_TYPE_ANY,
                         0,         // offset
                         0x1fffffff // length
        );

    transfer->property_start = 0;
    transfer->property_reply = xcb_get_property_reply(xwm->xcb_conn, cookie, NULL);

    if (!transfer->property_reply) {
        kywc_log(KYWC_ERROR, "cannot get selection property");
        return false;
    }

    return true;
}

static void xwm_notify_ready_for_next_incr_chunk(struct xwm_selection_transfer *transfer)
{
    struct xwm *xwm = transfer->selection->xwm;
    assert(transfer->incr);

    kywc_log(KYWC_DEBUG, "deleting property");
    xcb_delete_property(xwm->xcb_conn, transfer->incoming_window, xwm->atoms[WL_SELECTION].atom);
    xcb_flush(xwm->xcb_conn);

    xwm_selection_transfer_remove_event_source(transfer);
    xwm_selection_transfer_destroy_property_reply(transfer);
}

/**
 * Write the X11 selection to a Wayland client. Returns a nonzero value if the
 * Wayland client might become writeable again in the future.
 */
static int write_selection_property_to_wl_client(int fd, uint32_t mask, void *data)
{
    struct xwm_selection_transfer *transfer = data;

    char *property = xcb_get_property_value(transfer->property_reply);
    int remainder =
        xcb_get_property_value_length(transfer->property_reply) - transfer->property_start;

    ssize_t len = write(fd, property + transfer->property_start, remainder);
    if (len == -1) {
        kywc_log_errno(KYWC_ERROR, "write error to target fd %d", fd);
        xwm_selection_transfer_destroy(transfer);
        return 0;
    }

    kywc_log(KYWC_DEBUG, "wrote %zd (total %zd, remaining %d) of %d bytes to fd %d", len,
             transfer->property_start + len, remainder,
             xcb_get_property_value_length(transfer->property_reply), fd);

    if (len < remainder) {
        transfer->property_start += len;
        return 1;
    } else if (transfer->incr) {
        xwm_notify_ready_for_next_incr_chunk(transfer);
    } else {
        kywc_log(KYWC_DEBUG, "transfer complete");
        xwm_selection_transfer_destroy(transfer);
    }

    return 0;
}

static void xwm_write_selection_property_to_wl_client(struct xwm_selection_transfer *transfer)
{
    if (transfer->incr && transfer->wl_client_fd < 0) {
        // Wayland client closed its pipe prematurely before the X11 client finished
        // its incremental transfer. Continue draining the X11 client.
        xwm_notify_ready_for_next_incr_chunk(transfer);
        return;
    }

    bool wl_client_finished_consuming =
        !write_selection_property_to_wl_client(transfer->wl_client_fd, WL_EVENT_WRITABLE, transfer);
    if (!wl_client_finished_consuming) {
        // Wrote out part of the property to the Wayland client, but the client was
        // unable to accept all of it. Schedule an event to asynchronously complete
        // the transfer.
        struct wl_event_loop *loop = wl_display_get_event_loop(transfer->selection->xwm->display);
        transfer->event_source =
            wl_event_loop_add_fd(loop, transfer->wl_client_fd, WL_EVENT_WRITABLE,
                                 write_selection_property_to_wl_client, transfer);
    }
}

void xwm_get_incr_chunk(struct xwm_selection_transfer *transfer)
{
    kywc_log(KYWC_DEBUG, "xwm_get_incr_chunk");

    if (transfer->property_reply) {
        kywc_log(KYWC_ERROR, "X11 client offered a new property before we deleted");
        return;
    }

    if (!xwm_selection_transfer_get_incoming_selection_property(transfer, false)) {
        return;
    }

    if (xcb_get_property_value_length(transfer->property_reply) > 0) {
        xwm_write_selection_property_to_wl_client(transfer);
    } else {
        kywc_log(KYWC_DEBUG, "incremental transfer complete");
        xwm_selection_transfer_destroy(transfer);
    }
}

static void xwm_selection_transfer_get_data(struct xwm_selection_transfer *transfer)
{
    struct xwm *xwm = transfer->selection->xwm;

    if (!xwm_selection_transfer_get_incoming_selection_property(transfer, true)) {
        return;
    }

    if (transfer->property_reply->type == xwm->atoms[INCR].atom) {
        transfer->incr = true;
        xwm_selection_transfer_destroy_property_reply(transfer);
    } else {
        // Reply's ownership is transferred to wm, which is responsible for freeing
        // it.
        xwm_write_selection_property_to_wl_client(transfer);
    }
}

static void source_send(struct xwm_selection *selection, struct wl_array *mime_types,
                        struct wl_array *mime_types_atoms, const char *requested_mime_type, int fd)
{
    struct xwm *xwm = selection->xwm;

    xcb_atom_t *atoms = mime_types_atoms->data;
    bool found = false;
    xcb_atom_t mime_type_atom;
    char **mime_type_ptr;
    size_t i = 0;
    wl_array_for_each(mime_type_ptr, mime_types) {
        char *mime_type = *mime_type_ptr;
        if (strcmp(mime_type, requested_mime_type) == 0) {
            found = true;
            mime_type_atom = atoms[i];
            break;
        }
        ++i;
    }
    if (!found) {
        kywc_log(KYWC_DEBUG, "Cannot send X11 selection to Wayland: "
                             "unsupported MIME type");
        close(fd);
        return;
    }

    struct xwm_selection_transfer *transfer = xwm_selection_transfer_create_incoming(selection);
    if (!transfer) {
        kywc_log(KYWC_ERROR, "Cannot create transfer");
        close(fd);
        return;
    }

    xcb_convert_selection(xwm->xcb_conn, transfer->incoming_window, selection->atom, mime_type_atom,
                          xwm->atoms[WL_SELECTION].atom, XCB_TIME_CURRENT_TIME);

    xcb_flush(xwm->xcb_conn);

    fcntl(fd, F_SETFL, O_WRONLY | O_NONBLOCK);
    transfer->wl_client_fd = fd;
}

static const struct data_source_impl data_source_impl;

bool data_source_is_xwayland(struct data_source *source)
{
    return source->impl == &data_source_impl;
}

struct x11_data_source *x11_data_source_from_data_source(struct data_source *source)
{
    assert(data_source_is_xwayland(source));
    struct x11_data_source *x11_source = wl_container_of(source, x11_source, base);
    return x11_source;
}

static void x11_data_source_send(struct data_source *source, const char *mime_type, int32_t fd)
{
    struct x11_data_source *x11_source = x11_data_source_from_data_source(source);
    struct xwm_selection *selection = x11_source->selection;

    source_send(selection, &source->mime_types, &x11_source->mime_types_atoms, mime_type, fd);
}

static void x11_data_source_accept(struct data_source *source, uint32_t serial,
                                   const char *mime_type)
{
    struct x11_data_source *data_source = wl_container_of(source, data_source, base);
    struct xwm *xwm = data_source->selection->xwm;
    enum wl_data_device_manager_dnd_action action =
        mime_type ? source->current_dnd_action : WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
    xwm_send_dnd_status(xwm, xwm->dnd_selection.window, xwm->dnd_selection.owner, action);
}

static void x11_data_source_finish(struct data_source *source)
{
    struct x11_data_source *data_source = wl_container_of(source, data_source, base);
    data_source->finished = true;

    struct xwm *xwm = data_source->selection->xwm;
    bool accepted = source->accepted;
    enum wl_data_device_manager_dnd_action action = WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
    if (accepted) {
        action = data_device_manager_dnd_action_to_atom(xwm, source->actions);
    }
    xwm_send_dnd_finish(xwm, xwm->dnd_selection.window, xwm->dnd_selection.owner, accepted, action);
}

static void x11_data_source_destroy(struct data_source *source)
{
    struct x11_data_source *x11_source = x11_data_source_from_data_source(source);
    if (!x11_source->finished) {
        x11_data_source_finish(source);
    }

    wl_array_release(&x11_source->mime_types_atoms);
    free(x11_source);
}

static pid_t xwm_selection_get_owner_pid(struct xwm_selection *selection)
{
    struct xwayland_surface *xsurface = xwm_lookup_surface(selection->xwm, selection->owner);
    return xsurface ? xsurface->pid : -1;
}

static void x11_data_source_client(struct data_source *source, struct client *client)
{
    struct x11_data_source *x11_source = x11_data_source_from_data_source(source);
    client->type = CLIENT_TYPE_X11;
    client->pid = xwm_selection_get_owner_pid(x11_source->selection);
    client_set_comm(client);
}

static const struct data_source_impl data_source_impl = {
    .send = x11_data_source_send,
    .accept = x11_data_source_accept,
    .dnd_finish = x11_data_source_finish,
    .destroy = x11_data_source_destroy,
    .client = x11_data_source_client,
};

struct x11_primary_selection_source {
    struct primary_selection_source base;
    struct xwm_selection *selection;
    struct wl_array mime_types_atoms;
};

static const struct primary_selection_source_impl primary_selection_source_impl;

bool primary_selection_source_is_xwayland(struct primary_selection_source *source)
{
    return source->impl == &primary_selection_source_impl;
}

static void x11_primary_selection_source_send(struct primary_selection_source *source,
                                              const char *mime_type, int fd)
{
    struct x11_primary_selection_source *x11_source = wl_container_of(source, x11_source, base);
    struct xwm_selection *selection = x11_source->selection;

    source_send(selection, &source->mime_types, &x11_source->mime_types_atoms, mime_type, fd);
}

static void x11_primary_selection_source_destroy(struct primary_selection_source *source)
{
    struct x11_primary_selection_source *x11_source = wl_container_of(source, x11_source, base);
    wl_array_release(&x11_source->mime_types_atoms);
    free(x11_source);
}

static void x11_primary_selection_source_client(struct primary_selection_source *source,
                                                struct client *client)
{
    struct x11_primary_selection_source *x11_source = wl_container_of(source, x11_source, base);
    client->type = CLIENT_TYPE_X11;
    client->pid = xwm_selection_get_owner_pid(x11_source->selection);
    client_set_comm(client);
}

static const struct primary_selection_source_impl primary_selection_source_impl = {
    .send = x11_primary_selection_source_send,
    .destroy = x11_primary_selection_source_destroy,
    .client = x11_primary_selection_source_client,
};

static bool source_get_targets(struct xwm_selection *selection, struct wl_array *mime_types,
                               struct wl_array *mime_types_atoms)
{
    struct xwm *xwm = selection->xwm;

    xcb_get_property_cookie_t cookie =
        xcb_get_property(xwm->xcb_conn, 1, selection->window, xwm->atoms[WL_SELECTION].atom,
                         XCB_GET_PROPERTY_TYPE_ANY, 0, 4096);

    xcb_get_property_reply_t *reply = xcb_get_property_reply(xwm->xcb_conn, cookie, NULL);
    if (reply == NULL) {
        return false;
    }

    if (reply->type != XCB_ATOM_ATOM) {
        free(reply);
        return false;
    }

    xcb_atom_t *value = xcb_get_property_value(reply);
    for (uint32_t i = 0; i < reply->value_len; i++) {
        char *mime_type = NULL;

        if (value[i] == xwm->atoms[UTF8_STRING].atom) {
            mime_type = strdup("text/plain;charset=utf-8");
        } else if (value[i] == xwm->atoms[TEXT].atom) {
            mime_type = strdup("text/plain");
        } else if (value[i] != xwm->atoms[TARGETS].atom && value[i] != xwm->atoms[TIMESTAMP].atom) {
            xcb_get_atom_name_cookie_t name_cookie = xcb_get_atom_name(xwm->xcb_conn, value[i]);
            xcb_get_atom_name_reply_t *name_reply =
                xcb_get_atom_name_reply(xwm->xcb_conn, name_cookie, NULL);
            if (name_reply == NULL) {
                continue;
            }
            size_t len = xcb_get_atom_name_name_length(name_reply);
            char *name = xcb_get_atom_name_name(name_reply); // not a C string
            if (1 || memchr(name, '/', len) != NULL) {
                mime_type = malloc((len + 1) * sizeof(char));
                if (mime_type == NULL) {
                    free(name_reply);
                    continue;
                }
                memcpy(mime_type, name, len);
                mime_type[len] = '\0';
            }
            free(name_reply);
        }

        if (mime_type != NULL) {
            char **mime_type_ptr = wl_array_add(mime_types, sizeof(*mime_type_ptr));
            if (mime_type_ptr == NULL) {
                free(mime_type);
                break;
            }

            xcb_atom_t *atom_ptr = wl_array_add(mime_types_atoms, sizeof(*atom_ptr));
            if (atom_ptr == NULL) {
                mime_types->size -= sizeof(*mime_type_ptr);
                free(mime_type);
                break;
            }
            *mime_type_ptr = mime_type;
            *atom_ptr = value[i];
        }
    }

    free(reply);
    return true;
}

static void xwm_selection_get_targets(struct xwm_selection *selection)
{
    // set the wayland selection to the X11 selection
    struct xwm *xwm = selection->xwm;

    if (selection == &xwm->clipboard_selection) {
        struct x11_data_source *source = calloc(1, sizeof(*source));
        if (source == NULL) {
            return;
        }
        source->selection = selection;
        data_source_init(&source->base, &data_source_impl);

        wl_array_init(&source->mime_types_atoms);

        bool ok =
            source_get_targets(selection, &source->base.mime_types, &source->mime_types_atoms);
        if (ok) {
            seat_request_set_selection(xwm->seat, NULL, &source->base,
                                       wl_display_next_serial(xwm->display));
        } else {
            data_source_destroy(&source->base);
        }
    } else if (selection == &xwm->primary_selection) {
        struct x11_primary_selection_source *source = calloc(1, sizeof(*source));
        if (source == NULL) {
            return;
        }
        source->selection = selection;
        primary_selection_source_init(&source->base, &primary_selection_source_impl);

        wl_array_init(&source->mime_types_atoms);

        bool ok =
            source_get_targets(selection, &source->base.mime_types, &source->mime_types_atoms);
        if (ok) {
            seat_set_primary_selection(xwm->seat, &source->base,
                                       wl_display_next_serial(xwm->display));
        } else {
            primary_selection_source_destroy(&source->base);
        }
    } else if (selection == &xwm->dnd_selection) {
        // TODO
    }
}

void xwm_handle_selection_notify(struct xwm *xwm, xcb_selection_notify_event_t *event)
{
    kywc_log(KYWC_DEBUG, "XCB_SELECTION_NOTIFY (selection=%u, property=%u, target=%u)",
             event->selection, event->property, event->target);

    struct xwm_selection *selection = xwm_get_selection(xwm, event->selection);
    if (selection == NULL) {
        return;
    }

    struct xwm_selection_transfer *transfer =
        xwm_selection_find_incoming_transfer_by_window(selection, event->requestor);

    if (event->property == XCB_ATOM_NONE) {
        if (transfer) {
            kywc_log(KYWC_ERROR, "convert selection failed");
            xwm_selection_transfer_destroy(transfer);
        }
    } else if (event->target == xwm->atoms[TARGETS].atom) {
        // No xwayland surface focused, deny access to clipboard
        if (xwm->focus_surface == NULL) {
            kywc_log(KYWC_DEBUG, "denying write access to clipboard: "
                                 "no xwayland surface focused");
            return;
        }

        // This sets the Wayland clipboard (by calling wlr_seat_set_selection)
        xwm_selection_get_targets(selection);
    } else if (transfer) {
        xwm_selection_transfer_get_data(transfer);
    }
}

static bool xwm_start_drag_x11(struct xwm *xwm, struct x11_data_source *source)
{
    if (seat_is_dragging(xwm->seat)) {
        return false;
    }

    struct seat *seat = xwm->seat;
    struct wlr_surface *current_surface =
        seat->cursor->hover.node ? wlr_surface_try_from_node(seat->cursor->hover.node) : NULL;
    struct view *current_view = current_surface ? view_try_from_wlr_surface(current_surface) : NULL;
    if (!current_view || !xwayland_check_view(current_view)) {
        kywc_log(KYWC_ERROR, "start drag x11 not hover xwayland view");
        return false;
    }

    struct wl_client *wl_client = xwayland_get_client();
    if (!wl_client) {
        return false;
    }
    struct wlr_seat_client *wlr_seat_client =
        wlr_seat_client_for_wl_client(seat->wlr_seat, wl_client);
    if (!wlr_seat_client) {
        return false;
    }
    // Checked before drag_create: on denial the source must be left to the
    // caller (xwm_handle_xfixes_selection_notify frees it), and no drag is
    // created. Destroying the source here would double-free it.
    if (security_check_selection(seat) && !data_source_allow_start_drag(seat, &source->base)) {
        return false;
    }

    struct drag *drag = drag_create(wlr_seat_client, &source->base, NULL, true);
    if (!drag) {
        return false;
    }

    // fallback to hover surface
    if (!source->view) {
        source->view = current_view;
    }

    drag->grab_type = DRAG_GRAB_KEYBOARD_POINTER;
    wlr_seat_pointer_start_grab(seat->wlr_seat, &drag->pointer_grab);
    seat_start_drag(seat, drag, 0);

    xwm_map_dnd_selection_window(xwm, true);
    kywc_log(KYWC_DEBUG, "start drag X11");

    return true;
}

int xwm_handle_xfixes_selection_notify(struct xwm *xwm, xcb_xfixes_selection_notify_event_t *event)
{
    kywc_log(KYWC_DEBUG, "XCB_XFIXES_SELECTION_NOTIFY (selection=%u, owner=%u)", event->selection,
             event->owner);

    struct seat *seat = xwm->seat;
    if (!seat) {
        return 0;
    }

    struct xwm_selection *selection = xwm_get_selection(xwm, event->selection);
    if (selection == NULL) {
        return 0;
    }

    if (event->owner == XCB_WINDOW_NONE) {
        if (selection->owner != selection->window) {
            // A real X client selection went away, not our proxy selection
            if (selection == &xwm->clipboard_selection) {
                seat_request_set_selection(seat, NULL, NULL, wl_display_next_serial(xwm->display));
            } else if (selection == &xwm->primary_selection) {
                seat_request_set_primary_selection(seat, NULL, NULL,
                                                   wl_display_next_serial(xwm->display));
            } else if (selection == &xwm->dnd_selection) {
                // end drag x11
                if (xwm->drag && xwm->drag->drag_x11) {
                    data_source_destroy(xwm->drag->source);
                }
            } else {
                kywc_log(KYWC_DEBUG, "X11 selection has been cleared, but cannot "
                                     "clear Wayland selection");
            }
        }

        selection->owner = XCB_WINDOW_NONE;
        return 1;
    }

    if (selection->owner == selection->window && event->owner != selection->owner) {
        kywc_log(KYWC_DEBUG, "proxy window lost selection ownership");
    }

    selection->owner = event->owner;

    if (selection->owner == selection->window) {
        // We have to use XCB_TIME_CURRENT_TIME when we claim the selection, so
        // grab the actual timestamp here so we can answer TIMESTAMP conversion
        // requests correctly.
        selection->timestamp = event->timestamp;
        return 1;
    }

    if (selection == &xwm->dnd_selection) {
        // start drag x11
        struct x11_data_source *source = calloc(1, sizeof(*source));
        if (!source) {
            return 0;
        }
        source->selection = selection;
        data_source_init(&source->base, &data_source_impl);

        struct xwayland_surface *xsurface = xwm_lookup_surface(xwm, event->owner);
        struct wlr_surface *wlr_surface = xsurface ? xsurface->surface : NULL;
        source->view = wlr_surface ? view_try_from_wlr_surface(wlr_surface) : NULL;
        wl_array_init(&source->mime_types_atoms);

        // TODO: maybe touch/tablet drag, we don't know what started drag
        if (!xwm_start_drag_x11(xwm, source)) {
            wl_array_release(&source->mime_types_atoms);
            free(source);
            kywc_log(KYWC_INFO, "xwm start drag x11 failed!");
        }

        return 1;
    }

    // doing this will give a selection notify where we actually handle the sync
    xcb_convert_selection(xwm->xcb_conn, selection->window, selection->atom,
                          xwm->atoms[TARGETS].atom, xwm->atoms[WL_SELECTION].atom,
                          event->timestamp);
    xcb_flush(xwm->xcb_conn);

    return 1;
}
