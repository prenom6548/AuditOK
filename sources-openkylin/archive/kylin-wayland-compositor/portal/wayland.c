// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include "buffer.h"
#include "portal.h"

static void output_handle_state(kywc_output *kywc_output, uint32_t mask)
{
    struct portal_output *output = kywc_output_get_user_data(kywc_output);
    struct kywc_box geometry = { kywc_output->x, kywc_output->y, kywc_output->width,
                                 kywc_output->height };

    if (mask & KYWC_OUTPUT_STATE_ENABLED) {
        output->geometry = geometry;
        portal_input_capture_emit_zones_changed(output->ctx);
        portal_screenshot_emit_output_changed(output->ctx);
        portal_stream_emit_output_changed(output->ctx);
        return;
    }

    if (mask & (KYWC_OUTPUT_STATE_MODE | KYWC_OUTPUT_STATE_POSITION | KYWC_OUTPUT_STATE_TRANSFORM |
                KYWC_OUTPUT_STATE_SCALE)) {
        if (!kywc_box_equal(&output->geometry, &geometry)) {
            output->geometry = geometry;
            portal_input_capture_emit_zones_changed(output->ctx);
        }
        return;
    }
}

static void output_handle_destroy(kywc_output *kywc_output)
{
    struct portal_output *output = kywc_output_get_user_data(kywc_output);

    if (kywc_output->enabled) {
        portal_input_capture_emit_zones_changed(output->ctx);
        portal_screenshot_emit_output_changed(output->ctx);
        portal_stream_emit_output_changed(output->ctx);
    }

    wl_list_remove(&output->link);
    free(output);
}

static struct kywc_output_interface output_impl = {
    .state = output_handle_state,
    .destroy = output_handle_destroy,
};

static void handle_new_output(kywc_context *context, kywc_output *kywc_output, void *data)
{
    struct portal_output *output = calloc(1, sizeof(*output));
    if (!output) {
        return;
    }

    struct portal_context *ctx = data;
    output->ctx = ctx;
    wl_list_insert(&ctx->wayland.outputs, &output->link);

    output->output = kywc_output;
    output->geometry = (struct kywc_box){ kywc_output->x, kywc_output->y, kywc_output->width,
                                          kywc_output->height };

    kywc_output_set_interface(kywc_output, &output_impl);
    kywc_output_set_user_data(kywc_output, output);

    /* skip if output is disabled */
    if (kywc_output->enabled) {
        portal_input_capture_emit_zones_changed(ctx);
        portal_screenshot_emit_output_changed(ctx);
        portal_stream_emit_output_changed(ctx);
    }
}

static void toplevel_handle_destory(kywc_toplevel *kywc_toplevel)
{
    struct portal_context *ctx = kywc_toplevel_get_user_data(kywc_toplevel);
    portal_screenshot_emit_toplevel_changed(ctx);
    portal_stream_emit_toplevel_changed(ctx);
}

static struct kywc_toplevel_interface toplevel_impl = {
    .state = NULL,
    .destroy = toplevel_handle_destory,
};

static void handle_new_toplevel(kywc_context *context, kywc_toplevel *kywc_toplevel, void *data)
{
    kywc_toplevel_set_interface(kywc_toplevel, &toplevel_impl);

    struct portal_context *ctx = data;
    kywc_toplevel_set_user_data(kywc_toplevel, ctx);

    portal_screenshot_emit_toplevel_changed(ctx);
    portal_stream_emit_toplevel_changed(ctx);
}

static void handle_create(kywc_context *ctx, void *data)
{
    struct portal_context *context = data;
    context->wayland.helper = kywc_buffer_helper_create(ctx);
}

static void handle_destroy(kywc_context *ctx, void *data)
{
    struct portal_context *context = data;
    kywc_buffer_helper_destroy(context->wayland.helper);
}

static struct kywc_context_interface context_impl = {
    .create = handle_create,
    .destroy = handle_destroy,
    .new_output = handle_new_output,
    .new_toplevel = handle_new_toplevel,
};

static int handle_wayland_event(uint32_t mask, void *data)
{
    if (mask & (EVENT_HANGUP | EVENT_ERROR)) {
        kywc_log(KYWC_FATAL, "wayland eventfd failed");
        return -1;
    }

    kywc_context *kywc = data;
    return kywc_context_process(kywc);
}

bool portal_wayland_init(struct portal_context *ctx)
{
    ctx->wayland.display = wl_display_connect(NULL);
    if (!ctx->wayland.display) {
        kywc_log(KYWC_ERROR, "Wayland: failed to connect to display");
        return false;
    }
    kywc_log(KYWC_DEBUG, "Wayland: wl_display connected");

    /* init before kywc_context_create_by_display */
    wl_list_init(&ctx->wayland.outputs);

    uint32_t capabilities = KYWC_CONTEXT_CAPABILITY_OUTPUT | KYWC_CONTEXT_CAPABILITY_THUMBNAIL_EXT |
                            KYWC_CONTEXT_CAPABILITY_TOPLEVEL | KYWC_CONTEXT_CAPABILITY_STREAM;
    ctx->wayland.kywc =
        kywc_context_create_by_display(ctx->wayland.display, capabilities, &context_impl, ctx);
    if (!ctx->wayland.kywc) {
        return false;
    }

    int wayland_fd = wl_display_get_fd(ctx->wayland.display);
    if (event_loop_add_source(ctx->event_loop, wayland_fd, handle_wayland_event,
                              ctx->wayland.kywc) < 0) {
        kywc_log(KYWC_ERROR, "Wayland: add to event loop failed");
        return false;
    }

    return true;
}

void portal_wayland_fini(struct portal_context *ctx)
{
    if (ctx->wayland.kywc) {
        kywc_context_destroy(ctx->wayland.kywc);
    }
    if (ctx->wayland.display) {
        wl_display_disconnect(ctx->wayland.display);
    }
}
