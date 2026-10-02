// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L

#include <libdrm/drm_fourcc.h>

#include <pipewire/pipewire.h>
#include <spa/debug/format.h>
#include <spa/debug/pod.h>
#include <spa/param/tag-utils.h>
#include <spa/param/video/format-utils.h>

#include "buffer.h"

struct context {
    struct pw_main_loop *loop;
    struct pw_stream *stream;
    struct spa_source *reneg;
    struct spa_video_info format;

    kywc_context *kywc;
    struct kywc_buffer_helper *helper;
    struct spa_source *kywc_source;

    enum kywc_stream_type type;
    const char *source_id;
    bool avoid_dmabufs;
};

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

static void on_process(void *data)
{
    struct context *ctx = data;
    struct pw_buffer *b = NULL;

    while (true) {
        struct pw_buffer *t;
        if ((t = pw_stream_dequeue_buffer(ctx->stream)) == NULL) {
            break;
        }
        if (b) {
            pw_stream_queue_buffer(ctx->stream, b);
        }
        b = t;
    }
    if (b == NULL) {
        pw_log_warn("out of buffers: %m");
        return;
    }

    struct spa_buffer *buf = b->buffer;
    struct spa_data *d = buf->datas;

    struct spa_meta_header *h;
    if ((h = spa_buffer_find_meta_data(buf, SPA_META_Header, sizeof(*h)))) {
        if ((h->flags & SPA_META_HEADER_FLAG_CORRUPTED) > 0) {
            fprintf(stderr, "buffer is corrupt\n");
            goto done;
        }
    }

    struct kywc_thumbnail_buffer buffer = {
        .fd = d[0].fd,
        .format = drm_fourcc_from_pw_format(ctx->format.info.raw.format),
        .width = ctx->format.info.raw.size.width,
        .height = ctx->format.info.raw.size.height,
        .offset = d[0].chunk->offset,
        .stride = d[0].chunk->stride,
        .modifier = ctx->format.info.raw.modifier,
        .flags = KYWC_THUMBNAIL_BUFFER_IS_REUSED |
                 (d[0].type == SPA_DATA_DmaBuf ? KYWC_THUMBNAIL_BUFFER_IS_DMABUF : 0),
        .n_planes = buf->n_datas,
    };
    bool corrupt = false;

    for (uint32_t plane = 0; plane < buf->n_datas; plane++) {
        // fprintf(stdout, "plane %d:\n", plane);
        // fprintf(stdout, "  fd %lu\n", d[plane].fd);
        buffer.planes[plane].fd = d[plane].fd;
        // fprintf(stdout, "  stride %d\n", d[plane].chunk->stride);
        buffer.planes[plane].stride = d[plane].chunk->stride;
        // fprintf(stdout, "  offset %d\n", d[plane].chunk->offset);
        buffer.planes[plane].offset = d[plane].chunk->offset;
        // fprintf(stdout, "  chunk flags %d\n", d[plane].chunk->flags);
        corrupt |= (d[plane].chunk->flags & SPA_CHUNK_FLAG_CORRUPTED) > 0;
    }

    if (corrupt) {
        goto done;
    }

    struct kywc_buffer *kywc_buffer = kywc_buffer_helper_import_thumbnail(ctx->helper, b, &buffer);

    static uint64_t index = 0;
    char path[256];
    snprintf(path, sizeof(path), "%lu-%dx%d.bmp", index++, buffer.width, buffer.height);
    kywc_buffer_write_to_file(kywc_buffer, 0, 0, buffer.width, buffer.height, path);

    // buffer import, remove and reneg if failed
    // pw_loop_signal_event(pw_main_loop_get_loop(ctx->loop), ctx->reneg);

done:
    pw_stream_queue_buffer(ctx->stream, b);
}

static void on_stream_state_changed(void *data, enum pw_stream_state old,
                                    enum pw_stream_state state, const char *error)
{
    struct context *ctx = data;
    fprintf(stdout, "stream state: \"%s\"\n", pw_stream_state_as_string(state));
    fprintf(stdout, "node id: %d\n", pw_stream_get_node_id(ctx->stream));

    switch (state) {
    case PW_STREAM_STATE_ERROR:
    case PW_STREAM_STATE_UNCONNECTED:
        pw_main_loop_quit(ctx->loop);
        break;
    case PW_STREAM_STATE_PAUSED:
        if (old == PW_STREAM_STATE_STREAMING) {
            kywc_buffer_helper_destroy_buffers(ctx->helper);
        }
        break;
    case PW_STREAM_STATE_STREAMING:
        break;
    default:
        break;
    }
}

