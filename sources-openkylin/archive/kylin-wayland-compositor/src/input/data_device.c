// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _XOPEN_SOURCE 700 // for ffs()
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include <wlr/types/wlr_compositor.h>

#include <kywc/log.h>

#include "input/data_device.h"
#include "input/seat.h"
#include "input_p.h"
#include "security/security.h"
#include "util/client.h"
#include "xwayland/xwayland.h"

#define DATA_DEVICE_MANAGER_VERSION 3

#define DATA_DEVICE_ALL_ACTIONS                                                                    \
    (WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY | WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE |             \
     WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK)

struct data_device_manager {
    struct wl_global *global;
    struct wl_listener display_destroy;
};

struct client_data_source {
    struct data_source source;
    struct data_source_impl impl;
    struct wl_resource *resource;
    bool finalized;
};

enum data_offer_type {
    DATA_OFFER_SELECTION,
    DATA_OFFER_DRAG,
};

struct data_offer {
    struct wl_resource *resource;
    struct wl_list link; // wlr_seat.{selection_offers,drag_offers}
    struct seat *seat;
    struct data_source *source;
    struct wl_listener source_destroy;
    enum data_offer_type type;

    uint32_t actions;
    enum wl_data_device_manager_dnd_action preferred_action;
    bool in_ask;
};

static const struct wl_data_source_interface data_source_impl;

static struct client_data_source *client_data_source_from_resource(struct wl_resource *resource)
{
    assert(wl_resource_instance_of(resource, &wl_data_source_interface, &data_source_impl));
    return wl_resource_get_user_data(resource);
}

static const struct wl_data_device_interface data_device_impl;

static struct data_device_client *data_device_client_from_resource(struct wl_resource *resource)
{
    assert(wl_resource_instance_of(resource, &wl_data_device_interface, &data_device_impl));
    return wl_resource_get_user_data(resource);
}

struct data_device_client *data_device_client_from_seat_client(struct wlr_seat_client *seat_client)
{
    struct wl_list *list = &seat_client->data_devices;
    if (list->prev == list) {
        return NULL;
    }
    return (struct data_device_client *)list->prev;
}

static const struct wl_data_offer_interface data_offer_impl;

static struct data_offer *data_offer_from_resource(struct wl_resource *resource)
{
    assert(wl_resource_instance_of(resource, &wl_data_offer_interface, &data_offer_impl));
    return wl_resource_get_user_data(resource);
}

void data_source_init(struct data_source *source, const struct data_source_impl *impl)
{
    assert(impl->send);

    *source = (struct data_source){
        .impl = impl,
        .actions = -1,
    };

    wl_array_init(&source->mime_types);
    wl_signal_init(&source->events.accepted);
    wl_signal_init(&source->events.dnd_action);
    wl_signal_init(&source->events.destroy);

    if (impl->client) {
        impl->client(source, &source->client);
    }
}

void data_source_accept(struct data_source *source, uint32_t serial, const char *mime_type)
{
    source->accepted = (mime_type && *mime_type != '\0');
    wl_signal_emit_mutable(&source->events.accepted, NULL);

    if (source->impl->accept) {
        source->impl->accept(source, serial, mime_type);
    }
}

void data_source_send(struct data_source *source, const char *mime_type, int32_t fd)
{
    source->impl->send(source, mime_type, fd);
}

static void client_from_surface(struct client *client, struct wlr_surface *surface)
{
    pid_t pid = xwayland_get_pid_from_wlr_surface(surface);
    if (pid != -1) {
        client->type = CLIENT_TYPE_X11;
        client->pid = pid;
    } else {
        client->type = CLIENT_TYPE_WAYLAND;
        wl_client_get_credentials(wl_resource_get_client(surface->resource), &client->pid,
                                  &client->uid, &client->gid);
    }
    client_set_comm(client);
}

bool data_source_allow_read(struct seat *seat, struct data_source *source,
                            struct client *destination)
{
    struct security_action_info info = {
        .action = SECURITY_READ_SELECTION,
        .selection = {
            .seat = seat,
            .source = &source->client,
            .destination = destination,
            .mime_types = &source->mime_types,
        },
    };

    return security_access(&info);
}

bool data_source_allow_drop(struct seat *seat, struct data_source *source,
                            struct client *destination)
{
    struct security_action_info info = {
        .action = SECURITY_DROP,
        .dnd = {
            .seat = seat,
            .source = &source->client,
            .destination = destination,
            .mime_types = &source->mime_types,
        },
    };

    return security_access(&info);
}

bool data_source_allow_write(struct seat *seat, struct data_source *source)
{
    if (!source) {
        return true;
    }

    struct security_action_info info = {
        .action = SECURITY_WRITE_SELECTION,
        .selection = {
            .seat = seat,
            .source = &source->client,
            .mime_types = &source->mime_types,
        },
    };

    return security_access(&info);
}

bool data_source_allow_start_drag(struct seat *seat, struct data_source *source)
{
    if (!source) {
        return true;
    }

    struct security_action_info info = {
        .action = SECURITY_START_DRAG,
        .dnd = {
            .seat = seat,
            .source = &source->client,
            .mime_types = &source->mime_types,
        },
    };

    return security_access(&info);
}

void data_source_dnd_drop(struct data_source *source)
{
    if (source->impl->dnd_drop) {
        source->impl->dnd_drop(source);
    }
}

void data_source_dnd_finish(struct data_source *source)
{
    if (source->impl->dnd_finish) {
        source->impl->dnd_finish(source);
    }
}

void data_source_dnd_action(struct data_source *source,
                            enum wl_data_device_manager_dnd_action action)
{
    source->current_dnd_action = action;
    wl_signal_emit_mutable(&source->events.dnd_action, NULL);

    if (source->impl->dnd_action) {
        source->impl->dnd_action(source, action);
    }
}

static void data_source_handle_offer(struct wl_client *client, struct wl_resource *resource,
                                     const char *mime_type)
{
    struct client_data_source *source = client_data_source_from_resource(resource);
    if (!source) {
        return;
    }

    if (source->finalized) {
        kywc_log(KYWC_DEBUG, "Offering additional MIME type after wl_data_device.set_selection");
    }

    const char **mime_type_ptr;
    wl_array_for_each(mime_type_ptr, &source->source.mime_types) {
        if (strcmp(*mime_type_ptr, mime_type) == 0) {
            kywc_log(KYWC_DEBUG, "Ignoring duplicate MIME type offer %s", mime_type);
            return;
        }
    }

    char *dup_mime_type = strdup(mime_type);
    if (!dup_mime_type) {
        wl_resource_post_no_memory(resource);
        return;
    }

    char **p = wl_array_add(&source->source.mime_types, sizeof(*p));
    if (!p) {
        free(dup_mime_type);
        wl_resource_post_no_memory(resource);
        return;
    }

    *p = dup_mime_type;
}

