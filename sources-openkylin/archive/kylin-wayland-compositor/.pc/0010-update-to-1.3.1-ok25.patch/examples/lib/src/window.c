// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "wlu_private.h"

static inline void commit_buffer(struct wlu_window *window, struct wlu_buffer *buffer)
{
    if (window->buffer_type == WLU_WINDOW_BUFFER_TYPE_DMABUF_EGL) {
        glFlush();
    }

    wl_surface_attach(window->surface, buffer->buffer, 0, 0);

    struct wl_region *opaque_region = NULL;
    if (!window->transparent) {
        opaque_region = wl_compositor_create_region(window->display->compositor);
        wl_region_add(opaque_region, 0, 0, INT32_MAX, INT32_MAX);
        wl_surface_set_opaque_region(window->surface, opaque_region);
    }

    if (pixman_region32_not_empty(&window->damage)) {
        pixman_region32_t damage_region;
        pixman_region32_init_rect(&damage_region, 0, 0, window->width, window->height);
        pixman_region32_intersect(&damage_region, &damage_region, &window->damage);
        pixman_box32_t *damage = pixman_region32_extents(&damage_region);
        wl_surface_damage(window->surface, damage->x1, damage->y1, damage->x2 - damage->x1,
                          damage->y2 - damage->y1);
        pixman_region32_fini(&damage_region);
    } else {
        wl_surface_damage(window->surface, 0, 0, window->width, window->height);
    }

    wl_surface_commit(window->surface);

    if (opaque_region) {
        wl_region_destroy(opaque_region);
    }

    pixman_region32_clear(&window->damage);

    buffer->compositor_using = true;
}

static void frame_done(void *data, struct wl_callback *callback, uint32_t time)
{
    if (callback) {
        wl_callback_destroy(callback);
    }

    struct wlu_window *window = data;
    window->frame_done = true;
    wlu_window_try_invoke_draw(window);
}
static const struct wl_callback_listener frame_listener = {
    .done = frame_done,
};

static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial)
{
    xdg_surface_ack_configure(xdg_surface, serial);

    struct wlu_window *window = data;
    if (window->width != window->pending_width || window->height != window->pending_height) {
        if (!window->xdg_surface_configured) {
            window->xdg_surface_configured = true;
        } else {
            // 窗口大小调整
            for (int i = 0; i < WLU_SWAPCHAIN_COUNT; i++) {
                if (window->swapchain[i]) {
                    wlu_buffer_destroy(window->swapchain[i]);
                    window->swapchain[i] = NULL;
                }
            }
            if (window->pending_width > 0 && window->pending_height > 0) {
                window->width = window->pending_width;
                window->height = window->pending_height;
            }
        }

        for (int i = 0; i < WLU_SWAPCHAIN_COUNT; i++) {
            window->swapchain[i] = wlu_buffer_create(window);
            assert(window->swapchain[i] && "Failed to create swapchain buffer");
        }

        struct wl_callback *frame_callback = wl_surface_frame(window->surface);
        wl_callback_add_listener(frame_callback, &frame_listener, window);
        window->frame_done = false;

        window->swapchain_index = 0;
        struct wlu_buffer *buffer = window->swapchain[window->swapchain_index];
        // 执行resize回调
        if (window->event_callbacks.resize) {
            window->event_callbacks.resize(window, window->width, window->height);
        }
        // 等待frame_done来触发draw回调，调整窗口大小会闪。所以立即调用draw回调
        if (window->draw_pending.callback) {
            window->draw_pending.done = true;
            window->draw_pending.callback(window, window->draw_pending.data);
        }
        commit_buffer(window, buffer);
    }
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

static void xdg_toplevel_configure(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width,
                                   int32_t height, struct wl_array *states)
{
    struct wlu_window *window = data;
    if (width > 0 && height > 0) {
        window->pending_width = width;
        window->pending_height = height;
    }

    uint32_t *state;
    wl_array_for_each(state, states) {
        switch (*state) {
        case XDG_TOPLEVEL_STATE_ACTIVATED:
            window->minimized = false;
            window->display->activated_window = window;
            break;
        default:
            break;
        }
    }
}

static void xdg_toplevel_close(void *data, struct xdg_toplevel *xdg_toplevel)
{
    struct wlu_window *window = data;
    wlu_window_destroy(window);
}

static void xdg_toplevel_configure_bounds(void *data, struct xdg_toplevel *xdg_toplevel,
                                          int32_t width, int32_t height)
{
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure,
    .close = xdg_toplevel_close,
    .configure_bounds = xdg_toplevel_configure_bounds,
};

static bool window_init_gl(struct wlu_window *window, bool use_gles)
{
    eglBindAPI(use_gles ? EGL_OPENGL_ES_API : EGL_OPENGL_API);

    // clang-format off
    EGLint config_attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
		EGL_NONE
	};
    // clang-format on
    if (use_gles) {
        config_attribs[11] = EGL_OPENGL_ES_BIT;
    }
    EGLint num_config;
    EGLBoolean ret = eglChooseConfig(window->display->egl_display, config_attribs,
                                     &window->egl.config, 1, &num_config);
    assert((ret && num_config >= 1) && "eglChooseConfig failure");

    // clang-format off
    const EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
    // clang-format on
    window->egl.context = eglCreateContext(window->display->egl_display, window->egl.config,
                                           EGL_NO_CONTEXT, context_attribs);
    if (window->egl.context == EGL_NO_CONTEXT) {
        fprintf(stderr, "Failed to create EGLContext\n");
        goto error;
    }

    eglMakeCurrent(window->display->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                   window->egl.context);

    window->egl.image_target_texture_2d = (void *)eglGetProcAddress("glEGLImageTargetTexture2DOES");
    assert(window->egl.image_target_texture_2d);

    const char *version = (const char *)glGetString(GL_VERSION);
    const char *vendor = (const char *)glGetString(GL_VENDOR);
    const char *renderer = (const char *)glGetString(GL_RENDERER);
    const char *glsl_version = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
    printf("===== OpenGL Information =====\n");
    printf("Version:    %s\n", version);
    printf("Vendor:     %s\n", vendor);
    printf("Renderer:   %s\n", renderer);
    printf("GLSL Ver:   %s\n", glsl_version);
    printf("=================================\n");

    return true;

error:
    return false;
}

