// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdio.h>
#include <stdlib.h>

#include <kywc/boxes.h>
#include <kywc/identifier.h>

#include "buffer.h"
#include "commands.h"

struct capture_state {
    const char *area;
    const char *path;
    bool getpixel;
    bool done;
    bool ok;
};

void capture_context_create(kywc_context *ctx)
{
    struct kywc_buffer_helper *helper = kywc_buffer_helper_create(ctx);
    if (!helper) {
        return;
    }

    kywc_context_set_user_data(ctx, helper);
}

void capture_context_destroy(kywc_context *ctx)
{
    struct kywc_buffer_helper *helper = kywc_context_get_user_data(ctx);
    if (helper) {
        kywc_buffer_helper_destroy(helper);
    }
}

static bool capture_pixel(struct kywc_buffer *kywc_buffer, const char *area)
{
    long xy[2];
    if (!parse_ints(area, xy, 2) || xy[0] < 0 || xy[1] < 0) {
        fprintf(stderr, "The --getpixelcolor option expects two non-negative integers "
                        "separated with a comma.\n");
        return false;
    }

    uint32_t pixel = kywc_buffer_get_pixel(kywc_buffer, xy[0], xy[1]);
    printf("pixel position %ld,%ld\tR:%d G:%d B:%d A:%0.1f\n", xy[0], xy[1], (pixel >> 16) & 0xff,
           (pixel >> 8) & 0xff, pixel & 0xff, ((pixel >> 24) & 0xff) / 255.0);
    return true;
}

static bool thumbnail_handle_buffer(kywc_thumbnail *thumbnail,
                                    const struct kywc_thumbnail_buffer *buffer, void *data)
{
    struct capture_state *state = data;

    state->done = true;

    kywc_context *ctx = kywc_thumbnail_get_context(thumbnail);
    struct kywc_buffer_helper *helper = kywc_context_get_user_data(ctx);
    if (!helper) {
        return false;
    }

    struct kywc_buffer *kywc_buffer =
        kywc_buffer_helper_import_thumbnail(helper, thumbnail, buffer);
    if (!kywc_buffer) {
        return false;
    }

    if (state->getpixel) {
        state->ok = capture_pixel(kywc_buffer, state->area);
        return false;
    }

    struct kywc_box area = { 0, 0, (int)buffer->width, (int)buffer->height };
    if (state->area) {
        long vals[4];
        if (!parse_ints(state->area, vals, 4) || vals[0] < 0 || vals[1] < 0 || vals[2] <= 0 ||
            vals[3] <= 0) {
            fprintf(stderr, "The --setarea option expects four integers separated with a comma: "
                            "x,y,width,height.\n");
            return false;
        }
        area.x = vals[0];
        area.y = vals[1];
        area.width = vals[2];
        area.height = vals[3];
    }

    const char *filename = state->path;
    char *generated = NULL;
    if (!filename) {
        generated = (char *)kywc_identifier_time_generate(NULL, ".png");
        filename = generated;
        if (!filename) {
            fprintf(stderr, "Failed to generate the file name.\n");
            return false;
        }
    }

    if (kywc_buffer_write_to_file(kywc_buffer, area.x, area.y, area.width, area.height, filename)) {
        printf("Create file %s success!\n", filename);
        state->ok = true;
    } else {
        fprintf(stderr, "Create file %s failed!\n", filename);
    }

    free(generated);
    return false;
}

static void thumbnail_handle_destroy(kywc_thumbnail *thumbnail, void *data)
{
    struct capture_state *state = data;

    state->done = true;
}

static struct kywc_thumbnail_interface thumbnail_impl = {
    .buffer = thumbnail_handle_buffer,
    .destroy = thumbnail_handle_destroy,
};

static int capture_run(struct context *ctx, const char *source_uuid, const char *output_uuid,
                       struct capture_state *state)
{
    kywc_thumbnail *thumbnail = NULL;
    if (source_uuid && output_uuid) {
        thumbnail = kywc_thumbnail_create_from_workspace(ctx->kywc, source_uuid, output_uuid,
                                                         &thumbnail_impl, state);
    } else if (source_uuid) {
        thumbnail = kywc_thumbnail_create_from_toplevel(ctx->kywc, source_uuid, false,
                                                        &thumbnail_impl, state);
    } else if (output_uuid) {
        thumbnail =
            kywc_thumbnail_create_from_output(ctx->kywc, output_uuid, &thumbnail_impl, state);
    }
    if (!thumbnail) {
        fprintf(stderr, "Failed to create the thumbnail for capture.\n");
        return -1;
    }

    struct wl_display *wl_display = kywc_context_get_display(ctx->kywc);
    int ret = 0;
    while (!state->done && ret >= 0) {
        ret = wl_display_dispatch(wl_display);
    }
    if (ret < 0) {
        fprintf(stderr, "Failed to dispatch the wayland display.\n");
        return -1;
    }
    return state->ok ? 0 : -1;
}

static const char *target_output(struct context *ctx)
{
    return ctx->output ? ctx->output : context_primary_output(ctx);
}

int cmd_windowcapture(struct context *ctx, const char *param)
{
    kywc_toplevel *toplevel = window_resolve(ctx, param);
    if (!toplevel) {
        return -1;
    }

    struct capture_state state = { .path = ctx->path };
    return capture_run(ctx, toplevel->uuid, NULL, &state);
}

int cmd_workspacecapture(struct context *ctx, const char *param)
{
    char *w_uuid, *o_uuid;
    if (!split_pair(param, &w_uuid, &o_uuid)) {
        fprintf(stderr, "The --workspacecapture option expects w_uuid:o_uuid parameters.\n");
        return -1;
    }

    struct capture_state state = { .path = ctx->path };
    int ret = capture_run(ctx, w_uuid, o_uuid, &state);
    free(w_uuid);
    free(o_uuid);
    return ret;
}

int cmd_fullscreencapture(struct context *ctx, const char *param)
{
    struct capture_state state = { .path = ctx->path };
    return capture_run(ctx, NULL, target_output(ctx), &state);
}

int cmd_setarea(struct context *ctx, const char *param)
{
    struct capture_state state = { .area = param, .path = ctx->path };
    return capture_run(ctx, NULL, target_output(ctx), &state);
}

int cmd_getpixelcolor(struct context *ctx, const char *param)
{
    struct capture_state state = { .area = param, .path = ctx->path, .getpixel = true };
    return capture_run(ctx, NULL, target_output(ctx), &state);
}
