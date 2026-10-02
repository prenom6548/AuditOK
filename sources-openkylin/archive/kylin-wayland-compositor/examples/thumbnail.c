// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdio.h>
#include <string.h>
#include <wlu.h>

#include "buffer.h"

static struct wlu_display *wlu_display = NULL;
static struct wlu_window *wlu_window = NULL;
struct wlu_buffer *wlu_buffer = NULL;

static void window_draw_frame(struct wlu_window *window, void *data)
{
    size_t swapchain_index = wlu_window_get_swapchain_index(window);
    wlu_draw_texture(&(struct wlu_draw_texture_options){
        .texture = wlu_buffer,
        .target = wlu_window_get_buffer(window, swapchain_index),
    });
}

static void handle_window_destroy(struct wlu_window *window)
{
    wlu_buffer_destroy(wlu_buffer);
    wlu_buffer = NULL;
    wlu_draw_fini(wlu_window);
    wlu_window = NULL;
}

static const wlu_window_event_callbacks_t window_event_callbacks = {
    .destroy = handle_window_destroy,
};

static bool show_buffer_in_window(const struct kywc_thumbnail_buffer *buffer)
{
    bool new_window = false;
    if (!wlu_window) {
        wlu_window = wlu_window_create(&(struct wlu_window_create_info){
            .display = wlu_display,
            .event_callbacks = &window_event_callbacks,
            .app_id = "os.kylin.wlcom.thumbnail",
            .width = buffer->width,
            .height = buffer->height,
            .format = WLU_FORMAT_ARGB8888,
            .title = "Thumbnail Viewer",
            .flags =
                WLU_WINDOW_CREATE_RESIZABLE | WLU_WINDOW_CREATE_DECORATION | WLU_WINDOW_CREATE_GLES,
        });
        wlu_draw_init(wlu_window);
        new_window = true;
    }

    // 初始或窗口关闭后清理了资源或buffer过期，需要重新导入转换wlu_buffer
    if (new_window || !(buffer->flags & KYWC_THUMBNAIL_BUFFER_IS_REUSED)) {
        wlu_buffer_destroy(wlu_buffer);

        struct wlu_shmbuf wlu_shmbuf = {
            .width = buffer->width,
            .height = buffer->height,
            .format = buffer->format,
            .fd = buffer->fd,
            .offset = buffer->offset,
            .stride = buffer->stride,
        };
        if (wlu_window_supports_gl(wlu_window)) {
            if (buffer->flags & KYWC_THUMBNAIL_BUFFER_IS_DMABUF) {
                // 协议接受是dmabuf并且支持gl渲染
                struct wlu_dmabuf wlu_dmabuf = {
                    .width = buffer->width,
                    .height = buffer->height,
                    .format = buffer->format,
                    .modifier = buffer->modifier,
                    .planes_count = buffer->n_planes,
                };
                memcpy(&wlu_dmabuf.planes, &buffer->planes, sizeof(int32_t) * 3 * 4);
                wlu_buffer = wlu_buffer_import_dmabuf(wlu_window, &wlu_dmabuf);
            } else {
                wlu_buffer = wlu_buffer_import_shmbuf_gl(wlu_window, &wlu_shmbuf);
            }
        } else {
            wlu_buffer = wlu_buffer_import_shmbuf(wlu_window, &wlu_shmbuf);
        }
    }

    wlu_window_request_draw(wlu_window, window_draw_frame, NULL);
    wlu_window_show(wlu_window, true);

    return true;
}

