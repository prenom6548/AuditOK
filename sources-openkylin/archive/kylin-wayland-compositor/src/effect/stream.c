// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <drm_fourcc.h>

#include <pipewire/pipewire.h>
#include <spa/param/video/format-utils.h>
#include <spa/pod/dynamic.h>

#include <kywc/identifier.h>
#include <kywc/log.h>

#include "effect/capture.h"
#include "effect/stream.h"
#include "effect_p.h"
#include "render/allocator.h"
#include "render/renderer.h"
#include "scene/thumbnail.h"
#include "util/drm_format.h"
#include "util/macros.h"
#include "util/time.h"

#define DEBUG 0
#if DEBUG
#define debug(fmt, ...) kywc_log(KYWC_INFO, fmt, ##__VA_ARGS__)
#else
#define debug(fmt, ...) ((void)0)
#endif

#define MAX_PARAMS 8 // 2 format with 2 type (shm and dmabuf)

#define BUFFER_FLAGS (ALLOCATOR_BUFFER_RENDERING | ALLOCATOR_BUFFER_NO_SCANOUT)
//(ALLOCATOR_BUFFER_RENDERING | ALLOCATOR_BUFFER_NO_SCANOUT | ALLOCATOR_BUFFER_SINGLE_PLANE)

struct stream {
    struct wl_list link;

    const struct stream_interface *impl;
    void *user_data;

    struct pw_stream *stream;
    struct spa_hook stream_listener;
    struct spa_video_info_raw format;

    struct capture *capture;
    struct thumbnail *thumbnail;

    struct wl_event_source *timer;

    struct swapchain *buffers;
    struct pw_buffer *current;
    struct wlr_dmabuf_attributes dmabuf_attrs;

    int width, height;
    uint32_t max_framerate;
    uint64_t seq;
    uint64_t dequeue, queue;

    bool avoid_dmabufs;
    bool streaming;
};

struct pipewire_context {
    struct pw_loop *loop;
    struct pw_context *context;
    struct pw_core *core;
    struct spa_hook core_listener;

    struct wl_event_source *event;
};

static struct stream_manager {
    struct wl_list streams;

    struct server *server;
    struct wl_listener display_destroy;
    struct wl_listener server_destroy;

    struct wlr_drm_format_set formats;

    struct pipewire_context *context;
} *manager = NULL;

static struct pipewire_context *get_pipewire_context(void);

static enum spa_video_format pw_format_from_drm_fourcc(uint32_t format)
{
    switch (format) {
    case DRM_FORMAT_ARGB8888:
        return SPA_VIDEO_FORMAT_BGRA;
    case DRM_FORMAT_XRGB8888:
        return SPA_VIDEO_FORMAT_BGRx;
    default:
        return SPA_VIDEO_FORMAT_UNKNOWN;
    }
}

static uint32_t drm_fourcc_from_pw_format(enum spa_video_format format)
{
    switch (format) {
    case SPA_VIDEO_FORMAT_BGRA:
        return DRM_FORMAT_ARGB8888;
    case SPA_VIDEO_FORMAT_BGRx:
        return DRM_FORMAT_XRGB8888;
    default:
        return DRM_FORMAT_INVALID;
    }
}

static enum spa_video_format pw_format_strip_alpha(enum spa_video_format format)
{
    switch (format) {
    case SPA_VIDEO_FORMAT_BGRA:
        return SPA_VIDEO_FORMAT_BGRx;
    default:
        return SPA_VIDEO_FORMAT_UNKNOWN;
    }
}

static struct spa_pod *build_buffer(struct spa_pod_builder *b, uint32_t blocks, uint32_t size,
                                    uint32_t stride, uint32_t buffer_type)
{
    struct spa_pod_frame f[1];

    spa_pod_builder_push_object(b, &f[0], SPA_TYPE_OBJECT_ParamBuffers, SPA_PARAM_Buffers);
    spa_pod_builder_add(b, SPA_PARAM_BUFFERS_buffers, SPA_POD_CHOICE_RANGE_Int(3, 2, 4), 0);
    spa_pod_builder_add(b, SPA_PARAM_BUFFERS_dataType, SPA_POD_CHOICE_FLAGS_Int(buffer_type), 0);
    spa_pod_builder_add(b, SPA_PARAM_BUFFERS_blocks, SPA_POD_Int(blocks), 0);
    if (size > 0) {
        spa_pod_builder_add(b, SPA_PARAM_BUFFERS_size, SPA_POD_Int(size), 0);
    }
    if (stride > 0) {
        spa_pod_builder_add(b, SPA_PARAM_BUFFERS_stride, SPA_POD_Int(stride), 0);
    }
    spa_pod_builder_add(b, SPA_PARAM_BUFFERS_align, SPA_POD_Int(16), 0);
    return spa_pod_builder_pop(b, &f[0]);
}

