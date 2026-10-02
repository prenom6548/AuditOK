// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "kywc-capture-v1-client-protocol.h"

#include "libkywc_p.h"

bool _kywc_capture_init(kywc_context *ctx, enum kywc_context_capability capability);

static void frame_handle_failed(void *data, struct kywc_capture_frame_v1 *kywc_capture_frame_v1)
{
    struct ky_thumbnail *thumbnail = data;
    ky_thumbnail_destroy(thumbnail);
}

static void frame_handle_cancelled(void *data, struct kywc_capture_frame_v1 *kywc_capture_frame_v1)
{
    struct ky_thumbnail *thumbnail = data;
    ky_thumbnail_destroy(thumbnail);
}

static void frame_release(struct ky_thumbnail *thumbnail, bool wants_buffer)
{
    struct kywc_capture_frame_v1 *frame = thumbnail->data;
    kywc_capture_frame_v1_release_buffer(frame, wants_buffer);
    wl_display_flush(thumbnail->manager->ctx->display);

    for (uint32_t i = 0; i < thumbnail->buffer.n_planes; i++) {
        close(thumbnail->buffer.planes[i].fd);
    }
    thumbnail->buffer.n_planes = 0;

    if (!wants_buffer) {
        ky_thumbnail_destroy(thumbnail);
    }
}

static void frame_handle_buffer(void *data, struct kywc_capture_frame_v1 *kywc_capture_frame_v1,
                                int32_t fd, uint32_t format, uint32_t width, uint32_t height,
                                uint32_t offset, uint32_t stride, uint32_t modifier_hi,
                                uint32_t modifier_lo, uint32_t flags)
{
    struct ky_thumbnail *thumbnail = data;
    bool want_buffer = false;

    thumbnail->buffer = (struct kywc_thumbnail_buffer){
        .fd = fd,
        .format = format,
        .width = width,
        .height = height,
        .offset = offset,
        .stride = stride,
        .modifier = (uint64_t)modifier_hi << 32 | modifier_lo,
        .flags = flags,
        .n_planes = 1,
        .planes[0] = { fd, offset, stride },
    };

    uint32_t version = kywc_capture_frame_v1_get_version(kywc_capture_frame_v1);
    if (version >= KYWC_CAPTURE_FRAME_V1_BUFFER_DONE_SINCE_VERSION) {
        return;
    }

    if (!ky_thumbnail_update_buffer(thumbnail, &thumbnail->buffer, &want_buffer)) {
        frame_release(thumbnail, want_buffer);
    }
}

static void frame_handle_buffer_with_plane(void *data,
                                           struct kywc_capture_frame_v1 *kywc_capture_frame_v1,
                                           uint32_t index, int32_t fd, uint32_t offset,
                                           uint32_t stride)
{
    struct ky_thumbnail *thumbnail = data;
    thumbnail->buffer.planes[index].fd = fd;
    thumbnail->buffer.planes[index].offset = offset;
    thumbnail->buffer.planes[index].stride = stride;
    thumbnail->buffer.n_planes = index + 1;
}

static void frame_handle_buffer_done(void *data,
                                     struct kywc_capture_frame_v1 *kywc_capture_frame_v1)
{
    struct ky_thumbnail *thumbnail = data;
    bool want_buffer = false;

    if (!ky_thumbnail_update_buffer(thumbnail, &thumbnail->buffer, &want_buffer)) {
        frame_release(thumbnail, want_buffer);
        return;
    }
}

static const struct kywc_capture_frame_v1_listener frame_listener = {
    .failed = frame_handle_failed,
    .cancelled = frame_handle_cancelled,
    .buffer = frame_handle_buffer,
    .buffer_with_plane = frame_handle_buffer_with_plane,
    .buffer_done = frame_handle_buffer_done,
};

static void frame_destroy(struct ky_thumbnail *thumbnail)
{
    struct kywc_capture_frame_v1 *frame = thumbnail->data;
    kywc_capture_frame_v1_destroy(frame);
    wl_display_flush(thumbnail->manager->ctx->display);
    /* release may not be called before destroy */
    for (uint32_t i = 0; i < thumbnail->buffer.n_planes; i++) {
        close(thumbnail->buffer.planes[i].fd);
    }
}

static void manager_capture_output(struct ky_thumbnail_manager *manager,
                                   struct ky_thumbnail *thumbnail, const char *uuid)
{
    struct kywc_capture_manager_v1 *thumbnail_manager = manager->data;
    struct kywc_capture_frame_v1 *frame =
        kywc_capture_manager_v1_capture_output(thumbnail_manager, 0, uuid);
    kywc_capture_frame_v1_add_listener(frame, &frame_listener, thumbnail);
    wl_display_flush(manager->ctx->display);
    thumbnail->release = frame_release;
    thumbnail->destroy = frame_destroy;
    thumbnail->data = frame;
}

static void manager_capture_workspace(struct ky_thumbnail_manager *manager,
                                      struct ky_thumbnail *thumbnail, const char *uuid,
                                      const char *output)
{
    struct kywc_capture_manager_v1 *thumbnail_manager = manager->data;
    struct kywc_capture_frame_v1 *frame =
        kywc_capture_manager_v1_capture_workspace(thumbnail_manager, uuid, output);
    kywc_capture_frame_v1_add_listener(frame, &frame_listener, thumbnail);
    wl_display_flush(manager->ctx->display);
    thumbnail->release = frame_release;
    thumbnail->destroy = frame_destroy;
    thumbnail->data = frame;
}

