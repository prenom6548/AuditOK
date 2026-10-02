// SPDX-FileCopyrightText: 2025 The wlroots contributors
// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <dlfcn.h>
#include <drm_fourcc.h>
#include <gbm.h>
#include <stdlib.h>
#include <unistd.h>
#include <xf86drm.h>

#include <wlr/interfaces/wlr_buffer.h>

#include <kywc/log.h>

#include "allocator_p.h"
#include "render/allocator.h"
#include "render/opengl.h"

struct gbm_buffer {
    struct wlr_buffer base;

    struct wl_list link;

    struct gbm_bo *gbm_bo; // NULL if the gbm_device has been destroyed

    struct wlr_dmabuf_attributes dmabuf;
};

struct gbm_alloc {
    int fd;
    struct gbm_device *gbm_device;

    struct wl_list buffers;

    struct gbm_bo *(*create_with_modifiers2)(struct gbm_device *gbm, uint32_t width,
                                             uint32_t height, uint32_t format,
                                             const uint64_t *modifiers, const unsigned int count,
                                             uint32_t flags);
    int (*get_fd_for_plane)(struct gbm_bo *bo, int plane);
    int (*get_format_modifier_plane_count)(struct gbm_device *gbm, uint32_t format,
                                           uint64_t modifier);

    struct wlr_drm_format_set formats;
};

static const struct wlr_buffer_impl buffer_impl;

static struct gbm_buffer *get_gbm_buffer_from_buffer(struct wlr_buffer *wlr_buffer)
{
    assert(wlr_buffer->impl == &buffer_impl);
    struct gbm_buffer *buffer = wl_container_of(wlr_buffer, buffer, base);
    return buffer;
}

static bool export_gbm_bo(struct gbm_alloc *alloc, struct gbm_bo *bo,
                          struct wlr_dmabuf_attributes *out)
{
    struct wlr_dmabuf_attributes attribs = { 0 };

    attribs.n_planes = gbm_bo_get_plane_count(bo);
    if (attribs.n_planes > WLR_DMABUF_MAX_PLANES) {
        kywc_log(KYWC_ERROR, "GBM BO contains too many planes (%d)", attribs.n_planes);
        return false;
    }

    attribs.width = gbm_bo_get_width(bo);
    attribs.height = gbm_bo_get_height(bo);
    attribs.format = gbm_bo_get_format(bo);
    attribs.modifier = gbm_bo_get_modifier(bo);

    int i;
    int32_t handle = -1;

    for (i = 0; i < attribs.n_planes; ++i) {
        if (alloc->get_fd_for_plane != NULL) {
            attribs.fd[i] = alloc->get_fd_for_plane(bo, i);
            if (attribs.fd[i] < 0) {
                kywc_log(KYWC_ERROR, "gbm_bo_get_fd_for_plane failed");
                goto error_fd;
            }
        } else {
            // GBM is lacking a function to get a FD for a given plane. Instead,
            // check all planes have the same handle. We can't use
            // drmPrimeHandleToFD because that messes up handle ref'counting in
            // the user-space driver.
            union gbm_bo_handle plane_handle = gbm_bo_get_handle_for_plane(bo, i);
            if (plane_handle.s32 < 0) {
                kywc_log(KYWC_ERROR, "Gbm_bo_get_handle_for_plane failed");
                goto error_fd;
            }
            if (i == 0) {
                handle = plane_handle.s32;
            } else if (plane_handle.s32 != handle) {
                kywc_log(KYWC_ERROR, "Failed to export GBM BO: "
                                     "all planes don't have the same GEM handle");
                goto error_fd;
            }

            attribs.fd[i] = gbm_bo_get_fd(bo);
            if (attribs.fd[i] < 0) {
                kywc_log(KYWC_ERROR, "gbm_bo_get_fd failed");
                goto error_fd;
            }
        }

        attribs.offset[i] = gbm_bo_get_offset(bo, i);
        attribs.stride[i] = gbm_bo_get_stride_for_plane(bo, i);
    }

    *out = attribs;
    return true;

error_fd:
    for (int j = 0; j < i; ++j) {
        close(attribs.fd[j]);
    }
    return false;
}

static bool drm_format_has(const struct wlr_drm_format *fmt, uint64_t modifier)
{
    for (size_t i = 0; i < fmt->len; ++i) {
        if (fmt->modifiers[i] == modifier) {
            return true;
        }
    }
    return false;
}

static uint32_t gbm_usage_from_flags(uint32_t flags)
{
    uint32_t usage = GBM_BO_USE_SCANOUT;

    if (flags & ALLOCATOR_BUFFER_RENDERING) {
        usage |= GBM_BO_USE_RENDERING;
    }
    if (flags & ALLOCATOR_BUFFER_NO_SCANOUT) {
        usage &= ~GBM_BO_USE_SCANOUT;
    }
    if (flags & ALLOCATOR_BUFFER_CURSOR) {
        usage |= GBM_BO_USE_CURSOR;
    }
    if (flags & ALLOCATOR_BUFFER_LINEAR) {
        usage |= GBM_BO_USE_LINEAR;
    }

    return usage;
}