static struct spa_pod *build_format(struct spa_pod_builder *b, enum spa_video_format format,
                                    uint32_t width, uint32_t height, uint32_t framerate,
                                    uint64_t *modifiers, size_t modifier_count)
{
    enum spa_video_format format_without_alpha = pw_format_strip_alpha(format);

    struct spa_pod_frame f[2];
    spa_pod_builder_push_object(b, &f[0], SPA_TYPE_OBJECT_Format, SPA_PARAM_EnumFormat);
    spa_pod_builder_add(b, SPA_FORMAT_mediaType, SPA_POD_Id(SPA_MEDIA_TYPE_video), 0);
    spa_pod_builder_add(b, SPA_FORMAT_mediaSubtype, SPA_POD_Id(SPA_MEDIA_SUBTYPE_raw), 0);
    spa_pod_builder_add(b, SPA_FORMAT_VIDEO_size, SPA_POD_Rectangle(&SPA_RECTANGLE(width, height)),
                        0);
    /*  0/1 means variable rate */
    spa_pod_builder_add(b, SPA_FORMAT_VIDEO_framerate, SPA_POD_Fraction(&SPA_FRACTION(0, 1)), 0);
    spa_pod_builder_add(b, SPA_FORMAT_VIDEO_maxFramerate,
                        SPA_POD_CHOICE_RANGE_Fraction(&SPA_FRACTION(framerate, 1),
                                                      &SPA_FRACTION(1, 1),
                                                      &SPA_FRACTION(framerate, 1)),
                        0);

    if (modifier_count > 0 || format_without_alpha == SPA_VIDEO_FORMAT_UNKNOWN) {
        // When modifiers are present, don't offer alpha-stripped variant
        spa_pod_builder_add(b, SPA_FORMAT_VIDEO_format, SPA_POD_Id(format), 0);
    } else {
        spa_pod_builder_add(b, SPA_FORMAT_VIDEO_format,
                            SPA_POD_CHOICE_ENUM_Id(3, format, format, format_without_alpha), 0);
    }

    // build an enumeration of modifiers
    if (modifier_count == 0) {
        return spa_pod_builder_pop(b, &f[0]);
    }

    if (modifier_count == 1) {
        spa_pod_builder_prop(b, SPA_FORMAT_VIDEO_modifier, SPA_POD_PROP_FLAG_MANDATORY);
        spa_pod_builder_long(b, modifiers[0]);
    } else {
        spa_pod_builder_prop(b, SPA_FORMAT_VIDEO_modifier,
                             SPA_POD_PROP_FLAG_MANDATORY | SPA_POD_PROP_FLAG_DONT_FIXATE);
        spa_pod_builder_push_choice(b, &f[1], SPA_CHOICE_Enum, 0);
        // modifiers from the array
        for (size_t i = 0, c = 0; i < modifier_count; i++) {
            spa_pod_builder_long(b, modifiers[i]);
            if (c++ == 0)
                spa_pod_builder_long(b, modifiers[i]);
        }
        spa_pod_builder_pop(b, &f[1]);
    }

    return spa_pod_builder_pop(b, &f[0]);
}

static uint32_t stream_build_formats(struct stream *stream, struct spa_pod_builder *builder,
                                     const struct spa_pod **params, uint32_t offset)
{
    uint32_t n_params = offset;

    for (size_t i = 0; i < manager->formats.len; i++) {
        struct wlr_drm_format *fmt = &manager->formats.formats[i];
        enum spa_video_format pw_format = pw_format_from_drm_fourcc(fmt->format);
        if (pw_format == SPA_VIDEO_FORMAT_UNKNOWN) {
            continue;
        }
        if (!stream->avoid_dmabufs && fmt->len > 0) {
            params[offset++] = build_format(builder, pw_format, stream->width, stream->height,
                                            stream->max_framerate, fmt->modifiers, fmt->len);
        }
        params[offset++] = build_format(builder, pw_format, stream->width, stream->height,
                                        stream->max_framerate, NULL, 0);
    }

    return offset - n_params;
}