static void manager_capture_toplevel(struct ky_thumbnail_manager *manager,
                                     struct ky_thumbnail *thumbnail, const char *uuid,
                                     bool without_decoration)
{
    struct kywc_capture_manager_v1 *thumbnail_manager = manager->data;
    struct kywc_capture_frame_v1 *frame =
        kywc_capture_manager_v1_capture_toplevel(thumbnail_manager, uuid, without_decoration);
    kywc_capture_frame_v1_add_listener(frame, &frame_listener, thumbnail);
    wl_display_flush(manager->ctx->display);
    thumbnail->release = frame_release;
    thumbnail->destroy = frame_destroy;
    thumbnail->data = frame;
}

static void cursor_handle_enter(void *data, struct kywc_capture_cursor_v1 *kywc_capture_cursor_v1)
{
    struct ky_cursor *cursor = data;
    ky_cursor_enter(cursor);
}

static void cursor_handle_leave(void *data, struct kywc_capture_cursor_v1 *kywc_capture_cursor_v1)
{
    struct ky_cursor *cursor = data;
    ky_cursor_leave(cursor);
}

static void cursor_handle_position(void *data,
                                   struct kywc_capture_cursor_v1 *kywc_capture_cursor_v1, int32_t x,
                                   int32_t y)
{
    struct ky_cursor *cursor = data;
    ky_cursor_update_position(cursor, x, y);
}

static void cursor_handle_hotspot(void *data, struct kywc_capture_cursor_v1 *kywc_capture_cursor_v1,
                                  int32_t x, int32_t y)
{
    struct ky_cursor *cursor = data;
    ky_cursor_update_hotspot(cursor, x, y);
}

static const struct kywc_capture_cursor_v1_listener cursor_listener = {
    .enter = cursor_handle_enter,
    .leave = cursor_handle_leave,
    .position = cursor_handle_position,
    .hotspot = cursor_handle_hotspot,
};

static void cursor_destroy(struct ky_cursor *cursor)
{
    struct kywc_capture_cursor_v1 *p = cursor->data;
    kywc_capture_cursor_v1_destroy(p);
    wl_display_flush(cursor->manager->ctx->display);
}

static void manager_create_cursor(struct ky_cursor_manager *manager, struct ky_cursor *cursor,
                                  struct wl_seat *seat, struct ky_thumbnail *thumbnail)
{
    struct kywc_capture_manager_v1 *capture_manager = manager->data;
    struct kywc_capture_cursor_v1 *p = kywc_capture_manager_v1_capture_cursor(
        capture_manager, seat, thumbnail ? thumbnail->data : NULL);
    kywc_capture_cursor_v1_add_listener(p, &cursor_listener, cursor);
    cursor->manager = manager;
    cursor->destroy = cursor_destroy;
    cursor->data = p;
}

static void manager_thumbnail_destroy(struct ky_thumbnail_manager *manager)
{
    struct kywc_capture_manager_v1 *thumbnail_manager = manager->data;
    kywc_capture_manager_v1_destroy(thumbnail_manager);
    wl_display_flush(manager->ctx->display);
}

static void manager_cursor_destroy(struct ky_cursor_manager *manager)
{
    struct kywc_capture_manager_v1 *cursor_manager = manager->data;
    kywc_capture_manager_v1_destroy(cursor_manager);
    wl_display_flush(manager->ctx->display);
}

static bool capture_provider_bind(struct ky_context_provider *provider,
                                  struct wl_registry *registry, uint32_t name,
                                  const char *interface, uint32_t version)
{
    if (strcmp(interface, kywc_capture_manager_v1_interface.name)) {
        return false;
    }

    bool extend = provider->capability == KYWC_CONTEXT_CAPABILITY_THUMBNAIL_EXT;
    uint32_t version_to_bind = (version > 1 && extend) ? version : 1;

    struct kywc_capture_manager_v1 *capture_manager =
        wl_registry_bind(registry, name, &kywc_capture_manager_v1_interface, version_to_bind);

    void *manager = provider->data;
    kywc_capture_manager_v1_set_user_data(capture_manager, manager);

    if (provider->capability == KYWC_CONTEXT_CAPABILITY_CURSOR) {
        struct ky_cursor_manager *cursor_manager = manager;
        cursor_manager->create_cursor = manager_create_cursor;
        cursor_manager->destroy = manager_cursor_destroy;
        cursor_manager->data = capture_manager;
    } else {
        struct ky_thumbnail_manager *thumbnail_manager = manager;
        thumbnail_manager->capture_output = manager_capture_output;
        thumbnail_manager->capture_workspace = manager_capture_workspace;
        thumbnail_manager->capture_toplevel = manager_capture_toplevel;
        thumbnail_manager->destroy = manager_thumbnail_destroy;
        thumbnail_manager->data = capture_manager;
    }

    return true;
}

static void capture_provider_destroy(struct ky_context_provider *provider)
{
    if (provider->capability == KYWC_CONTEXT_CAPABILITY_CURSOR) {
        struct ky_cursor_manager *manager = provider->data;
        ky_cursor_manager_destroy(manager);
    } else {
        struct ky_thumbnail_manager *manager = provider->data;
        ky_thumbnail_manager_destroy(manager);
    }

    free(provider);
}

bool _kywc_capture_init(kywc_context *ctx, enum kywc_context_capability capability)
{
    struct ky_context_provider *provider = calloc(1, sizeof(*provider));
    if (!provider) {
        return false;
    }

    wl_list_init(&provider->link);
    provider->capability = capability;
    provider->bind = capture_provider_bind;
    provider->destroy = capture_provider_destroy;

    void *manager = NULL;
    if (provider->capability == KYWC_CONTEXT_CAPABILITY_CURSOR) {
        manager = ky_cursor_manager_create(ctx);
    } else {
        manager = ky_thumbnail_manager_create(ctx);
    }

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
