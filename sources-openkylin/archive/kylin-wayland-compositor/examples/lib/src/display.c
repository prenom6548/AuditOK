// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <errno.h>
#include <fcntl.h>
#include <gbm.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "linux-dmabuf-v1-client-protocol.h"
#include "wlu_private.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};

static void seat_capabilities(void *data, struct wl_seat *seat, enum wl_seat_capability caps)
{
    struct wlu_display *display = data;

    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !display->keyboard) {
        display->keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(display->keyboard, wlu_input_keyboard_listener(), display);
    } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && display->keyboard) {
        wl_keyboard_destroy(display->keyboard);
        display->keyboard = NULL;
    }

    if ((caps & WL_SEAT_CAPABILITY_POINTER) && !display->pointer) {
        display->pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(display->pointer, wlu_input_pointer_listener(), display);
    } else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && display->pointer) {
        wl_pointer_destroy(display->pointer);
        display->pointer = NULL;
    }
}

static const struct wl_seat_listener seat_listener = {
    .capabilities = seat_capabilities,
};

static void shm_format(void *data, struct wl_shm *wl_shm, uint32_t format)
{
    struct wlu_display *display = data;

    switch (format) {
    case WL_SHM_FORMAT_XRGB8888:
        wlu_shm_format_init(&display->shm_formats[WLU_FORMAT_XRGB8888], 4, format);
        break;
    case WL_SHM_FORMAT_ARGB8888:
        wlu_shm_format_init(&display->shm_formats[WLU_FORMAT_ARGB8888], 4, format);
        break;
    case WL_SHM_FORMAT_BGRX8888:
        wlu_shm_format_init(&display->shm_formats[WLU_FORMAT_BGRX8888], 4, format);
        break;
    case WL_SHM_FORMAT_BGRA8888:
        wlu_shm_format_init(&display->shm_formats[WLU_FORMAT_BGRA8888], 4, format);
        break;
    default:
        break;
    }
}

struct wl_shm_listener shm_listener = {
    .format = shm_format,
};

static void dmabuf_modifiers(void *data, struct zwp_linux_dmabuf_v1 *zwp_linux_dmabuf,
                             uint32_t format, uint32_t modifier_hi, uint32_t modifier_lo)
{
    uint64_t modifier = ((uint64_t)modifier_hi << 32) + modifier_lo;
    struct wlu_display *display = data;

    switch (format) {
    case DRM_FORMAT_XRGB8888:
        wlu_dmabuf_format_init(&display->dmabuf_formats[WLU_FORMAT_XRGB8888], 4, format, modifier);
        break;
    case DRM_FORMAT_ARGB8888:
        wlu_dmabuf_format_init(&display->dmabuf_formats[WLU_FORMAT_ARGB8888], 4, format, modifier);
        break;
    case DRM_FORMAT_BGRX8888:
        wlu_dmabuf_format_init(&display->dmabuf_formats[WLU_FORMAT_BGRX8888], 4, format, modifier);
        break;
    case DRM_FORMAT_BGRA8888:
        wlu_dmabuf_format_init(&display->dmabuf_formats[WLU_FORMAT_BGRA8888], 4, format, modifier);
        break;
    default:
        break;
    }
}

static void dmabuf_format(void *data, struct zwp_linux_dmabuf_v1 *zwp_linux_dmabuf, uint32_t format)
{
}

static const struct zwp_linux_dmabuf_v1_listener dmabuf_listener = {
    .format = dmabuf_format,
    .modifier = dmabuf_modifiers,
};

static void registry_global(void *data, struct wl_registry *registry, uint32_t name,
                            const char *interface, uint32_t version)
{
    struct wlu_display *display = data;

    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        display->compositor =
            wl_registry_bind(registry, name, &wl_compositor_interface, MIN(version, 4));
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        display->xdg_wm_base =
            wl_registry_bind(registry, name, &xdg_wm_base_interface, MIN(version, 4));
        xdg_wm_base_add_listener(display->xdg_wm_base, &xdg_wm_base_listener, display);
    } else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
        display->xdg_decoration_manager =
            wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1);
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        display->seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
        wl_seat_add_listener(display->seat, &seat_listener, display);
    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
        display->shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
        wl_shm_add_listener(display->shm, &shm_listener, display);
    } else if (strcmp(interface, zwp_linux_dmabuf_v1_interface.name) == 0) {
        if (display->flags & WLU_DISPLAY_CREATE_EGL) {
            display->dmabuf =
                wl_registry_bind(registry, name, &zwp_linux_dmabuf_v1_interface, MIN(version, 3));
            zwp_linux_dmabuf_v1_add_listener(display->dmabuf, &dmabuf_listener, display);
        }
    }

    if (display->event_callbacks.registry_global) {
        display->event_callbacks.registry_global(display, registry, name, interface, version);
    }
}

static void registry_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
    struct wlu_display *display = data;
    if (display->event_callbacks.registry_global_remove) {
        display->event_callbacks.registry_global_remove(display, registry, name);
    }
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_global,
    .global_remove = registry_global_remove,
};

