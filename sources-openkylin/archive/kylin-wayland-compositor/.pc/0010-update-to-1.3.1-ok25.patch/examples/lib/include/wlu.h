// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _WLU_H_
#define _WLU_H_

#include <stdbool.h>
#include <stdint.h>
#include <wayland-client.h>

#include "xdg-shell-client-protocol.h"

enum wlu_format {
    WLU_FORMAT_XRGB8888,
    WLU_FORMAT_ARGB8888,
    WLU_FORMAT_BGRX8888,
    WLU_FORMAT_BGRA8888,
    WLU_FORMAT_COUNT,
};

struct wlu_display;

enum wlu_display_create_flag {
    WLU_DISPLAY_CREATE_NONE = 0,
    WLU_DISPLAY_CREATE_EGL = 1 << 0,
};

typedef struct {
    void (*registry_global)(struct wlu_display *display, struct wl_registry *registry,
                            uint32_t name, const char *interface, uint32_t version);
    void (*registry_global_remove)(struct wlu_display *display, struct wl_registry *registry,
                                   uint32_t name);
} wlu_display_event_callbacks_t;

struct wlu_display *wlu_display_create(const wlu_display_event_callbacks_t *event_callbacks,
                                       int32_t flags);

void wlu_display_destroy(struct wlu_display *display);

struct wl_display *wlu_display_get_wl_display(struct wlu_display *display);

struct wl_compositor *wlu_display_get_wl_compositor(struct wlu_display *display);

struct xdg_wm_base *wlu_display_get_xdg_wm_base(struct wlu_display *display);

void wlu_display_run(struct wlu_display *display);

void wlu_display_exit(struct wlu_display *display);

struct wlu_window;

enum { WLU_SWAPCHAIN_COUNT = 2 };

enum wlu_window_create_flag {
    WLU_WINDOW_CREATE_NONE = 0,
    WLU_WINDOW_CREATE_RESIZABLE = 1 << 0,
    WLU_WINDOW_CREATE_TRANSPARENT = 1 << 1,
    WLU_WINDOW_CREATE_DECORATION = 1 << 2,
    WLU_WINDOW_CREATE_GL = 1 << 3,
    WLU_WINDOW_CREATE_GLES = 1 << 4,
};

typedef struct {
    /// 窗口创建完毕。可在此函数调用wayland协议函数，执行回调之后内部会commit一次
    void (*create)(struct wlu_window *window);
    void (*resize)(struct wlu_window *window, int32_t width, int32_t height);
    void (*keyboard)(struct wlu_window *window, uint32_t key, uint32_t state);
    void (*pointer_motion)(struct wlu_window *window, double x, double y);
    void (*pointer_button)(struct wlu_window *window, uint32_t button, uint32_t state, double x,
                           double y);
} wlu_window_event_callbacks_t;

struct wlu_window_create_info {
    struct wlu_display *display;
    const wlu_window_event_callbacks_t *event_callbacks;
    const char *app_id;
    int32_t width;
    int32_t height;
    enum wlu_format format;
    const char *title;
    int32_t flags;
};

/// 支持多个窗口。如果需要OpenGL窗口，display需要WLU_DISPLAY_CREATE_EGL
struct wlu_window *wlu_window_create(struct wlu_window_create_info *info);

/// display摧毁时候会自动调用，只有主动关闭窗口时候才需要调用
void wlu_window_destroy(struct wlu_window *window);

struct wlu_display *wlu_window_get_display(struct wlu_window *window);

int32_t wlu_window_get_width(struct wlu_window *window);

int32_t wlu_window_get_height(struct wlu_window *window);

struct wl_surface *wlu_window_get_wl_surface(struct wlu_window *window);

struct xdg_surface *wlu_window_get_xdg_surface(struct wlu_window *window);

struct xdg_toplevel *wlu_window_get_xdg_toplevel(struct wlu_window *window);

size_t wlu_window_get_swapchain_index(struct wlu_window *window);

bool wlu_window_maximized(struct wlu_window *window);

bool wlu_window_minimized(struct wlu_window *window);

void wlu_window_set_maximized(struct wlu_window *window, bool maximized);

void wlu_window_set_minimized(struct wlu_window *window);

/// 可以在回调函数递归调用request_draw，也可以在任意事件响应调用
typedef void (*wlu_window_draw_callback_t)(struct wlu_window *, void *);
void wlu_window_request_draw(struct wlu_window *window, wlu_window_draw_callback_t callback,
                             void *data);

/// 在draw回调函数里面指定损伤区域，支持多次调用自动合并区域
void wlu_window_damage(struct wlu_window *window, int32_t x, int32_t y, int32_t width,
                       int32_t height);

/// 配合draw函数，写入像素数据即可渲染像素
void *wlu_window_get_shm_data(struct wlu_window *window, size_t swapchain_index);

/// 配合draw函数，OpenGL窗口绘制之前需要glBindFramebuffer此fbo
uint32_t wlu_window_get_fbo(struct wlu_window *window, size_t swapchain_index);

#endif // _WLU_H_
