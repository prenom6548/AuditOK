// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include <drm_fourcc.h>

#include <wlr/backend.h>
#include <wlr/config.h>
#include <wlr/render/allocator.h>
#include <wlr/render/vulkan.h>
#include <wlr/types/wlr_linux_dmabuf_v1.h>

#include <kywc/log.h>

#include "backend/backend.h"
#include "render/allocator.h"
#include "render/drm_syncobj_v1.h"
#include "render/opengl.h"
#include "render/pixel_format.h"
#include "render/pixman.h"
#include "render/profile.h"
#include "render/renderer.h"
#include "renderer_p.h"

struct wlr_renderer *renderer_autocreate(struct wlr_backend *backend, struct server *server)
{
    struct wlr_renderer *renderer = NULL;
    const char *api = getenv("KYWC_RENDERER");
    if (api && strcmp(api, "pixman") == 0) {
        renderer = pixman_renderer_create();
        goto done;
    }

    /* get drm fd from backend */
    int drm_fd = wlr_backend_get_drm_fd(backend);
    if (drm_fd < 0) {
        kywc_log(KYWC_ERROR, "Cannot create hardware renderer: no DRM fd available");
    } else {
        if (api && strcmp(api, "vulkan") == 0) {
#if WLR_HAS_VULKAN_RENDERER
            renderer = wlr_vk_renderer_create_with_drm_fd(drm_fd);
#else
            kywc_log(KYWC_ERROR, "Cannot create Vulkan renderer: wlroots not compiled with Vulkan");
#endif
        } else {
            renderer = opengl_renderer_create_with_drm_fd(drm_fd);
        }
    }

    if (!renderer) {
        kywc_log(KYWC_WARN, "Failed to create a hardware renderer");
        renderer = pixman_renderer_create();
    }

done:
    if (!renderer) {
        kywc_log(KYWC_ERROR, "Failed to create a pixman renderer");
        kywc_log(KYWC_ERROR, "Could not initialize renderer");
    } else {
        renderer_config_init(server);
    }

    PROFILE_RENDER_CREATE(renderer);

    return renderer;
}

bool renderer_init_wl_display(struct wlr_renderer *renderer, struct wlr_backend *backend,
                              struct wl_display *wl_display,
                              struct wlr_linux_dmabuf_v1 **linux_dmabuf_v1)
{
    wlr_renderer_init_wl_shm(renderer, wl_display);
    single_pixel_buffer_manager_create(wl_display);

    if (!wlr_renderer_get_dmabuf_texture_formats(renderer)) {
        kywc_log(KYWC_WARN, "Unable to initialize dmabuf");
        return true;
    }

    char *env = getenv("KYWC_ENABLE_TIMELINE");
    if ((env && strcmp(env, "1") == 0) && renderer_support_timeline(renderer) &&
        backend_support_timeline(backend)) {
        drm_syncobj_manager_v1_create(wl_display, wlr_renderer_get_drm_fd(renderer));
    }

    *linux_dmabuf_v1 = wlr_linux_dmabuf_v1_create_with_renderer(wl_display, 4, renderer);

    if (!wayland_buffer_create(wl_display, renderer)) {
        /* create wl_drm if not created in driver */
        int master_fd = wlr_backend_get_drm_fd(backend);
        if (master_fd >= 0) {
            wayland_drm_create(wl_display, renderer, master_fd);
        } else {
            kywc_log(KYWC_WARN, "Cannot get renderer DRM FD, disabling wl_drm");
        }
    }

    return true;
}

const struct wlr_drm_format_set *renderer_get_render_formats(struct wlr_renderer *renderer)
{
    if (!renderer->impl->get_render_formats) {
        return NULL;
    }
    return renderer->impl->get_render_formats(renderer);
}

const struct wlr_drm_format *renderer_get_render_format(struct wlr_renderer *renderer, uint32_t fmt)
{
    const struct wlr_drm_format_set *render_formats = renderer_get_render_formats(renderer);
    if (!render_formats) {
        kywc_log(KYWC_ERROR, "Failed to get render formats");
        return NULL;
    }

    const struct wlr_drm_format *render_format = wlr_drm_format_set_get(render_formats, fmt);
    if (!render_format) {
        kywc_log(KYWC_ERROR, "Renderer doesn't support format 0x%" PRIX32, fmt);
        return NULL;
    }

    return render_format;
}