static void data_source_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static void data_source_handle_set_actions(struct wl_client *client, struct wl_resource *resource,
                                           uint32_t dnd_actions)
{
    struct client_data_source *source = client_data_source_from_resource(resource);
    if (!source) {
        return;
    }

    if (source->source.actions >= 0) {
        wl_resource_post_error(source->resource, WL_DATA_SOURCE_ERROR_INVALID_ACTION_MASK,
                               "cannot set actions more than once");
        return;
    }

    if (dnd_actions & ~DATA_DEVICE_ALL_ACTIONS) {
        wl_resource_post_error(source->resource, WL_DATA_SOURCE_ERROR_INVALID_ACTION_MASK,
                               "invalid action mask %x", dnd_actions);
        return;
    }

    if (source->finalized) {
        wl_resource_post_error(source->resource, WL_DATA_SOURCE_ERROR_INVALID_ACTION_MASK,
                               "invalid action change after wl_data_device.start_drag");
        return;
    }

    source->source.actions = dnd_actions;
}

static const struct wl_data_source_interface data_source_impl = {
    .offer = data_source_handle_offer,
    .destroy = data_source_handle_destroy,
    .set_actions = data_source_handle_set_actions,
};

void data_source_destroy(struct data_source *source)
{
    if (!source) {
        return;
    }

    wl_signal_emit_mutable(&source->events.destroy, source);
    assert(wl_list_empty(&source->events.accepted.listener_list));
    assert(wl_list_empty(&source->events.dnd_action.listener_list));
    assert(wl_list_empty(&source->events.destroy.listener_list));

    char **p;
    wl_array_for_each(p, &source->mime_types) {
        free(*p);
    }
    wl_array_release(&source->mime_types);

    if (source->impl->destroy) {
        source->impl->destroy(source);
    } else {
        free(source);
    }
}

static void data_source_handle_resource_destroy(struct wl_resource *resource)
{
    struct client_data_source *source = client_data_source_from_resource(resource);
    if (source) {
        data_source_destroy(&source->source);
    }
}

static void client_data_source_accept(struct data_source *data_source, uint32_t serial,
                                      const char *mime_type);

static struct client_data_source *
client_data_source_from_data_source(struct data_source *data_source)
{
    assert(data_source->impl->accept == client_data_source_accept);
    struct client_data_source *source = wl_container_of(data_source, source, source);
    return source;
}

static void client_data_source_accept(struct data_source *data_source, uint32_t serial,
                                      const char *mime_type)
{
    struct client_data_source *source = client_data_source_from_data_source(data_source);
    wl_data_source_send_target(source->resource, mime_type);
}

static void client_data_source_send(struct data_source *data_source, const char *mime_type,
                                    int32_t fd)
{
    struct client_data_source *source = client_data_source_from_data_source(data_source);
    wl_data_source_send_send(source->resource, mime_type, fd);
    close(fd);
}

static void client_data_source_destroy(struct data_source *data_source)
{
    struct client_data_source *source = client_data_source_from_data_source(data_source);
    wl_data_source_send_cancelled(source->resource);
    wl_resource_set_user_data(source->resource, NULL);
    free(source);
}

static void client_data_source_dnd_drop(struct data_source *data_source)
{
    struct client_data_source *source = client_data_source_from_data_source(data_source);
    assert(wl_resource_get_version(source->resource) >=
           WL_DATA_SOURCE_DND_DROP_PERFORMED_SINCE_VERSION);
    wl_data_source_send_dnd_drop_performed(source->resource);
}

static void client_data_source_dnd_finish(struct data_source *data_source)
{
    struct client_data_source *source = client_data_source_from_data_source(data_source);
    assert(wl_resource_get_version(source->resource) >= WL_DATA_SOURCE_DND_FINISHED_SINCE_VERSION);
    wl_data_source_send_dnd_finished(source->resource);
}

static void client_data_source_dnd_action(struct data_source *data_source,
                                          enum wl_data_device_manager_dnd_action action)
{
    struct client_data_source *source = client_data_source_from_data_source(data_source);
    assert(wl_resource_get_version(source->resource) >= WL_DATA_SOURCE_ACTION_SINCE_VERSION);
    wl_data_source_send_action(source->resource, action);
}

static void client_data_source_client(struct data_source *data_source, struct client *client)
{
    struct client_data_source *source = client_data_source_from_data_source(data_source);
    client->type = CLIENT_TYPE_WAYLAND;
    wl_client_get_credentials(wl_resource_get_client(source->resource), &client->pid, &client->uid,
                              &client->gid);
    client_set_comm(client);
}

static struct client_data_source *client_data_source_create(struct wl_client *client,
                                                            uint32_t version, uint32_t id)
{
    struct client_data_source *source = calloc(1, sizeof(*source));
    if (!source) {
        return NULL;
    }

    source->resource = wl_resource_create(client, &wl_data_source_interface, version, id);
    if (!source->resource) {
        wl_resource_post_no_memory(source->resource);
        free(source);
        return NULL;
    }

    wl_resource_set_implementation(source->resource, &data_source_impl, source,
                                   data_source_handle_resource_destroy);

    source->impl.accept = client_data_source_accept;
    source->impl.send = client_data_source_send;
    source->impl.destroy = client_data_source_destroy;
    source->impl.client = client_data_source_client;

    if (wl_resource_get_version(source->resource) >=
        WL_DATA_SOURCE_DND_DROP_PERFORMED_SINCE_VERSION) {
        source->impl.dnd_drop = client_data_source_dnd_drop;
    }
    if (wl_resource_get_version(source->resource) >= WL_DATA_SOURCE_DND_FINISHED_SINCE_VERSION) {
        source->impl.dnd_finish = client_data_source_dnd_finish;
    }
    if (wl_resource_get_version(source->resource) >= WL_DATA_SOURCE_ACTION_SINCE_VERSION) {
        source->impl.dnd_action = client_data_source_dnd_action;
    }

    data_source_init(&source->source, &source->impl);
    return source;
}

static void data_device_manager_create_data_source(struct wl_client *client,
                                                   struct wl_resource *manager_resource,
                                                   uint32_t id)
{
    int version = wl_resource_get_version(manager_resource);
    client_data_source_create(client, version, id);
}

static const struct wlr_surface_role drag_icon_surface_role;

static void drag_icon_surface_role_commit(struct wlr_surface *surface)
{
    assert(surface->role == &drag_icon_surface_role);

    pixman_region32_clear(&surface->input_region);
    if (wlr_surface_has_buffer(surface)) {
        wlr_surface_map(surface);
    }
}