static struct gbm_buffer *create_buffer(struct gbm_alloc *alloc, int width, int height,
                                        const struct wlr_drm_format *format, uint32_t flags)
{
    assert(format->len > 0);

    errno = 0;
    bool has_modifier = true;
    uint64_t fallback_modifier = DRM_FORMAT_MOD_INVALID;
    struct gbm_bo *bo = NULL;
    struct gbm_device *gbm_device = alloc->gbm_device;
    uint32_t usage = gbm_usage_from_flags(flags);

    if (alloc->create_with_modifiers2) {
        bo = alloc->create_with_modifiers2(gbm_device, width, height, format->format,
                                           format->modifiers, format->len, usage);
    }

    if (!bo && (usage & GBM_BO_USE_SCANOUT)) {
        bo = gbm_bo_create_with_modifiers(gbm_device, width, height, format->format,
                                          format->modifiers, format->len);
    }

    if (!bo) {
        usage |= GBM_BO_USE_RENDERING;
        if (format->len == 1 && format->modifiers[0] == DRM_FORMAT_MOD_LINEAR) {
            usage |= GBM_BO_USE_LINEAR;
            fallback_modifier = DRM_FORMAT_MOD_LINEAR;
        } else if (!drm_format_has(format, DRM_FORMAT_MOD_INVALID)) {
            // If the format doesn't accept an implicit modifier, bail out.
            kywc_log_errno(KYWC_ERROR, "gbm_bo_create_with_modifiers failed");
            return NULL;
        }
        errno = 0;
        bo = gbm_bo_create(gbm_device, width, height, format->format, usage);
        has_modifier = false;
    }
    if (!bo) {
        kywc_log_errno(KYWC_ERROR, "gbm_bo_create failed");
        return NULL;
    }

    struct gbm_buffer *buffer = calloc(1, sizeof(*buffer));
    if (!buffer) {
        gbm_bo_destroy(bo);
        return NULL;
    }
    wlr_buffer_init(&buffer->base, &buffer_impl, width, height);
    buffer->gbm_bo = bo;

    if (!export_gbm_bo(alloc, bo, &buffer->dmabuf)) {
        free(buffer);
        gbm_bo_destroy(bo);
        return NULL;
    }

    // If the buffer has been allocated with an implicit modifier, make sure we
    // don't populate the modifier field: other parts of the stack may not
    // understand modifiers, and they can't strip the modifier.
    if (!has_modifier) {
        buffer->dmabuf.modifier = fallback_modifier;
    }

    wl_list_insert(&alloc->buffers, &buffer->link);

    char *format_name = drmGetFormatName(buffer->dmabuf.format);
    char *modifier_name = drmGetFormatModifierName(buffer->dmabuf.modifier);
    kywc_log(KYWC_DEBUG,
             "Allocated %dx%d GBM buffer "
             "with format %s (0x%08" PRIX32 "), modifier %s (0x%016" PRIX64 ")",
             buffer->base.width, buffer->base.height, format_name ? format_name : "<unknown>",
             buffer->dmabuf.format, modifier_name ? modifier_name : "<unknown>",
             buffer->dmabuf.modifier);
    free(format_name);
    free(modifier_name);

    return buffer;
}

static void buffer_destroy(struct wlr_buffer *wlr_buffer)
{
    struct gbm_buffer *buffer = get_gbm_buffer_from_buffer(wlr_buffer);
    wlr_dmabuf_attributes_finish(&buffer->dmabuf);

    if (buffer->gbm_bo) {
        gbm_bo_destroy(buffer->gbm_bo);
    }

    wl_list_remove(&buffer->link);
    free(buffer);
}

static bool buffer_get_dmabuf(struct wlr_buffer *wlr_buffer, struct wlr_dmabuf_attributes *attribs)
{
    struct gbm_buffer *buffer = get_gbm_buffer_from_buffer(wlr_buffer);
    *attribs = buffer->dmabuf;
    return true;
}

static const struct wlr_buffer_impl buffer_impl = {
    .destroy = buffer_destroy,
    .get_dmabuf = buffer_get_dmabuf,
};

