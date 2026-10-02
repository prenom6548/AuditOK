// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>
#include <string.h>

#include "kywc-stream-v1-client-protocol.h"

#include "libkywc_p.h"

bool _kywc_stream_init(kywc_context *ctx, enum kywc_context_capability capability);

static void manager_destroy(struct ky_stream_manager *manager)
{
    struct kywc_stream_manager_v1 *stream_manager = manager->data;
    kywc_stream_manager_v1_destroy(stream_manager);
    wl_display_flush(manager->ctx->display);
}

static void stream_handle_failed(void *data, struct kywc_stream_v1 *kywc_stream_v1)
{
    struct ky_stream *stream = data;
    ky_stream_destroy(stream);
}

static void stream_handle_cancelled(void *data, struct kywc_stream_v1 *kywc_stream_v1)
{
    struct ky_stream *stream = data;
    ky_stream_destroy(stream);
}

static void stream_handle_created(void *data, struct kywc_stream_v1 *kywc_stream_v1, uint32_t node)
{
    struct ky_stream *stream = data;
    ky_stream_ready(stream, node);
}

static const struct kywc_stream_v1_listener stream_listener = {
    .failed = stream_handle_failed,
    .cancelled = stream_handle_cancelled,
    .created = stream_handle_created,
};

static void stream_destroy(struct ky_stream *stream)
{
    struct kywc_stream_v1 *kywc_stream_v1 = stream->data;
    kywc_stream_v1_destroy(kywc_stream_v1);
    wl_display_flush(stream->manager->ctx->display);
}

static void manager_stream_output(struct ky_stream_manager *manager, struct ky_stream *stream,
                                  const char *uuid, enum kywc_stream_cursor_mode cursor_mode)
{
    struct kywc_stream_manager_v1 *stream_manager = manager->data;
    struct kywc_stream_v1 *kywc_stream_v1 =
        kywc_stream_manager_v1_stream_output(stream_manager, uuid, cursor_mode);
    kywc_stream_v1_add_listener(kywc_stream_v1, &stream_listener, stream);
    wl_display_flush(manager->ctx->display);
    stream->destroy = stream_destroy;
    stream->data = kywc_stream_v1;
}

static void manager_stream_toplevel(struct ky_stream_manager *manager, struct ky_stream *stream,
                                    const char *uuid, enum kywc_stream_cursor_mode cursor_mode)
{
    struct kywc_stream_manager_v1 *stream_manager = manager->data;
    struct kywc_stream_v1 *kywc_stream_v1 =
        kywc_stream_manager_v1_stream_toplevel(stream_manager, uuid, cursor_mode);
    kywc_stream_v1_add_listener(kywc_stream_v1, &stream_listener, stream);
    wl_display_flush(manager->ctx->display);
    stream->destroy = stream_destroy;
    stream->data = kywc_stream_v1;
}

static bool stream_provider_bind(struct ky_context_provider *provider, struct wl_registry *registry,
                                 uint32_t name, const char *interface, uint32_t version)
{
    if (strcmp(interface, kywc_stream_manager_v1_interface.name) != 0) {
        return false;
    }

    uint32_t version_to_bind = version <= 1 ? version : 1;
    struct kywc_stream_manager_v1 *stream_manager =
        wl_registry_bind(registry, name, &kywc_stream_manager_v1_interface, version_to_bind);

    struct ky_stream_manager *manager = provider->data;
    kywc_stream_manager_v1_set_user_data(stream_manager, manager);

    manager->stream_output = manager_stream_output;
    manager->stream_toplevel = manager_stream_toplevel;
    manager->destroy = manager_destroy;
    manager->data = stream_manager;

    return true;
}

static void stream_provider_destroy(struct ky_context_provider *provider)
{
    struct ky_stream_manager *manager = provider->data;
    ky_stream_manager_destroy(manager);
    free(provider);
}

bool _kywc_stream_init(kywc_context *ctx, enum kywc_context_capability capability)
{
    struct ky_context_provider *provider = calloc(1, sizeof(*provider));
    if (!provider) {
        return false;
    }

    wl_list_init(&provider->link);
    provider->capability = capability;
    provider->bind = stream_provider_bind;
    provider->destroy = stream_provider_destroy;

    struct ky_stream_manager *manager = ky_stream_manager_create(ctx);
    if (!manager) {
        free(provider);
        return false;
    }

    provider->data = manager;

    if (!ky_context_add_provider(ctx, provider, manager)) {
        free(manager);
        free(provider);
        return false;
    }

    return true;
}