static inline bool display_init_gbm(struct wlu_display *display)
{
    const char *drm_render_node = "/dev/dri/renderD128";
    int drm_fd = open(drm_render_node, O_RDWR);
    if (drm_fd < 0) {
        fprintf(stderr, "Failed to open drm render node %s\n", drm_render_node);
        return false;
    }

    struct gbm_device *device = gbm_create_device(drm_fd);
    if (device == NULL) {
        fprintf(stderr, "Failed to create gbm device\n");
        close(drm_fd);
        return false;
    }

    display->gbm.drm_fd = drm_fd;
    display->gbm.device = device;
    return true;
}

static inline bool display_init_egl(struct wlu_display *display)
{
    EGLDisplay egl_display = eglGetPlatformDisplay(EGL_PLATFORM_GBM_KHR, display->gbm.device, NULL);
    if (egl_display == EGL_NO_DISPLAY) {
        fprintf(stderr, "Failed to create EGLDisplay\n");
        return false;
    }

    EGLint major, minor;
    if (eglInitialize(egl_display, &major, &minor) == EGL_FALSE) {
        fprintf(stderr, "Failed to initialize EGLDisplay\n");
        return false;
    }

    display->egl_display = egl_display;
    return true;
}

static bool wlu_display_connect(struct wlu_display *display)
{
    display->display = wl_display_connect(NULL);
    if (display->display == NULL) {
        fprintf(stderr, "Failed to connect to wayland display: %s\n", strerror(errno));
        return false;
    }

    display->registry = wl_display_get_registry(display->display);
    wl_registry_add_listener(display->registry, &registry_listener, display);

    // 第一次roundtrip拿global
    wl_display_roundtrip(display->display);
    // 第二次roundtrip处理事件
    wl_display_roundtrip(display->display);

    if (!display->compositor || !display->xdg_wm_base || !display->seat) {
        fprintf(stderr, "Failed to process wayland connection: %s\n", strerror(errno));
        return false;
    }

    if (display->flags & WLU_DISPLAY_CREATE_EGL) {
        // 不支持zwp_linux_dmabuf协议也认为不支持GL。出现在硬件支持GL但是合成器是软件渲染情况下
        if (!display->dmabuf || !display_init_gbm(display) || !display_init_egl(display)) {
            // 允许EGL创建失败也成功创建display，可以通过wlu_display_supports_egl再判断
            return true;
        }
    }

    return true;
}

struct wlu_display *wlu_display_create(const wlu_display_event_callbacks_t *event_callbacks,
                                       int32_t flags)
{
    struct wlu_display *display = calloc(1, sizeof(*display));
    if (display == NULL) {
        return NULL;
    }

    wl_list_init(&display->window_list);
    if (event_callbacks) {
        display->event_callbacks = *event_callbacks;
    }
    display->flags = flags;

    if (!wlu_display_connect(display)) {
        wlu_display_destroy(display);
        return NULL;
    }

    return display;
}

void wlu_display_destroy(struct wlu_display *display)
{
    struct wlu_window *window, *tmp;
    wl_list_for_each_safe(window, tmp, &display->window_list, link) {
        wl_list_remove(&window->link);
        wlu_window_destroy(window);
    }

    if (display->gbm.device) {
        gbm_device_destroy(display->gbm.device);
    }
    if (display->gbm.drm_fd >= 0) {
        close(display->gbm.drm_fd);
    }

    for (int i = 0; i < WLU_FORMAT_COUNT; i++) {
        struct wlu_dmabuf_format *format = &display->dmabuf_formats[i];
        if (format->modifiers) {
            free(format->modifiers);
        }
    }

    if (display->dmabuf) {
        zwp_linux_dmabuf_v1_destroy(display->dmabuf);
    }
    if (display->shm) {
        wl_shm_destroy(display->shm);
    }
    if (display->seat) {
        wl_seat_destroy(display->seat);
    }
    if (display->xdg_decoration_manager) {
        zxdg_decoration_manager_v1_destroy(display->xdg_decoration_manager);
    }
    if (display->xdg_wm_base) {
        xdg_wm_base_destroy(display->xdg_wm_base);
    }
    if (display->compositor) {
        wl_compositor_destroy(display->compositor);
    }
    if (display->display) {
        wl_display_disconnect(display->display);
    }
    free(display);
}

bool wlu_display_supports_egl(struct wlu_display *display)
{
    return display->egl_display;
}

struct wl_display *wlu_display_get_wl_display(struct wlu_display *display)
{
    return display->display;
}

struct wl_compositor *wlu_display_get_wl_compositor(struct wlu_display *display)
{
    return display->compositor;
}

struct xdg_wm_base *wlu_display_get_xdg_wm_base(struct wlu_display *display)
{
    return display->xdg_wm_base;
}

void wlu_display_run(struct wlu_display *display)
{
    display->running = true;
    while (display->running) {
        if (wl_display_dispatch(display->display) == -1) {
            break;
        }
    }
}

void wlu_display_exit(struct wlu_display *display)
{
    display->running = false;
}