static const struct wlr_surface_role drag_icon_surface_role = {
    .name = "wl_data_device-icon",
    .no_object = true,
    .commit = drag_icon_surface_role_commit,
};

static void data_offer_source_dnd_finish(struct data_offer *offer)
{
    struct data_source *source = offer->source;
    if (source->actions < 0) {
        return;
    }

    if (offer->in_ask) {
        data_source_dnd_action(source, source->current_dnd_action);
    }

    data_source_dnd_finish(source);
}

static void data_offer_destroy(struct data_offer *offer)
{
    if (!offer) {
        return;
    }

    wl_list_remove(&offer->source_destroy.link);
    wl_list_remove(&offer->link);

    if (offer->type == DATA_OFFER_DRAG && offer->source) {
        // If the drag destination has version < 3, wl_data_offer.finish
        // won't be called, so do this here as a safety net, because
        // we still want the version >= 3 drag source to be happy.
        if (wl_resource_get_version(offer->resource) < WL_DATA_OFFER_ACTION_SINCE_VERSION) {
            data_offer_source_dnd_finish(offer);
        } else if (offer->source->impl->dnd_finish) {
            data_source_destroy(offer->source);
        }
    }

    // Make the resource inert
    wl_resource_set_user_data(offer->resource, NULL);

    free(offer);
}

static void data_offer_handle_accept(struct wl_client *client, struct wl_resource *resource,
                                     uint32_t serial, const char *mime_type)
{
    struct data_offer *offer = data_offer_from_resource(resource);
    if (!offer) {
        return;
    }

    if (offer->type != DATA_OFFER_DRAG) {
        kywc_log(KYWC_DEBUG, "Ignoring wl_data_offer.accept request on a non-drag-and-drop offer");
        return;
    }

    data_source_accept(offer->source, serial, mime_type);
}

static void data_offer_handle_receive(struct wl_client *client, struct wl_resource *resource,
                                      const char *mime_type, int32_t fd)
{
    struct data_offer *offer = data_offer_from_resource(resource);
    if (!offer) {
        close(fd);
        return;
    }

    if (security_check_selection(offer->seat)) {
        struct client reader = { .type = CLIENT_TYPE_WAYLAND };
        wl_client_get_credentials(client, &reader.pid, &reader.uid, &reader.gid);
        client_set_comm(&reader);
        if (offer->type == DATA_OFFER_SELECTION) {
            if (!data_source_allow_read(offer->seat, offer->source, &reader)) {
                close(fd);
                return;
            }
        } else if (!data_source_allow_drop(offer->seat, offer->source, &reader)) {
            close(fd);
            return;
        }
    }

    data_source_send(offer->source, mime_type, fd);
}

static void data_offer_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static void data_offer_handle_finish(struct wl_client *client, struct wl_resource *resource)
{
    struct data_offer *offer = data_offer_from_resource(resource);
    if (!offer) {
        return;
    }

    // TODO: also fail while we have a drag-and-drop grab
    if (offer->type != DATA_OFFER_DRAG) {
        wl_resource_post_error(offer->resource, WL_DATA_OFFER_ERROR_INVALID_FINISH,
                               "Offer is not drag-and-drop");
        return;
    }
    if (!offer->source->accepted) {
        wl_resource_post_error(offer->resource, WL_DATA_OFFER_ERROR_INVALID_FINISH,
                               "Premature finish request");
        return;
    }
    enum wl_data_device_manager_dnd_action action = offer->source->current_dnd_action;
    if (action == WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE ||
        action == WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK) {
        wl_resource_post_error(offer->resource, WL_DATA_OFFER_ERROR_INVALID_FINISH,
                               "Offer finished with an invalid action");
        return;
    }

    data_offer_source_dnd_finish(offer);
    data_offer_destroy(offer);
}

static uint32_t data_offer_choose_action(struct data_offer *offer)
{
    uint32_t offer_actions, preferred_action = 0;
    if (wl_resource_get_version(offer->resource) >= WL_DATA_OFFER_ACTION_SINCE_VERSION) {
        offer_actions = offer->actions;
        preferred_action = offer->preferred_action;
    } else {
        offer_actions = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
    }

    uint32_t source_actions;
    if (offer->source->actions >= 0) {
        source_actions = offer->source->actions;
    } else {
        source_actions = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
    }

    uint32_t available_actions = offer_actions & source_actions;
    if (!available_actions) {
        return WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
    }

    if (offer->source->compositor_action & available_actions) {
        return offer->source->compositor_action;
    }

    // If the dest side has a preferred DnD action, use it
    if ((preferred_action & available_actions) != 0) {
        return preferred_action;
    }

    // Use the first found action, in bit order
    return 1 << (ffs(available_actions) - 1);
}

static void data_offer_update_action(struct data_offer *offer)
{
    assert(offer->type == DATA_OFFER_DRAG);

    uint32_t action = data_offer_choose_action(offer);
    if (offer->source->current_dnd_action == action) {
        return;
    }
    offer->source->current_dnd_action = action;

    if (offer->in_ask) {
        return;
    }

    data_source_dnd_action(offer->source, action);

    if (wl_resource_get_version(offer->resource) >= WL_DATA_OFFER_ACTION_SINCE_VERSION) {
        wl_data_offer_send_action(offer->resource, action);
    }
}

static void data_offer_handle_set_actions(struct wl_client *client, struct wl_resource *resource,
                                          uint32_t actions, uint32_t preferred_action)
{
    struct data_offer *offer = data_offer_from_resource(resource);
    if (!offer) {
        return;
    }

    if (actions & ~DATA_DEVICE_ALL_ACTIONS) {
        wl_resource_post_error(offer->resource, WL_DATA_OFFER_ERROR_INVALID_ACTION_MASK,
                               "invalid action mask %x", actions);
        return;
    }

    if (preferred_action &&
        (!(preferred_action & actions) || __builtin_popcount(preferred_action) > 1)) {
        wl_resource_post_error(offer->resource, WL_DATA_OFFER_ERROR_INVALID_ACTION,
                               "invalid action %x", preferred_action);
        return;
    }

    if (offer->type != DATA_OFFER_DRAG) {
        wl_resource_post_error(offer->resource, WL_DATA_OFFER_ERROR_INVALID_OFFER,
                               "set_action can only be sent to drag-and-drop offers");
        return;
    }

    offer->actions = actions;
    offer->preferred_action = preferred_action;

    data_offer_update_action(offer);
}

static const struct wl_data_offer_interface data_offer_impl = {
    .accept = data_offer_handle_accept,
    .receive = data_offer_handle_receive,
    .destroy = data_offer_handle_destroy,
    .finish = data_offer_handle_finish,
    .set_actions = data_offer_handle_set_actions,
};

static void data_offer_handle_resource_destroy(struct wl_resource *resource)
{
    struct data_offer *offer = data_offer_from_resource(resource);
    data_offer_destroy(offer);
}