static void handle_param_changed(void *data, uint32_t id, const struct spa_pod *param)
{
    if (!param || id != SPA_PARAM_Format) {
        return;
    }

    struct stream *stream = data;
    spa_format_video_raw_parse(param, &stream->format);
    stream->max_framerate = stream->format.max_framerate.num / stream->format.max_framerate.denom;

    uint32_t buffer_type;
    uint32_t blocks = 1;

    const struct spa_pod_prop *prop_modifier =
        spa_pod_find_prop(param, NULL, SPA_FORMAT_VIDEO_modifier);
    if (prop_modifier) {
        buffer_type = 1 << SPA_DATA_DmaBuf;
    } else {
        buffer_type = 1 << SPA_DATA_MemFd;
    }

    const struct spa_pod *params[MAX_PARAMS];
    uint32_t n_params = 0;
    uint8_t buffer[1024];
    struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

    if (prop_modifier) {
        uint32_t fourcc = drm_fourcc_from_pw_format(stream->format.format);
        /* fixate the dmabuf modifier */
        if (prop_modifier->flags & SPA_POD_PROP_FLAG_DONT_FIXATE) {
            uint32_t n_modifiers = SPA_POD_CHOICE_N_VALUES(&prop_modifier->value);
            uint64_t *modifiers = SPA_POD_CHOICE_VALUES(&prop_modifier->value);

            struct wlr_drm_format format;
            drm_format_init(&format, fourcc, modifiers, n_modifiers);

            struct wlr_buffer *buffer =
                allocator_create_buffer(manager->server->allocator, ALLOCATOR_BUFFER_TYPE_GBM,
                                        stream->width, stream->height, &format, BUFFER_FLAGS);
            wlr_drm_format_finish(&format);
            if (!buffer) {
                stream->avoid_dmabufs = true;
            } else {
                wlr_buffer_get_dmabuf(buffer, &stream->dmabuf_attrs);
                wlr_buffer_drop(buffer);
                params[n_params++] =
                    build_format(&b, stream->format.format, stream->width, stream->height,
                                 stream->max_framerate, &stream->dmabuf_attrs.modifier, 1);
            }

            n_params += stream_build_formats(stream, &b, params, n_params);
            pw_stream_update_params(stream->stream, params, n_params);
            return;
        } else {
            uint64_t modifier = stream->format.modifier;
            if (fourcc == stream->dmabuf_attrs.format &&
                modifier == stream->dmabuf_attrs.modifier) {
                blocks = stream->dmabuf_attrs.n_planes;
            }
        }
    }

    debug("pipewire: Format negotiated:");
    debug("     blocks = %d", blocks);
    debug("     buffer_type: %u", buffer_type);
    debug("     format: %u", stream->format.format);
    debug("     modifier: %lu", stream->format.modifier);
    debug("     size: (%u, %u)", stream->format.size.width, stream->format.size.height);
    debug("     max_framerate: (%u / %u)", stream->format.max_framerate.num,
          stream->format.max_framerate.denom);

    params[n_params++] = build_buffer(&b, blocks, 0, 0, buffer_type);

    params[n_params++] = spa_pod_builder_add_object(
        &b, SPA_TYPE_OBJECT_ParamMeta, SPA_PARAM_Meta, SPA_PARAM_META_type,
        SPA_POD_Id(SPA_META_Header), SPA_PARAM_META_size,
        SPA_POD_Int(sizeof(struct spa_meta_header)));

    params[n_params++] = spa_pod_builder_add_object(
        &b, SPA_TYPE_OBJECT_ParamMeta, SPA_PARAM_Meta, SPA_PARAM_META_type,
        SPA_POD_Id(SPA_META_VideoDamage), SPA_PARAM_META_size,
        SPA_POD_CHOICE_RANGE_Int(sizeof(struct spa_meta_region) * 16,
                                 sizeof(struct spa_meta_region) * 1,
                                 sizeof(struct spa_meta_region) * 16));

    pw_stream_update_params(stream->stream, params, n_params);
}

