// spdx-filecopyrighttext: 2023 the wlroots contributors
// spdx-filecopyrighttext: 2026 kylinsoft co., ltd.
//
// spdx-license-identifier: gpl-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <kywc/log.h>

#include "input/data_device.h"
#include "input/primary_selection.h"
#include "input/seat.h"
#include "input_p.h"
#include "security/security.h"
#include "util/client.h"

#include "wlr-data-control-unstable-v1-protocol.h"

#define DATA_CONTROL_MANAGER_VERSION 2

struct data_control_manager {
    struct wl_global *global;
    struct wl_listener display_destroy;

    struct wl_list devices; // data_control_device.link
};

struct data_control_source {
    struct wl_resource *resource;
    struct wl_array mime_types;
    bool finalized;

    // Only one of these is non-NULL.
    struct data_source *active_source;
    struct primary_selection_source *active_primary_source;
};

struct data_control_device {
    struct wl_resource *resource;
    struct data_control_manager *manager;
    struct wl_list link; // data_control_manager.devices

    struct wl_resource *selection_offer_resource;         // current selection offer
    struct wl_resource *primary_selection_offer_resource; // current primary selection offer

    struct seat *seat;
    struct wl_listener seat_set_selection;
    struct wl_listener seat_set_primary_selection;
    struct wl_listener seat_destroy;
};

struct client_data_source {
    struct data_source source;
    struct wl_resource *resource;
};

struct client_primary_selection_source {
    struct primary_selection_source source;
    struct wl_resource *resource;
};

struct data_control_offer {
    struct wl_resource *resource;
    struct data_control_device *device;
    bool is_primary;
};

static const struct zwlr_data_control_source_v1_interface source_impl;

static struct data_control_source *source_from_resource(struct wl_resource *resource)
{
    assert(wl_resource_instance_of(resource, &zwlr_data_control_source_v1_interface, &source_impl));
    return wl_resource_get_user_data(resource);
}