static void on_stream_param_changed(void *data, uint32_t id, const struct spa_pod *param)
{
    if (param == NULL || id != SPA_PARAM_Format) {
        return;
    }

    struct context *ctx = data;
    if (spa_format_parse(param, &ctx->format.media_type, &ctx->format.media_subtype) < 0) {
        return;
    }

    if (ctx->format.media_type != SPA_MEDIA_TYPE_video ||
        ctx->format.media_subtype != SPA_MEDIA_SUBTYPE_raw) {
        return;
    }

    /* call a helper function to parse the format for us. */
    spa_format_video_raw_parse(param, &ctx->format.info.raw);

    enum spa_video_format format = ctx->format.info.raw.format;
    if (format != SPA_VIDEO_FORMAT_BGRA && format != SPA_VIDEO_FORMAT_BGRx) {
        pw_stream_set_error(ctx->stream, -EINVAL, "unknown pixel format");
        return;
    }

    struct spa_rectangle size =
        SPA_RECTANGLE(ctx->format.info.raw.size.width, ctx->format.info.raw.size.height);
    if (size.width == 0 || size.height == 0) {
        pw_stream_set_error(ctx->stream, -EINVAL, "invalid size");
        return;
    }

    uint32_t buffer_types = 1 << SPA_DATA_MemPtr;
    bool has_modifier = spa_pod_find_prop(param, NULL, SPA_FORMAT_VIDEO_modifier) != NULL;
    if (has_modifier) {
        buffer_types |= 1 << SPA_DATA_DmaBuf;
    }

    fprintf(stdout, "Negotiated format:\n");
    const char *format_name =
        spa_debug_type_find_name(spa_type_video_format, ctx->format.info.raw.format);
    fprintf(stdout, "     Format: %d (%s)\n", ctx->format.info.raw.format,
            format_name ? format_name : "unknown format");
    if (has_modifier) {
        fprintf(stdout, "     Modifier: 0x%" PRIx64 "\n", ctx->format.info.raw.modifier);
    }
    fprintf(stdout, "     Size: %dx%d\n", ctx->format.info.raw.size.width,
            ctx->format.info.raw.size.height);
    fprintf(stdout, "     Framerate: %d/%d (Max %d/%d))\n", ctx->format.info.raw.framerate.num,
            ctx->format.info.raw.framerate.denom, ctx->format.info.raw.max_framerate.num,
            ctx->format.info.raw.max_framerate.denom);

    const struct spa_pod *params[2];
    uint32_t n_params = 0;
    uint8_t params_buffer[1024];
    struct spa_pod_builder b = SPA_POD_BUILDER_INIT(params_buffer, sizeof(params_buffer));

    params[n_params++] = spa_pod_builder_add_object(&b, SPA_TYPE_OBJECT_ParamBuffers,
                                                    SPA_PARAM_Buffers, SPA_PARAM_BUFFERS_dataType,
                                                    SPA_POD_CHOICE_FLAGS_Int(buffer_types));

    params[n_params++] = spa_pod_builder_add_object(
        &b, SPA_TYPE_OBJECT_ParamMeta, SPA_PARAM_Meta, SPA_PARAM_META_type,
        SPA_POD_Id(SPA_META_Header), SPA_PARAM_META_size,
        SPA_POD_Int(sizeof(struct spa_meta_header)));

    pw_stream_update_params(ctx->stream, params, n_params);
}