static void stream_queue_buffer(struct stream *stream, bool completed)
{
    if (!stream->streaming || !stream->current) {
        return;
    }

    struct spa_buffer *spa_buf = stream->current->buffer;
    stream->queue = current_time_nsec();

    struct spa_meta_header *h;
    if ((h = spa_buffer_find_meta_data(spa_buf, SPA_META_Header, sizeof(*h)))) {
        h->pts = stream->queue;
        h->flags = completed ? 0 : SPA_META_HEADER_FLAG_CORRUPTED;
        h->seq = stream->seq++;
        h->dts_offset = 0;
        debug("pipewire: timestamp %" PRId64, h->pts);
    }

    struct spa_meta *damage;
    if ((damage = spa_buffer_find_meta(spa_buf, SPA_META_VideoDamage))) {
        struct spa_region *d_region = spa_meta_first(damage);
        *d_region = SPA_REGION(0, 0, stream->width, stream->height);
        debug("pipewire: damage %u,%u (%ux%u)", d_region->position.x, d_region->position.y,
              d_region->size.width, d_region->size.height);
    }

    struct spa_data *d = spa_buf->datas;

    if (completed) {
        for (uint32_t plane = 0; plane < spa_buf->n_datas; plane++) {
            d[plane].chunk->flags = SPA_CHUNK_FLAG_NONE;
        }
    } else {
        for (uint32_t plane = 0; plane < spa_buf->n_datas; plane++) {
            d[plane].chunk->flags = SPA_CHUNK_FLAG_CORRUPTED;
        }
    }

    for (uint32_t plane = 0; plane < spa_buf->n_datas; plane++) {
        debug("pipewire: plane %d", plane);
        debug("     fd %lu", d[plane].fd);
        debug("     pipewire: maxsize %d", d[plane].maxsize);
        debug("     pipewire: size %d", d[plane].chunk->size);
        debug("     pipewire: stride %d", d[plane].chunk->stride);
        debug("     pipewire: offset %d", d[plane].chunk->offset);
        debug("     pipewire: chunk flags %d", d[plane].chunk->flags);
    }

    debug("queue buffer %ld", d[0].fd);
    pw_stream_queue_buffer(stream->stream, stream->current);
    stream->current = NULL;
}

static void strean_trigger_process(struct stream *stream, bool force)
{
    if (!stream->streaming) {
        return;
    }

    pw_stream_trigger_process(stream->stream);

    if (!force) {
        return;
    }

    if (stream->capture) {
        capture_mark_wants_update(stream->capture, true, true);
    } else if (stream->thumbnail) {
        thumbnail_mark_wants_update(stream->thumbnail, true, true);
    }
}

static void trigger_process_in_idle(void *data)
{
    struct stream *stream = data;
    // force update when first frame
    strean_trigger_process(stream, true);
}

static int trigger_process(void *data)
{
    struct stream *stream = data;
    strean_trigger_process(stream, false);
    return 0;
}

static void stream_schedule_process(struct stream *stream, bool first)
{
    if (first) {
        wl_event_loop_add_idle(manager->server->event_loop, trigger_process_in_idle, stream);
        return;
    }

    if (!stream->timer) {
        stream->timer =
            wl_event_loop_add_timer(manager->server->event_loop, trigger_process, stream);
        if (!stream->timer) {
            return;
        }
    }

    uint32_t duration = 1000 / stream->max_framerate;
    uint32_t delay = 0, wait = 0;
    // wait a duration when out of buffers
    if (stream->dequeue > stream->queue) {
        wait = duration;
    } else {
        delay = (stream->queue - stream->dequeue) / 1000000;
        wait = delay < duration ? duration - delay : 1;
    }
    debug("pipewire: schedule delay %u wait %u", delay, wait);
    wl_event_source_timer_update(stream->timer, wait);
}

static void handle_state_changed(void *data, enum pw_stream_state old, enum pw_stream_state state,
                                 const char *error)
{
    struct stream *stream = data;
    debug("pipewire: stream state changed to \"%s\"", pw_stream_state_as_string(state));

    switch (state) {
    case PW_STREAM_STATE_STREAMING:
        stream->streaming = true;
        stream_schedule_process(stream, true);
        break;
    case PW_STREAM_STATE_PAUSED:
        if (old == PW_STREAM_STATE_STREAMING) {
            stream_queue_buffer(stream, false);
        } else if (stream->impl && stream->impl->ready) {
            uint32_t node_id = pw_stream_get_node_id(stream->stream);
            stream->impl->ready(stream, node_id, stream->user_data);
        }
        if (stream->capture) {
            capture_mark_wants_update(stream->capture, false, false);
        } else if (stream->thumbnail) {
            thumbnail_mark_wants_update(stream->thumbnail, false, false);
        }
        // fall through
    default:
        stream->streaming = false;
        break;
    }
}