static void query_single_plane_formats(struct gbm_alloc *alloc, struct wlr_renderer *renderer)
{
    if (!renderer) {
        return;
    }

    if (wlr_renderer_is_opengl(renderer)) {
        struct ky_opengl_renderer *r = ky_opengl_renderer_from_wlr_renderer(renderer);
        // modifiers is disabled, no need to query the formats
        if (!r->egl->has_modifiers) {
            return;
        }
    }

    if (!renderer->impl->get_render_formats) {
        return;
    }

    const struct wlr_drm_format_set *render_formats = renderer->impl->get_render_formats(renderer);
    if (!render_formats) {
        return;
    }

    const struct wlr_drm_format *fmt;
    for (size_t i = 0; i < render_formats->len; i++) {
        fmt = &render_formats->formats[i];
        for (size_t j = 0; j < fmt->len; ++j) {
            if (fmt->modifiers[j] == DRM_FORMAT_MOD_INVALID ||
                fmt->modifiers[j] == DRM_FORMAT_MOD_LINEAR) {
                wlr_drm_format_set_add(&alloc->formats, fmt->format, fmt->modifiers[j]);
                continue;
            }
            if (alloc->get_format_modifier_plane_count &&
                alloc->get_format_modifier_plane_count(alloc->gbm_device, fmt->format,
                                                       fmt->modifiers[j]) != 1) {
                continue;
            }
            // treat as single plane if no get_format_modifier_plane_count
            wlr_drm_format_set_add(&alloc->formats, fmt->format, fmt->modifiers[j]);
        }
    }
}

static const struct wlr_drm_format *
get_single_plane_formats(struct gbm_alloc *alloc, uint32_t drm_format)
{
    const struct wlr_drm_format *format =
        wlr_drm_format_set_get(&alloc->formats, drm_format);
    if (!format) {
        kywc_log(KYWC_ERROR, "Renderer doesn't support format 0x%" PRIX32, drm_format);
        return NULL;
    }
    return format;
}

struct wlr_buffer *gbm_buffer_create(struct gbm_alloc *alloc, int width, int height,
                                     const struct wlr_drm_format *format, uint32_t flags)
{
    if (!alloc) {
        return NULL;
    }

    const struct wlr_drm_format *alloc_format = NULL;
    if (flags & ALLOCATOR_BUFFER_SINGLE_PLANE) {
        alloc_format = get_single_plane_formats(alloc, format->format);
    }

    if (!alloc_format) {
        alloc_format = format;
    }

    struct gbm_buffer *buffer = create_buffer(alloc, width, height, alloc_format, flags);
    if (!buffer) {
        return NULL;
    }

    kywc_log(KYWC_DEBUG, "Allocated %" PRIu32 "x%" PRIu32 " GBM buffer", width, height);
    return &buffer->base;
}

void gbm_alloc_destroy(struct gbm_alloc *alloc)
{
    // The gbm_bo objects need to be destroyed before the gbm_device
    struct gbm_buffer *buf, *buf_tmp;
    wl_list_for_each_safe(buf, buf_tmp, &alloc->buffers, link) {
        gbm_bo_destroy(buf->gbm_bo);
        buf->gbm_bo = NULL;
        wl_list_remove(&buf->link);
        wl_list_init(&buf->link);
    }

    wlr_drm_format_set_finish(&alloc->formats);
    gbm_device_destroy(alloc->gbm_device);

    close(alloc->fd);
    free(alloc);
}

struct gbm_alloc *gbm_alloc_create(struct wlr_renderer *renderer, int fd)
{
    uint64_t cap;
    if (drmGetCap(fd, DRM_CAP_PRIME, &cap) || !(cap & DRM_PRIME_CAP_EXPORT)) {
        kywc_log(KYWC_ERROR, "PRIME export not supported");
        return false;
    }

    struct gbm_alloc *alloc = calloc(1, sizeof(*alloc));
    if (!alloc) {
        return NULL;
    }

    alloc->gbm_device = gbm_create_device(fd);
    if (!alloc->gbm_device) {
        kywc_log(KYWC_ERROR, "gbm_create_device failed");
        free(alloc);
        return NULL;
    }

    alloc->fd = fd;

    wl_list_init(&alloc->buffers);

    alloc->create_with_modifiers2 = dlsym(RTLD_DEFAULT, "gbm_bo_create_with_modifiers2");
    if (!alloc->create_with_modifiers2) {
        kywc_log(KYWC_WARN, "No gbm_bo_create_with_modifiers2 support!");
    }

    alloc->get_fd_for_plane = dlsym(RTLD_DEFAULT, "gbm_bo_get_fd_for_plane");
    if (!alloc->get_fd_for_plane) {
        kywc_log(KYWC_WARN, "No gbm_bo_get_fd_for_plane support!");
    }

    alloc->get_format_modifier_plane_count =
        dlsym(RTLD_DEFAULT, "gbm_device_get_format_modifier_plane_count");
    if (!alloc->get_format_modifier_plane_count) {
        kywc_log(KYWC_WARN, "No gbm_device_get_format_modifier_plane_count support!");
    }

    query_single_plane_formats(alloc, renderer);

    kywc_log(KYWC_INFO, "Created GBM allocator with backend %s",
             gbm_device_get_backend_name(alloc->gbm_device));
    char *drm_name = drmGetDeviceNameFromFd2(fd);
    kywc_log(KYWC_DEBUG, "Using DRM node %s", drm_name);
    free(drm_name);

    return alloc;
}