static bool thumbnail_update_buffer(kywc_thumbnail *thumbnail,
                                    const struct kywc_thumbnail_buffer *buffer, void *data)
{
    printf("thumbnail %d \"%s\"\n", thumbnail->type, thumbnail->source_uuid);
    printf("  %s%s %d x %d with 0x%x (0x%lx) %d plane\n",
           buffer->flags & KYWC_THUMBNAIL_BUFFER_IS_DMABUF ? "dmabuf" : "memfd",
           buffer->flags & KYWC_THUMBNAIL_BUFFER_IS_REUSED ? "(reused)" : "", buffer->width,
           buffer->height, buffer->format, buffer->modifier, buffer->n_planes);
    for (uint32_t i = 0; i < buffer->n_planes; i++) {
        printf("    %d: fd %d offset %d stride %d\n", i, buffer->planes[i].fd,
               buffer->planes[i].offset, buffer->planes[i].stride);
    }
    printf("\n");

#if 0
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

    kywc_thumbnail_set_user_data(thumbnail, kywc_buffer);

    char path[256];
    // files are named with YUView format
    // static int index = 0;
    // snprintf(path, sizeof(path), "%s-%d_%dx%d_bgra.rgba", thumbnail->source_uuid, index++,
    //         buffer->width, buffer->height);
    snprintf(path, sizeof(path), "%s-%dx%d.png", thumbnail->source_uuid, buffer->width,
             buffer->height);
    kywc_buffer_write_to_file(kywc_buffer, 0, 0, buffer->width, buffer->height, path);
    return false;
#else
    show_buffer_in_window(buffer);
    return true;
#endif
}

static bool thumbnail_handle_buffer(kywc_thumbnail *thumbnail,
                                    const struct kywc_thumbnail_buffer *buffer, void *data)
{
    return thumbnail_update_buffer(thumbnail, buffer, data);
}

static void thumbnail_handle_buffer_update(kywc_thumbnail *thumbnail,
                                           const struct kywc_thumbnail_buffer *buffer, void *data)
{
    bool wants_buffer = thumbnail_update_buffer(thumbnail, buffer, data);
    kywc_thumbnail_release(thumbnail, wants_buffer);
}

static void thumbnail_handle_destroy(kywc_thumbnail *thumbnail, void *data)
{
    printf("thumbnail for %s is gone\n", thumbnail->source_uuid);
    struct kywc_buffer *kywc_buffer = kywc_thumbnail_get_user_data(thumbnail);
    kywc_buffer_destroy(kywc_buffer);
}

static struct kywc_thumbnail_interface thumbnail_impl = {
    .buffer = thumbnail_handle_buffer,
    .destroy = thumbnail_handle_destroy,
    .buffer_update = thumbnail_handle_buffer_update,
};

static void handle_destroy(kywc_context *ctx, void *data)
{
    struct kywc_buffer_helper *helper = data;
    kywc_buffer_helper_destroy(helper);
}

static void handle_create(kywc_context *ctx, void *data)
{
    struct kywc_buffer_helper *helper = kywc_buffer_helper_create(ctx);
    if (!helper) {
        return;
    }

    kywc_context_set_user_data(ctx, helper);
}

static struct kywc_context_interface context_impl = {
    .create = handle_create,
    .destroy = handle_destroy,
};

int main(int argc, char *argv[])
{
    if (argc != 3 && argc != 4) {
        fprintf(stderr, "please input args\n");
        return -1;
    }

    enum kywc_thumbnail_type type = 0;
    const char *source_id, *output_id = NULL;

    const char *option = argv[1];
    if (strcmp(option, "output") == 0) {
        type = KYWC_THUMBNAIL_TYPE_OUTPUT;
        source_id = argv[2];
    } else if (strcmp(option, "toplevel") == 0) {
        type = KYWC_THUMBNAIL_TYPE_TOPLEVEL;
        source_id = argv[2];
    } else if (strcmp(option, "workspace") == 0) {
        if (argc != 4) {
            fprintf(stderr, "please input workspace id and output id both\n");
            return -1;
        }
        type = KYWC_THUMBNAIL_TYPE_WORKSPACE;
        source_id = argv[2];
        output_id = argv[3];
    } else {
        fprintf(stderr, "please input either output, toplevel or workspace\n");
        return -1;
    }

    wlu_display = wlu_display_create(NULL, WLU_DISPLAY_CREATE_EGL);
    if (!wlu_display) {
        return -1;
    }
    struct wl_display *display = wlu_display_get_wl_display(wlu_display);

    uint32_t caps = KYWC_CONTEXT_CAPABILITY_THUMBNAIL_EXT;
    kywc_context *ctx = kywc_context_create_by_display(display, caps, &context_impl, NULL);
    if (!ctx) {
        return -1;
    }

    if (kywc_thumbnail_create(ctx, type, source_id, output_id, &thumbnail_impl, NULL)) {
        kywc_context_dispatch(ctx);
    }

    kywc_context_destroy(ctx);
    wlu_display_destroy(wlu_display);

    return 0;
}