static void handle_add_buffer(void *data, struct pw_buffer *buffer)
{
    struct stream *stream = data;
    struct spa_data *spa_data = buffer->buffer->datas;
    enum allocator_buffer_type type;

    if (spa_data[0].type & (1 << SPA_DATA_DmaBuf)) {
        type = ALLOCATOR_BUFFER_TYPE_GBM;
    } else if (spa_data[0].type & (1 << SPA_DATA_MemFd)) {
        type = ALLOCATOR_BUFFER_TYPE_SHM;
    } else {
        kywc_log(KYWC_ERROR, "Pipewire: unsupported buffer type");
        return;
    }

    /* create buffers with swapchain */
    if (!stream->buffers) {
        struct wlr_drm_format format;
        drm_format_init(&format, drm_fourcc_from_pw_format(stream->format.format),
                        &stream->format.modifier, 1);
        debug("Create swapchain with format %d modifier %lu %lu", format.format,
              stream->format.modifier >> 32, stream->format.modifier & 0xFFFFFFFF);
        stream->buffers =
            allocator_create_swapchain(manager->server->allocator, type, stream->format.size.width,
                                       stream->format.size.height, &format, BUFFER_FLAGS);
        if (!stream->buffers) {
            return;
        }
    }

    struct wlr_buffer *wlr_buffer = swapchain_acquire(stream->buffers, NULL);
    if (!wlr_buffer) {
        return;
    }

    buffer->user_data = wlr_buffer;

    struct wlr_dmabuf_attributes dmabuf;
    struct wlr_shm_attributes shm;

    if (wlr_buffer_get_dmabuf(wlr_buffer, &dmabuf)) {
        assert(buffer->buffer->n_datas == (uint32_t)dmabuf.n_planes);
        for (int i = 0; i < dmabuf.n_planes; ++i) {
            spa_data[i].type = SPA_DATA_DmaBuf;
            spa_data[i].flags = SPA_DATA_FLAG_READWRITE;
            spa_data[i].fd = dmabuf.fd[i];
            spa_data[i].mapoffset = 0;
            spa_data[i].maxsize = i == 0 ? dmabuf.stride[i] * dmabuf.height : 0;
            spa_data[i].data = NULL;
            spa_data[i].chunk->offset = dmabuf.offset[i];
            spa_data[i].chunk->size = spa_data[i].maxsize;
            spa_data[i].chunk->stride = dmabuf.stride[i];
            spa_data[i].chunk->flags = SPA_CHUNK_FLAG_NONE;
        };

    } else if (wlr_buffer_get_shm(wlr_buffer, &shm)) {
        spa_data->type = SPA_DATA_MemFd;
        spa_data->flags = SPA_DATA_FLAG_READWRITE;
        spa_data->mapoffset = 0;
        spa_data->maxsize = shm.stride * shm.height;
        spa_data->fd = shm.fd;
        spa_data->data = NULL;
        spa_data->chunk->offset = 0;
        spa_data->chunk->size = spa_data->maxsize;
        spa_data->chunk->stride = shm.stride;
        spa_data->chunk->flags = SPA_CHUNK_FLAG_NONE;
    }
}

static void handle_remove_buffer(void *data, struct pw_buffer *buffer)
{
    for (uint32_t plane = 0; plane < buffer->buffer->n_datas; plane++) {
        buffer->buffer->datas[plane].fd = -1;
    }

    struct wlr_buffer *wlr_buffer = buffer->user_data;
    wlr_buffer_unlock(wlr_buffer);
    buffer->user_data = NULL;

    struct stream *stream = data;
    if (stream->buffers) {
        swapchain_destroy(stream->buffers);
        stream->buffers = NULL;
    }
}

static void handle_process(void *data)
{
    struct stream *stream = data;

    if (!stream->streaming) {
        debug("pipewire: not streaming");
        return;
    }

    if (stream->current) {
        debug("pipewire: buffer already exported");
        return;
    }

    stream->current = pw_stream_dequeue_buffer(stream->stream);
    // update to hint out of buffers
    stream->dequeue = current_time_nsec();

    if (!stream->current) {
        kywc_log_ratelimited(KYWC_INFO, "pipewire: out of buffers");
        stream_schedule_process(stream, false);
        return;
    } else {
        debug("dequeue buffer %ld", stream->current->buffer->datas[0].fd);
    }

    if (stream->capture) {
        capture_mark_wants_update(stream->capture, true, false);
    } else if (stream->thumbnail) {
        thumbnail_mark_wants_update(stream->thumbnail, true, false);
    }
}

static void stream_cancel(struct stream *stream)
{
    stream->thumbnail = NULL;
    stream->capture = NULL;
    stream_queue_buffer(stream, false);

    if (stream->impl && stream->impl->cancel) {
        stream->impl->cancel(stream, stream->user_data);
    }

    stream_destroy(stream);
}

