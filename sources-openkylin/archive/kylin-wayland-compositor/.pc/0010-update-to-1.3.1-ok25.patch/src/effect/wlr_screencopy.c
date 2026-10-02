// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <drm_fourcc.h>
#include <stdlib.h>

#include <wlr/render/allocator.h>
#include <wlr/util/region.h>

#include <kywc/log.h>

#include "effect/capture.h"
#include "effect_p.h"
#include "output.h"
#include "render/pixel_format.h"

#include "wlr-screencopy-unstable-v1-protocol.h"

struct wlr_screencopy_manager {
    struct wl_global *global;

    struct wl_listener display_destroy;
    struct wl_listener server_destroy;
};

struct wlr_screencopy_client {
    int ref;
    bool first_damage_handled;
};

struct wlr_screencopy_frame {
    struct wl_resource *resource;
    struct wlr_screencopy_client *client;

    struct wlr_output *output;
    bool overlay_cursor, with_damage;

    uint32_t shm_format, dmabuf_format; // DRM format codes
    int width, height, stride;
    struct wlr_buffer *buffer;

    struct capture *capture; // TODO: reuse capture in client
};

static const struct zwlr_screencopy_frame_v1_interface wlr_screencopy_frame_impl;

static struct wlr_screencopy_frame *frame_from_resource(struct wl_resource *resource)
{
    assert(wl_resource_instance_of(resource, &zwlr_screencopy_frame_v1_interface,
                                   &wlr_screencopy_frame_impl));
    return wl_resource_get_user_data(resource);
}

static const struct zwlr_screencopy_manager_v1_interface wlr_screencopy_manager_impl;

static struct wlr_screencopy_client *client_from_resource(struct wl_resource *resource)
{
    assert(wl_resource_instance_of(resource, &zwlr_screencopy_manager_v1_interface,
                                   &wlr_screencopy_manager_impl));
    return wl_resource_get_user_data(resource);
}

static void client_unref(struct wlr_screencopy_client *client)
{
    assert(client->ref > 0);
    if (--client->ref != 0) {
        return;
    }

    free(client);
}

static void frame_destroy(struct wlr_screencopy_frame *frame)
{
    if (!frame) {
        return;
    }
    if (frame->capture) {
        capture_destroy(frame->capture);
    }
    // Make the frame resource inert
    wl_resource_set_user_data(frame->resource, NULL);
    wlr_buffer_unlock(frame->buffer);
    client_unref(frame->client);
    free(frame);
}

static void frame_handle_copy(struct wl_client *wl_client, struct wl_resource *frame_resource,
                              struct wl_resource *buffer_resource)
{
    struct wlr_screencopy_frame *frame = frame_from_resource(frame_resource);
    if (!frame) {
        return;
    }

    struct wlr_output *output = frame->output;
    if (!output->enabled) {
        zwlr_screencopy_frame_v1_send_failed(frame->resource);
        frame_destroy(frame);
        return;
    }

    struct wlr_buffer *buffer = wlr_buffer_try_from_resource(buffer_resource);
    if (!buffer) {
        wl_resource_post_error(frame->resource, ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER,
                               "invalid buffer");
        return;
    }

    if (buffer->width != frame->width || buffer->height != frame->height) {
        wl_resource_post_error(frame->resource, ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER,
                               "invalid buffer dimensions");
        return;
    }

    if (frame->buffer) {
        wl_resource_post_error(frame->resource, ZWLR_SCREENCOPY_FRAME_V1_ERROR_ALREADY_USED,
                               "frame already used");
        return;
    }

    struct wlr_dmabuf_attributes dmabuf;
    void *data;
    uint32_t format;
    size_t stride;
    if (wlr_buffer_get_dmabuf(buffer, &dmabuf)) {
        if (dmabuf.format != frame->dmabuf_format) {
            wl_resource_post_error(frame->resource, ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER,
                                   "invalid buffer format");
            return;
        }
    } else if (wlr_buffer_begin_data_ptr_access(buffer, WLR_BUFFER_DATA_PTR_ACCESS_WRITE, &data,
                                                &format, &stride)) {
        wlr_buffer_end_data_ptr_access(buffer);
        if (format != frame->shm_format) {
            wl_resource_post_error(frame->resource, ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER,
                                   "invalid buffer format");
            return;
        }
        if (stride != (size_t)frame->stride) {
            wl_resource_post_error(frame->resource, ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER,
                                   "invalid buffer stride");
            return;
        }
    } else {
        wl_resource_post_error(frame->resource, ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER,
                               "unsupported buffer type");
        return;
    }

    frame->buffer = buffer;

    bool first_copy_with_damage = false;
    if (frame->with_damage && !frame->client->first_damage_handled) {
        frame->client->first_damage_handled = true;
        first_copy_with_damage = true;
    }
    capture_mark_wants_update(frame->capture, true, !frame->with_damage || first_copy_with_damage);
}

