// spdx-filecopyrighttext: 2023 the wlroots contributors
// spdx-filecopyrighttext: 2026 kylinsoft co., ltd.
//
// spdx-license-identifier: gpl-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include <kywc/log.h>

#include "input/primary_selection.h"
#include "input/seat.h"
#include "input_p.h"
#include "security/security.h"
#include "util/client.h"

#include "primary-selection-unstable-v1-protocol.h"

#define DEVICE_MANAGER_VERSION 1

struct primary_selection_device_manager {
    struct wl_global *global;
    struct wl_listener display_destroy;

    struct wl_list devices; // primary_selection_device.link
};

struct primary_selection_device {
    struct primary_selection_device_manager *manager;
    struct wl_list link;

    struct wl_list resources;
    struct wl_list offers; // wl_resource_get_link()

    struct seat *seat;
    struct wl_listener seat_focus_change;
    struct wl_listener seat_set_primary_selection;
    struct wl_listener seat_destroy;
};

struct client_data_source {
    struct primary_selection_source source;
    struct wl_resource *resource;
    bool finalized;
};

static const struct zwp_primary_selection_device_manager_v1_interface device_manager_impl;

static struct primary_selection_device_manager *manager_from_resource(struct wl_resource *resource)
{
    assert(wl_resource_instance_of(resource, &zwp_primary_selection_device_manager_v1_interface,
                                   &device_manager_impl));
    return wl_resource_get_user_data(resource);
}

static const struct zwp_primary_selection_offer_v1_interface offer_impl;

static struct primary_selection_device *device_from_offer_resource(struct wl_resource *resource)
{
    assert(
        wl_resource_instance_of(resource, &zwp_primary_selection_offer_v1_interface, &offer_impl));
    return wl_resource_get_user_data(resource);
}

static const struct zwp_primary_selection_device_v1_interface device_impl;

static struct primary_selection_device *device_from_resource(struct wl_resource *resource)
{
    assert(wl_resource_instance_of(resource, &zwp_primary_selection_device_v1_interface,
                                   &device_impl));
    return wl_resource_get_user_data(resource);
}

static const struct zwp_primary_selection_source_v1_interface source_impl;

static struct client_data_source *client_data_source_from_resource(struct wl_resource *resource)
{
    assert(wl_resource_instance_of(resource, &zwp_primary_selection_source_v1_interface,
                                   &source_impl));
    return wl_resource_get_user_data(resource);
}

void primary_selection_source_init(struct primary_selection_source *source,
                                   const struct primary_selection_source_impl *impl)
{
    assert(impl->send);
    *source = (struct primary_selection_source){
        .impl = impl,
    };
    wl_array_init(&source->mime_types);
    wl_signal_init(&source->events.destroy);

    if (impl->client) {
        impl->client(source, &source->client);
    }
}

void primary_selection_source_send(struct primary_selection_source *source, const char *mime_type,
                                   int32_t fd)
{
    source->impl->send(source, mime_type, fd);
}

bool primary_selection_source_allow_read(struct seat *seat, struct primary_selection_source *source,
                                         struct client *destination)
{
    struct security_action_info info = {
        .action = SECURITY_READ_SELECTION,
        .selection = {
            .seat = seat,
            .source = &source->client,
            .destination = destination,
            .mime_types = &source->mime_types,
            .primary = true,
        },
    };

    return security_access(&info);
}

bool primary_selection_source_allow_write(struct seat *seat,
                                          struct primary_selection_source *source)
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
            .primary = true,
        },
    };

    return security_access(&info);
}