static void data_offer_handle_source_destroy(struct wl_listener *listener, void *data)
{
    struct data_offer *offer = wl_container_of(listener, offer, source_destroy);
    // Prevent data_offer_destroy from destroying the source again
    offer->source = NULL;
    data_offer_destroy(offer);
}

static void drag_set_focus(struct drag *drag, struct wlr_surface *surface, double sx, double sy);

static void drag_handle_focus_destroy(struct wl_listener *listener, void *data)
{
    struct drag *drag = wl_container_of(listener, drag, focus_destroy);
    drag_set_focus(drag, NULL, 0, 0);
}

static void drag_handle_denied_focus_destroy(struct wl_listener *listener, void *data)
{
    struct drag *drag = wl_container_of(listener, drag, denied_focus_destroy);
    wl_list_remove(&drag->denied_focus_destroy.link);
    wl_list_init(&drag->denied_focus_destroy.link);
    drag->denied_focus = NULL;
}

static struct data_offer *data_offer_create(struct wl_resource *device_resource,
                                            struct data_source *source, enum data_offer_type type)
{
    struct data_device_client *client = data_device_client_from_resource(device_resource);
    assert(client != NULL);
    assert(source != NULL); // a NULL source means no selection

    struct data_offer *offer = calloc(1, sizeof(*offer));
    if (!offer) {
        return NULL;
    }

    offer->source = source;
    offer->type = type;

    struct wl_client *wl_client = wl_resource_get_client(device_resource);
    uint32_t version = wl_resource_get_version(device_resource);
    offer->resource = wl_resource_create(wl_client, &wl_data_offer_interface, version, 0);
    if (!offer->resource) {
        free(offer);
        return NULL;
    }

    wl_resource_set_implementation(offer->resource, &data_offer_impl, offer,
                                   data_offer_handle_resource_destroy);

    struct seat *seat = seat_from_wlr_seat(client->seat_client->seat);
    offer->seat = seat;
    switch (type) {
    case DATA_OFFER_SELECTION:
        wl_list_insert(&seat->selection_offers, &offer->link);
        break;
    case DATA_OFFER_DRAG:
        wl_list_insert(&seat->drag_offers, &offer->link);
        break;
    }

    offer->source_destroy.notify = data_offer_handle_source_destroy;
    wl_signal_add(&source->events.destroy, &offer->source_destroy);

    wl_data_device_send_data_offer(device_resource, offer->resource);

    char **p;
    wl_array_for_each(p, &source->mime_types) {
        wl_data_offer_send_offer(offer->resource, *p);
    }

    return offer;
}

static void drag_handle_focus_client_destroy(struct wl_listener *listener, void *data)
{
    struct drag *drag = wl_container_of(listener, drag, focus_client_destroy);
    wl_list_remove(&drag->focus_client_destroy.link);
    drag->focus_client = NULL;
}

static void drag_set_focus(struct drag *drag, struct wlr_surface *surface, double sx, double sy)
{
    if (surface && (drag->focus == surface || drag->denied_focus == surface)) {
        return;
    }

    if (!surface && !drag->focus && !drag->denied_focus) {
        return;
    }

    if (drag->focus_client) {
        wl_list_remove(&drag->focus_client_destroy.link);

        // If we're switching focus to another client, we want to destroy all
        // offers without destroying the source. If the drag operation ends, we
        // want to keep the offer around for the data transfer.
        struct seat *seat = seat_from_wlr_seat(drag->focus_client->seat);
        struct data_offer *offer, *tmp;
        wl_list_for_each_safe(offer, tmp, &seat->drag_offers, link) {
            struct wl_client *client = wl_resource_get_client(offer->resource);
            if (!drag->dropped && offer->source == drag->source &&
                client == drag->focus_client->client) {
                offer->source = NULL;
                data_offer_destroy(offer);
            }
        }

        struct data_device_client *client = data_device_client_from_seat_client(drag->focus_client);
        if (client) {
            struct wl_resource *resource;
            wl_resource_for_each(resource, &client->data_devices) {
                wl_data_device_send_leave(resource);
            }
        }

        drag->focus_client = NULL;
    }

    wl_list_remove(&drag->focus_destroy.link);
    wl_list_init(&drag->focus_destroy.link);
    drag->focus = NULL;

    wl_list_remove(&drag->denied_focus_destroy.link);
    wl_list_init(&drag->denied_focus_destroy.link);
    drag->denied_focus = NULL;

    if (!surface) {
        goto out;
    }

    if (!drag->source && drag->seat_client &&
        wl_resource_get_client(surface->resource) != drag->seat_client->client) {
        goto out;
    }

    struct wlr_seat_client *focus_client = wlr_seat_client_for_wl_client(
        drag->seat->wlr_seat, wl_resource_get_client(surface->resource));
    if (!focus_client) {
        goto out;
    }

    if (drag->source) {
        if (security_check_selection(drag->seat)) {
            struct client destination;
            client_from_surface(&destination, surface);
            if (!data_source_allow_drop(drag->seat, drag->source, &destination)) {
                // the client under the cursor is not allowed to receive the
                // drop; remember the denial so repeated pointer motions over
                // this surface do not re-run the security check
                drag->denied_focus = surface;
                drag->denied_focus_destroy.notify = drag_handle_denied_focus_destroy;
                wl_signal_add(&surface->events.destroy, &drag->denied_focus_destroy);
                goto out;
            }
        }

        drag->source->accepted = false;

        uint32_t serial = wl_display_next_serial(drag->seat->wlr_seat->display);
        struct data_device_client *client = data_device_client_from_seat_client(focus_client);

        if (client) {
            struct wl_resource *device_resource;
            wl_resource_for_each(device_resource, &client->data_devices) {
                struct data_offer *offer =
                    data_offer_create(device_resource, drag->source, DATA_OFFER_DRAG);
                if (!offer) {
                    wl_resource_post_no_memory(device_resource);
                    return;
                }

                data_offer_update_action(offer);

                if (wl_resource_get_version(offer->resource) >=
                    WL_DATA_OFFER_SOURCE_ACTIONS_SINCE_VERSION) {
                    wl_data_offer_send_source_actions(offer->resource, drag->source->actions);
                }

                wl_data_device_send_enter(device_resource, serial, surface->resource,
                                          wl_fixed_from_double(sx), wl_fixed_from_double(sy),
                                          offer->resource);
            }
        }
    }

    drag->focus = surface;
    drag->focus_client = focus_client;
    drag->focus_destroy.notify = drag_handle_focus_destroy;
    wl_signal_add(&surface->events.destroy, &drag->focus_destroy);
    drag->focus_client_destroy.notify = drag_handle_focus_client_destroy;
    wl_signal_add(&focus_client->events.destroy, &drag->focus_client_destroy);

    wlr_seat_keyboard_notify_modifiers(
        drag->seat->wlr_seat, drag->seat->wlr_seat->keyboard_state.keyboard
                                  ? &drag->seat->wlr_seat->keyboard_state.keyboard->modifiers
                                  : NULL);

out:
    wl_signal_emit_mutable(&drag->events.focus, drag);
}