static void frame_handle_copy_with_damage(struct wl_client *wl_client,
                                          struct wl_resource *frame_resource,
                                          struct wl_resource *buffer_resource)
{
    struct wlr_screencopy_frame *frame = frame_from_resource(frame_resource);
    if (!frame) {
        return;
    }
    frame->with_damage = true;
    frame_handle_copy(wl_client, frame_resource, buffer_resource);
}

static void frame_handle_destroy(struct wl_client *wl_client, struct wl_resource *frame_resource)
{
    wl_resource_destroy(frame_resource);
}

static const struct zwlr_screencopy_frame_v1_interface wlr_screencopy_frame_impl = {
    .copy = frame_handle_copy,
    .destroy = frame_handle_destroy,
    .copy_with_damage = frame_handle_copy_with_damage,
};

static void frame_handle_resource_destroy(struct wl_resource *frame_resource)
{
    struct wlr_screencopy_frame *frame = frame_from_resource(frame_resource);
    frame_destroy(frame);
}

static enum wl_shm_format convert_drm_format_to_wl_shm(uint32_t fmt)
{
    switch (fmt) {
    case DRM_FORMAT_XRGB8888:
        return WL_SHM_FORMAT_XRGB8888;
    case DRM_FORMAT_ARGB8888:
        return WL_SHM_FORMAT_ARGB8888;
    default:
        return (enum wl_shm_format)fmt;
    }
}

static void frame_handle_capture_update(struct capture *capture, struct wlr_buffer *buffer,
                                        bool buffer_changed, void *data)
{
    struct wlr_screencopy_frame *frame = data;

    zwlr_screencopy_frame_v1_send_flags(frame->resource, 0);

    if (frame->with_damage) {
        zwlr_screencopy_frame_v1_send_damage(frame->resource, 0, 0, frame->width, frame->height);
    }

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    time_t tv_sec = now.tv_sec;
    uint32_t tv_sec_hi = (sizeof(tv_sec) > 4) ? tv_sec >> 32 : 0;
    uint32_t tv_sec_lo = tv_sec & 0xFFFFFFFF;
    zwlr_screencopy_frame_v1_send_ready(frame->resource, tv_sec_hi, tv_sec_lo, now.tv_nsec);

    frame_destroy(frame);
}

static void frame_handle_capture_cancel(struct capture *capture, void *data)
{
    struct wlr_screencopy_frame *frame = data;
    frame->capture = NULL;
    zwlr_screencopy_frame_v1_send_failed(frame->resource);
    frame_destroy(frame);
}

static struct wlr_buffer *frame_handle_capture_buffer(struct capture *capture,
                                                      struct wlr_buffer *buffer, int width,
                                                      int height, bool single_plane, void *data)
{
    struct wlr_screencopy_frame *frame = data;
    return frame->buffer;
}

static const struct capture_interface frame_impl = {
    .update = frame_handle_capture_update,
    .cancel = frame_handle_capture_cancel,
    .buffer = frame_handle_capture_buffer,
};

static void capture_output(struct wl_client *wl_client, struct wlr_screencopy_client *client,
                           uint32_t version, uint32_t id, int32_t overlay_cursor,
                           struct wlr_output *wlr_output, struct kywc_box *box)
{
    struct wlr_screencopy_frame *frame = calloc(1, sizeof(*frame));
    if (!frame) {
        wl_client_post_no_memory(wl_client);
        return;
    }

    frame->resource =
        wl_resource_create(wl_client, &zwlr_screencopy_frame_v1_interface, version, id);
    if (!frame->resource) {
        free(frame);
        wl_client_post_no_memory(wl_client);
        return;
    }

    frame->output = wlr_output;
    frame->overlay_cursor = !!overlay_cursor;
    wl_resource_set_implementation(frame->resource, &wlr_screencopy_frame_impl, frame,
                                   frame_handle_resource_destroy);

    if (!wlr_output || !wlr_output->enabled) {
        wl_resource_set_user_data(frame->resource, NULL);
        zwlr_screencopy_frame_v1_send_failed(frame->resource);
        free(frame);
        return;
    }

    frame->client = client;
    frame->client->ref++;

    frame->shm_format = wlr_output_preferred_read_format(frame->output);
    if (frame->shm_format == DRM_FORMAT_INVALID) {
        kywc_log(KYWC_ERROR, "Failed to capture output: no read format supported by renderer");
        goto error;
    }

    const struct pixel_format *shm_info = pixel_format_from_drm(frame->shm_format);
    if (!shm_info) {
        kywc_log(KYWC_ERROR, "Failed to capture output: no pixel format info matching read format");
        goto error;
    }

    if (wlr_output->allocator && (wlr_output->allocator->buffer_caps & WLR_BUFFER_CAP_DMABUF)) {
        frame->dmabuf_format = wlr_output->render_format;
    } else {
        frame->dmabuf_format = DRM_FORMAT_INVALID;
    }

    uint32_t options = CAPTURE_NEED_UNSCALED | CAPTURE_PREFER_OUTPUT_SCALE;
    options |= frame->overlay_cursor ? CAPTURE_NEED_CURSOR : CAPTURE_NEED_NONE;
    struct output *output = output_from_wlr_output(wlr_output);