struct wlu_window *wlu_window_create(struct wlu_window_create_info *info)
{
    bool use_egl = info->flags & (WLU_WINDOW_CREATE_GL | WLU_WINDOW_CREATE_GLES);
    // 检查格式是否支持
    bool format_supported = use_egl ? info->display->dmabuf_formats[info->format].supported
                                    : info->display->shm_formats[info->format].supported;
    if (!format_supported) {
        fprintf(stderr, "Failed to create window. Unsupported format\n");
        return NULL;
    }

    struct wlu_window *window = calloc(1, sizeof(*window));
    if (window == NULL) {
        return NULL;
    }
    window->display = info->display;
    if (info->event_callbacks) {
        window->event_callbacks = *info->event_callbacks;
    }
    window->width = info->width;
    window->height = info->height;
    window->buffer_format = info->format;
    window->buffer_type = use_egl ? WLU_WINDOW_BUFFER_TYPE_DMABUF_EGL : WLU_WINDOW_BUFFER_TYPE_SHM;
    window->transparent = info->flags & WLU_WINDOW_CREATE_TRANSPARENT;
    pixman_region32_init(&window->damage);
    wl_list_insert(window->display->window_list.prev, &window->link);

    if (window->buffer_type == WLU_WINDOW_BUFFER_TYPE_DMABUF_EGL) {
        if (!window_init_gl(window, info->flags & WLU_WINDOW_CREATE_GLES)) {
            wlu_window_destroy(window);
            return NULL;
        }
    }

    window->surface = wl_compositor_create_surface(info->display->compositor);
    if (!window->surface) {
        fprintf(stderr, "Failed to create wayland surface: %s\n", strerror(errno));
        wlu_window_destroy(window);
        return NULL;
    }

