// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _WLU_PRIVATE_H_
#define _WLU_PRIVATE_H_

#include <drm_fourcc.h>
#include <pixman.h>
#include <stdlib.h>
// clang-format off
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
// clang-format on

#include "wlu.h"

#include "xdg-decoration-unstable-v1-client-protocol.h"
#include "xdg-shell-client-protocol.h"

struct wlu_shm_format {
    bool supported;
    uint8_t bytes_per_pixel;
    enum wl_shm_format format;
};

struct wlu_dmabuf_format {
    bool supported;
    uint8_t bytes_per_pixel;
    uint32_t format;
    uint64_t *modifiers;
    size_t modifiers_count;
};

struct wlu_display {
    wlu_display_event_callbacks_t event_callbacks;
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct xdg_wm_base *xdg_wm_base;
    struct zxdg_decoration_manager_v1 *xdg_decoration_manager;
    struct wl_seat *seat;
    struct wl_keyboard *keyboard;
    struct wl_pointer *pointer;
    struct wl_shm *shm;
    struct zwp_linux_dmabuf_v1 *dmabuf;
    int32_t flags;
    bool running;
    struct {
        double x;
        double y;
    } pointer_position;

    struct wl_list window_list;
    struct wlu_window *activated_window;

    struct wlu_shm_format shm_formats[WLU_FORMAT_COUNT];
    struct wlu_dmabuf_format dmabuf_formats[WLU_FORMAT_COUNT];

    struct {
        int drm_fd;
        struct gbm_device *device;
    } gbm;
    EGLDisplay egl_display;
};

struct wlu_buffer {
    struct wl_buffer *buffer;
    int32_t width;
    int32_t height;
    enum wlu_format format;

    struct wlu_window *window;
    bool compositor_using;

    enum wlu_buffer_type type;
    union {
        struct {
            size_t size;
            void *data;
        } shm;
        struct {
            struct gbm_bo *bo;
            EGLImageKHR image;
            GLuint texture;
            GLuint fbo;
        } gl;
    } backend;
};

struct wlu_window {
    struct wl_list link; /* struct display::window_list */
    struct wl_surface *surface;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;
    struct zxdg_toplevel_decoration_v1 *xdg_toplevel_decoration;
    bool xdg_surface_configured;
    struct wlu_display *display;
    int32_t width;
    int32_t height;
    int32_t pending_width;
    int32_t pending_height;
    enum wlu_format buffer_format;
    enum wlu_buffer_type buffer_type;
    pixman_region32_t damage;
    wlu_window_event_callbacks_t event_callbacks;

    /// 双缓冲交换链
    struct wlu_buffer *swapchain[WLU_SWAPCHAIN_COUNT];
    size_t swapchain_index;

    /// request_draw只执行一次，遇到超时占用再次调用时候防止执行多次
    struct {
        bool done;
        wlu_window_draw_callback_t callback;
        void *data;
    } draw_pending;
    /// 合成器发送完帧同步信号置否，避免超过渲染次数执行超过屏幕
    bool frame_done;

    struct {
        EGLConfig config;
        EGLContext context;
        bool has_image_dma_buf_import;
        bool has_image_dma_buf_import_modifiers;
        PFNEGLCREATEIMAGEKHRPROC create_image;
        PFNEGLDESTROYIMAGEKHRPROC destroy_image;
        PFNGLEGLIMAGETARGETTEXTURE2DOESPROC image_target_texture_2d;
    } egl;

    bool visible;
    bool transparent;
    bool maximized;
    bool minimized;
};

static inline void wlu_shm_format_init(struct wlu_shm_format *wlu_format, uint8_t bytes_per_pixel,
                                       uint32_t format)
{
    wlu_format->supported = true;
    wlu_format->bytes_per_pixel = bytes_per_pixel;
    wlu_format->format = format;
}

static inline void wlu_dmabuf_format_init(struct wlu_dmabuf_format *wlu_format,
                                          uint8_t bytes_per_pixel, uint32_t format,
                                          uint64_t modifier)
{
    wlu_format->supported = true;
    wlu_format->bytes_per_pixel = bytes_per_pixel;
    wlu_format->format = format;

    if (modifier != DRM_FORMAT_MOD_INVALID) {
        ++wlu_format->modifiers_count;
        wlu_format->modifiers = realloc(wlu_format->modifiers, wlu_format->modifiers_count *
                                                                   sizeof(*wlu_format->modifiers));
        wlu_format->modifiers[wlu_format->modifiers_count - 1] = modifier;
    }
}

uint32_t wlu_format_to_shm(enum wlu_format format);

enum wlu_format wlu_format_from_shm(uint32_t format);

uint32_t wlu_format_to_drm(enum wlu_format format);

enum wlu_format wlu_format_from_drm(uint32_t format);

pixman_format_code_t wlu_format_to_pixman(enum wlu_format format);

const struct wl_keyboard_listener *wlu_input_keyboard_listener(void);

const struct wl_pointer_listener *wlu_input_pointer_listener(void);

void wlu_window_try_invoke_draw(struct wlu_window *window);

#endif // _WLU_PRIVATE_H_
