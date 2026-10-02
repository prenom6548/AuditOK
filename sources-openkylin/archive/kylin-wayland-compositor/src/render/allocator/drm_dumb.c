// SPDX-FileCopyrightText: 2024 The wlroots contributors
// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <drm_fourcc.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include <wlr/interfaces/wlr_buffer.h>

#include <kywc/log.h>

#include "allocator_p.h"
#include "render/pixel_format.h"

struct dumb_buffer {
    struct wlr_buffer base;

    struct wl_list link;

    int drm_fd; // -1 if the allocator has been destroyed
    struct wlr_dmabuf_attributes dmabuf;

    uint32_t format;
    uint32_t handle;
    uint32_t stride;
    uint32_t width, height;

    uint64_t size;
    void *data;
};

struct dumb_alloc {
    int drm_fd;
    struct wl_list buffers;
};

static const struct wlr_buffer_impl buffer_impl;

static struct dumb_buffer *dumb_buffer_from_buffer(struct wlr_buffer *wlr_buffer)
{
    assert(wlr_buffer->impl == &buffer_impl);
    struct dumb_buffer *buffer = wl_container_of(wlr_buffer, buffer, base);
    return buffer;
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

struct wlr_buffer *dumb_buffer_create(struct dumb_alloc *alloc, int width, int height,
                                      const struct wlr_drm_format *drm_format)
{
    if (!alloc) {
        return NULL;
    }

    if (drm_format_has(drm_format, DRM_FORMAT_MOD_INVALID) &&
        !drm_format_has(drm_format, DRM_FORMAT_MOD_LINEAR)) {
        kywc_log(KYWC_ERROR, "DRM dumb allocator only supports INVALID and LINEAR modifiers");
        return NULL;
    }

    const struct pixel_format *info = pixel_format_from_drm(drm_format->format);
    if (!info) {
        kywc_log(KYWC_ERROR, "DRM format 0x%" PRIX32 " not supported", drm_format->format);
        return NULL;
    } else if (pixel_format_pixels_per_block(info) != 1) {
        kywc_log(KYWC_ERROR, "Block formats are not supported");
        return NULL;
    }

    struct dumb_buffer *buffer = calloc(1, sizeof(*buffer));
    if (!buffer) {
        return NULL;
    }

    wlr_buffer_init(&buffer->base, &buffer_impl, width, height);
    wl_list_insert(&alloc->buffers, &buffer->link);

    buffer->drm_fd = alloc->drm_fd;

    uint32_t bpp = 8 * info->bytes_per_block;
    if (drmModeCreateDumbBuffer(alloc->drm_fd, width, height, bpp, 0, &buffer->handle,
                                &buffer->stride, &buffer->size) != 0) {
        kywc_log_errno(KYWC_ERROR, "Failed to create DRM dumb buffer");
        goto create_destroy;
    }

    buffer->width = width;
    buffer->height = height;
    buffer->format = drm_format->format;

    uint64_t offset;
    if (drmModeMapDumbBuffer(alloc->drm_fd, buffer->handle, &offset) != 0) {
        kywc_log_errno(KYWC_ERROR, "Failed to map DRM dumb buffer");
        goto create_destroy;
    }

    buffer->data =
        mmap(NULL, buffer->size, PROT_READ | PROT_WRITE, MAP_SHARED, alloc->drm_fd, offset);
    if (buffer->data == MAP_FAILED) {
        kywc_log_errno(KYWC_ERROR, "Failed to mmap DRM dumb buffer");
        goto create_destroy;
    }

    memset(buffer->data, 0, buffer->size);

    int prime_fd;
    if (drmPrimeHandleToFD(alloc->drm_fd, buffer->handle, DRM_CLOEXEC, &prime_fd) != 0) {
        kywc_log_errno(KYWC_ERROR, "Failed to get PRIME handle from GEM handle");
        goto create_destroy;
    }

    buffer->dmabuf = (struct wlr_dmabuf_attributes){
        .width = buffer->width,
        .height = buffer->height,
        .format = drm_format->format,
        .modifier = DRM_FORMAT_MOD_LINEAR,
        .n_planes = 1,
        .offset[0] = 0,
        .stride[0] = buffer->stride,
        .fd[0] = prime_fd,
    };

    kywc_log(KYWC_DEBUG, "Allocated %" PRIu32 "x%" PRIu32 " DRM dumb buffer", buffer->width,
             buffer->height);

    return &buffer->base;

create_destroy:
    wlr_buffer_drop(&buffer->base);
    return NULL;
}

static bool dumb_buffer_begin_data_ptr_access(struct wlr_buffer *wlr_buffer, uint32_t flags,
                                              void **data, uint32_t *format, size_t *stride)
{
    struct dumb_buffer *buffer = dumb_buffer_from_buffer(wlr_buffer);
    *data = buffer->data;
    *stride = buffer->stride;
    *format = buffer->format;
    return true;
}

static void dumb_buffer_end_data_ptr_access(struct wlr_buffer *wlr_buffer)
{
    // This space is intentionally left blank
}

static bool buffer_get_dmabuf(struct wlr_buffer *wlr_buffer, struct wlr_dmabuf_attributes *attribs)
{
    struct dumb_buffer *buffer = dumb_buffer_from_buffer(wlr_buffer);
    *attribs = buffer->dmabuf;
    return true;
}

static void buffer_destroy(struct wlr_buffer *wlr_buffer)
{
    struct dumb_buffer *buffer = dumb_buffer_from_buffer(wlr_buffer);

    if (buffer->data) {
        munmap(buffer->data, buffer->size);
    }

    wlr_dmabuf_attributes_finish(&buffer->dmabuf);
    if (buffer->drm_fd >= 0) {
        if (drmModeDestroyDumbBuffer(buffer->drm_fd, buffer->handle) != 0) {
            kywc_log_errno(KYWC_ERROR, "Failed to destroy DRM dumb buffer");
        }
    }

    wl_list_remove(&buffer->link);
    free(buffer);
}

static const struct wlr_buffer_impl buffer_impl = {
    .destroy = buffer_destroy,
    .get_dmabuf = buffer_get_dmabuf,
    .begin_data_ptr_access = dumb_buffer_begin_data_ptr_access,
    .end_data_ptr_access = dumb_buffer_end_data_ptr_access,
};

void dumb_alloc_destroy(struct dumb_alloc *alloc)
{
    if (!alloc) {
        return;
    }

    struct dumb_buffer *buf, *buf_tmp;
    wl_list_for_each_safe(buf, buf_tmp, &alloc->buffers, link) {
        buf->drm_fd = -1;
        wl_list_remove(&buf->link);
        wl_list_init(&buf->link);
    }

    close(alloc->drm_fd);
    free(alloc);
}

struct dumb_alloc *dumb_alloc_create(int drm_fd)
{
    if (drmGetNodeTypeFromFd(drm_fd) != DRM_NODE_PRIMARY) {
        kywc_log(KYWC_ERROR, "Cannot use DRM dumb buffers with non-primary DRM FD");
        return NULL;
    }

    uint64_t has_dumb = 0;
    if (drmGetCap(drm_fd, DRM_CAP_DUMB_BUFFER, &has_dumb) < 0) {
        kywc_log(KYWC_ERROR, "Failed to get DRM capabilities");
        return NULL;
    }

    if (!has_dumb) {
        kywc_log(KYWC_ERROR, "DRM dumb buffers not supported");
        return NULL;
    }

    struct dumb_alloc *alloc = calloc(1, sizeof(*alloc));
    if (!alloc) {
        return NULL;
    }

    alloc->drm_fd = drm_fd;
    wl_list_init(&alloc->buffers);

    kywc_log(KYWC_INFO, "Created DRM dumb allocator");
    return alloc;
}