static void stream_update_param(struct stream *stream, int width, int height)
{
    stream->width = width;
    stream->height = height;

    if (stream->buffers) {
        swapchain_destroy(stream->buffers);
        stream->buffers = NULL;
    }

    const struct spa_pod *params[MAX_PARAMS];
    uint8_t buffer[4096];
    struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    uint32_t n_params = stream_build_formats(stream, &b, params, 0);

    pw_stream_update_params(stream->stream, params, n_params);
}

static const struct pw_stream_events stream_events = {
    .version = PW_VERSION_STREAM_EVENTS,
    .state_changed = handle_state_changed,
    .param_changed = handle_param_changed,
    .add_buffer = handle_add_buffer,
    .remove_buffer = handle_remove_buffer,
    .process = handle_process,
};

static bool stream_create_and_connect(struct stream *stream)
{
    char name[] = "kywc-stream-XXXXXX";
    kywc_identifier_rand_generate(name, 0);

    stream->stream = pw_stream_new(manager->context->core, name,
                                   pw_properties_new(PW_KEY_MEDIA_CLASS, "Video/Source", NULL));
    pw_stream_add_listener(stream->stream, &stream->stream_listener, &stream_events, stream);

    const struct spa_pod *params[MAX_PARAMS];
    uint8_t buffer[4096];
    struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    uint32_t n_params = stream_build_formats(stream, &b, params, 0);

    // use PW_STREAM_FLAG_DRIVER mode
    bool success = pw_stream_connect(stream->stream, PW_DIRECTION_OUTPUT, PW_ID_ANY,
                                     PW_STREAM_FLAG_ALLOC_BUFFERS | PW_STREAM_FLAG_DRIVER, params,
                                     n_params) == 0;

    kywc_log(KYWC_INFO, "Stream %s create %s", name, success ? "succeed" : "failed");
    return success;
}

static void stream_handle_capture_update(struct capture *capture, struct wlr_buffer *buffer,
                                         bool buffer_changed, void *data)
{
    struct stream *stream = data;
    stream_queue_buffer(stream, true);
    capture_mark_wants_update(stream->capture, false, false);
    // trigger next frame
    stream_schedule_process(stream, false);
}

static void stream_handle_capture_cancel(struct capture *capture, void *data)
{
    struct stream *stream = data;
    stream_cancel(stream);
}

static struct wlr_buffer *stream_handle_capture_buffer(struct capture *capture,
                                                       struct wlr_buffer *buffer, int width,
                                                       int height, bool single_plane, void *data)
{
    struct stream *stream = data;
    struct wlr_buffer *wlr_buffer = stream->current->user_data;

    if (width != stream->width || height != stream->height) {
        debug("buffer size %d x %d to %d x %d", stream->width, stream->height, width, height);
        stream_queue_buffer(stream, false);
        stream_update_param(stream, width, height);
        capture_mark_wants_update(stream->capture, false, false);
    }

    return wlr_buffer;
}

static void stream_handle_capature_client(struct capture *capture, struct client *client,
                                          void *data)
{
    struct stream *stream = data;
    if (stream->impl && stream->impl->client) {
        stream->impl->client(stream, client, stream->user_data);
    }
}

static const struct capture_interface capture_impl = {
    .update = stream_handle_capture_update,
    .cancel = stream_handle_capture_cancel,
    .buffer = stream_handle_capture_buffer,
    .client = stream_handle_capature_client,
};

struct stream *stream_create_for_output(struct output *output, uint32_t options,
                                        const struct stream_interface *impl, void *data)
{
    if (!manager || !get_pipewire_context()) {
        return NULL;
    }

    struct stream *stream = calloc(1, sizeof(*stream));
    if (!stream) {
        return NULL;
    }

    stream->impl = impl;
    stream->user_data = data;

    uint32_t capture_options =
        options & STREAM_NEED_CURSOR ? CAPTURE_NEED_CURSOR : CAPTURE_NEED_NONE;
    stream->capture = capture_create_from_output(output, capture_options, &capture_impl, stream);
    if (!stream->capture) {
        free(stream);
        return NULL;
    }

    capture_mark_wants_update(stream->capture, false, false);
    capture_get_buffer_size(stream->capture, &stream->width, &stream->height);
    stream->max_framerate = CLAMP(output->base.state.refresh / 1000, 30, 60);

    if (!stream_create_and_connect(stream)) {
        capture_destroy(stream->capture);
        free(stream);
        return NULL;
    }

    wl_list_insert(&manager->streams, &stream->link);

    return stream;
}