static void source_handle_offer(struct wl_client *client, struct wl_resource *resource,
                                const char *mime_type)
{
    struct data_control_source *source = source_from_resource(resource);
    if (!source) {
        return;
    }

    if (source->finalized) {
        wl_resource_post_error(resource, ZWLR_DATA_CONTROL_SOURCE_V1_ERROR_INVALID_OFFER,
                               "cannot mutate offer after set_selection or "
                               "set_primary_selection");
        return;
    }

    const char **mime_type_ptr;
    wl_array_for_each(mime_type_ptr, &source->mime_types) {
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

    char **p = wl_array_add(&source->mime_types, sizeof(char *));
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

static const struct zwlr_data_control_source_v1_interface source_impl = {
    .offer = source_handle_offer,
    .destroy = source_handle_destroy,
};

static void data_control_source_destroy(struct data_control_source *source)
{
    if (!source) {
        return;
    }

    char **p;
    wl_array_for_each(p, &source->mime_types) {
        free(*p);
    }
    wl_array_release(&source->mime_types);

    // Prevent destructors below from calling this recursively.
    wl_resource_set_user_data(source->resource, NULL);

    if (source->active_source) {
        data_source_destroy(source->active_source);
    } else if (source->active_primary_source) {
        primary_selection_source_destroy(source->active_primary_source);
    }

    free(source);
}

static void source_handle_resource_destroy(struct wl_resource *resource)
{
    struct data_control_source *source = source_from_resource(resource);
    data_control_source_destroy(source);
}

static const struct data_source_impl client_source_impl;

static struct client_data_source *client_data_source_from_source(struct data_source *data_source)
{
    assert(data_source->impl == &client_source_impl);
    struct client_data_source *source = wl_container_of(data_source, source, source);
    return source;
}

static void client_source_send(struct data_source *data_source, const char *mime_type, int fd)
{
    struct client_data_source *source = client_data_source_from_source(data_source);
    zwlr_data_control_source_v1_send_send(source->resource, mime_type, fd);
    close(fd);
}

static void client_source_destroy(struct data_source *data_source)
{
    struct client_data_source *client_source = client_data_source_from_source(data_source);
    struct data_control_source *source = source_from_resource(client_source->resource);
    free(client_source);

    if (!source) {
        return;
    }

    source->active_source = NULL;

    zwlr_data_control_source_v1_send_cancelled(source->resource);
    data_control_source_destroy(source);
}

static void client_source_client(struct data_source *data_source, struct client *client)
{
    struct client_data_source *source = client_data_source_from_source(data_source);
    client->type = CLIENT_TYPE_WAYLAND;
    wl_client_get_credentials(wl_resource_get_client(source->resource), &client->pid, &client->uid,
                              &client->gid);
    client_set_comm(client);
}

static const struct data_source_impl client_source_impl = {
    .send = client_source_send,
    .destroy = client_source_destroy,
    .client = client_source_client,
};

static const struct primary_selection_source_impl client_primary_selection_source_impl;

static struct client_primary_selection_source *
client_primary_selection_source_from_source(struct primary_selection_source *primary_source)
{
    assert(primary_source->impl == &client_primary_selection_source_impl);
    struct client_primary_selection_source *source =
        wl_container_of(primary_source, source, source);
    return source;
}

static void client_primary_selection_source_send(struct primary_selection_source *primary_source,
                                                 const char *mime_type, int fd)
{
    struct client_primary_selection_source *source =
        client_primary_selection_source_from_source(primary_source);
    zwlr_data_control_source_v1_send_send(source->resource, mime_type, fd);
    close(fd);
}

static void client_primary_selection_source_destroy(struct primary_selection_source *primary_source)
{
    struct client_primary_selection_source *client_source =
        client_primary_selection_source_from_source(primary_source);
    struct data_control_source *source = source_from_resource(client_source->resource);
    free(client_source);

    if (!source) {
        return;
    }

    source->active_primary_source = NULL;

    zwlr_data_control_source_v1_send_cancelled(source->resource);
    data_control_source_destroy(source);
}

static void client_primary_selection_source_client(struct primary_selection_source *primary_source,
                                                   struct client *client)
{
    struct client_primary_selection_source *source =
        client_primary_selection_source_from_source(primary_source);
    client->type = CLIENT_TYPE_WAYLAND;
    wl_client_get_credentials(wl_resource_get_client(source->resource), &client->pid, &client->uid,
                              &client->gid);
    client_set_comm(client);
}

static const struct primary_selection_source_impl client_primary_selection_source_impl = {
    .send = client_primary_selection_source_send,
    .destroy = client_primary_selection_source_destroy,
    .client = client_primary_selection_source_client,
};

static void data_offer_destroy(struct data_control_offer *offer)
{
    if (!offer) {
        return;
    }

    struct data_control_device *device = offer->device;
    if (device) {
        if (offer->is_primary) {
            device->primary_selection_offer_resource = NULL;
        } else {
            device->selection_offer_resource = NULL;
        }
    }

    wl_resource_set_user_data(offer->resource, NULL);
    free(offer);
}

static const struct zwlr_data_control_offer_v1_interface offer_impl;

static struct data_control_offer *data_offer_from_offer_resource(struct wl_resource *resource)
{
    assert(wl_resource_instance_of(resource, &zwlr_data_control_offer_v1_interface, &offer_impl));
    return wl_resource_get_user_data(resource);
}

static void offer_handle_receive(struct wl_client *client, struct wl_resource *resource,
                                 const char *mime_type, int fd)
{
    struct data_control_offer *offer = data_offer_from_offer_resource(resource);
    if (!offer) {
        close(fd);
        return;
    }

    struct data_control_device *device = offer->device;
    if (!device) {
        close(fd);
        return;
    }

    if (offer->is_primary) {
        struct primary_selection_source *source = device->seat->primary_selection_source;
        if (!source) {
            close(fd);
            return;
        }
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
    } else {
        struct data_source *source = device->seat->selection_source;
        if (!source) {
            close(fd);
            return;
        }
        if (security_check_selection(device->seat)) {
            struct client reader = { .type = CLIENT_TYPE_WAYLAND };
            wl_client_get_credentials(client, &reader.pid, &reader.uid, &reader.gid);
            client_set_comm(&reader);
            if (!data_source_allow_read(device->seat, source, &reader)) {
                close(fd);
                return;
            }
        }
        data_source_send(source, mime_type, fd);
    }
}

static void offer_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct zwlr_data_control_offer_v1_interface offer_impl = {
    .receive = offer_handle_receive,
    .destroy = offer_handle_destroy,
};

static void offer_handle_resource_destroy(struct wl_resource *resource)
{
    struct data_control_offer *offer = data_offer_from_offer_resource(resource);
    data_offer_destroy(offer);
}

static struct wl_resource *create_offer(struct data_control_device *device,
                                        struct wl_array *mime_types, bool is_primary)
{
    struct wl_client *client = wl_resource_get_client(device->resource);

    struct data_control_offer *offer = calloc(1, sizeof(*offer));
    if (!offer) {
        wl_client_post_no_memory(client);
        return NULL;
    }

    offer->device = device;
    offer->is_primary = is_primary;

    uint32_t version = wl_resource_get_version(device->resource);
    struct wl_resource *resource =
        wl_resource_create(client, &zwlr_data_control_offer_v1_interface, version, 0);
    if (!resource) {
        free(offer);
        return NULL;
    }

    offer->resource = resource;
    wl_resource_set_implementation(resource, &offer_impl, offer, offer_handle_resource_destroy);
    zwlr_data_control_device_v1_send_data_offer(device->resource, resource);

    char **p;
    wl_array_for_each(p, mime_types) {
        zwlr_data_control_offer_v1_send_offer(resource, *p);
    }

    return resource;
}

static const struct zwlr_data_control_device_v1_interface control_impl;

static struct data_control_device *control_from_resource(struct wl_resource *resource)
{
    assert(
        wl_resource_instance_of(resource, &zwlr_data_control_device_v1_interface, &control_impl));
    return wl_resource_get_user_data(resource);
}

static void control_handle_set_selection(struct wl_client *client,
                                         struct wl_resource *control_resource,
                                         struct wl_resource *source_resource)
{
    struct data_control_device *device = control_from_resource(control_resource);
    if (!device) {
        return;
    }

    struct data_control_source *source = NULL;
    if (source_resource) {
        source = source_from_resource(source_resource);
    }

    if (!source) {
        seat_request_set_selection(device->seat, NULL, NULL,
                                   wl_display_next_serial(device->seat->wlr_seat->display));
        return;
    }

    if (source->active_source || source->active_primary_source) {
        wl_resource_post_error(control_resource, ZWLR_DATA_CONTROL_DEVICE_V1_ERROR_USED_SOURCE,
                               "cannot use a data source in set_selection or "
                               "set_primary_selection more than once");
        return;
    }

    struct client_data_source *client_source = calloc(1, sizeof(*client_source));
    if (!client_source) {
        wl_client_post_no_memory(client);
        return;
    }
    client_source->resource = source_resource;

    struct data_source *data_source = &client_source->source;
    data_source_init(data_source, &client_source_impl);
    source->active_source = data_source;

    wl_array_release(&data_source->mime_types);
    data_source->mime_types = source->mime_types;
    wl_array_init(&source->mime_types);

    source->finalized = true;

    seat_request_set_selection(device->seat, NULL, data_source,
                               wl_display_next_serial(device->seat->wlr_seat->display));
}

static void control_handle_set_primary_selection(struct wl_client *client,
                                                 struct wl_resource *control_resource,
                                                 struct wl_resource *source_resource)
{
    struct data_control_device *device = control_from_resource(control_resource);
    if (!device) {
        return;
    }

    struct data_control_source *source = NULL;
    if (source_resource) {
        source = source_from_resource(source_resource);
    }

    if (!source) {
        seat_request_set_primary_selection(device->seat, NULL, NULL,
                                           wl_display_next_serial(device->seat->wlr_seat->display));
        return;
    }

    if (source->active_source || source->active_primary_source) {
        wl_resource_post_error(control_resource, ZWLR_DATA_CONTROL_DEVICE_V1_ERROR_USED_SOURCE,
                               "cannot use a data source in set_selection or "
                               "set_primary_selection more than once");
        return;
    }

    struct client_primary_selection_source *client_source = calloc(1, sizeof(*client_source));
    if (!client_source) {
        wl_client_post_no_memory(client);
        return;
    }
    client_source->resource = source_resource;

    struct primary_selection_source *primary_source = &client_source->source;
    primary_selection_source_init(primary_source, &client_primary_selection_source_impl);
    source->active_primary_source = primary_source;

    wl_array_release(&primary_source->mime_types);
    primary_source->mime_types = source->mime_types;
    wl_array_init(&source->mime_types);

    source->finalized = true;

    seat_request_set_primary_selection(device->seat, NULL, primary_source,
                                       wl_display_next_serial(device->seat->wlr_seat->display));
}

static void control_handle_destroy(struct wl_client *client, struct wl_resource *control_resource)
{
    wl_resource_destroy(control_resource);
}

static const struct zwlr_data_control_device_v1_interface control_impl = {
    .set_selection = control_handle_set_selection,
    .set_primary_selection = control_handle_set_primary_selection,
    .destroy = control_handle_destroy,
};

static void control_send_selection(struct data_control_device *device)
{
    struct data_source *source = device->seat->selection_source;

    if (device->selection_offer_resource) {
        // Make the offer inert
        struct data_control_offer *offer =
            data_offer_from_offer_resource(device->selection_offer_resource);
        data_offer_destroy(offer);
    }

    device->selection_offer_resource = NULL;
    if (source && security_check_selection(device->seat)) {
        struct client reader = { .type = CLIENT_TYPE_WAYLAND };
        wl_client_get_credentials(wl_resource_get_client(device->resource), &reader.pid,
                                  &reader.uid, &reader.gid);
        client_set_comm(&reader);
        if (!data_source_allow_read(device->seat, source, &reader)) {
            source = NULL;
        }
    }

    if (source) {
        device->selection_offer_resource = create_offer(device, &source->mime_types, false);
        if (!device->selection_offer_resource) {
            wl_resource_post_no_memory(device->resource);
            return;
        }
    }

    zwlr_data_control_device_v1_send_selection(device->resource, device->selection_offer_resource);
}

static void control_send_primary_selection(struct data_control_device *device)
{
    uint32_t version = wl_resource_get_version(device->resource);
    if (version < ZWLR_DATA_CONTROL_DEVICE_V1_PRIMARY_SELECTION_SINCE_VERSION) {
        return;
    }

    struct primary_selection_source *source = device->seat->primary_selection_source;

    if (device->primary_selection_offer_resource) {
        // Make the offer inert
        struct data_control_offer *offer =
            data_offer_from_offer_resource(device->primary_selection_offer_resource);
        data_offer_destroy(offer);
    }

    device->primary_selection_offer_resource = NULL;
    if (source && security_check_selection(device->seat)) {
        struct client reader = { .type = CLIENT_TYPE_WAYLAND };
        wl_client_get_credentials(wl_resource_get_client(device->resource), &reader.pid,
                                  &reader.uid, &reader.gid);
        client_set_comm(&reader);
        if (!primary_selection_source_allow_read(device->seat, source, &reader)) {
            source = NULL;
        }
    }

    if (source) {
        device->primary_selection_offer_resource = create_offer(device, &source->mime_types, true);
        if (!device->primary_selection_offer_resource) {
            wl_resource_post_no_memory(device->resource);
            return;
        }
    }

    zwlr_data_control_device_v1_send_primary_selection(device->resource,
                                                       device->primary_selection_offer_resource);
}

static void data_control_device_destroy(struct data_control_device *device)
{
    if (!device) {
        return;
    }

    zwlr_data_control_device_v1_send_finished(device->resource);

    // Make the resources inert
    wl_resource_set_user_data(device->resource, NULL);
    if (device->selection_offer_resource) {
        struct data_control_offer *offer =
            data_offer_from_offer_resource(device->selection_offer_resource);
        data_offer_destroy(offer);
    }

    if (device->primary_selection_offer_resource) {
        struct data_control_offer *offer =
            data_offer_from_offer_resource(device->primary_selection_offer_resource);
        data_offer_destroy(offer);
    }

    wl_list_remove(&device->seat_destroy.link);
    wl_list_remove(&device->seat_set_selection.link);
    wl_list_remove(&device->seat_set_primary_selection.link);
    wl_list_remove(&device->link);
    free(device);
}

static void control_handle_resource_destroy(struct wl_resource *resource)
{
    struct data_control_device *device = control_from_resource(resource);
    data_control_device_destroy(device);
}

static void control_handle_seat_destroy(struct wl_listener *listener, void *data)
{
    struct data_control_device *device = wl_container_of(listener, device, seat_destroy);
    data_control_device_destroy(device);
}

static void control_handle_seat_set_selection(struct wl_listener *listener, void *data)
{
    struct data_control_device *device = wl_container_of(listener, device, seat_set_selection);
    control_send_selection(device);
}

static void control_handle_seat_set_primary_selection(struct wl_listener *listener, void *data)
{
    struct data_control_device *device =
        wl_container_of(listener, device, seat_set_primary_selection);
    control_send_primary_selection(device);
}

static const struct zwlr_data_control_manager_v1_interface manager_impl;

static struct data_control_manager *manager_from_resource(struct wl_resource *resource)
{
    assert(
        wl_resource_instance_of(resource, &zwlr_data_control_manager_v1_interface, &manager_impl));
    return wl_resource_get_user_data(resource);
}

static void manager_handle_create_data_source(struct wl_client *client,
                                              struct wl_resource *manager_resource, uint32_t id)
{
    struct data_control_source *source = calloc(1, sizeof(*source));
    if (!source) {
        wl_resource_post_no_memory(manager_resource);
        return;
    }

    wl_array_init(&source->mime_types);

    uint32_t version = wl_resource_get_version(manager_resource);
    source->resource =
        wl_resource_create(client, &zwlr_data_control_source_v1_interface, version, id);
    if (!source->resource) {
        wl_resource_post_no_memory(manager_resource);
        wl_array_release(&source->mime_types);
        free(source);
        return;
    }
    wl_resource_set_implementation(source->resource, &source_impl, source,
                                   source_handle_resource_destroy);
}

static void manager_handle_get_data_device(struct wl_client *client,
                                           struct wl_resource *manager_resource, uint32_t id,
                                           struct wl_resource *seat_resource)
{
    struct data_control_manager *manager = manager_from_resource(manager_resource);
    struct wlr_seat_client *seat_client = wlr_seat_client_from_resource(seat_resource);

    uint32_t version = wl_resource_get_version(manager_resource);
    struct wl_resource *resource =
        wl_resource_create(client, &zwlr_data_control_device_v1_interface, version, id);
    if (!resource) {
        wl_resource_post_no_memory(manager_resource);
        return;
    }

    wl_resource_set_implementation(resource, &control_impl, NULL, control_handle_resource_destroy);
    if (!seat_client) {
        return;
    }

    struct data_control_device *device = calloc(1, sizeof(*device));
    if (!device) {
        wl_resource_post_no_memory(manager_resource);
        return;
    }

    device->manager = manager;
    device->seat = seat_from_wlr_seat(seat_client->seat);
    device->resource = resource;
    wl_resource_set_user_data(resource, device);

    device->seat_destroy.notify = control_handle_seat_destroy;
    wl_signal_add(&device->seat->events.destroy, &device->seat_destroy);

    device->seat_set_selection.notify = control_handle_seat_set_selection;
    wl_signal_add(&device->seat->events.set_selection, &device->seat_set_selection);

    device->seat_set_primary_selection.notify = control_handle_seat_set_primary_selection;
    wl_signal_add(&device->seat->events.set_primary_selection, &device->seat_set_primary_selection);

    wl_list_insert(&manager->devices, &device->link);

    // At this point maybe the compositor decided to destroy the device. If
    // it's the case then the resource will be inert.
    device = control_from_resource(resource);
    if (device) {
        control_send_selection(device);
        control_send_primary_selection(device);
    }
}

static void manager_handle_destroy(struct wl_client *client, struct wl_resource *manager_resource)
{
    wl_resource_destroy(manager_resource);
}

static const struct zwlr_data_control_manager_v1_interface manager_impl = {
    .create_data_source = manager_handle_create_data_source,
    .get_data_device = manager_handle_get_data_device,
    .destroy = manager_handle_destroy,
};

static void manager_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
    struct data_control_manager *manager = data;
    struct wl_resource *resource =
        wl_resource_create(client, &zwlr_data_control_manager_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &manager_impl, manager, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    struct data_control_manager *manager = wl_container_of(listener, manager, display_destroy);
    wl_list_remove(&manager->display_destroy.link);
    wl_global_destroy(manager->global);
    free(manager);
}

bool wlr_data_control_manager_create(struct wl_display *display)
{
    struct data_control_manager *manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    manager->global = wl_global_create(display, &zwlr_data_control_manager_v1_interface,
                                       DATA_CONTROL_MANAGER_VERSION, manager, manager_bind);
    if (!manager->global) {
        free(manager);
        return false;
    }

    wl_list_init(&manager->devices);

    manager->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(display, &manager->display_destroy);

    return true;
}
