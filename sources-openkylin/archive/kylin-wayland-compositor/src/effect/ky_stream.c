// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>

#include <kywc/log.h>

#include "kywc-stream-v1-protocol.h"

#include "effect/stream.h"
#include "effect_p.h"
#include "output.h"
#include "util/client.h"
#include "view/view.h"

struct ky_stream_manager {
    struct wl_global *global;
    struct wl_listener display_destroy;
    struct wl_listener server_destroy;
};

struct ky_stream {
    struct wl_resource *resource;
    struct stream *stream;
};

static void ky_stream_destroy(struct ky_stream *stream)
{
    wl_resource_set_user_data(stream->resource, NULL);
    if (stream->stream) {
        stream_destroy(stream->stream);
    }
    free(stream);
}

static void stream_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct kywc_stream_v1_interface ky_stream_impl = {
    .destroy = stream_handle_destroy,
};

static void stream_handle_resource_destroy(struct wl_resource *resource)
{
    struct ky_stream *stream = wl_resource_get_user_data(resource);
    if (stream) {
        ky_stream_destroy(stream);
    }
}

static void stream_handle_ready(struct stream *stream, uint32_t node, void *data)
{
    struct ky_stream *ky_stream = data;
    kywc_stream_v1_send_created(ky_stream->resource, node);
}

static void stream_handle_cancel(struct stream *stream, void *data)
{
    struct ky_stream *ky_stream = data;
    kywc_stream_v1_send_cancelled(ky_stream->resource);
    ky_stream->stream = NULL;
    ky_stream_destroy(ky_stream);
}

static void stream_handle_client(struct stream *stream, struct client *client, void *data)
{
    struct ky_stream *ky_stream = data;
    client->type = CLIENT_TYPE_WAYLAND;
    struct wl_client *wl_client = wl_resource_get_client(ky_stream->resource);
    wl_client_get_credentials(wl_client, &client->pid, &client->uid, &client->gid);
    client_set_comm(client);
}

static const struct stream_interface stream_impl = {
    .ready = stream_handle_ready,
    .cancel = stream_handle_cancel,
    .client = stream_handle_client,
};

static void manager_handle_stream_output(struct wl_client *client, struct wl_resource *resource,
                                         uint32_t id, const char *uuid, uint32_t pointer)
{
    struct ky_stream *stream = calloc(1, sizeof(*stream));
    if (!stream) {
        wl_client_post_no_memory(client);
        return;
    }

    /* create frame resource with id */
    uint32_t version = wl_resource_get_version(resource);
    stream->resource = wl_resource_create(client, &kywc_stream_v1_interface, version, id);
    if (!stream->resource) {
        wl_client_post_no_memory(client);
        free(stream);
        return;
    }

    wl_resource_set_implementation(stream->resource, &ky_stream_impl, stream,
                                   stream_handle_resource_destroy);

    /* check output is valid */
    struct kywc_output *kywc_output = kywc_output_by_uuid(uuid);
    if (!kywc_output || !kywc_output->state.enabled) {
        kywc_stream_v1_send_failed(stream->resource);
        wl_resource_set_user_data(stream->resource, NULL);
        free(stream);
        return;
    }

    uint32_t options =
        pointer == KYWC_STREAM_MANAGER_V1_POINTER_EMBEDDED ? STREAM_NEED_CURSOR : STREAM_NEED_NONE;
    stream->stream = stream_create_for_output(output_from_kywc_output(kywc_output), options,
                                              &stream_impl, stream);
    if (!stream->stream) {
        kywc_stream_v1_send_failed(stream->resource);
        wl_resource_set_user_data(stream->resource, NULL);
        free(stream);
        return;
    }
}

static void manager_handle_stream_toplevel(struct wl_client *client, struct wl_resource *resource,
                                           uint32_t id, const char *uuid, uint32_t pointer)
{
    struct ky_stream *stream = calloc(1, sizeof(*stream));
    if (!stream) {
        wl_client_post_no_memory(client);
        return;
    }

    /* create frame resource with id */
    uint32_t version = wl_resource_get_version(resource);
    stream->resource = wl_resource_create(client, &kywc_stream_v1_interface, version, id);
    if (!stream->resource) {
        wl_client_post_no_memory(client);
        free(stream);
        return;
    }

    wl_resource_set_implementation(stream->resource, &ky_stream_impl, stream,
                                   stream_handle_resource_destroy);

    struct kywc_view *kywc_view = kywc_view_by_uuid(uuid);
    if (!kywc_view || !kywc_view->mapped) {
        kywc_stream_v1_send_failed(stream->resource);
        wl_resource_set_user_data(stream->resource, NULL);
        free(stream);
        return;
    }

    uint32_t options =
        pointer == KYWC_STREAM_MANAGER_V1_POINTER_EMBEDDED ? STREAM_NEED_CURSOR : STREAM_NEED_NONE;
    stream->stream =
        stream_create_for_view(view_from_kywc_view(kywc_view), options, &stream_impl, stream);
    if (!stream->stream) {
        kywc_stream_v1_send_failed(stream->resource);
        wl_resource_set_user_data(stream->resource, NULL);
        free(stream);
        return;
    }
}

static void manager_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct kywc_stream_manager_v1_interface ky_stream_manager_impl = {
    .stream_output = manager_handle_stream_output,
    .stream_toplevel = manager_handle_stream_toplevel,
    .destroy = manager_handle_destroy,
};

static void ky_stream_manager_bind(struct wl_client *client, void *data, uint32_t version,
                                   uint32_t id)
{
    struct wl_resource *resource =
        wl_resource_create(client, &kywc_stream_manager_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    struct ky_stream_manager *manager = data;
    wl_resource_set_implementation(resource, &ky_stream_manager_impl, manager, NULL);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    struct ky_stream_manager *manager = wl_container_of(listener, manager, server_destroy);
    wl_list_remove(&manager->server_destroy.link);
    free(manager);
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    struct ky_stream_manager *manager = wl_container_of(listener, manager, display_destroy);
    wl_list_remove(&manager->display_destroy.link);
    wl_global_destroy(manager->global);
}

bool ky_stream_manager_create(struct server *server)
{
    struct ky_stream_manager *manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    manager->global = wl_global_create(server->display, &kywc_stream_manager_v1_interface, 1,
                                       manager, ky_stream_manager_bind);
    if (!manager->global) {
        kywc_log(KYWC_WARN, "Kywc stream manager create failed");
        free(manager);
        return false;
    }

    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &manager->server_destroy);
    manager->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(server->display, &manager->display_destroy);

    return true;
}