static void drag_icon_destroy(struct drag_icon *icon)
{
    icon->drag->icon = NULL;
    wl_list_remove(&icon->surface_destroy.link);
    wl_signal_emit_mutable(&icon->events.destroy, icon);
    assert(wl_list_empty(&icon->events.destroy.listener_list));

    free(icon);
}

static void drag_destroy(struct drag *drag)
{
    if (drag->cancelling) {
        return;
    }
    drag->cancelling = true;

    if (drag->started) {
        wlr_seat_keyboard_end_grab(drag->seat->wlr_seat);
        switch (drag->grab_type) {
        case DRAG_GRAB_KEYBOARD:
            break;
        case DRAG_GRAB_KEYBOARD_POINTER:
            wlr_seat_pointer_end_grab(drag->seat->wlr_seat);
            break;
        case DRAG_GRAB_KEYBOARD_TOUCH:
            wlr_seat_touch_end_grab(drag->seat->wlr_seat);
            break;
        }
    }

    if (drag->started) {
        drag_set_focus(drag, NULL, 0, 0);

        assert(drag->seat->drag == drag);
        drag->seat->drag = NULL;
    }

    // We issue destroy after ending the grab to allow focus changes.
    // Furthermore, we wait until after clearing the drag focus in order
    // to ensure that the wl_data_device.leave is sent before emitting the
    // signal. This allows e.g. wl_pointer.enter to be sent in the destroy
    // signal handler.
    wl_signal_emit_mutable(&drag->events.destroy, drag);

    assert(wl_list_empty(&drag->events.focus.listener_list));
    assert(wl_list_empty(&drag->events.motion.listener_list));
    assert(wl_list_empty(&drag->events.drop.listener_list));
    assert(wl_list_empty(&drag->events.destroy.listener_list));

    if (drag->source) {
        wl_list_remove(&drag->source_destroy.link);
    }
    wl_list_remove(&drag->focus_destroy.link);
    wl_list_remove(&drag->seat_client_destroy.link);

    if (drag->icon) {
        drag_icon_destroy(drag->icon);
    }
    free(drag);
}

static void drag_icon_handle_surface_destroy(struct wl_listener *listener, void *data)
{
    struct drag_icon *icon = wl_container_of(listener, icon, surface_destroy);
    drag_icon_destroy(icon);
}

static struct drag_icon *drag_icon_create(struct drag *drag, struct wlr_surface *surface)
{
    struct drag_icon *icon = calloc(1, sizeof(*icon));
    if (!icon) {
        return NULL;
    }

    icon->drag = drag;
    icon->surface = surface;

    wl_signal_init(&icon->events.destroy);

    icon->surface_destroy.notify = drag_icon_handle_surface_destroy;
    wl_signal_add(&surface->events.destroy, &icon->surface_destroy);

    drag_icon_surface_role_commit(surface);

    return icon;
}

static void drag_handle_icon_destroy(struct wl_listener *listener, void *data)
{
    struct drag *drag = wl_container_of(listener, drag, icon_destroy);
    drag->icon = NULL;
    wl_list_remove(&drag->icon_destroy.link);
}

static void drag_handle_drag_source_destroy(struct wl_listener *listener, void *data)
{
    struct drag *drag = wl_container_of(listener, drag, source_destroy);
    drag_destroy(drag);
}

static void drag_handle_pointer_enter(struct wlr_seat_pointer_grab *grab,
                                      struct wlr_surface *surface, double sx, double sy)
{
    struct drag *drag = grab->data;
    drag_set_focus(drag, surface, sx, sy);
}

static void drag_handle_pointer_clear_focus(struct wlr_seat_pointer_grab *grab)
{
    struct drag *drag = grab->data;
    drag_set_focus(drag, NULL, 0, 0);
}

static void drag_handle_pointer_motion(struct wlr_seat_pointer_grab *grab, uint32_t time, double sx,
                                       double sy)
{
    struct drag *drag = grab->data;
    if (drag->focus && drag->focus_client) {
        struct data_device_client *client = data_device_client_from_seat_client(drag->focus_client);
        if (client) {
            struct wl_resource *resource;
            wl_resource_for_each(resource, &client->data_devices) {
                wl_data_device_send_motion(resource, time, wl_fixed_from_double(sx),
                                           wl_fixed_from_double(sy));
            }
        }

        struct drag_motion_event event = {
            .drag = drag,
            .time = time,
            .sx = sx,
            .sy = sy,
        };
        wl_signal_emit_mutable(&drag->events.motion, &event);
    }
}

static void drag_drop(struct drag *drag, uint32_t time)
{
    assert(drag->focus_client);

    drag->dropped = true;

    struct data_device_client *client = data_device_client_from_seat_client(drag->focus_client);
    if (client) {
        struct wl_resource *resource;
        wl_resource_for_each(resource, &client->data_devices) {
            wl_data_device_send_drop(resource);
        }
    }
    if (drag->source) {
        data_source_dnd_drop(drag->source);
    }

    struct drag_drop_event event = {
        .drag = drag,
        .time = time,
    };
    wl_signal_emit_mutable(&drag->events.drop, &event);
}

static uint32_t drag_handle_pointer_button(struct wlr_seat_pointer_grab *grab, uint32_t time,
                                           uint32_t button, uint32_t state)
{
    struct drag *drag = grab->data;

    if (drag->source && grab->seat->pointer_state.grab_button == button &&
        state == WL_POINTER_BUTTON_STATE_RELEASED) {
        if (drag->focus_client && drag->source->current_dnd_action && drag->source->accepted) {
            drag_drop(drag, time);
        } else {
            if (drag->focus_client != drag->seat_client) {
                data_source_dnd_drop(drag->source);
            }
            if (drag->source->impl->dnd_finish) {
                // This will end the grab and free `drag`
                data_source_destroy(drag->source);
                return 0;
            }
        }
    }

    if (grab->seat->pointer_state.button_count == 0 && state == WL_POINTER_BUTTON_STATE_RELEASED) {
        drag_destroy(drag);
    }

    return 0;
}

static void drag_handle_pointer_axis(struct wlr_seat_pointer_grab *grab, uint32_t time_msec,
                                     enum wlr_axis_orientation orientation, double value,
                                     int32_t value_discrete, enum wlr_axis_source source)
{
    // This space is intentionally left blank
}

