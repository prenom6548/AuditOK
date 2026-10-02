// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>

#include "libkywc_p.h"

struct ky_stream *stream_from_kywc_stream(kywc_stream *kywc_stream)
{
    struct ky_stream *stream = wl_container_of(kywc_stream, stream, base);
    return stream;
}

void ky_stream_destroy(struct ky_stream *stream)
{
    if (stream->impl && stream->impl->destroy) {
        stream->impl->destroy(&stream->base, stream->user_data);
    }

    if (stream->destroy) {
        stream->destroy(stream);
    }

    wl_list_remove(&stream->link);

    free((void *)stream->base.source_uuid);
    free(stream);
}

void ky_stream_ready(struct ky_stream *stream, uint32_t node)
{
    if (stream->impl && stream->impl->ready) {
        stream->impl->ready(&stream->base, node, stream->user_data);
    }
}

struct ky_stream_manager *ky_stream_manager_create(kywc_context *ctx)
{
    struct ky_stream_manager *manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return NULL;
    }

    manager->ctx = ctx;
    wl_list_init(&manager->streams);

    return manager;
}

void ky_stream_manager_destroy(struct ky_stream_manager *manager)
{
    if (!manager) {
        return;
    }

    // destroy all streams
    struct ky_stream *stream, *tmp;
    wl_list_for_each_safe(stream, tmp, &manager->streams, link) {
        ky_stream_destroy(stream);
    }

    if (manager->destroy) {
        manager->destroy(manager);
    }

    free(manager);
}

static kywc_stream *kywc_stream_create(kywc_context *ctx, enum kywc_stream_type type,
                                       const char *source_uuid,
                                       enum kywc_stream_cursor_mode cursor_mode,
                                       const struct kywc_stream_interface *impl, void *data)
{
    if (!ctx || !ctx->stream) {
        return NULL;
    }

    struct ky_stream *stream = calloc(1, sizeof(*stream));
    if (!stream) {
        return NULL;
    }

    struct ky_stream_manager *manager = ctx->stream;
    stream->manager = manager;
    wl_list_insert(&manager->streams, &stream->link);

    stream->base.type = type;
    stream->base.source_uuid = strdup(source_uuid);
    stream->base.cursor_mode = cursor_mode;

    stream->impl = impl;
    stream->user_data = data;

    if (type == KYWC_STREAM_TYPE_OUTPUT && manager->stream_output) {
        manager->stream_output(manager, stream, source_uuid, cursor_mode);
    } else if (type == KYWC_STREAM_TYPE_TOPLEVEL && manager->stream_toplevel) {
        manager->stream_toplevel(manager, stream, source_uuid, cursor_mode);
    }

    return &stream->base;
}

kywc_stream *kywc_stream_create_from_output(kywc_context *ctx, const char *source_uuid,
                                            enum kywc_stream_cursor_mode cursor_mode,
                                            const struct kywc_stream_interface *impl, void *data)
{
    return kywc_stream_create(ctx, KYWC_STREAM_TYPE_OUTPUT, source_uuid, cursor_mode, impl, data);
}

kywc_stream *kywc_stream_create_from_toplevel(kywc_context *ctx, const char *source_uuid,
                                              enum kywc_stream_cursor_mode cursor_mode,
                                              const struct kywc_stream_interface *impl, void *data)
{
    return kywc_stream_create(ctx, KYWC_STREAM_TYPE_TOPLEVEL, source_uuid, cursor_mode, impl, data);
}

kywc_context *kywc_stream_get_context(kywc_stream *stream)
{
    struct ky_stream *ky_stream = stream_from_kywc_stream(stream);
    return ky_stream->manager->ctx;
}

void kywc_stream_set_user_data(kywc_stream *stream, void *data)
{
    struct ky_stream *ky_stream = stream_from_kywc_stream(stream);
    ky_stream->user_data = data;
}

void *kywc_stream_get_user_data(kywc_stream *stream)
{
    struct ky_stream *ky_stream = stream_from_kywc_stream(stream);
    return ky_stream->user_data;
}

void kywc_stream_destroy(kywc_stream *stream)
{
    struct ky_stream *ky_stream = stream_from_kywc_stream(stream);
    ky_stream_destroy(ky_stream);
}