struct wlr_buffer *renderer_create_buffer(struct wlr_renderer *renderer,
                                          struct wlr_allocator *alloc, int width, int height,
                                          uint32_t fmt, bool single_plane)
{
    uint32_t flags = ALLOCATOR_BUFFER_NO_SCANOUT;
    if (single_plane) {
        flags |= ALLOCATOR_BUFFER_SINGLE_PLANE;
    }

    enum allocator_buffer_type type = wlr_renderer_is_pixman_renderer(renderer)
                                          ? ALLOCATOR_BUFFER_TYPE_SHM
                                          : ALLOCATOR_BUFFER_TYPE_GBM;

    const struct wlr_drm_format *format = renderer_get_render_format(renderer, fmt);
    return allocator_create_buffer(alloc, type, width, height, format, flags);
}

bool renderer_is_software(struct wlr_renderer *renderer)
{
    if (wlr_renderer_is_pixman_renderer(renderer)) {
        return true;
    }

    /* return true if software opengl */
    if (wlr_renderer_is_opengl(renderer)) {
        struct opengl_renderer *r = opengl_renderer_from_wlr_renderer(renderer);
        return r->egl->is_software;
    }

    return false;
}

bool renderer_support_timeline(struct wlr_renderer *renderer)
{
    if (!wlr_renderer_is_opengl(renderer)) {
        return false;
    }

    struct opengl_renderer *r = opengl_renderer_from_wlr_renderer(renderer);
    return r->egl->support_timeline;
}

struct wlr_buffer *renderer_upload_pixels(struct wlr_renderer *renderer,
                                          struct wlr_allocator *alloc, int width, int height,
                                          struct wlr_buffer *pixels)
{
    /* upload is meaningless when pixman */
    if (wlr_renderer_is_pixman_renderer(renderer)) {
        return NULL;
    }

    void *data = NULL;
    uint32_t drm_format;
    size_t stride;
    if (!wlr_buffer_begin_data_ptr_access(pixels, WLR_BUFFER_DATA_PTR_ACCESS_READ, &data,
                                          &drm_format, &stride)) {
        return NULL;
    }
    wlr_buffer_end_data_ptr_access(pixels);

    struct wlr_buffer *buffer =
        renderer_create_buffer(renderer, alloc, width, height, drm_format, false);
    if (!buffer) {
        return NULL;
    }

    struct wlr_render_pass *pass = wlr_renderer_begin_buffer_pass(renderer, buffer, NULL);
    if (!pass) {
        wlr_buffer_drop(buffer);
        return NULL;
    }

    struct wlr_texture *tex = wlr_texture_from_buffer(renderer, pixels);
    struct wlr_render_texture_options options = {
        .dst_box = { 0, 0, width, height },
        .texture = tex,
        .blend_mode = WLR_RENDER_BLEND_MODE_NONE,
    };
    wlr_render_pass_add_texture(pass, &options);
    wlr_texture_destroy(tex);
    wlr_render_pass_submit(pass);

    return buffer;
}

bool renderer_check_render_buffer(struct wlr_renderer *renderer, struct wlr_buffer *buffer)
{
    uint32_t caps = renderer->impl->get_render_buffer_caps(renderer);
    bool renderer_support_dmabuf = caps & WLR_BUFFER_CAP_DMABUF;

    struct wlr_dmabuf_attributes dmabuf;
    bool buffer_is_dmabuf = wlr_buffer_get_dmabuf(buffer, &dmabuf);

    return renderer_support_dmabuf == buffer_is_dmabuf;
}

bool render_buffer_is_opaque(struct wlr_buffer *buffer)
{
    if (!buffer) {
        return false;
    }

    void *data;
    uint32_t format;
    size_t stride;
    struct wlr_dmabuf_attributes dmabuf;
    struct wlr_shm_attributes shm;
    if (wlr_buffer_get_dmabuf(buffer, &dmabuf)) {
        format = dmabuf.format;
    } else if (wlr_buffer_get_shm(buffer, &shm)) {
        format = shm.format;
    } else if (wlr_buffer_is_wayland_buffer(buffer)) {
        return wayland_buffer_is_opaque(buffer);
    } else if (wlr_buffer_begin_data_ptr_access(buffer, WLR_BUFFER_DATA_PTR_ACCESS_READ, &data,
                                                &format, &stride)) {
        bool opaque = false;
        if (buffer->width == 1 && buffer->height == 1 && format == DRM_FORMAT_ARGB8888) {
            // Special case for single-pixel-buffer-v1
            const uint8_t *argb8888 = data; // little-endian byte order
            opaque = argb8888[3] == 0xFF;
        }
        wlr_buffer_end_data_ptr_access(buffer);
        if (opaque) {
            return true;
        }
    } else {
        return false;
    }

    return pixel_format_is_opaque(format);
}