static void drag_handle_pointer_cancel(struct wlr_seat_pointer_grab *grab)
{
    struct drag *drag = grab->data;
    drag_destroy(drag);
}

static const struct wlr_pointer_grab_interface data_device_pointer_drag_interface = {
    .enter = drag_handle_pointer_enter,
    .clear_focus = drag_handle_pointer_clear_focus,
    .motion = drag_handle_pointer_motion,
    .button = drag_handle_pointer_button,
    .axis = drag_handle_pointer_axis,
    .cancel = drag_handle_pointer_cancel,
};

static uint32_t drag_handle_touch_down(struct wlr_seat_touch_grab *grab, uint32_t time,
                                       struct wlr_touch_point *point)
{
    // eat the event
    return 0;
}

static void drag_handle_touch_up(struct wlr_seat_touch_grab *grab, uint32_t time,
                                 struct wlr_touch_point *point)
{
    struct drag *drag = grab->data;
    if (drag->grab_touch_id != point->touch_id) {
        return;
    }

    if (drag->focus_client && drag->source->current_dnd_action && drag->source->accepted) {
        drag_drop(drag, time);
    } else {
        if (drag->focus_client != drag->seat_client) {
            data_source_dnd_drop(drag->source);
        }
        if (drag->source->impl->dnd_finish) {
            // This will end the grab and free `drag`
            data_source_destroy(drag->source);
            return;
        }
    }

    drag_destroy(drag);
}

static void drag_handle_touch_motion(struct wlr_seat_touch_grab *grab, uint32_t time,
                                     struct wlr_touch_point *point)
{
    struct drag *drag = grab->data;
    if (drag->focus && drag->focus_client) {
        struct data_device_client *client = data_device_client_from_seat_client(drag->focus_client);
        if (client) {
            struct wl_resource *resource;
            wl_resource_for_each(resource, &client->data_devices) {
                wl_data_device_send_motion(resource, time, wl_fixed_from_double(point->sx),
                                           wl_fixed_from_double(point->sy));
            }
        }

        struct drag_motion_event event = {
            .drag = drag,
            .time = time,
            .sx = point->sx,
            .sy = point->sy,
        };
        wl_signal_emit_mutable(&drag->events.motion, &event);
    }
}

static void drag_handle_touch_enter(struct wlr_seat_touch_grab *grab, uint32_t time,
                                    struct wlr_touch_point *point)
{
    struct drag *drag = grab->data;
    drag_set_focus(drag, point->focus_surface, point->sx, point->sy);
}

static void drag_handle_touch_cancel(struct wlr_seat_touch_grab *grab)
{
    struct drag *drag = grab->data;
    drag_destroy(drag);
}

static const struct wlr_touch_grab_interface data_device_touch_drag_interface = {
    .down = drag_handle_touch_down,
    .up = drag_handle_touch_up,
    .motion = drag_handle_touch_motion,
    .enter = drag_handle_touch_enter,
    .cancel = drag_handle_touch_cancel,
};

static void drag_handle_keyboard_enter(struct wlr_seat_keyboard_grab *grab,
                                       struct wlr_surface *surface, const uint32_t keycodes[],
                                       size_t num_keycodes,
                                       const struct wlr_keyboard_modifiers *modifiers)
{
    // nothing has keyboard focus during drags
}

static void drag_handle_keyboard_clear_focus(struct wlr_seat_keyboard_grab *grab)
{
    // nothing has keyboard focus during drags
}

static void drag_handle_keyboard_key(struct wlr_seat_keyboard_grab *grab, uint32_t time,
                                     uint32_t key, uint32_t state)
{
    // no keyboard input during drags
}

static void drag_handle_keyboard_modifiers(struct wlr_seat_keyboard_grab *grab,
                                           const struct wlr_keyboard_modifiers *modifiers)
{
    struct drag *drag = grab->data;
    struct wlr_seat_client *client = drag->focus_client;
    if (!client) {
        return;
    }

    uint32_t serial = wlr_seat_client_next_serial(client);
    struct wl_resource *resource;
    wl_resource_for_each(resource, &client->keyboards) {
        if (!wl_resource_get_user_data(resource)) {
            continue;
        }

        if (!modifiers) {
            wl_keyboard_send_modifiers(resource, serial, 0, 0, 0, 0);
        } else {
            wl_keyboard_send_modifiers(resource, serial, modifiers->depressed, modifiers->latched,
                                       modifiers->locked, modifiers->group);
        }
    }
}

static void drag_handle_keyboard_cancel(struct wlr_seat_keyboard_grab *grab)
{
    struct drag *drag = grab->data;
    drag_destroy(drag);
}

static const struct wlr_keyboard_grab_interface data_device_keyboard_drag_interface = {
    .enter = drag_handle_keyboard_enter,
    .clear_focus = drag_handle_keyboard_clear_focus,
    .key = drag_handle_keyboard_key,
    .modifiers = drag_handle_keyboard_modifiers,
    .cancel = drag_handle_keyboard_cancel,
};

static void drag_handle_seat_client_destroy(struct wl_listener *listener, void *data)
{
    struct drag *drag = wl_container_of(listener, drag, seat_client_destroy);
    wl_list_remove(&drag->seat_client_destroy.link);
    wl_list_init(&drag->seat_client_destroy.link);
    drag->seat_client = NULL;
}

struct drag *drag_create(struct wlr_seat_client *seat_client, struct data_source *source,
                         struct wlr_surface *icon_surface, bool drag_x11)
{
    struct drag *drag = calloc(1, sizeof(*drag));
    if (!drag) {
        return NULL;
    }

    wl_signal_init(&drag->events.focus);
    wl_signal_init(&drag->events.motion);
    wl_signal_init(&drag->events.drop);
    wl_signal_init(&drag->events.destroy);

    wl_list_init(&drag->focus_destroy.link);
    wl_list_init(&drag->denied_focus_destroy.link);

    drag->drag_x11 = drag_x11;
    drag->seat = seat_from_wlr_seat(seat_client->seat);
    drag->seat_client = seat_client;
    drag->seat_client_destroy.notify = drag_handle_seat_client_destroy;
    wl_signal_add(&seat_client->events.destroy, &drag->seat_client_destroy);

    if (icon_surface) {
        struct drag_icon *icon = drag_icon_create(drag, icon_surface);
        if (!icon) {
            free(drag);
            return NULL;
        }

        drag->icon = icon;

        drag->icon_destroy.notify = drag_handle_icon_destroy;
        wl_signal_add(&icon->events.destroy, &drag->icon_destroy);
    }

    drag->source = source;
    if (source) {
        drag->source_destroy.notify = drag_handle_drag_source_destroy;
        wl_signal_add(&source->events.destroy, &drag->source_destroy);
    }