    if (box) {
        box->x += output->geometry.x;
        box->y += output->geometry.y;
        if (kywc_box_intersection(box, box, &output->geometry)) {
            box->x *= wlr_output->scale;
            box->y *= wlr_output->scale;
            box->width *= wlr_output->scale;
            box->height *= wlr_output->scale;
            frame->capture = capture_create_from_area(
                &(struct wlr_box){ box->x, box->y, box->width, box->height }, options, &frame_impl,
                frame);
        }
    } else {
        frame->capture = capture_create_from_output(output, options, &frame_impl, frame);
    }
    if (!frame->capture) {
        goto error;
    }

    /* wants update in frame_handle_copy */
    capture_mark_wants_update(frame->capture, false, false);

    capture_get_buffer_size(frame->capture, &frame->width, &frame->height);
    frame->stride = pixel_format_min_stride(shm_info, frame->width);

    zwlr_screencopy_frame_v1_send_buffer(frame->resource,
                                         convert_drm_format_to_wl_shm(frame->shm_format),
                                         frame->width, frame->height, frame->stride);
    if (version >= 3) {
        if (frame->dmabuf_format != DRM_FORMAT_INVALID) {
            zwlr_screencopy_frame_v1_send_linux_dmabuf(frame->resource, frame->dmabuf_format,
                                                       frame->width, frame->height);
        }
        zwlr_screencopy_frame_v1_send_buffer_done(frame->resource);
    }

    return;

error:
    zwlr_screencopy_frame_v1_send_failed(frame->resource);
    frame_destroy(frame);
}

static void manager_handle_capture_output(struct wl_client *wl_client,
                                          struct wl_resource *manager_resource, uint32_t id,
                                          int32_t overlay_cursor,
                                          struct wl_resource *output_resource)
{
    struct wlr_screencopy_client *client = client_from_resource(manager_resource);
    uint32_t version = wl_resource_get_version(manager_resource);
    struct wlr_output *output = wlr_output_from_resource(output_resource);

    capture_output(wl_client, client, version, id, overlay_cursor, output, NULL);
}

static void manager_handle_capture_output_region(struct wl_client *wl_client,
                                                 struct wl_resource *manager_resource, uint32_t id,
                                                 int32_t overlay_cursor,
                                                 struct wl_resource *output_resource, int32_t x,
                                                 int32_t y, int32_t width, int32_t height)
{
    struct wlr_screencopy_client *client = client_from_resource(manager_resource);
    uint32_t version = wl_resource_get_version(manager_resource);
    struct wlr_output *output = wlr_output_from_resource(output_resource);

    struct kywc_box box = { x, y, width, height };
    capture_output(wl_client, client, version, id, overlay_cursor, output, &box);
}

static void manager_handle_destroy(struct wl_client *wl_client,
                                   struct wl_resource *manager_resource)
{
    wl_resource_destroy(manager_resource);
}

static const struct zwlr_screencopy_manager_v1_interface wlr_screencopy_manager_impl = {
    .capture_output = manager_handle_capture_output,
    .capture_output_region = manager_handle_capture_output_region,
    .destroy = manager_handle_destroy,
};

static void manager_handle_resource_destroy(struct wl_resource *resource)
{
    struct wlr_screencopy_client *client = client_from_resource(resource);
    client_unref(client);
}

static void wlr_screencopy_manager_bind(struct wl_client *wl_client, void *data, uint32_t version,
                                        uint32_t id)
{
    struct wlr_screencopy_client *client = calloc(1, sizeof(*client));
    if (!client) {
        wl_client_post_no_memory(wl_client);
        return;
    }

    struct wl_resource *resource =
        wl_resource_create(wl_client, &zwlr_screencopy_manager_v1_interface, version, id);
    if (!resource) {
        free(client);
        wl_client_post_no_memory(wl_client);
        return;
    }

    client->ref = 1;
    client->first_damage_handled = false;

    wl_resource_set_implementation(resource, &wlr_screencopy_manager_impl, client,
                                   manager_handle_resource_destroy);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    struct wlr_screencopy_manager *manager = wl_container_of(listener, manager, server_destroy);
    wl_list_remove(&manager->server_destroy.link);
    free(manager);
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    struct wlr_screencopy_manager *manager = wl_container_of(listener, manager, display_destroy);
    wl_list_remove(&manager->display_destroy.link);
    wl_global_destroy(manager->global);
}

bool wlr_screencopy_manager_create(struct server *server)
{
    struct wlr_screencopy_manager *manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    manager->global = wl_global_create(server->display, &zwlr_screencopy_manager_v1_interface, 3,
                                       manager, wlr_screencopy_manager_bind);
    if (!manager->global) {
        kywc_log(KYWC_WARN, "Wlr screencopy manager create failed");
        free(manager);
        return false;
    }

    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &manager->server_destroy);
    manager->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(server->display, &manager->display_destroy);

    return true;
}