static struct spa_pod *build_format(struct spa_pod_builder *b, uint32_t format, uint64_t *modifiers,
                                    size_t modifier_count)
{
    struct spa_pod_frame format_frame;

    spa_pod_builder_push_object(b, &format_frame, SPA_TYPE_OBJECT_Format, SPA_PARAM_EnumFormat);
    spa_pod_builder_add(b, SPA_FORMAT_mediaType, SPA_POD_Id(SPA_MEDIA_TYPE_video), 0);
    spa_pod_builder_add(b, SPA_FORMAT_mediaSubtype, SPA_POD_Id(SPA_MEDIA_SUBTYPE_raw), 0);
    spa_pod_builder_add(b, SPA_FORMAT_VIDEO_format, SPA_POD_Id(format), 0);

    /* modifier */
    if (modifier_count > 0) {
        struct spa_pod_frame modifier_frame;
        /* build an enumeration of modifiers */
        spa_pod_builder_prop(b, SPA_FORMAT_VIDEO_modifier,
                             SPA_POD_PROP_FLAG_MANDATORY | SPA_POD_PROP_FLAG_DONT_FIXATE);
        spa_pod_builder_push_choice(b, &modifier_frame, SPA_CHOICE_Enum, 0);
        /* The first element of choice pods is the preferred value */
        spa_pod_builder_long(b, modifiers[0]);
        for (uint32_t i = 0; i < modifier_count; i++) {
            spa_pod_builder_long(b, modifiers[i]);
        }
        spa_pod_builder_pop(b, &modifier_frame);
    }
    /* add size and framerate ranges */
    spa_pod_builder_add(b, SPA_FORMAT_VIDEO_size,
                        SPA_POD_CHOICE_RANGE_Rectangle(&SPA_RECTANGLE(320, 240), // Arbitrary
                                                       &SPA_RECTANGLE(1, 1),
                                                       &SPA_RECTANGLE(8192, 4320)),
                        SPA_FORMAT_VIDEO_maxFramerate,
                        SPA_POD_CHOICE_RANGE_Fraction(&SPA_FRACTION(30, 1), &SPA_FRACTION(0, 1),
                                                      &SPA_FRACTION(60, 1)),
                        0);
    return spa_pod_builder_pop(b, &format_frame);
}

static uint32_t build_format_params(struct context *ctx, struct spa_pod_builder *b,
                                    const struct spa_pod **params)
{
    static struct format_pair {
        uint32_t drm_format;
        enum spa_video_format spa_format;
    } formats[2] = {
        { DRM_FORMAT_ARGB8888, SPA_VIDEO_FORMAT_BGRA },
        { DRM_FORMAT_XRGB8888, SPA_VIDEO_FORMAT_BGRx },
    };

    uint32_t n_params = 0;

    for (size_t i = 0; i < sizeof(formats) / sizeof(formats[0]); i++) {
        struct format_pair *pair = &formats[i];
        /* add dmabuf format first */
        if (!ctx->avoid_dmabufs) {
            struct buffer_format *format =
                kywc_buffer_helper_get_format(ctx->helper, pair->drm_format);
            if (format && format->len > 0) {
                params[n_params++] =
                    build_format(b, pair->spa_format, format->modifiers, format->len);
            }
        }
        params[n_params++] = build_format(b, pair->spa_format, NULL, 0);
    }

    return n_params;
}

static const struct pw_stream_events stream_events = {
    .version = PW_VERSION_STREAM_EVENTS,
    .process = on_process,
    .state_changed = on_stream_state_changed,
    .param_changed = on_stream_param_changed,
};

static void do_quit(void *data, int signal_number)
{
    struct context *ctx = data;
    pw_main_loop_quit(ctx->loop);
}

static void on_kywc_event(void *data, int fd, uint32_t mask)
{
    struct context *ctx = data;

    if (mask & (SPA_IO_ERR | SPA_IO_HUP)) {
        pw_main_loop_quit(ctx->loop);
        return;
    }

    if (mask & SPA_IO_IN) {
        kywc_context_process(ctx->kywc);
    }
}

static void reneg_format(void *data, uint64_t expirations)
{
    struct context *ctx = data;
    if (ctx->format.info.raw.format == 0) {
        return;
    }

    fprintf(stdout, "renegotiate formats\n");

    const struct spa_pod *params[4];
    uint8_t buffer[4096];
    struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    uint32_t n_params = build_format_params(ctx, &b, params);

    pw_stream_update_params(ctx->stream, params, n_params);
}

static void stream_create(struct context *ctx, uint32_t node)
{
    /* If you plan to autoconnect your stream, you need to provide at least
     * media, category and role properties
     */
    struct pw_properties *props =
        pw_properties_new(PW_KEY_MEDIA_TYPE, "Video", PW_KEY_MEDIA_CATEGORY, "Capture",
                          PW_KEY_MEDIA_ROLE, "Screen", NULL);
    pw_properties_setf(props, PW_KEY_TARGET_OBJECT, "%u", node);

    ctx->stream = pw_stream_new_simple(pw_main_loop_get_loop(ctx->loop), "video-play", props,
                                       &stream_events, ctx);

    const struct spa_pod *params[4]; // 2 format with 2 type (shm and dmabuf)
    uint8_t buffer[4096];
    struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    uint32_t n_params = build_format_params(ctx, &b, params);

    pw_stream_connect(ctx->stream, PW_DIRECTION_INPUT, PW_ID_ANY,
                      PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS, params, n_params);
}