    drag->pointer_grab.data = drag;
    drag->pointer_grab.interface = &data_device_pointer_drag_interface;

    drag->touch_grab.data = drag;
    drag->touch_grab.interface = &data_device_touch_drag_interface;

    drag->keyboard_grab.data = drag;
    drag->keyboard_grab.interface = &data_device_keyboard_drag_interface;

    return drag;
}

static void data_device_handle_start_drag(struct wl_client *wl_client,
                                          struct wl_resource *device_resource,
                                          struct wl_resource *source_resource,
                                          struct wl_resource *origin_resource,
                                          struct wl_resource *icon_resource, uint32_t serial)
{
    struct data_device_client *client = data_device_client_from_resource(device_resource);
    if (!client) {
        return;
    }

    struct wlr_surface *origin = wlr_surface_from_resource(origin_resource);

    struct client_data_source *source = NULL;
    if (source_resource) {
        source = client_data_source_from_resource(source_resource);
    }

    struct wlr_surface *icon = NULL;
    if (icon_resource) {
        icon = wlr_surface_from_resource(icon_resource);
        if (!wlr_surface_set_role(icon, &drag_icon_surface_role, icon_resource,
                                  WL_DATA_DEVICE_ERROR_ROLE)) {
            return;
        }
    }

    struct data_source *wlr_source = source ? &source->source : NULL;
    struct drag *drag = drag_create(client->seat_client, wlr_source, icon, false);
    if (!drag) {
        wl_resource_post_no_memory(device_resource);
        return;
    }

    if (source) {
        source->finalized = true;
    }

    struct seat *seat = seat_from_wlr_seat(client->seat_client->seat);
    seat_request_start_drag(seat, drag, origin, serial);
}

static void data_device_handle_set_selection(struct wl_client *wl_client,
                                             struct wl_resource *device_resource,
                                             struct wl_resource *source_resource, uint32_t serial)
{
    struct data_device_client *client = data_device_client_from_resource(device_resource);
    if (!client) {
        return;
    }

    struct client_data_source *source = NULL;
    if (source_resource) {
        source = client_data_source_from_resource(source_resource);
    }

    if (source) {
        source->finalized = true;
    }

    struct data_source *wlr_source = source ? &source->source : NULL;
    struct seat *seat = seat_from_wlr_seat(client->seat_client->seat);
    seat_request_set_selection(seat, client->seat_client, wlr_source, serial);
}