    window->xdg_surface = xdg_wm_base_get_xdg_surface(info->display->xdg_wm_base, window->surface);
    window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
    if (!window->xdg_surface || !window->xdg_toplevel) {
        wlu_window_destroy(window);
        return NULL;
    }
    if (!(info->flags & WLU_WINDOW_CREATE_RESIZABLE)) {
        xdg_toplevel_set_min_size(window->xdg_toplevel, window->width, window->height);
        xdg_toplevel_set_max_size(window->xdg_toplevel, window->width, window->height);
    }
    if (info->display->xdg_decoration_manager && (info->flags & WLU_WINDOW_CREATE_DECORATION)) {
        window->xdg_toplevel_decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(
            info->display->xdg_decoration_manager, window->xdg_toplevel);
        zxdg_toplevel_decoration_v1_set_mode(window->xdg_toplevel_decoration,
                                             ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }

    xdg_surface_add_listener(window->xdg_surface, &xdg_surface_listener, window);
    xdg_toplevel_add_listener(window->xdg_toplevel, &xdg_toplevel_listener, window);

    assert(info->app_id && "must set window display id");
    xdg_toplevel_set_app_id(window->xdg_toplevel, info->app_id);
    if (info->title) {
        xdg_toplevel_set_title(window->xdg_toplevel, info->title);
    }

    if (window && window->event_callbacks.create) {
        window->event_callbacks.create(window);
    }

    wl_surface_commit(window->surface);

    return window;
}

void wlu_window_destroy(struct wlu_window *window)
{
    struct wlu_window *window_tmp, *tmp;
    wl_list_for_each_safe(window_tmp, tmp, &window->display->window_list, link) {
        if (window_tmp == window) {
            wl_list_remove(&window_tmp->link);
            wl_list_init(&window_tmp->link);
        }
    }
    if (wl_list_empty(&window->display->window_list)) {
        wlu_display_exit(window->display);
    }

    pixman_region32_fini(&window->damage);

    for (int i = 0; i < WLU_SWAPCHAIN_COUNT; i++) {
        struct wlu_buffer *buffer = window->swapchain[i];
        wlu_buffer_destroy(buffer);
    }
    if (window->xdg_toplevel_decoration) {
        zxdg_toplevel_decoration_v1_destroy(window->xdg_toplevel_decoration);
    }
    if (window->xdg_toplevel) {
        xdg_toplevel_destroy(window->xdg_toplevel);
    }
    if (window->xdg_surface) {
        xdg_surface_destroy(window->xdg_surface);
    }
    if (window->surface) {
        wl_surface_destroy(window->surface);
    }
    free(window);
}

struct wlu_display *wlu_window_get_display(struct wlu_window *window)
{
    return window->display;
}

int32_t wlu_window_get_width(struct wlu_window *window)
{
    return window->width;
}

int32_t wlu_window_get_height(struct wlu_window *window)
{
    return window->height;
}

struct wl_surface *wlu_window_get_wl_surface(struct wlu_window *window)
{
    return window->surface;
}

struct xdg_surface *wlu_window_get_xdg_surface(struct wlu_window *window)
{
    return window->xdg_surface;
}

struct xdg_toplevel *wlu_window_get_xdg_toplevel(struct wlu_window *window)
{
    return window->xdg_toplevel;
}

size_t wlu_window_get_swapchain_index(struct wlu_window *window)
{
    return window->swapchain_index;
}

bool wlu_window_maximized(struct wlu_window *window)
{
    return window->maximized;
}

bool wlu_window_minimized(struct wlu_window *window)
{
    return window->minimized;
}

void wlu_window_set_maximized(struct wlu_window *window, bool maximized)
{
    if (maximized) {
        xdg_toplevel_set_maximized(window->xdg_toplevel);
    } else {
        xdg_toplevel_unset_maximized(window->xdg_toplevel);
    }
    window->maximized = maximized;
}

void wlu_window_set_minimized(struct wlu_window *window)
{
    xdg_toplevel_set_minimized(window->xdg_toplevel);
    window->minimized = true;
}

void wlu_window_request_draw(struct wlu_window *window, wlu_window_draw_callback_t callback,
                             void *data)
{
    window->draw_pending.done = false;
    window->draw_pending.callback = callback;
    window->draw_pending.data = data;

    if (window->frame_done) {
        wlu_window_try_invoke_draw(window);
    }
}

void wlu_window_damage(struct wlu_window *window, int32_t x, int32_t y, int32_t width,
                       int32_t height)
{
    pixman_region32_union_rect(&window->damage, &window->damage, x, y, width, height);
}

void *wlu_window_get_shm_data(struct wlu_window *window, size_t swapchain_index)
{
    if (window->buffer_type != WLU_WINDOW_BUFFER_TYPE_SHM) {
        return NULL;
    }

    struct wlu_buffer *buffer = window->swapchain[swapchain_index];
    if (buffer) {
        return buffer->data.shm.data;
    }

    return NULL;
}

uint32_t wlu_window_get_fbo(struct wlu_window *window, size_t swapchain_index)
{
    if (window->buffer_type != WLU_WINDOW_BUFFER_TYPE_DMABUF_EGL) {
        return 0;
    }

    struct wlu_buffer *buffer = window->swapchain[swapchain_index];
    if (buffer) {
        return buffer->data.egl.fbo;
    }

    return 0;
}

static inline struct wlu_buffer *acquire_next_buffer(struct wlu_window *window)
{
    // 从当前index下一个位置开始找，实现循环轮询
    for (size_t offset = 1; offset <= WLU_SWAPCHAIN_COUNT; offset++) {
        size_t i = (window->swapchain_index + offset) % WLU_SWAPCHAIN_COUNT;
        struct wlu_buffer *buffer = window->swapchain[i];
        if (buffer && !buffer->compositor_using) {
            window->swapchain_index = i;
            return buffer;
        }
    }
    return NULL;
}

void wlu_window_try_invoke_draw(struct wlu_window *window)
{
    if (window->draw_pending.done || !window->draw_pending.callback || !window->frame_done) {
        return;
    }

    struct wlu_buffer *buffer = acquire_next_buffer(window);
    if (!buffer) {
        // 合成器一直占用buffer，等待compositor_release_buffer事件再次触发此函数
        return;
    }

    window->draw_pending.done = true;

    struct wl_callback *frame_callback = wl_surface_frame(window->surface);
    wl_callback_add_listener(frame_callback, &frame_listener, window);
    window->frame_done = false;

    window->draw_pending.callback(window, window->draw_pending.data);

    commit_buffer(window, buffer);
}