static void stream_handle_thumbnail_update(struct thumbnail *thumbnail, struct wlr_buffer *buffer,
                                           bool buffer_changed, void *data)
{
    struct stream *stream = data;
    stream_queue_buffer(stream, true);
    thumbnail_mark_wants_update(stream->thumbnail, false, false);
    // trigger next frame
    stream_schedule_process(stream, false);
}

static void stream_handle_thumbnail_cancel(struct thumbnail *thumbnail, void *data)
{
    struct stream *stream = data;
    stream_cancel(stream);
}

static struct wlr_buffer *stream_handle_thumbnail_buffer(struct thumbnail *thumbnail,
                                                         struct wlr_buffer *buffer, int width,
                                                         int height, bool single_plane, void *data)
{
    struct stream *stream = data;
    struct wlr_buffer *wlr_buffer = stream->current->user_data;

    if (width != stream->width || height != stream->height) {
        debug("buffer size %d x %d to %d x %d", stream->width, stream->height, width, height);
        stream_queue_buffer(stream, false);
        stream_update_param(stream, width, height);
        thumbnail_mark_wants_update(stream->thumbnail, false, false);
    }

    return wlr_buffer;
}

static void stream_handle_thumbnail_client(struct thumbnail *thumbnail, struct client *client,
                                           void *data)
{
    struct stream *stream = data;
    if (stream->impl && stream->impl->client) {
        stream->impl->client(stream, client, stream->user_data);
    }
}

static const struct thumbnail_interface thumbnail_impl = {
    .update = stream_handle_thumbnail_update,
    .cancel = stream_handle_thumbnail_cancel,
    .buffer = stream_handle_thumbnail_buffer,
    .client = stream_handle_thumbnail_client,
};

struct stream *stream_create_for_view(struct view *view, uint32_t options,
                                      const struct stream_interface *impl, void *data)
{
    if (!manager || !get_pipewire_context()) {
        return NULL;
    }

    struct stream *stream = calloc(1, sizeof(*stream));
    if (!stream) {
        return NULL;
    }

    stream->impl = impl;
    stream->user_data = data;

    uint32_t thumbnail_options =
        THUMBNAIL_DISABLE_SHADOW | THUMBNAIL_DISABLE_ROUND_CORNER | THUMBNAIL_ENABLE_SECURITY;
    stream->thumbnail =
        thumbnail_create_from_view(view, thumbnail_options, 1.0, &thumbnail_impl, stream);
    if (!stream->thumbnail) {
        free(stream);
        return NULL;
    }

    thumbnail_mark_wants_update(stream->thumbnail, false, false);
    thumbnail_get_buffer_size(stream->thumbnail, &stream->width, &stream->height);
    stream->max_framerate = CLAMP(view->output->state.refresh / 1000, 30, 60);

    if (!stream_create_and_connect(stream)) {
        thumbnail_destroy(stream->thumbnail);
        free(stream);
        return NULL;
    }

    wl_list_insert(&manager->streams, &stream->link);

    return stream;
}

void stream_destroy(struct stream *stream)
{
    if (!stream) {
        return;
    }

    kywc_log(KYWC_INFO, "stream %s destroy", pw_stream_get_name(stream->stream));

    pw_stream_flush(stream->stream, false);
    pw_stream_disconnect(stream->stream);
    pw_stream_destroy(stream->stream);

    if (stream->capture) {
        capture_destroy(stream->capture);
    }
    if (stream->thumbnail) {
        thumbnail_destroy(stream->thumbnail);
    }
    if (stream->buffers) {
        swapchain_destroy(stream->buffers);
    }
    if (stream->timer) {
        wl_event_source_remove(stream->timer);
    }

    wl_list_remove(&stream->link);
    free(stream);
}

static void pipewire_context_destroy(struct pipewire_context *context)
{
    if (!context) {
        return;
    }

    if (context->loop) {
        pw_loop_leave(context->loop);
    }
    if (context->core) {
        pw_core_disconnect(context->core);
    }
    if (context->context) {
        pw_context_destroy(context->context);
    }
    if (context->loop) {
        pw_loop_destroy(context->loop);
    }

    if (context->event) {
        wl_event_source_remove(context->event);
    }

    free(context);
}

static void pipewire_context_invalid(struct pipewire_context *context)
{
    if (context->event) {
        wl_event_source_remove(context->event);
        context->event = NULL;
    }
}