static void data_device_handle_release(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct wl_data_device_interface data_device_impl = {
    .start_drag = data_device_handle_start_drag,
    .set_selection = data_device_handle_set_selection,
    .release = data_device_handle_release,
};

static void data_device_handle_resource_destroy(struct wl_resource *resource)
{
    wl_list_remove(wl_resource_get_link(resource));
    wl_list_init(wl_resource_get_link(resource));
}

static void device_resource_send_selection(struct wl_resource *device_resource)
{
    struct data_device_client *client = data_device_client_from_resource(device_resource);
    assert(client != NULL);

    struct seat *seat = seat_from_wlr_seat(client->seat_client->seat);
    struct data_source *source = seat->selection_source;
    struct wl_client *wl_client = client->seat_client->client;

    // XWayland does not bind these protocols, X11 reads are enforced at SelectionRequest
    if (source && security_check_selection(seat)) {
        struct client reader = { .type = CLIENT_TYPE_WAYLAND };
        wl_client_get_credentials(wl_client, &reader.pid, &reader.uid, &reader.gid);
        client_set_comm(&reader);
        if (!data_source_allow_read(seat, source, &reader)) {
            source = NULL;
        }
    }

    if (source) {
        struct data_offer *offer = data_offer_create(device_resource, source, DATA_OFFER_SELECTION);
        if (!offer) {
            wl_client_post_no_memory(wl_client);
            return;
        }

        wl_data_device_send_selection(device_resource, offer->resource);
    } else {
        wl_data_device_send_selection(device_resource, NULL);
    }
}

static void handle_seat_client_destroy(struct wl_listener *listener, void *data)
{
    struct data_device_client *client = wl_container_of(listener, client, seat_client_destroy);
    wl_list_remove(&client->seat_client_destroy.link);

    struct wl_resource *resource, *tmp;
    wl_resource_for_each_safe(resource, tmp, &client->data_devices) {
        // Make the data device inert
        wl_list_remove(wl_resource_get_link(resource));
        wl_list_init(wl_resource_get_link(resource));
        wl_resource_set_user_data(resource, NULL);
    }

    free(client);
}

static struct data_device_client *
data_device_client_get_or_create(struct wlr_seat_client *seat_client)
{
    struct data_device_client *client = data_device_client_from_seat_client(seat_client);
    if (client) {
        return client;
    }

    client = calloc(1, sizeof(*client));
    if (!client) {
        return NULL;
    }

    wl_list_init(&client->data_devices);
    client->seat_client = seat_client;
    client->seat_client_destroy.notify = handle_seat_client_destroy;
    wl_signal_add(&seat_client->events.destroy, &client->seat_client_destroy);

    // trick to binding data_device_client to wlr_seat_client
    seat_client->data_devices.prev = (struct wl_list *)client;
    return client;
}

static void data_device_manager_get_data_device(struct wl_client *wl_client,
                                                struct wl_resource *manager_resource, uint32_t id,
                                                struct wl_resource *seat_resource)
{
    uint32_t version = wl_resource_get_version(manager_resource);
    struct wl_resource *resource =
        wl_resource_create(wl_client, &wl_data_device_interface, version, id);
    if (!resource) {
        wl_resource_post_no_memory(manager_resource);
        return;
    }

    // seat_client may be NULL
    struct wlr_seat_client *seat_client = wlr_seat_client_from_resource(seat_resource);
    struct data_device_client *client = NULL;
    if (seat_client) {
        client = data_device_client_get_or_create(seat_client);
    }

    wl_resource_set_implementation(resource, &data_device_impl, client,
                                   data_device_handle_resource_destroy);
    if (!client) {
        wl_list_init(wl_resource_get_link(resource));
        return;
    }

    wl_list_insert(&client->data_devices, wl_resource_get_link(resource));

    struct wlr_seat_client *focused_client = seat_client->seat->keyboard_state.focused_client;
    if (focused_client == seat_client) {
        device_resource_send_selection(resource);
    }
}

static const struct wl_data_device_manager_interface data_device_manager_impl = {
    .create_data_source = data_device_manager_create_data_source,
    .get_data_device = data_device_manager_get_data_device,
};

static void data_device_manager_bind(struct wl_client *client, void *data, uint32_t version,
                                     uint32_t id)
{
    struct wlr_data_device_manager *manager = data;

    struct wl_resource *resource =
        wl_resource_create(client, &wl_data_device_manager_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &data_device_manager_impl, manager, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    struct data_device_manager *manager = wl_container_of(listener, manager, display_destroy);
    wl_list_remove(&manager->display_destroy.link);
    wl_global_destroy(manager->global);
    free(manager);
}

bool data_device_manager_create(struct wl_display *display)
{
    struct data_device_manager *manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    manager->global =
        wl_global_create(display, &wl_data_device_manager_interface, DATA_DEVICE_MANAGER_VERSION,
                         manager, data_device_manager_bind);
    if (!manager->global) {
        kywc_log(KYWC_ERROR, "Could not create data device manager wl_global");
        free(manager);
        return false;
    }

    manager->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(display, &manager->display_destroy);

    return true;
}

void seat_request_start_drag(struct seat *seat, struct drag *drag, struct wlr_surface *origin,
                             uint32_t serial)
{
    assert(drag->seat == seat);

    if (seat->drag) {
        kywc_log(KYWC_DEBUG, "Rejecting start_drag request, "
                             "another drag-and-drop operation is already in progress");
        return;
    }

    if (drag->source && security_check_selection(seat) &&
        !data_source_allow_start_drag(seat, drag->source)) {
        data_source_destroy(drag->source);
        return;
    }

    struct seat_request_start_drag_event event = {
        .drag = drag,
        .origin = origin,
        .serial = serial,
    };
    wl_signal_emit_mutable(&seat->events.request_start_drag, &event);
}

void wlr_seat_client_send_selection(struct wlr_seat_client *seat_client)
{
    struct seat *seat = seat_from_wlr_seat(seat_client->seat);
    struct data_source *source = seat->selection_source;
    if (source) {
        source->accepted = false;
    }

    // Make all current offers inert
    struct data_offer *offer, *tmp;
    wl_list_for_each_safe(offer, tmp, &seat->selection_offers, link) {
        data_offer_destroy(offer);
    }

    struct data_device_client *client = data_device_client_from_seat_client(seat_client);
    if (client) {
        struct wl_resource *device_resource;
        wl_resource_for_each(device_resource, &client->data_devices) {
            device_resource_send_selection(device_resource);
        }
    }
}

static void seat_handle_selection_source_destroy(struct wl_listener *listener, void *data)
{
    struct seat *seat = wl_container_of(listener, seat, selection_source_destroy);

    wl_list_remove(&seat->selection_source_destroy.link);
    seat->selection_source = NULL;

    struct wlr_seat_client *focused_client = seat->wlr_seat->keyboard_state.focused_client;
    if (focused_client) {
        wlr_seat_client_send_selection(focused_client);
    }

    wl_signal_emit_mutable(&seat->events.set_selection, seat);
}

void seat_request_set_selection(struct seat *seat, struct wlr_seat_client *client,
                                struct data_source *source, uint32_t serial)
{
    if (client && !wlr_seat_client_validate_event_serial(client, serial)) {
        kywc_log(KYWC_DEBUG,
                 "Rejecting set_selection request, "
                 "serial %" PRIu32 " was never given to client",
                 serial);
        return;
    }

    if (seat->selection_source && serial - seat->selection_serial > UINT32_MAX / 2) {
        kywc_log(KYWC_DEBUG,
                 "Rejecting set_selection request, serial indicates superseded "
                 "(%" PRIu32 " < %" PRIu32 ")",
                 serial, seat->selection_serial);
        return;
    }

    struct seat_request_set_selection_event event = {
        .source = source,
        .serial = serial,
    };
    wl_signal_emit_mutable(&seat->events.request_set_selection, &event);
}

void seat_set_selection(struct seat *seat, struct data_source *source, uint32_t serial)
{
    if (seat->selection_source == source) {
        seat->selection_serial = serial;
        return;
    }

    if (source && security_check_selection(seat) && !data_source_allow_write(seat, source)) {
        data_source_destroy(source);
        return;
    }

    if (seat->selection_source) {
        wl_list_remove(&seat->selection_source_destroy.link);
        data_source_destroy(seat->selection_source);
        seat->selection_source = NULL;
    }

    seat->selection_source = source;
    seat->selection_serial = serial;

    if (source) {
        seat->selection_source_destroy.notify = seat_handle_selection_source_destroy;
        wl_signal_add(&source->events.destroy, &seat->selection_source_destroy);
    }

    struct wlr_seat_client *focused_client = seat->wlr_seat->keyboard_state.focused_client;
    if (focused_client) {
        wlr_seat_client_send_selection(focused_client);
    }

    wl_signal_emit_mutable(&seat->events.set_selection, seat);
}

static void seat_handle_drag_source_destroy(struct wl_listener *listener, void *data)
{
    struct seat *seat = wl_container_of(listener, seat, drag_source_destroy);
    wl_list_remove(&seat->drag_source_destroy.link);
    seat->drag_source = NULL;
}

void seat_start_drag(struct seat *seat, struct drag *drag, uint32_t serial)
{
    assert(drag->seat == seat);
    assert(!drag->started);
    drag->started = true;

    wlr_seat_keyboard_start_grab(seat->wlr_seat, &drag->keyboard_grab);

    seat->drag = drag;
    seat->drag_serial = serial;

    // We need to destroy the previous source, because listeners only expect one
    // active drag source at a time.
    data_source_destroy(seat->drag_source);
    seat->drag_source = drag->source;
    if (drag->source) {
        seat->drag_source_destroy.notify = seat_handle_drag_source_destroy;
        wl_signal_add(&drag->source->events.destroy, &seat->drag_source_destroy);
    }

    wl_signal_emit_mutable(&seat->events.start_drag, drag);
}

void seat_start_pointer_drag(struct seat *seat, struct drag *drag, uint32_t serial)
{
    drag->grab_type = DRAG_GRAB_KEYBOARD_POINTER;

    wlr_seat_pointer_clear_focus(seat->wlr_seat);
    wlr_seat_pointer_start_grab(seat->wlr_seat, &drag->pointer_grab);

    seat_start_drag(seat, drag, serial);
}

void seat_start_touch_drag(struct seat *seat, struct drag *drag, uint32_t serial,
                           struct wlr_touch_point *point)
{
    drag->grab_type = DRAG_GRAB_KEYBOARD_TOUCH;
    drag->grab_touch_id = seat->wlr_seat->touch_state.grab_id;
    drag->touch_id = point->touch_id;

    wlr_seat_touch_start_grab(seat->wlr_seat, &drag->touch_grab);
    drag_set_focus(drag, point->surface, point->sx, point->sy);

    seat_start_drag(seat, drag, serial);
}

struct data_source *data_source_from_resource(struct wl_resource *resource)
{
    struct client_data_source *source = client_data_source_from_resource(resource);
    return source ? &source->source : NULL;
}