static void stream_handle_ready(kywc_stream *stream, uint32_t node, void *data)
{
    struct context *ctx = data;
    fprintf(stdout, "pipewire stream ready with node %d\n", node);
    stream_create(ctx, node);
}

static void stream_handle_destroy(kywc_stream *stream, void *data)
{
    struct context *ctx = data;
    fprintf(stdout, "pipewire stream cancel, quit main loop\n");
    pw_main_loop_quit(ctx->loop);
}

static const struct kywc_stream_interface stream_impl = {
    .ready = stream_handle_ready,
    .destroy = stream_handle_destroy,
};

static void handle_destroy(kywc_context *ctx, void *data)
{
    fprintf(stdout, "kywc context destroy\n");
    struct context *_ctx = data;
    kywc_buffer_helper_destroy(_ctx->helper);
}

static void handle_create(kywc_context *ctx, void *data)
{
    fprintf(stdout, "kywc context create\n");
    struct context *_ctx = data;
    _ctx->helper = kywc_buffer_helper_create(ctx);
}

static const struct kywc_context_interface context_impl = {
    .create = handle_create,
    .destroy = handle_destroy,
};

int main(int argc, char *argv[])
{
    struct context ctx = { 0 };
    int ret = 0;

    if (argc != 3 && argc != 4) {
        fprintf(stderr, "please input args\n");
        return -1;
    }

    const char *option = argv[1];
    if (strcmp(option, "output") == 0) {
        ctx.type = KYWC_STREAM_TYPE_OUTPUT;
        ctx.source_id = argv[2];
    } else if (strcmp(option, "toplevel") == 0) {
        ctx.type = KYWC_STREAM_TYPE_TOPLEVEL;
        ctx.source_id = argv[2];
    } else {
        fprintf(stderr, "please input either output or toplevel\n");
        return -1;
    }

    if (argc == 4) {
        ctx.avoid_dmabufs = strcmp(argv[3], "1") == 0;
    }

    pw_init(NULL, NULL);
    fprintf(stdout,
            "Compiled with libpipewire %s\n"
            "Linked with libpipewire %s\n",
            pw_get_headers_version(), pw_get_library_version());

    ctx.loop = pw_main_loop_new(NULL);

    struct pw_loop *loop = pw_main_loop_get_loop(ctx.loop);
    pw_loop_add_signal(loop, SIGINT, do_quit, &ctx);
    pw_loop_add_signal(loop, SIGTERM, do_quit, &ctx);

    uint32_t caps = KYWC_CONTEXT_CAPABILITY_STREAM;
    ctx.kywc = kywc_context_create(NULL, caps, &context_impl, &ctx);
    if (!ctx.kywc) {
        fprintf(stderr, "couldn't connect to wayland\n");
        goto cleanup;
    }

    ctx.reneg = pw_loop_add_event(loop, reneg_format, &ctx);

    ctx.kywc_source =
        pw_loop_add_io(loop, kywc_context_get_fd(ctx.kywc), SPA_IO_IN | SPA_IO_HUP | SPA_IO_ERR,
                       false, on_kywc_event, &ctx);

    if (ctx.type == KYWC_STREAM_TYPE_OUTPUT) {
        kywc_stream_create_from_output(ctx.kywc, ctx.source_id, KYWC_STREAM_CURSOR_MODE_HIDDEN,
                                       &stream_impl, &ctx);
    } else if (ctx.type == KYWC_STREAM_TYPE_TOPLEVEL) {
        kywc_stream_create_from_toplevel(ctx.kywc, ctx.source_id, KYWC_STREAM_CURSOR_MODE_HIDDEN,
                                         &stream_impl, &ctx);
    }

    pw_main_loop_run(ctx.loop);

    kywc_context_destroy(ctx.kywc);

cleanup:
    if (ctx.stream) {
        pw_stream_destroy(ctx.stream);
    }
    pw_main_loop_destroy(ctx.loop);
    pw_deinit();

    return ret;
}