static void on_core_error(void *data, uint32_t id, int seq, int res, const char *message)
{
    kywc_log(KYWC_ERROR, "Pipewire: fatal error event from core");
    struct pipewire_context *context = data;
    pipewire_context_invalid(context);
}

static const struct pw_core_events core_events = {
    .version = PW_VERSION_CORE_EVENTS,
    .error = on_core_error,
};

static int pipewire_event_procss(int fd, uint32_t mask, void *data)
{
    struct pipewire_context *context = data;

    if (mask & (WL_EVENT_HANGUP | WL_EVENT_ERROR)) {
        kywc_log(KYWC_ERROR, "Pipewire eventfd failed");
        pipewire_context_invalid(context);
        return 0;
    }

    if (pw_loop_iterate(context->loop, 0) < 0) {
        kywc_log(KYWC_ERROR, "pw_loop_iterate failed: %s", strerror(errno));
    }

    return 0;
}

static struct pipewire_context *pipewire_context_create(void)
{
    struct pipewire_context *context = calloc(1, sizeof(*context));
    if (!context) {
        return NULL;
    }

    context->loop = pw_loop_new(NULL);
    if (!context->loop) {
        kywc_log(KYWC_ERROR, "Faild to create pipewire loop");
        goto failed;
    }

    pw_loop_enter(context->loop);

    int fd = pw_loop_get_fd(context->loop);
    context->event = wl_event_loop_add_fd(manager->server->event_loop, fd, WL_EVENT_READABLE,
                                          pipewire_event_procss, context);
    if (!context->event) {
        kywc_log(KYWC_ERROR, "Failed to create event on event loop");
        goto failed;
    }

    context->context = pw_context_new(context->loop, NULL, 0);
    if (!context->context) {
        kywc_log(KYWC_ERROR, "Failed to create pipewire context");
        goto failed;
    }

    context->core = pw_context_connect(context->context, NULL, 0);
    if (!context->core) {
        kywc_log(KYWC_ERROR, "Couldn't connect to pipewire context");
        goto failed;
    }

    if (pw_loop_iterate(context->loop, 0) < 0) {
        kywc_log(KYWC_ERROR, "Stream: failed to start pipewire loop");
        goto failed;
    }

    spa_zero(context->core_listener);
    pw_core_add_listener(context->core, &context->core_listener, &core_events, context);

    return context;

failed:
    pipewire_context_destroy(context);
    return NULL;
}

static struct pipewire_context *get_pipewire_context(void)
{
    if (!manager->context || !manager->context->event) {
        pipewire_context_destroy(manager->context);
        manager->context = pipewire_context_create();
    }
    return manager->context;
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&manager->display_destroy.link);

    struct stream *stream, *tmp;
    wl_list_for_each_safe(stream, tmp, &manager->streams, link) {
        stream_destroy(stream);
    }

    pipewire_context_destroy(manager->context);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    pw_deinit();

    wlr_drm_format_set_finish(&manager->formats);
    wl_list_remove(&manager->server_destroy.link);

    free(manager);
    manager = NULL;
}

bool stream_manager_create(struct server *server)
{
    manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    wl_list_init(&manager->streams);

    manager->server = server;
    manager->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(server->display, &manager->display_destroy);
    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &manager->server_destroy);

    /* use ARGB8888 and XRGB8888 */
    uint32_t formats[] = { DRM_FORMAT_ARGB8888, DRM_FORMAT_XRGB8888 };
    bool dmabuf_support = server->allocator->buffer_caps & WLR_BUFFER_CAP_DMABUF;

    for (size_t i = 0; i < ARRAY_SIZE(formats); i++) {
        /* add shm buffer format */
        wlr_drm_format_set_add(&manager->formats, formats[i], DRM_FORMAT_MOD_INVALID);
        if (!dmabuf_support) {
            continue;
        }
        /* dmabuf is support, use format that support render */
        const struct wlr_drm_format *fmt = renderer_get_render_format(server->renderer, formats[i]);
        if (!fmt) {
            continue;
        }
        for (size_t j = 0; j < fmt->len; j++) {
            wlr_drm_format_set_add(&manager->formats, fmt->format, fmt->modifiers[j]);
        }
    }

    /* init pipewire */
    pw_init(NULL, NULL);
    kywc_log(KYWC_INFO, "Compiled with libpipewire %s(Linked with %s)", pw_get_headers_version(),
             pw_get_library_version());

    return true;
}
