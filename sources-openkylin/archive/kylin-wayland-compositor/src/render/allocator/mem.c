// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>

#include <wlr/interfaces/wlr_buffer.h>

#include <kywc/log.h>

#include "render/allocator.h"
#include "render/pixel_format.h"

struct mem_buffer {
    struct wlr_buffer base;
    void *data;
    uint32_t format;
    size_t stride, size;
};

static const struct wlr_buffer_impl buffer_impl;

static struct mem_buffer *mem_buffer_from_buffer(struct wlr_buffer *wlr_buffer)
{
    assert(wlr_buffer->impl == &buffer_impl);
    struct mem_buffer *buffer = wl_container_of(wlr_buffer, buffer, base);
    return buffer;
}

static void buffer_destroy(struct wlr_buffer *wlr_buffer)
{
    struct mem_buffer *buffer = mem_buffer_from_buffer(wlr_buffer);
    free(buffer->data);
    free(buffer);
}

static bool mem_buffer_begin_data_ptr_access(struct wlr_buffer *wlr_buffer, uint32_t flags,
                                             void **data, uint32_t *format, size_t *stride)
{
    struct mem_buffer *buffer = mem_buffer_from_buffer(wlr_buffer);
    *data = buffer->data;
    *format = buffer->format;
    *stride = buffer->stride;
    return true;
}

static void mem_buffer_end_data_ptr_access(struct wlr_buffer *wlr_buffer)
{
    // This space is intentionally left blank
}

static const struct wlr_buffer_impl buffer_impl = {
    .destroy = buffer_destroy,
    .begin_data_ptr_access = mem_buffer_begin_data_ptr_access,
    .end_data_ptr_access = mem_buffer_end_data_ptr_access,
};

struct wlr_buffer *allocator_create_mem_buffer(int width, int height, uint32_t format)
{
    const struct pixel_format *info = pixel_format_from_drm(format);
    if (!info) {
        kywc_log(KYWC_ERROR, "Unsupported pixel format 0x%" PRIX32, format);
        return NULL;
    }

    struct mem_buffer *buffer = calloc(1, sizeof(*buffer));
    if (!buffer) {
        return NULL;
    }

    buffer->format = format;
    buffer->stride = pixel_format_min_stride(info, width);
    buffer->size = buffer->stride * height;

    buffer->data = malloc(buffer->size);
    if (!buffer->data) {
        kywc_log(KYWC_ERROR, "Allocate MEM buffer failed");
        free(buffer);
        return NULL;
    }

    wlr_buffer_init(&buffer->base, &buffer_impl, width, height);
    kywc_log(KYWC_DEBUG, "Allocated %" PRIu32 "x%" PRIu32 " MEM buffer", width, height);

    return &buffer->base;
}