void primary_selection_source_destroy(struct primary_selection_source *source)
{
    if (!source) {
        return;
    }

    wl_signal_emit_mutable(&source->events.destroy, source);
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

static void offer_handle_receive(struct wl_client *client, struct wl_resource *resource,
                                 const char *mime_type, int32_t fd)
{
    struct primary_selection_device *device = device_from_offer_resource(resource);
    if (!device || !device->seat->primary_selection_source) {
        close(fd);
        return;
    }

    struct primary_selection_source *source = device->seat->primary_selection_source;

    if (security_check_selection(device->seat)) {
        struct client reader = { .type = CLIENT_TYPE_WAYLAND };
        wl_client_get_credentials(client, &reader.pid, &reader.uid, &reader.gid);
        client_set_comm(&reader);
        if (!primary_selection_source_allow_read(device->seat, source, &reader)) {
            close(fd);
            return;
        }
    }

    primary_selection_source_send(source, mime_type, fd);
}

static void offer_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct zwp_primary_selection_offer_v1_interface offer_impl = {
    .receive = offer_handle_receive,
    .destroy = offer_handle_destroy,
};

static void offer_handle_resource_destroy(struct wl_resource *resource)
{
    wl_list_remove(wl_resource_get_link(resource));
}

static void create_offer(struct wl_resource *device_resource,
                         struct primary_selection_source *source)
{
    struct primary_selection_device *device = device_from_resource(device_resource);
    assert(device != NULL);

    struct wl_client *client = wl_resource_get_client(device_resource);
    uint32_t version = wl_resource_get_version(device_resource);
    struct wl_resource *resource =
        wl_resource_create(client, &zwp_primary_selection_offer_v1_interface, version, 0);
    if (!resource) {
        wl_resource_post_no_memory(device_resource);
        return;
    }
    wl_resource_set_implementation(resource, &offer_impl, device, offer_handle_resource_destroy);

    wl_list_insert(&device->offers, wl_resource_get_link(resource));

    zwp_primary_selection_device_v1_send_data_offer(device_resource, resource);

    char **p;
    wl_array_for_each(p, &source->mime_types) {
        zwp_primary_selection_offer_v1_send_offer(resource, *p);
    }

    zwp_primary_selection_device_v1_send_selection(device_resource, resource);
}

static void destroy_offer(struct wl_resource *resource)
{
    if (!device_from_offer_resource(resource)) {
        return;
    }

    // Make the offer inert
    wl_resource_set_user_data(resource, NULL);

    struct wl_list *link = wl_resource_get_link(resource);
    wl_list_remove(link);
    wl_list_init(link);
}

static void client_source_send(struct primary_selection_source *wlr_source, const char *mime_type,
                               int fd)
{
    struct client_data_source *source = wl_container_of(wlr_source, source, source);
    zwp_primary_selection_source_v1_send_send(source->resource, mime_type, fd);
    close(fd);
}

static void client_source_destroy(struct primary_selection_source *wlr_source)
{
    struct client_data_source *source = wl_container_of(wlr_source, source, source);
    zwp_primary_selection_source_v1_send_cancelled(source->resource);
    // Make the source resource inert
    wl_resource_set_user_data(source->resource, NULL);
    free(source);
}

static void client_source_client(struct primary_selection_source *wlr_source, struct client *client)
{
    struct client_data_source *source = wl_container_of(wlr_source, source, source);
    client->type = CLIENT_TYPE_WAYLAND;
    wl_client_get_credentials(wl_resource_get_client(source->resource), &client->pid, &client->uid,
                              &client->gid);
    client_set_comm(client);
}

static const struct primary_selection_source_impl client_source_impl = {
    .send = client_source_send,
    .destroy = client_source_destroy,
    .client = client_source_client,
};

static void source_handle_offer(struct wl_client *client, struct wl_resource *resource,
                                const char *mime_type)
{
    struct client_data_source *source = client_data_source_from_resource(resource);
    if (!source) {
        return;
    }

    if (source->finalized) {
        kywc_log(KYWC_DEBUG, "Offering additional MIME type after set_selection");
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

static void source_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct zwp_primary_selection_source_v1_interface source_impl = {
    .offer = source_handle_offer,
    .destroy = source_handle_destroy,
};

static void source_resource_handle_destroy(struct wl_resource *resource)
{
    struct client_data_source *source = client_data_source_from_resource(resource);
    if (!source) {
        return;
    }
    primary_selection_source_destroy(&source->source);
}

static void device_handle_set_selection(struct wl_client *client, struct wl_resource *resource,
                                        struct wl_resource *source_resource, uint32_t serial)
{
    struct primary_selection_device *device = device_from_resource(resource);
    if (!device) {
        return;
    }

    struct client_data_source *client_source = NULL;
    if (source_resource) {
        client_source = client_data_source_from_resource(source_resource);
    }

    struct primary_selection_source *source = NULL;
    if (client_source) {
        client_source->finalized = true;
        source = &client_source->source;
    }

    struct wlr_seat_client *seat_client =
        wlr_seat_client_for_wl_client(device->seat->wlr_seat, client);
    seat_request_set_primary_selection(device->seat, seat_client, source, serial);
}

static void device_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct zwp_primary_selection_device_v1_interface device_impl = {
    .set_selection = device_handle_set_selection,
    .destroy = device_handle_destroy,
};

static void device_handle_resource_destroy(struct wl_resource *resource)
{
    wl_list_remove(wl_resource_get_link(resource));
}

static void device_resource_send_selection(struct wl_resource *resource,
                                           struct primary_selection_source *source)
{
    struct primary_selection_device *device = device_from_resource(resource);
    assert(device != NULL);

    struct wl_client *client = wl_resource_get_client(resource);
    // XWayland does not bind these protocols, X11 reads are enforced at SelectionRequest
    if (source && security_check_selection(device->seat)) {
        struct client reader = { .type = CLIENT_TYPE_WAYLAND };
        wl_client_get_credentials(client, &reader.pid, &reader.uid, &reader.gid);
        client_set_comm(&reader);
        if (!primary_selection_source_allow_read(device->seat, source, &reader)) {
            source = NULL;
        }
    }

    if (source) {
        create_offer(resource, source);
    } else {
        zwp_primary_selection_device_v1_send_selection(resource, NULL);
    }
}

static void device_send_selection(struct primary_selection_device *device)
{
    struct wlr_seat_client *seat_client = device->seat->wlr_seat->keyboard_state.focused_client;
    if (!seat_client) {
        return;
    }

    struct wl_resource *resource;
    wl_resource_for_each(resource, &device->resources) {
        if (wl_resource_get_client(resource) == seat_client->client) {
            device_resource_send_selection(resource, device->seat->primary_selection_source);
        }
    }
}

static void device_destroy(struct primary_selection_device *device)
{
    if (!device) {
        return;
    }

    wl_list_remove(&device->link);
    wl_list_remove(&device->seat_destroy.link);
    wl_list_remove(&device->seat_focus_change.link);
    wl_list_remove(&device->seat_set_primary_selection.link);

    struct wl_resource *resource, *resource_tmp;
    wl_resource_for_each_safe(resource, resource_tmp, &device->offers) {
        destroy_offer(resource);
    }

    wl_resource_for_each_safe(resource, resource_tmp, &device->resources) {
        // Make the resource inert
        wl_resource_set_user_data(resource, NULL);
        struct wl_list *link = wl_resource_get_link(resource);
        wl_list_remove(link);
        wl_list_init(link);
    }

    free(device);
}

static void device_handle_seat_destroy(struct wl_listener *listener, void *data)
{
    struct primary_selection_device *device = wl_container_of(listener, device, seat_destroy);
    device_destroy(device);
}

static void device_handle_seat_focus_change(struct wl_listener *listener, void *data)
{
    struct primary_selection_device *device = wl_container_of(listener, device, seat_focus_change);
    // TODO: maybe make previous offers inert, or set a NULL selection for
    // previous client?
    device_send_selection(device);
}

static void device_handle_seat_set_primary_selection(struct wl_listener *listener, void *data)
{
    struct primary_selection_device *device =
        wl_container_of(listener, device, seat_set_primary_selection);

    struct wl_resource *resource, *tmp;
    wl_resource_for_each_safe(resource, tmp, &device->offers) {
        destroy_offer(resource);
    }

    device_send_selection(device);
}

static struct primary_selection_device *
get_or_create_device(struct primary_selection_device_manager *manager, struct wlr_seat *seat)
{
    struct primary_selection_device *device;
    wl_list_for_each(device, &manager->devices, link) {
        if (device->seat->wlr_seat == seat) {
            return device;
        }
    }

    device = calloc(1, sizeof(*device));
    if (!device) {
        return NULL;
    }

    device->manager = manager;
    wl_list_insert(&manager->devices, &device->link);
    wl_list_init(&device->offers);
    wl_list_init(&device->resources);

    device->seat = seat_from_wlr_seat(seat);
    device->seat_destroy.notify = device_handle_seat_destroy;
    wl_signal_add(&seat->events.destroy, &device->seat_destroy);
    device->seat_focus_change.notify = device_handle_seat_focus_change;
    wl_signal_add(&seat->keyboard_state.events.focus_change, &device->seat_focus_change);
    device->seat_set_primary_selection.notify = device_handle_seat_set_primary_selection;
    wl_signal_add(&device->seat->events.set_primary_selection, &device->seat_set_primary_selection);

    return device;
}

static void device_manager_handle_create_source(struct wl_client *client,
                                                struct wl_resource *manager_resource, uint32_t id)
{
    struct client_data_source *source = calloc(1, sizeof(*source));
    if (!source) {
        wl_client_post_no_memory(client);
        return;
    }

    uint32_t version = wl_resource_get_version(manager_resource);
    source->resource =
        wl_resource_create(client, &zwp_primary_selection_source_v1_interface, version, id);
    if (!source->resource) {
        free(source);
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(source->resource, &source_impl, source,
                                   source_resource_handle_destroy);

    primary_selection_source_init(&source->source, &client_source_impl);
}

static void device_manager_handle_get_device(struct wl_client *client,
                                             struct wl_resource *manager_resource, uint32_t id,
                                             struct wl_resource *seat_resource)
{
    struct wlr_seat_client *seat_client = wlr_seat_client_from_resource(seat_resource);
    struct primary_selection_device_manager *manager = manager_from_resource(manager_resource);

    uint32_t version = wl_resource_get_version(manager_resource);
    struct wl_resource *resource =
        wl_resource_create(client, &zwp_primary_selection_device_v1_interface, version, id);
    if (!resource) {
        wl_resource_post_no_memory(manager_resource);
        return;
    }

    wl_resource_set_implementation(resource, &device_impl, NULL, device_handle_resource_destroy);
    wl_list_init(wl_resource_get_link(resource));
    if (!seat_client) {
        return;
    }

    struct primary_selection_device *device = get_or_create_device(manager, seat_client->seat);
    if (!device) {
        wl_resource_destroy(resource);
        wl_resource_post_no_memory(manager_resource);
        return;
    }

    wl_resource_set_user_data(resource, device);
    wl_list_insert(&device->resources, wl_resource_get_link(resource));

    if (device->seat->wlr_seat->keyboard_state.focused_client == seat_client) {
        device_resource_send_selection(resource, device->seat->primary_selection_source);
    }
}

static void device_manager_handle_destroy(struct wl_client *client,
                                          struct wl_resource *manager_resource)
{
    wl_resource_destroy(manager_resource);
}

static const struct zwp_primary_selection_device_manager_v1_interface device_manager_impl = {
    .create_source = device_manager_handle_create_source,
    .get_device = device_manager_handle_get_device,
    .destroy = device_manager_handle_destroy,
};

static void primary_selection_device_manager_bind(struct wl_client *client, void *data,
                                                  uint32_t version, uint32_t id)
{
    struct wlr_primary_selection_v1_device_manager *manager = data;

    struct wl_resource *resource =
        wl_resource_create(client, &zwp_primary_selection_device_manager_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &device_manager_impl, manager, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    struct primary_selection_device_manager *manager =
        wl_container_of(listener, manager, display_destroy);

    struct primary_selection_device *device, *tmp;
    wl_list_for_each_safe(device, tmp, &manager->devices, link) {
        device_destroy(device);
    }

    wl_list_remove(&manager->display_destroy.link);
    wl_global_destroy(manager->global);
    free(manager);
}

bool primary_selection_device_manager_create(struct wl_display *display)
{
    struct primary_selection_device_manager *manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    manager->global =
        wl_global_create(display, &zwp_primary_selection_device_manager_v1_interface,
                         DEVICE_MANAGER_VERSION, manager, primary_selection_device_manager_bind);
    if (!manager->global) {
        free(manager);
        return false;
    }

    wl_list_init(&manager->devices);

    manager->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(display, &manager->display_destroy);

    return true;
}

void seat_request_set_primary_selection(struct seat *seat, struct wlr_seat_client *client,
                                        struct primary_selection_source *source, uint32_t serial)
{
    if (client && !wlr_seat_client_validate_event_serial(client, serial)) {
        kywc_log(KYWC_DEBUG,
                 "Rejecting set_primary_selection request, "
                 "serial %" PRIu32 " was never given to client",
                 serial);
        return;
    }

    if (seat->primary_selection_source &&
        serial - seat->primary_selection_serial > UINT32_MAX / 2) {
        kywc_log(KYWC_DEBUG,
                 "Rejecting set_primary_selection request, "
                 "serial indicates superseded (%" PRIu32 " < %" PRIu32 ")",
                 serial, seat->primary_selection_serial);
        return;
    }

    struct seat_request_set_primary_selection_event event = {
        .source = source,
        .serial = serial,
    };
    wl_signal_emit_mutable(&seat->events.request_set_primary_selection, &event);
}

static void seat_handle_primary_selection_source_destroy(struct wl_listener *listener, void *data)
{
    struct seat *seat = wl_container_of(listener, seat, primary_selection_source_destroy);
    wl_list_remove(&seat->primary_selection_source_destroy.link);
    seat->primary_selection_source = NULL;
    wl_signal_emit_mutable(&seat->events.set_primary_selection, seat);
}

void seat_set_primary_selection(struct seat *seat, struct primary_selection_source *source,
                                uint32_t serial)
{
    if (seat->primary_selection_source == source) {
        seat->primary_selection_serial = serial;
        return;
    }

    if (source && security_check_selection(seat) &&
        !primary_selection_source_allow_write(seat, source)) {
        primary_selection_source_destroy(source);
        return;
    }

    if (seat->primary_selection_source) {
        wl_list_remove(&seat->primary_selection_source_destroy.link);
        primary_selection_source_destroy(seat->primary_selection_source);
        seat->primary_selection_source = NULL;
    }

    seat->primary_selection_source = source;
    seat->primary_selection_serial = serial;

    if (source) {
        seat->primary_selection_source_destroy.notify =
            seat_handle_primary_selection_source_destroy;
        wl_signal_add(&source->events.destroy, &seat->primary_selection_source_destroy);
    }

    wl_signal_emit_mutable(&seat->events.set_primary_selection, seat);
}
